# Niagara Fluids 系统分析

本文档分析 UE NiagaraFluids 插件中的 Gas 2D 流体系统实现。

---

## 1. 系统概览

### 1.1 分析的系统

| 系统 | 路径 | 用途 |
|------|------|------|
| Grid2D_Gas_Explosion | `/NiagaraFluids/Templates/Gas/2D/Systems/Grid2D_Gas_Explosion` | 爆炸烟雾效果 |
| Grid2D_Gas_MovingFire | `/NiagaraFluids/Templates/Gas/2D/Systems/Grid2D_Gas_MovingFire` | 移动火焰效果 |

### 1.2 Emitter 结构

每个系统包含两个 Emitter：

```
Niagara System
├── SourceParticles          # 源粒子发射器（驱动流体）
│   ├── Spawn Script         # 粒子生成
│   └── Update Script        # 粒子更新 + 散射到 Grid2D
│
└── Grid2D_Gas_SmokeFire_Emitter  # 流体模拟发射器
    ├── Spawn Script         # 初始化 Grid2D、参数设置
    └── Update Script        # 完整流体模拟管线 (400+ nodes)
```

---

## 2. 属性数据流追踪

### 2.1 Grid2D 核心属性

| 属性 | 索引 | 类型 | 作用 |
|------|------|------|------|
| **Velocity** | 0-1 | float2 | 速度场，驱动所有平流和投影 |
| **Density** | 2 | float | 烟雾密度，用于渲染和浮力计算 |
| **Temperature** | 3 | float | 温度，驱动浮力上升 |
| **Pressure** | 4 | float | 压力场，用于投影修正速度 |
| **Divergence** | - | float | 临时存储散度值 |
| **Curl** | - | float | 临时存储旋度值 |
| **Boundary** | 5 | int | 边界标记 (0=流体, 1=固体, 2=空) |
| **SolidVelocity** | 6-7 | float2 | 固体边界速度 |

### 2.2 属性生命周期追踪表

| 阶段 | 模块 | Velocity | Density | Temperature | Pressure | Boundary | SolidVel |
|------|------|:--------:|:-------:|:-----------:|:--------:|:--------:|:--------:|
| **Spawn** | SetResolution | - | - | - | - | - | - |
| | CreateTransform | - | - | - | - | - | - |
| **Input** | ParticleScatter | **W** | **W** | **W** | - | - | - |
| | IntegrateForces | **RW** | - | - | - | - | - |
| | ComputeBoundary | R | R | - | - | **W** | **W** |
| **Advection** | AdvectDensity | **R** | **RW** | - | - | R | - |
| | AdvectTemp | **R** | - | **RW** | - | R | - |
| | AdvectVelocity | **RW** | - | - | - | R | - |
| **Forces** | ComputeCurl | **R** | - | - | - | - | - |
| | VorticityForce | R(Curl) | - | - | - | - | - |
| | Buoyancy | - | **R** | **R** | - | - | - |
| | IntegrateForces | **RW** | - | - | - | - | - |
| **Projection** | ComputeDiv | **R** | - | - | - | R | - |
| | PressureIter | - | - | - | **RW** | **R** | R |
| | ProjectPress | **RW** | - | - | **R** | **R** | R |
| **Output** | ComputeLighting | - | **R** | R | - | - | - |
| | SetRTValues | R | **R** | R | - | - | - |

> **图例**: R = 读取, W = 写入, RW = 读写, - = 不涉及

### 2.3 详细数据流图

```
┌─────────────────────────────────────────────────────────────────────────────────────┐
│                              属性数据流完整追踪                                        │
└─────────────────────────────────────────────────────────────────────────────────────┘

┌─ Spawn Script ──────────────────────────────────────────────────────────────────────┐
│                                                                                      │
│  Grid2D_SetResolution                                                               │
│  ┌─────────────────────────────────────────────────────────────────────┐            │
│  │ 输入: NumCellsX/Y, WorldGridExtents                                  │            │
│  │ 输出: Emitter.Module.NumCellsX/Y, WorldCellSize                      │            │
│  │ 作用: 计算网格分辨率和单元格大小                                        │            │
│  └─────────────────────────────────────────────────────────────────────┘            │
│                                                                                      │
│  Grid2D_CreateUnitToWorldTransform                                                  │
│  ┌─────────────────────────────────────────────────────────────────────┐            │
│  │ 输出: UnitToWorldTransform (矩阵)                                     │            │
│  │ 作用: 创建网格空间→世界空间的变换矩阵                                    │            │
│  └─────────────────────────────────────────────────────────────────────┘            │
│                                                                                      │
└──────────────────────────────────────────────────────────────────────────────────────┘
                                        │
                                        ▼
┌─ Update Script ─────────────────────────────────────────────────────────────────────┐
│                                                                                      │
│  ╔════════════════════════════════════════════════════════════════════════════════╗ │
│  ║  阶段1: 输入 (Input Stage)                                                      ║ │
│  ╠════════════════════════════════════════════════════════════════════════════════╣ │
│  │                                                                                │ │
│  │  Grid2D_Gas_ParticleScatterSource                                              │ │
│  │  ┌────────────────────────────────────────────────────────────────────────┐   │ │
│  │  │ 【写入】                                                                 │   │ │
│  │  │   Velocity  ← Particle.Velocity * Falloff                             │   │ │
│  │  │   Density   ← Particle.Density * Falloff                              │   │ │
│  │  │   Temperature ← Particle.Temperature * Falloff                        │   │ │
│  │  │                                                                         │   │ │
│  │  │ 【数据来源】                                                             │   │ │
│  │  │   源粒子发射器 (SourceParticles Emitter)                                │   │ │
│  │  │   - Position → 计算网格索引                                             │   │ │
│  │  │   - Velocity → 写入 Velocity 属性                                       │   │ │
│  │  │   - Density/Temperature → 写入对应属性                                  │   │ │
│  │  │                                                                         │   │ │
│  │  │ 【处理】                                                                 │   │ │
│  │  │   1. 坐标变换: World → Local                                           │   │ │
│  │  │   2. 计算网格索引: Index = Position / CellSize                         │   │ │
│  │  │   3. 拖尾处理: 快速移动粒子生成多个采样点                                 │   │ │
│  │  │   4. 循环写入: 在粒子半径范围内累加属性值                                 │   │ │
│  │  └────────────────────────────────────────────────────────────────────────┘   │ │
│  │                                                                                │ │
│  │  Grid2D_IntegrateForces (第一次)                                               │ │
│  │  ┌────────────────────────────────────────────────────────────────────────┐   │ │
│  │  │ 【读取】 Transient.PhysicsForce, PhysicsForceWorld                    │   │ │
│  │  │ 【读写】 Velocity                                                     │   │ │
│  │  │                                                                         │   │ │
│  │  │ 公式: Velocity += (PhysicsForce + WorldToLocal^T * PhysicsForceWorld) * dt│
│  │  │                                                                         │   │ │
│  │  │ 注意: 此时 Force 为0，只是初始化                                        │   │ │
│  │  └────────────────────────────────────────────────────────────────────────┘   │ │
│  │                                                                                │ │
│  │  Grid2D_ComputeBoundary                                                        │ │
│  │  ┌────────────────────────────────────────────────────────────────────────┐   │ │
│  │  │ 【读取】 Velocity (检测自由表面)                                       │   │ │
│  │  │ 【写入】 Boundary, SolidVelocity                                      │   │ │
│  │  │                                                                         │   │ │
│  │  │ 边界类型:                                                               │   │ │
│  │  │   0 = FLUID_CELL  (流体单元)                                           │   │ │
│  │  │   1 = SOLID_CELL  (固体边界)                                           │   │ │
│  │  │   2 = EMPTY_CELL  (自由表面)                                           │   │ │
│  │  │                                                                         │   │ │
│  │  │ 检测来源:                                                               │   │ │
│  │  │   - 网格边缘 → SOLID 或 EMPTY                                          │   │ │
│  │  │   - CollisionQuery → SOLID                                             │   │ │
│  │  │   - GBuffer Depth → SOLID (同时获取 SolidVelocity)                     │   │ │
│  │  │   - 粒子碰撞 → SOLID                                                   │   │ │
│  │  │   - 速度为0 → EMPTY (自由表面)                                         │   │ │
│  │  └────────────────────────────────────────────────────────────────────────┘   │ │
│  │                                                                                │ │
│  ╚════════════════════════════════════════════════════════════════════════════════╝ │
│                                        │                                           │
│                                        ▼                                           │
│  ╔════════════════════════════════════════════════════════════════════════════════╗ │
│  ║  阶段2: 平流 (Advection Stage)                                                 ║ │
│  ╠════════════════════════════════════════════════════════════════════════════════╣ │
│  │                                                                                │ │
│  │  Grid2D_AdvectScalar (Density)                                                 │ │
│  │  ┌────────────────────────────────────────────────────────────────────────┐   │ │
│  │  │ 【读取】 Velocity (当前帧) → 计算回溯位置                              │   │ │
│  │  │ 【读取】 Boundary → 跳过固体单元格                                     │   │ │
│  │  │ 【读写】 Density                                                      │   │ │
│  │  │         - 读取: 上一帧 Density (PreviousGrid)                         │   │ │
│  │  │         - 写入: 当前帧 Density                                        │   │ │
│  │  │                                                                         │   │ │
│  │  │ 算法: Semi-Lagrangian 或 RK2                                           │   │ │
│  │  │   1. 回溯: pos_back = pos - Velocity * dt                             │   │ │
│  │  │   2. 采样: Density_new = Sample(Density_prev, pos_back)              │   │ │
│  │  │   3. 衰减: Density_new *= (1 - Dissipation * dt)                     │   │ │
│  │  └────────────────────────────────────────────────────────────────────────┘   │ │
│  │                                                                                │ │
│  │  Grid2D_AdvectScalar (Temperature)                                             │ │
│  │  ┌────────────────────────────────────────────────────────────────────────┐   │ │
│  │  │ 【读取】 Velocity, Boundary                                            │   │ │
│  │  │ 【读写】 Temperature (Previous → Current)                              │   │ │
│  │  │                                                                         │   │ │
│  │  │ 同 Density，使用相同的平流算法                                          │   │ │
│  │  └────────────────────────────────────────────────────────────────────────┘   │ │
│  │                                                                                │ │
│  │  Grid2D_AdvectVelocity                                                        │ │
│  │  ┌────────────────────────────────────────────────────────────────────────┐   │ │
│  │  │ 【读写】 Velocity (Previous → Current)                                 │   │ │
│  │  │ 【读取】 Boundary → 边界条件                                           │   │ │
│  │  │                                                                         │   │ │
│  │  │ 特殊处理:                                                               │   │ │
│  │  │   - 固体边界: 保持固体速度不变                                          │   │ │
│  │  │   - 自由表面: 允许自由流动                                              │   │ │
│  │  └────────────────────────────────────────────────────────────────────────┘   │ │
│  │                                                                                │ │
│  ╚════════════════════════════════════════════════════════════════════════════════╝ │
│                                        │                                           │
│                                        ▼                                           │
│  ╔════════════════════════════════════════════════════════════════════════════════╗ │
│  ║  阶段3: 力学 (Forces Stage)                                                    ║ │
│  ╠════════════════════════════════════════════════════════════════════════════════╣ │
│  │                                                                                │ │
│  │  Grid2D_ComputeCurl                                                           │ │
│  │  ┌────────────────────────────────────────────────────────────────────────┐   │ │
│  │  │ 【读取】 Velocity[邻居] → 计算旋度                                     │   │ │
│  │  │ 【写入】 Curl (临时变量)                                               │   │ │
│  │  │                                                                         │   │ │
│  │  │ 公式: curl = (∂Vy/∂x - ∂Vx/∂y) / (2*dx)                              │   │ │
│  │  │        = (Vy_right - Vy_left - Vx_up + Vx_down) / (2*dx)             │   │ │
│  │  └────────────────────────────────────────────────────────────────────────┘   │ │
│  │                                                                                │ │
│  │  Grid2D_VorticityConfinementForce                                             │ │
│  │  ┌────────────────────────────────────────────────────────────────────────┐   │ │
│  │  │ 【读取】 Curl, GradCurl                                                │   │ │
│  │  │ 【写入】 Transient.PhysicsForce                                        │   │ │
│  │  │                                                                         │   │ │
│  │  │ 公式: F = ε * dx * (N × ω)                                            │   │ │
│  │  │        N = ∇|ω| / |∇|ω||  (涡度梯度方向)                              │   │ │
│  │  │        ε = VorticityConfinement 参数                                  │   │ │
│  │  │                                                                         │   │ │
│  │  │ 效果: 补偿数值耗散，保持涡旋细节                                        │   │ │
│  │  └────────────────────────────────────────────────────────────────────────┘   │ │
│  │                                                                                │ │
│  │  Grid2D_Gas_3DBuoyancy                                                        │ │
│  │  ┌────────────────────────────────────────────────────────────────────────┐   │ │
│  │  │ 【读取】 Density, Temperature                                          │   │ │
│  │  │ 【写入】 Transient.PhysicsForceWorld                                   │   │ │
│  │  │                                                                         │   │ │
│  │  │ 公式: F = (DensityBuoyancy * Density                                  │   │ │
│  │  │              - TemperatureBuoyancy * Temperature) * g                │   │ │
│  │  │                                                                         │   │ │
│  │  │ 效果: 热空气上升，冷空气下降                                            │   │ │
│  │  └────────────────────────────────────────────────────────────────────────┘   │ │
│  │                                                                                │ │
│  │  Grid2D_TurbulenceForce / WindForce (可选)                                    │ │
│  │  ┌────────────────────────────────────────────────────────────────────────┐   │ │
│  │  │ 【写入】 Transient.PhysicsForce / PhysicsForceWorld                   │   │ │
│  │  └────────────────────────────────────────────────────────────────────────┘   │ │
│  │                                                                                │ │
│  │  Grid2D_IntegrateForces (第二次)                                               │ │
│  │  ┌────────────────────────────────────────────────────────────────────────┐   │ │
│  │  │ 【读取】 Transient.PhysicsForce, PhysicsForceWorld                    │   │ │
│  │  │ 【读写】 Velocity                                                     │   │ │
│  │  │                                                                         │   │ │
│  │  │ 公式: Velocity += Force * dt                                          │   │ │
│  │  │                                                                         │   │ │
│  │  │ 此时 Force 包含: Vorticity + Buoyancy + Turbulence + Wind            │   │ │
│  │  └────────────────────────────────────────────────────────────────────────┘   │ │
│  │                                                                                │ │
│  ╚════════════════════════════════════════════════════════════════════════════════╝ │
│                                        │                                           │
│                                        ▼                                           │
│  ╔════════════════════════════════════════════════════════════════════════════════╗ │
│  ║  阶段4: 投影 (Projection Stage) - 速度场散度为零                                ║ │
│  ╠════════════════════════════════════════════════════════════════════════════════╣ │
│  │                                                                                │ │
│  │  Grid2D_ComputeDivergence                                                     │ │
│  │  ┌────────────────────────────────────────────────────────────────────────┐   │ │
│  │  │ 【读取】 Velocity[邻居], Boundary                                      │   │ │
│  │  │ 【写入】 Divergence (临时变量)                                         │   │ │
│  │  │                                                                         │   │ │
│  │  │ 公式: div = (Vx_right - Vx_left + Vy_up - Vy_down) / (2*dx)          │   │ │
│  │  │                                                                         │   │ │
│  │  │ 物理意义: 散度 ≠ 0 表示流体压缩/膨胀                                    │   │ │
│  │  └────────────────────────────────────────────────────────────────────────┘   │ │
│  │                                                                                │ │
│  │  Grid2D_PressureIteration (循环 N 次)                                         │ │
│  │  ┌────────────────────────────────────────────────────────────────────────┐   │ │
│  │  │ 【读取】 Divergence, Boundary, SolidVelocity                           │   │ │
│  │  │ 【读写】 Pressure (迭代更新)                                           │   │ │
│  │  │                                                                         │   │ │
│  │  │ 求解: ∇²p = ρ * div / dt  (泊松方程)                                  │   │ │
│  │  │                                                                         │   │ │
│  │  │ Jacobi 迭代:                                                            │   │ │
│  │  │   p_new = (p_right + p_left + p_up + p_down                           │   │ │
│  │  │           - ρ * dx² * div / dt) / 4                                   │   │ │
│  │  │                                                                         │   │ │
│  │  │ 边界条件:                                                               │   │ │
│  │  │   - SOLID_CELL: 使用固体速度修正边界项                                  │   │ │
│  │  │   - EMPTY_CELL: 压力设为 0 (自由表面)                                  │   │ │
│  │  │                                                                         │   │ │
│  │  │ 迭代次数: 40-100 次 (分辨率越高需要越多)                                 │   │ │
│  │  └────────────────────────────────────────────────────────────────────────┘   │ │
│  │                                                                                │ │
│  │  Grid2D_ProjectPressure                                                       │ │
│  │  ┌────────────────────────────────────────────────────────────────────────┐   │ │
│  │  │ 【读取】 Pressure[邻居], Boundary, SolidVelocity                       │   │ │
│  │  │ 【读写】 Velocity                                                      │   │ │
│  │  │                                                                         │   │ │
│  │  │ 公式: Velocity -= ∇p * dt / ρ                                        │   │ │
│  │  │        ∇p = (p_right - p_left, p_up - p_down) / (2*dx)              │   │ │
│  │  │                                                                         │   │ │
│  │  │ 效果: 投影后速度场散度为零 (不可压缩)                                    │   │ │
│  │  │                                                                         │   │ │
│  │  │ 边界条件:                                                               │   │ │
│  │  │   - SOLID_CELL: Velocity = SolidVelocity                              │   │ │
│  │  │   - EMPTY_CELL: 允许自由流动                                           │   │ │
│  │  └────────────────────────────────────────────────────────────────────────┘   │ │
│  │                                                                                │ │
│  ╚════════════════════════════════════════════════════════════════════════════════╝ │
│                                        │                                           │
│                                        ▼                                           │
│  ╔════════════════════════════════════════════════════════════════════════════════╗ │
│  ║  阶段5: 输出 (Output Stage)                                                    ║ │
│  ╠════════════════════════════════════════════════════════════════════════════════╣ │
│  │                                                                                │ │
│  │  Grid2D_ComputeLighting                                                       │ │
│  │  ┌────────────────────────────────────────────────────────────────────────┐   │ │
│  │  │ 【读取】 Density, Temperature                                          │   │ │
│  │  │ 【写入】 RGB (光照颜色)                                                │   │ │
│  │  │                                                                         │   │ │
│  │  │ 计算:                                                                   │   │ │
│  │  │   1. 光线衰减: extinction = Density * ExtinctionScale                 │   │ │
│  │  │   2. 体积散射: scattering = Density * ScatteringScale                 │   │ │
│  │  │   3. 应用光源方向和颜色                                                 │   │ │
│  │  └────────────────────────────────────────────────────────────────────────┘   │ │
│  │                                                                                │ │
│  │  Grid2D_Gas_SetRTValues                                                       │ │
│  │  ┌────────────────────────────────────────────────────────────────────────┐   │ │
│  │  │ 【读取】 Density, Temperature, Velocity, RGB                           │   │ │
│  │  │ 【写入】 RenderTarget                                                  │   │ │
│  │  │                                                                         │   │ │
│  │  │ 输出到材质:                                                             │   │ │
│  │  │   R = Density (用于不透明度)                                           │   │ │
│  │  │   G = Temperature (用于颜色变化)                                       │   │ │
│  │  │   B = RGB.r (光照结果)                                                 │   │ │
│  │  │   A = RGB.g / 其他                                                     │   │ │
│  │  └────────────────────────────────────────────────────────────────────────┘   │ │
│  │                                                                                │ │
│  ╚════════════════════════════════════════════════════════════════════════════════╝ │
│                                                                                      │
└──────────────────────────────────────────────────────────────────────────────────────┘
```

### 2.4 属性依赖关系图

```
                    ┌─────────────┐
                    │   源粒子     │
                    │ (Position,  │
                    │  Velocity,  │
                    │  Density,   │
                    │  Temp)      │
                    └──────┬──────┘
                           │ Scatter
                           ▼
┌──────────────────────────────────────────────────────────────────────┐
│                          Grid2D 数据空间                              │
│                                                                       │
│   Velocity ◄─────────────────────────────────────────────────────────┤─┐
│      │                                                                │ │
│      ├─► Density ──► (平流) ──► Density'                             │ │
│      │      │                      │                                  │ │
│      │      └──────────────────────┼──► Buoyancy ──► Force           │ │
│      │                             │                                  │ │
│      ├─► Temperature ──► (平流) ──► Temp'                            │ │
│      │      │                      │                                  │ │
│      │      └──────────────────────┼──► Buoyancy ──► Force           │ │
│      │                             │                                  │ │
│      ├─► (旋度计算) ──► Curl ──► Vorticity ──► Force                 │ │
│      │                                                                │ │
│      ├─► (散度计算) ──► Div ──► Pressure (迭代) ──► Pressure'        │ │
│      │                                                │               │ │
│      └────────────────────────────────────────────────┘               │ │
│                           │                                          │ │
│                           ▼                                          │ │
│                    Velocity' ◄── 投影修正 ─────────────────────────────┘ │
│                           │                                            │
│                           ▼                                            │
│                    Boundary ──► 边界条件处理                            │
│                           │                                            │
└───────────────────────────┼────────────────────────────────────────────┘
                            │
                            ▼
                    ┌─────────────┐
                    │ RenderTarget│
                    │  (输出到    │
                    │   材质)     │
                    └─────────────┘
```

---

## 3. 模块参数速查

### 3.1 输入阶段参数

| 模块 | 参数 | 类型 | 默认值 | 效果 |
|------|------|------|--------|------|
| ParticleScatterSource | DensityMult | float | 1.0 | 密度写入倍数 |
| | TemperatureMult | float | 1.0 | 温度写入倍数 |
| | VelocityMult | float | 1.0 | 速度写入倍数 |
| | ParticleRadius | float | 0.1 | 粒子影响半径 |
| | UseStreaking | bool | true | 启用拖尾效果 |
| IntegrateForces | - | - | - | 无额外参数 |
| ComputeBoundary | BorderWidth | int | 2 | 边缘边界宽度 |

### 3.2 平流阶段参数

| 模块 | 参数 | 类型 | 默认值 | 效果 |
|------|------|------|--------|------|
| AdvectScalar | Dissipation | float | 0.0 | 衰减率 (0-1) |
| | AdvectionVelocityMult | float | 1.0 | 平流速度倍数 |
| | InterpolationMethod | int | 0 | 0=线性, 1=三次 |
| | UseRK2 | bool | false | 使用 RK2 方法 |

### 3.3 力学阶段参数

| 模块 | 参数 | 类型 | 默认值 | 效果 |
|------|------|------|--------|------|
| VorticityConfinement | VorticityConfinement | float | 0.0 | 涡度强度 (0-10) |
| 3DBuoyancy | DensityBuoyancy | float | -0.5 | 密度浮力 |
| | TemperatureBuoyancy | float | 1.0 | 温度浮力 |
| | UpVector | float3 | (0,0,1) | 上升方向 |
| TurbulenceForce | TurbulenceStrength | float | 0.0 | 湍流强度 |
| WindForce | WindVelocity | float3 | (0,0,0) | 风速 |

### 3.4 投影阶段参数

| 模块 | 参数 | 类型 | 默认值 | 效果 |
|------|------|------|--------|------|
| PressureIteration | NumIterations | int | 40 | 迭代次数 |
| | PressureRelaxation | float | 1.0 | SOR 松弛因子 |
| | Density | float | 1.0 | 流体密度 |
| ProjectPressure | - | - | - | 无额外参数 |

### 3.5 输出阶段参数

| 模块 | 参数 | 类型 | 默认值 | 效果 |
|------|------|------|--------|------|
| ComputeLighting | LightDirection | float3 | (1,1,1) | 光源方向 |
| | LightColor | float3 | (1,1,1) | 光源颜色 |
| | ExtinctionScale | float | 1.0 | 消光系数 |
| | ScatteringScale | float | 1.0 | 散射系数 |
| SetRTValues | OutputDensity | bool | true | 输出密度 |
| | OutputTemperature | bool | true | 输出温度 |

---

## 4. Grid2D DataInterface 数据结构

### 4.1 属性索引表

| 索引 | 属性名 | 类型 | 读阶段 | 写阶段 |
|------|--------|------|--------|--------|
| 0 | VelocityX | float | 平流、旋度、散度、投影 | 输入、平流、力积分、投影 |
| 1 | VelocityY | float | 平流、旋度、散度、投影 | 输入、平流、力积分、投影 |
| 2 | Density | float | 平流、浮力、光照 | 输入、平流 |
| 3 | Temperature | float | 平流、浮力、光照 | 输入、平流 |
| 4 | Pressure | float | 投影 | 投影迭代 |
| 5 | Boundary | int | 平流、投影 | 边界检测 |
| 6 | SolidVelocityX | float | 投影 | 边界检测 |
| 7 | SolidVelocityY | float | 投影 | 边界检测 |

### 4.2 双缓冲机制

Grid2D 使用双缓冲实现时间积分：

```
Frame N:
┌─────────────────┐     ┌─────────────────┐
│  CurrentData    │     │ DestinationData │
│  (上一帧结果)   │ ──► │  (当前帧结果)   │
└─────────────────┘     └─────────────────┘
         ▲                       │
         │      帧末交换          │
         └───────────────────────┘

平流采样时:
  - 读取 CurrentData (上一帧)
  - 写入 DestinationData (当前帧)
```

### 4.3 HLSL 接口速查

```hlsl
// === 写入接口 ===
Grid2D.SetFloatValue<Attribute="Density">(x, y, value);
Grid2D.SetVector2DValue<Attribute="Velocity">(x, y, float2(vx, vy));

// === 读取接口 ===
Grid2D.GetFloatValue<Attribute="Density">(x, y, value);
Grid2D.GetGridValue(x, y, attributeIndex, value);  // 通用索引访问

// === 上一帧数据 (平流用) ===
Grid2D.SamplePreviousGridAtIndex(unitX, unitY, attrIndex, value);
Grid2D.CubicSamplePreviousGridAtIndex(unitX, unitY, attrIndex, value);  // 三次插值

// === 坐标转换 ===
Grid2D.IndexToUnit(x, y, out unit);  // 网格索引 → 单位坐标
Grid2D.UnitToIndex(unitX, unitY, out x, out y);  // 反向
```

详见 `references/grid2d-datainterface.md`

---

## 5. 美术效果参数映射

### 5.1 效果 → 参数对照表

| 想要的效果 | 调整参数 | 方向 |
|------------|----------|------|
| 烟雾更快消散 | DensityDissipation | ↑ |
| 烟雾上升更快 | TemperatureBuoyancy | ↑ |
| 烟雾下沉 | DensityBuoyancy | ↑ (正值) |
| 更多涡旋细节 | VorticityConfinement | ↑ |
| 更平滑的流动 | VorticityConfinement | ↓ |
| 火焰感更强 | TemperatureMult | ↑ |
| 烟雾更浓 | DensityMult | ↑ |
| 流体更"稠" | PressureIterations | ↑ |

### 5.2 预设模板

**轻烟 (Light Smoke)**:
```
DensityDissipation: 0.02
TemperatureBuoyancy: 0.5
VorticityConfinement: 2.0
PressureIterations: 40
```

**浓烟 (Heavy Smoke)**:
```
DensityDissipation: 0.005
TemperatureBuoyancy: 1.5
VorticityConfinement: 5.0
PressureIterations: 60
```

**火焰 (Fire)**:
```
TemperatureMult: 2.0
TemperatureBuoyancy: 2.0
DensityDissipation: 0.03
TemperatureDissipation: 0.01
```

---

## 6. 常见问题排查

### 6.1 视觉问题

| 问题 | 属性/模块 | 解决方案 |
|------|-----------|----------|
| 烟雾太"飘" | TemperatureBuoyancy 过高 | 降低值 |
| 细节丢失 | VorticityConfinement 过低 | 提高到 3-5 |
| 边界泄漏 | Boundary 未正确设置 | 检查 ComputeBoundary 参数 |
| 烟雾消失太快 | DensityDissipation 过高 | 降低到 0.01 以下 |
| 流体"爆炸" | PressureIterations 过少 | 增加迭代次数 |

### 6.2 性能问题

| 问题 | 原因 | 解决方案 |
|------|------|----------|
| 帧率低 | 分辨率过高 | 降低 NumCells |
| 帧率低 | 迭代过多 | 减少 PressureIterations |
| GPU 负载高 | 网格太大 | 使用 256x256 或更低 |
| 内存占用高 | 多个 Grid2D | 合并或减少属性数量 |

### 6.3 调试工具

```python
# 获取编译后的 HLSL 代码
result = get_niagara_compiled_code(
    asset_path="/Game/MyFluidSystem",
    emitter="Grid2D_Emitter",
    script="update"
)
print(f"GPU Shader length: {result['hlsl_gpu_length']}")
```

---

## 7. 性能优化建议

### 7.1 分辨率与迭代平衡

| 目标帧率 | 分辨率 | 迭代次数 |
|----------|--------|----------|
| 60+ FPS | 192x192 | 40 |
| 30+ FPS | 256x256 | 60 |
| 高质量 | 384x384 | 80 |

### 7.2 性能开销分布

```
投影阶段 (PressureIteration): ~50% 计算时间
平流阶段 (Advect): ~25% 计算时间
力学阶段 (Forces): ~15% 计算时间
其他: ~10%
```

---

## 8. 相关文档

| 文档 | 内容 |
|------|------|
| `fluid-simulation.md` | 理论基础 (Navier-Stokes, Stable Fluids, 投影方法) |
| `fluid-module-details.md` | 模块详细实现 (完整 HLSL 代码、数学推导) |
| `grid2d-datainterface.md` | Grid2D 底层实现 (存储结构、GPU 调度) |
| `niagara-architecture.md` | Niagara 底层架构 (CPU/GPU 模式、VectorVM) |

**UE 源码位置**: `Engine/Plugins/FX/NiagaraFluids/`
