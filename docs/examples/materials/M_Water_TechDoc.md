# M_Water 技术文档

## 概述

| 属性 | 值 |
|------|-----|
| 材质路径 | `/DatasmithContent/Materials/Water/M_Water.M_Water` |
| Blend Mode | Translucent |
| Shading Model | DefaultLit |
| Two Sided | False |
| Material Domain | Surface |

## 架构设计

材质采用 **Material Function 封装** 架构，主材质仅包含一个 `MaterialFunctionCall` 节点，所有逻辑封装在 `MFAttr_Water` 函数中。

### 核心函数

```
M_Water
└── MFAttr_Water (Material Function)
    ├── 深度计算模块
    ├── 流向动画模块
    ├── 颜色混合模块
    ├── 菲涅尔反射模块
    └── 输出合并 (MakeMaterialAttributes)
```

---

## 参数说明

### 暴露参数

| 参数名 | 类型 | 默认值 | 说明 |
|--------|------|--------|------|
| `WaterColor` | Vector | (0.024, 0.125, 0.177) | 水体基础颜色 |
| `MaxDepth (cm)` | Scalar | 200 | 最大深度（厘米） |
| `Turbidity` | Scalar | 1.0 | 浊度系数 |
| `FlowDirection` | Scalar | 0 | 流向角度（度） |
| `FlowSpeed` | Scalar | 0.1 | 流动速度 |

---

## 核心算法

### 1. 深度遍历计算 (Custom HLSL)

```hlsl
// 入口深度和结束深度校正
float entryDepth = PxDepth/FOVcos;
float endDepth = min(ScDepth/FOVcos, entryDepth + maxDepth);

// 底部位置计算
BottomPos = CamPos + (CamRay * (ScDepth/FOVcos));

// 密度计算：在最大深度时密度为1
TraversalLength = endDepth - entryDepth;
float density = saturate(TraversalLength / maxDepth);

// 归一化深度
NormalizedDepth = saturate((SurfacePos - BottomPos).z / maxDepth);

return density;
```

**输入参数**:
- `CamRay`: 相机射线方向
- `FOVcos`: 视场角余弦值（通过 DotProduct 计算）
- `PxDepth`: 像素深度
- `ScDepth`: 场景深度
- `maxDepth`: 最大深度
- `CamPos`: 相机位置
- `SurfacePos`: 表面位置

### 2. 流向动画系统

```
FlowDirection (角度)
    ├── Cosine → X方向
    └── Sine → Y方向
        ↓
    Normalize (方向向量)
        ↓
    × FlowSpeed
        ↓
    × Time
        ↓
    UV偏移
```

### 3. 深度淡入 (DepthFade)

用于处理水面与水下物体的边缘混合：

```
VertexNormalWS
    ↓
Lerp(Normal, UpVector, DepthFade)
    ↓
Saturate
    ↓
深度权重
```

### 4. 菲涅尔反射

```
Fresnel (BaseReflectFraction)
    ↓
OneMinus
    ↓
× 边缘颜色
    ↓
反射强度
```

---

## 场景颜色处理

### 焦散计算

```
SceneColor (RGB)
    ↓
ComponentMask (R, G, B)
    ↓
Add(R+G) → Add(B) → Add(R+G+B)
    ↓
Divide / 4
    ↓
× WaterColor (Tint)
    ↓
焦散贡献
```

### 水下暗化

```
Power(SceneColor, Darkening)
    ↓
Lerp(Original, Darkened, Turbidity)
    ↓
混合结果
```

---

## 节点统计

| 类型 | 数量 |
|------|------|
| MaterialFunctionCall | 7 |
| Multiply | 8 |
| Add | 6 |
| Divide | 5 |
| Lerp | 5 |
| Saturate | 6 |
| SceneColor | 5 |
| ComponentMask | 5 |
| Reroute | 8 |
| 其他 | 67 |
| **总计** | **117** |

---

## 性能考虑

1. **Custom HLSL**: 包含复杂深度计算，GPU开销中等
2. **SceneColor 采样**: 5次场景颜色采样，影响带宽
3. **Translucent Blend**: 需要正确的排序和深度处理

### 优化建议

- 减少 `MaxDepth` 可降低深度计算复杂度
- 降低 `Turbidity` 值可简化混合计算
- 考虑使用 LOD 切换简化版本

---

## 依赖关系

```
M_Water
└── MFAttr_Water
    ├── [嵌套 MaterialFunctionCall x 7]
    └── Custom HLSL 代码
```

---

## 技术要点

1. **FOV 校正**: 使用 `DotProduct(CameraVector, CameraForward)` 计算视角校正
2. **深度校正**: `PxDepth/FOVcos` 消除透视畸变
3. **密度累积**: 基于遍历长度计算水体密度
4. **边缘处理**: Fresnel + OneMinus 实现水面边缘反射
