---
name: ue-niagara-workflow
description: |
  Unreal Engine Niagara 粒子系统的分析与制作工作流。
  
  触发场景：
  - 用户想要创建或修改 Niagara 粒子效果
  - 用户想要分析现有 Niagara 系统的结构
  - 用户想要优化粒子性能
  - 用户想要将 Standard 模式转换为 Stateless
---

# UE Niagara 分析与制作

## Overview

本技能覆盖 UE Niagara 粒子系统的完整工作流：
- Niagara 系统分析
- 发射器与模块配置
- 参数调整与优化
- Stateless 转换

---

## 核心原理：Niagara 仿真架构

### CPU vs GPU 模式

Niagara 支持两种仿真目标（Simulation Target）：

```cpp
enum class ENiagaraSimTarget : uint8
{
    CPUSim,         // CPU 仿真，使用 VectorVM 虚拟机
    GPUComputeSim   // GPU 仿真，使用 Compute Shader
};
```

| 特性 | CPU 模式 | GPU 模式 |
|------|----------|----------|
| **执行方式** | VectorVM 解释执行字节码 | Compute Shader 直接执行 |
| **并行度** | 多线程（CPU 核心） | 大规模并行（GPU 线程） |
| **适合场景** | 复杂逻辑、数据接口访问、碰撞检测 | 大量粒子、简单物理模拟 |
| **数据访问** | 可访问任意引擎数据 | 需要通过 Data Interface |
| **限制** | 粒子数量受限 | 不支持所有 Data Interface |

### 编译流程

```
┌─────────────────────────────────────────────────────────────────────┐
│                    Niagara Script 编译流程                          │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  Graph (UNiagaraGraph)                                              │
│       │                                                             │
│       ▼                                                             │
│  HLSL Translator (FNiagaraHlslTranslator)                          │
│       │                                                             │
│       ├─── CPU 模式 ───→ VectorVM Bytecode                         │
│       │                  (FNiagaraVMExecutableData)                 │
│       │                                                             │
│       └─── GPU 模式 ───→ Compute Shader HLSL                       │
│                          → Shader Compiler (DXC)                   │
│                          → GPU Bytecode                            │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### VectorVM 虚拟机

CPU 模式使用 **VectorVM** 作为字节码解释器：

```cpp
// FNiagaraScriptExecutionContext 核心结构
struct FNiagaraScriptExecutionContextBase
{
    UNiagaraScript* Script;
    VectorVM::Runtime::FVectorVMState* VectorVMState;  // VM 状态
    
    // 外部函数表（Data Interface 调用）
    TArray<const FVMExternalFunction*> FunctionTable;
    TArray<void*> UserPtrTable;
    
    // 参数存储
    FNiagaraScriptInstanceParameterStore Parameters;
    
    // 数据集信息
    TArray<FNiagaraDataSetExecutionInfo> DataSetInfo;
};
```

**VectorVM 特点**：
- 基于寄存器的虚拟机
- 支持 SIMD 向量运算（SSE/AVX）
- 字节码在编辑器编译时生成
- 运行时解释执行，无需 JIT

### GPU Compute Shader 执行

GPU 模式使用 Compute Shader 进行并行计算：

```
┌─────────────────────────────────────────────────────────────────────┐
│                    GPU 仿真管线                                      │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  FNiagaraGpuComputeDispatchInterface                               │
│       │                                                             │
│       ▼                                                             │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │  Compute Dispatch                                            │   │
│  │                                                              │   │
│  │  Dispatch(NumParticles / ThreadGroupSize)                   │   │
│  │      │                                                       │   │
│  │      ├──→ Spawn Kernel: 生成新粒子                           │   │
│  │      │                                                       │   │
│  │      ├──→ Update Kernel: 更新粒子状态                        │   │
│  │      │                                                       │   │
│  │      └──→ Event Kernel: 处理事件                             │   │
│  │                                                              │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                                                                     │
│  数据流：                                                           │
│  [Particle Buffer UAV] ←→ [Constant Buffer] ←→ [Data Interface]   │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### ParameterMap：核心数据结构

所有粒子数据存储在 **ParameterMap** 中：

```
┌─────────────────────────────────────────────────────────────────────┐
│                    ParameterMap 结构                                 │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  Particles 命名空间（每个粒子实例）：                                │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │ Particles.Position      → float3[MaxParticles]              │   │
│  │ Particles.Velocity      → float3[MaxParticles]              │   │
│  │ Particles.Color         → float4[MaxParticles]              │   │
│  │ Particles.SpriteSize    → float2[MaxParticles]              │   │
│  │ Particles.Lifetime      → float[MaxParticles]               │   │
│  │ Particles.Age           → float[MaxParticles]               │   │
│  │ Particles.Mass          → float[MaxParticles]               │   │
│  │ Particles.SpriteRotation→ float[MaxParticles]               │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                                                                     │
│  Emitter 命名空间（每个发射器实例）：                                │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │ Emitter.SpawnRate        → float                            │   │
│  │ Emitter.Age              → float                            │   │
│  │ Emitter.SimulationTarget → ENiagaraSimTarget                │   │
│  │ Emitter.RandomSeed       → int32                            │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                                                                     │
│  Engine 命名空间（全局）：                                          │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │ Engine.DeltaTime         → float                            │   │
│  │ Engine.SimTime           → float                            │   │
│  │ Engine.LODLevel          → float                            │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### Module 执行模型

每个 Module 是一个 HLSL 函数，通过 ParameterMap 读写数据：

```hlsl
// CPU 模式：VectorVM 字节码
// GPU 模式：Compute Shader

// Module 函数签名
void ModuleFunction(
    inout FParticleParameterMap ParameterMap,  // 读写粒子数据
    const FConstantParameters Constants,       // 常量参数
    FDataInterfaceParameters DataInterfaces    // 数据接口
)
{
    // 读取: float3 pos = ParameterMap.Position;
    // 计算: pos += Velocity * DeltaTime;
    // 写入: ParameterMap.Position = pos;
}
```

### 数据流示例

以 `OmnidirectionalBurst` 为例：

```
┌──────────────────────────────────────────────────────────────────────┐
│                        Spawn Script (一次性执行)                      │
├──────────────────────────────────────────────────────────────────────┤
│                                                                      │
│  [InputMap: 空]                                                      │
│       │                                                              │
│       ▼                                                              │
│  ┌───────────────────┐                                               │
│  │ InitializeParticle│  写入: Lifetime(1-2.25s), Color(white),      │
│  │                   │        SpriteSize(2-6), Mass(0.4-5.0)        │
│  └───────────────────┘                                               │
│       │                                                              │
│       ▼                                                              │
│  ┌───────────────────┐                                               │
│  │ ShapeLocation     │  写入: Particles.Position                    │
│  │ (Sphere, R=40)    │        在半径40的球体内随机分布               │
│  └───────────────────┘                                               │
│       │                                                              │
│       ▼                                                              │
│  ┌───────────────────┐                                               │
│  │ AddVelocity       │  写入: Particles.Velocity                    │
│  │                   │        向外方向 * 随机速度(75-500)            │
│  └───────────────────┘                                               │
│       │                                                              │
│       ▼                                                              │
│  [OutputMap] → 粒子带着初始属性诞生                                  │
│                                                                      │
└──────────────────────────────────────────────────────────────────────┘

┌──────────────────────────────────────────────────────────────────────┐
│                     Update Script (每帧执行)                          │
├──────────────────────────────────────────────────────────────────────┤
│                                                                      │
│  [InputMap: 当前所有粒子状态]                                        │
│       │                                                              │
│       ▼                                                              │
│  ┌───────────────────┐                                               │
│  │ ParticleState     │  检查: if (Age > Lifetime) Kill()            │
│  │                   │  更新: Age += DeltaTime                      │
│  └───────────────────┘                                               │
│       │                                                              │
│       ▼                                                              │
│  ┌───────────────────┐                                               │
│  │ GravityForce      │  累加: Force += [0,0,-980] * Mass            │
│  └───────────────────┘                                               │
│       │                                                              │
│       ▼                                                              │
│  ┌───────────────────┐                                               │
│  │ Drag              │  累加: Force -= Velocity * Drag(0.75)        │
│  └───────────────────┘                                               │
│       │                                                              │
│       ▼                                                              │
│  ┌───────────────────┐                                               │
│  │ ScaleSpriteSize   │  修改: SpriteSize *= Curve(NormalizedAge)    │
│  └───────────────────┘                                               │
│       │                                                              │
│       ▼                                                              │
│  ┌───────────────────┐                                               │
│  │ ScaleColor        │  修改: Color.a *= Curve(NormalizedAge)       │
│  └───────────────────┘                                               │
│       │                                                              │
│       ▼                                                              │
│  ┌───────────────────────┐                                           │
│  │ SolveForcesAndVelocity│  最终结算:                               │
│  │                       │    Acceleration = Force / Mass           │
│  │                       │    Velocity += Acceleration * dt         │
│  │                       │    Position += Velocity * dt             │
│  │                       │    清空 Force 累加器                     │
│  └───────────────────────┘                                           │
│       │                                                              │
│       ▼                                                              │
│  [OutputMap] → 更新后的粒子状态 → Renderer                          │
│                                                                      │
└──────────────────────────────────────────────────────────────────────┘
```

### Renderer 绑定

Renderer 作为消费者，从 ParameterMap 读取需要的属性：

```
┌─────────────────────────────────────────────────────────────────────┐
│                    NiagaraSpriteRenderer                            │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  绑定的粒子属性:                                                    │
│  ┌─────────────────┐    ┌─────────────────┐                        │
│  │ Particles.      │    │                 │                        │
│  │ Position    ────┼──→ │ Sprite 世界位置 │                        │
│  │ Color       ────┼──→ │ Sprite 顶点颜色 │                        │
│  │ SpriteSize  ────┼──→ │ Sprite 缩放     │                        │
│  │ SpriteRotation ─┼──→ │ Sprite 旋转     │                        │
│  │ MaterialRandom ─┼──→ │ 材质随机值      │                        │
│  └─────────────────┘    └─────────────────┘                        │
│                                                                     │
│  GPU 渲染流程:                                                      │
│  1. 每个粒子生成一个 Billboard (始终面向相机)                        │
│  2. Vertex Shader: Position → Clip Space                           │
│  3. Pixel Shader: 采样材质 * Color                                  │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### 选择 CPU 还是 GPU？

| 场景 | 推荐模式 | 原因 |
|------|----------|------|
| 大量粒子 (>10,000) | GPU | 大规模并行优势 |
| 复杂数据接口访问 | CPU | GPU 有限制 |
| 需要碰撞检测 | CPU | GPU Collision 有额外开销 |
| 简单物理模拟 | GPU | 性能更优 |
| 需要事件系统 | CPU/GPU | 都支持，但 CPU 更灵活 |
| 需要与蓝图交互 | CPU | 更好的数据访问 |

---

## 核心工作流

### 决策树

```
用户目标?
│
├─ 查看现有效果 ──→ get_niagara_emitter + get_niagara_graph
├─ 创建新效果 ──→ update_niagara_emitter (add emitter)
├─ 修改现有效果 ──→ update_niagara_graph / update_niagara_emitter
├─ 性能优化 ──→ 检查 SimTarget + Stateless 转换
└─ Stateless 转换 ──→ update_niagara_emitter (convert_to_stateless)
```

---

## 1. Niagara 系统分析

### 获取 Emitter 详情

```python
get_niagara_emitter(
    asset_path="/Game/Effects/NS_MyEffect",
    detail_level="full",  # 或 "overview"
    emitters=["MainEmitter"],  # 可选，指定发射器
    include=["scripts", "renderers", "parameters", "stateless_analysis"]
)
```

### 获取 Graph 结构

```python
# 方式1: 从 System 的 Emitter 中获取
get_niagara_graph(
    asset_path="/Game/Effects/NS_MyEffect",
    emitter="MainEmitter",
    script="spawn"  # 或 "update"
)

# 方式2: 从独立 Script 资产获取
get_niagara_graph(
    script_path="/Game/Effects/Scripts/NMS_MyScript"
)
```

### 分析输出结构

**Emitter 详情**
- `mode`: "Standard" 或 "Stateless"
- `scripts`: spawn/update 脚本的参数列表
- `renderers`: 渲染器类型和配置
- `stateless_analysis`: Stateless 兼容性检查结果

**Graph 结构**
- `nodes`: 所有节点（FunctionCall, Input, Output, ParameterMapSet）
- `connections`: 节点间的连接关系
- `parameters`: 快速迭代参数列表

### 理解 Niagara 层级

```
Niagara System (NS_)
├── Niagara Emitter (NE_)
│   ├── Spawn Script    (粒子初始化)
│   ├── Update Script   (每帧更新)
│   └── Renderers       (渲染配置)
└── Parameters
    ├── System Parameters
    ├── Emitter Parameters
    └── User Parameters
```

---

## 2. Niagara 修改

### 使用 update_niagara_graph

```python
update_niagara_graph(
    asset_path="/Game/Effects/NS_MyEffect",
    emitter="MainEmitter",
    script="update",
    operations=[
        # 添加模块
        {"action": "add_module", "module_path": "/Niagara/Modules/Update/Forces/CurlNoiseForce"},
        # 移除模块
        {"action": "remove_module", "module_name": "Drag"},
        # 设置参数
        {"action": "set_parameter", "name": "SpawnRate", "value": 100.0},
        # 连接节点
        {"action": "connect", "from_node": "A", "from_pin": "Out", "to_node": "B", "to_pin": "In"},
    ]
)
```

### 使用 update_niagara_emitter

```python
update_niagara_emitter(
    asset_path="/Game/Effects/NS_MyEffect",
    operations=[
        # Emitter 操作
        {"target": "emitter", "name": "MainEmitter", "action": "set_enabled", "value": True},
        {"target": "emitter", "name": "Smoke", "action": "add", "template": "/Niagara/Templates/SimpleSmoke"},
        {"target": "emitter", "name": "OldEmitter", "action": "remove"},
        
        # Stateless 转换 (UE 5.7+)
        {"target": "emitter", "name": "MainEmitter", "action": "convert_to_stateless", "force": False},
        
        # 参数设置
        {"target": "parameter", "emitter": "MainEmitter", "script": "spawn", "name": "SpawnRate", "value": 100.0},
        
        # 渲染器操作
        {"target": "renderer", "emitter": "MainEmitter", "index": 0, "action": "set_enabled", "value": True},
    ]
)
```

### 常用模块路径

| 类别 | 模块路径 |
|------|----------|
| **Spawn** | `/Niagara/Modules/Spawn/Initialization/V2/InitializeParticle` |
| **Location** | `/Niagara/Modules/Spawn/Location/V2/ShapeLocation` |
| **Velocity** | `/Niagara/Modules/Spawn/Velocity/AddVelocity` |
| **Force** | `/Niagara/Modules/Update/Forces/GravityForce` |
| **Force** | `/Niagara/Modules/Update/Forces/Drag` |
| **Force** | `/Niagara/Modules/Update/Forces/CurlNoiseForce` |
| **Color** | `/Niagara/Modules/Update/Color/ScaleColor` |
| **Size** | `/Niagara/Modules/Update/Size/ScaleSpriteSize` |
| **Solver** | `/Niagara/Modules/Solvers/SolveForcesAndVelocity` |

---

## 3. 性能优化

### Stateless 模式 (UE 5.7+)

Stateless 模式减少了运行时开销：

```python
# 检查兼容性
result = get_niagara_emitter(
    asset_path="/Game/Effects/NS_MyEffect",
    emitters=["MainEmitter"],
    include=["stateless_analysis"]
)

# 转换为 Stateless
update_niagara_emitter(
    asset_path="/Game/Effects/NS_MyEffect",
    operations=[
        {"target": "emitter", "name": "MainEmitter", "action": "convert_to_stateless", "force": False}
    ]
)
```

### Stateless 兼容性检查

| 检查项 | 说明 |
|--------|------|
| 无动态参数 | 所有参数必须在编译时确定 |
| 无事件响应 | 不支持事件触发 |
| 无复杂数据接口 | 部分数据接口不支持 |
| 无循环依赖 | 参数间无循环引用 |

---

## 4. 最佳实践

### CPU/GPU 选择

```python
# 大量粒子用 GPU
if particle_count > 10000 and simple_physics:
    sim_target = "GPUComputeSim"
else:
    sim_target = "CPUSim"
```

### 命名规范

| 类型 | 前缀 |
|------|------|
| Niagara System | NS_ |
| Niagara Emitter | NE_ |
| Niagara Script | NMS_ |

### 性能建议

1. **优先 GPU 粒子** - 大量粒子场景
2. **使用 Stateless** - 当不需要动态参数时
3. **控制发射率** - 根据视距调整
4. **合并渲染器** - 减少绘制调用
5. **使用 LOD** - 远距离降低复杂度

---

## 常见问题排查

### Q: 粒子不可见？
A: 检查：
1. 是否有 Render 模块
2. 材质是否正确
3. 粒子大小是否为 0
4. Position 是否在相机视野内

### Q: GPU 模式报错？
A: 检查：
1. 使用的 Data Interface 是否支持 GPU
2. 是否有 GPU 不支持的操作（如某些碰撞检测）
3. 尝试切换到 CPU 模式调试

### Q: 参数不生效？
A: 检查：
1. 参数命名空间是否正确 (Particles.*, Emitter.*, User.*)
2. 参数类型是否匹配
3. 是否被其他模块覆盖

---

## 调试工具

### 获取编译后的 HLSL 代码

```python
# 查看 spawn 脚本的编译后代码
result = get_niagara_compiled_code(
    asset_path="/Game/Effects/NS_Fire",
    emitter="Flame",
    script="spawn"
)
print(result["hlsl_cpu"])  # CPU/VM HLSL
print(result["hlsl_gpu"])  # GPU Compute Shader

# 支持的脚本类型：
# - "spawn": 粒子生成脚本
# - "update": 粒子更新脚本  
# - "gpu_compute": GPU 计算着色器
# - "system_spawn": 系统级生成脚本
# - "system_update": 系统级更新脚本
```

### HLSL 代码分析

编译后的 HLSL 包含：
```hlsl
/*0000*/ // Shader generated by Niagara HLSL Translator
/*0002*/ // SimStage[0] = ParticleSpawnUpdate
/*0003*/ //   NumIterations = 1
/*0004*/ //   ExecuteBehavior = Always
/*0005*/ //   WritesParticles = True

/*0022*/ struct NiagaraEmitterID { int ID; };
/*0027*/ struct NiagaraID { int Index; int AcquireTag; };

/*0033*/ float Engine_WorldDeltaTime;
/*0034*/ float3 Particles_Position[PARTICLE_COUNT];
/*0035*/ float4 Particles_Color[PARTICLE_COUNT];
// ...
```

### 常见调试场景

| 问题 | 检查方法 |
|------|---------|
| 属性未写入 | 查看 HLSL 中是否有 `SET_Particles_XXX` |
| 模块执行顺序 | 查看 SimStage 和函数调用顺序 |
| 性能问题 | 分析 HLSL 中的循环和分支 |
| GPU 编译错误 | 查看 `compile_errors` 数组 |

---

## 相关 Skill

- **renderdoc-reverse-engineering**: 从捕获中分析粒子效果
- **ue-material-workflow**: 创建粒子材质
