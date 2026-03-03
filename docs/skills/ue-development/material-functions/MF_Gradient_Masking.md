# UE Material Functions - Gradient & Masking

> 渐变和遮罩相关 Material Function 详细文档

---

## Linear Gradients

### LinearGradient
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Gradient/LinearGradient.LinearGradient`
**Function**: Uses UV Channel 0 to generate a linear gradient along U or V axis

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| UV Channel | Vector2 | UV coordinates |

**Outputs**:
| Name | Description |
|------|-------------|
| VGradient | V axis gradient |
| UGradient | U axis gradient |

---

## Radial Gradients

### RadialGradient
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Gradient/RadialGradient.RadialGradient`
**Function**: Uses UV Channel 0 to create a radial gradient

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| CenterPosition | Vector2 | Position in UV space |
| Radius | Scalar | Radius in fraction of UV Space |

**Outputs**:
| Name | Description |
|------|-------------|
| RadialGradient | Radial gradient result |

---

### RadialGradientExponential
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Gradient/RadialGradientExponential.RadialGradientExponential`
**Function**: Creates radial gradient with exponential density

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| CenterPosition | Vector2 | Position in UV space |
| Radius | Scalar | Radius in fraction of UV Space |
| UVs | Vector2 | Position in UV space |
| Density | Scalar | Density exponent |
| Invert Density | Unknown | Invert density result |

**Outputs**:
| Name | Description |
|------|-------------|
| RadialGradientExponential | Radial gradient with exponential falloff |

---

### SphereGradient-2D
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Gradient/SphereGradient-2D.SphereGradient-2D`
**Function**: Generates gradient representing depth of a sphere centered on desired point

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| CenterPosition | Vector2 | Position in UV space |
| Radius | Scalar | Radius in fraction of UV Space |
| UVs | Vector2 | Position in UV space |

**Outputs**:
| Name | Description |
|------|-------------|
| Result 0-1 | Normalized gradient result |
| Result Diameter | Diameter result |
| Result Radius | Radius result |

---

### SphereGradient-3D
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Gradient/SphereGradient-3D.SphereGradient-3D`
**Function**: Generates gradient representing depth of a sphere in 3D

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Radius | Scalar | Sphere radius |
| Location | Vector3 | Sphere center location |
| Offset | Vector3 | Offset from location |
| Calculate Camera Inside | Unknown | Extra calculations for camera inside |
| Depth Biased Alpha | Unknown | Enable depth-biased alpha |
| FadeDistance | Scalar | Fade distance for depth bias |

**Outputs**:
| Name | Description |
|------|-------------|
| Result 0-1 | Normalized gradient result |
| Result Diameter | Diameter result |
| Result Radius | Radius result |

---

## Box Masks

### BoxMask-2D
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Gradient/BoxMask-2D.BoxMask-2D`
**Function**: Draws a box in 2D Space with size and falloff controls

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| A | Vector2 | Position coordinates (default: World Position) |
| Bounds | Vector2 | Bounds of the box in X Y |
| B | Vector2 | Center point location of the box |
| Edge Falloff | Scalar | Falloff of edge gradient in units |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Box mask result |

---

### BoxMask-3D
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Gradient/BoxMask-3D.BoxMask-3D`
**Function**: Draws a box in 3D Space with size and falloff controls

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| A | Vector3 | Position coordinates (default: World Position) |
| Bounds | Vector3 | Bounds of the cube in X Y Z |
| B | Vector3 | Center point location of the box |
| Edge Falloff | Scalar | Falloff of edge gradient in units |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Box mask result |

---

## Special Gradients

### BellCurve
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Gradient/BellCurve.BellCurve`
**Function**: Gives a Bell Curve (Normal Distribution)

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| x | Scalar | Input to map to curve |

**Outputs**:
| Name | Description |
|------|-------------|
| 0-1 | Input -1 to 1, returns 0-1 values |
| Standard | Input -pi to pi, area under curve = 1 |

---

### DiamondGradient
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Gradient/DiamondGradient.DiamondGradient`
**Function**: Uses UV Channel 0 to generate a Diamond Gradient

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Falloff | Scalar | Power exponent for diamond |

**Outputs**:
| Name | Description |
|------|-------------|
| DiamondGradient | Diamond shaped gradient |

---

### GeneratedRoundRect
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Gradient/GeneratedRoundRect.GeneratedRoundRect`
**Function**: Generates a rounded rectangle shape

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| UVs | Vector2 | UV coordinates |
| Rect Width | Scalar | Rectangle width |
| Rect Height | Scalar | Rectangle height |
| Corner Radius | Scalar | Corner radius |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Rounded rectangle mask |

---

## Advanced Gradients

### SmoothCurve
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Gradients/SmoothCurve.SmoothCurve`
**Function**: Creates smooth curve transitions

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Phase | Scalar | Curve phase |
| Frequency | Scalar | Curve frequency |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Smooth curve result |

---

### StretchGradient
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Gradients/StretchGradient.StretchGradient`
**Function**: Stretches gradient across specified range

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Gradient Input | Scalar | Input gradient |
| Min | Scalar | Minimum stretch value |
| Max | Scalar | Maximum stretch value |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Stretched gradient |

---

## Masking Functions

### ComponentWise_SphereMask
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Masking/ComponentWise_SphereMask.ComponentWise_SphereMask`
**Function**: Creates sphere mask per component

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| A | Vector3 | Position A |
| B | Vector3 | Position B |
| Radius | Scalar | Sphere radius |
| Hardness | Scalar | Edge hardness |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Component-wise sphere mask |

---

### ThresholdWithRange
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Masking/ThresholdWithRange.ThresholdWithRange`
**Function**: Threshold operation with range control

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Input | Scalar | Input value |
| Threshold | Scalar | Threshold value |
| Range | Scalar | Transition range |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Thresholded result |

---

[查看完整路径索引](./index.md)
