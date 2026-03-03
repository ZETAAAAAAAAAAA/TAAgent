# UE Material Functions - Utility

> 工具函数相关 Material Function 详细文档

---

## Vector Construction

### MakeFloat2
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Utility/MakeFloat2.MakeFloat2`
**Function**: Creates a Float2 vector from scalar inputs

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| X | Scalar | X component |
| Y | Scalar | Y component |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Float2 vector |

---

### MakeFloat3
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Utility/MakeFloat3.MakeFloat3`
**Function**: Creates a Float3 vector from scalar inputs

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| X | Scalar | X component |
| Y | Scalar | Y component |
| Z | Scalar | Z component |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Float3 vector |

---

### MakeFloat4
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Utility/MakeFloat4.MakeFloat4`
**Function**: Creates a Float4 vector from scalar inputs

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| X | Scalar | X component |
| Y | Scalar | Y component |
| Z | Scalar | Z component |
| A | Scalar | A (alpha) component |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Float4 vector |

---

## Vector Deconstruction

### BreakOutFloat2Components
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Utility/BreakOutFloat2Components.BreakOutFloat2Components`
**Function**: Breaks Float2 into individual components

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Float2 | Vector2 | Input vector |

**Outputs**:
| Name | Description |
|------|-------------|
| R | Red/X component |
| G | Green/Y component |

---

### BreakOutFloat3Components
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Utility/BreakOutFloat3Components.BreakOutFloat3Components`
**Function**: Breaks Float3 into individual components

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Float3 | Vector3 | Input vector |

**Outputs**:
| Name | Description |
|------|-------------|
| R | Red/X component |
| G | Green/Y component |
| B | Blue/Z component |

---

### BreakOutFloat4Components
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Utility/BreakOutFloat4Components.BreakOutFloat4Components`
**Function**: Breaks Float4 into individual components

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Float4 | Vector4 | Input vector |

**Outputs**:
| Name | Description |
|------|-------------|
| R | Red/X component |
| G | Green/Y component |
| B | Blue/Z component |
| A | Alpha/W component |

---

### SplitComponents
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/SplitComponents.SplitComponents`
**Function**: Splits vector into R, G, B components

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Color | Vector3 | Input color |

**Outputs**:
| Name | Description |
|------|-------------|
| R | Red channel |
| G | Green channel |
| B | Blue channel |

---

## Vector Operations

### VectorLength
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Utility/VectorLength.VectorLength`
**Function**: Returns vector length

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Vector 3 | Vector3 | 3D vector |
| Vector 2 | Vector2 | 2D vector |

**Outputs**:
| Name | Description |
|------|-------------|
| V3 Length | Length of 3D vector |
| V2 Length | Length of 2D vector |

---

### SafeNormalize
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/SafeNormalize.SafeNormalize`
**Function**: Safe normalization avoiding NAN/NULL for zero vectors

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Vector | Vector3 | Input vector |
| Default | Vector3 | Default when length is 0 |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Normalized vector |
| Length==0 | 1 if length is 0 |

---

### Swizzle
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Utility/Swizzle.Swizzle`
**Function**: Swizzles vector components

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| XY | Vector2 | Input Vector2 |
| XYZ | Vector3 | Input Vector3 |

**Outputs**:
| Name | Description |
|------|-------------|
| YX | Swizzled Vector2 (Y,X) |
| YXZ | Swizzled Vector3 (Y,X,Z) |

---

### VectorToRadialValue
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Utility/VectorToRadialValue.VectorToRadialValue`
**Function**: Transforms vector to radial coordinates

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Swizzle Coordinate Output | Unknown | Switch output channel order |
| Vector or UVs | Vector2 | Input vector or UVs |

**Outputs**:
| Name | Description |
|------|-------------|
| Radial Coordinates | Angle and length |
| Vector Converted to Angle | Angle value |
| Linear Distance | Vector length |

---

## Camera & Screen

### CameraDirectionVector
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Utility/CameraDirectionVector.CameraDirectionVector`
**Function**: Returns camera direction vector

**Inputs**: None

**Outputs**:
| Name | Description |
|------|-------------|
| Camera Direction | Camera forward direction |

---

### FOV
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Utility/FOV.FOV`
**Function**: Returns camera FOV

**Inputs**: None

**Outputs**:
| Name | Description |
|------|-------------|
| Field of View | Camera field of view |

---

### ScreenResolution
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/ScreenResolution.ScreenResolution`
**Function**: Returns screen resolution

**Inputs**: None

**Outputs**:
| Name | Description |
|------|-------------|
| Resolution | Screen resolution (X, Y) |

---

## Object Info

### ObjectLocalBounds
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/ObjectLocalBounds.ObjectLocalBounds`
**Function**: Returns object local bounds

**Inputs**: None

**Outputs**:
| Name | Description |
|------|-------------|
| Bounds | Local bounds extent |

---

### ObjectScale
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/WorldPositionOffset/ObjectScale.ObjectScale`
**Function**: Returns object's XYZ scale

**Inputs**: None

**Outputs**:
| Name | Description |
|------|-------------|
| Scale X | X scale |
| Scale Y | Y scale |
| Scale Z | Z scale |
| Scale XYZ | Full scale vector |

---

### ObjectPivotPoint
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/WorldPositionOffset/ObjectPivotPoint.ObjectPivotPoint`
**Function**: Returns object's pivot in world space

**Inputs**: None

**Outputs**:
| Name | Description |
|------|-------------|
| Object Pivot Location | Object pivot in world space |
| Mesh Particle Pivot Location | Mesh particle pivot |

---

## Utility Functions

### DitherTemporalAA
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Utility/DitherTemporalAA.DitherTemporalAA`
**Function**: Stipple pattern for TemporalAA masked translucency

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Alpha Threshold | Scalar | Dither cutoff threshold |
| Random | Scalar | 0=no random, 1=random |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Dithered mask for Opacity Mask |

---

### DynamicBranch
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Utility/DynamicBranch.DynamicBranch`
**Function**: Dynamic branch based on condition

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Condition | Scalar | Branch condition |
| True Value | Vector3 | Value when true |
| False Value | Vector3 | Value when false |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Selected value |

---

### Fresnel_Function
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Fresnel_Function.Fresnel_Function`
**Function**: Calculates Fresnel effect

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

### HueShift
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/HueShift.HueShift`
**Function**: Shifts hue of input color

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Hue Shift | Scalar | Amount to shift hue (0-1) |
| Color | Vector3 | Input color |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Hue-shifted color |

---

### Posterize
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Posterize.Posterize`
**Function**: Posterizes input values

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Value | Vector3 | Input value |
| Steps | Scalar | Number of steps |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Posterized value |

---

### Round
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Utility/Round.Round`
**Function**: Rounds to nearest integer

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Value | Scalar | Input value |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Rounded value |

---

### Sign
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Utility/Sign.Sign`
**Function**: Returns sign of value

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Value | Scalar | Input value |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | -1, 0, or 1 |

---

### LinearSine
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Utility/LinearSine.LinearSine`
**Function**: Linear approximation of sine

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Input | Scalar | Input value |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Linear sine result |

---

### Sine_Remapped
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Utility/Sine_Remapped.Sine_Remapped`
**Function**: Sine remapped to 0-1

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Input | Scalar | Input value |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Sine remapped to 0-1 |

---

## Debug Functions

### DebugScalarValues
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Utility/DebugScalarValues.DebugScalarValues`
**Function**: Display scalar value as numbers in UV space

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Number | Scalar | Value to display |
| MaximumNumberOfDigits | Scalar | Max digits |
| UVs | Vector2 | UV coordinates |
| DebugTextLocation RG_UpperRight BA_LowerLeft | Vector4 | Text position |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Debug visualization |

---

### DebugFloat3Values
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Utility/DebugFloat3Values.DebugFloat3Values`
**Function**: Display vector values as numbers

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| UVs | Vector2 | UV coordinates |
| DebugTextLocation RG_UpperRight BA_LowerLeft | Vector4 | Text position |
| MaximumNumberOfDigits | Scalar | Max digits |
| Vector3 | Vector3 | Vector to display |
| Component Spacing | Vector4 | Spacing between components |

**Outputs**:
| Name | Description |
|------|-------------|
| ColorCodedOutput | RGB color coded |
| GreyScaleOutput | Grayscale output |

---

### DebugOnOff
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Utility/DebugOnOff.DebugOnOff`
**Function**: Simple on/off debug switch

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Input | Vector3 | Input value |
| Enabled | Scalar | Enable switch |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Input or zero based on enabled |

---

[查看完整路径索引](./index.md)
