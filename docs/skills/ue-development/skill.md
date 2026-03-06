# UE Development

Unreal Engine development workflows via MCP using generic reflection-based tools.

## Overview

This skill provides unified tools for UE asset and actor management through reflection. Instead of separate tools for each asset/actor type, use generic tools that work with any UE class.

## Design Philosophy

**Match and Modify**: Property names match UE UPROPERTY names. Match → modify, no match → ignore (no error).

## Generic Asset Tools (7 tools)

| Tool | Purpose |
|------|---------|
| `create_asset(type, name, path, properties)` | Create any asset type |
| `delete_asset(path)` | Delete asset |
| `set_asset_properties(path, properties)` | Set any asset properties |
| `get_asset_properties(path, properties?)` | Get asset properties |
| `get_assets(path?, asset_class?, name_filter?)` | List assets by type |
| `batch_create_assets(items)` | Batch create |
| `batch_set_assets_properties(items)` | Batch modify |

### Asset Listing Examples

```python
# Get all materials (replaces get_available_materials)
get_assets("/Game/", "Material")

# Get material functions (replaces get_material_functions)
get_assets("/Engine/Functions/", "MaterialFunction")

# Get textures with name filter
get_assets("/Game/Textures/", "Texture", name_filter="T_")

# Get all assets in folder
get_assets("/Game/Meshes/")
```

### Examples

```python
# Create materials
create_asset("Material", "M_Red", "/Game/Materials/")
create_asset("MaterialInstance", "MI_Red", properties={"parent": "/Game/Materials/M_Base"})

# Set properties (works for any asset type)
set_asset_properties("/Game/Materials/M_Red.M_Red", {
    "shading_model": "DefaultLit",
    "blend_mode": "Opaque"
})

set_asset_properties("/Game/Textures/T_Normal.T_Normal", {
    "srgb": False,
    "compression_settings": "TC_NormalMap"
})
```

## Generic Actor Tools (8 tools)

| Tool | Purpose |
|------|---------|
| `spawn_actor(class, name?, location?, rotation?, scale?, properties?)` | Spawn any actor |
| `delete_actor(name)` | Delete actor |
| `get_actors(class?, detailed?)` | List actors |
| `set_actor_properties(name, properties)` | Set actor properties |
| `get_actor_properties(name, properties?)` | Get actor properties |
| `batch_spawn_actors(items)` | Batch spawn |
| `batch_delete_actors(names)` | Batch delete |
| `batch_set_actors_properties(items)` | Batch modify |

### Examples

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
    {"actor_class": "Sphere", "name": "Ball1", "location": {"x": 0, "y": 0, "z": 100}},
    {"actor_class": "Sphere", "name": "Ball2", "location": {"x": 200, "y": 0, "z": 100}},
    {"actor_class": "DirectionalLight", "name": "KeyLight", "properties": {"intensity": 10}}
])

batch_set_actors_properties([
    {"name": "KeyLight", "properties": {"intensity": 15.0}},
    {"name": "FillLight", "properties": {"intensity": 8.0}}
])
```

## Material Graph Tools (Recommended)

### `build_material_graph` (Batch Operation)

Build an entire material graph in a single call. Replaces multiple `add_material_expression` + `connect_material_nodes` calls.

```python
build_material_graph(
    material_name="M_MyMaterial",
    nodes=[
        {"id": "color", "type": "Constant3Vector", "pos_x": -300, "pos_y": 0, 
         "value": [1.0, 0.0, 0.0]},
        {"id": "roughness", "type": "Constant", "pos_x": -300, "pos_y": 200, 
         "value": 0.5},
        {"id": "tex_base", "type": "TextureSample", "pos_x": -500, "pos_y": 0,
         "texture": "/Game/Textures/T_BaseColor"}
    ],
    connections=[
        {"source": "tex_base", "target": "Material", "target_input": "BaseColor"},
        {"source": "color", "target": "Material", "target_input": "EmissiveColor"},
        {"source": "roughness", "target": "Material", "target_input": "Roughness"}
    ],
    properties={"shading_model": "DefaultLit", "blend_mode": "Opaque"},
    compile=True
)
```

**Supported Node Types:** Constant, Constant2/3/4Vector, ScalarParameter, VectorParameter, TextureSample, Multiply, Add, Subtract, Divide, Lerp, Clamp, TextureCoordinate, Time, VertexNormal, WorldPosition, OneMinus, Saturate, Normalize, DotProduct, CrossProduct, ComponentMask, AppendVector, Fresnel, Power, Panner, Rotator, Sine, Cosine, Abs, Floor, Ceil, Frac, SquareRoot, Desaturation, ReflectionVector, CameraPosition, CameraVector

### Material Graph Analysis

```python
# Get complete material graph (nodes + connections)
get_material_graph("/Game/Materials/M_Material")

# Returns:
# {
#   "nodes": [...],
#   "connections": [...],
#   "property_connections": {...}
# }
```

### Material Function Inspection

```python
# Get material function details
get_material_function_content("/Engine/Functions/Engine_MaterialFunctions01/Texturing/BitMask")
```

## Other Tools

| Category | Tools |
|----------|-------|
| **Material** | `compile_material` |
| **Asset Listing** | `get_assets` (replaces `get_available_materials`, `get_material_functions`) |
| **Texture Import** | `import_texture`, `import_fbx` |
| **Mesh** | `create_static_mesh_from_data` |
| **Viewport** | `get_viewport_screenshot` |

## Legacy Tools (Removed)

| Removed Tool | Replacement |
|--------------|-------------|
| `get_available_materials` | `get_assets(path, "Material")` |
| `get_material_functions` | `get_assets(path, "MaterialFunction")` |
| `get_material_expressions` | `get_material_graph` (nodes field) |
| `get_material_connections` | `get_material_graph` (connections field) |
| `get_material_properties` | `get_asset_properties` |
| `create_material` | `create_asset("Material", ...)` |
| `set_material_properties` | `set_asset_properties(...)` |
| `create_light` | `spawn_actor("DirectionalLight", ...)` |
| `set_light_properties` | `set_actor_properties(...)` |

## Common Property Names

### Material
- `shading_model`: "DefaultLit", "Unlit", "Subsurface"
- `blend_mode`: "Opaque", "Masked", "Translucent"
- `two_sided`: true/false

### Light
- `intensity`: float
- `light_color`: [R, G, B, A]
- `temperature`: float (K)
- `cast_shadows`: true/false
- `attenuation_radius`: float

### Texture
- `srgb`: true/false
- `compression_settings`: "Default", "TC_NormalMap", "TC_Grayscale"

## Files

| File | Description |
|------|-------------|
| `tools-reference.md` | Detailed tool documentation |
| `material-analysis.md` | Analyzing existing materials |
| `material-functions/` | Material Function library |

## Related Skills

- `lookdev` - Asset validation environment
- `renderdoc-material-reconstruction` - Material reconstruction from captures
- `shadertoy-conversion` - Shadertoy to UE conversion
