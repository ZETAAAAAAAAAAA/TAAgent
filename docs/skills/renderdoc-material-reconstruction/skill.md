# RenderDoc Material Reconstruction

Reconstruct UE materials from RenderDoc shader analysis and texture extraction.

## Description

This skill provides workflows and techniques for analyzing RenderDoc captures to reconstruct materials in Unreal Engine. It covers DXBC shader analysis, texture identification, and material node creation.

## Capabilities

- Analyze DXBC shader bytecode to understand material logic
- Identify and extract textures from captures
- Determine material properties (blend mode, shading model)
- Create corresponding UE materials with proper node connections
- Handle compressed and encoded texture formats

## Files

| File | Description |
|------|-------------|
| `workflow.md` | Complete material reconstruction workflow |
| `dxbc-analysis.md` | DXBC instruction reference and analysis guide |
| `texture-identification.md` | Guide for identifying texture types and formats |

## Prerequisites

- RenderDoc capture loaded via MCP
- Understanding of HLSL and shader concepts
- Knowledge of UE material system

## Quick Start

```
1. Get pipeline state from target draw call
2. Get shader info (vertex and pixel shader)
3. Analyze DXBC to understand material logic
4. Identify textures used in shader
5. Export textures from capture
6. Create UE material with identified properties
7. Import textures and connect material nodes
```

## Material Property Decision Guide

| Shader Feature | Property Setting |
|---------------|------------------|
| No alpha operations | Blend Mode: Opaque |
| `clip()` / `discard` | Blend Mode: Masked |
| Alpha blending | Blend Mode: Translucent |
| Standard PBR flow | Shading Model: DefaultLit |
| No lighting calculation | Shading Model: Unlit |
| SSS calculation | Shading Model: Subsurface |
| Vegetation foliage | Shading Model: TwoSidedFoliage |

## Texture Compression Guide

| Texture Type | Compression | sRGB |
|--------------|-------------|------|
| BaseColor | Default (BC7) | True |
| Normal Map | TC_NormalMap | False |
| ORM Map | Default | False |
| Emissive | Default | True |

## Related Skills

- `renderdoc-fbx-export` - For exporting meshes from captures
- `ue-development` - For UE material creation workflows
