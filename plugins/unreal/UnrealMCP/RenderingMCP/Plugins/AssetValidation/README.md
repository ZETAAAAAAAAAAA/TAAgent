# AssetValidation Plugin

Niagara System 资产验证与性能分析工具。

## 功能概览

### 静态分析

对 Niagara System 进行静态配置检查：

| 检查类别 | 检查项目 |
|---------|---------|
| **发射器** | 发射器数量、GPU/CPU 发射器数量、禁用发射器、混合模拟警告 |
| **粒子** | 总粒子数估算、单发射器粒子数 |
| **渲染器** | Sprite/Mesh/Ribbon/Light 渲染器数量 |
| **内存** | GPU 内存估算、CPU 内存估算、纹理内存 |

### 动态分析

运行时 Overdraw 过度绘制分析：

1. 自动在编辑器世界生成 Niagara 系统
2. 基于包围盒计算相机位置
3. 切换到 Quad Overdraw 视图模式
4. 按时间间隔捕获帧序列
5. 分析像素颜色计算最大 Overdraw 值
6. 保存最大 Overdraw 帧截图

### Overdraw 颜色图例

| 颜色 | Overdraw 范围 | 性能影响 |
|------|--------------|---------|
| 绿色 | 0-2 | 低 |
| 黄色 | 3-5 | 中 |
| 红色 | 6-10 | 高 |
| 紫色 | >10 | 极高 |

## 使用方法

1. 在内容浏览器中选择 Niagara System 资产
2. 右键菜单 → **Validate Niagara System**
3. 等待静态分析和动态分析完成
4. 查看验证报告窗口

## 配置

编辑 `EditorPerProjectUserSettings.ini` 自定义验证阈值：

```ini
[/Script/AssetValidation.NiagaraValidationConfig]
; 粒子数量阈值
ParticleCountWarningThreshold=50000
ParticleCountErrorThreshold=200000
TotalParticlesWarningThreshold=100000
TotalParticlesErrorThreshold=500000

; 发射器配置
MaxEmittersWarning=15
MaxEmittersError=30
bWarnOnMixedSimulation=True
bWarnOnGPUAutoAllocation=True

; 内存阈值
MemoryWarningThresholdMB=50
MemoryErrorThresholdMB=200

; 检查开关
bCheckParticleCount=True
bCheckEmitterConfig=True
bCheckRendererConfig=True
bCheckMemory=True
```

## 文件结构

```
AssetValidation/
├── Source/AssetValidation/
│   ├── Public/
│   │   ├── AssetValidationModule.h      # 模块入口
│   │   ├── AssetValidationResult.h      # 结果数据结构
│   │   ├── NiagaraSystemValidator.h     # 静态验证器
│   │   ├── NiagaraOverdrawAnalyzer.h    # 动态分析器
│   │   ├── NiagaraValidationConfig.h    # 配置类
│   │   └── UI/
│   │       └── ValidationReportWidget.h # 报告窗口
│   └── Private/
│       ├── AssetValidationModule.cpp
│       ├── NiagaraSystemValidator.cpp
│       ├── NiagaraOverdrawAnalyzer.cpp
│       ├── NiagaraValidationConfig.cpp
│       └── UI/
│           └── ValidationReportWidget.cpp
└── AssetValidation.uplugin
```

## 依赖

- Niagara 模块
- 项目需启用 `DebugViewModes` 以支持 Quad Overdraw 视图模式

## API 用法

### 静态验证

```cpp
#include "NiagaraSystemValidator.h"

UNiagaraSystem* MySystem = ...;
FAssetValidationResult Result = UNiagaraSystemValidator::ValidateSystem(MySystem);

if (Result.HasIssues())
{
    // 处理验证问题
    for (const auto& Check : Result.Checks)
    {
        if (!Check.bPassed)
        {
            UE_LOG(LogTemp, Warning, TEXT("%s: %s vs %s"), 
                *Check.CheckName, *Check.CurrentValue, *Check.LimitValue);
        }
    }
}
```

### 动态 Overdraw 分析

```cpp
#include "NiagaraOverdrawAnalyzer.h"

FNiagaraOverdrawAnalyzer::FAnalysisConfig Config;
Config.DurationSeconds = 5.0f;
Config.CaptureInterval = 0.1f;
Config.MaxOverdrawThreshold = 10;
Config.bSaveScreenshots = true;

FAnalysisResult Result = FNiagaraOverdrawAnalyzer::AnalyzeOverdraw(MySystem, Config);

if (Result.bSuccess)
{
    UE_LOG(LogTemp, Log, TEXT("Max Overdraw: %d"), Result.MaxOverdraw);
    UE_LOG(LogTemp, Log, TEXT("Passed: %s"), Result.bPassed ? TEXT("Yes") : TEXT("No"));
}
```
