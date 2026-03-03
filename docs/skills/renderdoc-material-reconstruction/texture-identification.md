# Texture Identification Guide

本指南帮助AI识别RenderDoc中纹理的用途。

## 1. 纹理识别维度

### 格式维度
| 格式 | 常见用途 |
|------|---------|
| BC1/DXT1 | 压缩颜色(无Alpha) |
| BC3/DXT5 | 颜色+Alpha 或 数据 |
| BC4 | 单通道数据(高度/遮罩) |
| BC5 | 法线图(两通道) |
| BC6H | HDR纹理 |
| BC7 | 高质量颜色 |
| R8G8B8A8 | 未压缩RGBA |

### 尺寸维度
| 尺寸特征 | 可能用途 |
|---------|---------|
| 2的幂次(256,512,1024) | 标准纹理 |
| 非正方形(128x32) | FlowMap/梯度 |
| 小尺寸(64x64以下) | LUT/噪声 |
| 大尺寸(2048+) | 主纹理 |

### 采样模式
| 采样特征 | 用途推断 |
|---------|---------|
| 标准UV采样 | 常规纹理 |
| 三平面采样 | 地形/物体投影 |
| 立方体贴图采样 | 环境/反射 |
| 带LOD采样 | 远距离细节 |
| 带偏移采样 | 程序化效果 |

## 2. 纹理命名模式

### 常见命名约定
| 命名模式 | 用途 |
|---------|------|
| *Albedo*, *Diffuse*, *Base*, *Color* | BaseColor |
| *Normal*, *Nrm*, *NM* | Normal Map |
| *ORM*, *ARM*, *AO* | ORM贴图 |
| *Roughness*, *Rough*, *R* | 粗糙度 |
| *Metallic*, *Metal*, *M* | 金属度 |
| *Emissive*, *Emit*, *Glow* | 自发光 |
| *Flow*, *FlowMap* | 流动图 |
| *Caustics*, *Caust* | 焦散 |
| *Noise*, *NoiseMap* | 噪声 |
| *Mask*, *Alpha* | 遮罩 |
| *Detail*, *Det* | 细节纹理 |

## 3. 着色器绑定分析

### 从Pipeline State分析
```python
# 获取管线状态
pipeline = get_pipeline_state(event_id)

# 分析SRV绑定
for srv in pipeline["srvs"]:
    slot = srv["slot"]
    name = srv["name"]
    resource_id = srv["resource_id"]
    
    # 根据槽位推断用途
    if slot == 0:
        print(f"t0: 可能是BaseColor - {name}")
    elif slot == 1:
        print(f"t1: 可能是Normal - {name}")
    # ...
```

### 常见槽位分配
| 槽位 | 常见用途 |
|------|---------|
| t0 | BaseColor / Albedo |
| t1 | Normal Map |
| t2 | ORM / Masks |
| t3 | Emissive / Detail |
| t4-t7 | 特效纹理 |
| t8+ | 光照/阴影 |

## 4. 纹理内容分析

### 颜色分布
| 颜色特征 | 用途 |
|---------|------|
| 彩色、自然色彩 | BaseColor |
| 蓝紫色调 | Normal Map (未压缩) |
| 单色/灰度 | 高度/遮罩 |
| RG通道编码 | Normal/FlowMap |
| RGB分离通道 | ORM |

### 值范围
| 值范围 | 用途 |
|-------|------|
| 0-1正常分布 | 颜色/数据 |
| 高对比度 | 遮罩 |
| 平滑渐变 | 梯度/FlowMap |
| 随机噪声 | Noise |

## 5. 特殊纹理类型

### Flow Map
```
特征:
- RG通道编码流动方向
- 通常为小尺寸(128x32等)
- 值范围0-1，中心为0.5
- 用于水体、流动效果

识别: 
名称含"flow"
RG通道呈现方向性渐变
```

### Normal Map
```
特征:
- 蓝紫色调
- BC5格式(双通道)或BC1/BC3(编码)
- 值中心为(0.5, 0.5, 1.0)
- 用于表面细节

识别:
名称含"normal"
格式BC5或解码后为法线
```

### ORM Map
```
特征:
- 三通道分离
- R: Ambient Occlusion
- G: Roughness
- B: Metallic
- 通常为灰度图

识别:
名称含"orm", "arm"
三通道独立使用
```

### LUT (Look-Up Table)
```
特征:
- 长条形(256x16等)
- 颜色渐变
- 用于颜色校正/色调映射

识别:
尺寸为1D纹理形状
颜色渐变排列
```

## 6. 导入参数决策

### 决策矩阵
```python
def determine_import_params(texture_info, shader_context):
    """根据纹理信息决定导入参数"""
    
    name = texture_info["name"].lower()
    format = texture_info["format"]
    
    # BaseColor
    if any(x in name for x in ["albedo", "diffuse", "base", "color"]):
        return {
            "compression_settings": "Default",
            "srgb": True,
            "sampler_type": "Color"
        }
    
    # Normal Map
    elif any(x in name for x in ["normal", "nrm"]):
        return {
            "compression_settings": "TC_NormalMap",
            "srgb": False,
            "sampler_type": "Normal"  # 关键!
        }
    
    # ORM
    elif any(x in name for x in ["orm", "arm", "ao"]):
        return {
            "compression_settings": "Default",
            "srgb": False,
            "sampler_type": "Color"
        }
    
    # Flow Map
    elif "flow" in name:
        return {
            "compression_settings": "Default",
            "srgb": False,
            "sampler_type": "Color"
        }
    
    # 默认
    else:
        return {
            "compression_settings": "Default",
            "srgb": True,
            "sampler_type": "Color"
        }
```

## 7. 验证方法

### 导入后验证
```python
# 1. 检查纹理属性
texture_info = get_texture_info(resource_id)

# 2. 确认格式正确
if texture_info["format"] == "BC5":
    # 法线图应使用TC_NormalMap
    assert import_params["compression_settings"] == "TC_NormalMap"

# 3. 确认sRGB设置
if texture_info["is_srgb"]:
    # sRGB纹理不应是Normal
    assert import_params["sampler_type"] != "Normal"
```

## 8. 常见错误避免

### 法线图错误
```python
# ❌ 错误: 不设置sampler_type
import_texture(path, name, compression_settings="TC_NormalMap")
# 导致: Sampler type is Color, should be Normal

# ✅ 正确: 设置sampler_type
import_texture(path, name, 
               compression_settings="TC_NormalMap",
               sampler_type="Normal")  # 关键!
```

### sRGB错误
```python
# ❌ 错误: 数据纹理使用sRGB
import_texture(path, "T_Normal", srgb=True)  # 法线会变色

# ✅ 正确: 数据纹理禁用sRGB
import_texture(path, "T_Normal", srgb=False)
```
