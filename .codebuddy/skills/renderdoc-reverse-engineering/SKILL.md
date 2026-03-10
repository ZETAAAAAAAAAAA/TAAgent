---
name: renderdoc-reverse-engineering
description: This skill should be used when analyzing GPU captures from RenderDoc, including reverse engineering rendering techniques, extracting meshes/textures/shaders, or recreating effects in Unreal Engine. Trigger when the user asks about rendering implementation, wants to extract assets from a capture, or needs to understand how a visual effect works.
---

# RenderDoc 逆向工程

## Overview

本技能提供从 GPU 捕获中逆向分析渲染资产的完整工作流，包括：
- 帧分析与性能诊断
- Shader 逆向分析
- 网格提取与导出
- 纹理提取与分析
- 材质重建

## 参考文档

以下参考文档按需加载：

| 文件 | 用途 | 加载时机 |
|------|------|----------|
| `references/mcp-tools-reference.md` | RenderDoc MCP 工具完整 API 参考 | 需要了解具体工具参数和用法时 |
| `references/vertex-formats.md` | 各引擎顶点格式、编码解码、坐标系统 | 导出网格时需要确定属性映射 |
| `references/control-hiss-fluid-analysis.md` | Control 游戏 Hiss 屏幕空间流体效果分析 | 学习屏幕空间流体实现或分析 Control 渲染技术时 |

## 核心工作流

### 决策树

```
用户目标?
│
├─ 分析性能问题 ──→ frame-performance-analysis
├─ 理解渲染技术 ──→ shader-analysis
├─ 提取模型资产 ──→ mesh-extraction
├─ 提取纹理资产 ──→ texture-extraction
└─ 重建完整效果 ──→ material-reconstruction
```

---

## 1. 帧性能分析

### 工作流程

1. **获取帧概览**
   ```
   get_frame_summary() → 了解 API 类型、绘制调用统计、资源数量
   ```

2. **定位性能热点**
   ```
   get_action_timings() → 找出耗时最长的 Draw Call / Dispatch
   ```

3. **分析热点原因**
   - 高顶点数？→ 几何处理瓶颈
   - 高像素覆盖？→ 片段着色瓶颈
   - 大量纹理采样？→ 带宽瓶颈
   - 复杂 Shader？→ ALU 瓶颈

4. **生成诊断报告**
   - 瓶颈类型
   - 影响范围
   - 优化建议

### 关键指标解读

| 指标 | 正常范围 | 异常信号 |
|------|----------|----------|
| Draw Calls/帧 | < 2000 | > 5000 |
| 顶点数/帧 | < 1M | > 10M |
| 纹理带宽 | < 2GB | > 8GB |
| Shader 指令数 | < 500 | > 2000 |

---

## 2. Shader 逆向分析

### 工作流程

1. **定位目标 Shader**
   ```
   find_draws_by_shader(shader_name) → 获取使用该 Shader 的 Draw Call
   ```

2. **获取 Shader 信息**
   ```
   get_shader_info(event_id, stage) → 获取反汇编代码、常量缓冲区、资源绑定
   ```

3. **分析 Shader 逻辑**
   - 识别输入/输出
   - 分析算法模式（光照、PBR、后处理等）
   - 提取关键参数

4. **理解数据流**
   - Constant Buffer → 全局参数
   - Texture → 采样逻辑
   - Sampler → 过滤模式

### DXBC 指令模式识别

| 指令模式 | 可能用途 |
|----------|----------|
| `mul`, `mad` | 矩阵变换、向量运算 |
| `dp3`, `dp4` | 点积、光照计算 |
| `sample`, `sample_l` | 纹理采样 |
| `rcp`, `rsq` | 归一化、菲涅尔 |
| `lerp` | 线性插值、混合 |

---

## 3. 网格提取

### 工作流程

1. **定位目标 Draw Call**
   ```
   get_draw_calls() → 浏览绘制调用列表
   ```

2. **获取网格数据**
   ```
   get_mesh_data(event_id) → 获取顶点位置、法线、UV、切线、索引
   ```

3. **分析顶点格式**
   ```
   export_mesh_csv(event_id) → 导出 CSV 进行详细分析
   ```

4. **确定属性映射**
   - 位置数据：vs_input:ATTRIBUTE0 还是 vs_output:POSITION?
   - 法线数据：是否压缩编码？
   - UV 数据：哪个 TEXCOORD？
   - 切线数据：是否有符号问题？

5. **导出 FBX**
   ```
   export_mesh_as_fbx(event_id, output_path, attribute_mapping, decode)
   ```

### 常见顶点格式

| 引擎 | 位置 | 法线 | UV | 切线 |
|------|------|------|-----|------|
| UE5 | vs_input:ATTRIBUTE0 | buffer (压缩) | vs_output | buffer |
| Unity | vs_input:POSITION | vs_input:NORMAL | vs_input:TEXCOORD0 | vs_input:TANGENT |
| 自定义 | 需分析 | 需分析 | 需分析 | 需分析 |

### 解码表达式

```python
# UE5 压缩法线
"NORMAL": "normalize(itof(x) / 32768)"

# [0,1] 到 [-1,1]
"TANGENT": "x * 2 - 1"

# 组合解码
"NORMAL": "normalize(itof(x) / 32768), x * 2 - 1"
```

---

## 4. 纹理提取

### 工作流程

1. **定位目标纹理**
   ```
   find_draws_by_texture(texture_name) → 找到使用该纹理的 Draw Call
   get_pipeline_state(event_id) → 查看绑定的纹理资源
   ```

2. **获取纹理信息**
   ```
   get_texture_info(resource_id) → 尺寸、格式、Mip 级别
   ```

3. **提取纹理数据**
   ```
   get_texture_data(resource_id, mip, slice) → 获取像素数据
   save_texture(resource_id, output_path, file_type) → 保存为图像文件
   ```

4. **纹理类型识别**
   - 漫反射：通常为 sRGB
   - 法线贴图：BC5/BC3 压缩，XY 存储
   - 粗糙度/金属度：灰度或打包格式
   - HDR 环境图：BC6H 或 Float 格式

### 纹理格式转换

| 原格式 | 保存格式 | 说明 |
|--------|----------|------|
| BC1-BC7 | PNG/TGA | 自动解压 |
| BC6H/BC7 | DDS | 保留压缩 |
| Float | HDR/EXR | 保留 HDR |
| Depth | PNG | 归一化显示 |

---

## 5. 材质重建

### 完整工作流

```
1. 帧分析 → 定位目标 Draw Call
        ↓
2. Shader 分析 → 理解渲染算法
        ↓
3. 纹理提取 → 获取贴图资源
        ↓
4. 参数提取 → 从 Constant Buffer 获取参数
        ↓
5. UE 材质构建 → 使用 ue-material-workflow skill
        ↓
6. 验证效果 → 对比截图
```

### 参数提取策略

| 参数类型 | 来源 | 示例 |
|----------|------|------|
| 全局参数 | Constant Buffer | 光照方向、颜色 |
| 材质参数 | Constant Buffer | 粗糙度、金属度 |
| 纹理参数 | Resource Binding | 各类贴图槽位 |
| 实例参数 | Instance Buffer | 变换矩阵 |

---

## 常见问题排查

### Q: 网格导出后法线反向？
A: 设置 `flip_winding_order: true`

### Q: 纹理提取出来是黑色的？
A: 检查是否为深度纹理或 HDR 纹理，尝试不同保存格式

### Q: Shader 反汇编看不懂？
A: 重点关注输入输出关系，而非逐行分析指令

### Q: 找不到目标 Draw Call？
A: 使用 `marker_filter` 过滤特定渲染阶段

---

## 相关 Skill

- **ue-material-workflow**: 重建材质后在 UE 中构建材质图
- **ue-niagara-workflow**: 如需重建粒子效果
