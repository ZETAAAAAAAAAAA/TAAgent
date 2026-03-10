---
name: ue-niagara-workflow
description: This skill should be used when working with Unreal Engine Niagara particle systems, including creating or modifying particle effects, analyzing existing Niagara systems, optimizing particle performance, or converting Standard mode to Stateless. Trigger when the user needs to configure emitters, add modules, or debug particle behavior.
---

# UE Niagara 分析与制作

## Overview

本技能覆盖 UE Niagara 粒子系统的完整工作流：
- Niagara 系统分析
- 发射器与模块配置
- 参数调整与优化
- Stateless 转换

## 参考文档

以下参考文档按需加载：

| 文件 | 用途 | 加载时机 |
|------|------|----------|
| `references/engine-modules.md` | UE5.7 引擎 Niagara 模块完整列表 (270+) | 需要查找模块路径时 |
| `references/common-modules.md` | 常用模块详解 (参数、属性、实现原理) | 需要了解模块参数、编写模块配置时 |
| `references/niagara-architecture.md` | 底层架构原理 (CPU/GPU模式、VectorVM、ParameterMap) | 需要深入理解底层实现或排查复杂问题时 |
| `references/fluid-systems-analysis.md` | Niagara Fluids 系统分析 (属性数据流、模块参数、美术效果) | 分析或创建流体效果时 |
| `references/fluid-module-details.md` | 流体模块详细实现 (每个模块的HLSL代码、数学推导、数据流图) | 深入理解模块内部实现时 |
| `references/grid2d-datainterface.md` | Grid2D DataInterface 实现原理 (数据结构、GPU存储、HLSL接口) | 深入理解 Grid2D 底层实现时 |

---

## 核心原理

详细架构原理请参考 `references/niagara-architecture.md`。

### CPU vs GPU 模式选择

| 场景 | 推荐模式 | 原因 |
|------|----------|------|
| 大量粒子 (>10,000) | GPU | 大规模并行优势 |
| 复杂数据接口访问 | CPU | GPU 有限制 |
| 需要碰撞检测 | CPU | GPU Collision 有额外开销 |
| 简单物理模拟 | GPU | 性能更优 |
| 需要与蓝图交互 | CPU | 更好的数据访问 |

### Module 执行顺序

Update Script 中的模块有依赖顺序：

```
ParticleState → Forces → Drag → Scale* → SolveForcesAndVelocity
```

**关键**: `SolveForcesAndVelocity` 必须是最后一个模块！

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

### 获取运行时粒子属性

```python
# 从场景中的 Niagara 组件获取粒子数据
result = get_niagara_particle_attributes(
    component_name="NiagaraComponent",  # 组件名称
    emitter="Flame",                    # 可选：指定 emitter
    frame=0                             # 帧索引（默认当前帧）
)

# 返回结构
{
    "success": true,
    "component_name": "NiagaraComponent",
    "num_frames": 1,
    "emitters": [
        {
            "name": "Flame",
            "particle_count": 100,
            "attributes": [
                {"name": "Position", "float_count": 3},
                {"name": "Color", "float_count": 4},
                {"name": "Lifetime", "float_count": 1},
                {"name": "Age", "float_count": 1},
                {"name": "Velocity", "float_count": 3}
            ],
            "particles": [
                {
                    "index": 0,
                    "position": [10.5, 20.3, 5.0],
                    "color": [1.0, 0.8, 0.2, 1.0],
                    "lifetime": 2.5,
                    "age": 0.5,
                    "velocity": [1.0, 2.0, 0.5]
                },
                // ... 更多粒子
            ]
        }
    ]
}
```

### Attribute Spreadsheet 使用场景

| 场景 | 用法 |
|------|------|
| 调试粒子位置 | 检查 Position 属性是否在预期范围 |
| 验证生命周期 | 对比 Age 和 Lifetime 检查消亡逻辑 |
| 性能分析 | 查看 particle_count 识别峰值 |
| 效果调优 | 检查 Color、Size 等渲染属性 |

---

## 相关 Skill

- **renderdoc-reverse-engineering**: 从捕获中分析粒子效果
- **ue-material-workflow**: 创建粒子材质
