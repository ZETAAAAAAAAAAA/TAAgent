// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AssetValidationResult.generated.h"

/** Validation severity levels for asset checking */
UENUM(BlueprintType)
enum class EAssetCheckSeverity : uint8
{
	Info,
	Warning,
	Error
};

/** A single validation check result */
USTRUCT(BlueprintType)
struct FAssetValidationCheck
{
	GENERATED_BODY()

	/** Display name of the check */
	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	FString CheckName;

	/** The limit/threshold value as string */
	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	FString LimitValue;

	/** The current value as string */
	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	FString CurrentValue;

	/** Whether the check passed */
	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	bool bPassed = true;

	/** Severity if failed */
	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	EAssetCheckSeverity Severity = EAssetCheckSeverity::Info;

	/** Category for grouping */
	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	FString Category;

	FAssetValidationCheck() = default;

	FAssetValidationCheck(const FString& InName, const FString& InLimit, const FString& InCurrent, 
		bool InPassed, EAssetCheckSeverity InSeverity = EAssetCheckSeverity::Info, const FString& InCategory = TEXT("General"))
		: CheckName(InName)
		, LimitValue(InLimit)
		, CurrentValue(InCurrent)
		, bPassed(InPassed)
		, Severity(InSeverity)
		, Category(InCategory)
	{
	}
};

/** A single validation issue (legacy, kept for compatibility) */
USTRUCT(BlueprintType)
struct FAssetValidationIssue
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	FString IssueType;

	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	FString Description;

	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	EAssetCheckSeverity Severity = EAssetCheckSeverity::Info;

	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	FString Recommendation;

	FAssetValidationIssue() = default;

	FAssetValidationIssue(const FString& InType, const FString& InDesc, EAssetCheckSeverity InSeverity, const FString& InRec = FString())
		: IssueType(InType)
		, Description(InDesc)
		, Severity(InSeverity)
		, Recommendation(InRec)
	{
	}
};

/** Texture asset reference info */
USTRUCT(BlueprintType)
struct FTextureReferenceInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	FString TextureName;

	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	FString TexturePath;

	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	int32 Width = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	int32 Height = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	int64 EstimatedMemoryBytes = 0;
};

/** Mesh asset reference info */
USTRUCT(BlueprintType)
struct FMeshReferenceInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	FString MeshName;

	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	FString MeshPath;

	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	int32 TriangleCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	int32 VertexCount = 0;
};

/** Dynamic overdraw frame result */
USTRUCT(BlueprintType)
struct FOverdrawFrameResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	int32 FrameIndex = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	float Time = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	int32 MaxOverdraw = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	FString ScreenshotPath;
};

/** Dynamic overdraw analysis result */
USTRUCT(BlueprintType)
struct FDynamicOverdrawResult
{
	GENERATED_BODY()

	/** Whether analysis was successful */
	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	FString ErrorMessage;

	// Analysis results
	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	int32 MaxOverdraw = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	int32 MaxOverdrawFrame = -1;

	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	FString MaxOverdrawScreenshotPath;

	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	int32 MaxOverdrawThreshold = 10;

	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	bool bPassed = true;

	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	float AverageOverdraw = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	int32 TotalFrames = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	float DurationSeconds = 0.0f;

	// Camera info
	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	FVector CameraLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	FRotator CameraRotation = FRotator::ZeroRotator;

	// Frame data
	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	TArray<FOverdrawFrameResult> Frames;
};

/** Validation result for a Niagara System */
USTRUCT(BlueprintType)
struct FAssetValidationResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	FName SystemName;

	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	FSoftObjectPath SystemPath;

	/** Structured check results */
	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	TArray<FAssetValidationCheck> Checks;

	/** Legacy issues (kept for compatibility) */
	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	TArray<FAssetValidationIssue> Issues;

	// ============================================
	// Basic Stats
	// ============================================
	
	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	int32 EmitterCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	int32 TotalRendererCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	int32 EstimatedMaxParticles = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	int32 GPUEmitterCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	int32 CPUEmitterCount = 0;

	// ============================================
	// Memory Stats
	// ============================================
	
	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	int64 EstimatedGPUMemoryBytes = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	int64 EstimatedCPUMemoryBytes = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	int64 TotalTextureMemoryBytes = 0;

	// ============================================
	// Asset References
	// ============================================
	
	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	TArray<FTextureReferenceInfo> TextureReferences;

	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	TArray<FMeshReferenceInfo> MeshReferences;

	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	int32 TotalMeshTriangles = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	int32 TotalMeshVertices = 0;

	// ============================================
	// Dynamic Analysis Results
	// ============================================

	/** Dynamic overdraw analysis result */
	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	FDynamicOverdrawResult OverdrawResult;

	/** Whether dynamic analysis was performed */
	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	bool bHasDynamicAnalysis = false;

	bool HasIssues() const 
	{ 
		if (Issues.Num() > 0 || GetFailedCheckCount() > 0) return true;
		if (bHasDynamicAnalysis && !OverdrawResult.bPassed) return true;
		return false;
	}
	
	int32 GetErrorCount() const 
	{ 
		int32 Count = 0;
		for (const auto& Check : Checks)
		{
			if (!Check.bPassed && Check.Severity == EAssetCheckSeverity::Error) Count++;
		}
		for (const auto& Issue : Issues)
		{
			if (Issue.Severity == EAssetCheckSeverity::Error) Count++;
		}
		return Count;
	}

	int32 GetWarningCount() const 
	{ 
		int32 Count = 0;
		for (const auto& Check : Checks)
		{
			if (!Check.bPassed && Check.Severity == EAssetCheckSeverity::Warning) Count++;
		}
		for (const auto& Issue : Issues)
		{
			if (Issue.Severity == EAssetCheckSeverity::Warning) Count++;
		}
		return Count;
	}

	int32 GetFailedCheckCount() const
	{
		int32 Count = 0;
		for (const auto& Check : Checks)
		{
			if (!Check.bPassed) Count++;
		}
		return Count;
	}
};
