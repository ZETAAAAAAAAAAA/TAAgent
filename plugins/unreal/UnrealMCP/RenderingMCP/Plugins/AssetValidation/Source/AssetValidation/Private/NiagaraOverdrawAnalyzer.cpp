// Copyright Epic Games, Inc. All Rights Reserved.

#include "NiagaraOverdrawAnalyzer.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraActor.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Editor.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"

// Viewport includes
#include "Slate/SceneViewport.h"
#include "LevelEditor.h"
#include "ILevelEditor.h"
#include "SLevelViewport.h"
#include "EditorViewportClient.h"
#include "RenderingThread.h"

// Image includes
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Misc/FileHelper.h"
#include "Modules/ModuleManager.h"

// Console commands
#include "Framework/Application/SlateApplication.h"
#include "HAL/IConsoleManager.h"

FNiagaraOverdrawAnalyzer::FAnalysisResult FNiagaraOverdrawAnalyzer::AnalyzeOverdraw(UNiagaraSystem* System, const FAnalysisConfig& Config)
{
	FAnalysisResult Result;
	Result.bSuccess = false;
	Result.MaxOverdrawThreshold = Config.MaxOverdrawThreshold;

	if (!System)
	{
		Result.ErrorMessage = TEXT("Null Niagara System provided");
		return Result;
	}

	Result.AssetPath = System->GetPathName();
	Result.AssetName = System->GetName();

	// Create output directory
	FString OutputDir = Config.OutputDir;
	if (OutputDir.IsEmpty())
	{
		OutputDir = FPaths::ProjectSavedDir() / TEXT("OverdrawAnalysis") / System->GetName();
	}
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	PlatformFile.CreateDirectoryTree(*OutputDir);

	// Get editor world
	UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	if (!World)
	{
		Result.ErrorMessage = TEXT("No editor world available");
		return Result;
	}

	// Spawn Niagara actor
	AActor* SpawnedActor = SpawnNiagaraActor(System, World);
	if (!SpawnedActor)
	{
		Result.ErrorMessage = TEXT("Failed to spawn Niagara actor");
		return Result;
	}

	// Find the Niagara component
	UNiagaraComponent* NiagaraComp = nullptr;
	TArray<UNiagaraComponent*> Components;
	SpawnedActor->GetComponents<UNiagaraComponent>(Components);
	if (Components.Num() > 0)
	{
		NiagaraComp = Components[0];
	}

	if (!NiagaraComp)
	{
		SpawnedActor->Destroy();
		Result.ErrorMessage = TEXT("Failed to get Niagara component");
		return Result;
	}

	// Activate the system
	NiagaraComp->Activate();

	// Wait for system to initialize
	FlushRenderingCommands();
	FPlatformProcess::Sleep(0.5f);

	// Get bounding box
	FBox Bounds = GetNiagaraBounds(NiagaraComp);
	if (!Bounds.IsValid)
	{
		Bounds = FBox(FVector(-100, -100, -100), FVector(100, 100, 100));
	}

	Result.BoundsMin = Bounds.Min;
	Result.BoundsMax = Bounds.Max;

	// Calculate camera position
	FVector CameraLocation;
	FRotator CameraRotation;
	CalculateCameraFromBounds(Bounds, CameraLocation, CameraRotation, Config.CameraFOV);

	Result.CameraLocation = CameraLocation;
	Result.CameraRotation = CameraRotation;

	// Get viewport client using GEditor->GetLevelViewportClients()
	FViewport* TargetViewport = nullptr;
	FEditorViewportClient* ViewportClient = nullptr;

	if (GEditor)
	{
		// Get the first available level viewport client
		const TArray<FLevelEditorViewportClient*>& ViewportClients = GEditor->GetLevelViewportClients();
		if (ViewportClients.Num() > 0)
		{
			ViewportClient = ViewportClients[0];
		}

		// Get the active viewport from LevelEditor module
		FLevelEditorModule* LevelEditorModule = FModuleManager::GetModulePtr<FLevelEditorModule>(TEXT("LevelEditor"));
		if (LevelEditorModule)
		{
			TSharedPtr<SLevelViewport> ActiveLevelViewport = LevelEditorModule->GetFirstActiveLevelViewport();
			if (ActiveLevelViewport.IsValid())
			{
				TSharedPtr<FSceneViewport> SceneViewport = ActiveLevelViewport->GetSceneViewport();
				if (SceneViewport.IsValid())
				{
					TargetViewport = SceneViewport.Get();
				}
				if (!ViewportClient)
				{
					ViewportClient = &ActiveLevelViewport->GetLevelViewportClient();
				}
			}
		}
	}

	if (!ViewportClient)
	{
		SpawnedActor->Destroy();
		Result.ErrorMessage = TEXT("No active viewport found");
		return Result;
	}

	// Store original camera state
	FVector OriginalCameraLocation = ViewportClient->GetViewLocation();
	FRotator OriginalCameraRotation = ViewportClient->GetViewRotation();
	float OriginalFOV = ViewportClient->FOVAngle;

	// Set camera position
	ViewportClient->SetViewLocation(CameraLocation);
	ViewportClient->SetViewRotation(CameraRotation);
	// Set FOV by directly setting the properties (no SetFOVAngle method)
	ViewportClient->FOVAngle = Config.CameraFOV;
	ViewportClient->ViewFOV = Config.CameraFOV;
	ViewportClient->Invalidate();

	// Switch to Quad Overdraw view mode
	if (!SetQuadOverdrawViewMode(ViewportClient, true))
	{
		SpawnedActor->Destroy();
		Result.ErrorMessage = TEXT("Failed to set Quad Overdraw view mode. Make sure the project is built with DebugViewModes enabled.");
		return Result;
	}

	// Get viewport for screenshot
	if (!TargetViewport && ViewportClient)
	{
		TargetViewport = ViewportClient->Viewport;
	}

	// Capture frames
	int32 TotalFrames = FMath::CeilToInt(Config.DurationSeconds / Config.CaptureInterval);
	Result.TotalFrames = TotalFrames;
	Result.DurationSeconds = Config.DurationSeconds;

	int32 MaxOverdraw = 0;
	int32 MaxOverdrawFrame = -1;
	FString MaxOverdrawScreenshotPath;
	float TotalOverdraw = 0.0f;

	for (int32 FrameIndex = 0; FrameIndex < TotalFrames; FrameIndex++)
	{
		// Force viewport redraw
		FlushRenderingCommands();
		if (TargetViewport)
		{
			TargetViewport->Invalidate();
			TargetViewport->Draw();
		}
		ViewportClient->Invalidate();
		FlushRenderingCommands();

		// Capture and analyze
		FString ScreenshotPath = FPaths::Combine(*OutputDir, FString::Printf(TEXT("frame_%04d.png"), FrameIndex));
		FViewport* CaptureViewport = TargetViewport ? static_cast<FViewport*>(TargetViewport) : ViewportClient->Viewport;
		int32 FrameMaxOverdraw = CaptureAndAnalyzeOverdraw(CaptureViewport, ScreenshotPath, Config.bSaveScreenshots);

		// Record frame result
		FFrameResult FrameResult;
		FrameResult.FrameIndex = FrameIndex;
		FrameResult.Time = FrameIndex * Config.CaptureInterval;
		FrameResult.MaxOverdraw = FrameMaxOverdraw;
		FrameResult.ScreenshotPath = ScreenshotPath;
		Result.Frames.Add(FrameResult);

		TotalOverdraw += FrameMaxOverdraw;

		if (FrameMaxOverdraw > MaxOverdraw)
		{
			MaxOverdraw = FrameMaxOverdraw;
			MaxOverdrawFrame = FrameIndex;
			MaxOverdrawScreenshotPath = ScreenshotPath;
		}

		// Wait for next capture
		FPlatformProcess::Sleep(Config.CaptureInterval);
	}

	// Calculate statistics
	Result.MaxOverdraw = MaxOverdraw;
	Result.MaxOverdrawFrame = MaxOverdrawFrame;
	Result.MaxOverdrawScreenshotPath = MaxOverdrawScreenshotPath;
	Result.AverageOverdraw = Result.Frames.Num() > 0 ? TotalOverdraw / Result.Frames.Num() : 0.0f;
	Result.bPassed = MaxOverdraw <= Config.MaxOverdrawThreshold;

	// Restore view mode
	SetQuadOverdrawViewMode(ViewportClient, false);

	// Restore camera
	ViewportClient->SetViewLocation(OriginalCameraLocation);
	ViewportClient->SetViewRotation(OriginalCameraRotation);
	ViewportClient->FOVAngle = OriginalFOV;
	ViewportClient->ViewFOV = OriginalFOV;
	ViewportClient->Invalidate();

	// Clean up
	SpawnedActor->Destroy();

	Result.bSuccess = true;
	return Result;
}

TArray<FNiagaraOverdrawAnalyzer::FAnalysisResult> FNiagaraOverdrawAnalyzer::AnalyzeSystems(const TArray<UNiagaraSystem*>& Systems, const FAnalysisConfig& Config)
{
	TArray<FAnalysisResult> Results;
	Results.Reserve(Systems.Num());

	for (UNiagaraSystem* System : Systems)
	{
		Results.Add(AnalyzeOverdraw(System, Config));
	}

	return Results;
}

AActor* FNiagaraOverdrawAnalyzer::SpawnNiagaraActor(UNiagaraSystem* System, UWorld* World)
{
	if (!System || !World)
	{
		return nullptr;
	}

	// Spawn Niagara actor at origin
	FTransform SpawnTransform(FVector::ZeroVector);
	ANiagaraActor* NiagaraActor = World->SpawnActor<ANiagaraActor>(ANiagaraActor::StaticClass(), SpawnTransform);

	if (NiagaraActor)
	{
		UNiagaraComponent* Component = NiagaraActor->GetNiagaraComponent();
		if (Component)
		{
			Component->SetAsset(System);
		}
	}

	return NiagaraActor;
}

FBox FNiagaraOverdrawAnalyzer::GetNiagaraBounds(UNiagaraComponent* Component)
{
	if (!Component)
	{
		return FBox(EForceInit::ForceInit);
	}

	// Try to get bounds from component
	FBoxSphereBounds ComponentBounds = Component->CalcBounds(FTransform::Identity);
	
	if (ComponentBounds.BoxExtent.Size() > 0)
	{
		return ComponentBounds.GetBox();
	}

	return FBox(EForceInit::ForceInit);
}

void FNiagaraOverdrawAnalyzer::CalculateCameraFromBounds(const FBox& Bounds, FVector& OutLocation, FRotator& OutRotation, float FOV)
{
	// Calculate center and extent
	FVector Center = Bounds.GetCenter();
	FVector Extent = Bounds.GetExtent();

	// Calculate required distance to fit bounds in view
	float HalfFOV = FMath::DegreesToRadians(FOV * 0.5f);
	float MaxExtent = FMath::Max(Extent.X, FMath::Max(Extent.Y, Extent.Z));
	float Distance = MaxExtent / FMath::Tan(HalfFOV);

	// Add some padding
	Distance *= 1.5f;

	// Position camera in front of the bounds
	OutLocation = Center + FVector(Distance, 0.0f, 0.0f);

	// Look at center
	OutRotation = FRotationMatrix::MakeFromX(Center - OutLocation).Rotator();
}

bool FNiagaraOverdrawAnalyzer::SetQuadOverdrawViewMode(FEditorViewportClient* ViewportClient, bool bEnable)
{
	if (!ViewportClient)
	{
		return false;
	}

	// Set the view mode flags directly on the viewport client
	if (bEnable)
	{
		// Enable Quad Overdraw visualization
		// This requires the project to have "DebugViewModes" enabled in Build.cs
		ViewportClient->EngineShowFlags.SetQuadOverdraw(true);
		ViewportClient->EngineShowFlags.SetShaderComplexity(false); // Disable other debug modes
	}
	else
	{
		// Restore to lit mode
		ViewportClient->EngineShowFlags.SetQuadOverdraw(false);
	}
	
	ViewportClient->Invalidate();
	return true;
}

int32 FNiagaraOverdrawAnalyzer::CaptureAndAnalyzeOverdraw(FViewport* Viewport, const FString& OutputPath, bool bSaveScreenshot)
{
	if (!Viewport)
	{
		return 0;
	}

	// Read pixels
	TArray<FColor> PixelData;
	if (!Viewport->ReadPixels(PixelData))
	{
		return 0;
	}

	// Use GetSizeXY() instead of GetSize()
	FIntPoint ViewportSize = Viewport->GetSizeXY();
	int32 MaxOverdraw = AnalyzeOverdrawFromPixels(PixelData, ViewportSize.X, ViewportSize.Y);

	// Save screenshot if requested
	if (bSaveScreenshot)
	{
		SavePixelsAsPNG(PixelData, ViewportSize.X, ViewportSize.Y, OutputPath);
	}

	return MaxOverdraw;
}

int32 FNiagaraOverdrawAnalyzer::AnalyzeOverdrawFromPixels(const TArray<FColor>& Pixels, int32 Width, int32 Height)
{
	if (Pixels.Num() == 0 || Width <= 0 || Height <= 0)
	{
		return 0;
	}

	int32 MaxOverdraw = 0;

	// In Quad Overdraw view mode, the color represents overdraw count
	// The visualization uses a color ramp:
	// - Green (0-2): Low overdraw
	// - Yellow (3-5): Medium overdraw
	// - Red (6-10): High overdraw
	// - Purple (>10): Very high overdraw
	//
	// The overdraw count is encoded in the red channel primarily,
	// with additional info in green/blue for the ramp visualization.
	// We estimate the overdraw by analyzing the color intensity.

	for (const FColor& Pixel : Pixels)
	{
		// Skip near-black pixels (background)
		if (Pixel.R < 10 && Pixel.G < 10 && Pixel.B < 10)
		{
			continue;
		}

		// Estimate overdraw from color
		// In quad overdraw mode, brighter = more overdraw
		// The color scale roughly maps:
		// (0,255,0) -> overdraw 0-2
		// (255,255,0) -> overdraw 3-5
		// (255,0,0) -> overdraw 6-10
		// (255,0,255) -> overdraw >10

		int32 EstimatedOverdraw = 0;

		// Purple = very high overdraw
		if (Pixel.R > 200 && Pixel.B > 100)
		{
			EstimatedOverdraw = 10 + (Pixel.B / 25);
		}
		// Red = high overdraw
		else if (Pixel.R > 200)
		{
			EstimatedOverdraw = 6 + ((255 - Pixel.G) / 50);
		}
		// Yellow = medium overdraw
		else if (Pixel.R > 100 && Pixel.G > 100)
		{
			EstimatedOverdraw = 3 + (Pixel.R / 85);
		}
		// Green = low overdraw
		else if (Pixel.G > 50)
		{
			EstimatedOverdraw = (Pixel.G / 127);
		}

		MaxOverdraw = FMath::Max(MaxOverdraw, EstimatedOverdraw);
	}

	return MaxOverdraw;
}

bool FNiagaraOverdrawAnalyzer::SavePixelsAsPNG(const TArray<FColor>& Pixels, int32 Width, int32 Height, const FString& Path)
{
	if (Pixels.Num() == 0 || Width <= 0 || Height <= 0)
	{
		return false;
	}

	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

	if (!ImageWrapper.IsValid())
	{
		return false;
	}

	ImageWrapper->SetRaw(Pixels.GetData(), Pixels.Num() * sizeof(FColor), Width, Height, ERGBFormat::BGRA, 8);
	TArray64<uint8> CompressedData = ImageWrapper->GetCompressed();

	return FFileHelper::SaveArrayToFile(CompressedData, *Path);
}
