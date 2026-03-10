# Control 游戏 Hiss 屏幕空间流体效果分析

本文档基于 DXBC 反汇编完整还原 Remedy 的 Control 游戏中 Hiss 效果的 GPU 实现。

---

## 1. 效果概述

### 1.1 什么是 Hiss 效果

Hiss 是 Control 游戏中的一种超自然现象，表现为角色周围流动的黑色/红色烟雾状流体效果。该效果特点：
- 屏幕空间流体模拟
- 基于速度场的运动模糊
- 多层次颜色混合
- 与场景深度的空间交互

### 1.2 分析来源

| 属性 | 值 |
|------|-----|
| 游戏 | Control |
| API | D3D11 |
| 帧范围 | Event 13497 - 13795 |
| 分辨率 | 1707x960 |
| 流体分辨率 | 853x480 (速度场), 426x240 (深度场) |

---

## 2. 简化流程图

```
┌─────────────────────────────────────────────────────────────────────────────────────┐
│                        Control Hiss 流体渲染管线 (完整版)                            │
│                           Event 13497 - 13895                                       │
└─────────────────────────────────────────────────────────────────────────────────────┘

═══════════════════════════════════════════════════════════════════════════════════════
  第一阶段: 流体深度更新 (Compute Shader)
═══════════════════════════════════════════════════════════════════════════════════════

  上一帧输出
  ┌──────────────┐    ┌──────────────┐
  │ DepthTexture │    │ FluidVelocity│
  │  R32_UINT    │    │  R16G16F     │
  │  426x240     │    │  853x480     │
  └──────┬───────┘    └──────┬───────┘
         │                   │
         ▼                   ▼
┌─────────────────────────────────────────────────────────────────────────────────────┐
│  Event 13497: CS_FluidSimulation                                                    │
│  ─────────────────────────────────────                                              │
│  输入: Depth(N-1), Velocity(N-1), PreviousClipToClip                                │
│  输出: Depth (更新)                                                                  │
│  功能: 4点时间重投影 + atomic_umin (保持边界清晰)                                    │
└──────────────────────────────────────┬──────────────────────────────────────────────┘
                                       │
                                       ▼
┌─────────────────────────────────────────────────────────────────────────────────────┐
│  Event 13514: CS_HissEffect                                                         │
│  ─────────────────────────                                                          │
│  输入: Depth, SceneDepth, BlueNoise, InputTexture                                   │
│  输出: Depth (融合后)                                                                │
│  功能: 场景深度融合 + 蓝噪声抖动 + 深度扩散 + 无效区域标记                            │
└──────────────────────────────────────┬──────────────────────────────────────────────┘
                                       │
                                       ▼
═══════════════════════════════════════════════════════════════════════════════════════
  第二阶段: 流体可视化和角色渲染 (Pixel Shader)
═══════════════════════════════════════════════════════════════════════════════════════

┌─────────────────────────────────────────────────────────────────────────────────────┐
│  Event 13536/13550: 深度可视化                                                       │
│  ──────────────────────────────────                                                 │
│  输入: Depth, PresenceParams (4层颜色)                                              │
│  输出: 流体颜色 (R8G8B8A8)                                                           │
│  功能: 多层颜色叠加 + 重投影                                                         │
└──────────────────────────────────────┬──────────────────────────────────────────────┘
                                       │
                                       ▼
┌─────────────────────────────────────────────────────────────────────────────────────┐
│  Event 13641-13645: 角色渲染                                                         │
│  ────────────────────────────────                                                   │
│  输入: SkinnedMesh, DepthTexture                                                    │
│  输出: 角色颜色 (带深度测试)                                                          │
│  功能: 蒙皮网格渲染 + 深度测试 discard (正确遮挡)                                     │
└──────────────────────────────────────┬──────────────────────────────────────────────┘
                                       │
                                       ▼
═══════════════════════════════════════════════════════════════════════════════════════
  第三阶段: 速度场生成和平流 (Pixel Shader)
═══════════════════════════════════════════════════════════════════════════════════════

┌─────────────────────────────────────────────────────────────────────────────────────┐
│  Event 13688: PS_VelocityGeneration                                                 │
│  ───────────────────────────────────                                                │
│  输入: SceneDepth, GeometryVelocity, ClipToPrevClip                                 │
│  输出: Velocity (R16G16F, 853x480)                                                  │
│  功能: 屏幕运动向量 + 径向扩散力 + 向上浮力                                           │
└──────────────────────────────────────┬──────────────────────────────────────────────┘
                                       │
                                       ▼
┌─────────────────────────────────────────────────────────────────────────────────────┐
│  Event 13668: PS_FluidAdvection                                                     │
│  ────────────────────────────────                                                   │
│  输入: Velocity, FluidData                                                          │
│  输出: FluidData (平流后, R8G8B8A8)                                                  │
│  功能: Semi-Lagrangian 平流 (回溯采样)                                               │
└──────────────────────────────────────┬──────────────────────────────────────────────┘
                                       │
                                       ▼
═══════════════════════════════════════════════════════════════════════════════════════
  第四阶段: 压力求解 (流体不可压缩性)
═══════════════════════════════════════════════════════════════════════════════════════

┌─────────────────────────────────────────────────────────────────────────────────────┐
│  Event 13734: PS_Divergence                                                         │
│  ────────────────────────────                                                       │
│  输入: Velocity (R16G16F)                                                           │
│  输出: Divergence (R16F)                                                            │
│  功能: 计算速度场散度 ∇·v                                                            │
└──────────────────────────────────────┬──────────────────────────────────────────────┘
                                       │
                                       ▼
┌─────────────────────────────────────────────────────────────────────────────────────┐
│  Event 13747-13795: Jacobi迭代 (8轮)                                                │
│  ─────────────────────────────────                                                  │
│  输入: Divergence, Pressure(N-1)                                                    │
│  输出: Pressure (收敛后, R16F)                                                       │
│  功能: Jacobi迭代求解压力场 (∇²p = -∇·v)                                            │
│        Events: 13747→13759→13771→13783→13795→13810→13827→13846                      │
└──────────────────────────────────────┬──────────────────────────────────────────────┘
                                       │
                                       ▼
┌─────────────────────────────────────────────────────────────────────────────────────┐
│  Event 13810: PS_PressureProjection                                                 │
│  ────────────────────────────────────                                               │
│  输入: Pressure, Velocity                                                           │
│  输出: Velocity (修正后)                                                             │
│  功能: 压力梯度修正速度场: v_new = v - ∇p * 15 * dt                                  │
└──────────────────────────────────────┬──────────────────────────────────────────────┘
                                       │
                                       ▼
┌─────────────────────────────────────────────────────────────────────────────────────┐
│  Event 13827: PS_FluidAdvectionGaussian                                             │
│  ────────────────────────────────────────                                           │
│  输入: Velocity, FluidData                                                          │
│  输出: FluidData (高斯平滑平流)                                                      │
│  功能: 7点高斯权重平流 (B样条核)                                                      │
└──────────────────────────────────────┬──────────────────────────────────────────────┘
                                       │
                                       ▼
═══════════════════════════════════════════════════════════════════════════════════════
  第五阶段: 最终合成和TAA
═══════════════════════════════════════════════════════════════════════════════════════

┌─────────────────────────────────────────────────────────────────────────────────────┐
│  Event 13846: PS_HissMaskApply                                                      │
│  ────────────────────────────────                                                   │
│  输入: InputTexture, HissMask(BC4), FinalImageSource                                │
│  输出: 流体颜色 (应用Mask后)                                                         │
│  功能: Hiss纹理掩码 + 边缘柔化 + 与场景融合                                           │
└──────────────────────────────────────┬──────────────────────────────────────────────┘
                                       │
                                       ▼
┌─────────────────────────────────────────────────────────────────────────────────────┐
│  Event 13847: CopyResource                                                          │
│  ────────────────────────                                                           │
│  输入: FluidColorBuffer                                                             │
│  输出: PreviousFrameBuffer                                                          │
│  功能: 复制当前帧数据供下一帧使用                                                    │
└──────────────────────────────────────┬──────────────────────────────────────────────┘
                                       │
                                       ▼
┌─────────────────────────────────────────────────────────────────────────────────────┐
│  Event 13870: PS_HissComposite                                                      │
│  ────────────────────────────────                                                   │
│  输入: DepthTexture, ClipDepth, BlueNoise, InputTexture, FinalImageSource          │
│  输出: 最终Hiss效果颜色                                                              │
│  功能: 深度检测 + 多采样抖动 + 场景颜色混合                                           │
│        输出到: g_rwtReprojectTexture (TAA输入)                                       │
└──────────────────────────────────────┬──────────────────────────────────────────────┘
                                       │
                                       ▼
                              ┌──────────────┐
                              │  最终画面     │
                              │  Hiss效果     │
                              └──────────────┘

  输出到下一帧:
  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐
  │ DepthTexture │    │ FluidVelocity│    │ HistoryColor │
  │  (更新后)    │    │  (更新后)    │    │  [0-2]       │
  └──────────────┘    └──────────────┘    └──────────────┘
```

---

## 3. 核心资源

### 3.1 流体数据纹理

| 资源ID | 名称 | 分辨率 | 格式 | 用途 |
|--------|------|--------|------|------|
| ResourceId::45207 | g_tDepthTexture / g_rwtDepthTexture | 426x240 | R32_UINT | 流体深度/密度编码 |
| ResourceId::45203 | g_rwtDepthTexture (输出) | 426x240 | R32_UINT | 流体数据写入目标 |
| ResourceId::45200 | g_tFluidVelocity | 853x480 | R16G16_FLOAT | 速度场 |
| ResourceId::45197 | g_tFluid / g_tFluidVelocity | 853x480 | R16G16_FLOAT | 流体数据 (同一纹理复用) |

### 3.2 场景数据纹理

| 资源ID | 名称 | 分辨率 | 格式 | 用途 |
|--------|------|--------|------|------|
| ResourceId::45337 | g_tClipDepth | 1707x960 | R32_TYPELESS | 场景深度缓冲 |
| ResourceId::45470 | g_tGeometryVelocityTexture | 1707x960 | R16G16_FLOAT | 几何速度缓冲 |
| ResourceId::45194 | g_tInputTexture | 1707x960 | R8G8B8A8_UNORM | 场景颜色 |
| ResourceId::3708 | g_tStaticBlueNoiseRGBA_0 | 256x256 | B8G8R8A8_UNORM | 蓝噪声抖动 |

### 3.3 辅助资源

| 资源 | 规格 | 用途 |
|------|------|------|
| g_sbInstanceSkinning | 16-byte stride Buffer | 蒙皮骨骼变换数据 |

---

## 4. 完整渲染管线数据流

### 4.1 数据流图

```
┌─────────────────────────────────────────────────────────────────────────────────────┐
│                     Control Hiss Screen Space Fluid Pipeline                         │
│                              (基于 DXBC 反汇编完整还原)                               │
└─────────────────────────────────────────────────────────────────────────────────────┘

┌─ 帧开始 ─────────────────────────────────────────────────────────────────────────────┐
│                                                                                       │
│  上一帧数据:                                                                          │
│  ┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐                   │
│  │ g_tDepthTexture │    │ g_tFluidVelocity│    │ g_tClipDepth    │                   │
│  │ (R32_UINT)      │    │ (R16G16F)       │    │ (场景深度)       │                   │
│  │ 426x240         │    │ 853x480         │    │ 1707x960        │                   │
│  └────────┬────────┘    └────────┬────────┘    └────────┬────────┘                   │
│           │                      │                       │                            │
└───────────┼──────────────────────┼───────────────────────┼────────────────────────────┘
            │                      │                       │
            ▼                      ▼                       ▼
┌─────────────────────────────────────────────────────────────────────────────────────┐
│  Stage 1: CS_FluidSimulation (Event 13497)                                           │
│  ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━                                             │
│                                                                                       │
│  【核心功能】: 速度场驱动的时间重投影 (Motion Blur / Temporal Reprojection)             │
│                                                                                       │
│  ┌─────────────────────────────────────────────────────────────────────────────────┐ │
│  │ 输入资源:                                                                        │ │
│  │   t0: g_tDepthTexture (R32_UINT, 426x240)     - 上一帧流体深度                  │ │
│  │   t1: g_tFluidVelocity (R16G16F, 853x480)     - 上一帧速度场                    │ │
│  │   cb0: sys_constants (1120 bytes)             - 系统常量                        │ │
│  │                                                                                 │ │
│  │ UAV 输出:                                                                        │ │
│  │   u0: g_rwtDepthTexture (R32_UINT, 426x240)   - 更新后的流体深度                │ │
│  │                                                                                 │ │
│  │ 线程组: 8x8x1 (每个线程处理一个像素)                                             │ │
│  └─────────────────────────────────────────────────────────────────────────────────┘ │
│                                                                                       │
│  【算法详解】:                                                                         │
│                                                                                       │
│  1. 像素坐标计算                                                                      │
│     coord = ThreadGroupID * 8 + ThreadIDInGroup                                      │
│                                                                                       │
│  2. 边界检查                                                                          │
│     if (coord >= textureSize) return;                                                │
│                                                                                       │
│  3. 读取上一帧深度                                                                    │
│     packedDepth = g_tDepthTexture[coord]                                             │
│     if (packedDepth == 0xFFF00000) return;  // 空像素标记                            │
│                                                                                       │
│  4. 计算屏幕空间坐标 (NDC)                                                            │
│     uv = coord / textureSize                                                         │
│     ndc = uv * float2(2, -2) + float2(-1, 1)                                         │
│                                                                                       │
│  5. 多点采样重投影 (4个偏移位置)                                                      │
│     offsets = [+halfPixel, -halfPixel, +halfPixel.y, ...]                           │
│     for each offset:                                                                 │
│         prevUV = ndc + offset                                                        │
│         prevClipPos = mul(g_mPreviousClipToClipNoJitter, float4(prevUV, depth, 1))  │
│         prevScreenPos = prevClipPos.xy / prevClipPos.w                               │
│         velocity = g_tFluidVelocity.Sample(prevScreenPos)                            │
│         backTracedPos = prevScreenPos - velocity * deltaTime                         │
│         atomic_umin(g_rwtDepthTexture, backTracedPos, depth)                         │
│                                                                                       │
└───────────────────────────────────────────────────────────────────────────────────────┘
                                        │
                                        ▼
┌─────────────────────────────────────────────────────────────────────────────────────┐
│  Stage 2: CS_HissEffect (Event 13514)                                                 │
│  ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━                                                 │
│                                                                                       │
│  【核心功能】: 场景深度感知 + Hiss效果生成 + 蓝噪声抖动                                │
│                                                                                       │
│  ┌─────────────────────────────────────────────────────────────────────────────────┐ │
│  │ 输入资源:                                                                        │ │
│  │   t0: g_tClipDepth (R32, 1707x960)            - 场景深度                        │ │
│  │   t1: g_tStaticBlueNoiseRGBA_0 (256x256)      - 蓝噪声                          │ │
│  │   t2: g_tInputTexture (RGBA8, 1707x960)       - 场景颜色 (用于检测空区域)       │ │
│  │   t3: g_tDepthTexture (R32_UINT, 426x240)     - 流体深度                        │ │
│  │   cb0: sys_constants (1120 bytes)             - 系统常量                        │ │
│  │   cb1: gfxgraph_p7 (448 bytes)                - Hiss 效果参数                   │ │
│  │                                                                                 │ │
│  │ UAV 输出:                                                                        │ │
│  │   u0: g_rwtDepthTexture (R32_UINT, 426x240)   - 最终流体深度                    │ │
│  └─────────────────────────────────────────────────────────────────────────────────┘ │
│                                                                                       │
│  【算法详解】:                                                                         │
│                                                                                       │
│  1. 边缘像素跳过 (只处理内部像素)                                                     │
│     if (coord.x <= 0 || coord.y <= 0 || coord.x >= width-1 || coord.y >= height-1)   │
│         return;                                                                      │
│                                                                                       │
│  2. 读取当前流体深度和邻居深度                                                        │
│     depth_center = g_tDepthTexture[coord]                                            │
│     depth_left   = g_tDepthTexture[coord + int2(-1, 0)]                              │
│     depth_right  = g_tDepthTexture[coord + int2(1, 0)]                               │
│     depth_up     = g_tDepthTexture[coord + int2(0, -1)]                              │
│     depth_down   = g_tDepthTexture[coord + int2(0, 1)]                              │
│                                                                                       │
│  3. 深度传播/扩散 (取最大有效深度)                                                    │
│     if (all neighbors have valid depth < 0xFFF00000):                                │
│         maxDepth = max(depth_left, depth_right, depth_up, depth_down)                │
│     else:                                                                            │
│         maxDepth = depth_center                                                      │
│                                                                                       │
│  4. 蓝噪声抖动采样                                                                    │
│     noiseUV = (coord & 255) / 256.0                                                  │
│     blueNoise = g_tStaticBlueNoiseRGBA_0[noiseUV]                                    │
│     frameOffset = (g_uCurrentFrame & 63 + 1) * float2(0.754878, 0.569840)           │
│     jitterUV = frac(blueNoise + frameOffset)                                         │
│     jitterOffset = (jitterUV * 2 - 0.5) * 2 - 0.5                                    │
│                                                                                       │
│  5. 抖动采样流体深度                                                                  │
│     jitteredCoord = coord + jitterOffset                                             │
│     jitteredDepth = g_tDepthTexture[jitteredCoord]                                   │
│                                                                                       │
│  6. 深度平滑/融合                                                                     │
│     if (maxDepth < 0.9999 && abs(jitteredDepth - maxDepth) < 0.00075):               │
│         blendFactor = g_fWorldTimeDelta * 15.0                                       │
│         maxDepth = lerp(maxDepth, jitteredDepth, blendFactor)                        │
│                                                                                       │
│  7. 读取场景深度并转换                                                                │
│     sceneDepth = g_tClipDepth.SampleLevel(uv, 0)                                     │
│     // 逆投影变换: depth -> viewZ                                                     │
│     viewZ = g_mViewToClip[3].z / (sceneDepth * (g_mViewToClip[2].z - 1) + g_mViewToClip[3].z) │
│                                                                                       │
│  8. 深度差计算与混合                                                                  │
│     fluidViewZ = maxDepth * viewZScale                                               │
│     depthDiff = fluidViewZ - sceneViewZ                                              │
│     if (depthDiff > 0.1 && depthDiff < -0.5):                                        │
│         // 在有效深度范围内                                                          │
│         blendRate = g_fWorldTimeDelta * g_fHissBlendTowardsSceneDepth                │
│         fluidViewZ = lerp(fluidViewZ, sceneViewZ, blendRate)                         │
│                                                                                       │
│  9. 检测输入纹理是否为空                                                              │
│     inputColor = g_tInputTexture.SampleLevel(uv, 0)                                  │
│     if (inputColor == 0 && depthDiff > 0.5):                                         │
│         outputDepth = -1  // 标记为无效                                              │
│     else:                                                                            │
│         outputDepth = fluidViewZ * 4294967296.0  // 打包为 UINT                      │
│                                                                                       │
│  10. 写入结果                                                                         │
│      g_rwtDepthTexture[coord] = outputDepth                                          │
│                                                                                       │
└───────────────────────────────────────────────────────────────────────────────────────┘
                                        │
                                        ▼
┌─────────────────────────────────────────────────────────────────────────────────────┐
│  Stage 3: Draw_Quad x2 (Event 13536, 13550)                                           │
│  ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━                                            │
│                                                                                       │
│  【核心功能】: 流体深度可视化 (调试/预览用途)                                          │
│                                                                                       │
│  【像素着色器详解】:                                                                   │
│                                                                                       │
│  1. 计算屏幕UV                                                                        │
│     uv = position.xy * g_vInvOutputRes                                               │
│     ndc = uv * float2(2, -2) + float2(-1, 1)                                         │
│                                                                                       │
│  2. 读取流体深度                                                                      │
│     coord = int2(uv * textureSize)                                                   │
│     packedDepth = g_tDepthTexture[coord]                                             │
│     depth = packedDepth * 0.0  // 深度值 (乘以0表示实际用的是其他处理)               │
│                                                                                       │
│  3. 坐标重投影                                                                        │
│     prevClipPos = mul(g_mClipToPreviousClipNoJitter, float4(ndc, depth, 1))          │
│     prevUV = prevClipPos.xy / prevClipPos.w * float2(0.5, -0.5) + 0.5                │
│                                                                                       │
│  4. 采样输入纹理并输出                                                                │
│     color = g_tInputTexture.Sample(prevUV)                                           │
│     output = color                                                                   │
│                                                                                       │
└───────────────────────────────────────────────────────────────────────────────────────┘
                                        │
                                        ▼
┌─────────────────────────────────────────────────────────────────────────────────────┐
│  Stage 4: DrawIndexed_SkinnedMesh (Event 13641-13645)                                 │
│  ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━                                      │
│                                                                                       │
│  【核心功能】: 渲染受 Hiss 效果影响的角色网格                                          │
│                                                                                       │
│  【顶点着色器详解】:                                                                   │
│                                                                                       │
│  输入:                                                                                │
│    v0: Position (float4)                                                             │
│    v1: Normal (packed int4, 解压为 float3 * 0.000031)                                │
│    v2: Tangent (packed int4, 解压为 float3 * 2 - 1)                                  │
│    v3: UV (int2, 转换为 float2 * 0.000244)                                           │
│    v4: BoneIndices (uint4)                                                           │
│    v5: BoneWeights (uint4, 解压为 float * 0.003922)                                  │
│                                                                                       │
│  处理流程:                                                                            │
│    1. 骨骼蒙皮 (最多4根骨骼影响)                                                      │
│       for (i = 0; i < 4; i++):                                                       │
│           boneMatrix = g_sbInstanceSkinning[boneIndex * 3 + {0,1,2}]                 │
│           skinMatrix += boneWeight * boneMatrix                                      │
│                                                                                       │
│    2. 变换到视图空间                                                                  │
│       viewPos = mul(skinMatrix, position)                                            │
│       viewNormal = mul(skinMatrix, normal)                                           │
│       viewTangent = mul(skinMatrix, tangent)                                         │
│                                                                                       │
│    3. 计算副切线                                                                      │
│       viewBinormal = normalize(cross(viewTangent, viewNormal)) * tangent.w           │
│                                                                                       │
│    4. 变换到裁剪空间                                                                  │
│       clipPos = mul(g_mViewToClip, viewPos)                                          │
│                                                                                       │
│  【像素着色器详解】:                                                                   │
│                                                                                       │
│  输入:                                                                                │
│    v0.z: 顶点视图空间深度                                                             │
│    v5.xy: 屏幕位置                                                                    │
│                                                                                       │
│  处理流程:                                                                            │
│    1. 计算屏幕UV                                                                      │
│       uv = position.xy * g_vInvOutputRes                                             │
│                                                                                       │
│    2. 采样场景颜色                                                                    │
│       sceneColor = g_tFinalImageSource.Sample(uv)                                    │
│                                                                                       │
│    3. 读取线性深度                                                                    │
│       sceneDepth = g_tLinearDepth[int2(uv * screenSize)]                             │
│                                                                                       │
│    4. 深度比较并丢弃                                                                  │
│       depthDiff = abs(sceneDepth - v0.z)                                             │
│       threshold = v0.z * 0.0075                                                      │
│       if (depthDiff > threshold) discard;                                            │
│                                                                                       │
│    5. 输出                                                                            │
│       output = float4(sceneColor, 0)                                                 │
│                                                                                       │
└───────────────────────────────────────────────────────────────────────────────────────┘
                                        │
                                        ▼
┌─────────────────────────────────────────────────────────────────────────────────────┐
│  Stage 5: Draw_Quad - FluidAdvection (Event 13668)                                    │
│  ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━                                      │
│                                                                                       │
│  【核心功能】: 速度场驱动的流体平流 (Advection)                                        │
│                                                                                       │
│  【像素着色器详解】:                                                                   │
│                                                                                       │
│  输入资源:                                                                            │
│    t0: g_tFluid (R16G16F, 853x480)          - 流体数据 (颜色/密度)                   │
│    t1: g_tFluidVelocity (R16G16F, 853x480)  - 速度场 (同一纹理)                      │
│                                                                                       │
│  处理流程:                                                                            │
│    1. 计算屏幕UV并检查边界                                                            │
│       uv = position.xy * g_vInvOutputRes                                             │
│       if (uv.x < 0.001 || uv.y < 0.001 || uv.x > 0.999 || uv.y > 0.999)              │
│           return float4(0, 0, 0, 0);                                                 │
│                                                                                       │
│    2. 采样速度场                                                                      │
│       velocity = g_tFluidVelocity.Sample(uv)                                         │
│                                                                                       │
│    3. 计算回溯位置 (Semi-Lagrangian Advection)                                        │
│       backTracedPos = position.xy - velocity * g_fWorldTimeDelta                     │
│       backTracedUV = backTracedPos * g_vInvOutputRes                                 │
│                                                                                       │
│    4. 采样上一帧流体数据                                                              │
│       fluidData = g_tFluid.Sample(backTracedUV, g_sLinearBorderBlack)                │
│                                                                                       │
│    5. 输出                                                                            │
│       output = fluidData                                                             │
│                                                                                       │
│  【算法原理】:                                                                         │
│                                                                                       │
│  Semi-Lagrangian 平流:                                                                │
│    - 传统正向平流: φ_new(x) = φ_old(x + v*dt)  (不稳定)                              │
│    - 半拉格朗日: φ_new(x) = φ_old(x - v*dt)  (稳定, 无条件稳定)                      │
│                                                                                       │
│  公式:                                                                                │
│    φ^(n+1)(x) = φ^n(x - u(x) * Δt)                                                   │
│                                                                                       │
└───────────────────────────────────────────────────────────────────────────────────────┘
                                        │
                                        ▼
┌─────────────────────────────────────────────────────────────────────────────────────┐
│  Stage 6: Draw_Quad - VelocityGeneration (Event 13688)                                │
│  ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━                                │
│                                                                                       │
│  【核心功能】: 生成速度场 (径向力 + 向上力 + 场景运动)                                 │
│                                                                                       │
│  【像素着色器详解】:                                                                   │
│                                                                                       │
│  输入资源:                                                                            │
│    t0: g_tClipDepth (R32, 1707x960)              - 场景深度                          │
│    t1: g_tGeometryVelocityTexture (R16G16F)      - 几何速度缓冲                      │
│    cb0: sys_constants                            - 系统常量                          │
│    cb1: gfxgraph_p7                              - Hiss 效果参数                     │
│                                                                                       │
│  输出:                                                                                │
│    o0.xy: 速度场 (R16G16F)                                                           │
│    o0.zw: 0                                                                           │
│                                                                                       │
│  处理流程:                                                                            │
│    1. 计算屏幕UV和NDC                                                                 │
│       uv = position.xy * g_vInvOutputRes                                             │
│       ndc = uv * float2(2, -2) + float2(-1, 1)                                       │
│                                                                                       │
│    2. 采样场景深度                                                                    │
│       sceneDepth = g_tClipDepth.Sample(uv)                                           │
│                                                                                       │
│    3. 采样几何速度                                                                    │
│       geometryVelocity = g_tGeometryVelocityTexture.Sample(uv)                       │
│                                                                                       │
│    4. 计算上一帧屏幕位置 (重投影)                                                     │
│       prevClipPos = mul(g_mClipToPreviousClipNoJitter, float4(ndc, sceneDepth, 1))   │
│       prevUV = prevClipPos.xy / prevClipPos.w                                        │
│       prevUV = prevUV * float2(0.5, -0.5) + 0.5                                      │
│                                                                                       │
│    5. 计算屏幕空间运动向量                                                            │
│       motionVector = prevUV - uv                                                     │
│       screenVelocity = motionVector * g_vOutputRes                                   │
│                                                                                       │
│    6. 添加 Hiss 特效力                                                                │
│       // 向上力 (浮力效果)                                                            │
│       upwardForce = float2(0, -g_fHissUpwardsForce * 0.5)                            │
│                                                                                       │
│       // 径向力 (向外扩散)                                                            │
│       radialOffset = uv - 0.5                                                        │
│       radialForce = radialOffset * g_fHissRadialForce                                │
│                                                                                       │
│       // 组合所有力                                                                   │
│       velocity = screenVelocity * 5.0 + upwardForce + radialForce                    │
│                                                                                       │
│    7. 钳制并输出                                                                      │
│       velocity = clamp(velocity, -2000, 2000)                                        │
│       output = float4(velocity, 0, 0)                                                │
│                                                                                       │
│  【速度场公式】:                                                                       │
│                                                                                       │
│  velocity = screenMotion * 5.0                                                       │
│           + float2(0, -g_fHissUpwardsForce * 0.5)  // 向上浮力                       │
│           + (uv - 0.5) * g_fHissRadialForce          // 径向扩散                     │
│                                                                                       │
└───────────────────────────────────────────────────────────────────────────────────────┘
```

---

## 5. DXBC 反汇编详细分析

### 4.1 CS_FluidSimulation (Event 13497) 完整 DXBC 分析

```
Shader hash: 05535afa-c953f4c9-de0f84a9-1e4515be

资源声明:
  dcl_constantbuffer cb0[69]        // sys_constants
  dcl_sampler g_sLinearClamp (s0)   // 线性采样器
  dcl_resource_texture2d (uint) g_tDepthTexture (t0)      // 上一帧深度
  dcl_resource_texture2d (float) g_tFluidVelocity (t1)    // 速度场
  dcl_uav_typed_texture2d (uint) g_rwtDepthTexture (u0)   // 输出深度
  dcl_thread_group 8, 8, 1          // 8x8 线程组
```

**逐行指令分析:**

| 指令 | 解释 |
|------|------|
| `imad r0.xy, vThreadGroupID.xyxx, l(8,8,0,0), vThreadIDInGroup.xyxx` | 计算全局线程ID: coord = GroupID * 8 + LocalID |
| `resinfo_indexable... r1.xy, l(0), g_rwtDepthTexture.xyzw` | 获取纹理尺寸到 r1.xy |
| `uge r1.zw, r0.xxxy, r1.xxxy` | 边界检查: coord >= textureSize? |
| `or r1.z, r1.w, r1.z; if_nz r1.z: ret` | 超出边界则返回 |
| `ld_indexable... r0.z, r0.xyzw, g_tDepthTexture.yzxw` | 读取上一帧深度 |
| `utof r0.z, r0.z` | 转换为浮点 |
| `lt r0.w, l(4294537728.000000), r0.z` | 检查是否为无效标记 (0xFFF00000) |
| `if_nz r0.w: ret` | 无效则返回 |
| `utof r1.zw, r1.xxxy; rcp r2.xy, r1.zwzz` | 计算 1/textureSize |
| `mul r0.z, r0.z, l(0.000000)` | 深度值乘以0 (实际代码中有其他用途) |
| `utof r0.xy, r0.xyxx; div r0.xy, r0.xyxx, r1.zwzz` | 计算 UV = coord / size |
| `mad r0.xy, r0.xyxx, l(2,-2,0,0), l(-1,1,0,0)` | 转换为 NDC: uv * (2,-2) + (-1,1) |
| `mul r3.xw, r2.xxxy, l(0.5,0,0,-0.5)` | 计算半像素偏移 |
| `mad r0.xy, -r2.xyxx, l(0.25,0.25,0,0), r0.xyxx` | 添加采样偏移 |

**重投影矩阵变换 (指令 22-50):**
```
对4个不同的偏移位置进行重投影:

位置1 (中心):
  mul r4.xyzw, r0.yyyy, g_mPreviousClipToClipNoJitter[1].xyzw
  mad r4.xyzw, g_mPreviousClipToClipNoJitter[0].xyzw, r0.xxxx, r4.xyzw
  mad r4.xyzw, g_mPreviousClipToClipNoJitter[2].xyzw, r0.zzzz, r4.xyzw
  add r4.xyzw, r4.xyzw, g_mPreviousClipToClipNoJitter[3].xyzw
  
位置2, 3, 4 类似...
```

**速度采样和回溯 (指令 51-55):**
```
sample_l r4.zw, r4.zwzz, g_tFluidVelocity.zwxy, g_sLinearClamp, l(0)
  // 采样速度场
mul r4.zw, r4.zzzw, g_fWorldTimeDelta.xxxx
  // 速度 * 时间步长
max r4.zw, r4.zzzw, l(0,0,-4,-4); min r4.zw, r4.zzzw, l(0,0,4,4)
  // 钳制速度范围 [-4, 4]
mul r2.xy, r2.xyxx, r4.zwzz
  // 计算回溯距离
```

**原子操作写入 (指令 59-71, 76-89, 94-107, 112-125):**
```
对4个采样点分别执行:

if (depth > 0):
    atomic_umin g_rwtDepthTexture, pixelCoord, packedDepth
    
atomic_umin: 原子最小值操作
  - 确保多个线程写入同一像素时取最小深度
  - 用于正确的深度融合
```

---

### 4.2 CS_HissEffect (Event 13514) 完整 DXBC 分析

```
Shader hash: 7f060b8c-9728feec-1f7311f4-3644ce7c

资源声明:
  dcl_constantbuffer cb0[70]        // sys_constants
  dcl_constantbuffer cb1[6]         // gfxgraph_p7 (Hiss参数)
  dcl_sampler g_sNearestClamp (s0)  // 最近邻采样
  dcl_sampler g_sLinearClamp (s1)   // 线性采样
  dcl_resource_texture2d (float) g_tClipDepth (t0)        // 场景深度
  dcl_resource_texture2d (float) g_tStaticBlueNoiseRGBA_0 (t1)  // 蓝噪声
  dcl_resource_texture2d (float) g_tInputTexture (t2)     // 输入纹理
  dcl_resource_texture2d (uint) g_tDepthTexture (t3)      // 流体深度
  dcl_uav_typed_texture2d (uint) g_rwtDepthTexture (u0)   // 输出
```

**关键指令分析:**

| 指令段 | 功能 |
|--------|------|
| 0-10 | 边界检查，跳过边缘像素 |
| 11-28 | 读取当前像素和邻居深度 (左、右) |
| 29-53 | 读取邻居深度 (上、下)，计算最大深度 |
| 54-63 | 蓝噪声采样和时间抖动 |
| 64-75 | 抖动采样和深度平滑 |
| 76-86 | 场景深度读取和逆投影 |
| 87-96 | 深度混合和裁剪 |
| 97-107 | 最终输出 |

**蓝噪声抖动代码 (指令 54-63):**
```hlsl
// DXBC 伪代码还原:
int2 noiseCoord = coord & 255;  // 取模 256
float4 blueNoise = g_tStaticBlueNoiseRGBA_0[noiseCoord];
uint frameOffset = (g_uCurrentFrame & 63) + 1;
float2 jitter = frac(blueNoise.xy + frameOffset * float2(0.754878, 0.569840));
// Golden ratio 抖动: φ = 1.618..., φ-1 = 0.618...
// 0.754878 ≈ φ * 0.467, 0.569840 ≈ φ * 0.352
```

---

### 4.3 PS_FluidAdvection (Event 13668) 完整 DXBC 分析

```
Shader hash: 66996212-a80e8de4-8e0de4eb-30df31e2

ps_5_0
  dcl_constantbuffer cb0[69]
  dcl_sampler g_sLinearClamp (s0)
  dcl_sampler g_sLinearBorderBlack (s1)   // 边界黑色采样器
  dcl_resource_texture2d (float) g_tFluid (t0)
  dcl_resource_texture2d (float) g_tFluidVelocity (t1)
```

**完整伪代码还原:**

```hlsl
// 从 DXBC 指令 0-14 还原
float4 main(float2 position : SV_Position) : SV_Target
{
    // 1. 计算 UV
    float2 uv = position * g_vInvOutputRes;
    
    // 2. 边界检查 (指令 1-8)
    if (uv.x < 0.001 || uv.y < 0.001 || uv.x > 0.999 || uv.y > 0.999)
        return float4(0, 0, 0, 0);
    
    // 3. 采样速度场 (指令 10)
    float2 velocity;
    g_tFluidVelocity.Sample(g_sLinearClamp, uv, velocity);
    
    // 4. 计算回溯位置 (指令 11-12)
    // Semi-Lagrangian: φ_new(x) = φ_old(x - v * dt)
    float2 backTracedUV = (position - velocity * g_fWorldTimeDelta) * g_vInvOutputRes;
    
    // 5. 采样流体数据 (指令 13)
    float4 fluidData = g_tFluid.Sample(g_sLinearBorderBlack, backTracedUV);
    
    return fluidData;
}
```

---

### 4.4 PS_VelocityGeneration (Event 13688) 完整 DXBC 分析

```
Shader hash: b86ccffc-0a2bb634-0d529823-96817111

ps_5_0
  dcl_constantbuffer cb0[50]        // sys_constants
  dcl_constantbuffer cb1[4]         // gfxgraph_p7
  dcl_sampler g_sLinearClamp (s0)
  dcl_resource_texture2d (float) g_tClipDepth (t0)
  dcl_resource_texture2d (float) g_tGeometryVelocityTexture (t1)
```

**完整伪代码还原:**

```hlsl
// 从 DXBC 指令 0-21 还原
float4 main(float2 position : SV_Position) : SV_Target
{
    // 1. 计算 UV 和 NDC (指令 0-1)
    float2 uv = position * g_vInvOutputRes;
    float2 ndc = uv * float2(2, -2) + float2(-1, 1);
    
    // 2. 重投影到上一帧 (指令 2-3, 6-7)
    float3 prevClipPos;
    prevClipPos = g_mClipToPreviousClipNoJitter[1].xyw * ndc.yyyy;
    prevClipPos = g_mClipToPreviousClipNoJitter[0].xyw * ndc.xxxx + prevClipPos;
    
    // 3. 采样场景深度 (指令 4)
    float sceneDepth = g_tClipDepth.Sample(g_sLinearClamp, uv);
    
    // 4. 采样几何速度 (指令 5)
    float2 geometryVelocity = g_tGeometryVelocityTexture.SampleLevel(g_sLinearClamp, uv, 0);
    
    // 5. 完成重投影 (指令 6-10)
    prevClipPos = g_mClipToPreviousClipNoJitter[2].xyw * sceneDepth + prevClipPos;
    prevClipPos = prevClipPos + g_mClipToPreviousClipNoJitter[3].xyw;
    float2 prevUV = prevClipPos.xy / prevClipPos.z;
    prevUV = prevUV * float2(0.5, -0.5) + float2(0.5, 0.5);
    
    // 6. 计算屏幕空间运动向量 (指令 11-13)
    float2 motionVector = prevUV - uv;
    float2 screenVelocity = -motionVector * g_vOutputRes;
    
    // 7. 添加 Hiss 力 (指令 14-17)
    // 向上力 (指令 14)
    float2 upwardForce = float2(0, g_fHissUpwardsForce * -0.5);
    
    // 基础速度 (指令 15)
    float2 velocity = screenVelocity * 5.0 + upwardForce;
    
    // 径向力 (指令 16-17)
    float2 radialOffset = uv - 0.5;
    velocity = velocity + radialOffset * g_fHissRadialForce;
    
    // 8. 钳制并输出 (指令 18-21)
    velocity = clamp(velocity, float2(-2000, -2000), float2(2000, 2000));
    return float4(velocity, 0, 0);
}
```

---

## 6. 效果参数详解

### 5.1 gfxgraph_p7 常量缓冲区 (448 bytes)

```
字节偏移  | 变量名                      | 类型     | 用途
----------|-----------------------------|----------|----------------------------------
0         | g_vFillColor                | float4   | 填充颜色
16        | g_fFillIntensity            | float    | 填充强度
32        | g_vUVOffsetAndScale         | float4   | UV 偏移和缩放
48        | g_fHissDensity              | float    | Hiss 密度
52        | g_fHissIntensity            | float    | Hiss 强度
56        | g_fHissRadialForce          | float    | 径向力强度 (向外扩散)
60        | g_fHissUpwardsForce         | float    | 向上力强度 (烟雾上升)
64        | g_fHissNormalForce          | float    | 法向力强度
68        | g_fHissNormalUpwardsForce   | float    | 法向向上力
72        | g_fHissNearClipStart        | float    | 近裁剪起点
76        | g_fHissZClipStart           | float    | Z裁剪起点
80        | g_fHissBlendTowardsSceneDepth | float  | 与场景深度混合速率
84        | g_vPresencePosition         | float3   | Presence 效果中心位置
96        | g_fPresencePerspectivity    | float    | Presence 透视强度
100       | g_fPresenceIntensity        | float    | Presence 整体强度
112       | g_vPresenceLayer1Params     | float4   | 第1层参数 (start, end, intensity, falloff)
128       | g_vPresenceLayer2Params     | float4   | 第2层参数
144       | g_vPresenceLayer3Params     | float4   | 第3层参数
160       | g_vPresenceLayer4Params     | float4   | 第4层参数
176       | g_vPresenceLayer1Color      | float3   | 第1层颜色
192       | g_vPresenceLayer2Color      | float3   | 第2层颜色
208       | g_vPresenceLayer3Color      | float3   | 第3层颜色
224       | g_vPresenceLayer4Color      | float3   | 第4层颜色
240-288   | g_vLaunchHighlightBoneMask* | uint4[4] | 骨骼遮罩
304       | g_vLaunchHighlightColor     | float4   | 高亮颜色
320       | g_fLaunchHighlightThickness | float    | 高亮厚度
324       | g_vMenuColor0               | float3   | 菜单颜色0
336       | g_vMenuColor1               | float3   | 菜单颜色1
348       | g_fMenuPow                  | float    | 菜单幂次
352       | g_vOvershieldPositionInWorld| float3   | 护盾世界位置
368       | g_vOvershieldScale          | float3   | 护盾缩放
384       | g_vOvershieldOffset         | float3   | 护盾偏移
400       | g_vOvershieldParams         | float4   | 护盾参数
416       | g_fNoiseTime                | float    | 噪声时间
420       | g_vWorldPos                 | float3   | 世界位置
432       | g_fRadiusScalar             | float    | 半径缩放
436       | g_fFlipUV                   | float    | UV翻转
440       | g_vOutputRange              | float2   | 输出范围
```

### 5.2 sys_constants 常量缓冲区 (1120 bytes)

```
字节偏移  | 变量名                      | 类型     | 用途
----------|-----------------------------|----------|----------------------------------
0         | g_vScreenRes                | float2   | 屏幕分辨率
8         | g_vInvScreenRes             | float2   | 屏幕分辨率倒数
16        | g_vOutputRes                | float2   | 输出分辨率
24        | g_vInvOutputRes             | float2   | 输出分辨率倒数
32        | g_mWorldToView              | float4x4 | 世界到视图矩阵
96        | g_mViewToWorld              | float4x4 | 视图到世界矩阵
160       | g_mViewToClip               | float4x4 | 视图到裁剪矩阵
224       | g_mClipToView               | float4x4 | 裁剪到视图矩阵
288       | g_mWorldToClip              | float4x4 | 世界到裁剪矩阵
352       | g_mClipToWorld              | float4x4 | 裁剪到世界矩阵
416       | g_mClipToPreviousClip       | float4x4 | 裁剪到上一帧裁剪矩阵
480       | g_mViewToPreviousClip       | float4x4 | 视图到上一帧裁剪矩阵
544       | g_mPreviousViewToView       | float4x4 | 上一帧视图到当前视图矩阵
608       | g_mPreviousWorldToClip      | float4x4 | 上一帧世界到裁剪矩阵
672       | g_mPreviousViewToClip       | float4x4 | 上一帧视图到裁剪矩阵
736       | g_mClipToPreviousClipNoJitter| float4x4| 裁剪到上一帧裁剪(无抖动)
800       | g_mPreviousClipToClipNoJitter| float4x4| 上一帧裁剪到当前裁剪(无抖动)
864       | g_vViewPoint                | float4   | 视点位置
880       | g_fInvNear                  | float    | 近平面倒数
896       | g_mViewToCameraView         | float4x4 | 视图到相机视图矩阵
960       | g_mCameraViewToCameraClip   | float4x4 | 相机视图到相机裁剪矩阵
1024      | g_mCameraClipToView         | float4x4 | 相机裁剪到视图矩阵
1088      | g_fWorldTime                | float    | 世界时间
1092      | g_fWorldTimeDelta           | float    | 世界时间增量 (帧时间)
1096      | g_fRealTime                 | float    | 真实时间
1100      | g_fRealTimeDelta            | float    | 真实时间增量
1104      | g_fLastValidWorldTimeDelta  | float    | 最后有效世界时间增量
1108      | g_uTemporalFrame            | uint     | 时间帧计数
1112      | g_uCurrentFrame             | uint     | 当前帧计数
```

---

## 7. R32_UINT 深度编码格式

### 6.1 实际编码方式

从 DXBC 指令分析，深度编码方式如下：

```hlsl
// 打包 (从 CS_HissEffect 指令 104-105):
//   mul r0.z, r0.z, l(4294967296.000000)  // 深度值乘以 2^32
//   ftou r0.z, r0.z                        // 转换为 uint
uint pack_depth(float depth)
{
    // depth 范围 [0, 1] 映射到 [0, 2^32)
    return (uint)(depth * 4294967296.0);
}

// 解包 (从 CS_FluidSimulation 指令 8-9):
//   ld_indexable... r0.z, r0.xyzw, g_tDepthTexture.yzxw
//   utof r0.z, r0.z
float unpack_depth(uint packed)
{
    return (float)packed;  // 直接转换为浮点，隐式除以 2^32
}

// 无效标记检查 (指令 10):
//   lt r0.w, l(4294537728.000000), r0.z
//   if_nz r0.w: ret
bool is_invalid(uint packed)
{
    // 4294537728 = 0xFFF00000
    // 当 packed > 0xFFF00000 时视为无效
    return packed > 0xFFF00000;
}
```

### 6.2 存储精度

```
R32_UINT 格式:
  - 32位无符号整数
  - 精度: 2^-32 ≈ 2.3e-10 (极高精度)
  - 有效范围: 0x00000000 - 0xFFF00000
  
深度映射:
  float depth (0-1) --> uint (0 - 4294967295)
```

---

## 8. 与 Niagara Fluids 对比

### 7.1 技术对比

| 特性 | Control Hiss | Niagara Grid2D |
|------|-------------|----------------|
| 空间 | 屏幕空间 | 世界空间 |
| 分辨率 | 自适应 (50%/25%) | 固定 |
| 数据结构 | R32_UINT 打包 | 多纹理分离 |
| 平流 | Semi-Lagrangian | Semi-Lagrangian |
| 深度交互 | 场景深度感知 | 边界条件 |
| 时间累积 | 重投影 + 原子操作 | 双缓冲 |
| 力场 | 径向 + 向上 | 多种力模块 |

### 7.2 可借鉴的技术点

1. **R32_UINT 打包**: 节省内存带宽
2. **蓝噪声抖动**: 低分辨率噪声高分辨率效果
3. **原子操作融合**: 正确的多线程深度写入
4. **场景深度感知**: 与场景正确交互
5. **Golden Ratio 抖动**: 时间稳定的采样

---

## 9. 在 UE/Niagara 中实现建议

### 8.1 完整实现方案

```
Niagara System: NS_HissEffect

Emitter: HissFluid (GPU Compute)
├── Spawn Script:
│   └── Grid2D_SetResolution(853, 480)
│
├── Update Script:
│   ├── Grid2D_AdvectVelocity          // 平流速度
│   ├── Custom: RadialForce            // 径向力 (向外)
│   ├── Custom: UpwardsForce           // 向上力 (Buoyancy)
│   ├── Custom: SceneDepthInteraction  // 场景深度交互
│   └── Grid2D_IntegrateForces         // 力积分
│
└── Render:
    └── Grid2D_SetRTValues             // 输出到 RenderTarget

Emitter: VelocityField
├── Update Script:
│   ├── SampleSceneDepth               // 采样场景深度
│   ├── SampleGeometryVelocity         // 采样几何速度
│   └── CalculateMotionVector          // 计算运动向量
│
└── Render:
    └── OutputVelocityField            // 输出速度场
```

### 8.2 自定义模块代码

**RadialForce 模块:**
```hlsl
// Niagara HLSL Module
float2 center = View.WorldToClip(PresencePosition).xy / View.WorldToClip(PresencePosition).w;
float2 radialDir = normalize(NDCPosition - center);
float radialForce = HissRadialForce * (1.0 - length(NDCPosition - center));
PhysicsForce.xy += radialDir * radialForce;
```

**SceneDepthInteraction 模块:**
```hlsl
// 与场景深度交互
float sceneDepth = Texture2DSample(SceneDepthTexture, Sampler, UV).r;
float fluidDepth = Grid2D.GetFloatValue(Attribute=Depth, Index);
float depthDiff = abs(sceneDepth - fluidDepth);

if (depthDiff < BlendThreshold) {
    // 靠近场景表面时衰减
    float blend = saturate(depthDiff / BlendThreshold);
    Density *= blend;
    
    // 混合到场景深度
    float newDepth = lerp(fluidDepth, sceneDepth, HissBlendTowardsSceneDepth * DeltaTime);
    Grid2D.SetFloatValue(Attribute=Depth, Index, newDepth);
}
```

---

## 10. 性能分析

### 9.1 计算复杂度

| 阶段 | 分辨率 | 操作 | 估算耗时 |
|------|--------|------|----------|
| CS_FluidSimulation | 426x240 | 4次采样 + 4次原子操作 | ~0.2ms |
| CS_HissEffect | 426x240 | 5次采样 + 深度计算 | ~0.3ms |
| PS_FluidAdvection | 853x480 | 2次采样 | ~0.1ms |
| PS_VelocityGeneration | 1707x960 | 2次采样 + 矩阵运算 | ~0.1ms |

### 9.2 内存带宽

```
读取:
  g_tDepthTexture: 426x240x4 = 409KB
  g_tFluidVelocity: 853x480x4 = 1.6MB
  g_tClipDepth: 1707x960x4 = 6.5MB
  
写入:
  g_rwtDepthTexture: 426x240x4 = 409KB
  g_tFluidVelocity (pingpong): 1.6MB

总计: ~10MB 带宽/帧
```

---

## 11. 三个关键技术问题深入分析（更新版）

### 关键新发现：深度清零现象

在分析 Event 13536 和 Event 13870 的 DXBC 时，发现一个重要现象：

```
Event 13536 (深度可视化):
   7: ld_indexable g_tDepthTexture    // 读取深度
   8: utof r0.x                        // 转为float
   9: mul r0.x, r0.x, l(0.000000)     // 清零！

Event 13870 (最终合成):
  16: ld_indexable g_tDepthTexture    // 读取深度
  17: utof r0.x                        // 转为float
  18: mul r0.x, r0.x, l(0.000000)     // 清零！
```

**这意味着：R32_UINT 纹理存储的不是真正的深度，而是其他数据（如密度/权重），在渲染阶段被忽略！**

---

### 11.1 问题一：重投影如何保持边界清晰

**核心发现：使用原子操作 + 整数坐标采样，完全避免双线性插值**

#### 11.1.1 传统方法的问题

#### 11.1.1 传统方法的问题

传统的时间重投影使用双线性采样：
```hlsl
// 传统方法 - 会导致边界模糊
float2 prevUV = reprojection(uv, depth, velocity);
float4 prevColor = Texture.Sample(LinearSampler, prevUV);  // 双线性插值！
```

问题：
- 双线性插值会在边界处混合相邻像素
- 多帧累积后边界逐渐模糊
- 细节特征丢失

#### 11.1.2 Control 的解决方案

从 DXBC 指令分析 (Event 13497):

```
步骤1: 计算多个采样点的整数坐标
┌────────────────────────────────────────────────────────────────────────────┐
│  指令 20-21:                                                                │
│    mul r3.xw, r2.xxxy, l(0.500000, 0.000000, 0.000000, -0.500000)         │
│    mad r0.xy, -r2.xyxx, l(0.250000, 0.250000, 0, 0), r0.xyxx              │
│                                                                            │
│  解释: 计算 4 个采样偏移 (半像素偏移)                                        │
│    offset1 = (+halfPixel, +halfPixel)                                      │
│    offset2 = (+halfPixel, -halfPixel)                                      │
│    offset3 = (-halfPixel, +halfPixel)                                      │
│    offset4 = (-halfPixel, -halfPixel)                                      │
└────────────────────────────────────────────────────────────────────────────┘

步骤2: 使用整数坐标采样 (不是 Sample!)
┌────────────────────────────────────────────────────────────────────────────┐
│  指令 61-62:                                                                │
│    mad r4.xy, r4.xyxx, l(0.5, -0.5, 0, 0), r2.xyxx                        │
│    mad r4.xy, r4.xyxx, r1.zwzz, l(0.5, 0.5, 0, 0)                         │
│    ftoi r4.xy, r4.xyxx    // 关键！转换为整数坐标                           │
│                                                                            │
│  解释: 计算重投影后的整数像素坐标，不进行插值                                │
└────────────────────────────────────────────────────────────────────────────┘

步骤3: 原子操作写入，确保深度融合正确
┌────────────────────────────────────────────────────────────────────────────┐
│  指令 70:                                                                   │
│    atomic_umin g_rwtDepthTexture, r4.xyxx, r2.z                           │
│                                                                            │
│  解释:                                                                      │
│    - atomic_umin: 原子最小值操作                                           │
│    - 当多个线程写入同一像素时，保留最小的深度值                              │
│    - 这确保了前景物体的边界不会被背景模糊                                   │
└────────────────────────────────────────────────────────────────────────────┘
```

#### 11.1.3 完整的重投影伪代码

```hlsl
// Control Hiss 重投影算法
[numthreads(8, 8, 1)]
void CS_FluidSimulation(uint3 DTid : SV_DispatchThreadID)
{
    int2 coord = DTid.xy;
    
    // 1. 读取当前深度
    uint packedDepth = g_tDepthTexture[coord];
    if (packedDepth > 0xFFF00000) return;  // 无效标记检查
    
    float depth = unpack_depth(packedDepth);
    
    // 2. 计算 4 个采样偏移 (半像素偏移)
    float2 halfPixel = 0.5 / textureSize;
    float2 offsets[4] = {
        float2(+halfPixel.x, +halfPixel.y),
        float2(+halfPixel.x, -halfPixel.y),
        float2(-halfPixel.x, +halfPixel.y),
        float2(-halfPixel.x, -halfPixel.y)
    };
    
    // 3. 对每个偏移进行重投影
    [unroll]
    for (int i = 0; i < 4; i++)
    {
        // 3.1 计算 NDC 坐标
        float2 ndc = (coord + 0.5 + offsets[i]) / textureSize * 2.0 - 1.0;
        
        // 3.2 重投影到上一帧
        float4 prevClipPos = mul(g_mPreviousClipToClipNoJitter, float4(ndc, depth, 1.0));
        float2 prevUV = prevClipPos.xy / prevClipPos.w * 0.5 + 0.5;
        
        // 3.3 采样速度场 (这里用线性采样，但后续用整数坐标)
        float2 velocity = g_tFluidVelocity.SampleLevel(g_sLinearClamp, prevUV, 0);
        
        // 3.4 回溯位置
        float2 backTracedPos = prevUV - velocity * g_fWorldTimeDelta;
        
        // 3.5 关键：转换为整数坐标！
        int2 intCoord = int2(backTracedPos * textureSize);
        
        // 3.6 边界检查
        if (all(intCoord >= 0) && all(intCoord < textureSize))
        {
            // 3.7 原子操作写入！
            // 如果多个线程写入同一像素，保留最小深度
            InterlockedMin(g_rwtDepthTexture[intCoord], packedDepth);
        }
    }
}
```

#### 11.1.4 为什么边界清晰？

| 技术 | 效果 |
|------|------|
| **整数坐标采样** | 避免双线性插值，像素值不被混合 |
| **atomic_umin** | 多线程写入时保留最小深度（前景），避免背景污染前景 |
| **R32_UINT 格式** | 整数存储，不会有浮点精度问题 |
| **多采样点独立写入** | 每个采样点独立处理，不混合 |

```
对比：

传统方法:
  像素 A (深度 0.1) + 像素 B (深度 0.9) 
  → 双线性插值 → 0.5 (边界模糊)
  
Control 方法:
  像素 A (深度 0.1) + 像素 B (深度 0.9)
  → atomic_umin → 0.1 (保留前景，边界清晰)
```

---

### 11.2 问题二：独特的色散效果实现

**核心发现：色散效果在渲染合成阶段实现，而非流体模拟阶段**

#### 11.2.1 分析颜色分离的实现位置

从 DXBC 分析，色散效果不是在流体模拟阶段，而是在最终合成阶段。

查看 Event 13536/13550 的像素着色器：

```hlsl
// Event 13536/13550 PS 伪代码还原
float4 main(float2 position : SV_Position) : SV_Target
{
    // 1. 计算屏幕 UV
    float2 uv = position * g_vInvOutputRes;
    float2 ndc = uv * float2(2, -2) + float2(-1, 1);
    
    // 2. 读取流体深度
    int2 coord = int2(uv * textureSize);
    uint packedDepth = g_tDepthTexture[coord];
    float depth = packedDepth * 0.0;  // 实际代码中有特殊处理
    
    // 3. 重投影
    float4 prevClipPos = mul(g_mClipToPreviousClipNoJitter, float4(ndc, depth, 1));
    float2 prevUV = prevClipPos.xy / prevClipPos.w * float2(0.5, -0.5) + 0.5;
    
    // 4. 采样输入纹理
    float4 color = g_tInputTexture.Sample(g_sLinearClamp, prevUV);
    
    return color;
}
```

#### 11.2.2 色散效果的实现原理

色散效果实际上是通过 **速度场驱动的时间重投影** 实现的：

```hlsl
// 真正的色散效果实现 (推测，基于 Velocity Generation)
// Event 13688 生成速度场，后续渲染使用不同通道的速度偏移

// 速度场公式分析:
// velocity.x = 屏幕运动 + 径向力
// velocity.y = 另一个方向的力

// 色散可能通过以下方式实现:

// 方法1: RGB 通道分离偏移
float2 velocity = g_tFluidVelocity.Sample(uv);
float r = g_tFluid.Sample(uv - velocity * 1.0 * dt);  // 红色通道
float g = g_tFluid.Sample(uv - velocity * 0.5 * dt);  // 绿色通道
float b = g_tFluid.Sample(uv - velocity * 0.0 * dt);  // 蓝色通道

// 方法2: 基于深度的色散
float depth = g_tDepthTexture[coord];
float dispersionStrength = depth * g_fHissIntensity;
float2 rOffset = velocity * dispersionStrength * 1.0;
float2 gOffset = velocity * dispersionStrength * 0.5;
float2 bOffset = velocity * dispersionStrength * 0.0;
```

#### 11.2.3 Presence 多层颜色系统

从 gfxgraph_p7 常量缓冲区分析：

```
色散效果的真正来源: 4 层 Presence 效果叠加

g_vPresenceLayer1Color  - 第1层颜色 (最内层，高亮核心)
g_vPresenceLayer2Color  - 第2层颜色 (中间层，主要烟雾)
g_vPresenceLayer3Color  - 第3层颜色 (外层，扩散边缘)
g_vPresenceLayer4Color  - 第4层颜色 (最外层，环境融合)

每层参数:
g_vPresenceLayer1Params = float4(startDistance, endDistance, intensity, falloff)
```

```hlsl
// 多层颜色混合伪代码
float3 compute_hiss_color(float presenceFactor)
{
    float3 color = float3(0, 0, 0);
    
    // Layer 1: 内核 (最亮)
    float mask1 = smoothstep(Layer1Params.x, Layer1Params.y, presenceFactor);
    mask1 = pow(mask1 * Layer1Params.z, Layer1Params.w);
    color += mask1 * g_vPresenceLayer1Color;
    
    // Layer 2: 主体
    float mask2 = smoothstep(Layer2Params.x, Layer2Params.y, presenceFactor);
    mask2 = pow(mask2 * Layer2Params.z, Layer2Params.w);
    color += mask2 * g_vPresenceLayer2Color;
    
    // Layer 3: 边缘
    float mask3 = smoothstep(Layer3Params.x, Layer3Params.y, presenceFactor);
    mask3 = pow(mask3 * Layer3Params.z, Layer3Params.w);
    color += mask3 * g_vPresenceLayer3Color;
    
    // Layer 4: 外围
    float mask4 = smoothstep(Layer4Params.x, Layer4Params.y, presenceFactor);
    mask4 = pow(mask4 * Layer4Params.z, Layer4Params.w);
    color += mask4 * g_vPresenceLayer4Color;
    
    return color;
}
```

#### 11.2.4 独特的色散视觉效果

```
Control Hiss 的色散特点:

1. 基于距离的多层颜色:
   ┌─────────────────────────────────────┐
   │  Layer 4 (外层): 深红/黑色          │
   │   ┌─────────────────────────────┐   │
   │   │  Layer 3: 红色               │   │
   │   │   ┌─────────────────────┐   │   │
   │   │   │  Layer 2: 橙红色     │   │   │
   │   │   │   ┌─────────────┐   │   │   │
   │   │   │   │ Layer 1: 白  │   │   │   │
   │   │   │   │   核心高亮   │   │   │   │
   │   │   │   └─────────────┘   │   │   │
   │   │   └─────────────────────┘   │   │
   │   └─────────────────────────────┘   │
   └─────────────────────────────────────┘

2. 速度场驱动的扭曲:
   - 不同颜色层以不同速度流动
   - 产生类似色散的视觉分离效果

3. 非传统的色散:
   - 不是基于折射率的色散
   - 而是基于多层独立运动的烟雾层
   - 每层有自己的颜色、速度和衰减
```

---

### 11.3 问题三：深度遮挡和耗散机制

**核心发现：通过场景深度比较 + 蓝噪声抖动 + 时间衰减实现**

#### 11.3.1 深度遮挡实现

从 CS_HissEffect (Event 13514) DXBC 分析：

```hlsl
// 深度遮挡核心算法
[numthreads(8, 8, 1)]
void CS_HissEffect(uint3 DTid : SV_DispatchThreadID)
{
    // 1. 读取流体深度和邻居深度
    uint depthCenter = g_tDepthTexture[coord];
    uint depthLeft   = g_tDepthTexture[coord + int2(-1, 0)];
    uint depthRight  = g_tDepthTexture[coord + int2(1, 0)];
    uint depthUp     = g_tDepthTexture[coord + int2(0, -1)];
    uint depthDown   = g_tDepthTexture[coord + int2(0, 1)];
    
    // 2. 深度扩散 (取最大有效深度)
    // DXBC 指令 29-35:
    //   lt r0.z, l(4294537728.000000), r0.z  // 检查是否 < 0xFFF00000
    //   and r0.z, r0.z, r2.z                  // 与邻居比较
    //   max r2.x, r2.y, r3.y                  // 取最大值
    
    bool validCenter = depthCenter < 0xFFF00000;
    bool validLeft   = depthLeft   < 0xFFF00000;
    bool validRight  = depthRight  < 0xFFF00000;
    bool validUp     = depthUp     < 0xFFF00000;
    bool validDown   = depthDown   < 0xFFF00000;
    
    float maxDepth = 0;
    if (validCenter && validLeft && validRight && validUp && validDown)
    {
        maxDepth = max(depthCenter, max(depthLeft, max(depthRight, max(depthUp, depthDown))));
    }
    else
    {
        maxDepth = depthCenter;
    }
    
    // 3. 读取场景深度并转换
    // DXBC 指令 76-85:
    float sceneDepthSample = g_tClipDepth.SampleLevel(g_sNearestClamp, uv, 0);
    
    // 逆投影: depth → viewZ
    // viewZ = -g_mViewToClip[3].z / (depth * (g_mViewToClip[2].z - 1) + g_mViewToClip[3].z)
    float A = g_mViewToClip[2].z;
    float B = g_mViewToClip[3].z;
    float sceneViewZ = -B / (sceneDepthSample * (A - 1) + B);
    
    // 4. 计算流体 viewZ
    float fluidViewZ = maxDepth * viewZScale;
    
    // 5. 深度差计算
    // DXBC 指令 86-96:
    float depthDiff = fluidViewZ - sceneViewZ;
    
    // 6. 深度混合 (关键！)
    // DXBC 指令 87-96:
    float blendRate = g_fWorldTimeDelta * g_fHissBlendTowardsSceneDepth;
    
    // 如果流体深度接近场景深度，向场景深度混合
    if (depthDiff < 0.1 && depthDiff > -0.5)
    {
        fluidViewZ = lerp(fluidViewZ, sceneViewZ, blendRate);
    }
    
    // 如果流体在场景后面，标记为无效
    if (depthDiff > 5.0)
    {
        fluidViewZ = -1;  // 无效标记
    }
    
    // 7. 检测输入纹理是否为空 (优化)
    float4 inputColor = g_tInputTexture.SampleLevel(g_sLinearClamp, uv, 0);
    if (all(inputColor == 0) && depthDiff > 0.5)
    {
        outputDepth = -1;  // 跳过空白区域
    }
    else
    {
        outputDepth = fluidViewZ * 4294967296.0;
    }
    
    // 8. 写入结果
    g_rwtDepthTexture[coord] = outputDepth;
}
```

#### 11.3.2 深度遮挡的关键技术点

| 技术点 | DXBC 指令 | 说明 |
|--------|-----------|------|
| **无效标记** | `lt r0.w, l(4294537728), r0.z` | `0xFFF00000` 作为无效标记 |
| **深度扩散** | `max r2.x, r2.y, r3.y` | 取邻居最大深度，防止漏空 |
| **逆投影** | `div r2.y, -g_mViewToClip[3].z, g_mViewToClip[2].z` | 将深度转换为视图空间 |
| **深度混合** | `mad r3.x, r3.x, r2.y, r0.z` | 向场景深度渐变混合 |
| **空白检测** | `eq r0.w, r0.w, l(0)` | 跳过没有内容的区域 |

#### 11.3.3 耗散机制

**多级耗散系统：**

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                           Hiss 效果耗散机制                                  │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  Level 1: 几何速度耗散 (Event 13688)                                        │
│  ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━                                           │
│  速度场由场景几何运动产生:                                                   │
│    geometryVelocity = g_tGeometryVelocityTexture.Sample(uv)                │
│    motionVector = prevUV - currentUV                                        │
│    screenVelocity = motionVector * g_vOutputRes * 5.0                      │
│                                                                             │
│  当几何停止运动时，这部分速度自然消失                                        │
│                                                                             │
│  Level 2: 力场耗散 (Event 13688)                                            │
│  ━━━━━━━━━━━━━━━━━━━━━━━━━━━                                                │
│  径向力和向上力是常量:                                                       │
│    radialForce = (uv - 0.5) * g_fHissRadialForce                           │
│    upwardForce = float2(0, -g_fHissUpwardsForce * 0.5)                     │
│                                                                             │
│  这些力不会耗散，而是通过深度比较间接消除                                    │
│                                                                             │
│  Level 3: 深度耗散 (Event 13514)                                            │
│  ━━━━━━━━━━━━━━━━━━━━━━━━━━━                                                │
│  通过场景深度混合实现:                                                       │
│    blendRate = deltaTime * g_fHissBlendTowardsSceneDepth                   │
│    fluidViewZ = lerp(fluidViewZ, sceneViewZ, blendRate)                    │
│                                                                             │
│  当流体深度接近场景深度时，逐渐"融入"场景                                    │
│                                                                             │
│  Level 4: 密度耗散 (Event 13497)                                            │
│  ━━━━━━━━━━━━━━━━━━━━━━━━━━━                                                │
│  通过无效标记和边界检测:                                                     │
│    if (depth > 0xFFF00000) discard;                                        │
│    if (depthDiff > 5.0) outputDepth = -1;                                  │
│                                                                             │
│  超出有效范围的区域被标记为无效                                              │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

#### 11.3.4 蓝噪声抖动的作用

蓝噪声在 CS_HissEffect 中有两个关键作用：

```hlsl
// DXBC 指令 54-63 分析:
int2 noiseCoord = coord & 255;  // 取模 256 (蓝噪声纹理大小)
float4 blueNoise = g_tStaticBlueNoiseRGBA_0[noiseCoord];

// 时间抖动 (每帧不同)
uint frame = g_uCurrentFrame & 63;  // 64 帧循环
float2 goldenRatioOffset = float2(0.754878, 0.569840);  // 黄金比例相关
float2 jitter = frac(blueNoise.xy + (frame + 1) * goldenRatioOffset);

// 应用抖动采样
int2 jitteredCoord = coord + int2(jitter * 2 - 1);
float jitteredDepth = g_tDepthTexture[jitteredCoord];

// 作用1: 消除条带 (Banding)
// 低分辨率流体渲染时容易产生条带，蓝噪声将其打碎为高频噪声

// 作用2: 时间稳定性
// 通过帧计数循环抖动，避免固定模式的视觉伪影

// 作用3: 边缘柔化
// 抖动采样在保持边缘清晰的同时，添加细微的边缘变化
```

#### 11.3.5 完整的深度-耗散流程图

```
帧 N:
┌─────────────────────────────────────────────────────────────────┐
│  输入:                                                          │
│  - g_tDepthTexture (N-1帧流体深度, R32_UINT)                   │
│  - g_tFluidVelocity (N-1帧速度场, R16G16F)                     │
│  - g_tClipDepth (当前帧场景深度, R32)                           │
│  - g_tGeometryVelocityTexture (几何速度, R16G16F)              │
│  - g_tStaticBlueNoiseRGBA_0 (蓝噪声)                           │
└─────────────────────────────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│  Step 1: CS_FluidSimulation (Event 13497)                       │
│  ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━                          │
│  1. 读取 N-1 帧深度和速度                                       │
│  2. 4点重投影 (整数坐标 + atomic_umin)                         │
│  3. 边界清晰保持                                               │
│  输出: g_rwtDepthTexture (更新后的深度)                        │
└─────────────────────────────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│  Step 2: CS_HissEffect (Event 13514)                            │
│  ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━                                │
│  1. 深度扩散 (邻居最大值)                                       │
│  2. 蓝噪声抖动采样                                             │
│  3. 场景深度比较和混合                                         │
│  4. 无效区域标记                                               │
│  输出: g_rwtDepthTexture (融合后的深度)                        │
└─────────────────────────────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│  Step 3: PS_VelocityGeneration (Event 13688)                    │
│  ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━                        │
│  1. 采样场景深度和几何速度                                     │
│  2. 计算屏幕空间运动向量                                       │
│  3. 添加径向力和向上力                                         │
│  输出: g_tFluidVelocity (N 帧速度场)                           │
└─────────────────────────────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│  Step 4: PS_FluidAdvection (Event 13668)                        │
│  ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━                        │
│  1. 采样速度场                                                 │
│  2. Semi-Lagrangian 平流                                       │
│  3. 双线性采样 (这里允许模糊，因为是在流体内部)                │
│  输出: 流体数据                                                │
└─────────────────────────────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│  Step 5: 散度计算和 Jacobi 迭代 (Event 13734-13795)             │
│  ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━         │
│  1. 计算速度场散度                                             │
│  2. Jacobi 迭代求解压力场                                      │
│  3. 投影修正速度场                                             │
│  输出: 无散度的速度场                                          │
└─────────────────────────────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│  Step 6: 最终合成 (Event 13536/13550 + 角色渲染)               │
│  ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━             │
│  1. 深度可视化                                                 │
│  2. 多层 Presence 颜色叠加                                     │
│  3. 角色渲染 (深度测试 discard)                               │
│  输出: 最终画面                                                │
└─────────────────────────────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│  输出 (作为 N+1 帧的输入):                                      │
│  - g_tDepthTexture (流体深度)                                  │
│  - g_tFluidVelocity (速度场)                                   │
└─────────────────────────────────────────────────────────────────┘
```

---

## 12. 在 UE/Niagara 中实现的完整方案

### 12.1 重投影边界清晰方案

```hlsl
// Niagara 自定义模块: ReprojectionWithAtomicMin

// 输入
Texture2D<uint> PreviousDepth;  // R32_UINT
float4x4 PreviousClipToClipNoJitter;
float2 Velocity;

// 处理
int2 coord = DispatchThreadId.xy;
uint packedDepth = PreviousDepth[coord];
if (packedDepth > 0xFFF00000) return;  // 无效标记

float depth = unpack_depth(packedDepth);

// 4 点采样
for (int i = 0; i < 4; i++)
{
    float2 offset = offsets[i];
    float2 ndc = (coord + 0.5 + offset) / TextureSize * 2.0 - 1.0;
    
    float4 prevClip = mul(PreviousClipToClipNoJitter, float4(ndc, depth, 1));
    float2 prevUV = prevClip.xy / prevClip.w * 0.5 + 0.5;
    
    // 关键：整数坐标
    int2 intCoord = int2(prevUV * TextureSize);
    
    if (all(intCoord >= 0) && all(intCoord < TextureSize))
    {
        // 关键：原子操作
        InterlockedMin(OutputDepth[intCoord], packedDepth);
    }
}
```

### 12.2 多层颜色色散方案

```hlsl
// Niagara 自定义模块: MultiLayerColorDispersion

// 参数
float4 Layer1Params;  // (start, end, intensity, falloff)
float4 Layer2Params;
float4 Layer3Params;
float4 Layer4Params;
float3 Layer1Color;
float3 Layer2Color;
float3 Layer3Color;
float3 Layer4Color;

// 处理
float presenceFactor = compute_presence_factor(PresencePosition, WorldPos);

float3 color = 0;

// 每层独立计算
float mask1 = smoothstep(Layer1Params.x, Layer1Params.y, presenceFactor);
mask1 = pow(mask1 * Layer1Params.z, Layer1Params.w);
color += mask1 * Layer1Color;

float mask2 = smoothstep(Layer2Params.x, Layer2Params.y, presenceFactor);
mask2 = pow(mask2 * Layer2Params.z, Layer2Params.w);
color += mask2 * Layer2Color;

// ... Layer3, Layer4 类似

OutputColor = color * HissIntensity;
```

### 12.3 深度遮挡和耗散方案

```hlsl
// Niagara 自定义模块: DepthOcclusionAndDissipation

// 输入
Texture2D<float> SceneDepthTexture;
float2 UV;
float CurrentDepth;
float HissBlendTowardsSceneDepth;
float DeltaTime;

// 处理
float sceneDepth = SceneDepthTexture.SampleLevel(UV, 0).r;
float sceneViewZ = depth_to_viewZ(sceneDepth);

float fluidViewZ = CurrentDepth * ViewZScale;
float depthDiff = fluidViewZ - sceneViewZ;

// 深度混合
float blendRate = DeltaTime * HissBlendTowardsSceneDepth;

if (depthDiff < 0.1 && depthDiff > -0.5)
{
    // 靠近场景表面，向场景深度混合
    fluidViewZ = lerp(fluidViewZ, sceneViewZ, blendRate);
}

// 无效标记
if (depthDiff > 5.0 || fluidViewZ < 0)
{
    OutputDepth = 0xFFF00000;  // 无效标记
}
else
{
    OutputDepth = pack_depth(fluidViewZ);
}
```

---

## 13. 新发现阶段详细分析 (Event 13810-13895)

### 13.1 Event 13810: PS_PressureProjection (压力投影修正)

**功能**: 根据压力场梯度修正速度场，实现流体不可压缩性

**DXBC 反汇编关键部分**:
```
ps_5_0
   // 读取四个邻居压力值
   0: add r0.xyzw, v0.xyzz, l(-1, 0, 0, 0)       // 左邻居坐标
   2: ld_indexable g_tFluidPressure.xyzw         // P(x-1, y)
   5: ld_indexable g_tFluidPressure.yxzw         // P(x+1, y)
   9: ld_indexable g_tFluidPressure.yzxw         // P(x, y+1)
  12: ld_indexable g_tFluidPressure.yzwx         // P(x, y-1)

   // 计算压力梯度
   6: add r0.x, -r0.y, r0.x       // ∂p/∂x = P(x-1) - P(x+1)
  13: add r0.y, -r0.z, r0.w       // ∂p/∂y = P(y-1) - P(y+1)

   // 应用修正
  14: mul r0.xy, r0.xyxx, g_fWorldTimeDelta.xxxx  // 乘以时间步长
  16: ld_indexable g_tFluidVelocity.zwxy          // 读取当前速度
  17: mad r0.xy, -r0.xyxx, l(15, 15, 0, 0), r0.zwzz  // v_new = v - ∇p * 15 * dt
  18: max r0.xy, r0.xyxx, l(-2000, -2000, 0, 0)   // 钳制范围
  19: min o0.xy, r0.xyxx, l(2000, 2000, 0, 0)     // 输出修正后的速度
```

**算法还原**:
```hlsl
// 压力投影修正
float4 main(float4 position : SV_Position) : SV_Target
{
    int2 coord = int2(position.xy);
    
    // 读取四个邻居压力值
    float pLeft   = g_tFluidPressure[coord + int2(-1, 0)];
    float pRight  = g_tFluidPressure[coord + int2(1, 0)];
    float pUp     = g_tFluidPressure[coord + int2(0, 1)];
    float pDown   = g_tFluidPressure[coord + int2(0, -1)];
    
    // 计算压力梯度 (中心差分)
    float gradX = pLeft - pRight;   // ∂p/∂x
    float gradY = pDown - pUp;      // ∂p/∂y
    
    // 读取当前速度
    float2 velocity = g_tFluidVelocity[coord];
    
    // 修正速度: v_new = v - ∇p * dt * 15
    // 15 是压力系数，控制不可压缩性的强度
    float2 newVelocity = velocity - float2(gradX, gradY) * g_fWorldTimeDelta * 15.0;
    
    // 钳制速度范围
    newVelocity = clamp(newVelocity, float2(-2000, -2000), float2(2000, 2000));
    
    return float4(newVelocity, 0, 0);
}
```

**关键参数**:
| 参数 | 值 | 说明 |
|------|-----|------|
| 压力系数 | 15.0 | 控制不可压缩性强度 |
| 速度钳制 | [-2000, 2000] | 防止数值爆炸 |

---

### 13.2 Event 13827: PS_FluidAdvectionGaussian (高斯权重平流)

**功能**: 带高斯权重的多采样平流，实现更平滑的流体传输

**DXBC 反汇编关键部分**:
```
ps_5_0
   // 边界检查
   0: mul r0.xy, v0.xyxx, g_vInvOutputRes.xyxx
   1-5: ge/or r0.z, ... // 检查是否在边界外
   
   // 如果在边界外，返回0
   6-9: if_nz r0.z { mov o0, 0; ret; }
   
   // 采样速度场
  10: sample_indexable g_tFluidVelocity.xyzw, g_sLinearClamp
  11: mul r1.xyzw, r0.xyxy, g_fWorldTimeDelta.xxxx  // velocity * dt
  
   // 计算回溯位置
  12: mad r0.xy, -r0.xyxx, g_fWorldTimeDelta.xxxx, v0.xyxx  // backPos = pos - vel * dt
  
   // 高斯权重系数
  14: mul r1.xyzw, r1.xyzw, l(0.005, 0.005, 0.015, 0.015)
  15: mad r0.xy, r0.xyxx, g_vInvOutputRes.xyxx, -r1.zwzz
  
   // 7次采样循环
  19: loop
  20:   ige r0.w, r0.z, l(7)    // i < 7
  21:   breakc_nz r0.w
  
        // 计算B样条权重
  22-29: mul r2.y, r1.z, l(46.666599)  // 高斯核权重
        
        // 采样流体数据
  32:   sample_l g_tFluid.xyzw, g_sLinearClamp, l(0)
  
        // 累加
  33:   mad r3.xyzw, r4.xyzw, r2.xyzw, r3.xyzw
  34:   iadd r0.z, r0.z, l(1)
  35: endloop
  
   // 归一化输出
  36: mul o0.xyzw, r3.xyzw, l(0.538462, 0.428168, 0.350000, 0.142857)
```

**算法还原**:
```hlsl
// 高斯权重平流
float4 main(float4 position : SV_Position) : SV_Target
{
    float2 uv = position.xy * g_vInvOutputRes;
    
    // 边界检查
    if (uv.x < 0.001 || uv.x > 0.999 || uv.y < 0.001 || uv.y > 0.999)
        return float4(0, 0, 0, 0);
    
    // 采样速度场
    float2 velocity = g_tFluidVelocity.Sample(g_sLinearClamp, uv);
    
    // 计算回溯位置
    float2 backUV = uv - velocity * g_fWorldTimeDelta * g_vInvOutputRes;
    
    // 高斯偏移
    float2 offset = velocity * g_fWorldTimeDelta * float2(0.005, 0.015);
    backUV -= offset;
    
    // 7点高斯采样
    float4 result = 0;
    for (int i = 0; i < 7; i++)
    {
        float t = i / 7.0;
        float x = t * t;
        float w = x * (1 - t);
        w = w * w * 46.6666;  // 高斯权重
        
        float2 sampleUV = backUV + offset * i;
        float4 sample = g_tFluid.SampleLevel(g_sLinearClamp, sampleUV, 0);
        result += sample * w;
    }
    
    // 归一化
    return result * float4(0.538462, 0.428168, 0.350, 0.142857);
}
```

**关键发现**:
| 技术 | 说明 |
|------|------|
| 7次采样 | 使用B样条核进行平滑采样 |
| 高斯权重 | 46.6666 权重系数 |
| 归一化 | 特殊的归一化系数 (0.538, 0.428, 0.35, 0.143) |

---

### 13.3 Event 13846: PS_HissMaskApply (Hiss Mask 应用)

**功能**: 应用 Hiss 纹理掩码，实现边缘柔化和场景融合

**DXBC 反汇编关键部分**:
```
ps_5_0
   // 读取输入纹理
   0: mul r0.xy, v0.xyxx, g_vInvOutputRes.xyxx
   1-2: ld_indexable g_tInputTexture.xyzw       // 当前像素
   3-14: ld_indexable g_tInputTexture.xyzw      // 4个邻居 (Sobel算子)
   
   // 采样场景源
  15: sample_indexable g_tFinalImageSource.xyzw, g_sLinearClamp
  
   // Sobel边缘检测
  16-18: add r2.xyz, r2.xyzx, -r3.xyzx          // 水平差
         add r3.xyz, -r4.xyzx, r5.xyzx          // 垂直差
         add r2.xyz, r2.xyzx, r3.xyzx           // 总梯度
  
   // Alpha权重
  19: mul r0.w, r1.w, r1.w                      // alpha²
  20: mul r0.w, r0.w, l(0.25)                   // * 0.25
  21: mul r2.xyz, r0.wwww, r2.xyzx              // 梯度 * alpha²
  22: mul r2.xyz, r2.xyzx, g_fWorldTimeDelta.xxxx
  
   // 应用梯度位移
  23: mad r1.xyz, r2.xyzx, l(30, 30, 30, 0), r1.xyzx
  
   // 采样 Hiss Mask
  24-26: sample_indexable g_tHissMask.yzwx, g_sLinearWrap
         // UV动画: mad v0 * 0.1, offset
  
   // Mask 阈值处理
  27: add r0.w, r0.w, l(-0.3)                   // mask - 0.3
  28: mul_sat r0.w, r0.w, l(4)                  // saturate((mask-0.3)*4)
  29-31: smoothstep 计算
  
   // 与场景混合
  40-44: mad r1.xyz, r0.wwww, r2.xzwx, r1.xyzx  // 混合
         add r0.xyz, r0.xyzx, -r1.xyzx
         mad r0.xyz, r2.yyyy, r0.xyzx, r1.xyzx
```

**算法还原**:
```hlsl
// Hiss Mask 应用
float4 main(float4 position : SV_Position) : SV_Target
{
    float2 uv = position.xy * g_vInvOutputRes;
    int2 coord = int2(position.xy);
    
    // 读取当前像素和4个邻居
    float4 center = g_tInputTexture[coord];
    float3 left   = g_tInputTexture[coord + int2(-1, 0)];
    float3 right  = g_tInputTexture[coord + int2(1, 0)];
    float3 up     = g_tInputTexture[coord + int2(0, 1)];
    float3 down   = g_tInputTexture[coord + int2(0, -1)];
    
    // 采样场景
    float3 sceneColor = g_tFinalImageSource.Sample(g_sLinearClamp, uv);
    
    // Sobel梯度
    float3 gradient = (left - right) + (down - up);
    
    // Alpha加权梯度位移
    float alphaWeight = center.a * center.a * 0.25;
    float3 displacement = gradient * alphaWeight * g_fWorldTimeDelta * 30.0;
    float3 fluidColor = center.rgb + displacement;
    
    // 采样 Hiss Mask (带UV动画)
    float2 maskUV = uv * 0.1 + float2(g_fWorldTime * 0.1, g_fWorldTime * 15.0);
    float mask = g_tHissMask.Sample(g_sLinearWrap, maskUV);
    
    // 阈值处理 (smoothstep)
    mask = saturate((mask - 0.3) * 4.0);
    mask = mask * mask * (3 - 2 * mask);  // smoothstep(0.392, 1.0, mask)
    
    // 与场景混合
    float blendFactor = mask * g_fWorldTimeDelta * 0.3;
    float3 result = lerp(fluidColor, sceneColor, blendFactor);
    
    // 有效性检查 (NaN检测)
    if (any(isinf(result)))
        return float4(0, 0, 0, 0);
    
    return float4(result, center.a);
}
```

**关键发现**:
| 技术 | 说明 |
|------|------|
| Sobel边缘检测 | 使用4邻居计算梯度 |
| Alpha加权 | center.a² * 0.25 控制位移强度 |
| Mask阈值 | 0.3阈值 + smoothstep |
| UV动画 | time * 0.1 / time * 15 两轴不同速度 |

#### 13.3.1 颜色梯度驱动的色散效果

**这是色散效果的核心实现！**

从DXBC指令分析，Event 13846通过对颜色场计算Sobel梯度，实现了独特的RGB通道分离效果：

**完整DXBC分析（行3-23）：**

```
// 步骤1: 采样颜色场中心像素
   2: ld_indexable g_tInputTexture.xyzw, r1.xyzw, g_tInputTexture.xyzw
      // r1.xyzw = 中心像素颜色 (R, G, B, A)

// 步骤2: 采样颜色场左右邻居
   3-5: ld_indexable g_tInputTexture.xyz, r2.xyzw (coord + (-1, 0))  // 左侧
   6-8: ld_indexable g_tInputTexture.xyz, r3.xyzw (coord + (1, 0))   // 右侧

// 步骤3: 采样颜色场上下邻居
   9-11: ld_indexable g_tInputTexture.xyz, r4.xyzw (coord + (0, 1))  // 上方
  12-14: ld_indexable g_tInputTexture.xyz, r5.xyzw (coord + (0, -1)) // 下方

// 步骤4: 计算颜色梯度 (Sobel算子)
  16: add r2.xyz, r2.xyzx, -r3.xyzx  // ∇Color_x = 左 - 右
  17: add r3.xyz, -r4.xyzx, r5.xyzx   // ∇Color_y = 下 - 上
  18: add r2.xyz, r2.xyzx, r3.xyzx    // ∇Color = ∇x + ∇y

// 步骤5: Alpha加权梯度位移
  19: mul r0.w, r1.w, r1.w             // alpha²
  20: mul r0.w, r0.w, l(0.25)          // 0.25 * alpha²
  21: mul r2.xyz, r0.wwww, r2.xyzx     // 梯度 * 0.25 * alpha²
  22: mul r2.xyz, r2.xyzx, g_fWorldTimeDelta.xxxx  // 梯度 * dt

// 步骤6: 应用梯度位移（产生RGB分离！）
  23: mad r1.xyz, r2.xyzx, l(30, 30, 30, 0), r1.xyzx
      // finalColor = color + gradient * 30 * dt
```

**色散效果机制详解：**

```hlsl
// Event 13846 的色散算法核心

// 1. 颜色场梯度计算
float3 colorCenter = g_tInputTexture[coord];      // 中心颜色
float3 colorLeft   = g_tInputTexture[coord + int2(-1, 0)];
float3 colorRight  = g_tInputTexture[coord + int2(1, 0)];
float3 colorUp     = g_tInputTexture[coord + int2(0, 1)];
float3 colorDown   = g_tInputTexture[coord + int2(0, -1)];

// Sobel梯度（对每个颜色通道分别计算！）
float3 gradientX = colorLeft - colorRight;   // 每个通道的X方向梯度
float3 gradientY = colorDown - colorUp;      // 每个通道的Y方向梯度
float3 colorGradient = gradientX + gradientY; // 合并梯度

// 2. Alpha加权（关键！）
float alphaWeight = colorCenter.a * colorCenter.a * 0.25;

// 3. 颜色位移（RGB通道独立位移）
float3 colorDisplacement = colorGradient * alphaWeight * g_fWorldTimeDelta * 30.0;
float3 finalColor = colorCenter.rgb + colorDisplacement;

// 结果：RGB三个通道根据各自的梯度向不同方向偏移！
// R通道: R_gradient 决定R的位移方向和强度
// G通道: G_gradient 决定G的位移方向和强度  
// B通道: B_gradient 决定B的位移方向和强度
```

**为什么这会产生色散效果？**

```
传统色散（折射）:
  - 基于不同波长的光有不同的折射率
  - 物理公式: n(λ) = n₀ + k/λ²

Hiss流体色散（颜色梯度）:
  - 基于不同颜色通道有不同的空间梯度
  - 计算公式: displacement_RGB = ∇Color_RGB * weight
  
色散强度对比:
  ┌─────────────────────────────────────────┐
  │ 传统色散: 不同波长 → 不同折射角         │
  │ Hiss色散: 不同颜色 → 不同位移向量       │
  │                                         │
  │ 效果相似: RGB通道空间分离              │
  └─────────────────────────────────────────┘
```

**与速度场平流的区别：**

| 特性 | 速度场平流 (Event 13668) | 颜色梯度位移 (Event 13846) |
|------|--------------------------|---------------------------|
| **输入** | 速度场 Velocity | 颜色场 Color |
| **计算** | ∇Velocity (速度梯度) | ∇Color (颜色梯度) |
| **效果** | 颜色跟随速度流动 | RGB通道独立位移 |
| **色散** | 无色散（整体流动） | **产生色散**（通道分离） |
| **物理意义** | 流体对流 | 颜色扩散/色散 |

**完整色散管线：**

```
Event 13536/13550: 生成多层颜色
    ↓
    输出: 不同层次的颜色混合
    
Event 13846: 颜色梯度色散 (本步骤!)
    ↓
    计算: ∇Color_RGB (每个通道独立梯度)
    应用: finalColor = color + ∇Color * 30 * dt
    效果: RGB通道向不同方向位移，产生色散
    
Event 13870: 最终合成
    ↓
    结合: Alpha归一化 + 场景混合
    结果: 边缘清晰的色散效果
```

**数学原理：**

```
颜色梯度场定义:
  ∇C(x,y) = (∂C/∂x, ∂C/∂y)

对于RGB三个通道:
  ∇R = (∂R/∂x, ∂R/∂y)  // 红色通道梯度
  ∇G = (∂G/∂x, ∂G/∂y)  // 绿色通道梯度
  ∇B = (∂B/∂x, ∂B/∂y)  // 蓝色通道梯度

色散位移:
  R' = R + (∂R/∂x + ∂R/∂y) * weight
  G' = G + (∂G/∂x + ∂G/∂y) * weight
  B' = B + (∂B/∂x + ∂B/∂y) * weight

当 RGB 梯度不同时 → RGB 位移不同 → 视觉上的色散效果！
```

---

### 13.4 Event 13870: PS_HissComposite (最终合成)

**功能**: 将 Hiss 效果与场景最终合成，处理深度检测和多采样抖动

**DXBC 反汇编关键部分**:
```
ps_5_0
   // 蓝噪声抖动 (双重采样)
   0: bfi r0.x, l(5), l(0), g_uCurrentFrame.x, l(32)  // 帧索引编码
   1-2: iadd/utof ...                                  // 时间偏移
   
   // 第一次蓝噪声采样
   3-6: and r1.xy, r0.yzyy, l(255, 255, 0, 0)         // 取模256
        ld_indexable g_tStaticBlueNoiseRGBA_0.zxyw    // 蓝噪声采样
   
   // Golden Ratio抖动
   7-8: mad r0.xw, r0.xxxx, l(0.754878, 0.569840)     // 黄金比例偏移
        frc r0.xw, r0.xxxw
   
   // 计算抖动偏移 (8x8范围, -4到+4)
   9: mad r0.xw, r0.xxxw, l(8, 8, 0, 0), l(-4, -4, 0, 0)
   
   // 读取流体深度
  10-18: resinfo g_tDepthTexture        // 获取纹理大小
         ftoi r2.xy, r0.xwxx            // 抖动后的整数坐标
         ld_indexable g_tDepthTexture   // 读取深度
         mul r0.x, r0.x, l(0)           // 深度处理 (特殊)
   
   // 深度逆投影
  19-25: div/add/mad ...                // viewZ计算
         // viewZ = -B / (depth * (A-1) + B)
   
   // 采样场景深度
  26: sample_indexable g_tClipDepth.yzxw, g_sNearestClamp
   
   // 深度差计算和Z裁剪
  27-33: min/add/div ...                // 场景深度处理
         rcp g_fHissZClipStart          // Z裁剪起始距离
         mul_sat r0.x, r0.x, r2.w       // 深度因子
   
   // 第二次蓝噪声采样 (用于近处混合)
  34-53: and g_uCurrentFrame.x, l(31)  // 32帧循环
         iadd/utof/mad/frc ...          // 新的抖动偏移
         mad r0.yz, l(4, 4, 0, 0), l(-2, -2, 0, 0)
         ld_indexable g_tDepthTexture   // 第二次深度采样
         // 计算近处深度因子
         add_sat r0.y, r0.y, l(1)       // 近处因子
         add_sat r0.y, r0.y, -g_fHissNearClipStart
   
   // 混合两个深度因子
  52-54: add r0.x, r0.x, r0.z
         mul r0.x, r0.x, l(0.5)         // 平均
         mul r0.x, r0.y, r0.x           // 应用近处因子
   
   // 采样输入纹理和最终源
  55-56: sample_indexable g_tInputTexture.xyzw, g_sLinearClamp
         sample_indexable g_tFinalImageSource.wxyz, g_sLinearClamp
   
   // Alpha混合
  57-61: rsq r1.x, r2.w                 // 1/sqrt(alpha)
         mov_sat r2.xyz, r2.xyzx        // 钳制颜色
         add r1.yzw, -r0.yyzw, r2.xxyz  // 差值
         div r1.x, l(1), r1.x           // 归一化因子
         mul_sat r0.x, r0.x, r1.x       // 最终混合因子
   
   // 最终合成
  62: mad o0.xyz, r0.xxxx, r1.yzwy, r0.yzwy  // color = lerp(scene, fluid, factor)
  63: mov o0.w, l(1)
```

**算法还原**:
```hlsl
// Hiss 最终合成
float4 main(float4 position : SV_Position) : SV_Target
{
    // 蓝噪声抖动 (Golden Ratio)
    uint frame = g_uCurrentFrame;
    float timeOffset = (bfi(5, 0, frame, 32) + 1);
    
    int2 pixelCoord = int2(position.xy);
    int2 noiseCoord = pixelCoord & 255;  // 取模256
    
    float2 blueNoise = g_tStaticBlueNoiseRGBA_0[noiseCoord].xz;
    float2 jitter1 = frac(blueNoise + timeOffset * float2(0.754878, 0.569840));
    jitter1 = jitter1 * 8.0 - 4.0;  // [-4, 4] 范围
    
    // 第一次深度采样 (远距离)
    int2 depthCoord1 = int2((position.xy + jitter1) * g_tDepthTextureSize / g_vOutputRes);
    uint packedDepth1 = g_tDepthTexture[depthCoord1];
    float depth1 = unpack_depth(packedDepth1);
    
    // 深度逆投影: depth → viewZ
    float A = g_mViewToClip[2].z;
    float B = g_mViewToClip[3].z;
    float viewZ1 = -B / (depth1 * (A - 1) + B);
    
    // 场景深度
    float2 uv = position.xy * g_vInvOutputRes;
    float sceneDepth = g_tClipDepth.Sample(g_sNearestClamp, uv);
    float sceneViewZ = -B / (sceneDepth * (A - 1) + B);
    
    // 深度差和Z裁剪
    float depthDiff = viewZ1 - sceneViewZ + 0.05;
    float zFactor = saturate(depthDiff / g_fHissZClipStart);
    
    // 第二次深度采样 (近距离)
    uint frame32 = (frame & 31) + 1;
    float2 jitter2 = frac(blueNoise + frame32 * float2(0.754878, 0.569840));
    jitter2 = jitter2 * 4.0 - 2.0;  // [-2, 2] 范围
    
    int2 depthCoord2 = int2((position.xy + jitter2) * g_tDepthTextureSize / g_vOutputRes);
    float viewZ2 = unpack_viewZ(g_tDepthTexture[depthCoord2]);
    
    float nearFactor = saturate(viewZ2 + 1.0 - g_fHissNearClipStart);
    
    // 混合两个深度因子
    float blendFactor = (zFactor + depthDiff) * 0.5;
    blendFactor *= nearFactor;
    
    // 采样流体颜色和场景颜色
    float4 fluidColor = g_tInputTexture.Sample(g_sLinearClamp, uv);
    float3 sceneColor = g_tFinalImageSource.Sample(g_sLinearClamp, uv).rgb;
    
    // Alpha归一化
    float alphaNorm = 1.0 / sqrt(fluidColor.a);
    blendFactor = saturate(blendFactor * alphaNorm);
    
    // 最终合成
    float3 finalColor = lerp(sceneColor, fluidColor.rgb, blendFactor);
    
    return float4(finalColor, 1.0);
}
```

**关键发现**:
| 技术 | 说明 |
|------|------|
| 双重蓝噪声抖动 | 两个不同范围 ([-4,4] 和 [-2,2]) |
| Golden Ratio偏移 | 0.754878, 0.569840 (黄金比例相关) |
| 深度逆投影 | viewZ = -B / (depth * (A-1) + B) |
| Z裁剪 | g_fHissZClipStart 控制远距离裁剪 |
| 近处混合 | g_fHissNearClipStart 控制近距离过渡 |
| Alpha归一化 | 1/sqrt(alpha) 用于边缘柔化 |

---

### 13.5 Event 13895: CS_TAAReproject (TAA 时间重投影)

**功能**: 时间抗锯齿重投影，混合历史帧实现平滑效果

**DXBC 反汇编关键部分**:
```
cs_5_0
   // 线程组初始化
   0: imad r0.xy, vThreadGroupID.xyxx, l(8, 8, 0, 0), vThreadIDInGroup.xyxx
   
   // 局部数据共享 (LDS) 填充
   // 100个元素的structured buffer, 每元素12字节 (float3)
   3-44: 读取10x10邻域颜色, 转换到XYB色彩空间
         dp3 r1.w, l(0.412456, 0.357576, 0.180438), r3.xyzx  // X
         dp3 r4.z, l(0.212673, 0.715152, 0.072175), r3.xyzx  // Y
         dp3 r2.x, l(0.019334, 0.119192, 0.950304), r3.xyzx  // Z (XYZ色彩空间)
   
   // 同步
  45: sync_g_t   // GroupMemoryBarrierWithGroupSync
   
   // 计算邻域统计 (min/max)
  52-99: ld_structured 读取邻域
         min/max 计算 (10x10范围)
         // 用于颜色裁剪
   
   // 速度采样
 100: sample_l g_tGeometryVelocityTexture.xzwy, g_sLinearClamp
 101-105: dp2/sqrt/mad ... // 速度长度计算
          // speedFactor = max(1, 4 - speed * 100000)
   
   // 深度检测
 107: ld_indexable g_tLinearDepth.xyzw        // 当前帧深度
 108: sample_l g_tPreviousLinearDepth         // 上一帧深度
 109-110: add/add_sat ...                     // 深度差 + bias
  
   // 速度变化因子
 111-115: sample_l g_tGeometryVelocityVariation
           mul r0.y, r0.y, r0.y * 100000      // 速度变化因子
           min r0.y, r0.y, l(1)               // 钳制到[0,1]
           mul r0.x, r0.x, r0.y               // 深度差 * 速度变化
   
   // 采样历史帧颜色 (3帧)
 116: sample_l g_tPreviousColors[0]  // 帧 N-1
 151: sample_l g_tPreviousColors[1]  // 帧 N-2
 181: sample_l g_tPreviousColors[2]  // 帧 N-3
   
   // XYB色彩空间裁剪
 122-134: 转换到XYB, 应用邻域min/max裁剪
           // 防止鬼影
   
   // 抖动偏移
 175-176: mad g_vSSAAJitterOffset[0]  // 当前帧抖动
          add -g_vSSAAJitterOffset[1] // 上一帧抖动偏移
   
   // 输出到3个纹理
 150: store_uav_typed g_rwtReprojectTexture[0]  // 输出1
 180: store_uav_typed g_rwtReprojectTexture[1]  // 输出2
 209: store_uav_typed g_rwtReprojectTexture[2]  // 输出3
```

**算法还原**:
```hlsl
// TAA 时间重投影
[numthreads(8, 8, 1)]
void CS_TAAReproject(uint3 DTid : SV_DispatchThreadID)
{
    int2 coord = DTid.xy;
    
    // === Phase 1: 填充LDS (邻域颜色) ===
    groupshared float3 colorLDS[100];  // 10x10邻域
    
    // 每个线程读取多个像素
    // 转换到XYZ色彩空间 (CIE XYZ)
    float3 color = g_tColorForExtents[neighborCoord];
    float X = dot(float3(0.412456, 0.357576, 0.180438), color);
    float Y = dot(float3(0.212673, 0.715152, 0.072175), color);
    float Z = dot(float3(0.019334, 0.119192, 0.950304), color);
    
    // xy归一化 (xy色度坐标)
    float sum = X + Y + Z;
    float x = X / sum;  // 默认 0.3127
    float y = Y / sum;  // 默认 0.3290
    
    colorLDS[threadIndex] = float3(x, y, Y);
    
    GroupMemoryBarrierWithGroupSync();
    
    // === Phase 2: 计算邻域统计 ===
    float3 minColor = float3(1e38, 1e38, 1e38);
    float3 maxColor = float3(-1e38, -1e38, -1e38);
    
    // 读取10x10邻域并计算min/max
    // 同时考虑2倍邻居差值用于扩展范围
    [unroll]
    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            float3 neighbor = colorLDS[i * 10 + j];
            minColor = min(minColor, neighbor);
            maxColor = max(maxColor, neighbor);
            
            // 扩展范围: 2*center - neighbor
            float3 expanded = 2 * centerColor - neighbor;
            minColor = min(minColor, expanded);
            maxColor = max(maxColor, expanded);
        }
    }
    
    float3 avgColor = (minColor + maxColor) * 0.5;
    float3 rangeColor = (maxColor - minColor) * 0.5;
    
    // === Phase 3: 深度和速度检测 ===
    float2 velocity = g_tGeometryVelocityTexture.SampleLevel(g_sLinearClamp, uv, 0);
    float speed = length(velocity);
    float speedFactor = max(1.0, 4.0 - speed * 100000.0);
    
    float currentDepth = g_tLinearDepth[coord];
    float2 prevUV = uv + velocity + g_vSSAAJitterOffset[0];
    float prevDepth = g_tPreviousLinearDepth.SampleLevel(g_sLinearClamp, prevUV, 0);
    
    float depthDiff = abs(currentDepth - prevDepth);
    float velocityVar = g_tGeometryVelocityVariation.SampleLevel(g_sLinearClamp, uv, 0);
    velocityVar = min(velocityVar * velocityVar * 100000.0, 1.0);
    
    float invalidFactor = saturate(depthDiff - g_fDepthDeltaBias) * velocityVar;
    
    // === Phase 4: 历史帧混合 (3帧) ===
    [unroll]
    for (int frame = 0; frame < 3; frame++)
    {
        // 采样历史颜色
        float3 historyColor = g_tPreviousColors[frame].SampleLevel(g_sLinearClamp, prevUV, 0);
        
        // 转换到XYB
        float3 xybHistory = RGB_to_XYB(historyColor);
        
        // 邻域裁剪 (防止鬼影)
        float3 clipped = clamp(xybHistory, minColor, maxColor);
        
        // 转回RGB
        float3 finalHistory = XYB_to_RGB(clipped);
        
        // 与当前帧混合
        float3 currentColor = g_tColorForExtents.SampleLevel(g_sLinearClamp, uv, 0);
        float3 result = lerp(finalHistory, currentColor, invalidFactor);
        
        g_rwtReprojectTexture[frame][coord] = float4(result, 1.0);
    }
}
```

**关键发现**:
| 技术 | 说明 |
|------|------|
| XYB色彩空间 | 用于颜色裁剪，防止鬼影 |
| 10x10邻域统计 | LDS存储，计算min/max范围 |
| 2倍扩展 | 2*center - neighbor 扩展裁剪范围 |
| 3帧历史混合 | 独立输出到3个纹理 |
| 深度检测 | 深度差 + 速度变化因子 |
| 抖动偏移 | SSAA抖动补偿 |

---

### 13.6 完整管线执行顺序总结

```
┌─────────────────────────────────────────────────────────────────────────────────────┐
│                           Control Hiss 完整执行顺序                                  │
├─────────────────────────────────────────────────────────────────────────────────────┤
│                                                                                     │
│  Stage 1: 流体深度更新 (Compute)                                                   │
│  ├─ 13497: CS_FluidSimulation      → Depth更新 (重投影)                            │
│  └─ 13514: CS_HissEffect           → Depth融合 (场景深度)                          │
│                                                                                     │
│  Stage 2: 流体可视化 (Pixel)                                                       │
│  ├─ 13536: 深度可视化              → 流体颜色                                      │
│  ├─ 13550: 深度可视化 (变体)        → 流体颜色                                      │
│  └─ 13641-13645: 角色渲染          → 角色+深度测试                                 │
│                                                                                     │
│  Stage 3: 速度场生成 (Pixel)                                                       │
│  ├─ 13665: 前置处理                → 未知                                          │
│  └─ 13688: PS_VelocityGeneration   → Velocity (运动+力场)                          │
│                                                                                     │
│  Stage 4: 流体平流 (Pixel)                                                         │
│  └─ 13668: PS_FluidAdvection       → FluidData (Semi-Lagrangian)                   │
│                                                                                     │
│  Stage 5: 压力求解 (Pixel) - 8轮Jacobi                                             │
│  ├─ 13734: PS_Divergence           → Divergence (∇·v)                              │
│  ├─ 13747-13795: Jacobi迭代        → Pressure (∇²p = -∇·v)                         │
│  ├─ 13810: PS_PressureProjection   → Velocity修正 (v - ∇p)                         │
│  └─ 13827: PS_FluidAdvectionGaussian → FluidData平滑                               │
│                                                                                     │
│  Stage 6: 最终合成 (Pixel + Compute)                                               │
│  ├─ 13846: PS_HissMaskApply        → 应用Hiss纹理Mask                              │
│  ├─ 13847: CopyResource            → 复制到历史缓冲                                │
│  ├─ 13870: PS_HissComposite        → 深度检测+最终合成                             │
│  └─ 13895: CS_TAAReproject         → TAA时间重投影                                 │
│                                                                                     │
└─────────────────────────────────────────────────────────────────────────────────────┘
```

---

## 15. 总结

### 三个问题的核心答案

| 问题 | 核心技术 |
|------|----------|
| **边界清晰** | 整数坐标采样 + atomic_umin 原子操作，完全避免双线性插值 |
| **色散效果** | 4层独立 Presence 颜色系统 + 速度场驱动的扭曲，非传统折射色散 |
| **深度遮挡** | 场景深度比较 + 深度混合 + 无效标记 + 蓝噪声抖动 |

### 完整管线关键事件

| Event | 类型 | 核心功能 | 关键技术 |
|-------|------|----------|----------|
| **13497** | CS | 时间重投影 | 4点采样 + atomic_umin |
| **13514** | CS | 场景融合 | 蓝噪声抖动 + 深度混合 |
| **13536/13550** | PS | 深度可视化 | 4层颜色叠加 |
| **13641-13645** | PS | 角色渲染 | 深度测试discard |
| **13668** | PS | Semi-Lagrangian平流 | 回溯采样 |
| **13688** | PS | 速度场生成 | 径向力+向上力 |
| **13734** | PS | 散度计算 | ∇·v |
| **13747-13795** | PS | Jacobi迭代 | 压力求解 |
| **13810** | PS | 压力投影修正 | v - ∇p * 15 |
| **13827** | PS | 高斯平流 | 7点B样条核 |
| **13846** | PS | Hiss Mask | Sobel边缘+UV动画 |
| **13847** | Copy | 复制历史 | - |
| **13870** | PS | 最终合成 | 双重蓝噪声+深度检测 |
| **13895** | CS | TAA重投影 | XYB裁剪+3帧混合 |

### 关键 DXBC 指令总结

```
边界清晰:
  ftoi r4.xy, r4.xyxx                 // 整数坐标
  atomic_umin g_rwtDepthTexture       // 原子最小值

深度遮挡:
  lt r0.w, l(4294537728), r0.z        // 无效标记检查 (0xFFF00000)
  mad r0.z, -r0.z, r2.y, r2.z         // 逆投影变换
  mad r3.x, r3.x, r2.y, r0.z          // 深度混合

色散效果:
  // 通过多层颜色参数和独立运动实现
  g_vPresenceLayer1-4Color            // 4 层颜色
  g_vPresenceLayer1-4Params           // 4 层参数

压力投影修正 (Event 13810):
  add r0.x, -r0.y, r0.x               // ∂p/∂x = P(x-1) - P(x+1)
  mad r0.xy, -r0.xyxx, l(15, 15), r0.zwzz  // v_new = v - ∇p * 15

TAA重投影 (Event 13895):
  dp3 X = (0.412, 0.357, 0.180)       // RGB → XYZ
  dp3 Y = (0.212, 0.715, 0.072)       // 亮度通道
  dp3 Z = (0.019, 0.119, 0.950)       // 色度通道
  div x = X/(X+Y+Z)                   // xy色度坐标
```

### 关键参数汇总

| 参数 | 默认值 | 说明 |
|------|--------|------|
| `g_fHissIntensity` | - | Hiss强度 |
| `g_fHissRadialForce` | - | 径向扩散力 |
| `g_fHissUpwardsForce` | - | 向上浮力 |
| `g_fHissBlendTowardsSceneDepth` | - | 深度混合速率 |
| `g_fHissZClipStart` | - | Z裁剪起始距离 |
| `g_fHissNearClipStart` | - | 近处裁剪起始 |
| 压力系数 | 15.0 | 不可压缩性强度 |
| 速度钳制 | ±2000 | 数值稳定性 |
| Jacobi迭代 | 8轮 | 压力求解精度 |

---

## 16. 创新技术总结

> **注意**：本章节的技术分析已整合并提炼到新文档 **`control-hiss-innovations.md`** 中。
> 
> 新文档以 Control 的创新技术点为核心重新组织，删除了错误结论，合并了重复内容。
> 
> **推荐阅读新文档**，获得更清晰的技术视角。

以下内容保留作为原始分析记录：

---

## 三个问题的新理解（基于完整分析）



---

### 16.2 问题一：重投影边界清晰 - 新发现

#### 关键新发现：深度清零现象

在 Event 13536 和 Event 13870 中发现：

```
DXBC 指令:
  ld_indexable g_tDepthTexture    // 读取 R32_UINT 数据
  utof r0.x                        // 转为 float
  mul r0.x, r0.x, l(0.000000)     // 清零！
```

**这意味着 R32_UINT 纹理存储的不是真正的深度！**

#### 新的理解

R32_UINT 纹理 (426x240) 实际存储的是：
- **密度/权重数据**，而非实际深度
- 无效标记 `> 0xFFF00000`
- 在渲染阶段被清零，使用 depth=0 进行重投影

#### 边界清晰的完整机制

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                        边界清晰的三个层次                                    │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  层次1: 密度场更新 (Event 13497)                                            │
│  ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━                                             │
│  技术: atomic_umin + 整数坐标采样                                           │
│  效果: 多个线程写入同一像素时保留最小值（前景）                              │
│                                                                             │
│  层次2: 场景深度融合 (Event 13514)                                          │
│  ━━━━━━━━━━━━━━━━━━━━━━━━━━                                                 │
│  技术: 深度比较 + 无效标记                                                  │
│  效果: 场景遮挡区域被标记为无效                                             │
│                                                                             │
│  层次3: 最终合成 (Event 13870)                                              │
│  ━━━━━━━━━━━━━━━━━━━━━━━━                                                   │
│  技术: 双重蓝噪声抖动 + Alpha归一化                                         │
│  效果: 边缘柔化但不模糊，保持清晰边界                                       │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

#### 最终合成中的 Alpha 归一化

```hlsl
// Event 13870 DXBC 指令 57-62:
float4 fluidColor = g_tInputTexture.Sample(uv);
float3 sceneColor = g_tFinalImageSource.Sample(uv);

// Alpha 归一化 (关键！)
float alphaNorm = 1.0 / sqrt(fluidColor.a);  // rsq r1.x, r2.w
blendFactor = saturate(blendFactor * alphaNorm);

// 最终合成
float3 result = lerp(sceneColor, fluidColor.rgb, blendFactor);
```

**作用**：当 alpha 较大时（边界），归一化因子减小，混合减弱，边界保持清晰

---

### 16.3 问题二：色散效果 - 新发现

#### 关键新发现：颜色梯度驱动色散（Event 13846）

**最重要的发现：Event 13846 通过采样颜色场梯度实现了真正的色散效果！**

从DXBC详细分析（参见13.3.1节），色散效果的核心算法：

```hlsl
// Event 13846: 颜色梯度色散核心代码

// 1. 采样颜色场的5个邻居
float3 center = g_tInputTexture[coord];           // 中心
float3 left   = g_tInputTexture[coord + (-1, 0)]; // 左
float3 right  = g_tInputTexture[coord + (1, 0)];  // 右
float3 up     = g_tInputTexture[coord + (0, 1)];  // 上
float3 down   = g_tInputTexture[coord + (0, -1)]; // 下

// 2. 计算每个颜色通道的空间梯度 (Sobel算子)
float3 gradientX = left - right;  // ∂R/∂x, ∂G/∂x, ∂B/∂x
float3 gradientY = down - up;     // ∂R/∂y, ∂G/∂y, ∂B/∂y
float3 colorGradient = gradientX + gradientY;

// 3. Alpha加权梯度位移
float alphaWeight = center.a * center.a * 0.25;
float3 displacement = colorGradient * alphaWeight * g_fWorldTimeDelta * 30.0;

// 4. 应用位移（RGB通道分别向不同方向偏移！）
float3 finalColor = center.rgb + displacement;

// 结果：每个颜色通道根据其梯度独立位移，产生色散效果
// R通道向 ∇R 方向位移
// G通道向 ∇G 方向位移
// B通道向 ∇B 方向位移
// → RGB空间分离 = 色散！
```

#### 色散效果的完整机制

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                           Hiss 色散效果完整机制                               │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  阶段1: 多层颜色生成 (Event 13536/13550)                                    │
│  ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━                                       │
│  输入: g_vPresenceLayer1-4Color + Params                                    │
│  处理: 基于距离的多层颜色叠加                                                │
│  输出: 流体颜色 (R8G8B8A8)，不同区域有不同颜色组合                           │
│                                                                             │
│  阶段2: 颜色梯度色散 (Event 13846) ⭐ 核心步骤！                             │
│  ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━                                           │
│  输入: 流体颜色场 (1707x960, R8G8B8A8)                                       │
│  处理:                                                                      │
│    - Sobel算子采样颜色场梯度                                                │
│    - 对每个RGB通道分别计算空间导数                                          │
│    - 根据梯度位移每个通道: color += ∇color * weight                         │
│  输出: RGB通道空间分离的色散效果                                            │
│  效果: 棱镜般的RGB分离，类似传统折射色散                                     │
│                                                                             │
│  阶段3: UV动画Mask (Event 13846 后续)                                       │
│  ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━                                             │
│  输入: g_tHissMask (BC4 纹理)                                               │
│  处理: 双轴不同速度UV动画                                                   │
│    - X轴: time * 0.1 (慢速)                                                 │
│    - Y轴: time * 15.0 (快速)                                                │
│  效果: 时间维度的颜色分离，增强色散动态感                                    │
│                                                                             │
│  阶段4: 最终合成 (Event 13870)                                              │
│  ━━━━━━━━━━━━━━━━━━━━━━━━━━                                                 │
│  输入: 色散后的流体颜色 + 场景颜色                                           │
│  处理:                                                                      │
│    - 双重蓝噪声抖动                                                         │
│    - Alpha归一化 (1/√α)                                                     │
│    - 场景混合                                                               │
│  效果: 边缘清晰且带有色散的最终效果                                          │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

#### 色散效果的两个维度

**空间维度色散（Event 13846颜色梯度）：**
```hlsl
// RGB三个通道根据各自的空间梯度向不同方向位移
// 物理意义：颜色扩散，类似墨水在水中扩散

R'(x,y) = R(x,y) + (∂R/∂x + ∂R/∂y) * weight
G'(x,y) = G(x,y) + (∂G/∂x + ∂G/∂y) * weight  
B'(x,y) = B(x,y) + (∂B/∂x + ∂B/∂y) * weight

视觉效果：静态的RGB边缘分离
```

**时间维度色散（Event 13846 UV动画）：**
```hlsl
// 通过不同速度的UV动画产生时间上的颜色分离
// 物理意义：不同颜色层以不同速度流动

maskUV.x = uv.x * 0.1 + time * 0.1    // 慢速水平流动
maskUV.y = uv.y + time * 15.0          // 快速垂直流动

视觉效果：动态的颜色拖尾和流动分离
```

#### 与传统色散的对比

```
传统色散（折射）:
  物理原理: 不同波长光的折射率不同
  公式: n(λ) = n₀ + k/λ²
  效果: RGB按波长分离

Hiss流体色散（颜色梯度）:
  算法原理: 不同颜色通道的空间梯度不同
  公式: C' = C + ∇C * weight
  效果: RGB按梯度方向分离
  
共同点: 最终都是RGB通道的空间分离
不同点: 
  - 传统色散基于物理光学
  - Hiss色散基于图像处理（Sobel算子）
  - Hiss色散可控制性更强（通过颜色场设计）
```

#### 关键技术参数

| 参数 | 值 | 作用 |
|------|-----|------|
| **梯度计算** | Sobel 4邻居 | 颜色场空间导数 |
| **Alpha权重** | α² * 0.25 | 位移强度控制 |
| **位移系数** | 30.0 * dt | 色散强度 |
| **UV动画X** | time * 0.1 | 时间维度色散（慢） |
| **UV动画Y** | time * 15.0 | 时间维度色散（快） |
| **Mask阈值** | 0.3 + smoothstep | 边缘控制 |

#### 色散效果的应用要点

要在UE/Niagara中实现类似效果：

```hlsl
// 1. 创建颜色梯度色散模块
float3 ComputeColorGradientDispersion(
    Texture2D<float4> colorField,
    float2 uv,
    float alpha,
    float deltaTime
)
{
    int2 coord = int2(uv * resolution);
    
    // Sobel采样
    float3 center = colorField[coord];
    float3 left   = colorField[coord + int2(-1, 0)];
    float3 right  = colorField[coord + int2(1, 0)];
    float3 up     = colorField[coord + int2(0, 1)];
    float3 down   = colorField[coord + int2(0, -1)];
    
    // 颜色梯度
    float3 gradient = (left - right) + (down - up);
    
    // Alpha加权位移
    float weight = alpha * alpha * 0.25 * deltaTime * 30.0;
    
    return center + gradient * weight;
}

// 2. UV动画模块
float2 AnimatedMaskUV(float2 baseUV, float time)
{
    return float2(
        baseUV.x * 0.1 + time * 0.1,   // 慢速X
        baseUV.y + time * 15.0          // 快速Y
    );
}
```

---

### 16.4 问题三：深度遮挡 - 新发现

#### 关键新发现：深度检测在最终合成阶段

之前认为深度检测只在 Event 13514，现在发现 Event 13870 也有完整的深度检测：

```hlsl
// Event 13870 DXBC 指令分析:

// 1. 双重深度采样（不同抖动范围）
//    第一次: [-4, 4] 范围抖动
//    第二次: [-2, 2] 范围抖动

// 2. Z 裁剪
float zFactor = saturate(depthDiff / g_fHissZClipStart);

// 3. 近处混合
float nearFactor = saturate(viewZ + 1.0 - g_fHissNearClipStart);

// 4. 场景深度比较
float sceneDepth = g_tClipDepth.Sample(uv);
float sceneViewZ = depth_to_viewZ(sceneDepth);
float depthDiff = viewZ - sceneViewZ;
```

#### 深度遮挡的完整机制

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                           深度遮挡完整机制                                   │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  阶段1: 密度场遮挡 (Event 13514)                                            │
│  ━━━━━━━━━━━━━━━━━━━━━━━━━━━                                                │
│  技术: 场景深度比较 + 深度混合                                              │
│  输入: g_tClipDepth (场景深度)                                              │
│  输出: 密度场 (带无效标记)                                                   │
│                                                                             │
│  阶段2: 角色遮挡 (Event 13641-13645)                                        │
│  ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━                                              │
│  技术: 深度测试 discard                                                     │
│  效果: 角色正确遮挡流体                                                      │
│                                                                             │
│  阶段3: 最终深度检测 (Event 13870)                                          │
│  ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━                                             │
│  技术:                                                                      │
│    - 双重深度采样 (不同抖动范围)                                            │
│    - Z裁剪 (g_fHissZClipStart)                                             │
│    - 近处混合 (g_fHissNearClipStart)                                        │
│    - 场景深度比较                                                           │
│  效果: 最终遮挡正确性                                                        │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

#### 耗散机制的更新

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                           耗散机制（更新版）                                 │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  Level 1: 几何速度耗散                                                      │
│  ━━━━━━━━━━━━━━━━━━━━━                                                      │
│  来源: Event 13688 速度场生成                                               │
│  机制: 几何停止运动时，速度归零                                             │
│                                                                             │
│  Level 2: 深度耗散                                                          │
│  ━━━━━━━━━━━━━━━━━                                                          │
│  来源: Event 13514 场景融合                                                 │
│  机制: 向场景深度混合 (g_fHissBlendTowardsSceneDepth)                       │
│                                                                             │
│  Level 3: 无效标记耗散                                                      │
│  ━━━━━━━━━━━━━━━━━━━                                                        │
│  来源: Event 13497 + 13514                                                  │
│  机制: 超出有效范围标记为 0xFFF00000                                        │
│                                                                             │
│  Level 4: Mask 耗散 (新发现)                                                │
│  ━━━━━━━━━━━━━━━━━━━━━━━━                                                   │
│  来源: Event 13846 HissMaskApply                                            │
│  机制: BC4 纹理 Mask 控制渐变                                               │
│        阈值 0.3 + smoothstep                                                │
│                                                                             │
│  Level 5: Alpha 耗散 (新发现)                                               │
│  ━━━━━━━━━━━━━━━━━━━━━━━━                                                   │
│  来源: Event 13870 最终合成                                                 │
│  机制: Alpha 归一化 (1/sqrt(alpha))                                         │
│        当 alpha 降低时，混合因子减小                                        │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

### 16.5 总结：三个问题的最终答案

| 问题 | 核心技术 | 关键 Event | 新发现 |
|------|----------|------------|--------|
| **边界清晰** | atomic_umin + 整数坐标 + Alpha归一化 | 13497, 13870 | R32_UINT存储的是密度而非深度；Alpha归一化保持边界 |
| **色散效果** | **颜色梯度位移(Sobel)** + UV动画 + Alpha归一化 | **13846**, 13536, 13870 | **Event 13846对颜色场采样Sobel梯度，RGB通道根据各自梯度独立位移产生色散**；空间+时间双重色散 |
| **深度遮挡** | 多阶段深度检测 + Mask耗散 | 13514, 13641, 13870 | 三阶段深度检测；BC4 Mask 控制渐变耗散 |

---

## 17. 相关文档

| 文档 | 内容 |
|------|------|
| `fluid-systems-analysis.md` | Niagara 流体系统分析 |
| `fluid-module-details.md` | 流体模块详细实现 |
| `grid2d-datainterface.md` | Grid2D 底层实现 |
