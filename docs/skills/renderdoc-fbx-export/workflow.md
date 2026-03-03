# FBX Export Workflow

## Overview

A comprehensive workflow for exporting meshes from RenderDoc captures to FBX files.

**Core Principle: AI analyzes data and provides explicit mappings - NO automatic detection.**

---

## Complete Workflow

```
┌─────────────────────────────────────────────────────────────────┐
│  Step 0: Identify Source Data Information                       │
│  ┌───────────────────────────────────────────────────────────┐  │
│  │ • Identify game engine type (UE/Unity/Other)              │  │
│  │ • Confirm coordinate system (Z-up/Y-up, Left/Right-hand)  │  │
│  │ • Confirm unit scale (centimeters/meters)                 │  │
│  │ • Confirm winding order (CW/CCW)                          │  │
│  └───────────────────────────────────────────────────────────┘  │
│                              ↓                                  │
│  Step 1: Analyze Shader                                         │
│  ┌───────────────────────────────────────────────────────────┐  │
│  │ get_shader_info(event_id, "vertex")                       │  │
│  │                                                           │  │
│  │ • Identify data source (VS Input / VS Output / Buffer)    │  │
│  │ • Determine if Position is model or world space           │  │
│  │ • Check if Normal/Tangent are space-transformed           │  │
│  │ • Analyze Buffer read patterns (GPU-Driven rendering)     │  │
│  └───────────────────────────────────────────────────────────┘  │
│                              ↓                                  │
│  Step 2: Export VS Input CSV                                    │
│  ┌───────────────────────────────────────────────────────────┐  │
│  │ export_mesh_csv(event_id, "vs_input")                     │  │
│  │                                                           │  │
│  │ • Get model-space POSITION ✅                             │  │
│  │ • Get model-space NORMAL/TANGENT ✅                        │  │
│  │ • Observe value ranges to determine unit scale            │  │
│  └───────────────────────────────────────────────────────────┘  │
│                              ↓                                  │
│  Step 3: Export VS Output CSV                                   │
│  ┌───────────────────────────────────────────────────────────┐  │
│  │ export_mesh_csv(event_id, "vs_output")                    │  │
│  │                                                           │  │
│  │ • Get UV data (usually in VS Output) ✅                    │  │
│  │ • ⚠️ Avoid SV_Position (clip space)                        │  │
│  │ • ⚠️ Avoid VS Output Normal (usually world space)          │  │
│  └───────────────────────────────────────────────────────────┘  │
│                              ↓                                  │
│  Step 4: Build attribute_mapping                                │
│  ┌───────────────────────────────────────────────────────────┐  │
│  │ attribute_mapping = {                                      │  │
│  │   "POSITION": "vs_input:ATTRIBUTE0",   # Model space      │  │
│  │   "NORMAL":   "vs_input:ATTRIBUTE1",   # Model space      │  │
│  │   "TANGENT":  "vs_input:ATTRIBUTE2",   # Model space      │  │
│  │   "UV":       "vs_output:TEXCOORD0"    # No space concept │  │
│  │ }                                                         │  │
│  └───────────────────────────────────────────────────────────┘  │
│                              ↓                                  │
│  Step 5: Export FBX                                             │
│  ┌───────────────────────────────────────────────────────────┐  │
│  │ export_mesh_as_fbx(                                       │  │
│  │     event_id,                                             │  │
│  │     output_path,                                          │  │
│  │     attribute_mapping={...},                              │  │
│  │     coordinate_system="ue",                               │  │
│  │     unit_scale=1,                                         │  │
│  │     flip_winding_order=False,                             │  │
│  │     buffer_config={...}        # For GPU-Driven mode      │  │
│  │ )                                                         │  │
│  └───────────────────────────────────────────────────────────┘  │
│                              ↓                                  │
│  Step 6: Verify in Target Software                              │
│  ┌───────────────────────────────────────────────────────────┐  │
│  │ • Normals inverted → Set flip_winding_order=True          │  │
│  │ • Wrong scale → Adjust unit_scale                         │  │
│  │ • Position offset → Check if world space was used         │  │
│  │ • Lighting issues → Check if Normal is world space        │  │
│  └───────────────────────────────────────────────────────────┘  │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## Space Type Selection Principle

### ⭐ Core Principle: Prefer Model Space Data

| Data Type | Recommended Source | Space Type | Reason |
|-----------|-------------------|------------|--------|
| POSITION | **vs_input** | Model Space | Original vertex position, reusable |
| NORMAL | **vs_input** or **buffer** | Model Space | Original normal, correct lighting |
| TANGENT | **vs_input** or **buffer** | Model Space | Original tangent, correct normal maps |
| UV | **vs_output** | No Space | Texture coordinates, usually in VS Output |
| SV_Position | ❌ DO NOT USE | Clip Space | Transformed by MVP, unusable |

### Why Not Use VS Output Normal/Tangent?

```hlsl
// Typical Vertex Shader code
VSOutput main(VSInput input) {
    VSOutput output;
    output.Position = mul(MVP, input.Position);       // → Clip Space
    output.Normal = mul(WorldMatrix, input.Normal);   // → World Space ❌
    output.Tangent = mul(WorldMatrix, input.Tangent); // → World Space ❌
    output.UV = input.UV;                             // → No transform ✅
    return output;
}
```

**VS Output Normal is World Space:**
- Used for lighting calculations, already transformed to world space
- When imported to FBX, the model will apply transforms again, causing incorrect normals
- Symptoms: wrong lighting direction, abnormal normal map display

---

## Coordinate System & Unit Scale

### Engine Conventions

| Engine | Up Axis | Coordinate System | Front Face Winding | Default Unit | unit_scale |
|--------|---------|-------------------|-------------------|--------------|------------|
| UE4/UE5 | Z | Left-handed | CW (Clockwise) | Centimeters | 1 |
| Unity | Y | Left-handed | CW (Clockwise) | Meters | 100 |
| Godot | Y | Right-handed | CCW (Counter-clockwise) | Meters | 100 |
| CryEngine | Z | Left-handed | CW (Clockwise) | Centimeters | 1 |

### Unit Scale Determination

```
Position value range analysis:
├─ Values ~1-10       → Meters (Unity)
├─ Values ~100-1000   → Centimeters (UE)
├─ Values ~1000-10000 → Millimeters (some games)
└─ Check if object size matches real-world logic
```

### Winding Order Conversion

```
Source Winding → Target Winding → flip_winding_order
───────────────────────────────────────────────────
CW → CW   (UE→UE, Unity→Unity)    → False
CW → CCW  (UE→Blender)            → True
CCW → CW  (Blender→UE)            → True
CCW → CCW (Blender→Blender)       → False
```

---

## Data Source Identification

### Traditional vs GPU-Driven Rendering

| Rendering Mode | Position | Normal | Tangent | UV |
|---------------|----------|--------|---------|-----|
| Traditional | VS Input | VS Input | VS Input | VS Output |
| UE5 GPU-Driven | VS Input | Buffer | Buffer | VS Output |
| Skeletal Animation | VS Input* | Buffer | Buffer | VS Output |

*Note: For skeletal animation, VS Input is bind pose, may need special handling

### How to Identify GPU-Driven Rendering

```
Analyze Shader code:
├─ Search for "Buffer.Load" or "StructuredBuffer"
│   └─ Found → GPU-Driven mode, need buffer_config
│
├─ Search for index calculation formulas
│   └─ e.g., "index = vertexId * 2" → Buffer layout basis
│
└─ Search for matrix multiplication
    └─ "mul(World, normal)" → Normal will be transformed to world space
```

---

## attribute_mapping Configuration Templates

### Standard Configuration (Traditional Rendering)

```python
attribute_mapping = {
    "POSITION": "vs_input:ATTRIBUTE0",   # ✅ Model Space
    "NORMAL":   "vs_input:ATTRIBUTE1",   # ✅ Model Space
    "TANGENT":  "vs_input:ATTRIBUTE2",   # ✅ Model Space
    "UV":       "vs_output:TEXCOORD0",   # ✅ UV
}
```

### UE5 GPU-Driven Configuration

```python
attribute_mapping = {
    "POSITION": "vs_input:ATTRIBUTE0",   # ✅ Model Space
    "NORMAL":   "buffer:Buffer1",        # ✅ Buffer stores model space
    "TANGENT":  "buffer:Buffer1",        # ✅ Buffer stores model space
    "UV":       "vs_output:TEXCOORD0",   # ✅ UV
}

buffer_config = {
    "Buffer1": {
        "stride": 32,           # Bytes per vertex
        "tangent_offset": 0,    # tangent at [v*2]
        "normal_offset": 16,    # normal at [v*2+1]
        "format": "float4"
    }
}
```

### ❌ Wrong Configuration Examples

```python
# Wrong 1: Using world space Position
attribute_mapping = {
    "POSITION": "vs_output:SV_Position",  # ❌ Clip Space!
}

# Wrong 2: Using world space Normal
attribute_mapping = {
    "NORMAL": "vs_output:NORMAL",  # ❌ Usually World Space!
}

# Wrong 3: GPU-Driven without buffer_config
attribute_mapping = {
    "NORMAL": "buffer:Buffer1",  # ❌ Missing buffer_config!
}
```

---

## Identifying Semantics from CSV Data

| Semantic | Characteristics to Look For |
|----------|----------------------------|
| **POSITION** | Large values (>1), 3-4 components. Check if values are model space (centered around origin) or world space (large offset). Prefer vs_input. |
| **NORMAL** | Normalized vector (length ≈ 1), 3-4 components. Values in [-1, 1]. May be compressed as int16. Prefer vs_input. |
| **TANGENT** | Normalized vector, 4 components (w = sign for binormal). May be UNORM [0,1] needing conversion to [-1,1]. Prefer vs_input. |
| **UV** | 2 components, typically in [0, 1] range. Look for TEXCOORD with 2 components. Usually in vs_output. |
| **VertexColor** | 4 components, values in [0, 1] range, RGBA color. |

---

## Common Decode Patterns

| Pattern | Decode Expression |
|---------|-------------------|
| int16 normalized (SNORM) | `itof(x) / 32768` |
| UNORM to SNORM | `x * 2 - 1` |
| Normalize after decode | `normalize(itof(x) / 32768)` |

---

## Coordinate Systems

| Target Software | coordinate_system | Characteristics |
|-----------------|-------------------|-----------------|
| Unreal Engine | "ue" | Z-up, left-handed |
| Unity | "unity" | Y-up, left-handed |
| Blender | "blender" | Z-up, right-handed |
| Maya | "maya" | Y-up, right-handed |

---

## Common Issues & Solutions

| Issue | Cause | Solution |
|-------|-------|----------|
| Normals facing inward | Winding order mismatch | Set `flip_winding_order=True` |
| Model too large/small | Unit mismatch | Adjust `unit_scale` |
| Wrong lighting direction | Normal is world space | Use vs_input Normal instead |
| Model position offset | Position is world space | Use vs_input Position instead |
| Normal map issues | Tangent is world space | Use vs_input Tangent instead |
| UV misaligned | Wrong TEXCOORD semantic | Check CSV for correct TEXCOORD |
| Missing Buffer data | GPU-Driven mode | Add `buffer_config` |

---

## Verification Checklist

```
Before Export:
□ Confirmed game engine type and coordinate system
□ Confirmed unit scale (cm/m)
□ Confirmed Position from vs_input (model space)
□ Confirmed Normal/Tangent from vs_input or buffer (model space)
□ Confirmed UV from correct TEXCOORD
□ Added buffer_config for GPU-Driven mode

After Import:
□ Model size is reasonable
□ Normal direction is correct (facing outward)
□ Lighting response is normal
□ UV is correctly unwrapped
□ Multiple models have correct relative positions
```

---

## Complete Example

```python
# Step 0: Identify engine (UE5) → Z-up, left-handed, centimeters

# Step 1: Analyze shader for GPU-Driven rendering
get_shader_info(event_id=5249, stage="vertex")
# Found: Buffer1.Load(vertexId * 2) for tangent
# Found: Buffer1.Load(vertexId * 2 + 1) for normal

# Step 2: Export CSV to verify
export_mesh_csv(event_id=5249, output_path="vs_input.csv", stage="vs_input")
export_mesh_csv(event_id=5249, output_path="vs_output.csv", stage="vs_output")

# Step 3: Analyze CSV
# - vs_input ATTRIBUTE0: Position data (model space)
# - vs_output TEXCOORD0: UV data
# - Buffer needed for Normal/Tangent

# Step 4: Export FBX
export_mesh_as_fbx(
    event_id=5249,
    output_path="mesh.fbx",
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
    unit_scale=1,
    flip_winding_order=False  # Set to True if normals are inverted
)

# Step 5: Import to UE and verify
# - If normals inverted → Set flip_winding_order=True and re-export
```

---

## Error Messages

| Error | Cause | Solution |
|-------|-------|----------|
| "attribute_mapping is REQUIRED" | No mapping provided | AI must analyze CSV/DXBC and provide mappings |
| "must include stage prefix" | Mapping without stage | Use "vs_output:ATTR" or "vs_input:ATTR" format |
| "Attribute not found in stage" | Wrong attribute name | Check CSV headers for exact names |
| "POSITION must be mapped" | Missing position | Always map POSITION |
| "buffer_config required" | Buffer mapping without config | Provide buffer_config with stride and offsets |
