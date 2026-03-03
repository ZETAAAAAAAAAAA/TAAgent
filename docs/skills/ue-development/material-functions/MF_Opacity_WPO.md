# UE Material Functions - Opacity & World Position Offset

> 透明度和世界位置偏移相关 Material Function 详细文档

---

## Opacity Functions

### CameraDepthFade
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Opacity/CameraDepthFade.CameraDepthFade`
**Function**: Creates gradient from 0 near camera to 1 at Fade Length

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Fade Length | Scalar | Fade distance (units) |
| Fade Offset | Scalar | Offset value |
| For Vertex Shader | Unknown | For WPO shader |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | 0-1 fade value |

---

### AlphaOffset
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Opacity/AlphaOffset.AlphaOffset`
**Function**: Applies offset to alpha while keeping range 0-1

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Alpha | Scalar | Input alpha |
| Offset | Scalar | Offset amount |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Offset alpha (0-1) |

---

### SoftOpacity
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Opacity/SoftOpacity.SoftOpacity`
**Function**: Softens Opacity by multiplying against Fresnel, DepthBiasedAlpha, PixelDepth

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| OpacityIn | Scalar | Opacity to be softened |
| FadeDistance | Scalar | Distance before fading (default=512) |
| DepthFadeDistance | Scalar | Depth fade distance |

**Outputs**:
| Name | Description |
|------|-------------|
| OutputUsesDepthBias | Result with depth bias |
| OutputNoDepthBias | Result without depth bias |

---

### Chroma_Key_Alpha
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Opacity/Chroma_Key_Alpha.Chroma_Key_Alpha`
**Function**: Creates alpha from chroma keying

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Color | Vector3 | Input color |
| Key Color | Vector3 | Color to key out |
| Threshold | Scalar | Key threshold |

**Outputs**:
| Name | Description |
|------|-------------|
| Alpha | Chroma key alpha |

---

## Wind Functions

### SimpleGrassWind
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/WorldPositionOffset/SimpleGrassWind.SimpleGrassWind`
**Function**: Simple waving grass wind (must be last WPO applied)

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| AdditionalWPO | Vector3 | Additional WPO |
| WindIntensity | Scalar | Wind intensity |
| WindWeight | Scalar | Wind weight (vertex color/texture) |
| WindSpeed | Scalar | Wind speed |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | WPO offset value |

---

### Wind
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/WorldPositionOffset/Wind.Wind`
**Function**: Provides wind speed, strength, and direction

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| WindActor | Vector4 | Wind actor reference |

**Outputs**:
| Name | Description |
|------|-------------|
| WindSpeed | Speed × time |
| Wind Strength | Wind magnitude |
| Normalized Wind Vector | Direction vector |
| WindActor | Standard wind actor |

---

### CanopyCreator_Branches
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/WorldPositionOffset/CanopyCreator_Branches.CanopyCreator_Branches`
**Function**: Creates canopy branch wind movement

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Branch Parameters | Vector4 | Branch configuration |
| Wind Parameters | Vector3 | Wind settings |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Canopy WPO offset |

---

## Scale Functions

### ConstantScalebyDistance
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/WorldPositionOffset/ConstantScalebyDistance.ConstantScalebyDistance`
**Function**: Keeps quad at constant scale on screen

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| SizeScale | Scalar | Scale multiplier |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | WPO offset for constant screen scale |

---

### FoliageScaleFactor
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/WorldPositionOffset/FoliageScaleFactor.FoliageScaleFactor`
**Function**: Scale factor for foliage

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Base Scale | Scalar | Base scale value |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Foliage scale factor |

---

## Position Functions

### LocalPosition
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/WorldPositionOffset/LocalPosition.LocalPosition`
**Function**: Returns local position

**Inputs**: None

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Local position |

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

### ComponentPivotLocation
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/WorldPositionOffset/ComponentPivotLocation.ComponentPivotLocation`
**Function**: Returns component pivot location

**Inputs**: None

**Outputs**:
| Name | Description |
|------|-------------|
| Pivot Location | Component pivot in world space |

---

## Camera Alignment

### AlignMeshToTheCamera
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/WorldPositionOffset/AlignMeshToTheCamera.AlignMeshToTheCamera`
**Function**: Aligns mesh to face camera

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Position | Vector3 | World position |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | WPO for camera alignment |

---

### AttachMeshToTheCamera
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/WorldPositionOffset/AttachMeshToTheCamera.AttachMeshToTheCamera`
**Function**: Attaches mesh to camera position

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Offset | Vector3 | Offset from camera |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | WPO to attach to camera |

---

### CameraOffset
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/WorldPositionOffset/CameraOffset.CameraOffset`
**Function**: Offsets mesh toward/away from camera

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Offset | Scalar | Offset distance |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Camera offset WPO |

---

## Rotation

### RotateAboutWorldAxis_cheap
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/WorldPositionOffset/RotateAboutWorldAxis_cheap.RotateAboutWorldAxis_cheap`
**Function**: Cheap rotation about world axis

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Rotation X | Scalar | X rotation |
| Rotation Y | Scalar | Y rotation |
| Rotation Z | Scalar | Z rotation |
| Position | Vector3 | World position |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Rotated position offset |

---

### PivotAxis
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/WorldPositionOffset/PivotAxis.PivotAxis`
**Function**: Creates rotation around pivot axis

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Axis | Vector3 | Rotation axis |
| Angle | Scalar | Rotation angle |
| Pivot | Vector3 | Pivot point |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Pivot rotation WPO |

---

## Sprite/Billboard

### Sprite
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/WorldPositionOffset/Sprite.Sprite`
**Function**: Creates sprite/billboard effect

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Scale | Vector2 | Sprite scale |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Sprite WPO |

---

### Sprite_Capsule
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/WorldPositionOffset/Sprite_Capsule.Sprite_Capsule`
**Function**: Creates capsule-shaped sprite

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Scale | Vector3 | Capsule scale |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Capsule sprite WPO |

---

### Sprite_Ellipsoid
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/WorldPositionOffset/Sprite_Ellipsoid.Sprite_Ellipsoid`
**Function**: Creates ellipsoid sprite

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Scale | Vector3 | Ellipsoid scale |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Ellipsoid sprite WPO |

---

## Deformation

### SplineThicken
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/WorldPositionOffset/SplineThicken.SplineThicken`
**Function**: Thickens along spline

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Thickness | Scalar | Thickness amount |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Spline thickening WPO |

---

### SplineBasedModelDeformation
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/WorldPositionOffset/SplineBasedModelDeformation.SplineBasedModelDeformation`
**Function**: Deforms model based on spline

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Spline Data | Vector4 | Spline information |
| Influence | Scalar | Deformation strength |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Spline deformation WPO |

---

### GenerateASpline
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/WorldPositionOffset/GenerateASpline.GenerateASpline`
**Function**: Generates spline data

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Points | Vector3 | Control points |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Generated spline data |

---

## Gravity

### Gravity_WPO
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/WorldPositionOffset/Gravity_WPO.Gravity_WPO`
**Function**: Applies gravity to vertices

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Gravity Strength | Scalar | Gravity amount |
| Mask | Scalar | Influence mask |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Gravity WPO |

---

## Morphing

### StaticMeshMorphTargets
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/WorldPositionOffset/StaticMeshMorphTargets.StaticMeshMorphTargets`
**Function**: Applies morph target deformation

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Morph Target | Vector3 | Morph target delta |
| Weight | Scalar | Morph weight |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Morph target WPO |

---

[查看完整路径索引](./index.md)
