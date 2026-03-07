# UE Material Function Complete Knowledge Base

**Total: 645 functions** | Sources: `/Engine/Functions/`, `/Engine/ArtTools/`, `/Engine/EditorMaterials/`, `/Engine/MaterialTemplates/`, `/InterchangeAssets/`

> This knowledge base contains detailed Material Function information: inputs, outputs, and descriptions for direct node usage.

---

## InterchangeAssets - MaterialX Core (v3)

### MX_Surface
**Path**: `/InterchangeAssets/Functions/MX_Surface.MX_Surface`
**Function**: Constructs a surface shader describing light scattering and emission for closed "thick" objects

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| bsdf | Unknown | Distribution function for surface scattering |
| edf | Unknown | Distribution function for surface emission |
| opacity | Scalar | Surface cutout opacity |

**Outputs**:
| Name | Description |
|------|-------------|
| Surface | MaterialAttributes output |

---

### MX_StandardSurface
**Path**: `/InterchangeAssets/Functions/MX_StandardSurface.MX_StandardSurface`
**Function**: Arnold Standard Surface compatible shader with full PBR features

**Inputs** (main):
| Name | Type | Description |
|------|------|-------------|
| base_color | Vector3 | Diffuse reflection color |
| metalness | Scalar | Metallic value |
| specular | Scalar | Specular intensity multiplier |
| specular_roughness | Scalar | Specular reflection roughness |
| specular_IOR | Scalar | Index of refraction for specular reflection |
| specular_anisotropy | Scalar | Directional bias of reflected light |
| coat | Scalar | Clear-coat layer weight |
| coat_roughness | Scalar | Clear-coat reflections roughness |
| sheen | Scalar | Sheen layer weight |
| sheen_color | Vector3 | Sheen layer color |
| subsurface | Scalar | Subsurface scattering weight |
| subsurface_color | Vector3 | Subsurface scattering color |
| emission | Scalar | Emitted incandescent light amount |
| emission_color | Vector3 | Emitted light color |
| thin_film_thickness | Scalar | Thin film layer thickness (nanometers) |
| opacity | Vector3 | Material opacity |

**Outputs** (16):
| Name | Description |
|------|-------------|
| Base Color | Base color |
| Metallic | Metallic value |
| Specular | Specular |
| Roughness | Roughness |
| Anisotropy | Anisotropy |
| EmissiveColor | Emissive color |
| Normal | Normal |
| Opacity | Opacity |
| ClearCoat | Clear coat |
| ClearCoatRoughness | Clear coat roughness |
| ClearCoatNormal | Clear coat normal |
| SheenColor | Sheen color |
| SheenRoughness | Sheen roughness |
| SubsurfaceColor | Subsurface color |
| Tangent | Tangent |
| SurfaceCoverage | Surface coverage |

---

### MX_OpenPBR_Opaque
**Path**: `/InterchangeAssets/Functions/MX_OpenPBR_Opaque.MX_OpenPBR_Opaque`
**Function**: OpenPBR opaque surface shader (requires Substrate for full support)

**Inputs** (main):
| Name | Type | Description |
|------|------|-------------|
| base_color | Vector3 | Diffuse reflection color |
| base_metalness | Scalar | Metallic value |
| base_weight | Scalar | Diffuse reflection intensity multiplier |
| base_diffuse_roughness | Scalar | Diffuse reflection roughness |
| specular_weight | Scalar | Specular reflection weight |
| specular_ior | Scalar | Index of refraction for specular reflection |
| specular_roughness | Scalar | Specular reflection roughness |
| specular_roughness_anisotropy | Scalar | Specular anisotropy |
| coat_weight | Scalar | Clear-coat layer weight |
| coat_roughness | Scalar | Clear-coat roughness |
| fuzz_weight | Scalar | Sheen layer weight |
| fuzz_color | Vector3 | Sheen layer color |
| subsurface_weight | Scalar | Subsurface scattering weight |
| thin_film_thickness | Scalar | Thin film thickness |

**Outputs**: Same as MX_StandardSurface (15 outputs)

---

### MX_BurleyDiffuseBSDF
**Path**: `/InterchangeAssets/Functions/MX_BurleyDiffuseBSDF.MX_BurleyDiffuseBSDF`
**Function**: Constructs a diffuse reflection BSDF based on Disney Principled model

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| weight | Scalar | Weight for this BSDF's contribution |
| color | Vector3 | Diffuse reflectivity (albedo) |
| roughness | Scalar | Surface roughness |
| normal | Vector3 | Normal vector of the surface |

**Outputs**:
| Name | Description |
|------|-------------|
| Output | BSDF output |

---

### MX_Artistic_IOR
**Path**: `/InterchangeAssets/Functions/MX_Artistic_IOR.MX_Artistic_IOR`
**Function**: Converts artistic parameterization reflectivity and edge_color to complex IOR values for conductor BSDF

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| reflectivity | Vector3 | Reflectivity per color component at facing angles |
| edge_color | Vector3 | Reflectivity per color component at grazing angles |

**Outputs**:
| Name | Description |
|------|-------------|
| ior | Computed index of refraction |
| extinction | Computed extinction coefficient |

---

### SpecGlossToMetalRoughness
**Path**: `/InterchangeAssets/gltf/SpecGlossToMetalRoughness.SpecGlossToMetalRoughness`
**Function**: Converts from Specular-Glossiness PBR workflow to Metallic-Roughness according to GLTF reference implementation

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| DiffuseColor | Vector3 | Linear-encoded diffuse color (metals have [0,0,0]) |
| SpecularColor | Vector3 | Linear-encoded specular color |

**Outputs**:
| Name | Description |
|------|-------------|
| Metallic (S) | Converted metallic value |
| BaseColor (V3) | Linear converted base color |

---

### MF_Clearcoat (glTF)
**Path**: `/InterchangeAssets/gltf/MaterialFunctions/MF_Clearcoat.MF_Clearcoat`
**Function**: glTF clearcoat layer material function

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| ClearCoatTexture | Unknown | Clearcoat intensity texture |
| ClearCoatRoughnessTexture | Unknown | Clearcoat roughness texture |
| ClearCoatNormalTexture | Unknown | Clearcoat normal texture (Top Normal) |
| ClearCoatFactor | Scalar | Clearcoat intensity factor |
| ClearCoatRoughnessFactor | Scalar | Clearcoat roughness factor |
| ClearCoatNormalScale | Scalar | Normal scale |
| ClearCoatTexture_UVs | Vector2 | UV coordinates |
| ... | | (DDX/DDY derivative inputs) |

**Outputs**:
| Name | Description |
|------|-------------|
| ClearCoatRoughness | Clearcoat roughness |
| ClearCoatAmount | Clearcoat intensity |
| ClearCoatTopNormal | Clearcoat top normal |

---

## New Categories (v2)

### DistanceField_Sphere
**Path**: `/Engine/Functions/DistanceFields/Shapes/DistanceField_Sphere.DistanceField_Sphere`
**Function**: Calculates signed distance field to a sphere (SDF)

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Coordinates | Vector3 | World coordinates |
| Radius | Scalar | Sphere radius |
| Center | Vector3 | Sphere center position |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Distance value (negative=inside, positive=outside) |

---

### ImposterUVs
**Path**: `/Engine/ArtTools/RenderToTexture/MaterialFunctions/ImposterUVs.ImposterUVs`
**Function**: Imposter UV generation for Billboard optimized rendering

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Position | Vector3 | Position |
| FramesX | Scalar | Frame count on X axis |
| FramesY | Scalar | Frame count on Y axis |
| SpriteWidth | Scalar | Sprite width |
| SpriteHeight | Scalar | Sprite height |
| Normals | Vector3 | Normals |
| Rotation | Scalar | Rotation angle |
| Fixed Z | Unknown | Fixed Z axis |
| Rotation For Normals | Scalar | Normal rotation |

**Outputs**:
| Name | Description |
|------|-------------|
| BlendPhase xy | Blend phase |
| UVs | Generated UVs |
| WorldPositionOffset | WPO offset |
| TransformedNormals | Transformed normals |

---

### MF_UI_CursorPosition
**Path**: `/Engine/Functions/UserInterface/MF_UI_CursorPosition.MF_UI_CursorPosition`
**Function**: Gets UI cursor position

**Inputs**: None

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Cursor position (Float2) |

---

### MF_Chromakeyer
**Path**: `/Engine/MaterialTemplates/Chromakeying/MF_Chromakeyer.MF_Chromakeyer`
**Function**: Parameterized green screen keyer template with 2D garbage matte support

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| InputUV | Vector2 | UV coordinates |
| Input Color | Vector3 | Input image color |

**Outputs**:
| Name | Description |
|------|-------------|
| Emissive Color | Keyed color output |
| Opacity | Alpha mask |

**Built-in Parameters**:
- Key Color: Keying color
- LumaMask: Luminance mask threshold
- Alpha Min/Divisor: Alpha control
- Despill Amount/Color: Spill suppression
- Crop Left/Right/Top/Bottom: Cropping

---

### PivotPainter2FoliageShader
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/PivotPainter2/PivotPainter2FoliageShader.PivotPainter2FoliageShader`
**Function**: Pivot Painter 2 foliage shader with 4-layer wind animation support

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Material Attributes | Unknown | Material attributes (requires tangent space normals) |
| Pivot Painter Texture Coordinate | Vector2 | PP texture coordinate |

**Outputs**:
| Name | Description |
|------|-------------|
| World Position Offset Component | WPO offset |
| Modified World Space Normal Component | World space normal |
| Final Material with World Space Normals | Final material |

**Wind Parameters** (per layer):
- Wind Speed, Local Wind Scale
- Max Rotation, Dampening Radius
- Random Rotation Influence
- Shelter Dot Product Bias/Modulation

---

## Core Functions (Detailed Documentation)

### BitMask
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Texturing/BitMask.BitMask`
**Function**: Provides a 1bit mask from a supplied Grayscale image and a value between 0-255

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| BitMask | Scalar | Should be texture using TC_Grayscale |
| Bit | Scalar | Value between 0-255 |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | 0 or 1 mask output |

---

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

### ParallaxOcclusionMapping
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Texturing/ParallaxOcclusionMapping.ParallaxOcclusionMapping`
**Function**: Parallax Occlusion Mapping uses Ray Tracing to find the correct offsets at each point

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

### UnpackNormalFromFloat
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Normals/UnpackNormalFromFloat.UnpackNormalFromFloat`
**Function**: Unpack Normal/Float3 values from a scalar value

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Scalar Value | Scalar | Packed normal value |

**Outputs**:
| Name | Description |
|------|-------------|
| XXY.YZZ Vector | Unpacked vector |
| XXY.YZZ Normalized Vector | Normalized vector |
| ( z sign ) XXX.YYY Normal | Normal with Z sign |
| ( z sign ) XXYY.0 | XY components |

---

### CameraDepthFade
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Opacity/CameraDepthFade.CameraDepthFade`
**Function**: Creates a gradient of 0 near the camera to white at Fade Length. Useful for preventing particles from camera clipping

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

### LinearGradient
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Gradient/LinearGradient.LinearGradient`
**Function**: Uses UV Channel 0 to generate a linear gradient along the U or V axis

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

### MakeFloat2
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Utility/MakeFloat2.MakeFloat2`
**Function**: Creates a float 2 vector from a series of scalar inputs

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
**Function**: Creates a float 3 vector from a series of scalar inputs

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

### BreakOutFloat3Components
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Utility/BreakOutFloat3Components.BreakOutFloat3Components`
**Function**: Enter a float 3 vector into the function and retrieve individual channels back through the scalar outputs

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Float3 | Vector3 | Input vector |

**Outputs**:
| Name | Description |
|------|-------------|
| R | Red channel / X component |
| G | Green channel / Y component |
| B | Blue channel / Z component |

---

### SafeNormalize
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/SafeNormalize.SafeNormalize`
**Function**: If the input vectors length == 0 then return 0 else return the normalized vector by default. This avoids returning NAN or NULL

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Vector | Vector3 | Input vector |
| Default | Vector3 | Default value when length is 0 |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Normalized vector |
| Length==0 | Whether length is 0 (1=yes) |

---

### SimpleGrassWind
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/WorldPositionOffset/SimpleGrassWind.SimpleGrassWind`
**Function**: Simple Waving grass wind, Grass must be textured to full sheet. This should be the last WPO node applied

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| AdditionalWPO | Vector3 | Additional WPO |
| WindIntensity | Scalar | Wind intensity |
| WindWeight | Scalar | Wind weight (vertex color or texture) |
| WindSpeed | Scalar | Wind speed |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | WPO offset value |

---

### DitherTemporalAA
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Utility/DitherTemporalAA.DitherTemporalAA`
**Function**: Stipple pattern/Dither in screen space and time to work well with TemporalAA for things like masked translucency or anisotropic materials

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Alpha Threshold | Scalar | Alpha threshold for dither cutoff |
| Random | Scalar | 0=no random, 1=random (default) |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Dithered mask value for Opacity Mask |

**Use Case**: Combine with Masked material blend mode to simulate translucency through temporal anti-aliasing. Creates stipple pattern that blends over time with TAA enabled.

**Internal Nodes**: Uses screen position, view property, custom Bayer matrix dithering (Mod pattern), and temporal randomization.

---

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

### Blend_Screen
**Path**: `/Engine/Functions/Engine_MaterialFunctions03/Blends/Blend_Screen.Blend_Screen`
**Function**: Inverts each texture, multiplies them together, then re-inverts the results.

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

### BlendAngleCorrectedNormals
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Utility/BlendAngleCorrectedNormals.BlendAngleCorrectedNormals`
**Function**: Corrects the normal direction of normal map that is overlayed on other normal maps (typically detail map).

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

### ConstantScalebyDistance
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/WorldPositionOffset/ConstantScalebyDistance.ConstantScalebyDistance`
**Function**: Keeps a quad a constant scale on the screen, falls apart at close distances.

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| SizeScale | Scalar | Scale multiplier |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | WPO offset for constant screen scale |

---

### ObjectScale
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/WorldPositionOffset/ObjectScale.ObjectScale`
**Function**: Returns the object's XYZ scale together and separately. Not compatible with pixel shader.

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
**Function**: Returns the object's pivot point in world space. Not compatible with pixel shader.

**Inputs**: None

**Outputs**:
| Name | Description |
|------|-------------|
| Object Pivot Location | Object pivot in world space |
| Mesh Particle Pivot Location | Mesh particle pivot |

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

## Gradient & Masking Functions

### BoxMask-2D
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Gradient/BoxMask-2D.BoxMask-2D`
**Function**: Draws a box in 2D Space. Controls for size and falloff.

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| A | Vector2 | Position coordinates to measure (default: World Position) |
| Bounds | Vector2 | Bounds of the box in X Y |
| B | Vector2 | Center point location of the box |
| Edge Falloff | Scalar | Falloff of the edge gradient in units |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Box mask result |

---

### SphereGradient-3D
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Gradient/SphereGradient-3D.SphereGradient-3D`
**Function**: Generates a gradient representing the depth of a sphere centered on the desired point.

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Radius | Scalar | Sphere radius |
| Location | Vector3 | Sphere center location |
| Offset | Vector3 | Offset from location |
| Calculate Camera Inside | Unknown | Extra calculations for camera inside sphere |
| Depth Biased Alpha | Unknown | Enable depth-biased alpha |
| FadeDistance | Scalar | Fade distance for depth bias |

**Outputs**:
| Name | Description |
|------|-------------|
| Result 0-1 | Normalized gradient result |
| Result Diameter | Diameter result |
| Result Radius | Radius result |

---

## Opacity & Transparency Functions

### SoftOpacity
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Opacity/SoftOpacity.SoftOpacity`
**Function**: Softens the Opacity input by multiplying against Fresnel, DepthBiasedAlpha and Pixel Depth.

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| OpacityIn | Scalar | The Opacity Value to be Softened |
| FadeDistance | Scalar | How close before fading out (DEFAULT=512) |
| DepthFadeDistance | Scalar | Depth fade distance |

**Outputs**:
| Name | Description |
|------|-------------|
| OutputUsesDepthBias | Result with depth bias (12 more instructions) |
| OutputNoDepthBias | Result without depth bias (12 fewer instructions) |

---

## Shading & Lighting Functions

### FuzzyShading
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Shading/FuzzyShading.FuzzyShading`
**Function**: Emulates shading similar to velvet or moss.

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

### AxisAlignedFresnel
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Shading/AxisAlignedFresnel.AxisAlignedFresnel`
**Function**: Fresnel falloff perpendicular to specified axis. Useful for fading cylinder edges.

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

## Texturing Functions

### FlattenNormal
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Texturing/FlattenNormal.FlattenNormal`
**Function**: Lerps a NormalMap with (0,0,1) to flatten it.

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

### HeightLerp
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Texturing/HeightLerp.HeightLerp`
**Function**: Lerp between two values based on height map and transition value.

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
| Alpha | Alpha value used in lerp |
| Lerp Alpha No Contrast | Alpha without contrast |

---

### FlipBook
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Texturing/FlipBook.FlipBook`
**Function**: Animates a texture in a flipbook fashion.

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
| UV Center | Center UV for rotation operations |

---

## Volumetric Functions

### BeersLaw
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Volumetrics/BeersLaw.BeersLaw`
**Function**: Returns e^(-d) which is a standard exponential density function.

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Depth Scale | Scalar | Multiplier for thickness |
| Thickness | Scalar | Distance ray travelled through medium |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Exponential density result |

---

## Image Adjustment Functions

### CheapContrast
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/ImageAdjustment/CheapContrast.CheapContrast`
**Function**: Cheaply adds contrast similar to pulling edges in Photoshop Levels.

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

## Reflection Functions

### ReflectionVectorSoft
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Reflections/ReflectionVectorSoft.ReflectionVectorSoft`
**Function**: Softens the normal for a smoother reflection.

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Normal | Vector3 | Normal to be softened |
| Softness | Scalar | Softness value (0-1) |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Softened reflection vector |

---

## Procedural Functions

### NormalFromHeightmap
**Path**: `/Engine/Functions/Engine_MaterialFunctions03/Procedurals/NormalFromHeightmap.NormalFromHeightmap`
**Function**: Takes a heightmap input and generates a normal map from it.

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Height Map | Unknown | Heightmap texture (uses R channel by default) |
| Coordinates | Vector2 | UV coordinates |
| Height Map UV Offset | Scalar | Spread for height derivation (default: 0.005) |
| Normal Map Intensity | Scalar | Normal strength (default: 8) |
| Height Map Channel Selector | Vector4 | Channel mask (1,0,0,0 for R) |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Generated normal map |

---

## Material Layer Blend Functions

### MatLayerBlend_Standard
**Path**: `/Engine/Functions/MaterialLayerFunctions/MatLayerBlend_Standard.MatLayerBlend_Standard`
**Function**: Blends all attributes of 2 Materials.

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

### Blend_Overlay
**Path**: `/Engine/Functions/Engine_MaterialFunctions03/Blends/Blend_Overlay.Blend_Overlay`
**Function**: Emulates Photoshop's Overlay blend mode. Lighter pixels become lighter, darker become darker.

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

## Lighting & Specular Functions

### BlinnPhongSpecular
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Lighting/BlinnPhongSpecular.BlinnPhongSpecular`
**Function**: Returns a specular lobe using Blinn-Phong.

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
**Function**: Returns a specular lobe using GGX distribution.

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

### Sphere_AO
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Shading/Sphere_AO.Sphere_AO`
**Function**: Calculates Ambient Occlusion for a sphere affecting the world at any point.

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

## Volumetric Functions

### Henyey-Greenstein-Phase
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Volumetrics/Henyey-Greenstein-Phase.Henyey-Greenstein-Phase`
**Function**: Phase function for volumetric light scattering.

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| AngleDegrees | Scalar | Viewing angle (optional, overrides Light Vector) |
| Anisotropy | Scalar | 0=isotropic, 1=forward scattering |
| Light Vector | Vector3 | Light direction |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Phase function result |

---

## Image Adjustment Functions

### ColorCorrection
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/ImageAdjustment/ColorCorrection.ColorCorrection`
**Function**: Full color correction with lift/gamma/gain controls.

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
**Function**: Blends between 3 colors based on a greyscale input.

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

## Coordinate Functions

### WorldPosition-XY
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Coordinates/WorldPosition-XY.WorldPosition-XY`
**Function**: Returns world position XY coordinates with scale.

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Scale | Scalar | Scale multiplier |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | XY world position |

---

### WorldAlignedBlend
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/AlphaBlend/WorldAlignedBlend.WorldAlignedBlend`
**Function**: World-aligned alpha blending based on surface direction.

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| In World Vector | Vector3 | Direction vector (default: 0,0,1) |
| Blend Bias | Scalar | Shift alpha up/down |
| Blend Sharpness | Scalar | Blend edge sharpness |
| In Explicit Normal | Vector3 | Custom normal for blending |
| Clamped? | Unknown | Enable for downstream modulation |
| Alpha | Scalar | Additional alpha multiplier |

**Outputs**:
| Name | Description |
|------|-------------|
| Alpha | Default output using PixelNormalWS |
| w/ Explicit Normal | Using specified normal |
| w/Vertex Normals | Using mesh vertex normals |

---

## Debug Functions

### DebugScalarValues
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Utility/DebugScalarValues.DebugScalarValues`
**Function**: Display scalar value as numbers in UV space.

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Number | Scalar | Value to display |
| MaximumNumberOfDigits | Scalar | Max digits to show |
| UVs | Vector2 | UV coordinates |
| DebugTextLocation RG_UpperRight BA_LowerLeft | Vector4 | Text position |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Debug visualization |

---

### DebugFloat3Values
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Utility/DebugFloat3Values.DebugFloat3Values`
**Function**: Display vector values as numbers.

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

## Wind Functions

### Wind
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/WorldPositionOffset/Wind.Wind`
**Function**: Provides wind speed, strength, and direction.

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

## Math Functions

### WrapFloat
**Path**: `/Engine/Functions/Engine_MaterialFunctions03/Math/WrapFloat.WrapFloat`
**Function**: Similar to fmod with support for negative floats.

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

### WithinRange
**Path**: `/Engine/Functions/Engine_MaterialFunctions03/Math/WithinRange.WithinRange`
**Function**: Returns 1 if value is within range, 0 otherwise.

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

### Swizzle
**Path**: `/Engine/Functions/Engine_MaterialFunctions02/Utility/Swizzle.Swizzle`
**Function**: Swizzles vector components.

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
**Function**: Transforms vector to radial coordinates.

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

## Detail Texturing

### DetailTexturing
**Path**: `/Engine/Functions/Engine_MaterialFunctions01/Texturing/DetailTexturing.DetailTexturing`
**Function**: Applies detail textures to diffuse and normal.

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

## Full Index

See [index.md](./index.md) - Contains path index for all 645 MFs

---

## Usage Recommendations

### Common Combinations

| Use Case | Recommended MF Combination |
|----------|---------------------------|
| World space textures | WorldAlignedTexture + WorldAlignedNormal |
| Grass/foliage | SimpleGrassWind + CameraDepthFade |
| Precision depth | ParallaxOcclusionMapping |
| Mask generation | BitMask + LinearGradient + RadialGradient |
| Vector operations | MakeFloat* + BreakOut* + SafeNormalize |

### Performance Tiers

| Tier | MFs |
|------|-----|
| **Lightweight** | BitMask, MakeFloat*, BreakOut*, LinearGradient, RadialGradient |
| **Medium** | WorldAlignedTexture, SafeNormalize, CameraDepthFade |
| **Heavy** | ParallaxOcclusionMapping, Texture_Bombing, RayMarchHeightMap |
