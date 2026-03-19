// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/ValidationReportWidget.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Layout/SSpacer.h"
#include "Styling/AppStyle.h"
#include "Framework/Application/SlateApplication.h"
#include "HAL/PlatformApplicationMisc.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Modules/ModuleManager.h"
#include "Misc/FileHelper.h"
#include "Engine/Texture2D.h"
#include "RenderingThread.h"
#include "UObject/UObjectGlobals.h"
#include "Widgets/Colors/SColorBlock.h"

#define LOCTEXT_NAMESPACE "AssetValidation"

void SValidationReportWidget::Construct(const FArguments& InArgs)
{
	Results = InArgs._ValidationResults;
	
	// Build check list from results
	BuildCheckList(Results);

	// Calculate totals
	TotalSystems = Results.Num();
	TotalPassed = 0;
	TotalFailed = 0;

	for (const FAssetValidationResult& Result : Results)
	{
		for (const FAssetValidationCheck& Check : Result.Checks)
		{
			if (Check.bPassed)
				TotalPassed++;
			else
				TotalFailed++;
		}
	}

	// Get overdraw screenshot path from first result
	FString ScreenshotPath;
	bool bHasDynamicAnalysis = false;
	int32 MaxOverdraw = 0;
	bool bOverdrawPassed = true;
	
	if (Results.Num() > 0 && Results[0].bHasDynamicAnalysis)
	{
		ScreenshotPath = Results[0].OverdrawResult.MaxOverdrawScreenshotPath;
		bHasDynamicAnalysis = Results[0].OverdrawResult.bSuccess;
		MaxOverdraw = Results[0].OverdrawResult.MaxOverdraw;
		bOverdrawPassed = Results[0].OverdrawResult.bPassed;
	}

	// Load screenshot if available
	TSharedPtr<FSlateDynamicImageBrush> LoadedBrush;
	if (!ScreenshotPath.IsEmpty() && FPaths::FileExists(*ScreenshotPath))
	{
		LoadedBrush = LoadScreenshotBrush(ScreenshotPath);
		ScreenshotBrush = LoadedBrush;
	}

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.Padding(8.0f)
		[
			SNew(SVerticalBox)
			// Title Bar
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 8)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("ValidationReportTitle", "Niagara System Validation Report"))
					.Font(FAppStyle::GetFontStyle("HeadingLargeFont"))
					.ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f))
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("Export", "Export"))
					.ToolTipText(LOCTEXT("ExportTooltip", "Copy report to clipboard"))
					.OnClicked(this, &SValidationReportWidget::OnExportClicked)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(4.0f, 0, 0, 0)
				[
					SNew(SButton)
					.Text(LOCTEXT("Close", "Close"))
					.OnClicked(this, &SValidationReportWidget::OnCloseClicked)
				]
			]
			// Summary Bar
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 8)
			[
				SNew(SBorder)
				.BorderBackgroundColor(FLinearColor(0.05f, 0.05f, 0.05f, 1.0f))
				.Padding(8.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(0, 0, 16, 0)
					[
						SNew(STextBlock)
						.Text(FText::Format(LOCTEXT("SystemsFormat", "Systems: {0}"), FText::AsNumber(TotalSystems)))
						.ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f))
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(0, 0, 16, 0)
					[
						SNew(STextBlock)
						.Text(FText::Format(LOCTEXT("PassedFormat", "Passed: {0}"), FText::AsNumber(TotalPassed)))
						.ColorAndOpacity(FLinearColor(0.4f, 0.9f, 0.4f, 1.0f))
						.Font(FAppStyle::GetFontStyle("BoldFont"))
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(0, 0, 16, 0)
					[
						SNew(STextBlock)
						.Text(FText::Format(LOCTEXT("FailedFormat", "Failed: {0}"), FText::AsNumber(TotalFailed)))
						.ColorAndOpacity(TotalFailed > 0 ? FLinearColor(1.0f, 0.3f, 0.3f, 1.0f) : FLinearColor(0.7f, 0.7f, 0.7f, 1.0f))
						.Font(FAppStyle::GetFontStyle("BoldFont"))
					]
				]
			]
			// Main Content: Table + Screenshot
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SNew(SHorizontalBox)
				// Left: Check Table
				+ SHorizontalBox::Slot()
				.FillWidth(0.65f)
				.Padding(0, 0, 8, 0)
				[
					SNew(SVerticalBox)
					// Header Row
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						CreateHeaderRow()
					]
					// Check List
					+ SVerticalBox::Slot()
					.FillHeight(1.0f)
					.Padding(0, 4, 0, 0)
					[
						SNew(SScrollBox)
						+ SScrollBox::Slot()
						[
							SAssignNew(CheckListView, SListView<TSharedPtr<FCheckItem>>)
							.ListItemsSource(&CheckItems)
							.OnGenerateRow(this, &SValidationReportWidget::OnGenerateRow)
							.SelectionMode(ESelectionMode::Single)
							.HeaderRow
							(
								SNew(SHeaderRow)
								.Visibility(EVisibility::Collapsed)
							)
						]
					]
				]
				// Right: Screenshot Panel
				+ SHorizontalBox::Slot()
				.FillWidth(0.35f)
				[
					CreateScreenshotPanel(Results.Num() > 0 ? Results[0] : FAssetValidationResult())
				]
			]
		]
	];
}

TSharedRef<SWidget> SValidationReportWidget::CreateScreenshotPanel(const FAssetValidationResult& Result)
{
	return SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
		.Padding(8.0f)
		[
			SNew(SVerticalBox)
			// Title
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 8)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("OverdrawScreenshotTitle", "Max Overdraw Frame"))
				.Font(FAppStyle::GetFontStyle("BoldFont"))
				.ColorAndOpacity(FLinearColor(0.9f, 0.8f, 0.4f, 1.0f))
			]
			// Info
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 8)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(STextBlock)
						.Text(LOCTEXT("MaxOverdrawLabel", "Max Overdraw: "))
						.ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f))
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(STextBlock)
						.Text(FText::AsNumber(Result.bHasDynamicAnalysis ? Result.OverdrawResult.MaxOverdraw : 0))
						.ColorAndOpacity(Result.bHasDynamicAnalysis && !Result.OverdrawResult.bPassed 
							? FLinearColor(1.0f, 0.3f, 0.3f, 1.0f) 
							: FLinearColor(0.4f, 0.9f, 0.4f, 1.0f))
						.Font(FAppStyle::GetFontStyle("BoldFont"))
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 4, 0, 0)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(STextBlock)
						.Text(LOCTEXT("ThresholdLabel", "Threshold: "))
						.ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f))
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(STextBlock)
						.Text(FText::AsNumber(Result.bHasDynamicAnalysis ? Result.OverdrawResult.MaxOverdrawThreshold : 10))
						.ColorAndOpacity(FLinearColor(0.5f, 0.8f, 0.5f, 1.0f))
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 4, 0, 0)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(STextBlock)
						.Text(LOCTEXT("StatusLabels", "Status: "))
						.ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f))
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(STextBlock)
						.Text(Result.bHasDynamicAnalysis && Result.OverdrawResult.bPassed 
							? LOCTEXT("Passed", "PASSED") 
							: LOCTEXT("Failed", "FAILED"))
						.ColorAndOpacity(Result.bHasDynamicAnalysis && Result.OverdrawResult.bPassed 
							? FLinearColor(0.4f, 0.9f, 0.4f, 1.0f) 
							: FLinearColor(1.0f, 0.3f, 0.3f, 1.0f))
						.Font(FAppStyle::GetFontStyle("BoldFont"))
					]
				]
			]
			// Screenshot
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			[
				SNew(SBox)
				.WidthOverride(400)
				.HeightOverride(300)
				[
					SNew(SBorder)
					.BorderBackgroundColor(FLinearColor(0.05f, 0.05f, 0.05f, 1.0f))
					.Padding(2.0f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Center)
						[
							ScreenshotBrush.IsValid()
							? static_cast<TSharedRef<SWidget>>(
								SNew(SImage)
								.Image(ScreenshotBrush.Get())
								)
							: static_cast<TSharedRef<SWidget>>(
								SNew(SVerticalBox)
								+ SVerticalBox::Slot()
								.VAlign(VAlign_Center)
								.HAlign(HAlign_Center)
								[
									SNew(STextBlock)
									.Text(Result.bHasDynamicAnalysis 
										? LOCTEXT("NoScreenshot", "No Screenshot Available") 
										: LOCTEXT("NoDynamicAnalysis", "Dynamic Analysis\nNot Performed"))
									.ColorAndOpacity(FLinearColor(0.5f, 0.5f, 0.5f, 1.0f))
									.Justification(ETextJustify::Center)
								]
							)
						]
					]
				]
			]
			// Legend
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 8, 0, 0)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("LegendTitle", "Color Legend:"))
					.Font(FAppStyle::GetFontStyle("BoldFont"))
					.ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 4, 0, 0)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SColorBlock)
						.Color(FLinearColor(0.0f, 1.0f, 0.0f))
						.Size(FVector2D(12, 12))
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(4, 0, 8, 0)
					[
						SNew(STextBlock).Text(LOCTEXT("LowOverdraw", "0-2")).ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f))
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SColorBlock)
						.Color(FLinearColor(1.0f, 1.0f, 0.0f))
						.Size(FVector2D(12, 12))
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(4, 0, 8, 0)
					[
						SNew(STextBlock).Text(LOCTEXT("MedOverdraw", "3-5")).ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f))
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SColorBlock)
						.Color(FLinearColor(1.0f, 0.0f, 0.0f))
						.Size(FVector2D(12, 12))
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(4, 0, 8, 0)
					[
						SNew(STextBlock).Text(LOCTEXT("HighOverdraw", "6-10")).ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f))
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SColorBlock)
						.Color(FLinearColor(1.0f, 0.0f, 1.0f))
						.Size(FVector2D(12, 12))
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(4, 0, 0, 0)
					[
						SNew(STextBlock).Text(LOCTEXT("VHighOverdraw", ">10")).ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f))
					]
				]
			]
		];
}

TSharedPtr<FSlateDynamicImageBrush> SValidationReportWidget::LoadScreenshotBrush(const FString& ImagePath)
{
	if (!FPaths::FileExists(*ImagePath))
	{
		return nullptr;
	}

	// Load image file
	TArray<uint8> FileData;
	if (!FFileHelper::LoadFileToArray(FileData, *ImagePath))
	{
		return nullptr;
	}

	// Create image wrapper
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

	if (!ImageWrapper.IsValid() || !ImageWrapper->SetCompressed(FileData.GetData(), FileData.Num()))
	{
		return nullptr;
	}

	// Get dimensions
	const int32 Width = ImageWrapper->GetWidth();
	const int32 Height = ImageWrapper->GetHeight();

	// Decompress to BGRA
	TArray<uint8> RawData;
	if (!ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, RawData))
	{
		return nullptr;
	}

	// Create texture name
	FName TextureName = MakeUniqueObjectName(nullptr, UTexture2D::StaticClass(), FName(*FPaths::GetBaseFilename(ImagePath)));

	// Create transient texture
	UTexture2D* Texture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8, TextureName);
	if (!Texture)
	{
		return nullptr;
	}

	// Lock and copy data
	void* TextureData = Texture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(TextureData, RawData.GetData(), RawData.Num());
	Texture->GetPlatformData()->Mips[0].BulkData.Unlock();
	Texture->UpdateResource();

	// Create brush name
	FName BrushName = FName(*FString::Printf(TEXT("OverdrawScreenshot_%s"), *ImagePath));

	// Create dynamic brush from texture
	TSharedPtr<FSlateDynamicImageBrush> Brush = MakeShareable(new FSlateDynamicImageBrush(Texture, FVector2D(Width, Height), BrushName));
	
	return Brush;
}

void SValidationReportWidget::BuildCheckList(const TArray<FAssetValidationResult>& InResults)
{
	CheckItems.Empty();

	for (const FAssetValidationResult& Result : InResults)
	{
		FString SystemNameStr = Result.SystemName.ToString();

		// Add system name header
		{
			TSharedPtr<FCheckItem> Item = MakeShareable(new FCheckItem());
			Item->CheckName = FString::Printf(TEXT("=== %s ==="), *SystemNameStr);
			Item->LimitValue = TEXT("");
			Item->CurrentValue = TEXT("");
			Item->bPassed = true;
			Item->Severity = EAssetCheckSeverity::Info;
			Item->Category = TEXT("Header");
			CheckItems.Add(Item);
		}

		// Convert structured checks to display items
		for (const FAssetValidationCheck& Check : Result.Checks)
		{
			TSharedPtr<FCheckItem> Item = MakeShareable(new FCheckItem());
			Item->CheckName = Check.CheckName;
			Item->LimitValue = Check.LimitValue;
			Item->CurrentValue = Check.CurrentValue;
			Item->bPassed = Check.bPassed;
			Item->Severity = Check.Severity;
			Item->Category = Check.Category;
			CheckItems.Add(Item);
		}

		// Add separator
		{
			TSharedPtr<FCheckItem> Item = MakeShareable(new FCheckItem());
			Item->CheckName = TEXT("");
			Item->LimitValue = TEXT("");
			Item->CurrentValue = TEXT("");
			Item->bPassed = true;
			Item->Category = TEXT("Separator");
			CheckItems.Add(Item);
		}
	}
}

TSharedRef<ITableRow> SValidationReportWidget::OnGenerateRow(TSharedPtr<FCheckItem> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	// Header row (system name)
	if (Item->Category == TEXT("Header"))
	{
		return SNew(STableRow<TSharedPtr<FCheckItem>>, OwnerTable)
			.Padding(FMargin(0, 4, 0, 4))
			[
				SNew(STextBlock)
				.Text(FText::FromString(Item->CheckName))
				.Font(FAppStyle::GetFontStyle("BoldFont"))
				.ColorAndOpacity(FLinearColor(0.9f, 0.8f, 0.4f, 1.0f))
			];
	}

	// Separator row
	if (Item->Category == TEXT("Separator"))
	{
		return SNew(STableRow<TSharedPtr<FCheckItem>>, OwnerTable)
			[
				SNew(SBox)
				.HeightOverride(4.0f)
			];
	}

	// Normal check row
	return SNew(STableRow<TSharedPtr<FCheckItem>>, OwnerTable)
		.Padding(FMargin(0, 1, 0, 1))
		[
			SNew(SBorder)
			.BorderBackgroundColor(FLinearColor(0.02f, 0.02f, 0.02f, 1.0f))
			.Padding(4.0f)
			[
				SNew(SHorizontalBox)
				// Column 1: Check Name
				+ SHorizontalBox::Slot()
				.FillWidth(0.30f)
				.VAlign(VAlign_Center)
				.Padding(4, 0, 8, 0)
				[
					SNew(STextBlock)
					.Text(FText::FromString(Item->CheckName))
					.ColorAndOpacity(FLinearColor(0.85f, 0.85f, 0.85f, 1.0f))
				]
				// Column 2: Limit Value
				+ SHorizontalBox::Slot()
				.FillWidth(0.20f)
				.VAlign(VAlign_Center)
				.Padding(0, 0, 8, 0)
				[
					SNew(STextBlock)
					.Text(FText::FromString(Item->LimitValue))
					.ColorAndOpacity(FLinearColor(0.5f, 0.8f, 0.5f, 1.0f))
				]
				// Column 3: Current Value
				+ SHorizontalBox::Slot()
				.FillWidth(0.25f)
				.VAlign(VAlign_Center)
				.Padding(0, 0, 8, 0)
				[
					SNew(STextBlock)
					.Text(FText::FromString(Item->CurrentValue))
					.ColorAndOpacity(Item->bPassed ? FLinearColor(0.7f, 0.7f, 0.7f, 1.0f) : GetStatusColor(false, Item->Severity))
				]
				// Column 4: Status (√ or ×)
				+ SHorizontalBox::Slot()
				.FillWidth(0.25f)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(GetStatusText(Item->bPassed))
					.ColorAndOpacity(GetStatusColor(Item->bPassed, Item->Severity))
					.Font(FAppStyle::GetFontStyle("BoldFont"))
				]
			]
		];
}

FSlateColor SValidationReportWidget::GetStatusColor(bool bPassed, EAssetCheckSeverity Severity) const
{
	if (bPassed)
	{
		return FLinearColor(0.4f, 0.9f, 0.4f, 1.0f); // Green
	}
	
	switch (Severity)
	{
	case EAssetCheckSeverity::Error:
		return FLinearColor(1.0f, 0.2f, 0.2f, 1.0f); // Red
	case EAssetCheckSeverity::Warning:
		return FLinearColor(1.0f, 0.6f, 0.0f, 1.0f); // Orange
	default:
		return FLinearColor(0.8f, 0.8f, 0.4f, 1.0f); // Yellow
	}
}

FText SValidationReportWidget::GetStatusText(bool bPassed) const
{
	return bPassed ? FText::FromString(TEXT("√ Pass")) : FText::FromString(TEXT("× Fail"));
}

FReply SValidationReportWidget::OnCloseClicked()
{
	TSharedPtr<SWindow> Window = FSlateApplication::Get().FindWidgetWindow(AsShared());
	if (Window.IsValid())
	{
		Window->RequestDestroyWindow();
	}
	return FReply::Handled();
}

FReply SValidationReportWidget::OnExportClicked()
{
	FString ReportText;
	ReportText += TEXT("=== Niagara System Validation Report ===\n\n");

	for (const FAssetValidationResult& Result : Results)
	{
		ReportText += FString::Printf(TEXT("System: %s\n"), *Result.SystemName.ToString());
		ReportText += TEXT("Check Name\t\tLimit\t\tCurrent\t\tStatus\n");
		ReportText += TEXT("----------------------------------------------------------------\n");

		for (const FAssetValidationCheck& Check : Result.Checks)
		{
			FString StatusStr = Check.bPassed ? TEXT("Pass") : TEXT("Fail");
			ReportText += FString::Printf(TEXT("%s\t\t%s\t\t%s\t\t%s\n"),
				*Check.CheckName, *Check.LimitValue, *Check.CurrentValue, *StatusStr);
		}
		
		// Add dynamic analysis info
		if (Result.bHasDynamicAnalysis)
		{
			ReportText += FString::Printf(TEXT("\nDynamic Analysis:\n"));
			ReportText += FString::Printf(TEXT("  Max Overdraw: %d (Threshold: %d)\n"), 
				Result.OverdrawResult.MaxOverdraw, Result.OverdrawResult.MaxOverdrawThreshold);
			ReportText += FString::Printf(TEXT("  Status: %s\n"), 
				Result.OverdrawResult.bPassed ? TEXT("PASSED") : TEXT("FAILED"));
		}
		
		ReportText += TEXT("\n");
	}

	FPlatformApplicationMisc::ClipboardCopy(*ReportText);
	return FReply::Handled();
}

FText SValidationReportWidget::GetSummaryText() const
{
	return FText::Format(
		LOCTEXT("SummaryFormat", "Validated {0} system(s) | {1} passed | {2} failed"),
		FText::AsNumber(TotalSystems),
		FText::AsNumber(TotalPassed),
		FText::AsNumber(TotalFailed)
	);
}

TSharedRef<SWidget> SValidationReportWidget::CreateHeaderRow()
{
	return SNew(SBorder)
	.BorderBackgroundColor(FLinearColor(0.1f, 0.1f, 0.1f, 1.0f))
	.Padding(4.0f)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(0.30f)
		.Padding(4, 0, 0, 0)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("HeaderCheckName", "Check Name"))
			.Font(FAppStyle::GetFontStyle("BoldFont"))
			.ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
		]
		+ SHorizontalBox::Slot()
		.FillWidth(0.20f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("HeaderLimit", "Limit"))
			.Font(FAppStyle::GetFontStyle("BoldFont"))
			.ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
		]
		+ SHorizontalBox::Slot()
		.FillWidth(0.25f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("HeaderCurrent", "Current"))
			.Font(FAppStyle::GetFontStyle("BoldFont"))
			.ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
		]
		+ SHorizontalBox::Slot()
		.FillWidth(0.25f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("HeaderStatus", "Status"))
			.Font(FAppStyle::GetFontStyle("BoldFont"))
			.ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
		]
	];
}

#undef LOCTEXT_NAMESPACE
