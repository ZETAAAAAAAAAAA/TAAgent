# Shadertoy to Unreal Engine Custom HLSL Migration Guide

This document records common compilation errors and solutions when porting Shadertoy shaders to Unreal Engine's Custom HLSL material nodes.

## 1. Function Definition Inside Custom Node

### Error
```
error: cannot define function inside a function
```

### Cause
UE's Custom HLSL node code is wrapped inside an auto-generated function:
```hlsl
MaterialFloat4 CustomExpression0(FMaterialPixelParameters Parameters) {
    // Your code gets pasted here
}
```
Defining functions directly inside the node results in "function inside function" syntax error.

### Solution
Wrap helper functions in a `struct`:

```hlsl
// WRONG - Will fail to compile
float square(float2 st) {
    return step(0, st.x) * step(0, st.y);
}

// CORRECT - Use struct wrapper
struct MyFunctions {
    float square(float2 st) {
        return step(0, st.x) * step(0, st.y);
    }
};

// Usage
MyFunctions func;
float result = func.square(uv);
```

## 2. Matrix Multiplication Syntax Differences

### Error
```
error: cannot convert from 'float2x2' to 'float2'
```

### Cause
GLSL and HLSL have different matrix multiplication syntax:

| GLSL | HLSL |
|------|------|
| `vec * mat` (vector on left) | `mul(mat, vec)` (matrix on left) |
| `mat2(a,b,c,d)` column-major | `float2x2(a,b,c,d)` row-major |

### Solution
Expand matrix multiplication to explicit calculations:

```hlsl
// GLSL style
float2 result = st * mat2(scaleX, 0, 0, scaleY);

// HLSL correct - expand directly
float2 result = float2(st.x * scaleX, st.y * scaleY);

// Or use mul (note the order)
float2x2 mat = float2x2(scaleX, 0, 0, scaleY);
float2 result = mul(mat, st);  // Matrix on left
```

## 3. Type Name Differences

### Type Conversion Table

| GLSL | HLSL |
|------|------|
| `vec2` | `float2` |
| `vec3` | `float3` |
| `vec4` | `float4` |
| `mat2` | `float2x2` |
| `mat3` | `float3x3` |
| `mat4` | `float4x4` |
| `ivec2` | `int2` |
| `bvec2` | `bool2` |

## 4. Function Name Differences

### Function Conversion Table

| GLSL | HLSL |
|------|------|
| `mix(a, b, t)` | `lerp(a, b, t)` |
| `fract(x)` | `frac(x)` |
| `texture(sampler, uv)` | `Texture2DSample(Tex, Sampler, uv)` |
| `dFdx(x)` | `ddx(x)` |
| `dFdy(x)` | `ddy(x)` |
| `mod(a, b)` | `fmod(a, b)` |

## 5. Array Definition and Initialization

### Error
```
error: arrays of user-defined type not allowed
```

### Cause
HLSL has stricter limitations on array initialization.

### Solution
```hlsl
// GLSL style
vec2 points[8];

// HLSL style - use static array
static float2 points[8];

// Or define inside struct
struct MyShader {
    static float2 points[8];
};
```

## 6. Constant Definitions

### Notes
```hlsl
// GLSL
#define PI 3.14159265359
const float value = 1.0;

// HLSL - Both work fine
#define PI 3.14159265359
static const float value = 1.0;
```

## 7. Vector Construction

### Notes
```hlsl
// GLSL
vec3 color = vec3(r, g, b);
vec2 uv = vec2(x, y);

// HLSL
float3 color = float3(r, g, b);
float2 uv = float2(x, y);

// Type expansion
float3 rgb = float3(1.0); // Auto-expands to (1, 1, 1)
```

## 8. Ternary Operator Type Matching

### Error
```
error: type mismatch in conditional expression
```

### Solution
Ensure both branches return the same type:
```hlsl
// Potentially wrong
float result = condition ? someValue : 0; // 0 is int

// Correct
float result = condition ? someValue : 0.0; // Both are float
```

## 9. Array Access in Loops

### Notes
HLSL loop indices should be statically determinable:

```hlsl
// Fixed iterations - OK
for(int i = 0; i < 8; i++) {
    points[i] = ...;
}

// Avoid dynamic loop bounds
int count = variable; // Not recommended
for(int i = 0; i < count; i++) { ... }
```

## 10. Shadertoy Specific Variables

### Variable Conversion Table

| Shadertoy | UE Custom HLSL |
|-----------|----------------|
| `iTime` | Pass as parameter `Time` |
| `iResolution` | Pass as parameter or use screen size |
| `fragCoord` | Use `UV * Resolution` |
| `iMouse` | Pass as parameter |

## 11. Parameter Passing Notes

### Python Calling Convention
```python
# ScalarParameter - Use array format
value=[0.076]  # Correct

# VectorParameter - Use 4-element array
value=[0, 0.5, 0.5, 1]  # Correct
```

## 12. Updating Existing Nodes

Use `node_id` parameter to update existing nodes instead of creating new ones:

```python
# Update CustomHLSL node
add_material_expression(
    material_name="M_Material",
    expression_type="Custom",
    node_id="Expr_Custom_12345",  # Existing node ID
    code="return 1.0;",
    inputs=["UV", "Time"]
)

# Update ScalarParameter
add_material_expression(
    material_name="M_Material",
    expression_type="ScalarParameter",
    node_id="Expr_ScalarParameter_12345",
    parameter_name="Speed",
    value=[0.5]
)
```

## 13. pow() Function with Vector Exponents

### Error
GLSL allows `pow(vec, vec)` but HLSL requires matching types.

### Solution
```hlsl
// GLSL
vec3 result = pow(value, vec3(0.4545));

// HLSL - Explicit components
float3 result = float3(
    pow(value.r, 0.4545),
    pow(value.g, 0.4545),
    pow(value.b, 0.4545)
);

// Or use uniform exponent
float3 result = pow(value, float3(0.4545, 0.4545, 0.4545));
```

## 14. Static Data Member in Local Class

### Error
```
error: static data member 'points' not allowed in local class 'NeonLoveShader'
    static float2 points[8];
                  ^
```

### Cause
In UE's Custom HLSL, the struct is defined inside the auto-generated function, making it a "local class". C++/HLSL does not allow static data members in local classes (classes defined inside functions).

### Solution
Move the array outside the struct as a local variable:

```hlsl
// WRONG - Static member not allowed in local class
struct MyShader {
    static float2 points[8];  // ERROR!
};

// CORRECT - Define array as local variable outside struct
struct MyShader {
    float2 hash2(float2 p) {
        return frac(sin(p) * 43758.5453);
    }
};

MyShader shader;

// Array defined outside struct, inside function body
float2 points[8];
points[0] = float2(0.0, 0.0);
points[1] = float2(0.0, 0.1);
// ... etc

for (int i = 0; i < 8; i++) {
    float2 point = points[i];
    // Use shader.hash2() for helper functions
}
```

### Alternative: Use Constant Array Initialization
```hlsl
// HLSL allows this syntax for constant arrays
static const float2 points[8] = {
    float2(0.0, 0.0),
    float2(0.0, 0.1),
    float2(0.0, 0.2),
    float2(0.0, 0.3),
    float2(0.0, 0.4),
    float2(0.0, 0.5),
    float2(0.0, 0.6),
    float2(0.0, 0.7)
};
```

## 15. Reserved Variable Names

### Error
```
error: modifiers must appear before type
    float2 point = points[i];
           ^
error: expected unqualified-id
    point += offset * sin(...);
```

### Cause
`point` is a reserved identifier in HLSL (possibly defined in shader includes). Using it as a variable name causes compilation errors.

### Solution
Avoid using `point` as a variable name. Use alternatives like `pos`, `pt`, `coord`:
```hlsl
// WRONG - 'point' is reserved
float2 point = points[i];

// CORRECT - Use different variable name
float2 pos = pts[i];
float2 pt = points[i];
```

**Common reserved names to avoid:**
- `point` - Reserved for geometry shader input
- `normal` - Often used in lighting calculations
- `color` - May conflict with output semantics
- `depth` - Used in depth buffer operations

---

## Quick Migration Checklist

When porting Shadertoy code to UE Custom HLSL:

1. [ ] Change `vec2/vec3/vec4` to `float2/float3/float4`
2. [ ] Change `mat2/mat3/mat4` to `float2x2/float3x3/float4x4`
3. [ ] Change `mix` to `lerp`
4. [ ] Change `fract` to `frac`
5. [ ] Wrap all functions in `struct`
6. [ ] Expand matrix multiplication or use `mul()`
7. [ ] Change `iTime/iResolution` to parameters
8. [ ] Ensure constant type matching (e.g., `0` to `0.0`)
9. [ ] Check array definitions are correct
10. [ ] Use `static` for arrays that persist across calls

---

## Example: Complete Struct Wrapper

```hlsl
#define PI 3.14159265359

struct ShaderFunctions {
    float Time;
    float Speed;
    
    float myFunction(float x) {
        return sin(x * Speed + Time);
    }
    
    float2 transform(float2 uv) {
        return uv * 2.0 - 1.0;
    }
};

// Main code
ShaderFunctions func;
func.Time = Time;
func.Speed = Speed;

float2 transformed = func.transform(UV);
float result = func.myFunction(transformed.x);

return float4(result, result, result, 1.0);
```

---

*Last Updated: 2026-02-19*
