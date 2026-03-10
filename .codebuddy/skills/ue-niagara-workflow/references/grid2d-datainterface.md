# Grid2D DataInterface 实现原理

本文档分析 `NiagaraDataInterfaceGrid2DCollection` 的 UE 源码实现。

---

## 1. 类继承关系

```
UObject
└── UNiagaraDataInterface
    └── UNiagaraDataInterfaceGrid2D
        └── UNiagaraDataInterfaceGrid2DCollection
            └── UNiagaraDataInterfaceGrid2DCollectionReader (只读)
```

---

## 2. 核心数据结构

### 2.1 GameThread 实例数据

```cpp
struct FGrid2DCollectionRWInstanceData_GameThread
{
    bool ClearBeforeNonIterationStage = true;
    
    FIntPoint NumCells;              // 网格分辨率 (256x256)
    int32 NumAttributes;             // 属性数量
    FVector2D CellSize;              // 单元格大小 (世界单位)
    FVector2D WorldBBoxSize;         // 世界边界大小
    
    UTextureRenderTarget* TargetTexture;  // 输出渲染目标
    TArray<FNiagaraVariableBase> Vars;    // 属性变量列表
    TArray<uint32> Offsets;               // 属性偏移量
};
```

### 2.2 RenderThread 实例数据

```cpp
struct FGrid2DCollectionRWInstanceData_RenderThread
{
    FIntPoint NumCells;
    int32 NumAttributes;
    
    // 双缓冲: CurrentData 和 DestinationData
    TArray<TUniquePtr<FGrid2DBuffer>> Buffers;
    FGrid2DBuffer* CurrentData;      // 当前帧数据
    FGrid2DBuffer* DestinationData;  // 下一帧数据
    
    TArray<int32> AttributeIndices;
    TArray<FName> Vars;
    TArray<int32> VarComponents;
};
```

### 2.3 Shader 参数结构

```cpp
BEGIN_SHADER_PARAMETER_STRUCT(FNDIGrid2DShaderParameters, )
    SHADER_PARAMETER(int, NumAttributes)
    SHADER_PARAMETER(FVector2f, UnitToUV)        // 单元格到UV转换
    SHADER_PARAMETER(FIntPoint, NumCells)        // 分辨率
    SHADER_PARAMETER(FVector2f, CellSize)        // 单元格大小
    SHADER_PARAMETER(FVector2f, WorldBBoxSize)   // 世界边界
    
    SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2DArray<float>, Grid)        // 读取
    SHADER_PARAMETER_SAMPLER(SamplerState, GridSampler)                  // 采样器
    SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2DArray<float>, OutputGrid) // 写入
END_SHADER_PARAMETER_STRUCT()
```

---

## 3. GPU 存储实现

### 3.1 底层数据类型

```cpp
// 纹理类型: Texture2DArray
// 每个属性一个 slice，支持多属性
// 格式: RGBA32Float (默认) 或可配置
```

### 3.2 双缓冲机制

```
Frame N:
  CurrentData    → 读取 (GetPrevious*)
  DestinationData → 写入 (Set*)
  
Frame N+1:
  Swap(CurrentData, DestinationData)
```

**用途**:
- 平流操作需要读取"上一帧"的数据
- 避免读写冲突

### 3.3 属性布局

```
Texture2DArray Slice Index:
├── Slice 0: Velocity (float2) + [Padding]
├── Slice 1: Density (float)
├── Slice 2: Temperature (float)
├── Slice 3: Pressure (float)
├── Slice 4: Boundary (int)
└── ...

每个 Slice:
└── Width x Height = NumCells.X x NumCells.Y
```

---

## 4. HLSL 函数接口

### 4.1 核心函数名

| 函数名 | 用途 | 类型 |
|--------|------|------|
| `SetFloatValue` | 设置 float 属性 | Write |
| `GetFloatValue` | 获取当前帧 float | Read |
| `GetPreviousFloatValue` | 获取上一帧 float | Read |
| `SampleGridFloatValue` | 采样 float 属性 | Read |
| `CubicSamplePreviousGridFloatValue` | 三次采样 | Read |
| `SetVector2DValue` | 设置 float2 | Write |
| `SetVectorValue` | 设置 float3 | Write |
| `SetVector4Value` | 设置 float4 | Write |
| `GetFloatAttributeIndex` | 获取属性索引 | Utility |

### 4.2 生成的 HLSL 代码示例

**SetFloatValue**:
```hlsl
void SetFloatValue(int IndexX, int IndexY, int AttributeIndex, float Value)
{
    int3 WriteCoord = int3(IndexX, IndexY, AttributeIndex / 4);
    float4 CurrentValue = OutputGrid[WriteCoord];
    
    // 根据 AttributeIndex % 4 决定写入哪个通道
    int Component = AttributeIndex % 4;
    switch (Component)
    {
        case 0: CurrentValue.x = Value; break;
        case 1: CurrentValue.y = Value; break;
        case 2: CurrentValue.z = Value; break;
        case 3: CurrentValue.w = Value; break;
    }
    
    OutputGrid[WriteCoord] = CurrentValue;
}
```

**GetPreviousFloatValue**:
```hlsl
float GetPreviousFloatValue(int IndexX, int IndexY, int AttributeIndex)
{
    int3 ReadCoord = int3(IndexX, IndexY, AttributeIndex / 4);
    float4 Value = Grid[ReadCoord];
    
    int Component = AttributeIndex % 4;
    switch (Component)
    {
        case 0: return Value.x;
        case 1: return Value.y;
        case 2: return Value.z;
        case 3: return Value.w;
    }
    return 0.0f;
}
```

**SampleGridFloatValue**:
```hlsl
float SampleGridFloatValue(float2 UV, int AttributeIndex)
{
    int3 ReadCoord = int3(UV * NumCells, AttributeIndex / 4);
    float4 Value = Grid.SampleLevel(GridSampler, float3(UV, AttributeIndex / 4), 0);
    
    int Component = AttributeIndex % 4;
    // ... 返回对应通道
}
```

---

## 5. 计算调度

### 5.1 GPU Dispatch 配置

```cpp
virtual ENiagaraGpuDispatchType GetGpuDispatchType() const override 
{ 
    return ENiagaraGpuDispatchType::TwoD;  // 2D 线程组
}

virtual FIntVector GetGpuDispatchNumThreads() const 
{ 
    return FIntVector(8, 8, 1);  // 每组 8x8 线程
}
```

### 5.2 线程映射

```
总线程数 = NumCells.X × NumCells.Y
线程组数 = (NumCells.X / 8) × (NumCells.Y / 8)

每个线程:
  - 处理一个网格单元格
  - 根据模块逻辑读写属性
```

---

## 6. 代理系统 (Proxy System)

### 6.1 代理类

```cpp
struct FNiagaraDataInterfaceProxyGrid2DCollectionProxy : public FNiagaraDataInterfaceProxyRW
{
    // 每个 System Instance 一份实例数据
    TMap<FNiagaraSystemInstanceID, FGrid2DCollectionRWInstanceData_RenderThread> SystemInstancesToProxyData_RT;
    
    // 生命周期回调
    virtual void ResetData(const FNDIGpuComputeResetContext& Context) override;
    virtual void PreStage(const FNDIGpuComputePreStageContext& Context) override;
    virtual void PostStage(const FNDIGpuComputePostStageContext& Context) override;
    virtual void PostSimulate(const FNDIGpuComputePostSimulateContext& Context) override;
    virtual void GetDispatchArgs(const FNDIGpuComputeDispatchArgsGenContext& Context) override;
};
```

### 6.2 帧生命周期

```
PreStage:
  ├── 检查是否需要 Clear
  ├── 准备双缓冲
  └── 设置 RenderGraph 资源

Execute (Compute Shader):
  └── 读写 Grid2D 数据

PostStage:
  └── 处理迭代循环逻辑

PostSimulate:
  ├── 交换双缓冲
  └── 复制到 RenderTarget (如有)
```

---

## 7. 渲染目标输出

### 7.1 用户参数绑定

```cpp
UPROPERTY(EditAnywhere, Category = "Grid")
FNiagaraUserParameterBinding RenderTargetUserParameter;
```

**用法**:
1. 在 Niagara System 中创建 `User.RenderTarget` 参数 (TextureRenderTarget2D)
2. 在 Emitter 的 Output 阶段自动将 Grid 数据写入该 RT

### 7.2 数据复制

```cpp
// 从 Texture2DArray 复制到 Texture2D (Tiled 布局)
// 用于材质采样或后处理
void CopyToRenderTarget(FRDGBuilder& GraphBuilder, ...);
```

**Tiled 布局**:
```
RenderTarget (2D):
┌────────────────────────────┐
│ Attr0 │ Attr1 │ Attr2 │... │  ← 每个 tile 是一个属性
│  NxN  │  NxN  │  NxN  │    │
└────────────────────────────┘
```

---

## 8. 格式配置

### 8.1 支持的格式

```cpp
enum ENiagaraGpuBufferFormat
{
    Float,      // 32-bit float (RGBA32Float)
    Half,       // 16-bit float (RGBA16Float)
    Integer,    // 整数格式
};
```

### 8.2 格式选择

```cpp
// 项目默认设置
ENiagaraGpuBufferFormat DefaultFormat = GetDefault<UNiagaraSettings>()->DefaultGpuBufferFormat;

// 单个 Grid2D 可覆盖
if (bOverrideFormat)
{
    Format = OverrideBufferFormat;
}
```

**性能考虑**:
| 格式 | 精度 | 内存 | 适用场景 |
|------|------|------|----------|
| Float | 高 | 16 bytes/cell | 物理模拟 |
| Half | 中 | 8 bytes/cell | 视觉效果 |
| Integer | 整数 | 16 bytes/cell | 索引数据 |

---

## 9. 调试功能

### 9.1 预览网格

```cpp
#if WITH_EDITORONLY_DATA
UPROPERTY(EditAnywhere, Category = "Grid")
uint8 bPreviewGrid : 1;

UPROPERTY(EditAnywhere, Category = "Grid")
FName PreviewAttribute = NAME_None;
#endif
```

**用法**: 在编辑器中启用 `bPreviewGrid`，选择要预览的属性，可在视口中显示网格数据。

### 9.2 控制台变量

```
fx.Niagara.Grid2D.ResolutionMultiplier  # 分辨率全局缩放
fx.Niagara.Grid2D.OverrideFormat        # 格式覆盖 (-1 = 不覆盖)
fx.Niagara.Grid2D.CubicInterpMethod     # 三次插值方法 (0=Bridson, 1=Fedkiw)
```

---

## 10. 性能优化

### 10.1 内存估算

```
内存 = NumCells.X × NumCells.Y × NumAttributes × 4 (float32)
      或 NumCells.X × NumCells.Y × (NumAttributes / 4) × 16 (RGBA32Float)

示例:
  256×256 分辨率, 8 属性
  = 256 × 256 × 2 × 16 = 2 MB (单缓冲)
  = 4 MB (双缓冲)
```

### 10.2 带宽优化

- 使用 Half 格式减少 50% 带宽
- 减少不必要的属性数量
- 合并相关属性到一个 float4 (如 Velocity.x/y)

---

## 11. 源码位置

| 文件 | 路径 |
|------|------|
| 头文件 | `Engine/Plugins/FX/Niagara/Source/Niagara/Classes/NiagaraDataInterfaceGrid2DCollection.h` |
| 实现 | `Engine/Plugins/FX/Niagara/Source/Niagara/Private/NiagaraDataInterfaceGrid2DCollection.cpp` |
| Shader | `Engine/Plugins/FX/Niagara/Shaders/NiagaraDataInterfaceGrid2DCollection.ush` |
