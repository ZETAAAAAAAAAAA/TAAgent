# DXBC Shader Analysis Guide

本指南帮助AI理解和分析RenderDoc导出的DXBC着色器字节码。

## 1. DXBC 基础概念

### 什么是DXBC?
DXBC (DirectX Bytecode) 是DirectX着色器的编译后字节码格式。它不是人类可读的，需要通过AI翻译成HLSL。

### 分析流程
```
RenderDoc Capture → get_shader_info() → DXBC Disassembly → AI Translation → HLSL
```

## 2. DXBC 指令参考

### 纹理采样指令
| DXBC指令 | 含义 | HLSL等价 |
|----------|------|----------|
| `sample r0, v1.xy, t0, s0` | 采样纹理 | `r0 = Texture2DSample(t0, s0, v1.xy)` |
| `sample_l` | 带LOD采样 | `Texture2DSampleLevel()` |
| `sample_b` | 带偏移采样 | `Texture2DSampleBias()` |
| `sample_c` | 比较采样 | `Texture2DSampleCmp()` |
| `sample_d` | 带导数采样 | `Texture2DSampleGrad()` |

### 数学运算指令
| DXBC指令 | 含义 | HLSL等价 |
|----------|------|----------|
| `add r0, r1, r2` | 加法 | `r0 = r1 + r2` |
| `mul r0, r1, r2` | 乘法 | `r0 = r1 * r2` |
| `mad r0, r1, r2, r3` | 乘加 | `r0 = r1 * r2 + r3` |
| `div r0, r1, r2` | 除法 | `r0 = r1 / r2` |
| `dp2 r0, r1, r2` | 2D点积 | `r0 = dot(r1.xy, r2.xy)` |
| `dp3 r0, r1, r2` | 3D点积 | `r0 = dot(r1.xyz, r2.xyz)` |
| `dp4 r0, r1, r2` | 4D点积 | `r0 = dot(r1, r2)` |

### 控制流指令
| DXBC指令 | 含义 | HLSL等价 |
|----------|------|----------|
| `if_nz r0.x` | 条件跳转 | `if (r0.x != 0)` |
| `else` | 否则 | `else` |
| `endif` | 条件结束 | - |
| `loop` | 循环开始 | `while(true)` |
| `endloop` | 循环结束 | - |
| `break_nz r0.x` | 条件跳出 | `if (r0.x != 0) break` |

### 函数指令
| DXBC指令 | 含义 | HLSL等价 |
|----------|------|----------|
| `frc r0, r1` | 小数部分 | `r0 = frac(r1)` |
| `round_ne r0, r1` | 四舍五入 | `r0 = round(r1)` |
| `floor r0, r1` | 向下取整 | `r0 = floor(r1)` |
| `ceil r0, r1` | 向上取整 | `r0 = ceil(r1)` |
| `sqrt r0, r1` | 平方根 | `r0 = sqrt(r1)` |
| `rsq r0, r1` | 平方根倒数 | `r0 = rsqrt(r1)` |
| `pow r0, r1, r2` | 幂运算 | `r0 = pow(r1, r2)` |
| `log r0, r1` | 对数 | `r0 = log(r1)` |
| `exp r0, r1` | 指数 | `r0 = exp(r1)` |

### 三角函数
| DXBC指令 | 含义 | HLSL等价 |
|----------|------|----------|
| `sin r0, r1` | 正弦 | `r0 = sin(r1)` |
| `cos r0, r1` | 余弦 | `r0 = cos(r1)` |
| `tan r0, r1` | 正切 | `r0 = tan(r1)` |
| `sincos r0, r1, r2` | 正弦余弦 | `sincos(r2, r0, r1)` |

### 向量操作
| DXBC指令 | 含义 | HLSL等价 |
|----------|------|----------|
| `mov r0, r1` | 赋值 | `r0 = r1` |
| `movc r0, r1, r2, r3` | 条件赋值 | `r0 = r1 ? r2 : r3` |
| `swapc` | 条件交换 | - |
| `dot` | 点积 | `dot()` |
| `crs` | 叉积 | `cross()` |

## 3. 纹理槽位识别

### 常见绑定模式
| 槽位范围 | 常见用途 |
|----------|---------|
| t0-t3 | BaseColor, Normal, ORM, Emissive |
| t4-t7 | 细节纹理, 流动图, 噪声 |
| t8-t15 | 光照贴图, 阴影贴图 |
| s0-s7 | 采样器状态 |

### 纹理类型识别
从shader代码中的采样方式推断:
- `Sample(t0, uv)` + `rgba` → BaseColor/Diffuse
- `Sample(t1, uv)` + 2通道 + 归一化 → Normal Map
- `Sample(t2, uv)` + 单通道 → ORM (Occlusion/Roughness/Metallic)
- `Sample(t_flow, uv)` + 2通道流向 → Flow Map

## 4. 材质类型识别

### PBR标准材质
特征: Albedo + Normal + ORM + Emissive
```hlsl
// 典型PBR采样模式
albedo = Sample(BaseTexture, uv);
normal = Sample(NormalTexture, uv) * 2 - 1;
orm = Sample(ORMTexture, uv);
// normal用于法线，orm.r=ao, orm.g=roughness, orm.b=metallic
```

### 水体材质
特征: FlowMap + Normal扰动 + 深度淡入 + 折射
```hlsl
// 水体特征
flowUV = CalculateFlow(FlowMap, time);
normal = BlendNormals(BaseNormal, FlowNoise);
depthFade = DepthFade(sceneDepth, distance);
refraction = CalculateRefraction(normal, depthFade);
```

### 皮肤材质
特征: SSS + Cavity + DetailNormal
```hlsl
// 皮肤特征
sssColor = SubsurfaceScattering(...);
cavity = Sample(CavityTexture, uv).r;
detailNormal = Sample(DetailNormal, uv * detailScale);
```

### 植被材质
特征: AlphaCutout + TwoSided + Wind
```hlsl
// 植被特征
alpha = Sample(AlphaTexture, uv).r;
clip(alpha - cutoff);  // Alpha test
windOffset = CalculateWind(position, time);
```

## 5. AI翻译策略

### 步骤1: 识别输入输出
```
输入结构 (PSInput):
- 语义名 (TEXCOORD, NORMAL, COLOR等)
- 用途推断

输出结构 (PSOutput):
- SV_Target0: BaseColor
- SV_Target1: 其他GBuffer
```

### 步骤2: 追踪数据流
从输出往回追踪每个变量的来源:
```
Output.BaseColor → 最后的颜色计算 → ... → 原始纹理采样
```

### 步骤3: 识别常量缓冲
从 `get_shader_info()` 返回的常量缓冲中提取参数:
- cb0[0-10]: 变换矩阵
- cb0[11-20]: 材质参数
- cb0[21+]: 光照参数

### 步骤4: 生成HLSL
将DXBC翻译为UE兼容的HLSL:
- 使用UE内置函数 (Texture2DSample, lerp等)
- 保持变量名有意义
- 添加注释说明

## 6. UE材质重建策略

### 优先级
1. **UE原生节点** - Panner, Fresnel, DepthFade, Lerp等
2. **内置MF** - FlowMaps, WorldAlignedTexture等
3. **CustomHLSL** - 仅用于复杂计算

### 常见DXBC → UE映射
| DXBC模式 | UE实现 |
|----------|--------|
| `frac(uv + time * speed)` | Panner节点 |
| `dot(N, V)` | Fresnel节点 |
| `1 - saturate(depth/fade)` | DepthFade节点 |
| `lerp(a, b, t)` | Lerp节点 |
| `saturate(x)` | Saturate节点 |
| `pow(x, y)` | Power节点 |

## 7. 注意事项

### 采样器限制
UE CustomHLSL中不能声明自定义采样器:
```hlsl
// ❌ 错误
SamplerState MySampler;

// ✅ 正确 - 使用内置采样器
Texture2DSample(Tex, View.SharedBilinearClampedSampler, UV);
```

### 输出类型
Custom节点输出类型必须匹配代码返回值:
| Output Type | Return Type |
|------------|-------------|
| Float1 | `float` |
| Float2 | `float2` |
| Float3 | `float3` |
| Float4 | `float4` |
