# UE Material Functions - Texturing

> 纹理处理相关 Material Function 详细文档

---

## World Aligned Textures

### WorldAlignedTexture
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Texturing/WorldAlignedTexture.WorldAlignedTexture`
**Function**: Tiles a texture in worldspace - Triplanar texture projection

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| World Space Normal | Vector3 | World space normal |
| TextureObject | Unknown | Texture object (not sampled) |
| TextureSize | Vector3 | Size of texture in worldspace units |
| WorldPosition | Vector3 | World position |
| Export Float 4 | Unknown | Default = False |
| ProjectionTransitionContrast | Scalar | Projection transition contrast |

**Outputs**:
| Name | Description |
|------|-------------|
| XYZ Texture | Triplanar blended result |
| XY Texture | XY plane result |
| Z Texture | Z plane result |

---

### WorldAlignedNormal
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Texturing/WorldAlignedNormal.WorldAlignedNormal`
**Function**: World space aligned normal mapping

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| TextureObject | Unknown | Normal texture object |
| TextureSize | Vector3 | Texture world size |
| WorldPosition | Vector3 | World position |

**Outputs**:
| Name | Description |
|------|-------------|
| XYZ Normal | Triplanar normal result |

---

### WorldAlignedNormal2
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Texturing/WorldAlignedNormal2.WorldAlignedNormal2`
**Function**: Tiles a texture in worldspace

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Normal | Vector3 | Normal input |
| TextureObject | Unknown | Texture object |
| TextureSize | Vector3 | Size of texture in worldspace units |

**Outputs**:
| Name | Description |
|------|-------------|
| Z Texture | Z plane result |
| XY Texture | XY plane result |
| XYZ Texture | Triplanar blended result |
| XYZFlatTop | Flat top result |

---

### WorldAlignedTexture_MipBias
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Texturing/WorldAlignedTexture_MipBias.WorldAlignedTexture_MipBias`
**Function**: Tiles a texture in worldspace with MipBias control

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| World Space Normal | Vector3 | World space normal |
| TextureObject | Unknown | Texture object |
| TextureSize | Vector3 | Size of texture in worldspace units |
| WorldPosition | Vector3 | World position |
| Export Float 4 | Unknown | Default = False |
| ProjectionTransitionContrast | Scalar | Projection transition contrast |
| MipBias | Scalar | Mip bias value |

**Outputs**:
| Name | Description |
|------|-------------|
| XYZ Texture | Triplanar blended result |
| XY Texture | XY plane result |
| Z Texture | Z plane result |

---

### LocalAlignedTexture
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Texturing/LocalAlignedTexture.LocalAlignedTexture`
**Function**: Tiles a texture in local object space

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| TextureObject | Unknown | Texture object |
| Normal | Vector3 | Normal input |

**Outputs**:
| Name | Description |
|------|-------------|
| Z Texture | Z plane result |
| XY Texture | XY plane result |
| XYZ Texture | Triplanar blended result |

---

### WorldCoordinate3Way
**Path**: `/Engine/Functions/Engine_MaterialFunctions03/Texturing/WorldCoordinate3Way.WorldCoordinate3Way`
**Function**: Takes textures and projects them in world-space onto the surface

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| XY Scale | Scalar | Scale of XY projected texture in world units |
| XZ Scale | Scalar | Scale of XZ projected texture in world units |
| YZ Scale | Scalar | Scale of YZ projected texture in world units |
| XY Texture | Unknown | XY plane texture |
| XZ Texture | Unknown | XZ plane texture |
| YZ Texture | Unknown | YZ plane texture |
| Normal | Vector3 | Brings the normal into the blend |
| Blend Exponent Y Z | Scalar | Power for blend between sides |
| Single Texture | Unknown | Use single texture for all projections |
| Blend Exponent X | Scalar | Power for blend between top and sides |
| Blend Mult X | Scalar | Multiplies blend between top and sides |
| Blend Mult Y Z | Scalar | Multiplies blend between side projections |
| WorldPosition | Vector3 | Optional altered world position |

**Outputs**:
| Name | Description |
|------|-------------|
| XYZ Output | 3 way blended texturing |
| XY | XY Plane texturing |
| XZ | XZ Plane texturing |
| YZ | YZ Plane texturing |

---

## UV Operations

### ScaleUVsByCenter
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Texturing/ScaleUVsByCenter.ScaleUVsByCenter`
**Function**: Scales UVs from center point

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Texture Scale | Vector2 | Scale factor. Default = 1 |
| UVs | Vector2 | UVs. Only need to plug in if you want a different UV channel |

**Outputs**:
| Name | Description |
|------|-------------|
| UVs | Scaled UV coordinates |
| 0-1 mask | Mask output |

---

### BrickAndTileUVs
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Texturing/BrickAndTileUVs.BrickAndTileUVs`
**Function**: Creates semi procedural UV offset pattern for patterned textures

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Tiles X | Scalar | Number of tiles on X axis |
| Tiles Y | Scalar | Number of tiles on Y axis |
| Tile Shift | Scalar | Stagger amount |
| Texture Tiling | Scalar | Texture tiling scale |
| Rotate 90 | Unknown | Rotate 90 degrees switch |
| Macro Tiling | Scalar | Macro tiling scale |
| Pattern Distortion | Vector2 | Distort random tile edge pattern |
| UV | Vector2 | UV coordinates |

**Outputs**:
| Name | Description |
|------|-------------|
| DiffuseUVs | Diffuse texture UVs |
| MacroUVs | Macro texture UVs |
| TileUVs | Individual tile UVs |
| Macro Offsets only | Macro offset values |

---

### CylindricalUVs
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Texturing/CylindricalUVs.CylindricalUVs`
**Function**: Tiles a texture using Cylindrical UVs centered around Object Center

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Normal | Vector3 | Normal input |
| TextureObject | Unknown | Texture object |
| In | Scalar | UVHeight |

**Outputs**:
| Name | Description |
|------|-------------|
| Cylinder Projection w Top | Cylindrical projection with top cap |
| Cylinder Projection | Cylindrical projection without top |

---

### ScreenAlignedUVs
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Texturing/ScreenAlignedUVs.ScreenAlignedUVs`
**Function**: Maps the 0-1 uv range to the screen

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Texture Dimensions | Vector2 | Resolution of the texture for scaled outputs |

**Outputs**:
| Name | Description |
|------|-------------|
| X 100%, Y 100% | X and Y scaled to fit screen |
| X Scale to Ratio, Y 100% | X center scaled to maintain aspect ratio |
| X 100%, Y Scale to Ratio | Y center scaled to maintain resolution |

---

### TextureCropping
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Texturing/TextureCropping.TextureCropping`
**Function**: Crops a texture, useful for Emissives

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| UpperLeftCorner | Vector2 | New UV location for UV value (0,0) |
| LowerRightCorner | Vector2 | New location for UV location (1,1) |
| TextureIn | Unknown | Texture to be positioned (set Address to Clamp) |
| UVs | Vector2 | Default is texture coordinate 0 |
| ExportFloat4 | Unknown | False=RGB, True=RGBA result |

**Outputs**:
| Name | Description |
|------|-------------|
| CroppedMasked | Cropped with mask (11 instructions) |
| Cropped | Cropped result (7 instructions) |
| Crop UVs | Crop UV coordinates |
| Crop Mask | Crop mask only |

---

### TwoSidedTexturing
**Path**: `/Engine/Functions/Engine_MaterialFunctions03/Texturing/TwoSidedTexturing.TwoSidedTexturing`
**Function**: Independent texturing control for both sides of a two-sided material

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Texture Side B | Vector3 | Input for second side |
| Texture Side A | Vector3 | Input for first side |

**Outputs**:
| Name | Description |
|------|-------------|
| Texture Blend Output | Blended result |
| Mask | Side mask output |

---

## Parallax & Displacement

### ParallaxOcclusionMapping
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Texturing/ParallaxOcclusionMapping.ParallaxOcclusionMapping`
**Function**: Uses Ray Tracing to find correct offsets for parallax effect

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Min Steps | Scalar | Steps when looking down (fewer) |
| Max Steps | Scalar | Steps at glancing angles (more) |
| Height Ratio | Scalar | Heightmap depth ratio, typical 0.05-0.1 |
| Heightmap Texture | Unknown | Heightmap texture object |
| UVs | Vector2 | UV coordinates |
| Heightmap Channel | Vector4 | Heightmap channel, default Alpha |
| Reference Plane | Scalar | Parallax reference plane, 1=down, 0=up |
| Light Vector | Vector3 | Light direction for shadow calculation |
| Shadow Steps | Scalar | Shadow steps |
| Shadow Penumbra | Scalar | Shadow softness |

**Outputs**:
| Name | Description |
|------|-------------|
| Parallax UVs | Offset UVs |
| Pixel Depth Offset | Pixel depth offset |
| Offset Only | Offset value only |
| Shadow | Shadow result |
| World Position | World position |

---

## Normal Mapping

### FlattenNormal
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Texturing/FlattenNormal.FlattenNormal`
**Function**: Lerps a NormalMap with (0,0,1) to flatten it

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Normal | Vector3 | Normal input |
| Flatness | Scalar | Flatness amount (0-1) |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Flattened normal |

---

### BlendAngleCorrectedNormals
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Utility/BlendAngleCorrectedNormals.BlendAngleCorrectedNormals`
**Function**: Corrects normal direction of normal map overlayed on other normal maps

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| BaseNormal | Vector3 | Base normal |
| AdditionalNormal | Vector3 | Additional normal to blend |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Blended normal |

---

## Detail Texturing

### DetailTexturing
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Texturing/DetailTexturing.DetailTexturing`
**Function**: Applies detail textures to diffuse and normal

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Diffuse | Vector3 | Base diffuse |
| Normal | Vector3 | Base normal |
| Scale | Scalar | Detail scale |
| NormalIntensity | Scalar | Normal blend intensity |
| DiffuseIntensity | Scalar | Diffuse blend intensity |
| DetailDiffuse | Unknown | Detail diffuse texture (highpass, sRGB off) |
| DetailNormal | Unknown | Detail normal texture |

**Outputs**:
| Name | Description |
|------|-------------|
| Diffuse | Blended diffuse (multiply) |
| Normal | Blended normal (add) |

---

## Texture Bombing

### Texture_Bombing
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Texturing/Texture_Bombing.Texture_Bombing`
**Function**: Uses multiple offset texture samples to break up tiling artifacts

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Texture Object | Unknown | Texture to bomb |
| UVs | Vector2 | UV coordinates |
| Tiling | Scalar | Tiling amount |
| Offset | Scalar | Strength of random offsets |
| Optional Heightmap | Unknown | Heightmap for height lerp |
| Contrast | Scalar | Blend contrast |
| Enable Height Lerp | Unknown | Enable height-based blending |
| Is Normalmap | Unknown | Set TRUE for normal maps |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Bombed texture result |

---

## Flow Maps

### TangentSpaceFlow
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Texturing/TangentSpaceFlow.TangentSpaceFlow`
**Function**: Pushes texture along flow direction in tangent space

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| FlowSpeed | Scalar | Flow animation speed |
| FlowTexture | Unknown | Texture to flow |
| FlowStrength | Scalar | Strength of flow effect |
| FlowDirection | Vector2 | Flow direction in tangent space (-1 to 1) |
| UVs | Vector2 | UV coordinates |
| Export Float 4 | Unknown | Export texture alpha with RGB |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Flowing texture result |

---

### FlowMaps
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Texturing/FlowMaps.FlowMaps`
**Function**: Pushes Diffuse and Normal textures along flow map vectors

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Diffuse | Unknown | Diffuse texture |
| Flow Vector Map | Vector2 | Flow direction (-1 to 1) |
| UVs | Vector2 | UV coordinates |
| Time | Scalar | Animation time |
| UV texture 2 offset | Vector2 | UV offset |
| Normal Map | Unknown | Normal map texture |
| Texture Mip Bias | Scalar | Blur amount for shorelines |
| Texture Size | Vector2 | Texture XY size |
| useMipLevel | Unknown | Mip level mode |

**Outputs**:
| Name | Description |
|------|-------------|
| Diffuse | Flowing diffuse |
| Diffuse Alpha | Diffuse alpha channel |
| Normal | Flowing normal |
| Distortion | Distortion values |
| UVs | Modified UVs |

---

## Flipbook

### FlipBook
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Texturing/FlipBook.FlipBook`
**Function**: Animates a texture in a flipbook fashion

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Texture | Unknown | Texture object |
| Animation Phase (0-1) | Scalar | Animation phase (apply frac for time) |
| UVs | Vector2 | Texture coordinates |
| Number of Columns | Scalar | Horizontal image count |
| Number of Rows | Scalar | Vertical image count |
| Clamp Anim | Unknown | Clamp phase to 0-0.9999 |
| MipBias/Level | Scalar | Mip control |
| Use Mip Bias (T) Level (F) | Unknown | Mip mode switch |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Sampled texture result |
| UVs | Calculated UVs |
| Alpha | Alpha channel |
| UV Center | Center UV for rotation |

---

## Masking & Distance

### BitMask
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Texturing/BitMask.BitMask`
**Function**: Provides a 1bit mask from a Grayscale image

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| BitMask | Scalar | Texture using TC_Grayscale |
| Bit | Scalar | Value between 0-255 |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | 0 or 1 mask output |

---

### SlopeMask
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Texturing/SlopeMask.SlopeMask`
**Function**: Slope mask useful for snow, moss effects

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| TangentNormal | Vector3 | Tangent space normal |
| CheapContrast | Scalar | Contrast amount |
| FalloffPower | Scalar | Falloff power |
| SlopAngle | Vector3 | World direction for slope (typically down) |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Slope mask result |

---

### DistanceField
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Texturing/DistanceField.DistanceField`
**Function**: Reads a Distance field texture

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| EdgeSoftness | Scalar | Edge softness |
| DistanceField | Scalar | Distance field value |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Distance field result |

---

## Height Blending

### HeightLerp
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Texturing/HeightLerp.HeightLerp`
**Function**: Lerp between two values based on height map

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Transition Phase | Scalar | Value between 0-1 |
| Height Texture | Scalar | Grayscale heightmap (0-1) |
| A | Vector3 | Base texture/color |
| B | Vector3 | Overlay texture/color |
| Contrast | Scalar | Contrast adjustment |

**Outputs**:
| Name | Description |
|------|-------------|
| Results | Lerped result |
| Alpha | Alpha value used |
| Lerp Alpha No Contrast | Alpha without contrast |

---

## Custom Rotation

### CustomRotator
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Texturing/CustomRotator.CustomRotator`
**Function**: Custom UV rotation with center point control

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| UVs | Vector2 | UV coordinates |
| Rotation Center | Vector2 | Center point for rotation |
| Rotation Angle (0-1) | Scalar | Rotation angle (0=0°, 1=360°) |

**Outputs**:
| Name | Description |
|------|-------------|
| Rotated UVs | Rotated UV coordinates |

---

## Texture Variation

### TextureVariation
**Path**: `/Engine/Functions/Engine_MaterialFunctions03/Texturing/TextureVariation.TextureVariation`
**Function**: Breaks up tiling of repetitive textures using displacement

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Heightmap | Unknown | Displacement map texture object |
| UVs | Vector2 | Base input UVs |
| Variation Scale | Scalar | Size of variation pattern |
| Variation Levels | Scalar | Number of random variation bands |
| Heightmap Influence | Scalar | How much displacement affects blend |
| Random Rotation and Scale | Unknown | Randomly rotate texture samples |
| Use Dither | Unknown | Enable TAA dithering pattern |
| HQ Edge Comparison | Unknown | Read second displacement sample |
| Mask Channel | Vector4 | Which channel to use |

**Outputs**:
| Name | Description |
|------|-------------|
| Shifted UVs | Randomly shifted UVs |
| Raw UVs | Unmodified UVs for mip calculation |
| DDX | Derivative X |
| Random Offset | Random UV offset values |
| DDY | Derivative Y |

---

[查看完整路径索引](./index.md)
