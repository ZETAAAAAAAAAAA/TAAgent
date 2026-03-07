# UE Material Functions - Shading & Lighting

> 着色和光照相关 Material Function 详细文档

---

## Shading Models

### FuzzyShading
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Shading/FuzzyShading.FuzzyShading`
**Function**: Emulates shading similar to velvet or moss

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Normal | Vector3 | Surface normal |
| CoreDarkness | Scalar | Darkness of the core |
| EdgeBrightness | Scalar | Brightness at edges |
| BaseColor | Vector3 | Base color input |
| Power | Scalar | Power exponent |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Fuzzy shaded result |

---

### FuzzyShadingGrass
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Shading/FuzzyShadingGrass.FuzzyShadingGrass`
**Function**: Diffuse portion of grass shading

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Diffuse | Vector3 | Diffuse color |
| CoreDarkness | Scalar | Darkness of the core |
| Normal | Vector3 | Surface normal |
| EdgeBrightness | Scalar | Brightness at edges |
| Power | Scalar | Power exponent |
| EdgeDesat | Scalar | Edge desaturation |
| EdgeColor | Vector3 | Edge tint color |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Grass shaded result |

---

### MetallicShading
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Shading/MetallicShading.MetallicShading`
**Function**: Apply to base color of metals for more interesting shading

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| BaseColor | Vector3 | Base color input |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Enhanced metallic shading |

---

### PowerToRoughness
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Shading/PowerToRoughness.PowerToRoughness`
**Function**: Converts specular power to roughness

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Power | Scalar | Specular power value |

**Outputs**:
| Name | Description |
|------|-------------|
| Roughness | Converted roughness value |

---

## Fresnel

### AxisAlignedFresnel
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Shading/AxisAlignedFresnel.AxisAlignedFresnel`
**Function**: Fresnel falloff perpendicular to specified axis

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Axis | Vector3 | Axis to align to (default: local Z) |
| Normal | Vector3 | Custom normal (world space) |
| Exponent | Scalar | Power exponent |

**Outputs**:
| Name | Description |
|------|-------------|
| Fresnel | Fresnel result with power applied |
| Cylinder Thickness | Ray travel distance through cylinder |

---

### Fresnel_Function
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Fresnel_Function.Fresnel_Function`
**Function**: Standard Fresnel calculation

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Exponent | Scalar | Fresnel power |
| Base Reflect Fraction | Scalar | Base reflectivity |
| Normal | Vector3 | Surface normal |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Fresnel value (0-1) |

---

## Ambient Occlusion

### Sphere_AO
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Shading/Sphere_AO.Sphere_AO`
**Function**: Calculates AO for a sphere affecting the world

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Sphere Position | Vector3 | Sphere center position |
| Source Radius | Scalar | Sphere radius in world units |

**Outputs**:
| Name | Description |
|------|-------------|
| AO Value | Ambient occlusion value |

---

## Specular

### BlinnPhongSpecular
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Lighting/BlinnPhongSpecular.BlinnPhongSpecular`
**Function**: Returns a specular lobe using Blinn-Phong

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Normal | Vector3 | Surface normal |
| Light Vector | Vector3 | Light direction |
| Glossiness | Scalar | Surface glossiness |
| ViewVector | Vector3 | View direction |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Specular highlight |

---

### GGXSpecular
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Lighting/GGXSpecular.GGXSpecular`
**Function**: Returns a specular lobe using GGX distribution

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Normal | Vector3 | Surface normal |
| Light Vector | Vector3 | Light direction |
| Roughness | Scalar | Surface roughness |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | GGX specular result |

---

### TextureDefinedSpecularShape
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Shading/TextureDefinedSpecularShape.TextureDefinedSpecularShape`
**Function**: Creates specular shape from texture

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Texture | Unknown | Specular shape texture |
| Normal | Vector3 | Surface normal |
| Roughness | Scalar | Surface roughness |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Texture-defined specular |

---

## Shadows

### Sphere-ConeShadow-Texture
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Shading/Sphere-ConeShadow-Texture.Sphere-ConeShadow-Texture`
**Function**: Sphere cone shadow from texture

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Texture | Unknown | Shadow texture |
| Position | Vector3 | Sphere position |
| Radius | Scalar | Sphere radius |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Shadow result |

---

### Ellipsoid-ConeShadow-Texture
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Shading/Ellipsoid-ConeShadow-Texture.Ellipsoid-ConeShadow-Texture`
**Function**: Ellipsoid cone shadow from texture

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Texture | Unknown | Shadow texture |
| Position | Vector3 | Ellipsoid position |
| Scale | Vector3 | Ellipsoid scale |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Shadow result |

---

## Pixel Depth Offset

### PixelDepthOffset_Foliage
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Lighting/PixelDepthOffset_Foliage.PixelDepthOffset_Foliage`
**Function**: Pixel depth offset optimized for foliage

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Offset | Scalar | Depth offset amount |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Pixel depth offset value |

---

## Converters

### ConvertFromDiffSpec
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Shading/ConvertFromDiffSpec.ConvertFromDiffSpec`
**Function**: Converts from Diffuse/Specular to BaseColor/Metallic

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Diffuse | Vector3 | Diffuse color |
| Specular | Vector3 | Specular color |

**Outputs**:
| Name | Description |
|------|-------------|
| BaseColor | Converted base color |
| Metallic | Converted metallic value |

---

### CalcLightsourceAngle
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Shading/CalcLightsourceAngle.CalcLightsourceAngle`
**Function**: Calculates light source angle

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Light Vector | Vector3 | Light direction |
| Normal | Vector3 | Surface normal |

**Outputs**:
| Name | Description |
|------|-------------|
| Angle | Angle in radians |

---

[查看完整路径索引](./index.md)
