# RenderDoc MCP Server

RenderDoc MCP 是一个独立的MCP服务器，用于与RenderDoc进行交互。

## 安装

RenderDoc MCP 需要单独安装和运行。请参考 RenderDocMCP 项目的安装说明。

## 工具列表 (19个)

### Capture 管理
| 工具名 | 功能 |
|--------|------|
| `get_capture_status` | 检查capture是否加载 |
| `open_capture` | 打开.rdc文件 |
| `list_captures` | 列出目录下的capture文件 |
| `get_frame_summary` | 获取帧摘要统计 |

### Draw Call 分析
| 工具名 | 功能 |
|--------|------|
| `get_draw_calls` | 获取draw call列表/层级 |
| `get_draw_call_details` | 获取draw call详情 |
| `find_draws_by_shader` | 按shader名查找draw call |
| `find_draws_by_texture` | 按纹理名查找draw call |
| `find_draws_by_resource` | 按资源ID查找draw call |
| `get_action_timings` | 获取GPU时间信息 |

### Shader 分析
| 工具名 | 功能 |
|--------|------|
| `get_shader_info` | 获取shader反汇编+常量缓冲 |
| `save_shader_as_hlsl` | 保存shader为HLSL文件 |
| `get_pipeline_state` | 获取完整管线状态 |

### 纹理操作
| 工具名 | 功能 |
|--------|------|
| `get_texture_info` | 获取纹理元数据 |
| `get_texture_data` | 获取纹理像素数据(Base64) |
| `save_texture` | 保存纹理(PNG/TGA/DDS等) |

### Mesh/Buffer 操作
| 工具名 | 功能 |
|--------|------|
| `get_mesh_data` | 获取mesh顶点数据 |
| `export_mesh_as_fbx` | 导出mesh为FBX |
| `get_buffer_contents` | 读取buffer内容 |

## 配置

在 MCP 配置文件中添加:

```json
{
  "mcpServers": {
    "renderdoc": {
      "command": "python",
      "args": ["-m", "mcp_server.server"],
      "cwd": "/path/to/RenderDocMCP",
      "env": {
        "RENDERDOC_PATH": "C:/Program Files/RenderDoc"
      }
    }
  }
}
```
