# Material Reconstruction Strategy

本指南帮助AI从DXBC分析结果重建UE材质。

## 1. 工作流程概览

```
RenderDoc Analysis        DXBC Translation         UE Material Creation
      ↓                         ↓                         ↓
get_pipeline_state()  →   AI理解材质逻辑  →    create_material()
get_shader_info()     →   翻译为HLSL      →    add_material_expression()
get_texture_info()    →   识别纹理用途    →    import_texture()
save_texture()        →   导出纹理        →    connect_material_nodes()
```

## 2. 材质属性决策

### Blend Mode 判断
| 着色器特征 | Blend Mode |
|-----------|------------|
| 无Alpha输出/操作 | Opaque |
| clip() / discard | Masked |
| Alpha混合输出 | Translucent |
| Additive混合 | Additive |

### Shading Model 判断
| 着色器特征 | Shading Model |
|-----------|--------------|
| 标准PBR流程 | DefaultLit |
| 无光照计算 | Unlit |
| SSS计算/皮肤 | Subsurface |
| 植被叶片 | TwoSidedFoliage |
| 头发 | Hair |
| 眼睛 | Eye |

### Two Sided 判断
| 场景 | Two Sided |
|------|-----------|
| 植被、布料、纸张 | True |
| 常规物体 | False |
| 背面剔除材质 | False |

## 3. 纹理导入策略

### 压缩格式选择
| 纹理类型 | compression_settings | sRGB | sampler_type |
|----------|---------------------|------|--------------|
| BaseColor/Diffuse | Default (BC7) | True | Color |
| Normal Map | TC_NormalMap | False | **Normal** |
| ORM (AO/Roughness/Metallic) | Default | False | Color |
| Emissive | Default | True | Color |
| Height/Displacement | TC_Grayscale | False | Grayscale |
| Flow Map (RG通道) | Default | False | Color |
| Mask/Alpha | Default | False | Alpha |

### 纹理用途识别表
| DXBC采样模式 | 可能用途 |
|-------------|---------|
| `t0, s0` + float4采样 + 直接输出 | BaseColor |
| `t1, s1` + 2通道 + 解码(×2-1) | Normal |
| `t2, s2` + 分通道使用 | ORM/Masks |
| `t_flow` + 时间动画 | FlowMap |
| `t_noise` + 分形叠加 | Noise |
| `t_caustics` + 投影采样 | Caustics |

## 4. 节点策略

### 优先使用原生节点
```python
# 推荐: 使用UE原生节点
add_material_expression(material, "Panner", ...)        # UV动画
add_material_expression(material, "Fresnel", ...)       # 菲涅尔
add_material_expression(material, "DepthFade", ...)     # 深度淡入
add_material_expression(material, "Lerp", ...)          # 混合
add_material_expression(material, "TextureSample", ...) # 纹理采样
```

### 内置Material Function
```python
# 使用UE内置MF
add_material_expression(
    material, 
    "MaterialFunctionCall",
    function_path="/Engine/Functions/Engine_MaterialFunctions02/Texturing/FlowMaps.FlowMaps"
)
```

### CustomHLSL 使用场景
仅在以下情况使用CustomHLSL:
1. 3D纹理采样 (UE无原生支持)
2. 复杂数学运算 (如分形噪声)
3. DXBC特殊指令 (UE无等价节点)
4. 性能优化 (合并多个计算)

## 5. 节点组织原则

### 单一职责
每个CustomHLSL节点应该只做一件事:
```hlsl
// ✅ 好: 单一功能
float2 FlowUVBlend(float2 uv, float2 flowMap, float time, float speed) {
    // 只计算流动UV
}

// ❌ 差: 功能过多
float4 WaterMaterialComplete(...) {
    // 包含流动、法线、焦散、泡沫...
}
```

### 输入输出清晰
```hlsl
// 输入命名清晰
// FlowMapRG: 流向图的RG通道
// Time: 时间
// Speed: 流动速度
// 输出: Float2 (流动后的UV)

float2 flowDir = FlowMapRG * 2.0 - 1.0;
float flowTime = frac(Time * Speed);
return uv + flowDir * flowTime;
```

## 6. 参数提取

### 从Constant Buffer提取
```python
# 从 get_shader_info() 返回的常量缓冲中提取
cb_data = shader_info["constant_buffers"][0]["variables"]

for var in cb_data:
    name = var["name"]
    value = var["value"]
    
    # 推断参数类型和用途
    if "Color" in name or "Tint" in name:
        add_material_expression(material, "VectorParameter", 
                               parameter_name=name, value=value)
    elif "Speed" in name or "Scale" in name or "Intensity" in name:
        add_material_expression(material, "ScalarParameter",
                               parameter_name=name, value=value[0])
```

### 常见参数名映射
| DXBC参数名 | UE参数名 | 类型 |
|-----------|---------|------|
| BaseColor / Albedo | BaseColor | Vector |
| NormalIntensity | NormalIntensity | Scalar |
| Roughness / RoughnessValue | Roughness | Scalar |
| Metallic / Metalness | Metallic | Scalar |
| FlowSpeed | FlowSpeed | Scalar |
| DepthFadeDistance | DepthFadeDistance | Scalar |
| Opacity | Opacity | Scalar |

## 7. 连接策略

### 输出连接
```python
# 连接到材质输出
connect_material_nodes(
    material_name,
    source_node=last_node_id,
    source_output="Output",
    target_node="Material",
    target_input="BaseColor"  # 或 Normal, Roughness, 等
)
```

### 常见连接模式
```
纹理采样 → [处理节点] → Material Property

TextureSampleParameter2D → ComponentMask → Material.BaseColor
TextureSampleParameter2D → [NormalBlend] → Material.Normal
ScalarParameter → [Multiply] → Material.Roughness
```

## 8. 调试技巧

### 分步验证
1. 先创建基础材质
2. 逐个添加节点
3. 每添加一个节点就编译检查
4. 最后连接输出

### 常见错误修复
| 错误 | 原因 | 解决方案 |
|-----|------|---------|
| Sampler type is Color, should be Normal | 法线纹理未设置sampler_type | `sampler_type="Normal"` |
| undeclared identifier | CustomHLSL变量未定义 | 检查输入参数名 |
| type mismatch | 输出类型不匹配 | 设置正确的output_type |
