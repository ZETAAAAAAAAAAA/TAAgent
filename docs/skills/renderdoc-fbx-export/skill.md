# RenderDoc FBX Export

Export meshes from RenderDoc captures to FBX files for use in 3D software.

## Description

This skill provides a comprehensive workflow for exporting meshes from RenderDoc frame captures to FBX format. It handles various rendering scenarios including traditional rendering, UE5 GPU-driven rendering, and skeletal animation models.

## Capabilities

- Export meshes from any draw call in a RenderDoc capture
- Support for multiple coordinate systems (UE, Unity, Blender, Maya)
- Handle GPU-driven rendering (UE5 Nanite, etc.)
- Automatic normal/tangent space handling
- Winding order conversion between different engines

## Files

| File | Description |
|------|-------------|
| `workflow.md` | Complete FBX export workflow with step-by-step instructions |
| `ue-integration.md` | Integration guide for importing exported meshes into Unreal Engine |

## Prerequisites

- RenderDoc capture loaded via MCP
- Understanding of the source game's coordinate system and unit scale
- Knowledge of whether the mesh uses traditional or GPU-driven rendering

## Quick Start

```
1. Load RenderDoc capture
2. Identify target draw call (event_id)
3. Analyze shader to determine data sources
4. Export CSV to verify vertex data
5. Build attribute_mapping based on analysis
6. Export FBX with appropriate parameters
7. Import to target software and verify
```

## Key Parameters

| Parameter | Description |
|-----------|-------------|
| `coordinate_system` | Target software coordinate system ("ue", "unity", "blender", "maya") |
| `unit_scale` | Unit scale factor (1 for cm, 100 for meters) |
| `flip_winding_order` | Set True if normals are inverted after import |
| `buffer_config` | Required for GPU-driven rendering meshes |

## Common Issues

| Issue | Solution |
|-------|----------|
| Normals facing inward | Set `flip_winding_order=True` |
| Wrong scale | Adjust `unit_scale` |
| Position offset | Use `vs_input` instead of `vs_output` for POSITION |
| Lighting issues | Use `vs_input` instead of `vs_output` for NORMAL/TANGENT |

## Related Skills

- `renderdoc-material-reconstruction` - For reconstructing materials from captures
- `ue-development` - For UE-specific workflows
