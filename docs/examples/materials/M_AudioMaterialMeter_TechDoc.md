# M_AudioMaterialMeter 技术文档

## 概述

`M_AudioMaterialMeter` 是 UE 引擎内置的音频可视化材质，用于 Slate UI 中的音频电平表显示。该材质通过参数化控制实现动态的音频计量可视化效果。

## 材质路径

```
/Script/Engine.Material'/AudioWidgets/AudioMaterialSlate/M_AudioMaterialMeter.M_AudioMaterialMeter'
```

## 节点统计

| 类型 | 数量 |
|------|------|
| 总节点数 | 33 |
| 参数节点 | 10 |
| 运算节点 | 23 |

## 参数列表

### 颜色参数 (VectorParameter)

| 参数名 | 默认值 (RGBA) | 用途 |
|--------|---------------|------|
| `A (V3)` | (0, 0.187, 0.602, 0) | 低电平颜色 (蓝色) |
| `B (V3)` | (0.529, 0.245, 0, 0) | 中电平颜色 (橙色) |
| `C (V3)` | (2, 0.064, 0, 0) | 高电平颜色 (红色，HDR) |
| `DotsOffColor` | (0, 0, 0, 1) | 点阵关闭状态颜色 (黑色) |

### 标量参数 (ScalarParameter)

| 参数名 | 默认值 | 用途 |
|--------|--------|------|
| `VALUE` | 1.0 | 当前电平值 (0-1) |
| `NumLinesX (S)` | 10 | 水平线条/分段数量 |
| `Rotation Angle (0-1) (S)` | 0 | UV 旋转角度 |
| `GradientPow` | 2.303 | 颜色渐变幂次 |
| `GlowAmt` | 2.519 | 发光强度 |
| `GlowMin (S)` | 0 | 发光最小值 |
| `GlowMax (S)` | 1 | 发光最大值 |

## 核心技术实现

### 1. UV 处理流程

```
TextureCoordinate → MaterialFunctionCall(Rotation) → Multiply → Ceil → ...
```

- 使用 `TextureCoordinate` 生成基础 UV
- 通过材质函数调用实现 UV 旋转
- `Ceil` 节点用于量化/分段处理

### 2. 颜色渐变系统

```
A, B, C (颜色参数) → LinearInterpolate (多层混合)
                     ↑
                  Power(Clamp(...))
```

- 三层颜色插值 (A→B→C)
- `GradientPow` 控制渐变过渡曲线
- 支持HDR输出 (颜色C的R通道为2)

### 3. 发光效果

```
VALUE → Multiply → MaterialFunctionCall → Add → 最终发光
          ↑
       GlowAmt
```

- `GlowAmt` 控制发光强度
- `GlowMin/GlowMax` 控制发光范围
- 使用材质函数处理发光逻辑

### 4. 分段显示逻辑

```
NumLinesX → Divide → If(条件判断) → ...
```

- `If` 节点实现分段阈值判断
- 与 `VALUE` 参数配合控制显示区域

## 节点位置分析

| X 坐标范围 | 功能区域 |
|------------|----------|
| -2400 ~ -1900 | UV 生成与旋转 |
| -1900 ~ -1300 | 分段处理 |
| -1300 ~ -600 | 颜色计算核心 |
| -600 ~ 0 | 发光计算 |
| 0 ~ 500 | 最终混合输出 |

## 技术要点

### HDR 支持
- 颜色 C 的红色通道值为 2.0，支持 HDR 输出
- 适用于需要高动态范围的音频可视化场景

### 旋转支持
- 内置 UV 旋转功能
- 通过 `Rotation Angle` 参数控制，范围为 0-1

### 性能优化
- 使用 `Ceil` 替代复杂分段逻辑
- 材质函数封装复用逻辑

## 已知限制

1. **节点连接信息缺失**: 当前无法通过 MCP 获取完整节点连接图
2. **材质函数内容未知**: `MaterialFunctionCall` 调用的函数内容需手动查看
3. **输出属性未知**: 无法确定材质的 BlendMode、ShadingModel 等属性

## 依赖项

该材质依赖以下材质函数（具体函数名需在编辑器中查看）:
- UV 旋转函数
- 发光处理函数
- 可能的其他工具函数

## 相关材质

- 同目录下的其他音频可视化材质
- Slate UI 材质系统

---

*文档生成时间: 2026-03-05*
*生成方式: MCP 自动分析*
