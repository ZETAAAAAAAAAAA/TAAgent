# Rendering MCP

AI-orchestrated rendering workflow MCP servers for RenderDoc to Unreal Engine asset reconstruction.

## Overview

Rendering MCP enables AI agents to analyze RenderDoc frame captures and reconstruct game assets (meshes, materials, textures) in Unreal Engine.

### Key Features

- **Mesh Export**: Extract meshes from draw calls to FBX (GPU-driven rendering support)
- **Material Reconstruction**: Analyze shaders and reconstruct UE materials
- **Material Analysis**: Read existing UE material graphs and connections
- **Texture Extraction**: Export and identify texture types automatically
- **DXBC Analysis**: AI-powered shader bytecode understanding
- **UE Integration**: Direct asset creation via reflection-based generic tools
- **Lookdev Tools**: Screenshot capture and lighting for asset validation

## Architecture

```
AI Agent
    │
    ├──► RenderDoc MCP (20 tools)
    │      • Capture analysis, draw calls, shaders
    │      • Texture extraction, mesh export (FBX)
    │
    └──► Unreal Render MCP (23 tools)
           • Generic asset/actor tools (reflection-based)
           • Material graph tools
           • Viewport capture
```

## Installation

### Prerequisites

- Python 3.10+
- RenderDoc 1.20+
- Unreal Engine 5.3+

### Setup

```bash
# 1. Clone and install
git clone https://github.com/your-repo/rendering-mcp.git
cd rendering-mcp
pip install -e .

# 2. Install RenderDoc extension
python src/scripts/install_extension.py

# 3. Open UE project
# plugins/unreal/UnrealMCP/RenderingMCP/RenderingMCP.uproject
```

### MCP Configuration

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

## Tools Reference

### RenderDoc MCP (20 Tools)

| Category | Tools |
|----------|-------|
| **Capture** | `get_capture_status`, `open_capture`, `list_captures`, `get_frame_summary` |
| **Draw Calls** | `get_draw_calls`, `get_draw_call_details`, `find_draws_by_*`, `get_action_timings` |
| **Shader** | `get_shader_info`, `save_shader_as_hlsl`, `get_pipeline_state` |
| **Texture** | `get_texture_info`, `get_texture_data`, `save_texture` |
| **Mesh** | `get_mesh_data`, `export_mesh_as_fbx`, `get_buffer_contents`, `export_mesh_csv` |

### Unreal Render MCP (23 Tools)

**Generic Asset Tools** (6) - *New reflection-based*
```python
create_asset(type, name, path, properties)      # Any asset type
delete_asset(path)
set_asset_properties(path, properties)          # Universal property setting
get_asset_properties(path, properties?)
batch_create_assets(items)
batch_set_assets_properties(items)
```

**Generic Actor Tools** (8) - *New reflection-based*
```python
spawn_actor(class, name?, location?, rotation?, scale?, properties?)
delete_actor(name)
get_actors(class?, detailed?)
set_actor_properties(name, properties)          # Universal property setting
get_actor_properties(name, properties?)
batch_spawn_actors(items)
batch_delete_actors(names)
batch_set_actors_properties(items)
```

**Material Graph** (4): `add_material_expression`, `connect_material_nodes`, `get_material_expressions`, `get_material_connections`

**Other** (5): `get_material_properties`, `get_material_functions`, `import_texture`, `import_fbx`, `get_viewport_screenshot`

## Usage Examples

### Mesh Export from Capture

```python
open_capture(capture_path="E:/captures/scene.rdc")
get_draw_calls(flags_filter=["Drawcall"])
export_mesh_as_fbx(
    event_id=5249,
    output_path="output/mesh.fbx",
    attribute_mapping={
        "POSITION": "vs_input:ATTRIBUTE0",
        "NORMAL": "buffer:Buffer1"
    },
    buffer_config={"Buffer1": {"stride": 32, "normal_offset": 16}}
)
```

### Generic Asset Creation (Reflection-Based)

```python
# Create any asset type
create_asset("Material", "M_Red", "/Game/Materials/")
create_asset("MaterialInstance", "MI_Red", properties={"parent": "/Game/Materials/M_Base"})

# Set properties (universal interface)
set_asset_properties("/Game/Materials/M_Red.M_Red", {
    "shading_model": "DefaultLit",
    "blend_mode": "Opaque"
})

# Batch operations
batch_create_assets([
    {"asset_type": "Material", "name": "M_Red"},
    {"asset_type": "Material", "name": "M_Green"},
    {"asset_type": "MaterialInstance", "name": "MI_Red", "properties": {"parent": "/Game/Materials/M_Base"}}
])
```

### Generic Actor Spawning (Reflection-Based)

```python
# Spawn any actor type
spawn_actor("DirectionalLight", "KeyLight",
    rotation={"pitch": -45, "yaw": 30},
    properties={"intensity": 10.0, "cast_shadows": True})

spawn_actor("StaticMeshActor", "MaterialBall",
    location={"x": 0, "y": 0, "z": 100},
    properties={"static_mesh": "/Engine/BasicShapes/Sphere"})

# Batch operations
batch_spawn_actors([
    {"actor_class": "Sphere", "name": "Ball1"},
    {"actor_class": "Sphere", "name": "Ball2"},
    {"actor_class": "DirectionalLight", "name": "KeyLight", "properties": {"intensity": 10}}
])

batch_set_actors_properties([
    {"name": "KeyLight", "properties": {"intensity": 15.0}},
    {"name": "FillLight", "properties": {"intensity": 8.0}}
])
```

## Skills

| Skill | Description |
|-------|-------------|
| **project-setup** | Installation and build guides |
| **renderdoc-fbx-export** | Mesh export workflow |
| **renderdoc-material-reconstruction** | Shader analysis and material reconstruction |
| **ue-development** | UE generic asset/actor tools, material graphs |
| **lookdev** | Asset validation environment |
| **shadertoy-conversion** | Shadertoy to UE conversion |

## Documentation

| Document | Path |
|----------|------|
| FBX Export Workflow | `docs/skills/renderdoc-fbx-export/workflow.md` |
| Material Reconstruction | `docs/skills/renderdoc-material-reconstruction/workflow.md` |
| UE Tools Reference | `docs/skills/ue-development/skill.md` |
| Lookdev Guide | `docs/skills/lookdev/skill.md` |
| Update & Build | `docs/skills/project-setup/update-and-build.md` |

## Troubleshooting

| Issue | Solution |
|-------|----------|
| MCP connection failed | Check config path and server status |
| FBX normals inverted | Set `flip_winding_order=True` |
| GPU-driven mesh export | Use `buffer_config` parameter |
| UE compile errors | Rebuild plugin for your UE version |

## License

MIT License
