# UE Material Functions - Blends

> 混合模式相关 Material Function 详细文档

---

## Photoshop-Style Blends

### Blend_Screen
**Path**: `/Engine/Functions/Engine_MaterialFunctions03/Blends/Blend_Screen.Blend_Screen`
**Function**: Inverts each texture, multiplies, then re-inverts

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Base | Vector3 | Base texture input |
| Blend | Vector3 | Blend texture input |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Screen blended result |

---

### Blend_Overlay
**Path**: `/Engine/Functions/Engine_MaterialFunctions03/Blends/Blend_Overlay.Blend_Overlay`
**Function**: Emulates Photoshop's Overlay blend mode

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Blend | Vector3 | Blend texture input |
| Base | Vector3 | Base texture input |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Overlay blended result |

---

### Blend_ColorBurn
**Path**: `/Engine/Functions/Engine_MaterialFunctions03/Blends/Blend_ColorBurn.Blend_ColorBurn`
**Function**: Color burn blend mode

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Base | Vector3 | Base texture |
| Blend | Vector3 | Blend texture |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Color burned result |

---

### Blend_ColorDodge
**Path**: `/Engine/Functions/Engine_MaterialFunctions03/Blends/Blend_ColorDodge.Blend_ColorDodge`
**Function**: Color dodge blend mode

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Base | Vector3 | Base texture |
| Blend | Vector3 | Blend texture |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Color dodged result |

---

### Blend_Darken
**Path**: `/Engine/Functions/Engine_MaterialFunctions03/Blends/Blend_Darken.Blend_Darken`
**Function**: Darken blend mode (keeps darker values)

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Base | Vector3 | Base texture |
| Blend | Vector3 | Blend texture |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Darkened result |

---

### Blend_Lighten
**Path**: `/Engine/Functions/Engine_MaterialFunctions03/Blends/Blend_Lighten.Blend_Lighten`
**Function**: Lighten blend mode (keeps lighter values)

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Base | Vector3 | Base texture |
| Blend | Vector3 | Blend texture |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Lightened result |

---

### Blend_Difference
**Path**: `/Engine/Functions/Engine_MaterialFunctions03/Blends/Blend_Difference.Blend_Difference`
**Function**: Difference blend mode

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Base | Vector3 | Base texture |
| Blend | Vector3 | Blend texture |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Difference result |

---

### Blend_Exclusion
**Path**: `/Engine/Functions/Engine_MaterialFunctions03/Blends/Blend_Exclusion.Blend_Exclusion`
**Function**: Exclusion blend mode

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Base | Vector3 | Base texture |
| Blend | Vector3 | Blend texture |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Exclusion result |

---

### Blend_HardLight
**Path**: `/Engine/Functions/Engine_MaterialFunctions03/Blends/Blend_HardLight.Blend_HardLight`
**Function**: Hard light blend mode

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Base | Vector3 | Base texture |
| Blend | Vector3 | Blend texture |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Hard light result |

---

### Blend_SoftLight
**Path**: `/Engine/Functions/Engine_MaterialFunctions03/Blends/Blend_SoftLight.Blend_SoftLight`
**Function**: Soft light blend mode

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Base | Vector3 | Base texture |
| Blend | Vector3 | Blend texture |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Soft light result |

---

### Blend_LinearBurn
**Path**: `/Engine/Functions/Engine_MaterialFunctions03/Blends/Blend_LinearBurn.Blend_LinearBurn`
**Function**: Linear burn blend mode

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Base | Vector3 | Base texture |
| Blend | Vector3 | Blend texture |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Linear burned result |

---

### Blend_LinearDodge
**Path**: `/Engine/Functions/Engine_MaterialFunctions03/Blends/Blend_LinearDodge.Blend_LinearDodge`
**Function**: Linear dodge blend mode (add)

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Base | Vector3 | Base texture |
| Blend | Vector3 | Blend texture |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Linear dodged result |

---

### Blend_LinearLight
**Path**: `/Engine/Functions/Engine_MaterialFunctions03/Blends/Blend_LinearLight.Blend_LinearLight`
**Function**: Linear light blend mode

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Base | Vector3 | Base texture |
| Blend | Vector3 | Blend texture |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Linear light result |

---

### Blend_PinLight
**Path**: `/Engine/Functions/Engine_MaterialFunctions03/Blends/Blend_PinLight.Blend_PinLight`
**Function**: Pin light blend mode

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Base | Vector3 | Base texture |
| Blend | Vector3 | Blend texture |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Pin light result |

---

## Scratch & Grime

### Lerp_ScratchGrime
**Path**: `/Engine/Functions/Engine_MaterialFunctions03/Blends/Lerp_ScratchGrime.Lerp_ScratchGrime`
**Function**: Scratch and grime blend

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Base | Vector3 | Base texture |
| Scratch | Vector3 | Scratch texture |
| Mask | Scalar | Blend mask |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Scratch grime result |

---

### Lerp_ScratchGrime2
**Path**: `/Engine/Functions/Engine_MaterialFunctions03/Blends/Lerp_ScratchGrime2.Lerp_ScratchGrime2`
**Function**: Scratch and grime blend variant 2

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Base | Vector3 | Base texture |
| Scratch | Vector3 | Scratch texture |
| Mask | Scalar | Blend mask |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Scratch grime result |

---

## Height Blending

### HeightLerp
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Texturing/HeightLerp.HeightLerp`
**Function**: Lerp based on height map

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Transition Phase | Scalar | Value between 0-1 |
| Height Texture | Scalar | Grayscale heightmap |
| A | Vector3 | Base texture/color |
| B | Vector3 | Overlay texture/color |
| Contrast | Scalar | Contrast adjustment |

**Outputs**:
| Name | Description |
|------|-------------|
| Results | Lerped result |
| Alpha | Alpha used in lerp |
| Lerp Alpha No Contrast | Alpha without contrast |

---

### HeightLerpWithTwoHeightMaps
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Texturing/HeightLerpWithTwoHeightMaps.HeightLerpWithTwoHeightMaps`
**Function**: Height lerp using two height maps

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Transition Phase | Scalar | Value between 0-1 |
| Height Texture A | Scalar | First heightmap |
| Height Texture B | Scalar | Second heightmap |
| A | Vector3 | Base texture |
| B | Vector3 | Overlay texture |
| Contrast | Scalar | Contrast adjustment |

**Outputs**:
| Name | Description |
|------|-------------|
| Results | Lerped result |
| Alpha | Alpha value |

---

## Normal Blending

### BlendAngleCorrectedNormals
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Utility/BlendAngleCorrectedNormals.BlendAngleCorrectedNormals`
**Function**: Corrects normal direction for overlayed normal maps

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| BaseNormal | Vector3 | Base normal |
| AdditionalNormal | Vector3 | Additional normal |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Blended normal |

---

### BlendAngleCorrectedNormals_WS
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Utility/BlendAngleCorrectedNormals_WS.BlendAngleCorrectedNormals_WS`
**Function**: World space version of angle corrected normal blend

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| BaseNormal | Vector3 | Base normal (world space) |
| AdditionalNormal | Vector3 | Additional normal (world space) |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Blended world space normal |

---

[查看完整路径索引](./index.md)
