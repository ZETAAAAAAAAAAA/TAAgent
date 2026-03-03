# Rendering MCP

AI-orchestrated rendering workflow MCP servers for RenderDoc to Unreal Engine asset reconstruction.

## Overview

Rendering MCP is a comprehensive toolkit that enables AI agents to analyze RenderDoc frame captures and reconstruct game assets (meshes, materials, textures) in Unreal Engine. It bridges the gap between runtime graphics debugging and content creation workflows.

### Key Features

- **Mesh Export**: Extract meshes from draw calls to FBX format
- **Material Reconstruction**: Analyze shaders and reconstruct UE materials
- **Texture Extraction**: Export and identify texture types automatically
- **DXBC Analysis**: AI-powered shader bytecode understanding
- **UE Integration**: Direct material and asset creation in Unreal Engine

---

## Architecture

```
┌─────────────────────────────────────────────────────────────────────────┐
│                              AI Agent                                    │
│     (Understands intent → Analyzes DXBC → Orchestrates workflow)        │
└─────────────────────────────────────────────────────────────────────────┘
                    │                              │
                    ▼                              ▼
    ┌───────────────────────────┐    ┌───────────────────────────┐
    │     RenderDoc MCP         │    │    Unreal Render MCP      │
    │     (Capture Analysis)    │    │    (Asset Creation)       │
    ├───────────────────────────┤    ├───────────────────────────┤
    │ • Capture management      │    │ • Material management     │
    │ • Draw call analysis      │    │ • Material node creation  │
    │ • Shader analysis (DXBC)  │    │ • Texture import          │
    │ • Texture extraction      │    │ • FBX mesh import         │
    │ • Mesh export (FBX)       │    │ • Material functions      │
    │ • Buffer data access      │    │                           │
    │                           │    │                           │
    │ 19 tools                  │    │ 15 tools                  │
    └───────────────────────────┘    └───────────────────────────┘
```

---

## Directory Structure

```
rendering-mcp/
│
├── src/                              # Source Code
│   ├── extension/                    # RenderDoc Python Extension
│   │   ├── services/                 # Core services (mesh, texture, shader)
│   │   ├── utils/                    # Utility functions
│   │   ├── extension.json            # Extension manifest
│   │   └── renderdoc_facade.py       # RenderDoc API wrapper
│   │
│   ├── servers/                      # MCP Servers
│   │   ├── renderdoc_mcp/            # RenderDoc MCP server
│   │   └── unreal_render_mcp/        # Unreal Render MCP server
│   │
│   └── scripts/                      # Utility Scripts
│       └── install_extension.py      # RenderDoc extension installer
│
├── plugins/                          # Editor Plugins
│   └── unreal/
│       └── UnrealMCP/                # Unreal Engine MCP Plugin
│           └── RenderingMCP/         # Sample UE Project
│
├── docs/                             # Documentation
│   └── skills/                       # Skill-based Documentation
│       ├── project-setup/            # Setup and build guides
│       ├── renderdoc-fbx-export/     # Mesh export workflow
│       ├── renderdoc-material-reconstruction/  # Material workflow
│       ├── ue-development/           # UE development guides
│       │   └── material-functions/   # UE Material Function library
│       └── shadertoy-conversion/     # Shadertoy to UE conversion
│
├── thirdparty/                       # Third-party Dependencies
│   ├── renderdoc/                    # RenderDoc source (for building)
│   └── renderdoc2fbx/                # FBX export utilities
│
├── config/                           # Configuration
│   └── mcp_config.example.json       # MCP configuration template
│
├── pyproject.toml                    # Python project configuration
├── README.md                         # This file
└── .gitignore
```

---

## Installation

### Prerequisites

- **Python 3.10+**
- **RenderDoc 1.20+** (installed)
- **Unreal Engine 5.3+**

### Step 1: Install Python Package

```bash
# Clone the repository
git clone https://github.com/your-repo/rendering-mcp.git
cd rendering-mcp

# Install in development mode
pip install -e .
```

### Step 2: Install RenderDoc Extension

```bash
# Install the extension to RenderDoc
python src/scripts/install_extension.py

# Output example:
# Extension installed to C:\Users\...\AppData\Roaming\qrenderdoc\extensions\renderdoc_mcp_bridge
```

Enable in RenderDoc:
1. Launch RenderDoc
2. `Tools` → `Manage Extensions`
3. Check `RenderDoc MCP Bridge`

> To uninstall: `python src/scripts/install_extension.py uninstall`

### Step 3: Unreal Engine Plugin

Open `plugins/unreal/UnrealMCP/RenderingMCP/RenderingMCP.uproject`

First launch will compile the plugin. Verify in output log:
```
LogUnrealMCP: MCP Bridge started on port 55557
```

### Step 4: Configure MCP Client

Add to your MCP client configuration:

**Claude Code / CodeBuddy** (`~/.codebuddy/mcp.json`):
```json
{
  "mcpServers": {
    "renderdoc": {
      "command": "python",
      "args": ["-m", "mcp_server.server"],
      "cwd": "/path/to/rendering-mcp/src/servers/renderdoc_mcp"
    },
    "unreal-render": {
      "command": "python",
      "args": ["server.py"],
      "cwd": "/path/to/rendering-mcp/src/servers/unreal_render_mcp"
    }
  }
}
```

### Verify Installation

```
# Test RenderDoc MCP: Open a .rdc file in RenderDoc, then:
get_capture_status  # Should return loaded=true

# Test Unreal MCP: Open UE project, then:
get_available_materials  # Should list materials in project
```

---

## Skills

Skills are domain-specific guides that help AI agents work effectively with this toolkit.

| Skill | Description |
|-------|-------------|
| **project-setup** | Installation, build, and maintenance guides |
| **renderdoc-fbx-export** | Mesh export from captures to FBX format |
| **renderdoc-material-reconstruction** | Shader analysis and material reconstruction |
| **ue-development** | UE material creation and development workflows |
| **shadertoy-conversion** | Convert Shadertoy shaders to UE materials |

---

## Tools Reference

### RenderDoc MCP (19 Tools)

| Category | Tools |
|----------|-------|
| **Capture** | `get_capture_status`, `open_capture`, `list_captures`, `get_frame_summary` |
| **Draw Calls** | `get_draw_calls`, `get_draw_call_details`, `find_draws_by_shader`, `find_draws_by_texture`, `find_draws_by_resource`, `get_action_timings` |
| **Shader** | `get_shader_info`, `save_shader_as_hlsl`, `get_pipeline_state` |
| **Texture** | `get_texture_info`, `get_texture_data`, `save_texture` |
| **Mesh/Buffer** | `get_mesh_data`, `export_mesh_as_fbx`, `get_buffer_contents`, `export_mesh_csv` |

### Unreal Render MCP (15 Tools)

| Category | Tools |
|----------|-------|
| **Material** | `create_material`, `create_material_instance`, `create_material_function`, `set_material_instance_parameter`, `set_material_properties`, `compile_material`, `get_material_expressions`, `get_material_functions`, `get_material_function_content`, `get_available_materials` |
| **Material Nodes** | `add_material_expression`, `connect_material_nodes` |
| **Texture** | `import_texture`, `set_texture_properties` |
| **Mesh** | `import_fbx`, `create_static_mesh_from_data` |

---

## Usage Examples

### Example 1: Mesh Export from Capture

```python
# 1. Open capture
open_capture(capture_path="E:/captures/scene.rdc")

# 2. Find target draw call
get_draw_calls(flags_filter=["Drawcall"])

# 3. Analyze shader for data sources
get_shader_info(event_id=5249, stage="vertex")

# 4. Export mesh to FBX
export_mesh_as_fbx(
    event_id=5249,
    output_path="output/mesh.fbx",
    attribute_mapping={
        "POSITION": "vs_input:ATTRIBUTE0",
        "NORMAL": "buffer:Buffer1",
        "TANGENT": "buffer:Buffer1",
        "UV": "vs_output:TEXCOORD0"
    },
    buffer_config={
        "Buffer1": {
            "stride": 32,
            "normal_offset": 16,
            "tangent_offset": 0,
            "format": "float4"
        }
    },
    coordinate_system="ue",
    unit_scale=1
)
```

### Example 2: Material Reconstruction

```python
# 1. Get shader info
get_shader_info(event_id=123, stage="pixel")

# 2. Get pipeline state for textures
get_pipeline_state(event_id=123)

# 3. Export textures
save_texture(resource_id="Texture2D::12345", 
             output_path="textures/basecolor.png")

# 4. Create UE material
create_material(name="M_Reconstructed")

# 5. Set properties based on shader analysis
set_material_properties(
    material_name="M_Reconstructed",
    blend_mode="Translucent",
    shading_model="Subsurface"
)

# 6. Import textures
import_texture(source_path="textures/basecolor.png", name="T_BaseColor")

# 7. Add material nodes
add_material_expression(
    material_name="M_Reconstructed",
    expression_type="TextureSampleParameter2D",
    parameter_name="BaseColor"
)

# 8. Connect nodes
connect_material_nodes(
    material_name="M_Reconstructed",
    source_node="node_id",
    source_output="RGB",
    target_node="Material",
    target_input="BaseColor"
)
```

---

## Documentation

Comprehensive documentation is available in `docs/skills/`:

- **FBX Export Workflow**: `docs/skills/renderdoc-fbx-export/workflow.md`
- **Material Reconstruction**: `docs/skills/renderdoc-material-reconstruction/workflow.md`
- **DXBC Analysis Guide**: `docs/skills/renderdoc-material-reconstruction/dxbc-analysis.md`
- **UE Tools Reference**: `docs/skills/ue-development/tools-reference.md`
- **Update & Build Guide**: `docs/skills/project-setup/update-and-build.md`

---

## Development

### Project Structure for Developers

| Directory | Purpose |
|-----------|---------|
| `src/extension/` | RenderDoc Python extension (runs in RenderDoc process) |
| `src/servers/renderdoc_mcp/` | MCP server for RenderDoc (runs in MCP client) |
| `src/servers/unreal_render_mcp/` | MCP server for Unreal (runs in MCP client) |
| `plugins/unreal/UnrealMCP/` | UE plugin with TCP server |

### Key Files

| File | Description |
|------|-------------|
| `src/extension/services/resource_service.py` | Mesh export, texture extraction |
| `src/extension/services/shader_service.py` | DXBC analysis, shader utilities |
| `src/servers/renderdoc_mcp/mcp_server/server.py` | MCP tool definitions |
| `plugins/unreal/UnrealMCP/Source/` | UE plugin C++ source |

---

## Troubleshooting

| Issue | Solution |
|-------|----------|
| MCP connection failed | Check config path and server status |
| RenderDoc extension not loading | Verify extension path in RenderDoc settings |
| UE plugin not working | Rebuild plugin for your UE version |
| Import errors | Reinstall: `pip install -e .` |
| FBX normals inverted | Set `flip_winding_order=True` |

---

## License

MIT License
