# UE Material Functions - Material Layer Blends

> 材质层混合相关 Material Function 详细文档

---

## Standard Blends

### MatLayerBlend_Standard
**Path**: `/Engine/Functions/MaterialLayerFunctions/MatLayerBlend_Standard.MatLayerBlend_Standard`
**Function**: Blends all attributes of 2 Materials

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Alpha | Scalar | Blend weight |
| Top Material | Unknown | Top material attributes |
| Base Material | Unknown | Base material attributes |

**Outputs**:
| Name | Description |
|------|-------------|
| Blended Material | Blended material attributes |

---

### MatLayerBlend_Simple
**Path**: `/Engine/Functions/MaterialLayerFunctions/MatLayerBlend_Simple.MatLayerBlend_Simple`
**Function**: Simple material layer blend

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Alpha | Scalar | Blend weight |
| Layer Top | Unknown | Top layer |
| Layer Bottom | Unknown | Bottom layer |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Blended layer |

---

### MatLayerBlend_Multiply
**Path**: `/Engine/Functions/MaterialLayerFunctions/MatLayerBlend_Multiply.MatLayerBlend_Multiply`
**Function**: Multiplies two material layers

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Layer A | Unknown | First layer |
| Layer B | Unknown | Second layer |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Multiplied layer |

---

## Tint & Stain

### MatLayerBlend_Tint
**Path**: `/Engine/Functions/MaterialLayerFunctions/MatLayerBlend_Tint.MatLayerBlend_Tint`
**Function**: Tints material layer

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Layer | Unknown | Material layer |
| Tint Color | Vector3 | Tint color |
| Tint Amount | Scalar | Tint strength |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Tinted layer |

---

### MatLayerBlend_Stain
**Path**: `/Engine/Functions/MaterialLayerFunctions/MatLayerBlend_Stain.MatLayerBlend_Stain`
**Function**: Stains material layer

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Layer | Unknown | Material layer |
| Stain Color | Vector3 | Stain color |
| Stain Amount | Scalar | Stain strength |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Stained layer |

---

### MatLayerBlend_TintAllChannels
**Path**: `/Engine/Functions/MaterialLayerFunctions/MatLayerBlend_TintAllChannels.MatLayerBlend_TintAllChannels`
**Function**: Tints all material channels

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Layer | Unknown | Material layer |
| Tint | Vector3 | Tint color |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Tinted layer |

---

## Normal Blending

### MatLayerBlend_NormalBlend
**Path**: `/Engine/Functions/MaterialLayerFunctions/MatLayerBlend_NormalBlend.MatLayerBlend_NormalBlend`
**Function**: Blends normal maps from two layers

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Alpha | Scalar | Blend weight |
| Layer Top | Unknown | Top layer (with normal) |
| Layer Bottom | Unknown | Bottom layer (with normal) |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Layer with blended normals |

---

### MatLayerBlend_TopNormal
**Path**: `/Engine/Functions/MaterialLayerFunctions/MatLayerBlend_TopNormal.MatLayerBlend_TopNormal`
**Function**: Uses top layer's normal only

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Alpha | Scalar | Blend weight |
| Layer Top | Unknown | Top layer |
| Layer Bottom | Unknown | Bottom layer |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Layer with top normal |

---

### MatLayerBlend_NormalFlatten
**Path**: `/Engine/Functions/MaterialLayerFunctions/MatLayerBlend_NormalFlatten.MatLayerBlend_NormalFlatten`
**Function**: Flattens normal based on blend

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Alpha | Scalar | Flatten amount |
| Layer | Unknown | Material layer |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Layer with flattened normal |

---

### MatLayerBlend_ReplaceNormals
**Path**: `/Engine/Functions/MaterialLayerFunctions/MatLayerBlend_ReplaceNormals.MatLayerBlend_ReplaceNormals`
**Function**: Replaces normals with top layer

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Alpha | Scalar | Blend weight |
| Layer Top | Unknown | Top layer |
| Layer Bottom | Unknown | Bottom layer |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Layer with replaced normals |

---

### MatLayerBlend_BlendAngleCorrectedNormals
**Path**: `/Engine/Functions/MaterialLayerFunctions/MatLayerBlend_BlendAngleCorrectedNormals.MatLayerBlend_BlendAngleCorrectedNormals`
**Function**: Angle corrected normal blend for layers

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Alpha | Scalar | Blend weight |
| Layer Top | Unknown | Top layer |
| Layer Bottom | Unknown | Bottom layer |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Layer with corrected normals |

---

### MatLayerBlend_BakedNormal
**Path**: `/Engine/Functions/MaterialLayerFunctions/MatLayerBlend_BakedNormal.MatLayerBlend_BakedNormal`
**Function**: Uses baked normal from layer

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Layer | Unknown | Material layer |
| Baked Normal | Vector3 | Baked normal map |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Layer with baked normal |

---

## Override Functions

### MatLayerBlend_OverrideBaseColor
**Path**: `/Engine/Functions/MaterialLayerFunctions/MatLayerBlend_OverrideBaseColor.MatLayerBlend_OverrideBaseColor`
**Function**: Overrides base color

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Layer | Unknown | Material layer |
| Base Color | Vector3 | New base color |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Layer with overridden base color |

---

### MatLayerBlend_OverrideMetalness
**Path**: `/Engine/Functions/MaterialLayerFunctions/MatLayerBlend_OverrideMetalness.MatLayerBlend_OverrideMetalness`
**Function**: Overrides metalness

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Layer | Unknown | Material layer |
| Metalness | Scalar | New metalness |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Layer with overridden metalness |

---

### MatLayerBlend_OverrideOpacity
**Path**: `/Engine/Functions/MaterialLayerFunctions/MatLayerBlend_OverrideOpacity.MatLayerBlend_OverrideOpacity`
**Function**: Overrides opacity

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Layer | Unknown | Material layer |
| Opacity | Scalar | New opacity |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Layer with overridden opacity |

---

### MatLayerBlend_OverrideOpacityMask
**Path**: `/Engine/Functions/MaterialLayerFunctions/MatLayerBlend_OverrideOpacityMask.MatLayerBlend_OverrideOpacityMask`
**Function**: Overrides opacity mask

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Layer | Unknown | Material layer |
| Opacity Mask | Scalar | New opacity mask |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Layer with overridden opacity mask |

---

### MatLayerBlend_OverrideWorldPositionOffset
**Path**: `/Engine/Functions/MaterialLayerFunctions/MatLayerBlend_OverrideWorldPositionOffset.MatLayerBlend_OverrideWorldPositionOffset`
**Function**: Overrides WPO

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Layer | Unknown | Material layer |
| WPO | Vector3 | New world position offset |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Layer with overridden WPO |

---

### MatLayerBlend_OverrideDisplacement
**Path**: `/Engine/Functions/MaterialLayerFunctions/MatLayerBlend_OverrideDisplacement.MatLayerBlend_OverrideDisplacement`
**Function**: Overrides displacement

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Layer | Unknown | Material layer |
| Displacement | Scalar | New displacement |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Layer with overridden displacement |

---

## Modulate Functions

### MatLayerBlend_ModulateRoughness
**Path**: `/Engine/Functions/MaterialLayerFunctions/MatLayerBlend_ModulateRoughness.MatLayerBlend_ModulateRoughness`
**Function**: Modulates roughness

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Layer | Unknown | Material layer |
| Roughness Multiplier | Scalar | Roughness multiplier |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Layer with modulated roughness |

---

### MatLayerBlend_ModulateSpecular
**Path**: `/Engine/Functions/MaterialLayerFunctions/MatLayerBlend_ModulateSpecular.MatLayerBlend_ModulateSpecular`
**Function**: Modulates specular

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Layer | Unknown | Material layer |
| Specular Multiplier | Scalar | Specular multiplier |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Layer with modulated specular |

---

### MatLayerBlend_MultiplyBaseColor
**Path**: `/Engine/Functions/MaterialLayerFunctions/MatLayerBlend_MultiplyBaseColor.MatLayerBlend_MultiplyBaseColor`
**Function**: Multiplies base color

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Layer | Unknown | Material layer |
| Multiply Color | Vector3 | Color to multiply |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Layer with multiplied base color |

---

## Special Blends

### MatLayerBlend_Decal
**Path**: `/Engine/Functions/MaterialLayerFunctions/MatLayerBlend_Decal.MatLayerBlend_Decal`
**Function**: Decal material layer blend

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Layer | Unknown | Base layer |
| Decal | Unknown | Decal layer |
| Mask | Scalar | Decal mask |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Layer with decal applied |

---

### MatLayerBlend_Emissive
**Path**: `/Engine/Functions/MaterialLayerFunctions/MatLayerBlend_Emissive.MatLayerBlend_Emissive`
**Function**: Adds emissive to layer

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Layer | Unknown | Material layer |
| Emissive Color | Vector3 | Emissive color |
| Emissive Strength | Scalar | Emissive strength |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Layer with emissive |

---

### MatLayerBlend_Displacement
**Path**: `/Engine/Functions/MaterialLayerFunctions/MatLayerBlend_Displacement.MatLayerBlend_Displacement`
**Function**: Adds displacement to layer

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Layer | Unknown | Material layer |
| Displacement | Scalar | Displacement amount |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Layer with displacement |

---

### MatLayerBlend_TenLayerBlend
**Path**: `/Engine/Functions/MaterialLayerFunctions/MatLayerBlend_TenLayerBlend.MatLayerBlend_TenLayerBlend`
**Function**: Blends up to 10 layers

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Layer 1-10 | Unknown | Material layers |
| Weights | Vector4 | Blend weights |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Multi-layer blend result |

---

### MatLayerBlend_BreakOpacity
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/MatLayerBlend_BreakOpacity.MatLayerBlend_BreakOpacity`
**Function**: Extracts opacity from layer

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Layer | Unknown | Material layer |

**Outputs**:
| Name | Description |
|------|-------------|
| Opacity | Extracted opacity |

---

[查看完整路径索引](./index.md)
