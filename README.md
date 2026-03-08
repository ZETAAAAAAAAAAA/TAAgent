# TA Agent Workspace

**AI-Powered Technical Artist** - 一个具备专业 TA 能力的智能代理。

## 核心理念

TA Agent 不只是被动响应工具调用，而是具备：
- **TA 领域知识**（材质、渲染、性能、管线）
- **问题分解能力**（分析→诊断→方案→执行）
- **工具编排能力**（选择合适的 MCP 工具组合）
- **学习迭代能力**（从结果中学习）


## 架构

```
┌─────────────────────────────────────────────────────────────────────────┐
│                            TA Agent Core                                │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐                   │
│  │   Persona    │  │  Knowledge   │  │    Skills    │                   │
│  │   TA 身份    │  │  领域知识     │  │  工作流 SOP  │                   │
│  └──────────────┘  └──────────────┘  └──────────────┘                   │
└───────────────────────────────┬─────────────────────────────────────────┘
                                │
         ┌──────────────────────┼──────────────────────┐
         ▼                      ▼                      ▼
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│  Analysis MCP   │    │   Creation MCP  │    │  Validation MCP │
│  分析类工具      │    │   创作类工具     │    │   验证类工具     │
├─────────────────┤    ├─────────────────┤    ├─────────────────┤
│ • RenderDoc     │    │ • Unreal Engine │    │ • Lookdev       │
│ • PIX (未来)    │    │ • Unity (未来)  │    │ • Performance   │
│ • Nsight (未来) │    │ • Blender (未来)│    │ • Quality Check │
└─────────────────┘    └─────────────────┘    └─────────────────┘
```

---

## 项目结构

```
TA Agent
|
├── .codebuddy/                   # Agent 核心
│   ├── agents/                   # Agent 定义
│   │   └── TA-Agent.md           # TA 身份与能力定义
│   │
│   ├── rules/                    # 项目规则 (始终加载)
│   │   ├── mcp-development.mdc   # MCP 开发指南
│   │   └── skills-guidelines.mdc # skill 开发指南
│   │
│   ├── skills/                   # 技能模块 (按需加载)
│   │   ├── renderdoc-reverse-engineering/   # GPU 捕获逆向分析
│   │   │   ├── SKILL.md
│   │   │   └── references/       # MCP工具参考、顶点格式
│   │   │
│   │   ├── ue-material-workflow/            # UE 材质系统
│   │   │   ├── SKILL.md
│   │   │   └── references/       # 节点类型、材质函数
│   │   │
│   │   ├── ue-niagara-workflow/             # Niagara 粒子系统
│   │   │   ├── SKILL.md
│   │   │   └── references/       # 模块列表、架构原理
│   │   │
│   │   └── ue-lookdev-workflow/             # 物理灯光与材质校准
│   │       ├── SKILL.md
│   │       └── references/       # 灯光参数、HDRI、LookDev工具
│   │
│   └── knowledge/                # 领域知识
│       └── mcp-tools/            # MCP 工具详细文档
│           ├── renderdoc-mcp.md
│           └── unreal-render-mcp.md
│
├── mcps/                         # MCP Server
│   ├── renderdoc_mcp/            # RenderDoc 分析 MCP
│   │   ├── mcp_server/           # 服务端实现
│   │   └── README.md
│   │
│   └── unreal_render_mcp/        # UE 创作类 MCP
│       ├── server.py             # 主入口
│       ├── handlers/             # 请求处理器
│       └── tools/                # 工具实现
│
├── src/extension/                # RenderDoc Python 扩展
│
├── plugins/unreal/               # UE C++ 插件
│   └── UnrealMCP/RenderingMCP/   # UE 项目
│       └── Plugins/UnrealMCP/    # MCP 插件源码
│
├── config/                       # 配置模板
├── build_logs/                   # 编译日志
└── tools/                        # 辅助工具
```

---

## MCP 概览

### RenderDoc MCP - 分析类

GPU 捕获分析与资产提取工具。

| 类别 | 工具 |
|------|------|
| **捕获** | `get_capture_status`, `open_capture`, `list_captures`, `get_frame_summary` |
| **Draw Call** | `get_draw_calls`, `get_draw_call_details`, `find_draws_by_*`, `get_action_timings` |
| **Shader** | `get_shader_info`, `get_pipeline_state` |
| **纹理** | `get_texture_info`, `get_texture_data`, `save_texture` |
| **网格** | `get_mesh_data`, `export_mesh_as_fbx`, `export_mesh_csv` |

### Unreal Render MCP - 创作类

UE 资产与场景操作工具。

| 类别 | 工具 |
|------|------|
| **通用资产** | `create_asset`, `delete_asset`, `get_assets`, `set_asset_properties`, `batch_*` |
| **通用 Actor** | `spawn_actor`, `delete_actor`, `get_actors`, `set_actor_properties`, `batch_*` |
| **材质图** | `build_material_graph`, `get_material_graph` |
| **纹理** | `import_texture` |
| **网格** | `import_fbx` |
| **Niagara** | `get_niagara_emitter`, `get_niagara_graph`, `update_niagara_emitter`, `update_niagara_graph` |
| **视口** | `get_viewport_screenshot` |

> 详细文档见 `.codebuddy/knowledge/mcp-tools/`

---

## Skills 概览

| Skill | 用途 | 触发场景 |
|-------|------|----------|
| **renderdoc-reverse-engineering** | GPU 捕获逆向分析 | 分析渲染技术、提取资产、复现效果 |
| **ue-material-workflow** | UE 材质系统操作 | 创建/修改材质、分析材质图 |
| **ue-niagara-workflow** | Niagara 粒子系统 | 创建/优化粒子效果、Stateless 转换 |
| **ue-lookdev-workflow** | 物理灯光与材质校准 | HDRI 处理、灯光匹配、材质校准 |

> 详细文档见 `.codebuddy/skills/*/SKILL.md`

---

## 初始化

### 环境要求

| 依赖 | 版本 |
|------|------|
| Python | 3.10+ |
| RenderDoc | 1.20+ |
| Unreal Engine | 5.3+ (推荐 5.7) |

### 首次设置

```bash
# 1. 安装 Python 依赖
pip install -e .

# 2. 初始化 RenderDoc 扩展
python src/scripts/renderdoc/install_extension.py

# 3. 打开 UE 项目（让引擎自动编译插件）
# plugins/unreal/UnrealMCP/RenderingMCP/RenderingMCP.uproject
```

### MCP 配置

**CodeBuddy / Claude Code** (`~/.codebuddy/mcp.json`):

```json
{
  "mcpServers": {
    "renderdoc": {
      "command": "python",
      "args": ["-m", "mcp_server.server"],
      "cwd": "D:/CodeBuddy/rendering-mcp/mcps/renderdoc_mcp"
    },
    "unreal-render": {
      "command": "python",
      "args": ["server.py"],
      "cwd": "D:/CodeBuddy/rendering-mcp/mcps/unreal_render_mcp"
    }
  }
}
```

### 开发提醒

| 修改了... | 需要操作 |
|-----------|----------|
| MCP 服务器代码 | `Ctrl+Shift+P` → `Reload Window` |
| UE 插件代码 | 重新编译 UE 项目 |
| RenderDoc 扩展 | 重启 RenderDoc |

---

## 编译

```bash
# Windows
Build.bat          # 或 Build.ps1

# 日志保存在 build_logs/ 目录
```

---

## 许可证

MIT License
