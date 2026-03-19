// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AssetValidationResult.h"

class UNiagaraSystem;
class UNiagaraComponent;
class AActor;
class FEditorViewportClient;

/**
 * Niagara Dynamic Overdraw Analyzer
 * 
 * Performs runtime overdraw analysis by:
 * 1. Spawning Niagara system in a test world
 * 2. Positioning camera based on bounding box
 * 3. Switching to Quad Overdraw view mode
 * 4. Capturing frames over time
 * 5. Analyzing pixel overdraw values
 */
class ASSETVALIDATION_API FNiagaraOverdrawAnalyzer
{
public:
	/** Configuration for overdraw analysis */
	struct FAnalysisConfig
	{
		/** Duration to capture in seconds */
		float DurationSeconds = 5.0f;
		
		/** Time between frame captures */
		float CaptureInterval = 0.1f;
		
		/** Maximum allowed overdraw (threshold for pass/fail) */
		int32 MaxOverdrawThreshold = 10;
		
		/** Output directory for screenshots */
		FString OutputDir;
		
		/** Camera FOV */
		float CameraFOV = 60.0f;
		
		/** Whether to save screenshots */
		bool bSaveScreenshots = true;
	};

	/** Result of a single frame capture */
	struct FFrameResult
	{
		int32 FrameIndex;
		float Time;
		int32 MaxOverdraw;
		FString ScreenshotPath;
	};

	/** Complete analysis result */
	struct FAnalysisResult
	{
		bool bSuccess = false;
		FString ErrorMessage;
		
		// Asset info
		FString AssetPath;
		FString AssetName;
		
		// Analysis results
		int32 MaxOverdraw = 0;
		int32 MaxOverdrawFrame = -1;
		FString MaxOverdrawScreenshotPath;
		
		// Threshold
		int32 MaxOverdrawThreshold = 10;
		bool bPassed = true;
		
		// Bounding box
		FVector BoundsMin;
		FVector BoundsMax;
		
		// Camera info
		FVector CameraLocation;
		FRotator CameraRotation;
		
		// Frame data
		TArray<FFrameResult> Frames;
		
		// Statistics
		float AverageOverdraw = 0.0f;
		int32 TotalFrames = 0;
		float DurationSeconds = 0.0f;
	};

	/**
	 * Analyze overdraw for a Niagara System asset
	 * 
	 * @param System Niagara System to analyze
	 * @param Config Analysis configuration
	 * @return Analysis result with max overdraw and screenshots
	 */
	static FAnalysisResult AnalyzeOverdraw(UNiagaraSystem* System, const FAnalysisConfig& Config);

	/**
	 * Analyze overdraw for multiple Niagara Systems
	 */
	static TArray<FAnalysisResult> AnalyzeSystems(const TArray<UNiagaraSystem*>& Systems, const FAnalysisConfig& Config);

private:
	/** Spawn Niagara component in world for testing */
	static AActor* SpawnNiagaraActor(UNiagaraSystem* System, UWorld* World);

	/** Get bounding box from Niagara component */
	static FBox GetNiagaraBounds(UNiagaraComponent* Component);

	/** Calculate camera position from bounding box */
	static void CalculateCameraFromBounds(const FBox& Bounds, FVector& OutLocation, FRotator& OutRotation, float FOV);

	/** Set viewport to Quad Overdraw view mode */
	static bool SetQuadOverdrawViewMode(class FEditorViewportClient* ViewportClient, bool bEnable);

	/** Capture viewport and analyze overdraw */
	static int32 CaptureAndAnalyzeOverdraw(FViewport* Viewport, const FString& OutputPath, bool bSaveScreenshot);

	/** Analyze pixel data to find max overdraw value */
	static int32 AnalyzeOverdrawFromPixels(const TArray<FColor>& Pixels, int32 Width, int32 Height);

	/** Save pixel data as PNG */
	static bool SavePixelsAsPNG(const TArray<FColor>& Pixels, int32 Width, int32 Height, const FString& Path);
};
