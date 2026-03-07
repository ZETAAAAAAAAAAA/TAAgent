# UE Material Functions - Procedural

> 程序化生成相关 Material Function 详细文档

---

## Normal Generation

### NormalFromHeightmap
**Path**: `/Engine/Functions/Engine_MaterialFunctions03/Procedurals/NormalFromHeightmap.NormalFromHeightmap`
**Function**: Generates normal map from heightmap

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Height Map | Unknown | Heightmap texture (uses R channel) |
| Coordinates | Vector2 | UV coordinates |
| Height Map UV Offset | Scalar | Spread for height derivation (default: 0.005) |
| Normal Map Intensity | Scalar | Normal strength (default: 8) |
| Height Map Channel Selector | Vector4 | Channel mask (1,0,0,0 for R) |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Generated normal map |

---

### NormalFromFunction
**Path**: `/Engine/Functions/Engine_MaterialFunctions03/Procedurals/NormalFromFunction.NormalFromFunction`
**Function**: Generates normal from function input

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Function | Scalar | Height function input |
| UVs | Vector2 | UV coordinates |
| Strength | Scalar | Normal strength |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Generated normal |

---

### NormalFromHeightmapChaos
**Path**: `/Engine/Functions/Engine_MaterialFunctions03/Procedurals/NormalFromHeightmapChaos.NormalFromHeightmapChaos`
**Function**: Generates chaotic normal from heightmap

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Height Map | Unknown | Heightmap texture |
| UVs | Vector2 | UV coordinates |
| Strength | Scalar | Normal strength |
| Chaos | Scalar | Chaos amount |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Chaotic normal map |

---

### HeightToNormalSmooth
**Path**: `/Engine/Functions/Engine_MaterialFunctions03/Procedurals/HeightToNormalSmooth.HeightToNormalSmooth`
**Function**: Smooth normal generation from height

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Height | Scalar | Height value |
| UVs | Vector2 | UV coordinates |
| Strength | Scalar | Normal strength |
| Smoothness | Scalar | Edge smoothness |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Smooth normal map |

---

## Normal Perturbation

### PerturbNormalHQ
**Path**: `/Engine/Functions/Engine_MaterialFunctions03/Procedurals/PerturbNormalHQ.PerturbNormalHQ`
**Function**: High quality normal perturbation

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Normal | Vector3 | Input normal |
| Perturbation | Vector3 | Perturbation vector |
| Strength | Scalar | Perturbation strength |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Perturbed normal |

---

### PerturbNormalLQ
**Path**: `/Engine/Functions/Engine_MaterialFunctions03/Procedurals/PerturbNormalLQ.PerturbNormalLQ`
**Function**: Low quality normal perturbation (faster)

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Normal | Vector3 | Input normal |
| Perturbation | Vector3 | Perturbation vector |
| Strength | Scalar | Perturbation strength |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Perturbed normal |

---

### PreparePerturbNormalHQ
**Path**: `/Engine/Functions/Engine_MaterialFunctions03/Procedurals/PreparePerturbNormalHQ.PreparePerturbNormalHQ`
**Function**: Prepares data for HQ normal perturbation

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| UVs | Vector2 | UV coordinates |
| Strength | Scalar | Perturbation strength |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Prepared perturbation data |

---

## Derivatives

### ComputeFilterWidth
**Path**: `/Engine/Functions/Engine_MaterialFunctions03/Procedurals/ComputeFilterWidth.ComputeFilterWidth`
**Function**: Computes filter width for anti-aliasing

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Value | Scalar | Input value |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Filter width |

---

### Compute3DDeriv
**Path**: `/Engine/Functions/Engine_MaterialFunctions03/Procedurals/Compute3DDeriv.Compute3DDeriv`
**Function**: Computes 3D derivatives

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Position | Vector3 | World position |
| Value | Scalar | Input value |

**Outputs**:
| Name | Description |
|------|-------------|
| Gradient | Gradient vector |
| Divergence | Divergence value |

---

### Prepare3DDeriv
**Path**: `/Engine/Functions/Engine_MaterialFunctions03/Procedurals/Prepare3DDeriv.Prepare3DDeriv`
**Function**: Prepares 3D derivative computation

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Position | Vector3 | World position |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Prepared derivative data |

---

### GradFrom3DDeriv
**Path**: `/Engine/Functions/Engine_MaterialFunctions03/Procedurals/GradFrom3DDeriv.GradFrom3DDeriv`
**Function**: Extracts gradient from 3D derivative

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| DerivData | Unknown | Derivative data |

**Outputs**:
| Name | Description |
|------|-------------|
| Gradient | Gradient vector |

---

### CurlFrom3DDeriv
**Path**: `/Engine/Functions/Engine_MaterialFunctions03/Procedurals/CurlFrom3DDeriv.CurlFrom3DDeriv`
**Function**: Extracts curl from 3D derivative

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| DerivData | Unknown | Derivative data |

**Outputs**:
| Name | Description |
|------|-------------|
| Curl | Curl vector |

---

## Band Patterns

### GeneratedBand
**Path**: `/Engine/Functions/Engine_MaterialFunctions03/Procedurals/GeneratedBand.GeneratedBand`
**Function**: Generates procedural band pattern

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| UVs | Vector2 | UV coordinates |
| Width | Scalar | Band width |
| Sharpness | Scalar | Edge sharpness |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Band pattern |

---

### GeneratedOffsetBands
**Path**: `/Engine/Functions/Engine_MaterialFunctions03/Procedurals/GeneratedOffsetBands.GeneratedOffsetBands`
**Function**: Generates offset band patterns

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| UVs | Vector2 | UV coordinates |
| Count | Scalar | Number of bands |
| Width | Scalar | Band width |
| Offset | Scalar | Band offset |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Offset band pattern |

---

## Falloff

### ObjectSpaceFalloff
**Path**: `/Engine/Functions/Engine_MaterialFunctions03/Procedurals/ObjectSpaceFalloff.ObjectSpaceFalloff`
**Function**: Creates falloff in object space

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Position | Vector3 | Object position |
| Falloff | Scalar | Falloff amount |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Falloff mask |

---

## Distance Fields

### DistanceField_Sphere
**Path**: `/Engine/Functions/DistanceFields/Shapes/DistanceField_Sphere.DistanceField_Sphere`
**Function**: Calculates signed distance field to a sphere

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Coordinates | Vector3 | World coordinates |
| Radius | Scalar | Sphere radius |
| Center | Vector3 | Sphere center |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Distance (negative=inside, positive=outside) |

---

### DistanceField_Capsule
**Path**: `/Engine/Functions/DistanceFields/Shapes/DistanceField_Capsule.DistanceField_Capsule`
**Function**: Calculates signed distance field to a capsule

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Coordinates | Vector3 | World coordinates |
| Radius | Scalar | Capsule radius |
| Height | Scalar | Capsule height |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Distance value |

---

### DistanceField_Cylinder
**Path**: `/Engine/Functions/DistanceFields/Shapes/DistanceField_Cylinder.DistanceField_Cylinder`
**Function**: Calculates signed distance field to a cylinder

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| Coordinates | Vector3 | World coordinates |
| Radius | Scalar | Cylinder radius |
| Height | Scalar | Cylinder height |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Distance value |

---

## Distance Field Operations

### DistanceField_Union
**Path**: `/Engine/Functions/DistanceFields/Combiners/DistanceField_Union.DistanceField_Union`
**Function**: Combines two distance fields with union

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| DistanceA | Scalar | First distance field |
| DistanceB | Scalar | Second distance field |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Union of distance fields |

---

### DistanceField_Intersection
**Path**: `/Engine/Functions/DistanceFields/Combiners/DistanceField_Intersection.DistanceField_Intersection`
**Function**: Combines two distance fields with intersection

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| DistanceA | Scalar | First distance field |
| DistanceB | Scalar | Second distance field |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Intersection of distance fields |

---

### DistanceField_Subtract
**Path**: `/Engine/Functions/DistanceFields/Combiners/DistanceField_Subtract.DistanceField_Subtract`
**Function**: Subtracts second distance field from first

**Inputs**:
| Name | Type | Description |
|------|------|-------------|
| DistanceA | Scalar | First distance field |
| DistanceB | Scalar | Distance field to subtract |

**Outputs**:
| Name | Description |
|------|-------------|
| Result | Subtracted distance field |

---

[查看完整路径索引](./index.md)
