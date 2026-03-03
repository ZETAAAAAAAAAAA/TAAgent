# RenderDoc to Unreal Engine Material Recreation Workflow

This document describes the general workflow for analyzing game rendering from RenderDoc and recreating materials in Unreal Engine.

---

## 1. Overview

```
RenderDoc Capture ──→ Draw Call Analysis ──→ DXBC Extraction ──→ AI Translation → HLSL ──→ Texture Export ──→ UE Material Recreation
```

---

## 2. RenderDoc Analysis Stage

### 2.1 Frame Information

| MCP Tool | Purpose |
|---------|---------|
| `get_capture_status` | Check if capture is loaded |
| `get_frame_summary` | Get frame statistics, resource counts |
| `get_draw_calls` | Get draw call list and hierarchy |

### 2.2 Target Draw Call Analysis

| MCP Tool | Purpose |
|---------|---------|
| `get_draw_call_details` | Get vertex/index counts, output resources |
| `get_pipeline_state` | Get full pipeline state (shaders, render states, resource bindings) |
| `get_action_timings` | Get GPU timing information |

### 2.3 Shader Analysis

| MCP Tool | Purpose |
|---------|---------|
| `get_shader_info` | Get shader disassembly, constant buffers, resource bindings |

**Output:** DXBC bytecode disassembly (needs AI translation)

### 2.4 Texture Export

| MCP Tool | Purpose |
|---------|---------|
| `get_texture_info` | Get texture metadata (dimensions, format, mip levels) |
| `get_texture_data` | Read texture pixel data (Base64) |
| `save_texture` | Save as PNG/TGA/DDS/HDR formats |
| `save_texture_as_tga` | Save as TGA (supports BC compressed formats) |

**Key properties to record:**
- Texture format (BC1/BC3/BC5/BC6H/BC7, etc.)
- Dimensions and mip levels
- SRV/UAV bind points

---

## 3. Shader Analysis Stage

### 3.1 DXBC → HLSL AI Translation (Critical Step)

**Important:** Do NOT use hardcoded tools to convert DXBC. Use AI to translate.

```
DXBC Bytecode ──→ AI Translation ──→ Human-Readable HLSL
```

**Process:**
1. Use `get_shader_info(event_id, stage)` to get DXBC disassembly
2. AI analyzes and translates DXBC to readable HLSL
3. AI extracts material logic from translated HLSL

**AI Translation Guidelines:**
- Understand DXBC instruction semantics (sample, mul, add, mad, etc.)
- Reconstruct control flow (if/else, loops)
- Identify variable purposes from usage patterns
- Convert to UE-compatible HLSL syntax

**Example Translation:**

```hlsl
// DXBC (before AI translation):
// sample r0.xyzw, v1.xyxx, t0.xyzw, s0
// mul r0.xyz, r0.xyzx, cb0[0].xyzx
// add o0.xyz, r0.xyzx, cb0[1].xyzx

// HLSL (after AI translation):
float4 baseColor = Texture2DSample(BaseTexture, BaseSampler, UV);
float3 result = baseColor.rgb * TintColor.rgb;
result += EmissionColor.rgb;
```

### 3.2 DXBC Decompilation Understanding

| Information Type | Analysis Method |
|-----------------|-----------------|
| Input/Output Structures | Analyze PSInput/PSOutput structs |
| Constant Buffers | Extract parameter names, types, offsets |
| Texture Sampling | Identify sampling logic and UV calculations |
| Mathematical Operations | Understand color processing, blending, lighting |

### 3.3 Material Logic Extraction

Identify from shader code:

1. **UV Processing** - UV scaling, offset, animation
2. **Texture Sampling** - Which textures, sampling methods
3. **Color Calculation** - Color blending, modulation, correction
4. **Opacity Calculation** - Alpha blending, depth fade
5. **Special Effects** - Flipbook, noise, distortion, etc.

---

## 4. Texture Import Stage

### 4.1 Format Mapping

| RDC Format | UE Compression | sRGB | Usage |
|-----------|---------------|------|-------|
| BC1/DXT1 | DXT1 | True | Compressed color (no alpha) |
| BC3/DXT5 | DXT5 | True/False | Color + Alpha or data |
| BC4 | TC_Grayscale | False | Single-channel data (height, mask) |
| BC5 | TC_NormalMap | False | Normal maps (two-channel) |
| BC6H | BC6H | False | HDR textures |
| BC7 | BC7 | True | High-quality color |
| R8G8B8A8 | TC_VectorDisplacementmap | False | Uncompressed RGBA |

### 4.2 ⚠️ 常见编译错误修复

#### 问题: Sampler type is Color, should be Normal

```
[SM6] (Node TextureSampleParameter2D) Sampler type is Color, 
should be Normal for /Game/Textures/Water/T_DetailNormal.T_DetailNormal
```

**原因**: 法线贴图未设置正确的压缩格式，导致采样器类型不匹配。

**解决方案**: 导入时指定 `compression_settings: "TC_NormalMap"`

```python
# ❌ 错误写法
import_texture(
    source_path="T_DetailNormal.png",
    name="T_DetailNormal",
    # 缺少 compression_settings
)

# ✅ 正确写法
import_texture(
    source_path="T_DetailNormal.png",
    name="T_DetailNormal",
    compression_settings="TC_NormalMap"  # 法线贴图必须指定
)
```

#### 纹理类型快速参考

| 纹理用途 | compression_settings | sRGB | sampler_type |
|----------|---------------------|------|--------------|
| **BaseColor/Diffuse** | Default (BC7) | True | Color |
| **Normal Map** | `TC_NormalMap` | False | **Normal** |
| **ORM (Occlusion/Roughness/Metallic)** | Default | False | Color |
| **Emissive** | Default | True | Color |
| **Height/Displacement** | `TC_Grayscale` | False | Grayscale |
| **Flow Map (RG通道)** | Default | False | Color |
| **Mask/Alpha** | Default | False | Alpha |

#### sampler_type 参数值

```
Color           - 标准颜色纹理 (sRGB)
Normal          - 法线贴图 ⚠️ 必须设置!
Grayscale       - 灰度纹理
Alpha           - 单通道遮罩
LinearColor     - 线性颜色纹理 (非sRGB)
LinearGrayscale - 线性灰度纹理
```

#### ⚠️ 创建法线贴图节点时必须指定 sampler_type

```python
# ❌ 错误 - 会导致编译错误
add_material_expression(
    expression_type="TextureSampleParameter2D",
    parameter_name="T_DetailNormal",
    texture="/Game/Textures/Water/T_DetailNormal.T_DetailNormal"
)

# ✅ 正确 - 法线贴图必须设置 sampler_type="Normal"
add_material_expression(
    expression_type="TextureSampleParameter2D",
    parameter_name="T_DetailNormal",
    texture="/Game/Textures/Water/T_DetailNormal.T_DetailNormal",
    sampler_type="Normal"  # 关键!
)
```

### 4.3 Import Tool Call

```
import_texture(
    source_path,
    name,
    destination_path,
    srgb,
    compression_settings,  # ⚠️ 法线贴图必须设置!
    filter,
    address_x,
    address_y
)
```

---

## 5. UE Material Creation Stage

### 5.1 Material Properties Setup

Determine based on original render state:

| Property | Options |
|---------|---------|
| Shading Model | Unlit, DefaultLit, Subsurface, etc. |
| Blend Mode | Opaque, Masked, Translucent, Additive |
| Lighting Mode (Translucent) | Surface, SurfaceTranslucencyVolume |
| Two Sided | True/False |

### 5.2 Node Creation Strategy

**Modular Design Principles:**
- Each Custom HLSL node should have a single responsibility
- Prioritize UE native nodes (Panner, Lerp, TextureSample, etc.)
- Use Custom HLSL only for complex calculations UE cannot express

**Node Type Selection:**

| Requirement | Recommended Node |
|------------|------------------|
| UV Animation | Panner, Rotator |
| Texture Sampling | TextureSample, TextureSampleParameter2D |
| Math Operations | Multiply, Add, Lerp, Saturate |
| Vector Processing | ComponentMask, AppendVector |
| Special Effects | Custom HLSL |

### 5.3 Node Connection

```
connect_material_nodes(
    material_name,
    source_node,
    source_output,
    target_node,
    target_input
)
```

Connect to material output pins:
- BaseColor, Metallic, Specular, Roughness
- Normal, EmissiveColor
- Opacity, OpacityMask
- WorldPositionOffset

---

## 6. Common Issues & Solutions

### 6.1 Custom HLSL Sampler Issues

**Problem:** Cannot declare custom samplers in Custom HLSL

```hlsl
// ❌ Incorrect
SamplerState MySampler;
float4 sample = Texture2DSample(Tex, MySampler, UV);
```

**Solutions:**

1. **Use built-in samplers:**
```hlsl
// ✅ Correct
float4 sample = Texture2DSample(Tex, View.SharedBilinearClampedSampler, UV);
```

2. **Use Texture2DSampleLevel:**
```hlsl
// Specify mip level, no sampler state required
float4 sample = Texture2DSampleLevel(Tex, UV, 0);
```

3. **Recommended approach: Do not sample in Custom HLSL**
   - Custom HLSL only calculates UVs
   - Use UE native TextureSample nodes for sampling

### 6.2 Output Type Mismatch

**Problem:** Custom node Output Type doesn't match code return value

| Output Type | Return Type |
|------------|-------------|
| Float1 | `float` |
| Float2 | `float2` |
| Float3 | `float3` |
| Float4 | `float4` |

**Solution:** Specify correct `output_type` when creating nodes. When updating, the type may not change - create a new node instead.

### 6.3 Texture Format Selection Errors

**Problem:** Texture displays incorrectly after import

| Symptom | Cause | Solution |
|---------|-------|----------|
| Colors too bright/dark | Wrong sRGB setting | Color textures: sRGB=True, Data textures: False |
| Normal map issues | Wrong compression format | Use TC_NormalMap or BC5 |
| Alpha lost | Format doesn't support alpha | Use DXT5/BC7 instead of DXT1 |

### 6.4 Parameter Default Value Loss

**Problem:** Default values become 0 after updating ScalarParameter/VectorParameter node positions

**Solution:** Always pass `value` parameter when updating nodes

### 6.5 Node Input Names

**Problem:** Connection fails with "Input not found" error

**Solution:** Different node types have different input names:

| Node Type | Input Names |
|----------|-------------|
| TextureSample | Coordinates |
| Multiply | A, B |
| Lerp | A, B, Alpha |
| ComponentMask | Input |
| Add | A, B |

### 6.6 Material Compilation Error Troubleshooting

**Common Errors:**

| Error Message | Cause | Solution |
|--------------|-------|----------|
| undeclared identifier | Variable not declared | Check variable names in Custom HLSL |
| type mismatch | Type mismatch | Check output/input types |
| sampler not found | Sampler issue | Use built-in samplers |

---

## 7. MCP Tool Call Sequence (General)

```
# RenderDoc Stage
1. get_capture_status()
2. get_draw_calls() → Select target event_id
3. get_shader_info(event_id, stage) → Get DXBC disassembly
4. [AI translates DXBC to readable HLSL]
5. get_texture_info(resource_id)
6. save_texture(resource_id, output_path)

# UE Stage
1. create_material(name)
2. set_material_properties(...)
3. import_texture(source_path, name, srgb, compression_settings)
4. add_material_expression(...) × N
5. connect_material_nodes(...) × N
6. compile_material(material_name)
```

---

## 8. UE Material Function 知识库

> **知识库路径**: `D:\CodeBuddy\unreal-engine-mcp\KnowledgeBase\UE_MaterialFunction_Library\`

### 8.1 知识库结构

```
KnowledgeBase/UE_MaterialFunction_Library/
├── complete_index.md      # 完整MF索引 (645个函数)
├── index.md               # 路径快速索引
├── MF_Texturing.md        # 纹理处理 MF
├── MF_Shading_Lighting.md # 着色光照 MF
├── MF_Blends.md           # 混合模式 MF
├── MF_Gradient_Masking.md # 梯度遮罩 MF
├── MF_ImageAdjustment.md  # 图像调整 MF
├── MF_MaterialLayer.md    # 材质分层 MF
├── MF_Math.md             # 数学运算 MF
├── MF_Opacity_WPO.md      # 透明度/世界偏移 MF
├── MF_Procedural.md       # 程序化生成 MF
├── MF_Utility.md          # 工具类 MF
└── README.md              # 使用说明
```

### 8.2 常用 MF 快速参考

#### 水体材质相关 MF

| 功能 | MF 名称 | 路径 |
|------|---------|------|
| **UV流动** | FlowMaps | `/Engine/Functions/Engine_MaterialFunctions02/Texturing/FlowMaps.FlowMaps` |
| **UV流动(简化)** | FlowMaps_Simple | `/Engine/Functions/Engine_MaterialFunctions02/Texturing/FlowMaps_Simple.FlowMaps_Simple` |
| **切线空间流动** | TangentSpaceFlow | `/Engine/Functions/Engine_MaterialFunctions01/Texturing/TangentSpaceFlow.TangentSpaceFlow` |
| **Z轴世界流动** | ZWorldSpaceFlow | `/Engine/Functions/Engine_MaterialFunctions01/Texturing/ZWorldSpaceFlow.ZWorldSpaceFlow` |
| **深度淡入** | CameraDepthFade | `/Engine/Functions/Engine_MaterialFunctions01/Opacity/CameraDepthFade.CameraDepthFade` |
| **场景深度采样** | SampleSceneDepth | `/Engine/Functions/Engine_MaterialFunctions02/SampleSceneDepth.SampleSceneDepth` |
| **菲涅尔** | Fresnel_Function | `/Engine/Functions/Engine_MaterialFunctions02/Fresnel_Function.Fresnel_Function` |
| **法线混合** | BlendAngleCorrectedNormals | `/Engine/Functions/Engine_MaterialFunctions02/Utility/BlendAngleCorrectedNormals.BlendAngleCorrectedNormals` |
| **高度混合** | HeightLerp | `/Engine/Functions/Engine_MaterialFunctions02/Texturing/HeightLerp.HeightLerp` |
| **GGX高光** | GGXSpecular | `/Engine/Functions/Engine_MaterialFunctions01/Lighting/GGXSpecular.GGXSpecular` |

#### 纹理处理 MF

| 功能 | MF 名称 | 路径 |
|------|---------|------|
| **三平面投影** | WorldAlignedTexture | `/Engine/Functions/Engine_MaterialFunctions01/Texturing/WorldAlignedTexture.WorldAlignedTexture` |
| **三平面法线** | WorldAlignedNormal | `/Engine/Functions/Engine_MaterialFunctions01/Texturing/WorldAlignedNormal.WorldAlignedNormal` |
| **视差映射** | ParallaxOcclusionMapping | `/Engine/Functions/Engine_MaterialFunctions01/Texturing/ParallaxOcclusionMapping.ParallaxOcclusionMapping` |
| **Flipbook动画** | FlipBook | `/Engine/Functions/Engine_MaterialFunctions02/Texturing/FlipBook.FlipBook` |
| **细节纹理** | DetailTexturing | `/Engine/Functions/Engine_MaterialFunctions01/Texturing/DetailTexturing.DetailTexturing` |
| **纹理轰炸** | Texture_Bombing | `/Engine/Functions/Engine_MaterialFunctions01/Texturing/Texture_Bombing.Texture_Bombing` |

#### 透明度与遮罩 MF

| 功能 | MF 名称 | 路径 |
|------|---------|------|
| **相机深度淡入** | CameraDepthFade | `/Engine/Functions/Engine_MaterialFunctions01/Opacity/CameraDepthFade.CameraDepthFade` |
| **软透明度** | SoftOpacity | `/Engine/Functions/Engine_MaterialFunctions01/Opacity/SoftOpacity.SoftOpacity` |
| **TAA抖动** | DitherTemporalAA | `/Engine/Functions/Engine_MaterialFunctions02/Utility/DitherTemporalAA.DitherTemporalAA` |
| **径向渐变** | RadialGradient | `/Engine/Functions/Engine_MaterialFunctions01/Gradient/RadialGradient.RadialGradient` |
| **线性渐变** | LinearGradient | `/Engine/Functions/Engine_MaterialFunctions01/Gradient/LinearGradient.LinearGradient` |
| **球形遮罩** | SphereGradient-3D | `/Engine/Functions/Engine_MaterialFunctions01/Gradient/SphereGradient-3D.SphereGradient-3D` |

### 8.3 HLSL 到 MF 映射表

| HLSL 函数 | UE MF 替代 |
|-----------|-----------|
| `frac(x)` | Frac 节点 |
| `lerp(a,b,t)` | Lerp 节点 |
| `smoothstep(min,max,x)` | MF_SmoothStep |
| `saturate(x)` | Saturate 节点 |
| `normalize(v)` | Normalize 节点 或 MF_SafeNormalize |
| `pow(x,y)` | Power 节点 |
| `reflect(v,n)` | ReflectionVectorWS 节点 |
| `fresnel()` | Fresnel 节点 或 MF_Fresnel_Function |
| `depthFade()` | DepthFade 节点 或 MF_CameraDepthFade |
| `flowMap()` | MF_FlowMaps / MF_TangentSpaceFlow |

### 8.4 使用 MF 的材质还原策略

**优先级顺序:**
1. **UE原生节点** - Panner, Lerp, Fresnel, DepthFade 等
2. **内置MF** - FlowMaps, WorldAlignedTexture, BlendAngleCorrectedNormals 等
3. **CustomHLSL** - 仅用于复杂计算（3D纹理采样、复杂逻辑）

**示例 - 水体材质还原:**

```
HLSL 函数                    →  UE 实现
─────────────────────────────────────────────────────
CalculateFlowUV()           →  MF_FlowMaps_Simple
CalculateDepthFade()        →  DepthFade 节点
CalculateFresnel()          →  Fresnel 节点
PerturbNormal()             →  MF_BlendAngleCorrectedNormals + CustomHLSL
SampleFlow3D()              →  CustomHLSL (UE无内置3D纹理MF)
CalculateRefraction()       →  SceneTexture + Lerp
CalculateFoam()             →  CustomHLSL + HeightLerp
```

---

## 9. 水体材质还原实战案例

> 基于 `water_material_complete.hlsl` 的完整还原流程

### 9.1 材质属性设置

```
Material: M_Water_Master
├── Material Domain: Surface
├── Blend Mode: Translucent
├── Shading Model: Subsurface (或 DefaultLit)
├── Two Sided: True
└── Translucency:
    └── Lighting Mode: Surface TranslucencyVolume
```

### 9.2 HLSL 功能模块映射

| HLSL 模块 | UE 实现方案 | 复杂度 |
|-----------|------------|--------|
| `CalculateFlowUV()` | **MF_FlowMaps** + CustomHLSL | ⭐⭐⭐ |
| `CalculateDepthFade()` | **DepthFade 节点** | ⭐ |
| `CalculateFresnel()` | **Fresnel 节点** | ⭐ |
| `SampleFlow3D()` | **CustomHLSL** (UE无3D纹理MF) | ⭐⭐⭐⭐ |
| `PerturbNormal()` | **MF_BlendAngleCorrectedNormals** + CustomHLSL | ⭐⭐⭐ |
| `DepthAwareRefraction()` | **SceneTexture** + CustomHLSL | ⭐⭐⭐⭐ |
| `CalculateFoam()` | **MF_HeightLerp** + CustomHLSL | ⭐⭐ |
| `CalculateCaustics()` | **CustomHLSL** | ⭐⭐⭐ |
| `CalculateReflection()` | **ReflectionVectorWS** + Cubemap | ⭐⭐ |

### 9.3 关键节点连接图

```
                    ┌─────────────────────────────────────────────────────────┐
                    │                    M_Water_Master                        │
                    └─────────────────────────────────────────────────────────┘
                                              │
            ┌─────────────────────────────────┼─────────────────────────────────┐
            │                                 │                                 │
            ▼                                 ▼                                 ▼
    ┌───────────────┐               ┌───────────────┐               ┌───────────────┐
    │  WorldPosition │               │     Time      │               │  CameraVector │
    └───────┬───────┘               └───────┬───────┘               └───────┬───────┘
            │                               │                               │
            │                               │                               │
            ▼                               ▼                               ▼
    ┌───────────────┐               ┌───────────────┐               ┌───────────────┐
    │ ComponentMask │               │   Multiply    │               │   Fresnel     │
    │    (R, G)     │               │ (FlowSpeed)   │               │   (5.0)       │
    └───────┬───────┘               └───────┬───────┘               └───────┬───────┘
            │                               │                               │
            └───────────────┬───────────────┘                               │
                            │                                               │
                            ▼                                               │
                    ┌───────────────┐                                       │
                    │  MF_FlowMaps  │──────────────────────┐                │
                    │  (内置MF)     │                      │                │
                    └───────┬───────┘                      │                │
                            │                              │                │
              ┌─────────────┼─────────────┐                │                │
              │             │             │                │                │
              ▼             ▼             ▼                ▼                │
        ┌──────────┐ ┌──────────┐ ┌──────────┐     ┌──────────┐            │
        │ Diffuse  │ │  Normal  │ │   UVs    │     │ SceneTex │            │
        │ Sample   │ │  Sample  │ │  Output  │     │(Refraction)│          │
        └────┬─────┘ └────┬─────┘ └──────────┘     └────┬─────┘            │
             │            │                             │                  │
             │            │                             │                  │
             │            ▼                             │                  │
             │    ┌───────────────┐                     │                  │
             │    │CustomHLSL     │                     │                  │
             │    │(3D Flow Noise)│                     │                  │
             │    └───────┬───────┘                     │                  │
             │            │                             │                  │
             │            ▼                             │                  │
             │    ┌───────────────┐                     │                  │
             │    │ Normal Blend  │◄────────────────────┤                  │
             │    │(MF_BlendAngle │                     │                  │
             │    │CorrectedNormals)                    │                  │
             │    └───────┬───────┘                     │                  │
             │            │                             │                  │
             │            ├─────────────────────────────┤                  │
             │            │                             │                  │
             │            ▼                             ▼                  ▼
             │    ┌───────────────┐             ┌───────────────┐   ┌───────────┐
             │    │CustomHLSL     │             │     Lerp      │◄──┤ Fresnel   │
             │    │(Foam/Sss/etc) │             │ (Refraction/  │   │ Mask      │
             │    └───────┬───────┘             │  Reflection)  │   └───────────┘
             │            │                     └───────┬───────┘
             │            │                             │
             └────────────┼─────────────────────────────┤
                          │                             │
                          ▼                             ▼
                  ┌───────────────┐             ┌───────────────┐
                  │    Lerp       │             │   Multiply    │
                  │(Color Blend)  │             │ (WaterColor)  │
                  └───────┬───────┘             └───────┬───────┘
                          │                             │
                          └──────────────┬──────────────┘
                                         │
                                         ▼
                                 ┌───────────────┐
                                 │   Lerp        │◄────── DepthFade
                                 │ (Foam Blend)  │
                                 └───────┬───────┘
                                         │
                                         ▼
                                 ┌───────────────┐
                                 │ Material Output│
                                 │ BaseColor      │
                                 │ Normal         │
                                 │ Opacity        │
                                 └───────────────┘
```

### 9.4 CustomHLSL 节点代码示例

#### 节点1: FlowUVBlend (Float2 输出)
```hlsl
// 输入: UV(float2), FlowMap(float2), Time(float), Speed(float)
float2 flowDir = FlowMap * 2.0 - 1.0;
float flowTime = frac(Time * Speed);

float2 uv1 = UV + flowDir * flowTime;
float2 uv2 = UV + flowDir * (flowTime + 0.5);
float blend = abs(flowTime - 0.5) * 2.0;

return lerp(uv1, uv2, blend);
```

#### 节点2: FlowNoise3D (Float3 输出)
```hlsl
// 输入: UV(float2), Time(float), Intensity(float)
// 注意: UE5 需要 Volume Texture 支持
// 替代方案: 使用多层2D纹理叠加
float z = frac(Time) * 64.0;
float3 samplePos = float3(UV * Intensity, z);

// 简化版: 用两个2D纹理模拟
float noise1 = Texture2DSample(NoiseTexture, UV + Time * 0.1).r;
float noise2 = Texture2DSample(NoiseTexture, UV * 2.0 - Time * 0.05).r;
float noise3 = Texture2DSample(NoiseTexture, UV * 0.5 + Time * 0.2).r;

return float3(noise1, noise2, noise3);
```

#### 节点3: FoamCalculation (Float 输出)
```hlsl
// 输入: FoamTexture(float), DepthFade(float), Noise(float3), Cutoff(float), Intensity(float)
float foam = FoamTexture;
foam += Noise.b * Intensity;

float edgeFoam = 1.0 - DepthFade;
foam += edgeFoam * 0.5;

return smoothstep(Cutoff, Cutoff + 0.1, foam);
```

#### 节点4: CausticsUV (Float3 输出)
```hlsl
// 输入: WorldPos(float3), Time(float), DepthFade(float)
float2 uv1 = WorldPos.xz * 0.1 + Time * 0.1;
float2 uv2 = uv1 * 1.5 + 0.5;

// 需要外部采样 CausticsTexture
float c1 = CausticsTexture.Sample(uv1).r;
float c2 = CausticsTexture.Sample(uv2).r;

float caustics = c1 * c2 * (1.0 - DepthFade) * 0.5;
return float3(0.8, 0.9, 1.0) * caustics;
```

### 9.5 参数列表

```cpp
// Scalar Parameters
FlowSpeed          = 0.3      // UV流动速度
FlowIntensity      = 1.0      // 流动强度
NormalIntensity    = 0.5      // 法线扰动强度
DepthFadeDistance  = 100.0    // 深度淡入距离
Opacity            = 0.8      // 基础不透明度
Roughness          = 0.1      // 粗糙度
RefractionIntensity = 0.1     // 折射强度
FoamCutoff         = 0.5      // 泡沫阈值
FoamIntensity      = 0.3      // 泡沫强度
FresnelExponent    = 5.0      // 菲涅尔强度

// Vector Parameters
WaterColor         = (0.1, 0.3, 0.5)  // 水的基础颜色
FoamColor          = (1.0, 1.0, 1.0)  // 泡沫颜色
DeepWaterColor     = (0.05, 0.15, 0.25) // 深水颜色

// Texture Parameters
T_FlowMap          // 流向图 (R16G16)
T_DetailNormal     // 细节法线
T_FoamTexture      // 泡沫纹理
T_Caustics         // 焦散纹理
T_NoiseTexture     // 噪声纹理
```

---

## 10. ✅ M_Water_Master 完成状态

### 10.1 材质属性

| 属性 | 值 |
|------|-----|
| **Material Domain** | Surface |
| **Blend Mode** | Translucent |
| **Shading Model** | Subsurface |
| **Two Sided** | True |

### 10.2 RDC 纹理映射 (已完成)

| RDC Slot | 纹理名 | 尺寸 | 格式 | UE路径 | 用途 |
|----------|--------|------|------|--------|------|
| t4 | T_FlowMap | 128×32 | R16G16_UNORM | `/Game/Textures/Water/T_FlowMap` | UV流动方向 |
| t8 | T_Caustics | 512×512 | B8G8R8A8 | `/Game/Textures/Water/T_Caustics` | 焦散纹理 |
| t9 | T_DetailNormal | 256×256 | B8G8R8A8 | `/Game/Textures/Water/T_DetailNormal` | 法线扰动 |
| t10 | T_MainColor | 256×256 | BC7 | `/Game/Textures/Water/T_MainColor` | 主颜色（备用） |

### 10.3 完整节点图 (41个节点)

```
┌─────────────────────────────────────────────────────────────────────────────────────┐
│                              M_Water_Master                                          │
├─────────────────────────────────────────────────────────────────────────────────────┤
│                                                                                      │
│  ┌─────────────┐     ┌──────────────┐     ┌──────────────────┐                      │
│  │WorldPosition│────→│ComponentMask │────→│  FlowUVFromMap   │──┐                   │
│  └─────────────┘     │   (R,G)      │     │  (CustomHLSL)    │  │                   │
│                      └──────────────┘     └──────────────────┘  │                   │
│  ┌─────────────┐            ↑                    ↑              │                   │
│  │    Time     │────→ [Multiply] ←──── FlowSpeed │              │                   │
│  └─────────────┘                               │               │                   │
│                                                │               │                   │
│  ┌─────────────┐     ┌──────────────┐         │               ↓                   │
│  │  T_FlowMap  │────→│   RG输出     │─────────┘     ┌──────────────────┐           │
│  └─────────────┘     └──────────────┘               │   ScreenRefraction│          │
│                                                     │   (CustomHLSL)    │          │
│  ┌─────────────────┐                               └──────────────────┘          │
│  │  T_DetailNormal │────→ [NormalPerturb] ←── FlowNoise                          │
│  └─────────────────┘       │    ↑                                                    │
│                            │    │                                                    │
│                            │  NormalIntensity                                        │
│                            │                                                          │
│                            └──────────────────────→ [Normal Output] ✅              │
│                                                                                      │
│  ┌─────────────┐     ┌──────────────────┐                                          │
│  │WorldPos.XY  │────→│CausticsFromTexture│────→ [Add] ←── T_Caustics.RGB          │
│  └─────────────┘     │  (CustomHLSL)    │         │                                │
│         ↑            └──────────────────┘         │                                │
│         │                    ↑                    ↓                                │
│         │                  Time             ┌──────────────────┐                  │
│         │                    │              │  FinalColorBlend │                  │
│         │                    │              │  (CustomHLSL)    │                  │
│         │                    │              └──────────────────┘                  │
│  ┌─────────────┐             │                      ↑                              │
│  │ T_Caustics  │─────────────┘         ┌───────────┼───────────┐                  │
│  └─────────────┘                       │           │           │                  │
│                                        │           │           │                  │
│                                   WaterColor  DeepWaterColor  Fresnel             │
│                                        │           │           │                  │
│                                        └───────────┴───────────┘                  │
│                                                    │                                │
│                                                    ↓                                │
│                                          ┌──────────────────┐                      │
│                                          │      Lerp        │←── Foam (泡沫遮罩)    │
│                                          └────────┬─────────┘                      │
│                                                   │                                 │
│                                                   └────→ [BaseColor Output] ✅     │
│                                                                                      │
│  ┌─────────────┐     ┌──────────────┐                                              │
│  │DepthFade    │────→│   Lerp       │←── Opacity                                  │
│  └─────────────┘     │              │                                              │
│         ↑            └──────┬───────┘                                              │
│         │                   │                                                      │
│    FadeDistance            Fresnel                                                  │
│         │                   │                                                      │
│         └──────────┬────────┘                                                      │
│                    ↓                                                               │
│           ┌──────────────┐                                                         │
│           │   Multiply   │←── Foam                                                 │
│           └──────┬───────┘                                                         │
│                  │                                                                 │
│                  └────→ [Opacity Output] ✅                                         │
│                                                                                      │
│  ┌─────────────────┐                                                                │
│  │ SubsurfaceColor │──────────────────────→ [SubsurfaceColor Output] ✅            │
│  └─────────────────┘                                                                │
│                                                                                      │
│  [Roughness Param] ──────────────────────→ [Roughness Output] ✅                    │
│                                                                                      │
└─────────────────────────────────────────────────────────────────────────────────────┘
```

### 10.4 CustomHLSL 节点清单

| 节点名称 | 功能 | 输入 | 输出 |
|----------|------|------|------|
| **FlowUVFromMap** | 从流向图计算流动UV | FlowMapRG, Time, Speed | Float2 |
| **FlowNoise** | 程序化流动噪声 | UV, Time, Intensity | Float3 |
| **NormalPerturb** | 法线扰动混合 | BaseNormal, FlowNoise, Intensity | Float3 |
| **CausticsFromTexture** | 焦散纹理采样+程序化增强 | UV, Time, Intensity, DepthFade | Float3 |
| **FinalColorBlend** | 最终颜色混合 | WaterColor, DeepColor, Caustics, Fresnel, DepthFade | Float3 |
| **FoamCalc** | 泡沫计算 | DepthFade, Noise, Cutoff, Intensity | Float |
| **ScreenRefraction** | 屏幕空间折射 | FlowUV, Normal, Intensity, DepthFade | Float2 |

### 10.5 参数完整列表

```cpp
// === Water 组 ===
// Scalar Parameters
FlowSpeed          = 0.3      // UV流动速度
NormalIntensity    = 0.5      // 法线扰动强度
DepthFadeDistance  = 100.0    // 深度淡入距离
Opacity            = 0.8      // 基础不透明度
Roughness          = 0.1      // 粗糙度
RefractionIntensity = 0.1     // 折射强度
FoamCutoff         = 0.5      // 泡沫阈值
FoamIntensity      = 0.3      // 泡沫强度
FresnelExponent    = 5.0      // 菲涅尔强度
CausticsIntensity  = 0.5      // 焦散强度

// Vector Parameters
WaterColor         = (0.1, 0.3, 0.5, 1)   // 浅水颜色
DeepWaterColor     = (0.02, 0.1, 0.2, 1)  // 深水颜色
SubsurfaceColor    = (0.05, 0.2, 0.3, 1)  // 次表面散射颜色

// === Water Textures 组 ===
T_FlowMap          // 流向图 (R16G16) - 控制UV流动方向
T_DetailNormal     // 细节法线 - 法线扰动基础
T_Caustics         // 焦散纹理 - 水下光斑效果
```

### 10.6 已完成功能清单

| 功能 | 状态 | 实现方式 |
|------|------|----------|
| ✅ UV流动动画 | 完成 | FlowUVFromMap (CustomHLSL) |
| ✅ 流向图采样 | 完成 | T_FlowMap 纹理 |
| ✅ 法线扰动 | 完成 | NormalPerturb + T_DetailNormal |
| ✅ 深度淡入 | 完成 | DepthFade 节点 |
| ✅ 菲涅尔反射 | 完成 | Fresnel 节点 + FinalColorBlend |
| ✅ 泡沫效果 | 完成 | FoamCalc (CustomHLSL) |
| ✅ 焦散效果 | 完成 | CausticsFromTexture + T_Caustics |
| ✅ 屏幕空间折射 | 完成 | ScreenRefraction (CustomHLSL) |
| ✅ 深水/浅水颜色混合 | 完成 | FinalColorBlend |
| ✅ 次表面散射 | 完成 | Subsurface Color 输出 |
| ✅ 不透明度控制 | 完成 | Fresnel + DepthFade 混合 |

---

## 11. ✅ 简洁材质架构 (推荐)

### 11.1 架构对比

| 方案 | Custom节点数 | 优点 | 缺点 |
|------|-------------|------|------|
| **碎片化方案** | 7个 | 每个功能独立可调 | 连线复杂，难以理解整体逻辑 |
| **简洁方案** ✅ | 1个 | 一目了然，接近原始HLSL | 参数调整需在代码内修改 |

### 11.2 M_Water_Final 节点图 (21个节点)

```
┌─────────────────────────────────────────────────────────────────────────────────────┐
│                              M_Water_Final                                            │
├─────────────────────────────────────────────────────────────────────────────────────┤
│                                                                                      │
│  ┌─────────────────┐                                                                │
│  │ WorldPosition   │──┬──→ [ComponentMask(R,G)] ──┐                                │
│  └─────────────────┘  │                           │                                │
│                       │                           │                                │
│  ┌─────────────────┐  │    ┌─────────────────────────────────────────────────┐     │
│  │     Time        │──┼───→│                                                   │     │
│  └─────────────────┘  │    │                                                   │     │
│                       │    │          WaterMaterialComplete                    │     │
│  ┌─────────────────┐  │    │              (CustomHLSL)                         │     │
│  │  CameraVector   │──┼───→│  包含完整水体材质逻辑:                            │     │
│  └─────────────────┘  │    │  - UV流动计算                                     │     │
│                       │    │  - 程序化噪声                                      │     │
│  ┌─────────────────┐  │    │  - 法线扰动                                        │     │
│  │   DepthFade     │──┼───→│  - 焦散效果                                        │     │
│  └─────────────────┘  │    │  - 菲涅尔                                          │     │
│                       │    │  - 次表面散射                                      │     │
│  ┌─────────────────┐  │    │  - 深浅水颜色混合                                  │     │
│  │   T_FlowMap     │──┼───→│  - 泡沫计算                                        │     │
│  └─────────────────┘  │    │  - 最终颜色合成                                    │     │
│                       │    │                                                   │     │
│  ┌─────────────────┐  │    │  输入: 16个参数                                    │     │
│  │ T_DetailNormal  │──┼───→│  输出: Float4(BaseColor.rgb, Foam)               │     │
│  └─────────────────┘  │    │                                                   │     │
│                       │    └──────────────────────┬──────────────────────────────┘     │
│  ┌─────────────────┐  │                           │                                   │
│  │   T_Caustics    │──┘                           │                                   │
│  └─────────────────┘                              │                                   │
│                                                   │                                   │
│  ┌─────────────────┐                              │                                   │
│  │   FlowSpeed     │──────────────────────────────┤                                   │
│  └─────────────────┘                              │                                   │
│                                                   │                                   │
│  ┌─────────────────┐                              ▼                                   │
│  │ NormalIntensity │─────────────────────→ [ComponentMask(RGB)] ──→ BaseColor ✅    │
│  └─────────────────┘                                                                  │
│                                                                                      │
│  ┌─────────────────┐     ┌──────────┐                                               │
│  │   WaterColor    │     │ Multiply │←── Opacity                                   │
│  └─────────────────┘     │          │                                               │
│                          └────┬─────┘                                               │
│  ┌─────────────────┐          │                                                      │
│  │ DeepWaterColor  │          └────→ Opacity ✅                                      │
│  └─────────────────┘                                                                  │
│                                                                                      │
│  ┌─────────────────┐                                                                  │
│  │   Roughness     │──────────────────────────────→ Roughness ✅                     │
│  └─────────────────┘                                                                  │
│                                                                                      │
│  ┌─────────────────┐                                                                  │
│  │ SubsurfaceColor │──────────────────────────→ SubsurfaceColor ✅                   │
│  └─────────────────┘                                                                  │
│                                                                                      │
│  ┌─────────────────┐                                                                  │
│  │   FoamCutoff    │                                                                  │
│  └─────────────────┘                                                                  │
│                                                                                      │
│  ┌─────────────────┐                                                                  │
│  │  FoamIntensity  │                                                                  │
│  └─────────────────┘                                                                  │
│                                                                                      │
└─────────────────────────────────────────────────────────────────────────────────────┘
```

### 11.3 单CustomHLSL节点代码

```hlsl
// WaterMaterialComplete - 完整水体材质逻辑
// 文件: test_output/WaterMaterial_SingleCustomHLSL.hlsl

// 输入: 16个参数
float2 baseUV = WorldPosXY;
float time = Time;
float flowSpeed = FlowSpeed;
float normalIntensity = NormalIntensity;
float3 flowMapRG = FlowMapRG;
float3 detailNormal1 = DetailNormal1;
float3 detailNormal2 = DetailNormal2;
float3 causticsTex = CausticsTex;
float depthFade = DepthFade;
float3 viewDir = ViewDir;
float3 worldPos = WorldPos;
float3 waterColor = WaterColor;
float3 deepWaterColor = DeepWaterColor;
float roughness = Roughness;
float foamCutoff = FoamCutoff;
float foamIntensity = FoamIntensity;

// === 核心算法 ===
// 1. UV流动
// 2. 程序化噪声
// 3. 法线扰动
// 4. 焦散效果
// 5. 菲涅尔
// 6. 次表面散射
// 7. 颜色合成
// 8. 泡沫计算

// 输出: Float4(BaseColor.rgb, Foam)
return float4(surfaceColor, foam);
```

### 11.4 参数列表

```cpp
// === Water 组 ===
FlowSpeed          = 0.3      // UV流动速度
NormalIntensity    = 0.5      // 法线扰动强度
Roughness          = 0.1      // 粗糙度
FoamCutoff         = 0.5      // 泡沫阈值
FoamIntensity      = 0.3      // 泡沫强度
Opacity            = 0.8      // 基础不透明度

WaterColor         = (0.1, 0.3, 0.5, 1)   // 浅水颜色
DeepWaterColor     = (0.02, 0.1, 0.2, 1)  // 深水颜色
SubsurfaceColor    = (0.05, 0.2, 0.3, 1)  // 次表面散射颜色

// === Water Textures 组 ===
T_FlowMap          // 流向图
T_DetailNormal     // 细节法线
T_Caustics         // 焦散纹理
```

### 11.5 优势

1. **一目了然** - 所有核心逻辑在单个Custom节点内
2. **易于调试** - 打开节点即可看到完整HLSL代码
3. **接近原始** - 与DXBC翻译后的HLSL结构一致
4. **易于修改** - 直接在代码中调整算法参数

---

## 12. Output Files Checklist

| Stage | File Type | Purpose |
|-------|-----------|---------|
| Shader Export | .hlsl | Disassembled shader code |
| Texture Export | .png/.tga/.dds | Exported texture resources |
| UE Assets | Texture UAsset | Imported textures |
| UE Assets | Material UAsset | Created materials |
| Documentation | WaterMaterial_UE_Restoration_Plan.md | 还原计划文档 |
| Documentation | RenderDoc_to_UE_Workflow.md | 完整工作流文档 |
