# Shadertoy Conversion

Convert Shadertoy shaders to Unreal Engine materials.

## Description

This skill provides techniques for porting Shadertoy shaders to Unreal Engine's material system, handling coordinate transformations, input mappings, and node-based implementation.

## Capabilities

- Analyze Shadertoy shader code
- Map Shadertoy inputs to UE equivalents
- Convert GLSL to HLSL/node-based material logic
- Handle time-based animations
- Implement common Shadertoy techniques in UE

## Files

| File | Description |
|------|-------------|
| `guide.md` | Complete conversion guide with examples |

## Prerequisites

- Understanding of GLSL and HLSL
- Knowledge of UE material system
- Familiarity with Shadertoy conventions

## Key Conversion Points

| Shadertoy | Unreal Engine |
|-----------|---------------|
| `iTime` | `Time` node |
| `iResolution` | `ViewSize` or custom parameter |
| `fragCoord` | `PixelPosition` / `ScreenPosition` |
| `iMouse` | Custom parameters |
| `texture(iChannel0, uv)` | `TextureSample` node |
| GLSL functions | HLSL equivalents |

## Common Techniques

### UV Coordinates
- Shadertoy: `fragCoord / iResolution`
- UE: `ScreenPosition` → `ComponentMask` → Divide by `ViewSize`

### Time Animation
- Shadertoy: `iTime`
- UE: `Time` node or `TimeWithSpeedVariable`

### Custom Parameters
- Create Scalar/Vector parameters for user-controllable values

## Related Skills

- `ue-development` - For UE material creation
- `renderdoc-material-reconstruction` - For shader analysis techniques
