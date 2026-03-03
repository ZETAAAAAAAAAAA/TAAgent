# Rendering MCP 更新与构建指南

## 完整更新流程

当修改任何代码后，需要按以下步骤更新：

---

## 1. RenderDoc MCP 更新

### 步骤 1: 关闭 RenderDoc
```bash
# Windows
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
# 方式 A: 打开指定 capture 文件
start "" "E:\Control_DX11_-frame82732.rdc"

# 方式 B: 正常启动
start qrenderdoc
```

### 步骤 4: 验证扩展加载
1. 打开 `Tools > Manage Extensions`
2. 确保 "RenderDoc MCP Bridge" 已启用
3. 检查 MCP 连接是否正常

---

## 2. Unreal Engine MCP 更新

### 步骤 1: 关闭 UE 编辑器
```bash
taskkill /F /IM UnrealEditor.exe
taskkill /F /IM UnrealEditor-Win64-DebugGame.exe
```

### 步骤 2: 生成项目文件（可选，首次或添加文件时需要）
```bash
# PowerShell 环境需要用 cmd /c 包装
cmd /c "pushd E:\UE\UE_5.7\Engine\Build\BatchFiles && Build.bat -projectfiles -project=d:\CodeBuddy\rendering-mcp\ue-plugin\RenderingMCP\RenderingMCP.uproject -game -rocket -progress"
```

### 步骤 3: 编译项目（推荐方式：启动 UE 自动编译）
```bash
# 推荐：直接启动 UE 编辑器，会自动编译插件
start "" "E:\UE\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe" "d:\CodeBuddy\rendering-mcp\ue-plugin\RenderingMCP\RenderingMCP.uproject"
```

> **注意**: 命令行编译 Build.bat 在 PowerShell 中可能失败（exit code 6），建议直接启动 UE 编辑器让引擎自动编译。

### 步骤 4: 验证编译成功
UE 编辑器启动后会自动编译 C++ 插件，查看输出日志确认编译成功

---

## 3. MCP 服务器更新

**重要**: 修改 MCP 服务器代码后，必须重启 MCP 客户端才能生效。

### 方法 1: 重新加载窗口（推荐）
在 VS Code / Cursor 中按 `Ctrl+Shift+P`，输入 `Reload Window`

### 方法 2: 重启 IDE/编辑器
关闭并重新打开 VS Code 或其他 MCP 客户端

> **注意**: MCP 服务器由 IDE 客户端管理，无法通过命令行自动重启。尝试 `taskkill python.exe` 会终止所有 Python 进程，可能导致系统问题。

---

## 4. 需要修改的文件列表

### 添加新工具时需要修改：

| 组件 | 文件路径 | 修改内容 |
|------|---------|---------|
| MCP Server | `servers/renderdoc_mcp/mcp_server/server.py` | 添加 `@mcp.tool()` 函数 |
| Request Handler | `renderdoc_extension/request_handler.py` | 添加到 `_methods` 字典和 handler 函数 |
| Facade | `renderdoc_extension/renderdoc_facade.py` | 添加 facade 方法 |
| Service | `renderdoc_extension/services/*.py` | 添加实际实现 |
| UE MCP Server | `servers/unreal_render_mcp/server.py` | 添加工具定义 |
| UE Plugin | `ue-plugin/RenderingMCP/Plugins/UnrealMCP/Source/` | 添加 C++ 实现 |

---

## 5. 快速更新脚本

### PowerShell 脚本（推荐）
```powershell
# 完整更新流程 - 在 PowerShell 中运行

Write-Host "=== 1. 关闭进程 ===" -ForegroundColor Cyan
taskkill /F /IM qrenderdoc.exe 2>$null
taskkill /F /IM renderdoccmd.exe 2>$null
taskkill /F /IM UnrealEditor.exe 2>$null
Write-Host "进程已关闭" -ForegroundColor Green

Write-Host "=== 2. 安装 RenderDoc 扩展 ===" -ForegroundColor Cyan
Set-Location d:\CodeBuddy\rendering-mcp
python scripts/renderdoc/install_extension.py
Write-Host "扩展安装完成" -ForegroundColor Green

Write-Host "=== 3. 生成 UE 项目文件 ===" -ForegroundColor Cyan
cmd /c "pushd E:\UE\UE_5.7\Engine\Build\BatchFiles && Build.bat -projectfiles -project=d:\CodeBuddy\rendering-mcp\ue-plugin\RenderingMCP\RenderingMCP.uproject -game -rocket -progress"
Write-Host "项目文件生成完成" -ForegroundColor Green

Write-Host "=== 4. 启动应用 ===" -ForegroundColor Cyan
# 启动 RenderDoc（打开 capture 文件）
Start-Process "E:\Control_DX11_-frame82732.rdc"
# 启动 UE 编辑器（会自动编译插件）
Start-Process "E:\UE\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe" -ArgumentList "d:\CodeBuddy\rendering-mcp\ue-plugin\RenderingMCP\RenderingMCP.uproject"
Write-Host "应用启动中..." -ForegroundColor Green

Write-Host ""
Write-Host "=== 完成! ===" -ForegroundColor Yellow
Write-Host "请手动执行以下操作:" -ForegroundColor Yellow
Write-Host "1. RenderDoc: Tools > Manage Extensions 确保扩展已启用" -ForegroundColor White
Write-Host "2. UE: 等待编译完成" -ForegroundColor White
Write-Host "3. IDE: Ctrl+Shift+P > Reload Window 重启 MCP" -ForegroundColor White
```

### CMD 批处理脚本
```batch
@echo off
echo === 1. Killing processes ===
taskkill /F /IM qrenderdoc.exe 2>nul
taskkill /F /IM renderdoccmd.exe 2>nul
taskkill /F /IM UnrealEditor.exe 2>nul

echo === 2. Installing RenderDoc extension ===
cd /d d:\CodeBuddy\rendering-mcp
python scripts/renderdoc/install_extension.py

echo === 3. Generating UE project files ===
pushd E:\UE\UE_5.7\Engine\Build\BatchFiles
call Build.bat -projectfiles -project=d:\CodeBuddy\rendering-mcp\ue-plugin\RenderingMCP\RenderingMCP.uproject -game -rocket -progress
popd

echo === 4. Starting applications ===
start "" "E:\Control_DX11_-frame82732.rdc"
start "" "E:\UE\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe" "d:\CodeBuddy\rendering-mcp\ue-plugin\RenderingMCP\RenderingMCP.uproject"

echo === Done! ===
echo Please:
echo   1. RenderDoc: Tools ^> Manage Extensions - enable extension
echo   2. UE: Wait for compilation
echo   3. IDE: Ctrl+Shift+P ^> Reload Window
pause
```

---

## 6. 当前可用工具

### RenderDoc MCP
| 工具名 | 描述 |
|--------|------|
| `get_capture_status` | 检查 capture 是否加载 |
| `get_draw_calls` | 获取所有 draw calls |
| `get_frame_summary` | 获取帧统计信息 |
| `find_draws_by_shader` | 按着色器查找 draw calls |
| `find_draws_by_texture` | 按纹理查找 draw calls |
| `find_draws_by_resource` | 按 resource ID 查找 draw calls |
| `get_draw_call_details` | 获取 draw call 详情 |
| `get_action_timings` | 获取 GPU 计时信息 |
| `get_shader_info` | 获取着色器信息 |
| `get_buffer_contents` | 读取 buffer 数据 |
| `get_texture_info` | 获取纹理元数据 |
| `get_texture_data` | 获取纹理像素数据 |
| `save_texture` | 保存纹理到文件 |
| `get_pipeline_state` | 获取完整管线状态 |
| `list_captures` | 列出目录中的 .rdc 文件 |
| `open_capture` | 打开 capture 文件 |
| `get_mesh_data` | 获取 draw call 的 mesh 数据 |
| `export_mesh_as_fbx` | 导出 mesh 为 FBX 文件 |
| `export_mesh_json` | 导出 mesh 为 JSON (需要重启 MCP) |

### Unreal MCP
| 工具名 | 描述 |
|--------|------|
| `create_material` | 创建新材质 |
| `create_material_instance` | 创建材质实例 |
| `create_material_function` | 创建材质函数 |
| `set_material_instance_parameter` | 设置材质实例参数 |
| `set_material_properties` | 设置材质属性 |
| `compile_material` | 编译材质 |
| `get_material_expressions` | 获取材质表达式列表 |
| `get_material_functions` | 获取材质函数列表 |
| `get_material_function_content` | 获取材质函数内容 |
| `get_available_materials` | 获取可用材质列表 |
| `add_material_expression` | 添加材质表达式节点 |
| `connect_material_nodes` | 连接材质节点 |
| `import_texture` | 导入纹理 |
| `set_texture_properties` | 设置纹理属性 |
| `import_fbx` | 导入 FBX 文件（FBX 需包含 UnitScaleFactor 单位信息） |
| `create_static_mesh_from_data` | 从 JSON 数据创建静态网格 |

---

## 7. 故障排除

### PowerShell vs CMD 环境问题
当前默认 shell 是 PowerShell，但很多命令是 CMD 批处理格式。

**问题表现**:
- `"文件名、目录名或卷标语法不正确"` - 引号在 PowerShell 中处理不同
- `"不是内部或外部命令"` - `pushd` 后无法找到批处理文件
- `exit code 6` - Build.bat 编译失败

**解决方案**:
```powershell
# PowerShell 中执行 CMD 命令，用 cmd /c 包装
cmd /c "你的批处理命令"

# 或者启动 UE 让其自动编译
start "" "E:\UE\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe" "项目路径.uproject"
```

### "Method not found" 错误
1. 确保已运行 `python scripts/renderdoc/install_extension.py`
2. 确保 RenderDoc 已完全重启（不是重新打开 capture）
3. 确保 MCP 客户端已重新加载

### 扩展未加载
1. 检查 `%APPDATA%/qrenderdoc/extensions/renderdoc_mcp_bridge/` 是否存在
2. 在 `Tools > Manage Extensions` 中启用扩展
3. 检查 RenderDoc Python 控制台是否有错误

### UE 编译失败
1. 确保 UE 5.7 已正确安装
2. 确保 .NET SDK 已安装
3. **推荐**: 直接启动 UE 编辑器让引擎自动编译，而非使用命令行 Build.bat

### MCP 工具不更新
- **必须**重新加载 MCP 客户端（`Ctrl+Shift+P` → `Reload Window`）
- MCP 服务器在客户端启动时加载，代码修改不会自动生效
- 无法通过命令行自动重启 MCP 服务器（进程名都是 python.exe，无法区分）
