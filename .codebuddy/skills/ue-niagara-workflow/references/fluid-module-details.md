# Niagara Fluids 模块详细实现

本文档详细分析每个流体模拟模块的内部实现和数据流。

---

## 1. 初始化阶段 (Spawn Script)

### 1.1 Grid2D_SetResolution

**功能**: 设置网格分辨率和物理尺寸

**输入参数**:
| 参数 | 类型 | 说明 |
|------|------|------|
| NumCellsX/Y | int | 独立分辨率模式 |
| NumCellsMaxAxis | int | 最大轴分辨率模式 |
| WorldCellSize | float | 单元格大小模式 |
| WorldGridExtents | float2 | 世界空间范围 |
| ResolutionMult | float | 分辨率缩放因子 |

**分辨率模式**:
```
┌─ Independent ────────→ 直接使用 NumCellsX/Y
├─ Max Axis ───────────→ 根据 WorldGridExtents 和 NumCellsMaxAxis 计算
├─ World Cell Size ────→ 根据 WorldGridExtents 和 WorldCellSize 计算
├─ Other Grid ─────────→ 从另一个 Grid2D 复制分辨率
└─ VolumeTexture ──────→ 从体积纹理获取尺寸
```

**Max Axis 模式 HLSL**:
```hlsl
float CellSize = max(WorldGridExtents.x, WorldGridExtents.y) / NumCellsMaxAxis;
NumCellsX = floor(WorldGridExtents.x / CellSize);
NumCellsY = floor(WorldGridExtents.y / CellSize);

// 修正确保覆盖整个范围
if (WorldGridExtents.x > WorldGridExtents.y && 
    abs(CellSize * NumCellsY - WorldGridExtents.y) > 1e-8)
{
    NumCellsY++;
}
```

**输出**:
- `Emitter.Module.WorldGridExtents` - 实际世界范围
- `Emitter.Module.WorldCellSize` - 单元格大小
- `Emitter.Module.NumCellsX/Y` - 分辨率

---

## 2. 输入阶段 (Update Script 开始)

### 2.1 Grid2D_Gas_ParticleScatterSource

**功能**: 将源粒子数据散射到网格

**核心 HLSL 实现**:
```hlsl
// 读取粒子数据
float3 Velocity = mul(float4(ParticleVel, 0.), WorldToLocal).xyz;
Velocity *= VelocityMult;

// 计算拖尾采样数（快速移动时）
int NumStreakParticles = 1;
if (UseStreaking)
{
    float IndexVelocityMagnitude = length(dt * Velocity / dx);
    NumStreakParticles = min(MaxStreakSamples, 
                             max(1, StreakDensity * IndexVelocityMagnitude / (2. * ParticleRadiusIndex)));
}

// 分摊密度和温度
ParticleDensity /= NumStreakParticles;
ParticleTemperature /= NumStreakParticles;

// 遍历拖尾采样点
for (int ww = 0; ww < NumStreakParticles; ++ww)
{
    float StreakPosition = 1.0 * ww / NumStreakParticles;
    float2 FloatIndexToUse = ParticleCenterFloatIndex - StreakPosition * IndexVelocity.xy;
    
    // 遍历粒子半径范围内的网格单元
    for (int y = -ParticleRadiusIndex; y < ParticleRadiusIndex; ++y) {
    for (int x = -ParticleRadiusIndex; x < ParticleRadiusIndex; ++x) {
        int2 CellIndex = int2(x,y) + FloatIndexToUse;
        float Dist = length(FloatIndexToUse - float2(CellIndex.x, CellIndex.y));
        
        if (Dist <= ParticleRadiusIndex && 
            CellIndex.x >= 0 && CellIndex.x < NumCellsX &&
            CellIndex.y >= 0 && CellIndex.y < NumCellsY)
        {
            // 计算衰减
            float FalloffMult = 1. - pow(smoothstep(ParticleRadius * Core, ParticleRadius, Dist), Falloff);
            
            // 写入网格
            Grid.SetFloatValue<Attribute="Density">(CellIndex.x, CellIndex.y, Density * FalloffMult);
            Grid.SetFloatValue<Attribute="Temperature">(CellIndex.x, CellIndex.y, Temperature * FalloffMult);
            Grid.SetVector2DValue<Attribute="Velocity">(CellIndex.x, CellIndex.y, Velocity.xy * FalloffMult);
        }
    }}
}
```

**数据流**:
```
源粒子 (Position, Velocity, Radius, Density, Temperature, Color)
    │
    ├─→ 坐标变换 (World → Local)
    │
    ├─→ 计算网格索引
    │
    └─→ 循环写入 Grid2D
         ├── Density 属性
         ├── Temperature 属性
         └── Velocity 属性
```

---

### 2.2 Grid2D_IntegrateForces

**功能**: 将累积的力积分到速度场

**数据流**:
```
Transient.PhysicsForce (本地空间力)
    │
    ├─→ 累加
    │
Transient.PhysicsForceWorld (世界空间力)
    │
    ├─→ TransformVector (WorldToLocal^T)
    │
    └─→ 累加到 PhysicsForce
           │
           ├─→ * dt
           │
           └─→ + Velocity → NewVelocity
```

**核心公式**:
```
NewVelocity = Velocity + (PhysicsForce + Transpose(WorldToLocal) * PhysicsForceWorld) * dt
```

---

### 2.3 Grid2D_ComputeBoundary

**功能**: 标记每个单元格的边界类型

**边界类型**:
| 值 | 类型 | 说明 |
|----|------|------|
| 0 | FLUID_CELL | 流体单元，正常模拟 |
| 1 | SOLID_CELL | 固体单元，使用固体速度 |
| 2 | EMPTY_CELL | 空单元，自由表面 |

**边界检测流程**:
```
┌─ 网格边缘检测 ────────────────────────────────┐
│  if (IndexX < BorderWidth) Boundary = 1 或 2  │
│  if (IndexX >= NumCellsX - BorderWidth) ...   │
│  if (IndexY < BorderWidth) ...                │
│  if (IndexY >= NumCellsY - BorderWidth) ...   │
└───────────────────────────────────────────────┘
                    ↓
┌─ 碰撞检测 (可选) ─────────────────────────────┐
│  CollisionQuery.DistanceToNearestSurface      │
│  if (Distance <= 0) Boundary = 1              │
└───────────────────────────────────────────────┘
                    ↓
┌─ 深度缓冲检测 (可选) ─────────────────────────┐
│  GBuffer 采样                                  │
│  ScreenUV 计算                                 │
│  WorldVelocity 获取                           │
│  if (距离 < DepthBufferThickness) Boundary = 1│
└───────────────────────────────────────────────┘
                    ↓
┌─ 粒子碰撞 (可选) ─────────────────────────────┐
│  遍历粒子，检测距离                            │
│  if (Dist <= ParticleRadius) Boundary = 1     │
│  同时获取粒子速度作为 SolidVelocity            │
└───────────────────────────────────────────────┘
                    ↓
┌─ 自由表面检测 ─────────────────────────────────┐
│  if (length(Velocity) < 1e-5 && Boundary==0)  │
│      Boundary = 2 (EMPTY_CELL)                │
└───────────────────────────────────────────────┘
```

**固体速度获取 (GBuffer 模式)**:
```hlsl
// 屏幕空间坐标计算
float4 SamplePosition = float4(In_SamplePos + View.PreViewTranslation, 1);
float4 ClipPosition = mul(SamplePosition, View.TranslatedWorldToClip);
float2 ScreenPosition = ClipPosition.xy / ClipPosition.w;
ScreenUV = ScreenPosition * View.ScreenPositionScaleBias.xy + View.ScreenPositionScaleBias.wz;

// 从 GBuffer 获取世界速度
GBufferInterface.GetGBufferWorldVelocity(ScreenUV, IsValid, WorldVelocity);

// 转换为本地速度
Vector = isnan(WorldVelocity) ? float3(0,0,0) : WorldVelocity/dt;
SolidVelocity = TransformVector(WorldToLocal, Vector) * VelocityScale;
```

---

## 3. 平流阶段

### 3.1 Grid2D_AdvectScalar

**功能**: 半拉格朗日平流，传输标量场

**核心原理**:
```
传统平流: q_new = q(x + u*dt)  // 向前追踪，不稳定
半拉格朗日: q_new = q(x - u*dt)  // 向后追踪，无条件稳定
```

**Semi-Lagrangian (一阶) HLSL**:
```hlsl
float Scale = dt/dx * AdvectionVelocityMult;

// 读取当前速度
float2 Velocity;
VelocityGrid.GetGridValue(IndexX, IndexY, VelocityIndex, Velocity.x);
VelocityGrid.GetGridValue(IndexX, IndexY, VelocityIndex + 1, Velocity.y);

// 回溯位置
float2 Index = float2(IndexX, IndexY);
float2 SampleIndex = Index - Scale * Velocity;

// 转换为单位坐标
float3 SampleUnit;
VelocityGrid.IndexToUnit(SampleIndex.x, SampleIndex.y, SampleUnit);

// 采样上一帧数据
if (InterpolationMethod == 1)  // 三次插值
    AdvectedGrid.CubicSamplePreviousGridAtIndex(SampleUnit.x, SampleUnit.y, ScalarIndex, AdvectedScalar);
else  // 线性插值
    AdvectedGrid.SamplePreviousGridAtIndex(SampleUnit.x, SampleUnit.y, ScalarIndex, AdvectedScalar);
```

**RK2 (Runge-Kutta 2阶) HLSL**:
```hlsl
float Scale = dt/dx * AdvectionVelocityMult;

// 读取当前速度
float2 Velocity;
VelocityGrid.GetGridValue(IndexX, IndexY, VelocityIndex, Velocity.x);
VelocityGrid.GetGridValue(IndexX, IndexY, VelocityIndex+1, Velocity.y);

// 第一步：半步回溯
float2 Index = float2(IndexX, IndexY);
float2 SampleIndex = Index - 0.5 * Scale * Velocity;

// 在半步位置采样速度
float3 SampleUnit;
VelocityGrid.IndexToUnit(SampleIndex.x, SampleIndex.y, SampleUnit);
VelocityGrid.SampleGrid(SampleUnit.x, SampleUnit.y, VelocityIndex, Velocity.x);
VelocityGrid.SampleGrid(SampleUnit.x, SampleUnit.y, VelocityIndex+1, Velocity.y);

// 第二步：完整回溯
SampleIndex = Index - Scale * Velocity;

// 最终采样
if (InterpolationMethod == 1)
    AdvectedGrid.CubicSamplePreviousGridAtIndex(SampleUnit.x, SampleUnit.y, ScalarIndex, AdvectedScalar);
else
    AdvectedGrid.SamplePreviousGridAtIndex(SampleUnit.x, SampleUnit.y, ScalarIndex, AdvectedScalar);
```

**数据流**:
```
VelocityGrid (速度场) ─────→ 计算回溯位置
        │
        ↓
AdvectedGrid (上一帧) ─────→ 采样标量值
        │
        ↓
Output: AdvectedScalar (平流后的值)
```

---

## 4. 力学阶段

### 4.1 Grid2D_ComputeCurl

**功能**: 计算速度场的旋度 ∇×u

**HLSL 实现**:
```hlsl
// 采样四个方向的垂直速度分量
float Vy_right;
Grid.GetGridValue(IndexX+1, IndexY, VectorIndex+1, Vy_right);

float Vy_left;
Grid.GetGridValue(IndexX-1, IndexY, VectorIndex+1, Vy_left);

float Vx_up;
Grid.GetGridValue(IndexX, IndexY+1, VectorIndex, Vx_up);

float Vx_down;
Grid.GetGridValue(IndexX, IndexY-1, VectorIndex, Vx_down);

// 2D 旋度公式
curl = ((Vy_right - Vy_left) - (Vx_up - Vx_down)) / (2. * dx);
```

**数学公式**:
$$\omega = \frac{\partial v}{\partial x} - \frac{\partial u}{\partial y} \approx \frac{v_{i+1} - v_{i-1}}{2\Delta x} - \frac{u_{j+1} - u_{j-1}}{2\Delta y}$$

---

### 4.2 Grid2D_VorticityConfinementForce

**功能**: 涡度限制力，补偿数值耗散

**HLSL 实现**:
```hlsl
VC_Force = float2(0,0);

float GradCurlLength = length(GradCurl);
if (GradCurlLength > 1e-5)
{
    // N = ∇|ω| / |∇|ω||
    float2 N = GradCurl / GradCurlLength;
    
    // F = ε * dx * (N × ω_z) 
    // 2D中: N × (0,0,1) = (N.y, -N.x)
    VC_Force = VorticityMult * dx * cross(float3(N, 0), float3(0,0,1)).xy;
}

// 累加到物理力
Transient.PhysicsForce += VC_Force;
```

**数学原理**:
$$\mathbf{f}_{vc} = \epsilon \cdot \Delta x \cdot (\mathbf{N} \times \boldsymbol{\omega})$$

其中：
- $\mathbf{N} = \frac{\nabla|\omega|}{|\nabla|\omega||}$ 是涡度梯度方向
- $\epsilon$ 是涡度强度系数
- $\omega$ 是旋度标量 (2D)

---

### 4.3 Grid2D_Gas_3DBuoyancy

**功能**: 浮力模拟

**HLSL 实现**:
```hlsl
// 浮力公式
Force = (DensityBuoyancy * SimGrid_Density - TemperatureBuoyancy * SimGrid_Temperature) * g;

// 累加到世界空间力
Transient.PhysicsForceWorld += Force;
```

**物理意义**:
```
密度浮力: 密度大 → 向下 (负值时向上)
温度浮力: 温度高 → 向上

典型设置:
- DensityBuoyancy = -0.5 (烟雾上升)
- TemperatureBuoyancy = 1.0 (热气上升)
- g = (0, 0, -9.8) (重力方向)
```

---

## 5. 投影阶段

### 5.1 Grid2D_ComputeDivergence

**功能**: 计算速度场散度 ∇·u

**HLSL 实现**:
```hlsl
#if GPU_SIMULATION
// 采样四个邻居的速度分量
float Vx_right;
Grid.GetGridValue(IndexX+1, IndexY, VectorIndex, Vx_right);

float Vx_left;
Grid.GetGridValue(IndexX-1, IndexY, VectorIndex, Vx_left);

float Vy_up;
Grid.GetGridValue(IndexX, IndexY+1, VectorIndex+1, Vy_up);

float Vy_down;
Grid.GetGridValue(IndexX, IndexY-1, VectorIndex+1, Vy_down);

// 中心差分
Div = (Vx_right - Vx_left + Vy_up - Vy_down) / (2. * dx);
#else
Div = 0.0;
#endif
```

**两种差分版本**:
| 版本 | 公式 | 用途 |
|------|------|------|
| 中心差分 | (V_right - V_left) / (2*dx) | 精确 |
| 单侧差分 | (V_right - V_left) / dx | 简化 |

---

### 5.2 Grid2D_PressureIteration

**功能**: Jacobi 迭代求解压力泊松方程

**压力泊松方程**:
$$\nabla^2 p = \frac{\rho}{\Delta t} \nabla \cdot \mathbf{u}$$

**离散形式**:
$$p_{i,j} = \frac{p_{i+1,j} + p_{i-1,j} + p_{i,j+1} + p_{i,j-1} - \rho \Delta x^2 \frac{\nabla \cdot \mathbf{u}}{\Delta t}}{4}$$

**完整 HLSL 实现**:
```hlsl
Pressure = 0;

#if GPU_SIMULATION

int FluidCellCount = 4;
float BoundaryAdd = 0.0;

const int FLUID_CELL = 0;
const int SOLID_CELL = 1;
const int EMPTY_CELL = 2;
int CellType = round(B_center);

// Red-Black SOR 权重计算
int RowParity = (IndexY + IterationIndex) % 2;
int CellParity = (IndexX + RowParity) % 2;
float Weight;

if (Relaxation < 1e-8)
{
    Weight = 1;  // 标准 Jacobi
}
else
{
    // Red-Black SOR: 交替更新红黑格子
    Weight = CellParity * min(1.93, Relaxation + 1);
}

// 检查右侧邻居
int CellType_right = round(B_right);
if (CellType_right == SOLID_CELL)
{
    FluidCellCount--;
    // 固体边界条件: 压力梯度抵消速度差
    BoundaryAdd += density * dx * (Velocity.x - SV_x_right) / dt;
    P_right = 0;
}
else if (CellType_right == EMPTY_CELL)
{
    P_right = 0;  // 自由表面压力为0
}

// 类似处理左、上、下邻居...

// Jacobi 迭代公式
float JacobiPressure;
if (FluidCellCount > 0 && CellType == FLUID_CELL)
{
    JacobiPressure = (P_right + P_left + P_up + P_down 
                      - density * dx * dx * Divergence / dt 
                      + BoundaryAdd) / FluidCellCount;
    
    // SOR 更新
    Pressure = (1.f - Weight) * P_center + Weight * JacobiPressure;
}
#endif
```

**Red-Black SOR 优化**:
```
迭代 N 次，每次:
  1. 更新 "红" 格子 (棋盘格)
  2. 更新 "黑" 格子
  
优点: 收敛速度更快，并行友好
```

**迭代次数建议**:
| 分辨率 | 迭代次数 | 说明 |
|--------|----------|------|
| 128x128 | 40-60 | 基础精度 |
| 256x256 | 60-80 | 推荐设置 |
| 512x512 | 80-100 | 高精度 |

---

### 5.3 Grid2D_ProjectPressure

**功能**: 用压力梯度修正速度场

**HLSL 实现**:
```hlsl
// 采样邻格压力
float P_right, P_left, P_up, P_down;
PressureGrid.GetFloatValue<Attribute=Pressure>(IndexX+1, IndexY, P_right);
PressureGrid.GetFloatValue<Attribute=Pressure>(IndexX-1, IndexY, P_left);
PressureGrid.GetFloatValue<Attribute=Pressure>(IndexX, IndexY+1, P_up);
PressureGrid.GetFloatValue<Attribute=Pressure>(IndexX, IndexY-1, P_down);

// 计算压力梯度
float2 PressureGradient = float2(P_right - P_left, P_up - P_down) / (2.0 * dx);

// 修正速度
Velocity = Velocity - PressureGradient * dt / density;

// 边界条件处理
VelocityOut = float2(0,0);
int CellType = round(B_center);

// 检查邻居边界类型...
if (CellType_left == SOLID_CELL)
    VelocityOut.x = SV_x_left;
else if (CellType_right == SOLID_CELL)
    VelocityOut.x = SV_x_right;
else
    VelocityOut.x = Velocity.x;

// 当前是固体
if (CellType == SOLID_CELL)
{
    VelocityOut.x = SV_x_center;
    VelocityOut.y = SV_y_center;
}
```

---

## 6. 完整数据流图

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                          Spawn Script (初始化)                               │
├─────────────────────────────────────────────────────────────────────────────┤
│  Grid2D_SetResolution                                                        │
│  ├── 输入: NumCells, WorldExtents                                           │
│  └── 输出: Emitter.NumCellsX/Y, WorldCellSize                               │
└─────────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                          Update Script (每帧)                                │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│  ┌─ 输入阶段 ─────────────────────────────────────────────────────────────┐ │
│  │                                                                        │ │
│  │  Grid2D_Gas_ParticleScatterSource                                      │ │
│  │  ├── 输入: Particles.Position/Velocity/Density/Temperature            │ │
│  │  └── 输出: Grid2D.Density/Temperature/Velocity (写入)                  │ │
│  │                                                                        │ │
│  │  Grid2D_IntegrateForces                                                │ │
│  │  ├── 输入: Transient.PhysicsForce + PhysicsForceWorld                 │ │
│  │  └── 输出: NewVelocity = Velocity + Force*dt                          │ │
│  │                                                                        │ │
│  │  Grid2D_ComputeBoundary                                                │ │
│  │  ├── 输入: Grid, Collision, GBuffer, Particles                        │ │
│  │  └── 输出: Grid.Boundary, Grid.SolidVelocity                          │ │
│  │                                                                        │ │
│  └────────────────────────────────────────────────────────────────────────┘ │
│                                    │                                         │
│                                    ▼                                         │
│  ┌─ 平流阶段 ─────────────────────────────────────────────────────────────┐ │
│  │                                                                        │ │
│  │  Grid2D_AdvectScalar (Density)                                         │ │
│  │  Grid2D_AdvectScalar (Temperature)                                     │ │
│  │  Grid2D_AdvectVelocity                                                 │ │
│  │                                                                        │ │
│  │  方法: Semi-Lagrangian 或 RK2                                          │ │
│  │  公式: q_new = Sample(q_prev, x - u*dt)                               │ │
│  │                                                                        │ │
│  └────────────────────────────────────────────────────────────────────────┘ │
│                                    │                                         │
│                                    ▼                                         │
│  ┌─ 力学阶段 ─────────────────────────────────────────────────────────────┐ │
│  │                                                                        │ │
│  │  Grid2D_ComputeCurl → curl = ∇×u                                      │ │
│  │                                                                        │ │
│  │  Grid2D_VorticityConfinementForce                                      │ │
│  │  └── F = ε * dx * N × ω                                               │ │
│  │                                                                        │ │
│  │  Grid2D_Gas_3DBuoyancy                                                 │ │
│  │  └── F = (ρ_buoy * Density - T_buoy * Temp) * g                       │ │
│  │                                                                        │ │
│  │  Grid2D_TurbulenceForce / WindForce                                    │ │
│  │                                                                        │ │
│  │  Grid2D_IntegrateForces (累积所有力)                                   │ │
│  │  └── Velocity += Σ(Forces) * dt                                       │ │
│  │                                                                        │ │
│  └────────────────────────────────────────────────────────────────────────┘ │
│                                    │                                         │
│                                    ▼                                         │
│  ┌─ 投影阶段 ─────────────────────────────────────────────────────────────┐ │
│  │                                                                        │ │
│  │  Grid2D_ComputeDivergence                                              │ │
│  │  └── div = ∇·u = (Vx_r - Vx_l + Vy_u - Vy_d) / (2*dx)                │ │
│  │                                                                        │ │
│  │  Grid2D_PressureIteration (循环 N 次)                                  │ │
│  │  └── 求解: ∇²p = ρ * div / dt                                         │ │
│  │                                                                        │ │
│  │  Grid2D_ProjectPressure                                                │ │
│  │  └── u_new = u - ∇p * dt / ρ                                          │ │
│  │                                                                        │ │
│  └────────────────────────────────────────────────────────────────────────┘ │
│                                    │                                         │
│                                    ▼                                         │
│  ┌─ 输出阶段 ─────────────────────────────────────────────────────────────┐ │
│  │                                                                        │ │
│  │  Grid2D_ComputeLighting                                                │ │
│  │  └── 计算体积光照                                                       │ │
│  │                                                                        │ │
│  │  Grid2D_Gas_SetRTValues                                                │ │
│  │  └── 写入 RenderTarget (用于材质采样)                                   │ │
│  │                                                                        │ │
│  └────────────────────────────────────────────────────────────────────────┘ │
│                                                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

## 7. Grid2D 属性索引参考

| 索引 | 属性 | 类型 | 用途 |
|------|------|------|------|
| 0 | VelocityX | float | X 方向速度 |
| 1 | VelocityY | float | Y 方向速度 |
| 2 | Density | float | 烟雾密度 |
| 3 | Temperature | float | 温度 |
| 4 | Pressure | float | 压力 |
| 5 | Boundary | int | 边界类型 (0/1/2) |
| 6 | SolidVelocityX | float | 固体速度 X |
| 7 | SolidVelocityY | float | 固体速度 Y |
| 8+ | Custom | varies | 自定义属性 |

---

## 8. 性能优化技巧

### 8.1 分辨率与迭代平衡

```python
def get_optimal_settings(target_fps=60):
    if target_fps >= 60:
        return {"resolution": 192, "iterations": 40}
    elif target_fps >= 30:
        return {"resolution": 256, "iterations": 60}
    else:
        return {"resolution": 384, "iterations": 80}
```

### 8.2 条件编译优化

```hlsl
// 使用 StaticSwitch 实现编译时分支
// 避免运行时开销

if (UseStreaking)  // 编译时决定
{
    // 拖尾代码
}
```

### 8.3 内存访问优化

```hlsl
// 好: 连续访问
for (int y = 0; y < NumCellsY; y++)
    for (int x = 0; x < NumCellsX; x++)
        Grid.GetValue(x, y, ...);

// 差: 跨步访问
for (int x = 0; x < NumCellsX; x++)
    for (int y = 0; y < NumCellsY; y++)
        Grid.GetValue(x, y, ...);
```
