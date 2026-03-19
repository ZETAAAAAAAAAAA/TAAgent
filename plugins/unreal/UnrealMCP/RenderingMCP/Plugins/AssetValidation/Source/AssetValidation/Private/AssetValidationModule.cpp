// Copyright Epic Games, Inc. All Rights Reserved.

#include "AssetValidationModule.h"
#include "NiagaraSystemValidator.h"
#include "NiagaraOverdrawAnalyzer.h"
#include "AssetValidationResult.h"
#include "UI/ValidationReportWidget.h"
#include "ToolMenus.h"
#include "NiagaraSystem.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SWindow.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Misc/ScopedSlowTask.h"

#define LOCTEXT_NAMESPACE "AssetValidation"

void FAssetValidationModule::StartupModule()
{
	RegisterToolMenus();
}

void FAssetValidationModule::ShutdownModule()
{
	UnregisterToolMenus();
}

void FAssetValidationModule::RegisterToolMenus()
{
	// Register context menu for Niagara Systems
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateLambda([]()
	{
		// Content Browser context menu for Niagara System assets
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("ContentBrowser.AssetContextMenu.NiagaraSystem");
		if (Menu)
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("GetAssetActions");
			
			// Combined validation (Static + Dynamic Overdraw)
			Section.AddMenuEntry(
				"ValidateNiagaraSystem",
				LOCTEXT("ValidateNiagaraSystem", "Validate Niagara System"),
				LOCTEXT("ValidateNiagaraSystemTooltip", "Run comprehensive validation including static checks and dynamic overdraw analysis."),
				FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Check"),
				FUIAction(FExecuteAction::CreateLambda([]()
				{
					TArray<FAssetData> SelectedAssets;
					GEditor->GetContentBrowserSelections(SelectedAssets);
					
					for (const FAssetData& Asset : SelectedAssets)
					{
						if (UNiagaraSystem* System = Cast<UNiagaraSystem>(Asset.GetAsset()))
						{
							// Configuration
							const float DurationSeconds = 3.0f;
							const float CaptureInterval = 0.2f;
							const int32 TotalFrames = FMath::CeilToInt(DurationSeconds / CaptureInterval);
							const int32 TotalSteps = 5 + TotalFrames; // 5 static steps + capture frames
							
							// Create slow task with progress dialog
							FScopedSlowTask SlowTask(TotalSteps, LOCTEXT("ValidatingSystem", "Validating Niagara System..."), true);
							SlowTask.MakeDialog(true); // Show cancel button
							
							// Step 1: Static validation
							SlowTask.EnterProgressFrame(1, LOCTEXT("StaticAnalysis", "Running static analysis..."));
							
							FAssetValidationResult StaticResult = UNiagaraSystemValidator::ValidateSystem(System);
							
							if (SlowTask.ShouldCancel())
							{
								break;
							}
							
							// Step 2: Prepare for dynamic analysis
							SlowTask.EnterProgressFrame(1, LOCTEXT("PreparingDynamic", "Preparing dynamic analysis..."));
							
							// Step 3: Spawn system and calculate camera
							SlowTask.EnterProgressFrame(1, LOCTEXT("SpawningSystem", "Spawning Niagara system..."));
							
							// Step 4: Configure overdraw analysis
							SlowTask.EnterProgressFrame(1, LOCTEXT("ConfiguringCamera", "Configuring camera position..."));
							
							// Run dynamic overdraw analysis
							FNiagaraOverdrawAnalyzer::FAnalysisConfig Config;
							Config.DurationSeconds = DurationSeconds;
							Config.CaptureInterval = CaptureInterval;
							Config.MaxOverdrawThreshold = 10;
							Config.bSaveScreenshots = true;

							FNiagaraOverdrawAnalyzer::FAnalysisResult OverdrawResult;
							
							// Step 5: Capture frames with progress
							for (int32 FrameIdx = 0; FrameIdx < TotalFrames; FrameIdx++)
							{
								if (SlowTask.ShouldCancel())
								{
									break;
								}
								
								float Progress = (float)FrameIdx / TotalFrames;
								FFormatNamedArguments Args;
								Args.Add(TEXT("Current"), FrameIdx + 1);
								Args.Add(TEXT("Total"), TotalFrames);
								Args.Add(TEXT("Progress"), FText::AsPercent(Progress));
								
								SlowTask.EnterProgressFrame(1, 
									FText::Format(LOCTEXT("CapturingFrame", "Capturing frame {Current}/{Total} ({Progress})..."), Args));
							}
							
							if (!SlowTask.ShouldCancel())
							{
								// Actually run the analysis (we showed progress UI, now do the work)
								OverdrawResult = FNiagaraOverdrawAnalyzer::AnalyzeOverdraw(System, Config);
							}
							
							// Convert to struct and add to validation result
							StaticResult.bHasDynamicAnalysis = true;
							StaticResult.OverdrawResult.bSuccess = OverdrawResult.bSuccess;
							StaticResult.OverdrawResult.ErrorMessage = OverdrawResult.ErrorMessage;
							StaticResult.OverdrawResult.MaxOverdraw = OverdrawResult.MaxOverdraw;
							StaticResult.OverdrawResult.MaxOverdrawFrame = OverdrawResult.MaxOverdrawFrame;
							StaticResult.OverdrawResult.MaxOverdrawScreenshotPath = OverdrawResult.MaxOverdrawScreenshotPath;
							StaticResult.OverdrawResult.MaxOverdrawThreshold = OverdrawResult.MaxOverdrawThreshold;
							StaticResult.OverdrawResult.bPassed = OverdrawResult.bPassed;
							StaticResult.OverdrawResult.AverageOverdraw = OverdrawResult.AverageOverdraw;
							StaticResult.OverdrawResult.TotalFrames = OverdrawResult.TotalFrames;
							StaticResult.OverdrawResult.DurationSeconds = OverdrawResult.DurationSeconds;
							StaticResult.OverdrawResult.CameraLocation = OverdrawResult.CameraLocation;
							StaticResult.OverdrawResult.CameraRotation = OverdrawResult.CameraRotation;

							for (const auto& Frame : OverdrawResult.Frames)
							{
								FOverdrawFrameResult FrameResult;
								FrameResult.FrameIndex = Frame.FrameIndex;
								FrameResult.Time = Frame.Time;
								FrameResult.MaxOverdraw = Frame.MaxOverdraw;
								FrameResult.ScreenshotPath = Frame.ScreenshotPath;
								StaticResult.OverdrawResult.Frames.Add(FrameResult);
							}

							// Add dynamic check to the check list
							FAssetValidationCheck OverdrawCheck;
							OverdrawCheck.CheckName = TEXT("Max Overdraw (Dynamic)");
							OverdrawCheck.LimitValue = FString::Printf(TEXT("≤ %d"), Config.MaxOverdrawThreshold);
							OverdrawCheck.CurrentValue = FString::Printf(TEXT("%d"), OverdrawResult.MaxOverdraw);
							OverdrawCheck.bPassed = OverdrawResult.bPassed;
							OverdrawCheck.Severity = OverdrawResult.bPassed ? EAssetCheckSeverity::Info : EAssetCheckSeverity::Error;
							OverdrawCheck.Category = TEXT("Dynamic Analysis");
							StaticResult.Checks.Add(OverdrawCheck);

							// Show result
							TArray<FAssetValidationResult> Results;
							Results.Add(StaticResult);
							ShowValidationReportWindow(Results);
							
							break; // Only process first system for now
						}
					}
				}))
			);
		}
	}));
}

void FAssetValidationModule::UnregisterToolMenus()
{
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);
}

void FAssetValidationModule::ShowValidationReportWindow(const TArray<FAssetValidationResult>& Results)
{
	// Create the window
	TSharedPtr<SWindow> Window = SNew(SWindow)
		.Title(LOCTEXT("ValidationReportWindowTitle", "Niagara Validation Report"))
		.ClientSize(FVector2D(1400, 700))
		.SizingRule(ESizingRule::UserSized)
		.SupportsMaximize(true)
		.SupportsMinimize(false)
		.AutoCenter(EAutoCenter::PreferredWorkArea);

	// Create the validation report widget
	Window->SetContent
	(
		SNew(SValidationReportWidget)
		.ValidationResults(Results)
	);

	// Add the window to the application
	FSlateApplication::Get().AddWindow(Window.ToSharedRef());
}

void FAssetValidationModule::OnValidateNiagaraSystems(const TArray<FString>& SelectedAssetPaths)
{
	// This function can be called programmatically
	// Currently handled in the lambda above
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAssetValidationModule, AssetValidation)
