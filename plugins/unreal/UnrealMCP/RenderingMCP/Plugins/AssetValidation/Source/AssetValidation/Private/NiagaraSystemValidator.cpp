// Copyright Epic Games, Inc. All Rights Reserved.

#include "NiagaraSystemValidator.h"
#include "NiagaraValidationConfig.h"
#include "NiagaraSystem.h"
#include "NiagaraEmitter.h"
#include "NiagaraEmitterHandle.h"
#include "NiagaraRendererProperties.h"
#include "NiagaraTypes.h"
#include "TextureResource.h"
#include "Engine/Texture.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "AssetRegistry/AssetRegistryModule.h"

// Helper to add a check
static void AddCheck(TArray<FAssetValidationCheck>& Checks, const FString& Name, const FString& Limit, 
	const FString& Current, bool bPassed, EAssetCheckSeverity Severity = EAssetCheckSeverity::Warning, 
	const FString& Category = TEXT("General"))
{
	FAssetValidationCheck Check;
	Check.CheckName = Name;
	Check.LimitValue = Limit;
	Check.CurrentValue = Current;
	Check.bPassed = bPassed;
	Check.Severity = Severity;
	Check.Category = Category;
	Checks.Add(Check);
}

// Helper to format number with commas
static FString FormatNumber(int32 Value)
{
	FString Result = FString::FromInt(Value);
	int32 InsertPos = Result.Len() - 3;
	while (InsertPos > 0)
	{
		Result.InsertAt(InsertPos, TEXT(','));
		InsertPos -= 3;
	}
	return Result;
}

// Helper to format memory
static FString FormatMemory(int64 Bytes)
{
	if (Bytes >= 1024 * 1024)
	{
		return FString::Printf(TEXT("%.1f MB"), Bytes / (1024.0 * 1024.0));
	}
	else if (Bytes >= 1024)
	{
		return FString::Printf(TEXT("%.1f KB"), Bytes / 1024.0);
	}
	return FString::Printf(TEXT("%d B"), (int32)Bytes);
}

FAssetValidationResult UNiagaraSystemValidator::ValidateSystem(UNiagaraSystem* System)
{
	FAssetValidationResult Result;
	
	if (!System)
	{
		return Result;
	}

	UNiagaraValidationConfig* Config = UNiagaraValidationConfig::Get();

	Result.SystemName = System->GetFName();
	Result.SystemPath = System->GetPathName();

	// Get emitter handles
	const TArray<FNiagaraEmitterHandle>& EmitterHandles = System->GetEmitterHandles();
	Result.EmitterCount = EmitterHandles.Num();

	// Track statistics
	int32 TotalEnabledRenderers = 0;
	int32 DisabledEmitterCount = 0;
	int32 MeshRendererCount = 0;
	int32 SpriteRendererCount = 0;
	int32 RibbonRendererCount = 0;
	int32 LightRendererCount = 0;

	// Analyze each emitter
	for (const FNiagaraEmitterHandle& Handle : EmitterHandles)
	{
		if (!Handle.GetIsEnabled())
		{
			DisabledEmitterCount++;
			continue;
		}

		FVersionedNiagaraEmitter VersionedEmitter = Handle.GetInstance();
		FVersionedNiagaraEmitterData* EmitterData = VersionedEmitter.GetEmitterData();
		
		if (!EmitterData)
		{
			continue;
		}

		// Count GPU vs CPU emitters
		if (EmitterData->SimTarget == ENiagaraSimTarget::GPUComputeSim)
		{
			Result.GPUEmitterCount++;
		}
		else
		{
			Result.CPUEmitterCount++;
		}

		// Get max particle estimate
		int32 MaxParticles = EmitterData->GetMaxParticleCountEstimate();
		Result.EstimatedMaxParticles += MaxParticles;

		// Count renderers and collect texture/mesh references
		const TArray<UNiagaraRendererProperties*>& Renderers = EmitterData->GetRenderers();
		Result.TotalRendererCount += Renderers.Num();

		int32 EnabledRendererCount = 0;
		for (UNiagaraRendererProperties* Renderer : Renderers)
		{
			if (Renderer && Renderer->GetIsEnabled())
			{
				EnabledRendererCount++;
				TotalEnabledRenderers++;

				FString RendererType = Renderer->GetClass()->GetName();
				if (RendererType.Contains(TEXT("Mesh")))
				{
					MeshRendererCount++;
					// TODO: Extract mesh references from renderer
				}
				else if (RendererType.Contains(TEXT("Sprite"))) 
				{
					SpriteRendererCount++;
				}
				else if (RendererType.Contains(TEXT("Ribbon"))) 
				{
					RibbonRendererCount++;
				}
				else if (RendererType.Contains(TEXT("Light"))) 
				{
					LightRendererCount++;
				}
			}
		}

		// ============================================
		// Emitter-level Checks
		// ============================================

		// Check: No renderers enabled
		if (Config->bCheckRendererConfig && EnabledRendererCount == 0)
		{
			Result.Issues.Add(FAssetValidationIssue(
				TEXT("NoRenderers"),
				FString::Printf(TEXT("Emitter '%s' has no enabled renderers"), *Handle.GetName().ToString()),
				EAssetCheckSeverity::Warning,
				TEXT("Enable at least one renderer or disable the emitter")
			));
		}

		// Check: High particle count for single emitter
		if (Config->bCheckParticleCount)
		{
			if (MaxParticles > Config->ParticleCountErrorThreshold)
			{
				Result.Issues.Add(FAssetValidationIssue(
					TEXT("VeryHighParticleCount"),
					FString::Printf(TEXT("Emitter '%s' has extremely high particle count: %d (limit: %d)"), 
						*Handle.GetName().ToString(), MaxParticles, Config->ParticleCountErrorThreshold),
					EAssetCheckSeverity::Error,
					TEXT("Reduce particle count immediately - this will cause severe performance issues")
				));
			}
			else if (MaxParticles > Config->ParticleCountWarningThreshold)
			{
				Result.Issues.Add(FAssetValidationIssue(
					TEXT("HighParticleCount"),
					FString::Printf(TEXT("Emitter '%s' has high particle count: %d (recommended: <%d)"), 
						*Handle.GetName().ToString(), MaxParticles, Config->ParticleCountWarningThreshold),
					EAssetCheckSeverity::Warning,
					TEXT("Consider reducing particle count or using LOD/culling")
				));
			}
		}

		// Check: GPU allocation mode
		if (Config->bCheckEmitterConfig && EmitterData->SimTarget == ENiagaraSimTarget::GPUComputeSim)
		{
			if (EmitterData->AllocationMode == EParticleAllocationMode::AutomaticEstimate && Config->bWarnOnGPUAutoAllocation)
			{
				Result.Issues.Add(FAssetValidationIssue(
					TEXT("GPUAutoAllocation"),
					FString::Printf(TEXT("GPU emitter '%s' uses automatic allocation"), *Handle.GetName().ToString()),
					EAssetCheckSeverity::Info,
					TEXT("Consider using FixedCount or ManualEstimate for better memory control")
				));
			}
		}

		// Check: Persistent IDs
		if (Config->bCheckMemory && EmitterData->RequiresPersistentIDs())
		{
			Result.Issues.Add(FAssetValidationIssue(
				TEXT("PersistentIDs"),
				FString::Printf(TEXT("Emitter '%s' uses persistent IDs"), *Handle.GetName().ToString()),
				EAssetCheckSeverity::Info,
				TEXT("Persistent IDs add memory overhead (~8 bytes/particle) - only enable if needed for particle spawning events")
			));
		}

		// Check: GPU emitter with very high particle count
		if (Config->bCheckParticleCount && EmitterData->SimTarget == ENiagaraSimTarget::GPUComputeSim && MaxParticles > 500000)
		{
			Result.Issues.Add(FAssetValidationIssue(
				TEXT("GPUMemoryPressure"),
				FString::Printf(TEXT("GPU emitter '%s' may cause GPU memory pressure: %d particles (~%.1f MB)"), 
					*Handle.GetName().ToString(), MaxParticles, MaxParticles * Config->BytesPerGPUParticle / (1024.0 * 1024.0)),
				EAssetCheckSeverity::Warning,
				TEXT("Reduce particle count or split into multiple systems")
			));
		}

		// Check: CPU emitter with high particle count
		if (Config->bCheckParticleCount && EmitterData->SimTarget != ENiagaraSimTarget::GPUComputeSim && MaxParticles > 10000)
		{
			Result.Issues.Add(FAssetValidationIssue(
				TEXT("CPUHighParticles"),
				FString::Printf(TEXT("CPU emitter '%s' has high particle count: %d (recommended for CPU: <10,000)"), 
					*Handle.GetName().ToString(), MaxParticles),
				EAssetCheckSeverity::Warning,
				TEXT("Consider switching to GPU simulation or reducing particle count")
			));
		}
	}

	// ============================================
	// Calculate Memory Estimates
	// ============================================
	
	Result.EstimatedGPUMemoryBytes = static_cast<int64>(Result.GPUEmitterCount) * Result.EstimatedMaxParticles * Config->BytesPerGPUParticle;
	Result.EstimatedCPUMemoryBytes = static_cast<int64>(Result.CPUEmitterCount) * Result.EstimatedMaxParticles * Config->BytesPerCPUParticle;
	int64 TotalEstimatedMemory = Result.EstimatedGPUMemoryBytes + Result.EstimatedCPUMemoryBytes;

	// ============================================
	// Build Structured Check List (Four-Column Format)
	// ============================================

	// --- Emitter Checks ---
	{
		AddCheck(Result.Checks, 
			TEXT("Max Emitter Count"), 
			FString::Printf(TEXT("≤ %d"), Config->MaxEmittersWarning),
			FormatNumber(Result.EmitterCount),
			Result.EmitterCount <= Config->MaxEmittersWarning,
			Result.EmitterCount > Config->MaxEmittersError ? EAssetCheckSeverity::Error : EAssetCheckSeverity::Warning,
			TEXT("Emitters"));

		AddCheck(Result.Checks, 
			TEXT("GPU Emitter Count"), 
			FString::Printf(TEXT("≤ %d"), Config->HighEndPCBudget.MaxGPUEmitters),
			FormatNumber(Result.GPUEmitterCount),
			Result.GPUEmitterCount <= Config->HighEndPCBudget.MaxGPUEmitters,
			EAssetCheckSeverity::Warning,
			TEXT("Emitters"));

		AddCheck(Result.Checks, 
			TEXT("CPU Emitter Count"), 
			FString::Printf(TEXT("≤ %d"), Config->HighEndPCBudget.MaxCPUEmitters),
			FormatNumber(Result.CPUEmitterCount),
			Result.CPUEmitterCount <= Config->HighEndPCBudget.MaxCPUEmitters,
			EAssetCheckSeverity::Warning,
			TEXT("Emitters"));

		if (DisabledEmitterCount > 0)
		{
			AddCheck(Result.Checks, 
				TEXT("Disabled Emitters"), 
				TEXT("0"),
				FormatNumber(DisabledEmitterCount),
				false,
				EAssetCheckSeverity::Info,
				TEXT("Emitters"));
		}

		if (Config->bWarnOnMixedSimulation && Result.GPUEmitterCount > 0 && Result.CPUEmitterCount > 0)
		{
			AddCheck(Result.Checks, 
				TEXT("Simulation Consistency"), 
				TEXT("GPU or CPU only"),
				TEXT("Mixed"),
				false,
				EAssetCheckSeverity::Info,
				TEXT("Emitters"));
		}
	}

	// --- Particle Count Checks ---
	{
		AddCheck(Result.Checks, 
			TEXT("Total Particles (Est.)"), 
			FString::Printf(TEXT("≤ %s"), *FormatNumber(Config->TotalParticlesWarningThreshold)),
			FormatNumber(Result.EstimatedMaxParticles),
			Result.EstimatedMaxParticles <= Config->TotalParticlesWarningThreshold,
			Result.EstimatedMaxParticles > Config->TotalParticlesErrorThreshold ? EAssetCheckSeverity::Error : EAssetCheckSeverity::Warning,
			TEXT("Particles"));
	}

	// --- Renderer Checks ---
	{
		AddCheck(Result.Checks, 
			TEXT("Total Renderers"), 
			TEXT("-"),
			FormatNumber(TotalEnabledRenderers),
			true,
			EAssetCheckSeverity::Info,
			TEXT("Renderers"));

		AddCheck(Result.Checks, 
			TEXT("Sprite Renderers"), 
			TEXT("-"),
			FormatNumber(SpriteRendererCount),
			true,
			EAssetCheckSeverity::Info,
			TEXT("Renderers"));

		AddCheck(Result.Checks, 
			TEXT("Mesh Renderers"), 
			FString::Printf(TEXT("≤ %d"), Config->MaxMeshRenderers),
			FormatNumber(MeshRendererCount),
			MeshRendererCount <= Config->MaxMeshRenderers,
			EAssetCheckSeverity::Warning,
			TEXT("Renderers"));

		AddCheck(Result.Checks, 
			TEXT("Ribbon Renderers"), 
			TEXT("-"),
			FormatNumber(RibbonRendererCount),
			true,
			EAssetCheckSeverity::Info,
			TEXT("Renderers"));

		AddCheck(Result.Checks, 
			TEXT("Light Renderers"), 
			TEXT("-"),
			FormatNumber(LightRendererCount),
			true,
			EAssetCheckSeverity::Info,
			TEXT("Renderers"));
	}

	// --- Memory Checks ---
	{
		double TotalMemoryMB = TotalEstimatedMemory / (1024.0 * 1024.0);
		AddCheck(Result.Checks, 
			TEXT("Estimated Memory"), 
			FString::Printf(TEXT("≤ %d MB"), Config->MemoryWarningThresholdMB),
			FormatMemory(TotalEstimatedMemory),
			TotalMemoryMB <= Config->MemoryWarningThresholdMB,
			TotalMemoryMB > Config->MemoryErrorThresholdMB ? EAssetCheckSeverity::Error : EAssetCheckSeverity::Warning,
			TEXT("Memory"));

		AddCheck(Result.Checks, 
			TEXT("GPU Memory"), 
			TEXT("-"),
			FormatMemory(Result.EstimatedGPUMemoryBytes),
			true,
			EAssetCheckSeverity::Info,
			TEXT("Memory"));

		AddCheck(Result.Checks, 
			TEXT("CPU Memory"), 
			TEXT("-"),
			FormatMemory(Result.EstimatedCPUMemoryBytes),
			true,
			EAssetCheckSeverity::Info,
			TEXT("Memory"));
	}

	// --- Texture Memory (if any) ---
	if (Result.TotalTextureMemoryBytes > 0)
	{
		AddCheck(Result.Checks, 
			TEXT("Texture Memory"), 
			TEXT("-"),
			FormatMemory(Result.TotalTextureMemoryBytes),
			true,
			EAssetCheckSeverity::Info,
			TEXT("Assets"));

		AddCheck(Result.Checks, 
			TEXT("Texture Count"), 
			TEXT("-"),
			FormatNumber(Result.TextureReferences.Num()),
			true,
			EAssetCheckSeverity::Info,
			TEXT("Assets"));
	}

	// --- Mesh Stats (if any) ---
	if (Result.TotalMeshTriangles > 0)
	{
		AddCheck(Result.Checks, 
			TEXT("Total Mesh Triangles"), 
			TEXT("-"),
			FormatNumber(Result.TotalMeshTriangles),
			true,
			EAssetCheckSeverity::Info,
			TEXT("Assets"));

		AddCheck(Result.Checks, 
			TEXT("Total Mesh Vertices"), 
			TEXT("-"),
			FormatNumber(Result.TotalMeshVertices),
			true,
			EAssetCheckSeverity::Info,
			TEXT("Assets"));

		AddCheck(Result.Checks, 
			TEXT("Mesh Count"), 
			TEXT("-"),
			FormatNumber(Result.MeshReferences.Num()),
			true,
			EAssetCheckSeverity::Info,
			TEXT("Assets"));
	}

	return Result;
}

TArray<FAssetValidationResult> UNiagaraSystemValidator::ValidateSystems(const TArray<UNiagaraSystem*>& Systems)
{
	TArray<FAssetValidationResult> Results;
	Results.Reserve(Systems.Num());

	for (UNiagaraSystem* System : Systems)
	{
		Results.Add(ValidateSystem(System));
	}

	return Results;
}
