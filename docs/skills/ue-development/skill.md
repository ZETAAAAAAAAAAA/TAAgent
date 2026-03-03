# UE Development

Development workflows and tools for Unreal Engine projects.

## Description

This skill provides comprehensive guides for UE development workflows, including world building, material creation, blueprint integration, and MCP tool references.

## Capabilities

- Create and modify UE materials via MCP
- Build procedural environments and structures
- Import assets (textures, FBX meshes)
- Work with Material Functions and blueprints
- Generate complex shapes and visualizations

## Files

| File | Description |
|------|-------------|
| `tools-reference.md` | Complete reference for all UE MCP tools |
| `blueprint-graph-guide.md` | Guide for creating blueprint-style visualizations |
| `colored-shapes-tutorial.md` | Tutorial for creating colored shapes and primitives |

## Prerequisites

- Unreal Engine with UnrealMCP plugin installed
- MCP connection established
- Basic understanding of UE material system

## Tool Categories

### World Building
- `create_town` - Create urban environments
- `construct_house` - Build houses with architectural details

### Materials
- `create_material` - Create new material assets
- `create_material_instance` - Create material instances
- `add_material_expression` - Add nodes to material graph
- `connect_material_nodes` - Connect material nodes
- `compile_material` - Compile and update materials

### Textures
- `import_texture` - Import texture files
- `set_texture_properties` - Configure texture settings

### Meshes
- `import_fbx` - Import FBX files
- `create_static_mesh_from_data` - Create meshes from vertex data

### Material Functions
- `create_material_function` - Create new material functions
- `get_material_functions` - List available functions
- `get_material_function_content` - Get function details

## Quick Start

```
1. Establish MCP connection to UE
2. Create material: create_material(name="M_NewMaterial")
3. Add nodes: add_material_expression(material_name, "TextureSample", ...)
4. Connect nodes: connect_material_nodes(...)
5. Compile: compile_material(material_name)
```

## Related Skills

- `renderdoc-material-reconstruction` - For reconstructing materials from captures
- `shadertoy-conversion` - For converting Shadertoy shaders to UE
