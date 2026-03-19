// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"
#include "AssetValidationResult.h"
#include "Styling/SlateBrush.h"

class SValidationReportWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SValidationReportWidget) {}
		SLATE_ARGUMENT(TArray<FAssetValidationResult>, ValidationResults)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	/** Check item for list view - represents a single validation check row */
	struct FCheckItem
	{
		FString CheckName;          // Column 1: 检测项目
		FString LimitValue;         // Column 2: 规范值
		FString CurrentValue;       // Column 3: 当前值
		bool bPassed = true;        // Column 4: 是否符合规范
		EAssetCheckSeverity Severity = EAssetCheckSeverity::Info;
		FString Category;           // Category for grouping
		int32 SortPriority = 0;     // For ordering within category
	};

	/** Build the check list from validation results */
	void BuildCheckList(const TArray<FAssetValidationResult>& InResults);

	/** Generate a row widget for a check item */
	TSharedRef<ITableRow> OnGenerateRow(TSharedPtr<FCheckItem> Item, const TSharedRef<STableViewBase>& OwnerTable);

	/** Get color for pass/fail status */
	FSlateColor GetStatusColor(bool bPassed, EAssetCheckSeverity Severity) const;

	/** Get status text */
	FText GetStatusText(bool bPassed) const;

	/** Handle close button */
	FReply OnCloseClicked();

	/** Handle export to clipboard */
	FReply OnExportClicked();

	/** Generate summary text */
	FText GetSummaryText() const;

	/** Generate header row */
	TSharedRef<SWidget> CreateHeaderRow();

	/** Load screenshot from file path */
	TSharedPtr<FSlateDynamicImageBrush> LoadScreenshotBrush(const FString& ImagePath);

	/** Create the overdraw screenshot panel */
	TSharedRef<SWidget> CreateScreenshotPanel(const FAssetValidationResult& Result);

private:
	/** List of all check items */
	TArray<TSharedPtr<FCheckItem>> CheckItems;

	/** Check list view */
	TSharedPtr<SListView<TSharedPtr<FCheckItem>>> CheckListView;

	/** Validation results */
	TArray<FAssetValidationResult> Results;

	/** Screenshot brush for overdraw image */
	TSharedPtr<FSlateDynamicImageBrush> ScreenshotBrush;

	/** Total counts */
	int32 TotalPassed = 0;
	int32 TotalFailed = 0;
	int32 TotalSystems = 0;
};
