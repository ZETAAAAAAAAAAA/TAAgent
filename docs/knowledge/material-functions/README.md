# UE Material Function Library

**Total: 645 functions** | Sources: `/Engine/Functions/`, `/Engine/ArtTools/`, `/Engine/EditorMaterials/`, `/Engine/MaterialTemplates/`, `/InterchangeAssets/`

---

## 📚 Documentation Index

| Category | Document | Functions | Description |
|----------|----------|-----------|-------------|
| **📝 Complete Index** | [index.md](./index.md) | 645 | Complete path index for all MFs |
| **🎨 Texturing** | [MF_Texturing.md](./MF_Texturing.md) | 75+ | World-aligned textures, UV operations, flow maps, flipbook |
| **🌅 Gradient & Masking** | [MF_Gradient_Masking.md](./MF_Gradient_Masking.md) | 17+ | Linear, radial, box masks, sphere gradients |
| **🔢 Math** | [MF_Math.md](./MF_Math.md) | 50+ | Smooth functions, range operations, matrix, vector math |
| **🔧 Utility** | [MF_Utility.md](./MF_Utility.md) | 55+ | Vector construction/deconstruction, debug, camera utils |
| **💡 Shading & Lighting** | [MF_Shading_Lighting.md](./MF_Shading_Lighting.md) | 15+ | Fuzzy shading, Fresnel, AO, specular |
| **👁️ Opacity & WPO** | [MF_Opacity_WPO.md](./MF_Opacity_WPO.md) | 30+ | Depth fade, wind, sprites, camera alignment |
| **🖼️ Image Adjustment** | [MF_ImageAdjustment.md](./MF_ImageAdjustment.md) | 15+ | Contrast, color correction, gamma, blur |
| **🔀 Blends** | [MF_Blends.md](./MF_Blends.md) | 16+ | Photoshop-style blend modes |
| **🧬 Procedural** | [MF_Procedural.md](./MF_Procedural.md) | 15+ | Normal from height, SDF shapes, derivatives |
| **📚 Material Layer** | [MF_MaterialLayer.md](./MF_MaterialLayer.md) | 25+ | Layer blending, normal blend, overrides |
| **📦 InterchangeAssets** | [complete_index.md](./complete_index.md) | 100+ | MaterialX, glTF, Substrate functions |

---

## Quick Reference

| Category | Key Functions |
|----------|---------------|
| Texturing | WorldAlignedTexture, ParallaxOcclusionMapping, BitMask, FlowMaps |
| Gradient/Mask | LinearGradient, RadialGradient, BoxMask-2D/3D, SphereGradient |
| Math | SmoothStep, RemapValueRange, AngleBetweenVectors, RGBtoHSV |
| Utility | MakeFloat*, BreakOut*, SafeNormalize, VectorLength |
| Shading | GGXSpecular, FuzzyShading, AxisAlignedFresnel, Sphere_AO |
| Opacity | CameraDepthFade, SoftOpacity, AlphaOffset |
| WPO | SimpleGrassWind, Sprite, Wind, ObjectScale, ObjectPivotPoint |
| Blends | Blend_Screen, Blend_Overlay, Blend_SoftLight, HeightLerp |
| Procedural | NormalFromHeightmap, DistanceField_Sphere, PerturbNormalHQ |
| Material Layer | MatLayerBlend_Standard, MatLayerBlend_NormalBlend |

---

## Usage

```
# Search for function by name or keyword in index.md
# Use path directly in Unreal: Create Material Function node → select MF

# Example paths:
/Engine/Functions/Engine_MaterialFunctions01/Texturing/WorldAlignedTexture.WorldAlignedTexture
/Engine/Functions/Engine_MaterialFunctions02/SmoothStep.SmoothStep
```

---

## Performance Guide

| Cost | Examples |
|------|----------|
| **Low** | BitMask, MakeFloat*, BreakOut*, LinearGradient |
| **Medium** | WorldAlignedTexture, RadialGradient, FlowMaps |
| **High** | ParallaxOcclusionMapping, Texture_Bombing, RayMarchHeightMap |

---

## Common Combinations

| Use Case | Recommended MFs |
|----------|-----------------|
| World space textures | WorldAlignedTexture + WorldAlignedNormal |
| Grass/foliage | SimpleGrassWind + CameraDepthFade |
| Precision depth | ParallaxOcclusionMapping |
| Mask generation | BitMask + LinearGradient + RadialGradient |
| Vector operations | MakeFloat* + BreakOut* + SafeNormalize |
| Normal blending | BlendAngleCorrectedNormals |
| Layer blending | MatLayerBlend_Standard + HeightLerp |
