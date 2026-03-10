# Control Hiss 流体效果 - 创新技术分析

本文档总结 Control 游戏中 Hiss 屏幕空间流体效果的创新技术点。

---

## 1. R32_UINT 密度打包机制

### 1.1 核心发现

在 Event 13536 和 Event 13870 中发现关键现象：

```
DXBC 指令:
  ld_indexable g_tDepthTexture    // 读取 R32_UINT 数据
  utof r0.x                        // 转为 float
  mul r0.x, r0.x, l(0.000000)     // 清零！
```

**关键洞察：R32_UINT 纹理存储的不是深度，而是密度/权重数据，在渲染阶段被清零。**

### 1.2 数据编码格式

```
R32_UINT 纹理 (426x240):
  - 有效范围: 0x00000000 - 0xFFF00000
  - 无效标记: > 0xFFF00000
  - 存储内容: 密度/权重（非深度）
  
打包方式:
  uint packed_density = (uint)(density * 4294967296.0);
  
解包方式:
  float density = (float)packed;  // 自动归一化
```

### 1.3 清零机制的意义

```hlsl
// 渲染阶段的处理
float density = g_tDepthTexture[coord];  // 读取密度
density = density * 0.0;  // 清零！
float depth = 0.0;        // 使用固定深度进行重投影

// 为什么清零？
// 1. R32_UINT 存储的是密度累积，不是几何深度
// 2. 渲染时使用 depth=0 进行时间重投影，确保正确的前后关系
// 3. 分离了"密度数据"和"几何深度"两个概念
```

### 1.4 对比传统方法

| 传统屏幕空间流体 | Control Hiss |
|-----------------|--------------|
| 单独的深度纹理 + 颜色纹理 | R32_UINT 打包密度 |
| 深度用于深度测试 | 密度用于累积，渲染时清零 |
| 需要多个 Render Target | 单纹理存储 |

**创新点：将密度数据打包为整数，渲染时清零使用固定深度，简化了数据管理。**

---

## 2. atomic_umin 原子操作边界保持

### 2.1 核心算法

Event 13497 使用 `atomic_umin` 实现多线程安全的密度写入：

```hlsl
// CS_FluidSimulation (Event 13497)
[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    // 4点重投影采样
    for (int i = 0; i < 4; i++)
    {
        float2 prevUV = calculate_reprojected_uv(uv, i);
        float2 backTracedPos = prevUV - velocity * deltaTime;
        
        // 原子最小值操作
        uint packedDepth = pack_density(density);
        InterlockedMin(g_rwtDepthTexture[int2(backTracedPos)], packedDepth);
    }
}
```

### 2.2 为什么能保持边界清晰？

```
问题: 多个线程可能写入同一个像素

传统方法:
  Thread A: depth[100] = 0.5
  Thread B: depth[100] = 0.8
  Result: 后写入的覆盖前写入的 (竞争条件)

atomic_umin 方法:
  Thread A: atomic_min(depth[100], 0.5)
  Thread B: atomic_min(depth[100], 0.8)
  Result: depth[100] = 0.5 (最小值，保留前景)

效果:
  - 近处物体的密度总是被保留
  - 远处物体的密度被丢弃
  - 边界始终清晰，无模糊
```

### 2.3 整数坐标采样的重要性

```hlsl
// 使用整数坐标，避免双线性插值
int2 coord = int2(position.xy);
float density = g_tDepthTexture[coord];  // 最近邻采样

// 而非：
// float density = g_tDepthTexture.Sample(uv);  // 双线性插值
```

**整数坐标 + atomic_umin = 完全避免插值模糊**

### 2.4 与传统时间重投影对比

| 传统 TAA | Control Hiss |
|---------|--------------|
| 线性插值历史帧 | 原子操作保留最小值 |
| 边缘模糊 | 边缘清晰 |
| 需要 Ghosting 修复 | 无 Ghosting 问题 |
| 需要速度缓冲裁剪 | 自动处理遮挡 |

---

## 3. 颜色梯度驱动色散效果 ⭐

### 3.1 核心发现

**Event 13846 通过采样颜色场的 Sobel 梯度，实现 RGB 通道独立位移，产生色散效果。**

这是最独特的创新：**不是基于物理的折射色散，而是基于图像处理的颜色梯度位移。**

### 3.2 完整算法

```hlsl
// Event 13846: PS_HissMaskApply

// Step 1: 采样颜色场的 5 个邻居（Sobel 算子）
float4 center = g_tInputTexture[coord];           // 中心 RGBA
float3 left   = g_tInputTexture[coord + int2(-1, 0)].rgb;
float3 right  = g_tInputTexture[coord + int2(1, 0)].rgb;
float3 up     = g_tInputTexture[coord + int2(0, 1)].rgb;
float3 down   = g_tInputTexture[coord + int2(0, -1)].rgb;

// Step 2: 计算每个颜色通道的空间梯度
float3 gradientX = left - right;  // ∂R/∂x, ∂G/∂x, ∂B/∂x
float3 gradientY = down - up;     // ∂R/∂y, ∂G/∂y, ∂B/∂y
float3 colorGradient = gradientX + gradientY;

// Step 3: Alpha 加权梯度位移
float alphaWeight = center.a * center.a * 0.25;
float3 displacement = colorGradient * alphaWeight * g_fWorldTimeDelta * 30.0;

// Step 4: 应用位移（RGB 通道分别位移！）
float3 finalColor = center.rgb + displacement;

// 结果：
//   R 通道向 ∇R 方向位移
//   G 通道向 ∇G 方向位移
//   B 通道向 ∇B 方向位移
//   → RGB 空间分离 = 色散效果！
```

### 3.3 与传统色散对比

| 传统色散（折射） | Control 颜色梯度色散 |
|----------------|---------------------|
| 物理原理：不同波长光折射率不同 | 图像处理：不同颜色通道梯度不同 |
| 公式：n(λ) = n₀ + k/λ² | 公式：C' = C + ∇C * weight |
| 需要 3 次渲染（RGB 分离） | 单次像素着色器 |
| 受折射介质影响 | 受颜色场设计影响 |
| 无法精确控制 | 完全可控 |

### 3.4 色散的两个维度

**空间维度色散（静态）：**

```
RGB 三个通道根据各自的空间梯度向不同方向位移

R'(x,y) = R(x,y) + (∂R/∂x + ∂R/∂y) * weight
G'(x,y) = G(x,y) + (∂G/∂x + ∂G/∂y) * weight
B'(x,y) = B(x,y) + (∂B/∂x + ∂B/∂y) * weight

视觉效果：边缘的 RGB 分离
```

**时间维度色散（动态）：**

```hlsl
// Event 13846 UV 动画
float2 maskUV = uv;
maskUV.x = uv.x * 0.1 + g_fWorldTime * 0.1;   // 慢速水平
maskUV.y = uv.y + g_fWorldTime * 15.0;         // 快速垂直

// 不同速度的 UV 动画产生时间上的颜色分离
视觉效果：动态的颜色拖尾和流动分离
```

### 3.5 关键参数

| 参数 | 值 | 作用 |
|------|-----|------|
| **梯度计算** | Sobel 4 邻居 | 颜色场空间导数 |
| **Alpha 权重** | α² * 0.25 | 位移强度控制 |
| **位移系数** | 30.0 * dt | 色散强度 |
| **UV 动画 X** | time * 0.1 | 时间维度色散（慢） |
| **UV 动画 Y** | time * 15.0 | 时间维度色散（快） |

### 3.6 实现要点

```hlsl
// UE/Niagara 实现建议

// 模块 1: 颜色梯度色散
float3 ComputeColorGradientDispersion(
    Texture2D<float4> colorField,
    float2 uv,
    float alpha,
    float deltaTime
)
{
    int2 coord = int2(uv * resolution);
    
    // Sobel 采样
    float3 center = colorField[coord].rgb;
    float3 left   = colorField[coord + int2(-1, 0)].rgb;
    float3 right  = colorField[coord + int2(1, 0)].rgb;
    float3 up     = colorField[coord + int2(0, 1)].rgb;
    float3 down   = colorField[coord + int2(0, -1)].rgb;
    
    // 颜色梯度
    float3 gradient = (left - right) + (down - up);
    
    // Alpha 加权位移
    float weight = alpha * alpha * 0.25 * deltaTime * 30.0;
    
    return center + gradient * weight;
}

// 模块 2: UV 动画 Mask
float2 AnimatedMaskUV(float2 baseUV, float time)
{
    return float2(
        baseUV.x * 0.1 + time * 0.1,
        baseUV.y + time * 15.0
    );
}
```

---

## 4. Alpha 归一化边界柔化

### 4.1 核心算法

Event 13870 使用 Alpha 归一化实现边缘柔化：

```hlsl
// 最终合成阶段
float4 fluidColor = g_tInputTexture.Sample(uv);
float3 sceneColor = g_tFinalImageSource.Sample(uv);

// Alpha 归一化（关键！）
float alphaNorm = 1.0 / sqrt(fluidColor.a);  // rsq 指令
blendFactor = saturate(blendFactor * alphaNorm);

// 最终合成
float3 result = lerp(sceneColor, fluidColor.rgb, blendFactor);
```

### 4.2 工作原理

```
Alpha 归一化效果：

当 alpha 大（边界中心）:
  - alphaNorm = 1/√α 小
  - blendFactor 减小
  - 混合减弱，流体颜色占主导
  - 边界保持清晰

当 alpha 小（边缘）:
  - alphaNorm = 1/√α 大
  - blendFactor 增大
  - 混合增强，场景颜色占主导
  - 边缘柔化过渡

数学关系:
  blendFactor ∝ √(1/α)
  
  α → 1: blendFactor → 1/√α → 小
  α → 0: blendFactor → 1/√α → 大
```

### 4.3 与传统混合对比

| 传统 Alpha 混合 | Control Alpha 归一化 |
|----------------|---------------------|
| `blend = alpha` | `blend = alpha * (1/√α) = √α` |
| 边缘线性过渡 | 边缘非线性过渡 |
| 边界可能模糊 | 边界清晰但边缘柔化 |
| 简单直接 | 更复杂的边缘控制 |

### 4.4 视觉效果

```
传统 Alpha 混合:
  ┌─────────────┐
  │  ████████   │  边界清晰，但过渡生硬
  │  ████████   │
  │  ████████   │
  └─────────────┘

Alpha 归一化:
  ┌─────────────┐
  │  ▓▓████▓▓   │  中心清晰，边缘柔化
  │  ▓▓████▓▓   │
  │  ▓▓████▓▓   │
  └─────────────┘
```

---

## 5. 双重蓝噪声抖动

### 5.1 核心算法

Event 13514 和 Event 13870 使用双重蓝噪声抖动：

```hlsl
// 第一次蓝噪声采样
int2 noiseCoord = coord & 255;  // 取模 256
float4 blueNoise = g_tStaticBlueNoiseRGBA_0[noiseCoord];

// Golden Ratio 抖动
uint frameOffset = (g_uCurrentFrame & 63) + 1;
float2 jitter = frac(blueNoise.xy + frameOffset * float2(0.754878, 0.569840));

// Golden Ratio: φ ≈ 1.618
// 0.754878 ≈ φ * 0.467
// 0.569840 ≈ φ * 0.352

// 计算抖动偏移（8x8 范围，-4 到 +4）
float2 jitterOffset = jitter * 8.0 - 4.0;
```

### 5.2 为什么使用 Golden Ratio？

```
Golden Ratio 特性:
  φ = (1 + √5) / 2 ≈ 1.618
  φ - 1 = 0.618

关键性质:
  - frac(φ * n) 在 [0, 1) 上均匀分布
  - 任意两个 frac(φ * n) 和 frac(φ * (n+k)) 差异最大化
  - 时间上的抖动样本不相关

效果:
  - 帧间抖动位置差异大
  - 时间累积平均后消除噪声
  - 低分辨率抖动 → 高分辨率效果
```

### 5.3 双重抖动的意义

```
第一重抖动（Event 13514）:
  - 范围: [-4, +4] 像素
  - 作用: 流体密度采样
  - 目的: 消除低分辨率流体的锯齿

第二重抖动（Event 13870）:
  - 范围: [-2, +2] 像素
  - 作用: 最终合成采样
  - 目的: 消除边缘锯齿

双重抖动效果:
  - 低分辨率流体（426x240）
  - 高分辨率视觉效果（1707x960）
  - 无明显锯齿
```

### 5.4 对比传统 TAA

| 传统 TAA | Control 双重抖动 |
|---------|------------------|
| Halton 序列抖动 | Golden Ratio 抖动 |
| 单次抖动 | 双重抖动（不同范围）|
| 需要速度缓冲裁剪 | 无需速度缓冲 |
| 可能有 Ghosting | 无 Ghosting |

---

## 6. 多层 Presence 颜色系统

### 6.1 系统设计

Control 使用 4 层独立颜色系统：

```hlsl
// gfxgraph_p7 常量缓冲区
float4 g_vPresenceLayer1Params;  // (start, end, intensity, falloff)
float4 g_vPresenceLayer2Params;
float4 g_vPresenceLayer3Params;
float4 g_vPresenceLayer4Params;

float3 g_vPresenceLayer1Color;
float3 g_vPresenceLayer2Color;
float3 g_vPresenceLayer3Color;
float3 g_vPresenceLayer4Color;
```

### 6.2 分层渲染原理

```
Presence 效果分层:

Layer 1 (最内层):
  - 距离范围: 0.0 - 0.25
  - 颜色: 高亮核心色
  - 强度: 最高
  - 作用: 效果中心高亮

Layer 2:
  - 距离范围: 0.25 - 0.5
  - 颜色: 主要烟雾色
  - 强度: 中高
  - 作用: 主要效果体

Layer 3:
  - 距离范围: 0.5 - 0.75
  - 颜色: 边缘过渡色
  - 强度: 中等
  - 作用: 边缘过渡

Layer 4 (最外层):
  - 距离范围: 0.75 - 1.0
  - 颜色: 外围淡色
  - 强度: 最低
  - 作用: 外围淡出
```

### 6.3 与色散效果的协同

```
多层颜色 + 颜色梯度色散:

1. Event 13536/13550 生成多层颜色
   ↓
   输出: 不同区域有不同颜色组合
   
2. Event 13846 颜色梯度色散
   ↓
   计算: 每个颜色通道的空间梯度
   应用: RGB 通道独立位移
   效果: 边缘产生色散感
   
协同效应:
  - 多层颜色提供了丰富的梯度变化
  - 不同层的边界产生强烈的梯度
  - 色散效果在层边界最明显
```

### 6.4 实现示例

```hlsl
// 多层颜色计算
float3 ComputePresenceColor(float distance, float3 worldPos)
{
    float3 color = float3(0, 0, 0);
    
    // Layer 1
    float layer1 = smoothstep(g_vPresenceLayer1Params.x, 
                               g_vPresenceLayer1Params.y, 
                               distance);
    layer1 *= g_vPresenceLayer1Params.z;
    color += g_vPresenceLayer1Color * layer1;
    
    // Layer 2
    float layer2 = smoothstep(g_vPresenceLayer2Params.x,
                               g_vPresenceLayer2Params.y,
                               distance);
    layer2 *= g_vPresenceLayer2Params.z;
    color += g_vPresenceLayer2Color * layer2;
    
    // ... Layer 3, 4 类似
    
    return color;
}
```

---

## 7. 三阶段深度检测机制

### 7.1 三个阶段

```
┌─────────────────────────────────────────────────────────────────┐
│                      深度检测完整机制                             │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  阶段 1: 密度场遮挡 (Event 13514)                               │
│  ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━                                 │
│  技术: 场景深度比较 + 深度混合                                  │
│  输入: g_tClipDepth (场景深度)                                  │
│  输出: 密度场 (带无效标记)                                       │
│  作用: 场景遮挡区域被标记为无效                                  │
│                                                                 │
│  阶段 2: 角色遮挡 (Event 13641-13645)                           │
│  ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━                                │
│  技术: 深度测试 discard                                         │
│  作用: 角色正确遮挡流体                                          │
│                                                                 │
│  阶段 3: 最终深度检测 (Event 13870)                             │
│  ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━                                │
│  技术:                                                          │
│    - 双重深度采样 (不同抖动范围)                                 │
│    - Z 裁剪 (g_fHissZClipStart)                                │
│    - 近处混合 (g_fHissNearClipStart)                            │
│    - 场景深度比较                                               │
│  作用: 最终合成时的精确深度控制                                  │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### 7.2 阶段 1 详解：密度场遮挡

```hlsl
// CS_HissEffect (Event 13514)

// 读取场景深度
float sceneDepth = g_tClipDepth.SampleLevel(uv, 0);

// 逆投影: depth → viewZ
float A = g_mViewToClip[2].z;
float B = g_mViewToClip[3].z;
float sceneViewZ = -B / (sceneDepth * (A - 1) + B);

// 计算流体 viewZ
float fluidViewZ = maxDepth * viewZScale;

// 深度差计算
float depthDiff = fluidViewZ - sceneViewZ;

// 深度混合
if (depthDiff < 0.1 && depthDiff > -0.5)
{
    float blendRate = g_fWorldTimeDelta * g_fHissBlendTowardsSceneDepth;
    fluidViewZ = lerp(fluidViewZ, sceneViewZ, blendRate);
}

// 如果流体在场景后面，标记为无效
if (depthDiff > 5.0)
{
    fluidViewZ = -1;  // 无效标记
}
```

### 7.3 阶段 2 详解：角色遮挡

```hlsl
// 角色渲染像素着色器 (Event 13641-13645)

// 采样场景深度
float sceneDepth = g_tLinearDepth[int2(uv * screenSize)];

// 顶点深度
float vertexDepth = v0.z;

// 深度比较并丢弃
float depthDiff = abs(sceneDepth - vertexDepth);
float threshold = vertexDepth * 0.0075;

if (depthDiff > threshold)
    discard;  // 被场景遮挡，丢弃像素
```

### 7.4 阶段 3 详解：最终深度检测

```hlsl
// PS_HissComposite (Event 13870)

// 双重深度采样（不同抖动范围）
float jitter1 = blue_noise_sample_1(coord, frame) * 8.0 - 4.0;  // [-4, 4]
float jitter2 = blue_noise_sample_2(coord, frame) * 4.0 - 2.0;  // [-2, 2]

float fluidDepth1 = g_tDepthTexture[coord + int2(jitter1)];
float fluidDepth2 = g_tDepthTexture[coord + int2(jitter2)];

// Z 裁剪
float zFactor = saturate(depthDiff / g_fHissZClipStart);

// 近处混合
float nearFactor = saturate(viewZ + 1.0 - g_fHissNearClipStart);

// 场景深度比较
float sceneDepth = g_tClipDepth.Sample(uv);
float sceneViewZ = depth_to_viewZ(sceneDepth);
float depthDiff = viewZ - sceneViewZ;

// 最终可见性判断
bool visible = (zFactor > 0.01) && (nearFactor > 0.01) && (depthDiff < threshold);
```

### 7.5 三阶段的必要性

```
为什么需要三个阶段？

阶段 1 (密度场遮挡):
  - 在低分辨率 (426x240) 计算
  - 效率高
  - 粗略的深度剔除

阶段 2 (角色遮挡):
  - 蒙皮网格渲染时
  - 逐像素深度测试
  - 精确的角色遮挡

阶段 3 (最终深度检测):
  - 在全分辨率 (1707x960) 计算
  - 双重抖动采样
  - 最精确的深度控制
  - Z 裁剪和近处混合

协同效应:
  - 阶段 1 提前剔除大量无效像素
  - 阶段 2 确保角色正确遮挡
  - 阶段 3 保证最终质量
```

---

## 8. 压力投影修正

### 8.1 流体不可压缩性

```
Navier-Stokes 方程:
  ∂u/∂t + (u·∇)u = -∇p/ρ + ν∇²u + f

不可压缩条件:
  ∇·u = 0  (速度场无散度)

压力投影:
  目标: 消除速度场的散度
  方法: 求解压力场，然后用压力梯度修正速度
```

### 8.2 实现流程

```
Event 13734: 计算散度
  ∇·v = ∂vx/∂x + ∂vy/∂y

Event 13747-13795: Jacobi 迭代 (8 轮)
  求解泊松方程: ∇²p = -∇·v
  
Event 13810: 压力投影
  v_new = v - ∇p * dt
```

### 8.3 核心算法

```hlsl
// PS_PressureProjection (Event 13810)

// 采样压力场邻居
float pLeft  = g_tFluidPressure[coord + int2(-1, 0)];
float pRight = g_tFluidPressure[coord + int2(1, 0)];
float pUp    = g_tFluidPressure[coord + int2(0, 1)];
float pDown  = g_tFluidPressure[coord + int2(0, -1)];

// 计算压力梯度（中心差分）
float gradX = pLeft - pRight;  // ∂p/∂x
float gradY = pDown - pUp;      // ∂p/∂y

// 修正速度场
float2 velocity = g_tFluidVelocity[coord];
velocity.x -= gradX * 15.0 * g_fWorldTimeDelta;
velocity.y -= gradY * 15.0 * g_fWorldTimeDelta;

// 压力系数 15.0 是关键参数
```

### 8.4 关键参数

| 参数 | 值 | 说明 |
|------|-----|------|
| **Jacobi 迭代** | 8 轮 | 压力求解精度 |
| **压力系数** | 15.0 | 不可压缩性强度 |
| **速度钳制** | ±2000 | 数值稳定性 |

---

## 9. 高斯平流（B 样条核）

### 9.1 核心算法

Event 13827 使用 7 点高斯权重平流：

```hlsl
// PS_FluidAdvectionGaussian (Event 13827)

// 回溯位置
float2 backUV = uv - velocity * g_fWorldTimeDelta;

// 7 点采样
float4 result = float4(0, 0, 0, 0);
for (int i = -3; i <= 3; i++)
{
    float t = (i + 3) / 6.0;  // [0, 1]
    float x = t * t;
    float w = x * (1 - t);
    w = w * w * 46.6666;  // B 样条核权重
    
    float2 sampleUV = backUV + offset * i;
    float4 sample = g_tFluid.SampleLevel(g_sLinearClamp, sampleUV, 0);
    result += sample * w;
}

// 归一化
result *= float4(0.538462, 0.428168, 0.350, 0.142857);
```

### 9.2 B 样条核权重

```
7 点权重分布:

位置:  -3    -2    -1     0     1     2     3
权重:  低    中    高   最高   高    中    低

B 样条核函数:
  w(t) = t²(1-t)² * 46.6666
  
归一化系数:
  (0.538, 0.428, 0.35, 0.143)
```

### 9.3 与普通平流对比

| 普通 Semi-Lagrangian 平流 | 高斯平流 |
|-------------------------|---------|
| 单次采样 | 7 次采样 |
| 双线性插值 | B 样条核插值 |
| 可能有锯齿 | 平滑 |
| 计算快 | 计算慢但质量高 |

### 9.4 应用场景

```
Event 13668: 普通平流
  - 用于流体数据更新
  - 每帧都需要
  - 性能优先

Event 13827: 高斯平流
  - 用于最终渲染前的平滑
  - 只在特定阶段使用
  - 质量优先
```

---

## 10. 完整管线总结

### 10.1 创新技术点汇总

| 技术 | Event | 核心创新 | 效果 |
|------|-------|---------|------|
| **R32_UINT 密度打包** | 13497, 13514 | 整数纹理存储密度，渲染时清零 | 简化数据管理 |
| **atomic_umin 边界保持** | 13497 | 原子最小值操作 + 整数坐标 | 边界清晰无模糊 |
| **颜色梯度色散** | 13846 | Sobel 梯度 RGB 独立位移 | 独特的色散效果 |
| **Alpha 归一化** | 13870 | 1/√α 边缘柔化 | 清晰边界 + 柔化边缘 |
| **双重蓝噪声抖动** | 13514, 13870 | Golden Ratio + 蓝噪声 | 低分辨率→高视觉效果 |
| **多层颜色系统** | 13536, 13550 | 4 层独立颜色 | 丰富的颜色变化 |
| **三阶段深度检测** | 13514, 13641, 13870 | 分级深度控制 | 精确的遮挡关系 |
| **压力投影修正** | 13734-13810 | Jacobi 迭代求解压力场 | 流体不可压缩性 |
| **高斯平流** | 13827 | 7 点 B 样条核 | 平滑的流体运动 |

### 10.2 关键参数汇总

```
密度编码:
  - R32_UINT 格式
  - 无效标记: 0xFFF00000
  
色散控制:
  - Alpha 权重: α² * 0.25
  - 位移系数: 30.0 * dt
  - UV 动画: (time * 0.1, time * 15.0)
  
边界控制:
  - atomic_umin 原子操作
  - Alpha 归一化: 1/√α
  
抖动参数:
  - Blue Noise: 256x256
  - Golden Ratio: (0.754878, 0.569840)
  - 抖动范围: [-4, 4] 和 [-2, 2]
  
流体模拟:
  - 压力系数: 15.0
  - Jacobi 迭代: 8 轮
  - 速度钳制: ±2000
  
分辨率:
  - 流体深度: 426x240 (25%)
  - 速度场: 853x480 (50%)
  - 最终渲染: 1707x960 (100%)
```

### 10.3 性能优化策略

```
1. 分辨率金字塔:
   - 深度场: 25% 分辨率
   - 速度场: 50% 分辨率
   - 最终合成: 100% 分辨率
   
2. 三阶段深度剔除:
   - 阶段 1: 低分辨率粗略剔除
   - 阶段 2: 蒙皮网格逐像素测试
   - 阶段 3: 全分辨率精确控制
   
3. Compute Shader 并行:
   - 密度场更新: 8x8 线程组
   - 原子操作确保正确性
   
4. 双重抖动上采样:
   - 低分辨率流体数据
   - 高分辨率视觉效果
   - 通过抖动消除锯齿
```

---

## 11. UE/Niagara 实现建议

### 11.1 核心模块

```hlsl
// 模块 1: 颜色梯度色散
float3 ComputeColorGradientDispersion(
    Texture2D<float4> colorField,
    float2 uv,
    float alpha,
    float deltaTime
)
{
    int2 coord = int2(uv * resolution);
    float3 center = colorField[coord].rgb;
    float3 left   = colorField[coord + int2(-1, 0)].rgb;
    float3 right  = colorField[coord + int2(1, 0)].rgb;
    float3 up     = colorField[coord + int2(0, 1)].rgb;
    float3 down   = colorField[coord + int2(0, -1)].rgb;
    
    float3 gradient = (left - right) + (down - up);
    float weight = alpha * alpha * 0.25 * deltaTime * 30.0;
    
    return center + gradient * weight;
}

// 模块 2: Alpha 归一化混合
float3 AlphaNormalizedBlend(
    float3 sceneColor,
    float4 fluidColor,
    float blendFactor
)
{
    float alphaNorm = 1.0 / sqrt(max(fluidColor.a, 0.001));
    blendFactor = saturate(blendFactor * alphaNorm);
    return lerp(sceneColor, fluidColor.rgb, blendFactor);
}

// 模块 3: Golden Ratio 抖动
float2 GoldenRatioJitter(float2 blueNoise, uint frame)
{
    float2 goldenRatio = float2(0.754878, 0.569840);
    return frac(blueNoise + (frame % 64 + 1) * goldenRatio);
}
```

### 11.2 Niagara System 结构

```
Niagara System: NS_HissEffect

Emitter: FluidDensity (GPU Compute)
├── Grid2D_SetResolution(426, 240)
├── Custom: AtomicDensityUpdate
│   - atomic_umin 密度写入
│   - 整数坐标采样
└── Output: Density (R32_UINT)

Emitter: FluidVelocity (GPU Compute)
├── Grid2D_SetResolution(853, 480)
├── SampleSceneDepth
├── SampleGeometryVelocity
├── Custom: VelocityGeneration
│   - 屏幕运动向量
│   - 径向力 + 向上力
└── Output: Velocity (R16G16F)

Emitter: FluidSimulation (GPU Compute)
├── Grid2D_AdvectVelocity
├── Custom: ComputeDivergence
├── Custom: JacobiPressureSolver (8 iterations)
├── Custom: PressureProjection
└── Output: CorrectedVelocity

Emitter: FluidRendering (GPU Sprite)
├── SampleDensity (R32_UINT)
├── Custom: ColorGradientDispersion
│   - Sobel 梯度采样
│   - RGB 独立位移
├── Custom: AlphaNormalizedBlend
└── Output: FinalColor
```

### 11.3 材质实现

```hlsl
// UE Material Function: MF_ColorGradientDispersion

// Input: ColorField (Texture2D)
// Input: UV (float2)
// Input: Alpha (float)
// Input: DeltaTime (float)

// Sample 5 neighbors
float3 center = Texture2DSample(ColorField, UV);
float3 left   = Texture2DSample(ColorField, UV + float2(-pixelSize, 0));
float3 right  = Texture2DSample(ColorField, UV + float2(pixelSize, 0));
float3 up     = Texture2DSample(ColorField, UV + float2(0, pixelSize));
float3 down   = Texture2DSample(ColorField, UV + float2(0, -pixelSize));

// Compute gradient
float3 gradient = (left - right) + (down - up);

// Apply displacement
float weight = Alpha * Alpha * 0.25 * DeltaTime * 30.0;
float3 result = center + gradient * weight;

// Output: DispersedColor
```

---

## 12. 总结

Control 的 Hiss 流体效果展示了多个创新技术的巧妙组合：

### 核心创新

1. **R32_UINT 密度打包** - 简化数据管理，分离密度和深度概念
2. **atomic_umin 边界保持** - 原子操作实现无模糊边界
3. **颜色梯度色散** - 图像处理方法实现独特的色散效果
4. **Alpha 归一化** - 非线性边界柔化
5. **双重蓝噪声抖动** - 低分辨率数据产生高视觉效果

### 技术价值

- **性能优化**: 分辨率金字塔 + 三阶段深度剔除
- **质量保证**: 原子操作 + 高斯平流 + 多层颜色
- **视觉效果**: 颜色梯度色散 + Alpha 归一化 + 双重抖动
- **可维护性**: 模块化设计，清晰的管线结构

### 可借鉴点

- R32_UINT 打包可应用于其他需要整数原子操作的场景
- 颜色梯度色散可用于制作独特的视觉风格
- Alpha 归一化可改进传统的 Alpha 混合
- 双重抖动是低分辨率高视觉效果的有效方法
