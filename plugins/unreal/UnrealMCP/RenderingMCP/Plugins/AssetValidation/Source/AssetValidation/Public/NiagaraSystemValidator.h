// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AssetValidationResult.h"
#include "NiagaraSystemValidator.generated.h"

class UNiagaraSystem;

/** Niagara System asset validator with configurable performance checks */
UCLASS(BlueprintType)
class ASSETVALIDATION_API UNiagaraSystemValidator : public UObject
{
	GENERATED_BODY()

public:
	/** Validate a Niagara System and return results */
	UFUNCTION(BlueprintCallable, Category = "Niagara|Validation")
	static FAssetValidationResult ValidateSystem(UNiagaraSystem* System);

	/** Validate multiple Niagara Systems */
	UFUNCTION(BlueprintCallable, Category = "Niagara|Validation")
	static TArray<FAssetValidationResult> ValidateSystems(const TArray<UNiagaraSystem*>& Systems);
};
