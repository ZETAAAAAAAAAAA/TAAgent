# UE Material Functions - Image Adjustment

> 图像调整相关 Material Function 详细文档

---

## Contrast

### CheapContrast
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/ImageAdjustment/CheapContrast.CheapContrast`
**Function**: Cheaply adds contrast similar to Photoshop Levels

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| In | Scalar | Input value |
| Contrast | Scalar | Contrast amount (0=default) |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Contrast-adjusted result |

---

### CheapContrast_RGB
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/ImageAdjustment/CheapContrast_RGB.CheapContrast_RGB`
**Function**: Cheap contrast per RGB channel

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| In | Vector3 | Input RGB |
| Contrast | Vector3 | Contrast per channel |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Contrast-adjusted RGB |

---

### Contrast_Preserve_Color
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/ImageAdjustment/Contrast_Preserve_Color.Contrast_Preserve_Color`
**Function**: Contrast adjustment that preserves color

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Color | Vector3 | Input color |
| Contrast | Scalar | Contrast amount |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Contrast with preserved hue |

---

## Color Correction

### ColorCorrection
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/ImageAdjustment/ColorCorrection.ColorCorrection`
**Function**: Full color correction with lift/gamma/gain controls

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Color | Vector3 | Input color |
| Gain | Vector3 | 3-way gain control |
| Gamma | Vector3 | 3-way gamma control |
| Lift | Vector3 | 3-way lift control |
| Contrast | Scalar | Contrast adjustment |
| Saturation | Scalar | Saturation adjustment |
| Hue Shift | Scalar | Hue rotation |
| Brightness | Scalar | Brightness adjustment |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Corrected color |

---

### 3ColorBlend
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/ImageAdjustment/3ColorBlend.3ColorBlend`
**Function**: Blends between 3 colors based on greyscale input

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Alpha | Scalar | Blend weight (0-1) |
| A | Vector3 | First color |
| B | Vector3 | Second color |
| C | Vector3 | Third color |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Blended color |

---

## Luminosity

### Luminosity_And_Color
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/ImageAdjustment/Luminosity_And_Color.Luminosity_And_Color`
**Function**: Extracts luminosity and color

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Color | Vector3 | Input color |

**Outputs**:
| Name | Description |
|------|-------------|
| Luminosity | Luminosity value |
| Color | Color without luminosity |

---

## Gamma Correction

### LinearTosRGB
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/ImageAdjustment/LinearTosRGB.LinearTosRGB`
**Function**: Converts linear to sRGB

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Linear | Vector3 | Linear color |

**Outputs**:
| Name | Description |
|------|-------------|
| sRGB | sRGB color |

---

### sRGBToLinear
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/ImageAdjustment/sRGBToLinear.sRGBToLinear`
**Function**: Converts sRGB to linear

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| sRGB | Vector3 | sRGB color |

**Outputs**:
| Name | Description |
|------|-------------|
| Linear | Linear color |

---

## High Pass

### HighPassFunction
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/ImageAdjustment/HighPassFunction.HighPassFunction`
**Function**: High pass filter function

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Color | Vector3 | Input color |
| UVs | Vector2 | UV coordinates |
| Size | Scalar | Filter size |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | High pass result |

---

### HighPassTexture
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/ImageAdjustment/HighPassTexture.HighPassTexture`
**Function**: High pass filter on texture

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Texture | Unknown | Input texture |
| UVs | Vector2 | UV coordinates |
| Size | Scalar | Filter size |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | High pass result |

---

## Curves

### SCurve
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/ImageAdjustment/SCurve.SCurve`
**Function**: S-curve adjustment

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| In | Scalar | Input value |
| Slope | Scalar | Curve slope |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | S-curve adjusted value |

---

## HDR

### DeriveHDRfromLDR
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/ImageAdjustment/DeriveHDRfromLDR.DeriveHDRfromLDR`
**Function**: Derives HDR from LDR input

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| LDR Color | Vector3 | LDR color input |
| Exposure | Scalar | Exposure adjustment |

**Outputs**:
| Name | Description |
|------|-------------|
| HDR Color | Derived HDR color |

---

## Unsharp Mask

### UnSharpMaskFunction
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/ImageAdjustment/UnSharpMaskFunction.UnSharpMaskFunction`
**Function**: Unsharp mask function

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Color | Vector3 | Input color |
| Amount | Scalar | Sharpening amount |
| Threshold | Scalar | Threshold |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Sharpened result |

---

### UnSharpMaskTexture
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/ImageAdjustment/UnSharpMaskTexture.UnSharpMaskTexture`
**Function**: Unsharp mask on texture

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Texture | Unknown | Input texture |
| Amount | Scalar | Sharpening amount |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Sharpened result |

---

## Blur

### BlurSampleOffsets
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/ImageAdjustment/BlurSampleOffsets.BlurSampleOffsets`
**Function**: Generates blur sample offsets

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| BlurSize | Scalar | Blur radius |
| SampleCount | Scalar | Number of samples |

**Outputs**:
| Name | Description |
|------|-------------|
| Offsets | Sample offset positions |

---

### SceneTextureAverage
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/ImageAdjustment/SceneTextureAverage.SceneTextureAverage`
**Function**: Averages scene texture samples

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| UVs | Vector2 | UV coordinates |
| SampleCount | Scalar | Number of samples |

**Outputs**:
| Name | Description |
|------|-------------|
| Average | Averaged color |

---

## 3-Point Levels

### 3PointLevels
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/3PointLevels.3PointLevels`
**Function**: 3-point levels adjustment (black, mid, white)

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| In | Scalar | Input value |
| Black Point | Scalar | Black point |
| Mid Point | Scalar | Mid point |
| White Point | Scalar | White point |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Levels adjusted result |

---

[查看完整路径索引](./index.md)
