// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

struct FAssetValidationResult;

class FAssetValidationModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/** Show the validation report window with results */
	static void ShowValidationReportWindow(const TArray<FAssetValidationResult>& Results);

private:
	void RegisterToolMenus();
	void UnregisterToolMenus();

	void OnValidateNiagaraSystems(const TArray<FString>& SelectedAssetPaths);
};
