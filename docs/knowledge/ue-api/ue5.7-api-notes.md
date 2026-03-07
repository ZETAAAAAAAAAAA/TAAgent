# UE 5.7+ API Notes

Engine-specific API notes for Unreal Engine 5.7+.

## Engine Directory

- **引擎路径**: `E:\UE\UE_5.7`
- **Niagara 插件**: `E:\UE\UE_5.7\Engine\Plugins\FX\Niagara`

---

## Stateless Niagara Headers (UE 5.7+)

Stateless 相关头文件位于 `Internal/Stateless/` 目录：

| 头文件 | 路径 |
|--------|------|
| `NiagaraStatelessEmitter.h` | `Niagara/Internal/Stateless/NiagaraStatelessEmitter.h` |
| `NiagaraStatelessModule.h` | `Niagara/Internal/Stateless/NiagaraStatelessModule.h` |
| `NiagaraStatelessEmitterTemplate.h` | `Niagara/Internal/Stateless/NiagaraStatelessEmitterTemplate.h` |
| `NiagaraStatelessCommon.h` | `Niagara/Internal/Stateless/NiagaraStatelessCommon.h` |

### Build.cs 配置

要访问 Stateless Internal API，需要在 Build.cs 中添加包含路径：

```csharp
// UE 5.7+ Stateless Niagara support - add Internal include path
if (Target.bBuildEditor == true)
{
    string NiagaraPath = System.IO.Path.Combine(Target.UEEngineDirectory, "Plugins", "FX", "Niagara", "Source", "Niagara");
    if (System.IO.Directory.Exists(System.IO.Path.Combine(NiagaraPath, "Internal")))
    {
        PublicIncludePaths.Add(System.IO.Path.Combine(NiagaraPath, "Internal"));
    }
}
```

### 包含方式

添加 Internal 路径后，使用以下方式包含：

```cpp
#include "Stateless/NiagaraStatelessEmitter.h"
#include "Stateless/NiagaraStatelessModule.h"
#include "Stateless/NiagaraStatelessEmitterTemplate.h"
```

---

## Stateless vs Standard Niagara

| 特性 | Standard | Stateless |
|------|----------|-----------|
| 执行方式 | VM 脚本执行 | GPU Compute Shader |
| 内存占用 | 较高（粒子池） | 极低（无状态） |
| 参数系统 | Script RapidIterationParameters | Module Properties |
| 适用场景 | 复杂逻辑、事件驱动 | 简单效果、高性能需求 |

### Stateless 模块列表 (28 modules)

- **Spawn**: Lifetime, SpawnRate
- **Size**: Size, Scale
- **Position**: Position, AddPosition
- **Velocity**: Velocity, AddVelocity
- **Color**: Color
- **Rotation**: Rotation, MeshRotation, Alignment
- **Physics**: Collision, Drag, CurlNoise
- **Other**: SubUV, Mesh, Material etc.

---

## Standard → Stateless Module Mapping

| Standard Module | Stateless Module | Status |
|-----------------|------------------|--------|
| Lifetime | NiagaraStatelessLifetimeModule | ✅ |
| Color | NiagaraStatelessColorModule | ✅ |
| SpriteSize/Size | NiagaraStatelessSizeModule | ✅ |
| Scale | NiagaraStatelessScaleModule | ✅ |
| SpriteRotation | NiagaraStatelessRotationModule | ✅ |
| Velocity/AddVelocity | NiagaraStatelessVelocityModule | ✅ |
| Position/AddPosition | NiagaraStatelessPositionModule | ✅ |
| SpawnRate | NiagaraStatelessSpawnRateModule | ✅ |
| CurlNoise | NiagaraStatelessCurlNoiseModule | ✅ |
| Drag | NiagaraStatelessDragModule | ✅ |
| Collision | NiagaraStatelessCollisionModule | ✅ |
| Event | - | ❌ Not supported |
| Ribbon | - | ❌ Not supported |

---

## Other UE5.7 API Notes

| Note | Details |
|------|---------|
| `TVertexInstanceAttributesRef` | Not `TVertiveDataAttributesRef` |
| `FPolygonGroupArray` | Use `GetFirstValidID()` not `[0]` |
| `BuildFromMeshDescriptions()` | Requires array parameter |
| `EngineUtils.h` | Contains `TActorIterator` |
| `ULightComponent` | Has `SetIntensity/SetLightColor`, `ULightComponentBase` does not |
| `bAffectsWorld` | Property, direct assignment not setter |
| `IImageWrapper::GetCompressed(Quality)` | Quality parameter passed directly |
| `IAssetViewport::GetSharedActiveViewport()` | Returns `TSharedPtr<FSceneViewport>` |
| Base64 include | `#include "Misc/Base64.h"` (not `HAL/Base64.h`) |
