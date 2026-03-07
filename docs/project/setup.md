# Rendering MCP 项目设置与构建

Setup and maintenance guide for the Rendering MCP project.

## Prerequisites

- Python 3.10+
- C++ compiler (MSVC on Windows)
- Unreal Engine 5.x (推荐 5.7+)
- RenderDoc installed

## Quick Start

```
1. Clone repository
2. Install Python dependencies: pip install -e .
3. Configure MCP server in IDE
4. Build Unreal MCP plugin (if needed)
5. Test connection to RenderDoc and UE
```

## Key Configuration Files

| File | Purpose |
|------|---------|
| `config/mcp_config.example.json` | MCP server configuration template |
| `pyproject.toml` | Python project dependencies |

---

## RenderDoc MCP 更新

### 步骤 1: 关闭 RenderDoc
```bash
taskkill /F /IM qrenderdoc.exe
taskkill /F /IM renderdoccmd.exe
```

### 步骤 2: 安装扩展
```bash
cd d:/CodeBuddy/rendering-mcp
python scripts/renderdoc/install_extension.py
```

### 步骤 3: 启动 RenderDoc
```bash
# 打开指定 capture 文件
start "" "E:\capture.rdc"

# 或正常启动
start qrenderdoc
```

### 步骤 4: 验证扩展加载
1. 打开 `Tools > Manage Extensions`
2. 确保 "RenderDoc MCP Bridge" 已启用

---

## Unreal Engine MCP 更新

### 步骤 1: 关闭 UE 编辑器
```bash
taskkill /F /IM UnrealEditor.exe
```

### 步骤 2: 编译项目

**推荐: 使用 Visual Studio**
1. 打开 `RenderingMCP.sln`
2. 选择 `Development Editor` 配置
3. 右键项目 → `Build`

**或: 启动 UE 编辑器自动编译**
1. 双击 `.uproject` 文件
2. UE 会自动检测并编译修改的插件

> **注意**: 不要自动启动 UE 编辑器，让用户手动编译

### 步骤 3: 验证编译成功
检查编译输出日志，确认无错误

---

## MCP 服务器更新

**重要**: 修改 MCP 服务器代码后，必须重启 MCP 客户端才能生效。

### 方法: 重新加载窗口
在 VS Code / Cursor 中按 `Ctrl+Shift+P`，输入 `Reload Window`

> **注意**: MCP 服务器由 IDE 客户端管理，无法通过命令行自动重启。

---

## 当前可用工具

### RenderDoc MCP (19 tools)
| 工具 | 描述 |
|------|------|
| `get_capture_status` | 检查 capture 是否加载 |
| `get_draw_calls` | 获取所有 draw calls |
| `get_frame_summary` | 获取帧统计信息 |
| `find_draws_by_shader` | 按着色器查找 draw calls |
| `find_draws_by_texture` | 按纹理查找 draw calls |
| `get_draw_call_details` | 获取 draw call 详情 |
| `get_shader_info` | 获取着色器信息 |
| `get_buffer_contents` | 读取 buffer 数据 |
| `get_texture_info` | 获取纹理元数据 |
| `get_texture_data` | 获取纹理像素数据 |
| `save_texture` | 保存纹理到文件 |
| `get_pipeline_state` | 获取完整管线状态 |
| `list_captures` | 列出 .rdc 文件 |
| `open_capture` | 打开 capture 文件 |
| `get_mesh_data` | 获取 mesh 数据 |
| `export_mesh_as_fbx` | 导出 FBX |
| `export_mesh_csv` | 导出 CSV |

### Unreal MCP (26+ tools)
详见 [tools/unreal-mcp-tools.md](../tools/unreal-mcp-tools.md)

---

## 故障排除

### "Method not found" 错误
1. 确保已运行 `python scripts/renderdoc/install_extension.py`
2. 确保 RenderDoc 已完全重启
3. 确保 MCP 客户端已重新加载

### UE 编译失败
1. 确保 UE 5.7 已正确安装
2. 确保 .NET SDK 已安装
3. **推荐**: 直接启动 UE 编辑器让引擎自动编译

### MCP 工具不更新
- **必须**重新加载 MCP 客户端（`Ctrl+Shift+P` → `Reload Window`）
