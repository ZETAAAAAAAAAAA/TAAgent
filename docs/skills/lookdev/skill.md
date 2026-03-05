# Lookdev (Look Development) Skill

Lookdev 是资产检查和材质验证的核心工作流程，目标是创建统一的预览环境，确保资产在不同环境下表现一致。

## 核心原则

### 1. 曝光准确
- **EV100=0** 作为基准亮度
- 使用灰板测光：Linear 0.18（中灰，sRGB 0.46/119）
- 关闭自动曝光，使用手动曝光
- 灰板 Final Color 应达到 0.18 左右

### 2. 光比控制
光比决定材质的结构、纹理、质感表现：

| 光比类型 | 对比度 | 适用场景 | 明暗比 |
|---------|--------|---------|--------|
| 低反差 | 1.65x (0.7档) | 阴天 | 检查灰阶、色彩、明度 |
| 中反差 | 2.6x (1.5档) | 多云 | 兼顾质感表现 |
| 高反差 | 16x (4档) | 晴朗 | 极端情况检查、质感强化 |

### 3. 中性光源
- 色温：6500K（中性白）
- 避免偏色影响材质色彩定义

## 环境搭建

### 测光对象设置

```
灰板材质参数：
- Shading Model: Default Lit
- BaseColor: 0.18 (Linear) / 0.46 (sRGB)
- Specular: 0（关闭高光）
- Roughness: 0.5

放置方式：面朝上，水平放置
```

### 后期处理设置

```
必须关闭（影响测光）：
- Auto Exposure → 关闭
- Bloom → 关闭
- Vignette → 关闭
- SSAO → 关闭
- SSR → 关闭

曝光参数：
- Exposure Compensation: 0
- Min/Max Brightness: 相同值（锁定曝光）
- EV100: 0
```

### 项目设置

```
关闭 Pre Exposure：
Project Settings → Engine → Rendering → PostProcessing → 
  ExtendDefaultLuminanceRange = false
```

## 灯光布置流程

### 1. 设置曝光基准
```
Post Process Volume:
- Exposure Mode: Manual
- Exposure Compensation: 0 (EV100=0)
```

### 2. 调整环境光（天光/IBL）
```
1. 关闭直射光
2. 使用 Pixel Inspector 吸取灰板阴影区域
3. 调整天光强度，使阴影区达到目标亮度

目标亮度计算：
- 低反差: 0.18 / 1.65 ≈ 0.109
- 中反差: 0.18 / 2.6 ≈ 0.069
- 高反差: 0.18 / 16 ≈ 0.011
```

### 3. 调整直射光
```
1. 开启直射光
2. 使用 Pixel Inspector 吸取灰板亮部区域
3. 调整直射光强度，使亮部达到 0.18
```

## UE MCP 工具使用

> **架构升级**: 现在使用 **5 个通用反射工具** 管理所有 Actor，无需为每种类型写硬编码。
> - 新增 Actor 类型只需更新本文档，**零代码修改**
> - 属性匹配采用"匹配即修改，不匹配即忽略"策略

### 通用 Actor 管理工具（5个核心）

```python
# 1. 创建任意 Actor
spawn_actor(
    actor_class="DirectionalLight",  # 类名（自动匹配A/U前缀）
    name="KeyLight",                 # 可选：指定名称
    location={"x": 0, "y": 0, "z": 0},
    rotation={"pitch": -45, "yaw": 30, "roll": 0},
    scale={"x": 1, "y": 1, "z": 1},
    properties={                     # 可选：初始化属性
        "intensity": 10.0,
        "light_color": [1.0, 1.0, 1.0, 1.0],
        "bCastShadows": True
    }
)

# 2. 修改 Actor 属性（反射自动匹配）
set_actor_properties(
    name="KeyLight",
    properties={
        # 匹配即修改，不匹配则忽略
        "intensity": 15.0,           # 匹配 LightComponent.intensity
        "temperature": 6500,         # 匹配 LightComponent.temperature  
        "foo_bar": 123               # 不匹配任何属性，自动忽略
    }
)

# 3. 获取 Actor 属性
props = get_actor_properties(name="KeyLight")
# 返回: {name, class, location, rotation, scale, properties: {...}}

# 4. 列出 Actor（支持类过滤）
actors = get_actors()                           # 所有 Actor
actors = get_actors(actor_class="Light")        # 仅灯光
actors = get_actors(actor_class="StaticMesh")   # 仅静态网格
actors = get_actors(detailed=True)              # 包含完整属性

# 5. 删除 Actor
delete_actor(name="KeyLight")
```

### Actor 类型配置表

| Actor 类名 | 说明 | 常用属性 |
|-----------|------|---------|
| `DirectionalLight` | 平行光（主光源） | `intensity`, `light_color`, `temperature`, `bCastShadows`, `source_radius` |
| `PointLight` | 点光源 | `intensity`, `light_color`, `attenuation_radius`, `source_radius` |
| `SpotLight` | 聚光灯 | `intensity`, `inner_cone_angle`, `outer_cone_angle` |
| `RectLight` | 矩形光源 | `intensity`, `source_width`, `source_height` |
| `StaticMeshActor` | 静态网格 | `static_mesh` (路径), `material` (材质路径) |
| `Sphere` | 材质球（自动加载Sphere网格） | 同 StaticMeshActor |
| `Cube` / `Box` | 立方体 | 同 StaticMeshActor |
| `Plane` | 平面（灰板） | 同 StaticMeshActor |
| `Cylinder` | 圆柱体 | 同 StaticMeshActor |
| `PostProcessVolume` | 后处理体积（自动unbound） | `bEnabled`, 详见下方配置 |

### 完整 Lookdev 环境搭建流程

```python
# Step 1: 创建 Post Process Volume（曝光基准）
spawn_actor(
    actor_class="PostProcessVolume",
    name="Lookdev_PP",
    location={"x": 0, "y": 0, "z": 0},
    scale={"x": 2000, "y": 2000, "z": 2000},
    properties={
        "bEnabled": True,
        "bUnbound": True  # 影响整个场景
    }
)

# Step 2: 设置曝光参数（EV100=0）
# 注意：PPV 的详细设置需要通过 Actor 属性访问，或者手动在编辑器设置

# Step 3: 创建材质球
spawn_actor(
    actor_class="Sphere",
    name="MaterialBall",
    location={"x": 0, "y": 0, "z": 100},
    scale={"x": 1, "y": 1, "z": 1}
)

# Step 4: 创建灰板（Linear 0.18 测光用）
spawn_actor(
    actor_class="Plane",
    name="GrayCard",
    location={"x": 200, "y": 0, "z": 0},
    rotation={"pitch": 0, "yaw": 0, "roll": 0},
    scale={"x": 2, "y": 2, "z": 1}
)

# Step 5: 创建三种光比环境的主光源
# 低反差 1.65x
spawn_actor(
    actor_class="DirectionalLight",
    name="KeyLight_Low",
    rotation={"pitch": -45, "yaw": 30, "roll": 0},
    properties={"intensity": 3.0, "bCastShadows": True}
)

# 中反差 2.6x  
spawn_actor(
    actor_class="DirectionalLight",
    name="KeyLight_Mid",
    rotation={"pitch": -45, "yaw": 30, "roll": 0},
    properties={"intensity": 5.0, "bCastShadows": True}
)

# 高反差 16x
spawn_actor(
    actor_class="DirectionalLight",
    name="KeyLight_High",
    rotation={"pitch": -45, "yaw": 30, "roll": 0},
    properties={"intensity": 16.0, "bCastShadows": True}
)

# Step 6: 验证截图
get_viewport_screenshot(
    output_path="C:/Lookdev/verification.png",
    format="png"
)

# Step 7: 使用 Pixel Inspector 检查灰板亮度应为 0.18（Final Color）
```

### 属性设置说明

反射系统会自动搜索 Actor 及其 Component 的属性：

```python
# 灯光属性示例
set_actor_properties(
    name="MyLight",
    properties={
        # 这些属性在 LightComponent 上
        "intensity": 10.0,              # float
        "light_color": [1, 0.9, 0.8, 1], # LinearColor [R,G,B,A]
        "temperature": 6500,            # float
        "bCastShadows": True,           # bool
        "attenuation_radius": 1000,     # float (PointLight/SpotLight)
        "outer_cone_angle": 45.0,       # float (SpotLight)
    }
)

# 静态网格属性示例
set_actor_properties(
    name="MyMesh",
    properties={
        "static_mesh": "/Engine/BasicShapes/Sphere",  # 网格路径
        # "material" 属性暂不支持动态设置，需使用材质工具
    }
)

# Transform 快捷属性（直接作用于 Actor）
set_actor_properties(
    name="AnyActor",
    properties={
        "location": {"x": 100, "y": 0, "z": 50},
        # 或使用数组: "location": [100, 0, 50]
        "rotation": {"pitch": 0, "yaw": 90, "roll": 0},
        "scale": {"x": 2, "y": 2, "z": 2}
    }
)
```

### 获取视口截图验证

```python
# 捕获当前视口
result = get_viewport_screenshot(
    output_path="C:/temp/lookdev_verify.png",
    format="png",      # png, jpg, bmp
    quality=95         # JPEG 质量 (1-100)
)
```

### 测光验证

```python
# 获取灰板属性检查
props = get_actor_properties(name="GrayCard")
print(props["location"])  # 确认位置

# 使用 Pixel Inspector（手动操作）
# 1. 在 UE 编辑器中打开 Window > Developer Tools > Pixel Inspector
# 2. 吸取灰板区域
# 3. 检查 Final Color 应接近 0.18
```

### 遗留工具（已弃用）

以下工具仍可工作，但建议迁移到通用工具：

| 旧工具 | 替代方案 |
|-------|---------|
| `create_light` | `spawn_actor(actor_class="DirectionalLight")` |
| `set_light_properties` | `set_actor_properties` |
| `get_lights` | `get_actors(actor_class="Light")` |
| `delete_light` | `delete_actor` |
| `create_post_process_volume` | `spawn_actor(actor_class="PostProcessVolume")` |
| `set_post_process_settings` | `set_actor_properties` |
| `spawn_basic_actor` | `spawn_actor` |
| `set_actor_material` | `set_actor_properties` |

## DCC 同步

### Substance Painter 设置

```
显示设置：
- Environment: 使用相同 HDRI
- Environment Rotation: 270°
- EV: -0.45
- Temporal AA: 开启

Shader 设置：
- Tonemapping: ACES

色彩管理（7.4+）：
- 启用 OCIO
- Post Effects Exposure: 1.45（补偿 UE 的 0.45 Gain）
```

### UE 对应设置

```
关闭差异项：
- Directional Light: 关闭（仅用 IBL）
- Environment Color: 0.04
- Bloom, Vignette, AO, SSR: 关闭

目的：最小化差异，以 UE 为准
```

## 非PBR材质适配

### Eye Adaptation 材质节点

用于自发光材质、特效材质等非PBR资产：

```
原理：以 EV100=0 为基准 Bias，自动适应不同亮度环境

使用场景：
- 自发光材质
- 特效材质
- 自定义 Shading Model

效果：在不同 EV 环境下保持恒定"观感"
```

## 常见问题排查

| 问题 | 可能原因 | 解决方案 |
|-----|---------|---------|
| 材质照不亮 | 光比过大、环境光不足 | 增加天光/IBL 强度 |
| 过曝 | 曝光补偿过高 | 检查 EV100 和 Exposure Compensation |
| 对比过强 | LUT/Tonemapping 极端 | 使用渐变灰板检查 |
| 显色性低 | 光源色温偏移 | 使用 6500K 中性光 |
| 质感不佳 | 反射源动态不足 | 增加环境反射变化 |

## 资产色彩范围

```
sRGB 安全范围：38-238（经验值）
Linear 对应：约 0.02-0.85

超出范围风险：
- < 38: 易死黑
- > 238: 易过曝
```

## 工作流总结

```
1. 搭建环境
   └── 创建灰板 → 设置曝光 → 布置灯光

2. 验证环境
   └── Pixel Inspector 测光 → 确认 EV100=0 → 确认光比

3. 检查资产
   └── 三种光比切换 → 检查灰阶/色彩/质感 → 记录问题

4. DCC 同步
   └── 匹配设置 → 快速迭代 → 以 UE 为准
```

## 文件保存

### 作为地图
```
优点：光源丰富、可多资产对比
方式：Save Current Level as Template
路径：Content/Maps/Templates/
```

### 作为 Preview Profile
```
优点：任何编辑器可用
方式：Editor Preferences → Preview Scene Settings → Save as Profile
共享：勾选 Shared Profile，同步 Editor.ini
```

## 参考资源

- [UE Lookdev 官方博客](https://www.unrealengine.com/zh-CN/tech-blog/a-few-tips-for-building-unified-assets-reviewing-enviroment)
- [HDRI Haven](https://hdri-haven.com) - 高质量 HDRI 资源
- Gray Card Linear: 0.18 / sRGB: 119,119,119
