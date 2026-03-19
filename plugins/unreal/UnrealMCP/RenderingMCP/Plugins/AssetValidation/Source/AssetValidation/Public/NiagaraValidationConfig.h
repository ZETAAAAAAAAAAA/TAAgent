// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraValidationConfig.generated.h"

/** Platform-specific performance budgets */
USTRUCT(BlueprintType)
struct FPlatformBudget
{
	GENERATED_BODY()

	/** Maximum recommended particles for this platform */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Budget")
	int32 MaxParticles = 100000;

	/** Maximum recommended GPU emitters */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Budget")
	int32 MaxGPUEmitters = 10;

	/** Maximum recommended CPU emitters */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Budget")
	int32 MaxCPUEmitters = 5;

	/** Maximum recommended emitters per system */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Budget")
	int32 MaxEmittersPerSystem = 20;

	/** Maximum estimated memory in MB */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Budget")
	int32 MaxEstimatedMemoryMB = 100;
};

/** Niagara validation configuration */
UCLASS(Config = EditorPerProjectUserSettings, DefaultConfig)
class ASSETVALIDATION_API UNiagaraValidationConfig : public UObject
{
	GENERATED_BODY()

public:
	/** Get the configuration singleton */
	static UNiagaraValidationConfig* Get();

	// ============================================
	// Particle Count Thresholds
	// ============================================
	
	/** Warning threshold for single emitter particle count */
	UPROPERTY(EditAnywhere, Config, Category = "Particle Count", meta = (DisplayName = "Warning: Single Emitter Particles"))
	int32 ParticleCountWarningThreshold = 50000;

	/** Error threshold for single emitter particle count */
	UPROPERTY(EditAnywhere, Config, Category = "Particle Count", meta = (DisplayName = "Error: Single Emitter Particles"))
	int32 ParticleCountErrorThreshold = 200000;

	/** Warning threshold for total system particles */
	UPROPERTY(EditAnywhere, Config, Category = "Particle Count", meta = (DisplayName = "Warning: Total System Particles"))
	int32 TotalParticlesWarningThreshold = 100000;

	/** Error threshold for total system particles */
	UPROPERTY(EditAnywhere, Config, Category = "Particle Count", meta = (DisplayName = "Error: Total System Particles"))
	int32 TotalParticlesErrorThreshold = 500000;

	// ============================================
	// Emitter Configuration
	// ============================================

	/** Maximum emitters before warning */
	UPROPERTY(EditAnywhere, Config, Category = "Emitter Config", meta = (DisplayName = "Max Emitters (Warning)"))
	int32 MaxEmittersWarning = 15;

	/** Maximum emitters before error */
	UPROPERTY(EditAnywhere, Config, Category = "Emitter Config", meta = (DisplayName = "Max Emitters (Error)"))
	int32 MaxEmittersError = 30;

	/** Warn when mixing GPU and CPU emitters */
	UPROPERTY(EditAnywhere, Config, Category = "Emitter Config", meta = (DisplayName = "Warn on Mixed Simulation"))
	bool bWarnOnMixedSimulation = true;

	/** Warn when GPU emitter uses automatic allocation */
	UPROPERTY(EditAnywhere, Config, Category = "Emitter Config", meta = (DisplayName = "Warn on GPU Auto Allocation"))
	bool bWarnOnGPUAutoAllocation = true;

	// ============================================
	// Memory Budgets
	// ============================================

	/** Estimated bytes per GPU particle (for memory calculation) */
	UPROPERTY(EditAnywhere, Config, Category = "Memory", meta = (DisplayName = "Bytes per GPU Particle"))
	int32 BytesPerGPUParticle = 64;

	/** Estimated bytes per CPU particle (for memory calculation) */
	UPROPERTY(EditAnywhere, Config, Category = "Memory", meta = (DisplayName = "Bytes per CPU Particle"))
	int32 BytesPerCPUParticle = 128;

	/** Memory warning threshold in MB */
	UPROPERTY(EditAnywhere, Config, Category = "Memory", meta = (DisplayName = "Memory Warning (MB)"))
	int32 MemoryWarningThresholdMB = 50;

	/** Memory error threshold in MB */
	UPROPERTY(EditAnywhere, Config, Category = "Memory", meta = (DisplayName = "Memory Error (MB)"))
	int32 MemoryErrorThresholdMB = 200;

	// ============================================
	// Renderer Configuration
	// ============================================

	/** Warn when emitter has no enabled renderers */
	UPROPERTY(EditAnywhere, Config, Category = "Renderer Config", meta = (DisplayName = "Warn on No Renderers"))
	bool bWarnOnNoRenderers = true;

	/** Maximum mesh renderers per system */
	UPROPERTY(EditAnywhere, Config, Category = "Renderer Config", meta = (DisplayName = "Max Mesh Renderers"))
	int32 MaxMeshRenderers = 10;

	// ============================================
	// Platform Budgets
	// ============================================

	/** Budget for high-end PC platforms */
	UPROPERTY(EditAnywhere, Config, Category = "Platform Budgets", meta = (DisplayName = "High-End PC"))
	FPlatformBudget HighEndPCBudget;

	/** Budget for console platforms */
	UPROPERTY(EditAnywhere, Config, Category = "Platform Budgets", meta = (DisplayName = "Console"))
	FPlatformBudget ConsoleBudget;

	/** Budget for mobile platforms */
	UPROPERTY(EditAnywhere, Config, Category = "Platform Budgets", meta = (DisplayName = "Mobile"))
	FPlatformBudget MobileBudget;

	// ============================================
	// Check Toggles
	// ============================================

	/** Enable particle count checks */
	UPROPERTY(EditAnywhere, Config, Category = "Check Toggles", meta = (DisplayName = "Check Particle Count"))
	bool bCheckParticleCount = true;

	/** Enable emitter configuration checks */
	UPROPERTY(EditAnywhere, Config, Category = "Check Toggles", meta = (DisplayName = "Check Emitter Config"))
	bool bCheckEmitterConfig = true;

	/** Enable renderer configuration checks */
	UPROPERTY(EditAnywhere, Config, Category = "Check Toggles", meta = (DisplayName = "Check Renderer Config"))
	bool bCheckRendererConfig = true;

	/** Enable memory estimation checks */
	UPROPERTY(EditAnywhere, Config, Category = "Check Toggles", meta = (DisplayName = "Check Memory"))
	bool bCheckMemory = true;

	/** Enable scalability checks */
	UPROPERTY(EditAnywhere, Config, Category = "Check Toggles", meta = (DisplayName = "Check Scalability"))
	bool bCheckScalability = true;

public:
	UNiagaraValidationConfig();
};
