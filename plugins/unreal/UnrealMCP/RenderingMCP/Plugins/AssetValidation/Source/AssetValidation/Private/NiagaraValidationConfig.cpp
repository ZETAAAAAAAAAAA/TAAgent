// Copyright Epic Games, Inc. All Rights Reserved.

#include "NiagaraValidationConfig.h"

UNiagaraValidationConfig::UNiagaraValidationConfig()
{
	// Initialize default platform budgets
	HighEndPCBudget.MaxParticles = 500000;
	HighEndPCBudget.MaxGPUEmitters = 20;
	HighEndPCBudget.MaxCPUEmitters = 10;
	HighEndPCBudget.MaxEmittersPerSystem = 30;
	HighEndPCBudget.MaxEstimatedMemoryMB = 200;

	ConsoleBudget.MaxParticles = 200000;
	ConsoleBudget.MaxGPUEmitters = 15;
	ConsoleBudget.MaxCPUEmitters = 5;
	ConsoleBudget.MaxEmittersPerSystem = 20;
	ConsoleBudget.MaxEstimatedMemoryMB = 100;

	MobileBudget.MaxParticles = 50000;
	MobileBudget.MaxGPUEmitters = 5;
	MobileBudget.MaxCPUEmitters = 3;
	MobileBudget.MaxEmittersPerSystem = 10;
	MobileBudget.MaxEstimatedMemoryMB = 32;
}

UNiagaraValidationConfig* UNiagaraValidationConfig::Get()
{
	static UNiagaraValidationConfig* Instance = nullptr;
	if (!Instance)
	{
		Instance = NewObject<UNiagaraValidationConfig>();
		Instance->AddToRoot(); // Prevent garbage collection
		Instance->LoadConfig();
	}
	return Instance;
}
