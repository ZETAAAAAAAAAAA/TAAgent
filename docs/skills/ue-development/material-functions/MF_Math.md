# UE Material Functions - Math

> 数学运算相关 Material Function 详细文档

---

## Trigonometry & Angle

### DegreesToRadians
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Math/DegreesToRadians.DegreesToRadians`
**Function**: Converts degrees to radians

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Degrees | Scalar | Angle in degrees |

**Outputs**:
| Name | Description |
|------|-------------|
| Radians | Angle in radians |

---

### RadiansToDegrees
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Math/RadiansToDegrees.RadiansToDegrees`
**Function**: Converts radians to degrees

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Radians | Scalar | Angle in radians |

**Outputs**:
| Name | Description |
|------|-------------|
| Degrees | Angle in degrees |

---

### AngleBetweenVectors
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Math/AngleBetweenVectors.AngleBetweenVectors`
**Function**: Find the angle between two normalized vectors in degrees

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Normalized V1 | Vector3 | First normalized vector |
| Normalized V2 | Vector3 | Second normalized vector |

**Outputs**:
| Name | Description |
|------|-------------|
| Angle In Degrees | Angle in degrees |
| 0-1 | Normalized angle (0-1) |

---

## Smooth Functions

### SmoothStep
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/SmoothStep.SmoothStep`
**Function**: HLSL smoothstep function

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Min | Scalar | Minimum value |
| Max | Scalar | Maximum value |
| Alpha | Scalar | Input value |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | 0-1 smooth transition value |

---

### SmoothThreshold
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/SmoothThreshold.SmoothThreshold`
**Function**: Threshold with smooth transition

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Value | Scalar | Input value |
| Threshold | Scalar | Threshold value |
| Smoothness | Scalar | Transition smoothness |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Smooth threshold result |

---

### Step
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Step.Step`
**Function**: HLSL step function

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| X | Scalar | Reference value |
| Y | Scalar | Input value |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | 0 if Y < X, else 1 |

---

### SmoothCeil
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Math/SmoothCeil.SmoothCeil`
**Function**: Smooth ceil function

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Value | Scalar | Input value |
| Smoothness | Scalar | Edge smoothness |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Smooth ceil result |

---

## Range Operations

### RemapValueRange
**Path**: `/Engine/Functions/Engine_MaterialFunctions03/Math/RemapValueRange.RemapValueRange`
**Function**: Remaps a value from one range to another

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Value | Scalar | Input value |
| In Range A | Scalar | Input range min |
| In Range B | Scalar | Input range max |
| Out Range A | Scalar | Output range min |
| Out Range B | Scalar | Output range max |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Remapped value |

---

### WithinRange
**Path**: `/Engine/Functions/Engine_MaterialFunctions03/Math/WithinRange.WithinRange`
**Function**: Returns 1 if value is within range, 0 otherwise

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Value | Scalar | Value to check |
| RangeMax | Scalar | Maximum bound |
| RangeMin | Scalar | Minimum bound |

**Outputs**:
| Name | Description |
|------|-------------|
| AllComponentsWithinRange | 1 if in range, 0 otherwise |

---

### WrapFloat
**Path**: `/Engine/Functions/Engine_MaterialFunctions03/Math/WrapFloat.WrapFloat`
**Function**: Similar to fmod with support for negative floats

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Value | Scalar | Value to wrap |
| Wrap | Scalar | Wrap interval |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Wrapped value (0 to Wrap) |

---

## Color Math

### RGBtoHSV
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Math/RGBtoHSV.RGBtoHSV`
**Function**: Converts RGB colors to HSV

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| RGB | Vector3 | RGB color input |

**Outputs**:
| Name | Description |
|------|-------------|
| RGB to HSV (0-1) | HSV values (H:0-1, S:0-1, V:0-1) |

---

## Arithmetic

### MultiplyAdd
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Math/MultiplyAdd.MultiplyAdd`
**Function**: Modulate add by base and add to base: Result = Base + (Add × Base)

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Add | Vector3 | Value to multiply and add |
| Base | Vector3 | Base value |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Base + (Add × Base) |

---

### AddComponents
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Math/AddComponents.AddComponents`
**Function**: Adds components of a vector

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Vector | Vector3 | Input vector |

**Outputs**:
| Name | Description |
|------|-------------|
| X+Y | X + Y |
| Y+Z | Y + Z |
| X+Z | X + Z |
| X+Y+Z | X + Y + Z |

---

### Pi
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Math/Pi.Pi`
**Function**: Returns the value of Pi

**Inputs**: None

**Outputs**:
| Name | Description |
|------|-------------|
| Pi | 3.14159... |

---

## Matrix Operations

### Transform3x3Matrix
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Math/Transform3x3Matrix.Transform3x3Matrix`
**Function**: Transforms vector by 3x3 matrix

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Vector | Vector3 | Input vector |
| M00-M22 | Scalar | Matrix elements |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Transformed vector |

---

### ConcatenateMatrices
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Math/ConcatenateMatrices.ConcatenateMatrices`
**Function**: Concatenates two matrices

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Matrix A components | Scalar | First matrix |
| Matrix B components | Scalar | Second matrix |

**Outputs**:
| Name | Description |
|------|-------------|
| Result Matrix | Concatenated matrix |

---

## Vector Operations

### CreateThirdOrthogonalVector
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Math/CreateThirdOrthogonalVector.CreateThirdOrthogonalVector`
**Function**: Creates third orthogonal vector from two input vectors

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Vector A | Vector3 | First vector |
| Vector B | Vector3 | Second vector |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Third orthogonal vector |

---

### MakeVectorsOrthogonal
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Math/MakeVectorsOrthogonal.MakeVectorsOrthogonal`
**Function**: Makes two vectors orthogonal to each other

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Vector A | Vector3 | First vector |
| Vector B | Vector3 | Second vector |

**Outputs**:
| Name | Description |
|------|-------------|
| Result A | Modified first vector |
| Result B | Modified second vector |

---

### ProjectVectorOntoPlane
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Math/ProjectVectorOntoPlane.ProjectVectorOntoPlane`
**Function**: Projects a vector onto a plane

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Vector | Vector3 | Vector to project |
| Plane Normal | Vector3 | Plane normal |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Projected vector |

---

### Refract
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Math/Refract.Refract`
**Function**: Calculates refraction vector

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Incident | Vector3 | Incident vector |
| Normal | Vector3 | Surface normal |
| IOR | Scalar | Index of refraction |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Refracted vector |

---

## Quadratic & Intersection

### QuadraticFormula
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Math/QuadraticFormula.QuadraticFormula`
**Function**: Solves quadratic equation

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| a | Scalar | Coefficient a |
| b | Scalar | Coefficient b |
| c | Scalar | Coefficient c |

**Outputs**:
| Name | Description |
|------|-------------|
| X1 | First solution |
| X2 | Second solution |
| Real Roots | Number of real roots |

---

### BoxIntersection
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Math/BoxIntersection.BoxIntersection`
**Function**: Ray-box intersection test

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Ray Origin | Vector3 | Ray origin point |
| Ray Direction | Vector3 | Ray direction |
| Box Min | Vector3 | Box minimum corner |
| Box Max | Vector3 | Box maximum corner |

**Outputs**:
| Name | Description |
|------|-------------|
| Hit | Whether intersection occurred |
| Distance | Distance to intersection |

---

### CylinderIntersection
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Math/CylinderIntersection.CylinderIntersection`
**Function**: Ray-cylinder intersection test

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Ray Origin | Vector3 | Ray origin point |
| Ray Direction | Vector3 | Ray direction |
| Cylinder Center | Vector3 | Cylinder center |
| Cylinder Radius | Scalar | Cylinder radius |
| Cylinder Height | Scalar | Cylinder height |

**Outputs**:
| Name | Description |
|------|-------------|
| Hit | Whether intersection occurred |
| Distance | Distance to intersection |

---

[查看完整路径索引](./index.md)
