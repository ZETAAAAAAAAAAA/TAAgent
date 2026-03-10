// Copyright Epic Games, Inc. All Rights Reserved.

#include "Commands/EpicUnrealMCPNiagaraCommands.h"
#include "NiagaraSystem.h"
#include "NiagaraEmitter.h"
#include "NiagaraEmitterHandle.h"
#include "NiagaraScript.h"
#include "NiagaraRendererProperties.h"
#include "NiagaraSimulationStageBase.h"
#include "NiagaraTypes.h"
#include "NiagaraParameterStore.h"
#include "NiagaraEditorUtilities.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/SavePackage.h"

// UE 5.7 Stateless Niagara support
#if ENGINE_MAJOR_VERSION > 5 || (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 7)
#include "Stateless/NiagaraStatelessEmitter.h"
#include "Stateless/NiagaraStatelessModule.h"
#include "Stateless/NiagaraStatelessEmitterTemplate.h"
#endif

// Standard Niagara Graph support
#include "NiagaraGraph.h"
#include "NiagaraNode.h"
#include "NiagaraNodeInput.h"
#include "NiagaraNodeOutput.h"
#include "NiagaraNodeFunctionCall.h"
#include "NiagaraNodeOp.h"
#include "NiagaraNodeCustomHlsl.h"
#include "NiagaraScriptSource.h"
#include "EdGraph/EdGraphPin.h"

// Niagara Stack utilities for module operations
#include "ViewModels/Stack/NiagaraStackGraphUtilities.h"

// Runtime particle data access
#include "NiagaraComponent.h"
#include "NiagaraSimCache.h"
#include "EngineUtils.h"

FEpicUnrealMCPNiagaraCommands::FEpicUnrealMCPNiagaraCommands()
{
}

TSharedPtr<FJsonObject> FEpicUnrealMCPNiagaraCommands::HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params)
{
    // Graph Form Tools
    if (CommandType == TEXT("get_niagara_graph"))
    {
        return HandleGetNiagaraGraph(Params);
    }
    else if (CommandType == TEXT("update_niagara_graph"))
    {
        return HandleUpdateNiagaraGraph(Params);
    }
    // Emitter Form Tools
    else if (CommandType == TEXT("get_niagara_emitter"))
    {
        return HandleGetNiagaraEmitter(Params);
    }
    else if (CommandType == TEXT("update_niagara_emitter"))
    {
        return HandleUpdateNiagaraEmitter(Params);
    }
    else if (CommandType == TEXT("get_niagara_compiled_code"))
    {
        return HandleGetNiagaraCompiledCode(Params);
    }
    else if (CommandType == TEXT("get_niagara_particle_attributes"))
    {
        return HandleGetNiagaraParticleAttributes(Params);
    }
    else
    {
        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetBoolField("success", false);
        Result->SetStringField("error", FString::Printf(TEXT("Unknown Niagara command: %s"), *CommandType));
        return Result;
    }
}

UNiagaraSystem* FEpicUnrealMCPNiagaraCommands::LoadNiagaraSystemAsset(const FString& AssetPath)
{
    return LoadObject<UNiagaraSystem>(nullptr, *AssetPath);
}

// ============================================================================
// Emitter Form Tools - Read Operations Implementation
// ============================================================================

TSharedPtr<FJsonObject> FEpicUnrealMCPNiagaraCommands::HandleGetNiagaraEmitter(const TSharedPtr<FJsonObject>& Params)
{
    TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
    
    FString AssetPath;
    if (!Params->TryGetStringField(TEXT("asset_path"), AssetPath))
    {
        Result->SetBoolField(TEXT("success"), false);
        Result->SetStringField(TEXT("error"), TEXT("Missing required parameter: asset_path"));
        return Result;
    }
    
    UNiagaraSystem* NiagaraSystem = LoadNiagaraSystemAsset(AssetPath);
    if (!NiagaraSystem)
    {
        Result->SetBoolField(TEXT("success"), false);
        Result->SetStringField(TEXT("error"), FString::Printf(TEXT("Failed to load Niagara system: %s"), *AssetPath));
        return Result;
    }
    
    FString DetailLevel = TEXT("overview");
    Params->TryGetStringField(TEXT("detail_level"), DetailLevel);
    
    TArray<FString> IncludeSections = ParseIncludeSections(Params);
    
    TArray<FString> RequestedEmitters;
    const TArray<TSharedPtr<FJsonValue>>* EmittersArray;
    if (Params->TryGetArrayField(TEXT("emitters"), EmittersArray))
    {
        for (const auto& Val : *EmittersArray)
        {
            RequestedEmitters.Add(Val->AsString());
        }
    }
    
    Result->SetStringField(TEXT("asset_name"), NiagaraSystem->GetName());
    Result->SetStringField(TEXT("asset_path"), AssetPath);
    Result->SetNumberField(TEXT("emitter_count"), NiagaraSystem->GetNumEmitters());
    
    TArray<TSharedPtr<FJsonValue>> EmittersJson;
    
    for (FNiagaraEmitterHandle& Handle : NiagaraSystem->GetEmitterHandles())
    {
        if (!Handle.IsValid()) continue;
        
        FString EmitterName = Handle.GetName().ToString();
        
        if (RequestedEmitters.Num() > 0 && !RequestedEmitters.Contains(EmitterName))
        {
            continue;
        }
        
        if (DetailLevel == TEXT("overview"))
        {
            TSharedPtr<FJsonObject> EmitterOverview = MakeShareable(new FJsonObject);
            EmitterOverview->SetStringField(TEXT("name"), EmitterName);
            EmitterOverview->SetBoolField(TEXT("is_enabled"), Handle.GetIsEnabled());
            
            FString ModeStr = TEXT("Standard");
            if (Handle.GetEmitterMode() == ENiagaraEmitterMode::Stateless)
            {
                ModeStr = TEXT("Stateless");
            }
            EmitterOverview->SetStringField(TEXT("mode"), ModeStr);
            
            EmittersJson.Add(MakeShareable(new FJsonValueObject(EmitterOverview)));
        }
        else
        {
            TSharedPtr<FJsonObject> EmitterDetails = GetEmitterDetails(Handle, NiagaraSystem, IncludeSections);
            EmittersJson.Add(MakeShareable(new FJsonValueObject(EmitterDetails)));
        }
    }
    
    Result->SetArrayField(TEXT("emitters"), EmittersJson);
    Result->SetBoolField(TEXT("success"), true);
    
    return Result;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPNiagaraCommands::GetNiagaraSystemOverview(UNiagaraSystem* System)
{
    TSharedPtr<FJsonObject> Overview = MakeShareable(new FJsonObject);
    
    Overview->SetStringField(TEXT("name"), System->GetName());
    Overview->SetNumberField(TEXT("emitter_count"), System->GetNumEmitters());
    
    return Overview;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPNiagaraCommands::GetEmitterDetails(FNiagaraEmitterHandle& Handle, 
    UNiagaraSystem* System, const TArray<FString>& IncludeSections)
{
    TSharedPtr<FJsonObject> EmitterJson = MakeShareable(new FJsonObject);
    
    FString EmitterName = Handle.GetName().ToString();
    EmitterJson->SetStringField(TEXT("name"), EmitterName);
    EmitterJson->SetBoolField(TEXT("is_enabled"), Handle.GetIsEnabled());
    
    FString ModeStr = TEXT("Standard");
    bool bIsStateless = false;
    if (Handle.GetEmitterMode() == ENiagaraEmitterMode::Stateless)
    {
        ModeStr = TEXT("Stateless");
        bIsStateless = true;
    }
    EmitterJson->SetStringField(TEXT("mode"), ModeStr);
    
    FVersionedNiagaraEmitterData* EmitterData = Handle.GetEmitterData();
    if (!EmitterData)
    {
        return EmitterJson;
    }
    
    // Stateless modules (UE 5.7+)
#if ENGINE_MAJOR_VERSION > 5 || (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 7)
    if (bIsStateless && (ShouldInclude(IncludeSections, TEXT("modules")) || ShouldInclude(IncludeSections, TEXT("all"))))
    {
        TArray<TSharedPtr<FJsonValue>> ModulesJson;
        
        UNiagaraStatelessEmitter* StatelessEmitter = Handle.GetStatelessEmitter();
        if (StatelessEmitter)
        {
            for (UNiagaraStatelessModule* Module : StatelessEmitter->GetModules())
            {
                if (Module)
                {
                    TSharedPtr<FJsonObject> ModuleJson = MakeShareable(new FJsonObject);
                    ModuleJson->SetStringField(TEXT("name"), Module->GetName());
                    ModuleJson->SetStringField(TEXT("type"), Module->GetClass()->GetName());
                    ModuleJson->SetBoolField(TEXT("is_enabled"), Module->IsModuleEnabled());
                    ModulesJson.Add(MakeShareable(new FJsonValueObject(ModuleJson)));
                }
            }
        }
        
        EmitterJson->SetArrayField(TEXT("modules"), ModulesJson);
        EmitterJson->SetNumberField(TEXT("module_count"), ModulesJson.Num());
    }
#endif
    
    // Standard modules (from Graph nodes)
    if (!bIsStateless && (ShouldInclude(IncludeSections, TEXT("modules")) || ShouldInclude(IncludeSections, TEXT("all"))))
    {
        TArray<TSharedPtr<FJsonValue>> ModulesJson;
        TSet<FString> UniqueModuleNames;  // Avoid duplicates
        
        // Get modules from Spawn script
        if (EmitterData->SpawnScriptProps.Script)
        {
            UNiagaraScriptSource* Source = Cast<UNiagaraScriptSource>(EmitterData->SpawnScriptProps.Script->GetLatestSource());
            UNiagaraGraph* Graph = Source ? Source->NodeGraph : nullptr;
            if (Graph)
            {
                for (UEdGraphNode* NodeBase : Graph->Nodes)
                {
                    UNiagaraNode* Node = Cast<UNiagaraNode>(NodeBase);
                    if (!Node) continue;
                    
                    FString NodeClass = Node->GetClass()->GetName();
                    // Check if this is a module node
                    if (NodeClass.Contains(TEXT("Module")) || NodeClass.Contains(TEXT("Output")))
                    {
                        FString ModuleName = Node->GetName();
                        
                        // Skip duplicates and system nodes
                        if (UniqueModuleNames.Contains(ModuleName))
                        {
                            continue;
                        }
                        
                        // Skip output and input nodes
                        if (NodeClass.Contains(TEXT("Output")) || NodeClass.Contains(TEXT("Input")))
                        {
                            continue;
                        }
                        
                        UniqueModuleNames.Add(ModuleName);
                        
                        TSharedPtr<FJsonObject> ModuleJson = MakeShareable(new FJsonObject);
                        ModuleJson->SetStringField(TEXT("name"), ModuleName);
                        ModuleJson->SetStringField(TEXT("type"), NodeClass);
                        ModuleJson->SetStringField(TEXT("script"), TEXT("spawn"));
                        ModuleJson->SetBoolField(TEXT("is_enabled"), true);  // Standard modules don't have enabled state
                        ModulesJson.Add(MakeShareable(new FJsonValueObject(ModuleJson)));
                    }
                }
            }
        }
        
        // Get modules from Update script
        if (EmitterData->UpdateScriptProps.Script)
        {
            UNiagaraScriptSource* Source = Cast<UNiagaraScriptSource>(EmitterData->UpdateScriptProps.Script->GetLatestSource());
            UNiagaraGraph* Graph = Source ? Source->NodeGraph : nullptr;
            if (Graph)
            {
                for (UEdGraphNode* NodeBase : Graph->Nodes)
                {
                    UNiagaraNode* Node = Cast<UNiagaraNode>(NodeBase);
                    if (!Node) continue;
                    
                    FString NodeClass = Node->GetClass()->GetName();
                    if (NodeClass.Contains(TEXT("Module")))
                    {
                        FString ModuleName = Node->GetName();
                        
                        if (UniqueModuleNames.Contains(ModuleName))
                        {
                            continue;
                        }
                        
                        UniqueModuleNames.Add(ModuleName);
                        
                        TSharedPtr<FJsonObject> ModuleJson = MakeShareable(new FJsonObject);
                        ModuleJson->SetStringField(TEXT("name"), ModuleName);
                        ModuleJson->SetStringField(TEXT("type"), NodeClass);
                        ModuleJson->SetStringField(TEXT("script"), TEXT("update"));
                        ModuleJson->SetBoolField(TEXT("is_enabled"), true);
                        ModulesJson.Add(MakeShareable(new FJsonValueObject(ModuleJson)));
                    }
                }
            }
        }
        
        EmitterJson->SetArrayField(TEXT("modules"), ModulesJson);
        EmitterJson->SetNumberField(TEXT("module_count"), ModulesJson.Num());
    }
    
    // Scripts (Standard mode only)
    if (!bIsStateless && (ShouldInclude(IncludeSections, TEXT("scripts")) || ShouldInclude(IncludeSections, TEXT("all"))))
    {
        TSharedPtr<FJsonObject> ScriptsJson = MakeShareable(new FJsonObject);
        
        if (EmitterData->SpawnScriptProps.Script)
        {
            ScriptsJson->SetObjectField(TEXT("spawn"), GetScriptDetails(EmitterData->SpawnScriptProps.Script));
        }
        if (EmitterData->UpdateScriptProps.Script)
        {
            ScriptsJson->SetObjectField(TEXT("update"), GetScriptDetails(EmitterData->UpdateScriptProps.Script));
        }
        
        TArray<TSharedPtr<FJsonValue>> EventHandlersJson;
        for (const FNiagaraEventScriptProperties& EventHandler : EmitterData->GetEventHandlers())
        {
            if (EventHandler.Script)
            {
                TSharedPtr<FJsonObject> EventJson = GetScriptDetails(EventHandler.Script);
                EventJson->SetStringField(TEXT("execution_mode"), 
                    EventHandler.ExecutionMode == EScriptExecutionMode::EveryParticle ? TEXT("EveryParticle") : 
                    (EventHandler.ExecutionMode == EScriptExecutionMode::SpawnedParticles ? TEXT("SpawnedParticles") : TEXT("SingleParticle")));
                EventHandlersJson.Add(MakeShareable(new FJsonValueObject(EventJson)));
            }
        }
        if (EventHandlersJson.Num() > 0)
        {
            ScriptsJson->SetArrayField(TEXT("event_handlers"), EventHandlersJson);
        }
        
        EmitterJson->SetObjectField(TEXT("scripts"), ScriptsJson);
    }
    
    // Renderers
    if (ShouldInclude(IncludeSections, TEXT("renderers")) || ShouldInclude(IncludeSections, TEXT("all")))
    {
        TArray<TSharedPtr<FJsonValue>> RenderersJson;
        const TArray<UNiagaraRendererProperties*>& Renderers = EmitterData->GetRenderers();
        for (UNiagaraRendererProperties* Renderer : Renderers)
        {
            if (Renderer)
            {
                RenderersJson.Add(MakeShareable(new FJsonValueObject(GetRendererDetails(Renderer))));
            }
        }
        EmitterJson->SetArrayField(TEXT("renderers"), RenderersJson);
        EmitterJson->SetNumberField(TEXT("renderer_count"), RenderersJson.Num());
    }
    
    // Simulation stages
    if (ShouldInclude(IncludeSections, TEXT("simulation_stages")) || ShouldInclude(IncludeSections, TEXT("all")))
    {
        TArray<TSharedPtr<FJsonValue>> StagesJson;
        for (UNiagaraSimulationStageBase* Stage : EmitterData->GetSimulationStages())
        {
            if (Stage)
            {
                StagesJson.Add(MakeShareable(new FJsonValueObject(GetSimulationStageDetails(Stage))));
            }
        }
        EmitterJson->SetArrayField(TEXT("simulation_stages"), StagesJson);
        EmitterJson->SetNumberField(TEXT("simulation_stage_count"), StagesJson.Num());
    }
    
    // Parameters
    if (ShouldInclude(IncludeSections, TEXT("parameters")) || ShouldInclude(IncludeSections, TEXT("all")))
    {
        TArray<TSharedPtr<FJsonValue>> ParametersJson;
        
        if (bIsStateless)
        {
            // Stateless parameters
#if ENGINE_MAJOR_VERSION > 5 || (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 7)
            UNiagaraStatelessEmitter* StatelessEmitter = Handle.GetStatelessEmitter();
            if (StatelessEmitter)
            {
                TSharedPtr<FJsonObject> ParamInfo = MakeShareable(new FJsonObject);
                ParamInfo->SetStringField(TEXT("source"), TEXT("stateless_modules"));
                ParamInfo->SetStringField(TEXT("note"), TEXT("Use 'modules' section to see available modules and their properties"));
                ParametersJson.Add(MakeShareable(new FJsonValueObject(ParamInfo)));
            }
#endif
        }
        else
        {
            // Standard mode - parameters from scripts
            if (EmitterData->SpawnScriptProps.Script)
            {
                TSharedPtr<FJsonObject> ParamInfo = MakeShareable(new FJsonObject);
                ParamInfo->SetStringField(TEXT("source"), TEXT("spawn_script"));
                ParamInfo->SetStringField(TEXT("script_name"), EmitterData->SpawnScriptProps.Script->GetName());
                ParametersJson.Add(MakeShareable(new FJsonValueObject(ParamInfo)));
            }
        }
        
        EmitterJson->SetArrayField(TEXT("parameters"), ParametersJson);
    }
    
    // Stateless compatibility analysis (Standard mode only)
    if (!bIsStateless && ShouldInclude(IncludeSections, TEXT("stateless_analysis")))
    {
        TSharedPtr<FJsonObject> Analysis = AnalyzeStatelessCompatibility(Handle);
        EmitterJson->SetObjectField(TEXT("stateless_analysis"), Analysis);
    }
    
    return EmitterJson;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPNiagaraCommands::GetScriptDetails(UNiagaraScript* Script)
{
    TSharedPtr<FJsonObject> ScriptJson = MakeShareable(new FJsonObject);
    
    if (!Script)
    {
        return ScriptJson;
    }
    
    ScriptJson->SetStringField(TEXT("name"), Script->GetName());
    
    FString UsageStr = TEXT("Unknown");
    switch (Script->GetUsage())
    {
        case ENiagaraScriptUsage::Function: UsageStr = TEXT("Function"); break;
        case ENiagaraScriptUsage::Module: UsageStr = TEXT("Module"); break;
        case ENiagaraScriptUsage::DynamicInput: UsageStr = TEXT("DynamicInput"); break;
        case ENiagaraScriptUsage::ParticleSpawnScript: UsageStr = TEXT("ParticleSpawn"); break;
        case ENiagaraScriptUsage::ParticleUpdateScript: UsageStr = TEXT("ParticleUpdate"); break;
        case ENiagaraScriptUsage::EmitterSpawnScript: UsageStr = TEXT("EmitterSpawn"); break;
        case ENiagaraScriptUsage::EmitterUpdateScript: UsageStr = TEXT("EmitterUpdate"); break;
        case ENiagaraScriptUsage::SystemSpawnScript: UsageStr = TEXT("SystemSpawn"); break;
        case ENiagaraScriptUsage::SystemUpdateScript: UsageStr = TEXT("SystemUpdate"); break;
        case ENiagaraScriptUsage::ParticleEventScript: UsageStr = TEXT("ParticleEvent"); break;
        default: break;
    }
    ScriptJson->SetStringField(TEXT("usage"), UsageStr);
    
    TArray<TSharedPtr<FJsonValue>> ParametersJson;
    const FNiagaraParameterStore& ParamStore = Script->RapidIterationParameters;
    
    TArrayView<const FNiagaraVariableWithOffset> ParamVariables = ParamStore.ReadParameterVariables();
    for (const FNiagaraVariableWithOffset& ParamWithOffset : ParamVariables)
    {
        TSharedPtr<FJsonObject> ParamJson = MakeShareable(new FJsonObject);
        
        FString ParamName = ParamWithOffset.GetName().ToString();
        ParamJson->SetStringField(TEXT("name"), ParamName);
        
        FNiagaraTypeDefinition ParamType = ParamWithOffset.GetType();
        FString TypeName = ParamType.GetName();
        ParamJson->SetStringField(TEXT("type"), TypeName);
        
        int32 Offset = ParamWithOffset.Offset;
        if (Offset >= 0)
        {
            const FNiagaraTypeDefinition& TypeDef = ParamWithOffset.GetType();
            
            if (TypeDef == FNiagaraTypeDefinition::GetFloatDef())
            {
                float Value = ParamStore.GetParameterValueFromOffset<float>(Offset);
                ParamJson->SetNumberField(TEXT("value"), Value);
            }
            else if (TypeDef == FNiagaraTypeDefinition::GetIntDef())
            {
                int32 Value = ParamStore.GetParameterValueFromOffset<int32>(Offset);
                ParamJson->SetNumberField(TEXT("value"), Value);
            }
            else if (TypeDef == FNiagaraTypeDefinition::GetBoolDef())
            {
                bool Value = ParamStore.GetParameterValueFromOffset<bool>(Offset);
                ParamJson->SetBoolField(TEXT("value"), Value);
            }
            else if (TypeDef == FNiagaraTypeDefinition::GetVec3Def() || 
                     TypeDef == FNiagaraTypeDefinition::GetPositionDef())
            {
                FVector3f Value = ParamStore.GetParameterValueFromOffset<FVector3f>(Offset);
                TArray<TSharedPtr<FJsonValue>> VecArray;
                VecArray.Add(MakeShareable(new FJsonValueNumber(Value.X)));
                VecArray.Add(MakeShareable(new FJsonValueNumber(Value.Y)));
                VecArray.Add(MakeShareable(new FJsonValueNumber(Value.Z)));
                ParamJson->SetArrayField(TEXT("value"), VecArray);
            }
            else if (TypeDef == FNiagaraTypeDefinition::GetVec4Def() ||
                     TypeDef == FNiagaraTypeDefinition::GetColorDef())
            {
                FVector4f Value = ParamStore.GetParameterValueFromOffset<FVector4f>(Offset);
                TArray<TSharedPtr<FJsonValue>> VecArray;
                VecArray.Add(MakeShareable(new FJsonValueNumber(Value.X)));
                VecArray.Add(MakeShareable(new FJsonValueNumber(Value.Y)));
                VecArray.Add(MakeShareable(new FJsonValueNumber(Value.Z)));
                VecArray.Add(MakeShareable(new FJsonValueNumber(Value.W)));
                ParamJson->SetArrayField(TEXT("value"), VecArray);
            }
            else
            {
                int32 Size = TypeDef.GetSize();
                ParamJson->SetNumberField(TEXT("size_bytes"), Size);
            }
        }
        
        ParametersJson.Add(MakeShareable(new FJsonValueObject(ParamJson)));
    }
    
    ScriptJson->SetArrayField(TEXT("parameters"), ParametersJson);
    ScriptJson->SetNumberField(TEXT("parameter_count"), ParametersJson.Num());
    
    return ScriptJson;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPNiagaraCommands::GetRendererDetails(UNiagaraRendererProperties* Renderer)
{
    TSharedPtr<FJsonObject> RendererJson = MakeShareable(new FJsonObject);
    
    if (!Renderer)
    {
        return RendererJson;
    }
    
    FString RendererType = Renderer->GetClass()->GetName();
    RendererType.RemoveFromEnd(TEXT("Properties"));
    RendererJson->SetStringField(TEXT("type"), RendererType);
    RendererJson->SetBoolField(TEXT("is_enabled"), Renderer->GetIsEnabled());
    
    return RendererJson;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPNiagaraCommands::GetSimulationStageDetails(UNiagaraSimulationStageBase* Stage)
{
    TSharedPtr<FJsonObject> StageJson = MakeShareable(new FJsonObject);
    
    if (!Stage)
    {
        return StageJson;
    }
    
    StageJson->SetStringField(TEXT("name"), Stage->GetName());
    StageJson->SetBoolField(TEXT("enabled"), Stage->bEnabled);
    
    return StageJson;
}

// ============================================================================
// Emitter Form Tools - Update Operations Implementation
// ============================================================================

TSharedPtr<FJsonObject> FEpicUnrealMCPNiagaraCommands::HandleUpdateNiagaraEmitter(const TSharedPtr<FJsonObject>& Params)
{
    TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
    
    FString AssetPath;
    if (!Params->TryGetStringField(TEXT("asset_path"), AssetPath))
    {
        Result->SetBoolField(TEXT("success"), false);
        Result->SetStringField(TEXT("error"), TEXT("Missing required parameter: asset_path"));
        return Result;
    }
    
    const TArray<TSharedPtr<FJsonValue>>* OperationsArray;
    if (!Params->TryGetArrayField(TEXT("operations"), OperationsArray))
    {
        Result->SetBoolField(TEXT("success"), false);
        Result->SetStringField(TEXT("error"), TEXT("Missing required parameter: operations"));
        return Result;
    }
    
    UNiagaraSystem* NiagaraSystem = LoadNiagaraSystemAsset(AssetPath);
    if (!NiagaraSystem)
    {
        Result->SetBoolField(TEXT("success"), false);
        Result->SetStringField(TEXT("error"), FString::Printf(TEXT("Failed to load Niagara system: %s"), *AssetPath));
        return Result;
    }
    
    TArray<TSharedPtr<FJsonValue>> ResultsArray;
    int32 SuccessCount = 0;
    int32 FailCount = 0;
    
    for (const TSharedPtr<FJsonValue>& OpValue : *OperationsArray)
    {
        TSharedPtr<FJsonObject> Op = OpValue->AsObject();
        if (!Op.IsValid()) continue;
        
        FString Target = Op->GetStringField(TEXT("target"));
        TSharedPtr<FJsonObject> OpResult;
        
        if (Target == TEXT("emitter"))
        {
            OpResult = ProcessEmitterOperation(NiagaraSystem, Op);
        }
        else if (Target == TEXT("renderer"))
        {
            OpResult = ProcessRendererOperation(NiagaraSystem, Op);
        }
        else if (Target == TEXT("parameter"))
        {
            OpResult = ProcessParameterOperation(NiagaraSystem, Op);
        }
        else if (Target == TEXT("sim_stage"))
        {
            OpResult = ProcessSimStageOperation(NiagaraSystem, Op);
        }
        else if (Target == TEXT("stateless_module"))
        {
            OpResult = ProcessStatelessModuleOperation(NiagaraSystem, Op);
        }
        else
        {
            OpResult = MakeShareable(new FJsonObject);
            OpResult->SetBoolField(TEXT("success"), false);
            OpResult->SetStringField(TEXT("error"), FString::Printf(TEXT("Unknown target: %s"), *Target));
        }
        
        if (OpResult->GetBoolField(TEXT("success")))
        {
            SuccessCount++;
        }
        else
        {
            FailCount++;
        }
        
        ResultsArray.Add(MakeShareable(new FJsonValueObject(OpResult)));
    }
    
    // Mark package dirty
    NiagaraSystem->MarkPackageDirty();
    
    Result->SetBoolField(TEXT("success"), true);
    Result->SetStringField(TEXT("asset_path"), AssetPath);
    Result->SetNumberField(TEXT("success_count"), SuccessCount);
    Result->SetNumberField(TEXT("fail_count"), FailCount);
    Result->SetArrayField(TEXT("results"), ResultsArray);
    
    return Result;
}

FNiagaraEmitterHandle* FEpicUnrealMCPNiagaraCommands::FindEmitterHandle(UNiagaraSystem* System, const FString& EmitterName)
{
    if (!System) return nullptr;
    
    TArray<FNiagaraEmitterHandle>& Handles = System->GetEmitterHandles();
    for (FNiagaraEmitterHandle& Handle : Handles)
    {
        if (Handle.IsValid() && Handle.GetName().ToString() == EmitterName)
        {
            return &Handle;
        }
    }
    return nullptr;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPNiagaraCommands::ProcessEmitterOperation(UNiagaraSystem* System, const TSharedPtr<FJsonObject>& Op)
{
    TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
    
    FString Action = Op->GetStringField(TEXT("action"));
    FString EmitterName;
    Op->TryGetStringField(TEXT("name"), EmitterName);
    
    // Add emitter
    if (Action == TEXT("add"))
    {
        FString TemplatePath;
        if (!Op->TryGetStringField(TEXT("template"), TemplatePath))
        {
            Result->SetBoolField(TEXT("success"), false);
            Result->SetStringField(TEXT("error"), TEXT("Missing 'template' parameter for add action"));
            return Result;
        }
        
        UNiagaraEmitter* TemplateEmitter = LoadObject<UNiagaraEmitter>(nullptr, *TemplatePath);
        if (!TemplateEmitter)
        {
            Result->SetBoolField(TEXT("success"), false);
            Result->SetStringField(TEXT("error"), FString::Printf(TEXT("Failed to load emitter template: %s"), *TemplatePath));
            return Result;
        }
        
        FGuid NewId = FNiagaraEditorUtilities::AddEmitterToSystem(*System, *TemplateEmitter, FGuid::NewGuid(), true);
        if (NewId.IsValid())
        {
            // Rename if name provided
            if (!EmitterName.IsEmpty())
            {
                FNiagaraEmitterHandle* NewHandle = nullptr;
                for (FNiagaraEmitterHandle& Handle : System->GetEmitterHandles())
                {
                    if (Handle.GetId() == NewId)
                    {
                        NewHandle = &Handle;
                        break;
                    }
                }
                if (NewHandle)
                {
                    NewHandle->SetName(*EmitterName, *System);
                }
            }
            
            Result->SetBoolField(TEXT("success"), true);
            Result->SetStringField(TEXT("action"), TEXT("add"));
            Result->SetStringField(TEXT("emitter_name"), EmitterName);
            Result->SetStringField(TEXT("emitter_id"), NewId.ToString());
        }
        else
        {
            Result->SetBoolField(TEXT("success"), false);
            Result->SetStringField(TEXT("error"), TEXT("Failed to add emitter to system"));
        }
        return Result;
    }
    
    // Remove emitter
    if (Action == TEXT("remove"))
    {
        FNiagaraEmitterHandle* Handle = FindEmitterHandle(System, EmitterName);
        if (!Handle)
        {
            Result->SetBoolField(TEXT("success"), false);
            Result->SetStringField(TEXT("error"), FString::Printf(TEXT("Emitter not found: %s"), *EmitterName));
            return Result;
        }
        
        TSet<FGuid> IdsToDelete;
        IdsToDelete.Add(Handle->GetId());
        System->RemoveEmitterHandlesById(IdsToDelete);
        
        Result->SetBoolField(TEXT("success"), true);
        Result->SetStringField(TEXT("action"), TEXT("remove"));
        Result->SetStringField(TEXT("emitter_name"), EmitterName);
        return Result;
    }
    
    // Find emitter for other operations
    FNiagaraEmitterHandle* Handle = FindEmitterHandle(System, EmitterName);
    if (!Handle)
    {
        Result->SetBoolField(TEXT("success"), false);
        Result->SetStringField(TEXT("error"), FString::Printf(TEXT("Emitter not found: %s"), *EmitterName));
        return Result;
    }
    
    // Set enabled
    if (Action == TEXT("set_enabled"))
    {
        bool bEnabled = Op->GetBoolField(TEXT("value"));
        Handle->SetIsEnabled(bEnabled, *System, true);
        
        Result->SetBoolField(TEXT("success"), true);
        Result->SetStringField(TEXT("action"), TEXT("set_enabled"));
        Result->SetStringField(TEXT("emitter_name"), EmitterName);
        Result->SetBoolField(TEXT("value"), bEnabled);
    }
    // Rename
    else if (Action == TEXT("rename"))
    {
        FString NewName = Op->GetStringField(TEXT("value"));
        Handle->SetName(*NewName, *System);
        
        Result->SetBoolField(TEXT("success"), true);
        Result->SetStringField(TEXT("action"), TEXT("rename"));
        Result->SetStringField(TEXT("old_name"), EmitterName);
        Result->SetStringField(TEXT("new_name"), NewName);
    }
    // Set mode
    else if (Action == TEXT("set_mode"))
    {
        FString ModeStr = Op->GetStringField(TEXT("value"));
        ENiagaraEmitterMode Mode = ENiagaraEmitterMode::Standard;
        if (ModeStr.ToLower() == TEXT("stateless"))
        {
            Mode = ENiagaraEmitterMode::Stateless;
        }
        Handle->SetEmitterMode(*System, Mode);
        
        Result->SetBoolField(TEXT("success"), true);
        Result->SetStringField(TEXT("action"), TEXT("set_mode"));
        Result->SetStringField(TEXT("emitter_name"), EmitterName);
        Result->SetStringField(TEXT("mode"), ModeStr);
    }
    // Add module to emitter script (spawn or update)
    else if (Action == TEXT("add_module"))
    {
        FString ModulePath;
        if (!Op->TryGetStringField(TEXT("module_path"), ModulePath))
        {
            Result->SetBoolField(TEXT("success"), false);
            Result->SetStringField(TEXT("error"), TEXT("Missing 'module_path' parameter for add_module action"));
            return Result;
        }

        // Determine which script to add to (default: update)
        FString ScriptType = TEXT("update");
        Op->TryGetStringField(TEXT("script"), ScriptType);

        ENiagaraScriptUsage TargetUsage = (ScriptType.ToLower() == TEXT("spawn"))
            ? ENiagaraScriptUsage::ParticleSpawnScript
            : ENiagaraScriptUsage::ParticleUpdateScript;

        // Load module script
        UNiagaraScript* ModuleScript = LoadObject<UNiagaraScript>(nullptr, *ModulePath);
        if (!ModuleScript)
        {
            Result->SetBoolField(TEXT("success"), false);
            Result->SetStringField(TEXT("error"), FString::Printf(TEXT("Failed to load module: %s"), *ModulePath));
            return Result;
        }

        // Get emitter data and target script
        FVersionedNiagaraEmitterData* EmitterData = Handle->GetEmitterData();
        if (!EmitterData)
        {
            Result->SetBoolField(TEXT("success"), false);
            Result->SetStringField(TEXT("error"), TEXT("Failed to get emitter data"));
            return Result;
        }

        UNiagaraScript* TargetScript = EmitterData->GetScript(TargetUsage, FGuid());
        if (!TargetScript)
        {
            Result->SetBoolField(TEXT("success"), false);
            Result->SetStringField(TEXT("error"), FString::Printf(TEXT("Failed to get %s script"), *ScriptType));
            return Result;
        }

        // Get graph from script source
        UNiagaraScriptSource* ScriptSource = Cast<UNiagaraScriptSource>(TargetScript->GetSource(FGuid()));
        if (!ScriptSource || !ScriptSource->NodeGraph)
        {
            Result->SetBoolField(TEXT("success"), false);
            Result->SetStringField(TEXT("error"), TEXT("Failed to get script graph"));
            return Result;
        }

        UNiagaraGraph* Graph = ScriptSource->NodeGraph;

        // Find output node
        UNiagaraNodeOutput* OutputNode = nullptr;
        for (UEdGraphNode* NodeBase : Graph->Nodes)
        {
            UNiagaraNodeOutput* Node = Cast<UNiagaraNodeOutput>(NodeBase);
            if (Node && Node->GetUsage() == TargetUsage)
            {
                OutputNode = Node;
                break;
            }
        }

        if (!OutputNode)
        {
            Result->SetBoolField(TEXT("success"), false);
            Result->SetStringField(TEXT("error"), TEXT("Failed to find output node"));
            return Result;
        }

        // Add module to stack
        UNiagaraNodeFunctionCall* NewModule = FNiagaraStackGraphUtilities::AddScriptModuleToStack(
            ModuleScript,
            *OutputNode,
            INDEX_NONE,      // TargetIndex - add to end
            FString(),       // SuggestedName - empty for default
            FGuid()          // VersionGuid - empty for default
        );

        if (NewModule)
        {
            TargetScript->Modify();

            Result->SetBoolField(TEXT("success"), true);
            Result->SetStringField(TEXT("action"), TEXT("add_module"));
            Result->SetStringField(TEXT("emitter_name"), EmitterName);
            Result->SetStringField(TEXT("script"), ScriptType);
            Result->SetStringField(TEXT("module_path"), ModulePath);
            Result->SetStringField(TEXT("module_name"), NewModule->GetName());
        }
        else
        {
            Result->SetBoolField(TEXT("success"), false);
            Result->SetStringField(TEXT("error"), TEXT("Failed to add module to stack"));
        }
    }
    // Convert to Stateless mode (UE 5.7+)
    else if (Action == TEXT("convert_to_stateless"))
    {
#if ENGINE_MAJOR_VERSION > 5 || (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 7)
        // Check if already Stateless
        if (Handle->GetEmitterMode() == ENiagaraEmitterMode::Stateless)
        {
            Result->SetBoolField(TEXT("success"), true);
            Result->SetStringField(TEXT("message"), TEXT("Emitter is already in Stateless mode"));
            return Result;
        }
        
        // Optional: force conversion even with warnings
        bool bForce = false;
        Op->TryGetBoolField(TEXT("force"), bForce);
        
        // Analyze compatibility
        TSharedPtr<FJsonObject> Analysis = AnalyzeStatelessCompatibility(*Handle);
        bool bCanConvert = Analysis->GetBoolField(TEXT("can_convert"));
        
        if (!bCanConvert && !bForce)
        {
            Result->SetBoolField(TEXT("success"), false);
            Result->SetStringField(TEXT("error"), TEXT("Emitter is not convertible. Use 'force=true' to attempt anyway."));
            Result->SetObjectField(TEXT("analysis"), Analysis);
            return Result;
        }
        
        // Get parameter values before conversion
        FVersionedNiagaraEmitterData* EmitterData = Handle->GetEmitterData();
        TMap<FString, float> FloatValues;
        TMap<FString, FVector3f> VectorValues;
        TMap<FString, FVector4f> ColorValues;
        
        auto CaptureParamValues = [&](UNiagaraScript* Script)
        {
            if (!Script) return;
            
            const FNiagaraParameterStore& ParamStore = Script->RapidIterationParameters;
            TArrayView<const FNiagaraVariableWithOffset> ParamVariables = ParamStore.ReadParameterVariables();
            
            for (const FNiagaraVariableWithOffset& ParamWithOffset : ParamVariables)
            {
                FString ParamName = ParamWithOffset.GetName().ToString();
                int32 Offset = ParamWithOffset.Offset;
                const FNiagaraTypeDefinition& TypeDef = ParamWithOffset.GetType();
                
                if (TypeDef == FNiagaraTypeDefinition::GetFloatDef())
                {
                    FloatValues.Add(ParamName, ParamStore.GetParameterValueFromOffset<float>(Offset));
                }
                else if (TypeDef == FNiagaraTypeDefinition::GetVec3Def() || 
                         TypeDef == FNiagaraTypeDefinition::GetPositionDef())
                {
                    VectorValues.Add(ParamName, ParamStore.GetParameterValueFromOffset<FVector3f>(Offset));
                }
                else if (TypeDef == FNiagaraTypeDefinition::GetVec4Def() ||
                         TypeDef == FNiagaraTypeDefinition::GetColorDef())
                {
                    ColorValues.Add(ParamName, ParamStore.GetParameterValueFromOffset<FVector4f>(Offset));
                }
            }
        };
        
        if (EmitterData)
        {
            if (EmitterData->SpawnScriptProps.Script)
            {
                CaptureParamValues(EmitterData->SpawnScriptProps.Script);
            }
            if (EmitterData->UpdateScriptProps.Script)
            {
                CaptureParamValues(EmitterData->UpdateScriptProps.Script);
            }
        }
        
        // Switch to Stateless mode
        Handle->SetEmitterMode(*System, ENiagaraEmitterMode::Stateless);
        
        // Get the Stateless emitter and migrate parameters
        UNiagaraStatelessEmitter* StatelessEmitter = Handle->GetStatelessEmitter();
        if (StatelessEmitter)
        {
            TArray<TSharedPtr<FJsonValue>> MigratedParams;
            
            for (UNiagaraStatelessModule* Module : StatelessEmitter->GetModules())
            {
                if (!Module) continue;
                
                FString ModuleName = Module->GetName();
                FString ParamPattern;
                
                if (ModuleName == TEXT("Lifetime")) ParamPattern = TEXT("Particles.Lifetime");
                else if (ModuleName == TEXT("Color")) ParamPattern = TEXT("Particles.Color");
                else if (ModuleName == TEXT("Size")) ParamPattern = TEXT("Particles.SpriteSize");
                else if (ModuleName == TEXT("Scale")) ParamPattern = TEXT("Particles.Scale");
                else if (ModuleName == TEXT("Rotation")) ParamPattern = TEXT("Particles.SpriteRotation");
                else if (ModuleName == TEXT("Velocity")) ParamPattern = TEXT("Particles.Velocity");
                else if (ModuleName == TEXT("SpawnRate")) ParamPattern = TEXT("SpawnRate");
                else continue;
                
                // Migrate float values
                for (auto& Pair : FloatValues)
                {
                    if (Pair.Key.StartsWith(ParamPattern))
                    {
                        FProperty* Property = Module->GetClass()->FindPropertyByName(TEXT("Value"));
                        if (!Property)
                        {
                            if (ModuleName == TEXT("Lifetime")) Property = Module->GetClass()->FindPropertyByName(TEXT("Lifetime"));
                            else if (ModuleName == TEXT("SpawnRate")) Property = Module->GetClass()->FindPropertyByName(TEXT("SpawnRate"));
                        }
                        
                        if (Property)
                        {
                            if (FFloatProperty* FloatProp = CastField<FFloatProperty>(Property))
                            {
                                FloatProp->SetValue_InContainer(Module, Pair.Value);
                                
                                TSharedPtr<FJsonObject> Migrated = MakeShareable(new FJsonObject);
                                Migrated->SetStringField(TEXT("parameter"), Pair.Key);
                                Migrated->SetStringField(TEXT("module"), ModuleName);
                                Migrated->SetNumberField(TEXT("value"), Pair.Value);
                                MigratedParams.Add(MakeShareable(new FJsonValueObject(Migrated)));
                            }
                        }
                    }
                }
            }
            
            Result->SetBoolField(TEXT("success"), true);
            Result->SetStringField(TEXT("action"), TEXT("convert_to_stateless"));
            Result->SetStringField(TEXT("emitter_name"), EmitterName);
            Result->SetStringField(TEXT("new_mode"), TEXT("Stateless"));
            Result->SetArrayField(TEXT("migrated_parameters"), MigratedParams);
            Result->SetNumberField(TEXT("migrated_count"), MigratedParams.Num());
        }
        else
        {
            Result->SetBoolField(TEXT("success"), false);
            Result->SetStringField(TEXT("error"), TEXT("Failed to get StatelessEmitter after mode switch"));
        }
#else
        Result->SetBoolField(TEXT("success"), false);
        Result->SetStringField(TEXT("error"), TEXT("Stateless conversion requires UE 5.7 or later"));
#endif
    }
    else
    {
        Result->SetBoolField(TEXT("success"), false);
        Result->SetStringField(TEXT("error"), FString::Printf(TEXT("Unknown emitter action: %s"), *Action));
    }

    return Result;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPNiagaraCommands::ProcessRendererOperation(UNiagaraSystem* System, const TSharedPtr<FJsonObject>& Op)
{
    TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
    
    FString EmitterName = Op->GetStringField(TEXT("emitter"));
    FString Action = Op->GetStringField(TEXT("action"));
    
    FNiagaraEmitterHandle* Handle = FindEmitterHandle(System, EmitterName);
    if (!Handle)
    {
        Result->SetBoolField(TEXT("success"), false);
        Result->SetStringField(TEXT("error"), FString::Printf(TEXT("Emitter not found: %s"), *EmitterName));
        return Result;
    }
    
    FVersionedNiagaraEmitterData* EmitterData = Handle->GetEmitterData();
    if (!EmitterData)
    {
        Result->SetBoolField(TEXT("success"), false);
        Result->SetStringField(TEXT("error"), TEXT("Failed to get emitter data"));
        return Result;
    }
    
    int32 Index = 0;
    if (Op->HasField(TEXT("index")))
    {
        Index = (int32)Op->GetNumberField(TEXT("index"));
    }
    
    const TArray<UNiagaraRendererProperties*>& Renderers = EmitterData->GetRenderers();
    if (Index < 0 || Index >= Renderers.Num())
    {
        Result->SetBoolField(TEXT("success"), false);
        Result->SetStringField(TEXT("error"), FString::Printf(TEXT("Invalid renderer index: %d (valid: 0-%d)"), Index, Renderers.Num() - 1));
        return Result;
    }
    
    UNiagaraRendererProperties* Renderer = Renderers[Index];
    if (!Renderer)
    {
        Result->SetBoolField(TEXT("success"), false);
        Result->SetStringField(TEXT("error"), TEXT("Renderer is null"));
        return Result;
    }
    
    if (Action == TEXT("set_enabled"))
    {
        bool bEnabled = Op->GetBoolField(TEXT("value"));
        Renderer->SetIsEnabled(bEnabled);
        
        Result->SetBoolField(TEXT("success"), true);
        Result->SetStringField(TEXT("action"), TEXT("set_enabled"));
        Result->SetStringField(TEXT("emitter_name"), EmitterName);
        Result->SetNumberField(TEXT("renderer_index"), Index);
        Result->SetBoolField(TEXT("value"), bEnabled);
    }
    else
    {
        Result->SetBoolField(TEXT("success"), false);
        Result->SetStringField(TEXT("error"), FString::Printf(TEXT("Unknown renderer action: %s"), *Action));
    }
    
    return Result;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPNiagaraCommands::ProcessParameterOperation(UNiagaraSystem* System, const TSharedPtr<FJsonObject>& Op)
{
    TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
    
    FString EmitterName = Op->GetStringField(TEXT("emitter"));
    FString ScriptType = Op->GetStringField(TEXT("script")); // "spawn" or "update"
    FString ParamName = Op->GetStringField(TEXT("name"));
    
    FNiagaraEmitterHandle* Handle = FindEmitterHandle(System, EmitterName);
    if (!Handle)
    {
        Result->SetBoolField(TEXT("success"), false);
        Result->SetStringField(TEXT("error"), FString::Printf(TEXT("Emitter not found: %s"), *EmitterName));
        return Result;
    }
    
    FVersionedNiagaraEmitterData* EmitterData = Handle->GetEmitterData();
    if (!EmitterData)
    {
        Result->SetBoolField(TEXT("success"), false);
        Result->SetStringField(TEXT("error"), TEXT("Failed to get emitter data"));
        return Result;
    }
    
    UNiagaraScript* Script = nullptr;
    if (ScriptType.ToLower() == TEXT("spawn"))
    {
        Script = EmitterData->SpawnScriptProps.Script;
    }
    else if (ScriptType.ToLower() == TEXT("update"))
    {
        Script = EmitterData->UpdateScriptProps.Script;
    }
    
    if (!Script)
    {
        Result->SetBoolField(TEXT("success"), false);
        Result->SetStringField(TEXT("error"), FString::Printf(TEXT("Script not found: %s"), *ScriptType));
        return Result;
    }
    
    FNiagaraParameterStore& ParamStore = Script->RapidIterationParameters;
    
    // Find parameter by name
    TArrayView<const FNiagaraVariableWithOffset> ParamVariables = ParamStore.ReadParameterVariables();
    int32 FoundOffset = -1;
    FNiagaraTypeDefinition FoundType;
    
    for (const FNiagaraVariableWithOffset& ParamWithOffset : ParamVariables)
    {
        if (ParamWithOffset.GetName().ToString() == ParamName)
        {
            FoundOffset = ParamWithOffset.Offset;
            FoundType = ParamWithOffset.GetType();
            break;
        }
    }
    
    if (FoundOffset < 0)
    {
        Result->SetBoolField(TEXT("success"), false);
        Result->SetStringField(TEXT("error"), FString::Printf(TEXT("Parameter not found: %s"), *ParamName));
        return Result;
    }
    
    // Get value from operation
    const TSharedPtr<FJsonValue>* ValuePtr = Op->Values.Find(TEXT("value"));
    if (!ValuePtr)
    {
        Result->SetBoolField(TEXT("success"), false);
        Result->SetStringField(TEXT("error"), TEXT("Missing 'value' parameter"));
        return Result;
    }
    
    // Set value based on type using SetParameterData (public method)
    bool bSetSuccess = false;
    
    if (FoundType == FNiagaraTypeDefinition::GetFloatDef())
    {
        float Value = (float)(*ValuePtr)->AsNumber();
        ParamStore.SetParameterData(reinterpret_cast<const uint8*>(&Value), FoundOffset, sizeof(float));
        bSetSuccess = true;
        Result->SetNumberField(TEXT("value_set"), Value);
    }
    else if (FoundType == FNiagaraTypeDefinition::GetIntDef())
    {
        int32 Value = (int32)(*ValuePtr)->AsNumber();
        ParamStore.SetParameterData(reinterpret_cast<const uint8*>(&Value), FoundOffset, sizeof(int32));
        bSetSuccess = true;
        Result->SetNumberField(TEXT("value_set"), Value);
    }
    else if (FoundType == FNiagaraTypeDefinition::GetBoolDef())
    {
        bool Value = (*ValuePtr)->AsBool();
        ParamStore.SetParameterData(reinterpret_cast<const uint8*>(&Value), FoundOffset, sizeof(bool));
        bSetSuccess = true;
        Result->SetBoolField(TEXT("value_set"), Value);
    }
    else if (FoundType == FNiagaraTypeDefinition::GetVec3Def() || 
             FoundType == FNiagaraTypeDefinition::GetPositionDef())
    {
        const TArray<TSharedPtr<FJsonValue>>* VecArray;
        if ((*ValuePtr)->TryGetArray(VecArray) && VecArray->Num() >= 3)
        {
            FVector3f Value;
            Value.X = (float)(*VecArray)[0]->AsNumber();
            Value.Y = (float)(*VecArray)[1]->AsNumber();
            Value.Z = (float)(*VecArray)[2]->AsNumber();
            ParamStore.SetParameterData(reinterpret_cast<const uint8*>(&Value), FoundOffset, sizeof(FVector3f));
            bSetSuccess = true;
        }
    }
    else if (FoundType == FNiagaraTypeDefinition::GetVec4Def() ||
             FoundType == FNiagaraTypeDefinition::GetColorDef())
    {
        const TArray<TSharedPtr<FJsonValue>>* VecArray;
        if ((*ValuePtr)->TryGetArray(VecArray) && VecArray->Num() >= 4)
        {
            FVector4f Value;
            Value.X = (float)(*VecArray)[0]->AsNumber();
            Value.Y = (float)(*VecArray)[1]->AsNumber();
            Value.Z = (float)(*VecArray)[2]->AsNumber();
            Value.W = (float)(*VecArray)[3]->AsNumber();
            ParamStore.SetParameterData(reinterpret_cast<const uint8*>(&Value), FoundOffset, sizeof(FVector4f));
            bSetSuccess = true;
        }
    }
    else
    {
        Result->SetBoolField(TEXT("success"), false);
        Result->SetStringField(TEXT("error"), FString::Printf(TEXT("Unsupported parameter type: %s"), *FoundType.GetName()));
        return Result;
    }
    
    if (bSetSuccess)
    {
        Result->SetBoolField(TEXT("success"), true);
        Result->SetStringField(TEXT("action"), TEXT("set_parameter"));
        Result->SetStringField(TEXT("emitter_name"), EmitterName);
        Result->SetStringField(TEXT("script"), ScriptType);
        Result->SetStringField(TEXT("parameter_name"), ParamName);
        Result->SetStringField(TEXT("parameter_type"), FoundType.GetName());
    }
    else
    {
        Result->SetBoolField(TEXT("success"), false);
        Result->SetStringField(TEXT("error"), TEXT("Failed to set parameter value"));
    }
    
    return Result;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPNiagaraCommands::ProcessSimStageOperation(UNiagaraSystem* System, const TSharedPtr<FJsonObject>& Op)
{
    TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
    
    FString EmitterName = Op->GetStringField(TEXT("emitter"));
    FString StageName = Op->GetStringField(TEXT("name"));
    FString Action = Op->GetStringField(TEXT("action"));
    
    FNiagaraEmitterHandle* Handle = FindEmitterHandle(System, EmitterName);
    if (!Handle)
    {
        Result->SetBoolField(TEXT("success"), false);
        Result->SetStringField(TEXT("error"), FString::Printf(TEXT("Emitter not found: %s"), *EmitterName));
        return Result;
    }
    
    FVersionedNiagaraEmitterData* EmitterData = Handle->GetEmitterData();
    if (!EmitterData)
    {
        Result->SetBoolField(TEXT("success"), false);
        Result->SetStringField(TEXT("error"), TEXT("Failed to get emitter data"));
        return Result;
    }
    
    // Find simulation stage by name
    UNiagaraSimulationStageBase* FoundStage = nullptr;
    for (UNiagaraSimulationStageBase* Stage : EmitterData->GetSimulationStages())
    {
        if (Stage && Stage->GetName() == StageName)
        {
            FoundStage = Stage;
            break;
        }
    }
    
    if (!FoundStage)
    {
        Result->SetBoolField(TEXT("success"), false);
        Result->SetStringField(TEXT("error"), FString::Printf(TEXT("Simulation stage not found: %s"), *StageName));
        return Result;
    }
    
    if (Action == TEXT("set_enabled"))
    {
        bool bEnabled = Op->GetBoolField(TEXT("value"));
        FoundStage->bEnabled = bEnabled;
        
        Result->SetBoolField(TEXT("success"), true);
        Result->SetStringField(TEXT("action"), TEXT("set_enabled"));
        Result->SetStringField(TEXT("emitter_name"), EmitterName);
        Result->SetStringField(TEXT("stage_name"), StageName);
        Result->SetBoolField(TEXT("value"), bEnabled);
    }
    else
    {
        Result->SetBoolField(TEXT("success"), false);
        Result->SetStringField(TEXT("error"), FString::Printf(TEXT("Unknown sim_stage action: %s"), *Action));
    }
    
    return Result;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPNiagaraCommands::ProcessStatelessModuleOperation(UNiagaraSystem* System, const TSharedPtr<FJsonObject>& Op)
{
    TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
    
#if ENGINE_MAJOR_VERSION > 5 || (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 7)
    FString EmitterName = Op->GetStringField(TEXT("emitter"));
    FString ModuleName = Op->GetStringField(TEXT("name"));
    FString Action = Op->GetStringField(TEXT("action"));
    
    FNiagaraEmitterHandle* Handle = FindEmitterHandle(System, EmitterName);
    if (!Handle)
    {
        Result->SetBoolField(TEXT("success"), false);
        Result->SetStringField(TEXT("error"), FString::Printf(TEXT("Emitter not found: %s"), *EmitterName));
        return Result;
    }
    
    // Verify emitter is in Stateless mode
    if (Handle->GetEmitterMode() != ENiagaraEmitterMode::Stateless)
    {
        Result->SetBoolField(TEXT("success"), false);
        Result->SetStringField(TEXT("error"), TEXT("Emitter is not in Stateless mode"));
        return Result;
    }
    
    UNiagaraStatelessEmitter* StatelessEmitter = Handle->GetStatelessEmitter();
    if (!StatelessEmitter)
    {
        Result->SetBoolField(TEXT("success"), false);
        Result->SetStringField(TEXT("error"), TEXT("Failed to get StatelessEmitter"));
        return Result;
    }
    
    // Find module by name
    UNiagaraStatelessModule* FoundModule = nullptr;
    for (UNiagaraStatelessModule* Module : StatelessEmitter->GetModules())
    {
        if (Module && Module->GetName() == ModuleName)
        {
            FoundModule = Module;
            break;
        }
    }
    
    if (!FoundModule)
    {
        Result->SetBoolField(TEXT("success"), false);
        Result->SetStringField(TEXT("error"), FString::Printf(TEXT("Stateless module not found: %s"), *ModuleName));
        return Result;
    }
    
    if (Action == TEXT("set_enabled"))
    {
        bool bEnabled = Op->GetBoolField(TEXT("value"));
        FoundModule->SetIsModuleEnabled(bEnabled);
        
        Result->SetBoolField(TEXT("success"), true);
        Result->SetStringField(TEXT("action"), TEXT("set_enabled"));
        Result->SetStringField(TEXT("emitter_name"), EmitterName);
        Result->SetStringField(TEXT("module_name"), ModuleName);
        Result->SetBoolField(TEXT("value"), bEnabled);
    }
    else if (Action == TEXT("set_property"))
    {
        // Set module property via reflection
        FString PropertyName = Op->GetStringField(TEXT("property"));
        const TSharedPtr<FJsonValue>* ValuePtr = Op->Values.Find(TEXT("value"));
        
        if (!ValuePtr)
        {
            Result->SetBoolField(TEXT("success"), false);
            Result->SetStringField(TEXT("error"), TEXT("Missing 'value' parameter"));
            return Result;
        }
        
        // Find property by name
        FProperty* Property = FoundModule->GetClass()->FindPropertyByName(*PropertyName);
        if (!Property)
        {
            Result->SetBoolField(TEXT("success"), false);
            Result->SetStringField(TEXT("error"), FString::Printf(TEXT("Property not found: %s"), *PropertyName));
            return Result;
        }
        
        // Set value based on property type
        void* Container = FoundModule;
        
        if (FFloatProperty* FloatProp = CastField<FFloatProperty>(Property))
        {
            FloatProp->SetValue_InContainer(Container, (float)(*ValuePtr)->AsNumber());
        }
        else if (FIntProperty* IntProp = CastField<FIntProperty>(Property))
        {
            IntProp->SetValue_InContainer(Container, (int32)(*ValuePtr)->AsNumber());
        }
        else if (FBoolProperty* BoolProp = CastField<FBoolProperty>(Property))
        {
            bool BoolValue = (*ValuePtr)->AsBool();
            BoolProp->SetValue_InContainer(Container, &BoolValue);
        }
        else if (FStructProperty* StructProp = CastField<FStructProperty>(Property))
        {
            const TArray<TSharedPtr<FJsonValue>>* VecArray;
            if ((*ValuePtr)->TryGetArray(VecArray))
            {
                if (StructProp->Struct == TBaseStructure<FVector>::Get() && VecArray->Num() >= 3)
                {
                    FVector Value(
                        (float)(*VecArray)[0]->AsNumber(),
                        (float)(*VecArray)[1]->AsNumber(),
                        (float)(*VecArray)[2]->AsNumber()
                    );
                    StructProp->SetValue_InContainer(Container, &Value);
                }
                else if (StructProp->Struct == TBaseStructure<FVector4>::Get() && VecArray->Num() >= 4)
                {
                    FVector4 Value(
                        (float)(*VecArray)[0]->AsNumber(),
                        (float)(*VecArray)[1]->AsNumber(),
                        (float)(*VecArray)[2]->AsNumber(),
                        (float)(*VecArray)[3]->AsNumber()
                    );
                    StructProp->SetValue_InContainer(Container, &Value);
                }
                else if (StructProp->Struct == TBaseStructure<FLinearColor>::Get() && VecArray->Num() >= 4)
                {
                    FLinearColor Value(
                        (float)(*VecArray)[0]->AsNumber(),
                        (float)(*VecArray)[1]->AsNumber(),
                        (float)(*VecArray)[2]->AsNumber(),
                        (float)(*VecArray)[3]->AsNumber()
                    );
                    StructProp->SetValue_InContainer(Container, &Value);
                }
                else
                {
                    Result->SetBoolField(TEXT("success"), false);
                    Result->SetStringField(TEXT("error"), FString::Printf(TEXT("Unsupported struct type for property: %s"), *PropertyName));
                    return Result;
                }
            }
            else
            {
                Result->SetBoolField(TEXT("success"), false);
                Result->SetStringField(TEXT("error"), TEXT("Struct property requires array value"));
                return Result;
            }
        }
        else
        {
            Result->SetBoolField(TEXT("success"), false);
            Result->SetStringField(TEXT("error"), FString::Printf(TEXT("Unsupported property type: %s"), *Property->GetClass()->GetName()));
            return Result;
        }
        
        Result->SetBoolField(TEXT("success"), true);
        Result->SetStringField(TEXT("action"), TEXT("set_property"));
        Result->SetStringField(TEXT("emitter_name"), EmitterName);
        Result->SetStringField(TEXT("module_name"), ModuleName);
        Result->SetStringField(TEXT("property"), PropertyName);
    }
    else
    {
        Result->SetBoolField(TEXT("success"), false);
        Result->SetStringField(TEXT("error"), FString::Printf(TEXT("Unknown stateless_module action: %s"), *Action));
    }
#else
    Result->SetBoolField(TEXT("success"), false);
    Result->SetStringField(TEXT("error"), TEXT("Stateless Niagara requires UE 5.7 or later"));
#endif
    
    return Result;
}

// ============================================================================
// Utility Functions
// ============================================================================

TArray<FString> FEpicUnrealMCPNiagaraCommands::ParseIncludeSections(const TSharedPtr<FJsonObject>& Params)
{
    TArray<FString> IncludeSections;
    
    const TArray<TSharedPtr<FJsonValue>>* IncludeArray;
    if (Params->TryGetArrayField(TEXT("include"), IncludeArray))
    {
        for (const auto& Val : *IncludeArray)
        {
            IncludeSections.Add(Val->AsString().ToLower());
        }
    }
    else
    {
        IncludeSections.Add(TEXT("all"));
    }
    
    return IncludeSections;
}

bool FEpicUnrealMCPNiagaraCommands::ShouldInclude(const TArray<FString>& IncludeSections, const FString& Section)
{
    if (IncludeSections.Contains(TEXT("all")))
    {
        return true;
    }
    return IncludeSections.Contains(Section.ToLower());
}

// ============================================================================
// Stateless Conversion Implementation
// ============================================================================

/**
 * Standard Parameter Name -> Stateless Module mapping
 * 
 * Standard Niagara parameters follow naming conventions:
 * - "Particles.Lifetime" -> Stateless Lifetime module
 * - "Engine.ExecutionState" -> not convertible
 * 
 * This mapping identifies convertible parameter patterns.
 */
struct FStatelessModuleMapping
{
    FString ParameterPrefix;        // e.g., "Particles.Lifetime"
    FString StatelessModuleClass;   // e.g., "NiagaraStatelessLifetimeModule"
    FString DisplayName;            // e.g., "Lifetime"
    int32 Priority;                 // Higher = more specific match
};

// Static mapping table for Standard -> Stateless conversion
static const TArray<FStatelessModuleMapping>& GetStatelessModuleMappings()
{
    static TArray<FStatelessModuleMapping> Mappings;
    if (Mappings.Num() == 0)
    {
        // Particle attribute modules
        Mappings.Add({"Particles.Lifetime", "NiagaraStatelessLifetimeModule", "Lifetime", 100});
        Mappings.Add({"Particles.SpriteSize", "NiagaraStatelessSizeModule", "Size", 100});
        Mappings.Add({"Particles.SpriteRotation", "NiagaraStatelessRotationModule", "Rotation", 100});
        Mappings.Add({"Particles.Color", "NiagaraStatelessColorModule", "Color", 100});
        Mappings.Add({"Particles.Position", "NiagaraStatelessPositionModule", "Position", 90});
        Mappings.Add({"Particles.Velocity", "NiagaraStatelessVelocityModule", "Velocity", 100});
        Mappings.Add({"Particles.Scale", "NiagaraStatelessScaleModule", "Scale", 100});
        Mappings.Add({"Particles.MeshRotation", "NiagaraStatelessMeshRotationModule", "MeshRotation", 100});
        Mappings.Add({"Particles.SubUV", "NiagaraStatelessSubUVModule", "SubUV", 100});
        
        // Spawn modules
        Mappings.Add({"SpawnRate", "NiagaraStatelessSpawnRateModule", "SpawnRate", 100});
        Mappings.Add({"SpawnPerUnit", "NiagaraStatelessSpawnPerUnitModule", "SpawnPerUnit", 100});
        
        // Force/Physics modules
        Mappings.Add({"CurlNoise", "NiagaraStatelessCurlNoiseModule", "CurlNoise", 100});
        Mappings.Add({"Drag", "NiagaraStatelessDragModule", "Drag", 100});
        Mappings.Add({"Collision", "NiagaraStatelessCollisionModule", "Collision", 100});
        Mappings.Add({"Attractor", "NiagaraStatelessAttractorModule", "Attractor", 100});
    }
    return Mappings;
}

/**
 * Check if a parameter is a system/engine parameter (not convertible)
 */
static bool IsSystemParameter(const FString& ParamName)
{
    return ParamName.StartsWith("Engine.") ||
           ParamName.StartsWith("System.") ||
           ParamName.StartsWith("Emitter.") ||
           ParamName.Contains("ExecutionState") ||
           ParamName.Contains("LoopCount") ||
           ParamName.Contains("Duration");
}

/**
 * Find matching Stateless module for a parameter name
 */
static const FStatelessModuleMapping* FindStatelessMapping(const FString& ParamName)
{
    const TArray<FStatelessModuleMapping>& Mappings = GetStatelessModuleMappings();
    const FStatelessModuleMapping* BestMatch = nullptr;
    
    for (const FStatelessModuleMapping& Mapping : Mappings)
    {
        if (ParamName.StartsWith(Mapping.ParameterPrefix) || ParamName == Mapping.ParameterPrefix)
        {
            if (!BestMatch || Mapping.Priority > BestMatch->Priority)
            {
                BestMatch = &Mapping;
            }
        }
    }
    
    return BestMatch;
}

/**
 * Standard Module Node Class -> Stateless Module mapping
 * 
 * Map Niagara Graph node class names to Stateless module equivalents.
 * E.g., "NiagaraNodeModule_Lifetime" -> "NiagaraStatelessLifetimeModule"
 */
struct FStandardModuleMapping
{
    FString NodeClassPattern;       // Pattern to match in node class name
    FString StatelessModuleClass;   // Corresponding Stateless module class
    FString DisplayName;            // Human-readable name
    bool bIsSupported;              // Whether conversion is supported
};

static const TArray<FStandardModuleMapping>& GetStandardModuleMappings()
{
    static TArray<FStandardModuleMapping> Mappings;
    if (Mappings.Num() == 0)
    {
        // Particle attribute modules - supported
        Mappings.Add({"Lifetime", "NiagaraStatelessLifetimeModule", "Lifetime", true});
        Mappings.Add({"Color", "NiagaraStatelessColorModule", "Color", true});
        Mappings.Add({"SpriteSize", "NiagaraStatelessSizeModule", "Size", true});
        Mappings.Add({"Size", "NiagaraStatelessSizeModule", "Size", true});
        Mappings.Add({"Scale", "NiagaraStatelessScaleModule", "Scale", true});
        Mappings.Add({"SpriteRotation", "NiagaraStatelessRotationModule", "Rotation", true});
        Mappings.Add({"Rotation", "NiagaraStatelessRotationModule", "Rotation", true});
        Mappings.Add({"Velocity", "NiagaraStatelessVelocityModule", "Velocity", true});
        Mappings.Add({"AddVelocity", "NiagaraStatelessVelocityModule", "Velocity", true});
        Mappings.Add({"Position", "NiagaraStatelessPositionModule", "Position", true});
        Mappings.Add({"AddPosition", "NiagaraStatelessPositionModule", "Position", true});
        Mappings.Add({"MeshRotation", "NiagaraStatelessMeshRotationModule", "MeshRotation", true});
        Mappings.Add({"SubUV", "NiagaraStatelessSubUVModule", "SubUV", true});
        
        // Spawn modules - supported
        Mappings.Add({"SpawnRate", "NiagaraStatelessSpawnRateModule", "SpawnRate", true});
        Mappings.Add({"SpawnPerUnit", "NiagaraStatelessSpawnPerUnitModule", "SpawnPerUnit", true});
        
        // Force/Physics modules - supported
        Mappings.Add({"CurlNoise", "NiagaraStatelessCurlNoiseModule", "CurlNoise", true});
        Mappings.Add({"Drag", "NiagaraStatelessDragModule", "Drag", true});
        Mappings.Add({"Collision", "NiagaraStatelessCollisionModule", "Collision", true});
        Mappings.Add({"Attractor", "NiagaraStatelessAttractorModule", "Attractor", true});
        
        // Common modules that need special handling
        Mappings.Add({"Initialize", "", "Initialize", false});  // Usually contains multiple attributes
        Mappings.Add({"Mesh", "NiagaraStatelessMeshModule", "Mesh", true});
        Mappings.Add({"Material", "NiagaraStatelessMaterialModule", "Material", true});
        
        // Unsupported modules
        Mappings.Add({"Event", "", "Event", false});  // Events not supported
        Mappings.Add({"Ribbon", "", "Ribbon", false});  // Ribbon renderer not supported
        Mappings.Add({"Light", "NiagaraStatelessLightModule", "Light", true});
    }
    return Mappings;
}

/**
 * Find matching Stateless module for a Graph node class
 */
static const FStandardModuleMapping* FindStandardModuleMapping(const FString& NodeClass)
{
    const TArray<FStandardModuleMapping>& Mappings = GetStandardModuleMappings();
    const FStandardModuleMapping* BestMatch = nullptr;
    
    for (const FStandardModuleMapping& Mapping : Mappings)
    {
        if (NodeClass.Contains(Mapping.NodeClassPattern))
        {
            // Prefer longer/more specific matches
            if (!BestMatch || Mapping.NodeClassPattern.Len() > BestMatch->NodeClassPattern.Len())
            {
                BestMatch = &Mapping;
            }
        }
    }
    
    return BestMatch;
}

// ============================================================================
// Stateless Helper Functions (Internal)
// ============================================================================

TSharedPtr<FJsonObject> FEpicUnrealMCPNiagaraCommands::AnalyzeStatelessCompatibility(FNiagaraEmitterHandle& Handle)
{
    TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
    
    FString EmitterName = Handle.GetName().ToString();
    
    // Check if already Stateless
    if (Handle.GetEmitterMode() == ENiagaraEmitterMode::Stateless)
    {
        Result->SetBoolField(TEXT("is_stateless"), true);
        Result->SetStringField(TEXT("message"), TEXT("Emitter is already in Stateless mode"));
        return Result;
    }
    
    FVersionedNiagaraEmitterData* EmitterData = Handle.GetEmitterData();
    if (!EmitterData)
    {
        Result->SetBoolField(TEXT("error"), true);
        Result->SetStringField(TEXT("message"), TEXT("Failed to get emitter data"));
        return Result;
    }
    
    // Analyze compatibility
    TArray<TSharedPtr<FJsonValue>> ConvertibleParams;
    TArray<TSharedPtr<FJsonValue>> UnsupportedParams;
    TArray<TSharedPtr<FJsonValue>> Blockers;
    TMap<FString, bool> FoundModules;  // Track unique modules needed
    
    // Check for blockers (features that prevent conversion)
    
    // 1. Event handlers
    const TArray<FNiagaraEventScriptProperties>& EventHandlers = EmitterData->GetEventHandlers();
    if (EventHandlers.Num() > 0)
    {
        TSharedPtr<FJsonObject> Blocker = MakeShareable(new FJsonObject);
        Blocker->SetStringField(TEXT("type"), TEXT("event_handlers"));
        Blocker->SetStringField(TEXT("reason"), TEXT("Stateless mode does not support event handlers"));
        Blocker->SetNumberField(TEXT("count"), EventHandlers.Num());
        Blockers.Add(MakeShareable(new FJsonValueObject(Blocker)));
    }
    
    // 2. Simulation stages (beyond basic spawn/update)
    TArray<UNiagaraSimulationStageBase*> SimStages = EmitterData->GetSimulationStages();
    if (SimStages.Num() > 0)
    {
        TSharedPtr<FJsonObject> Blocker = MakeShareable(new FJsonObject);
        Blocker->SetStringField(TEXT("type"), TEXT("simulation_stages"));
        Blocker->SetStringField(TEXT("reason"), TEXT("Custom simulation stages may not be supported in Stateless mode"));
        Blocker->SetNumberField(TEXT("count"), SimStages.Num());
        Blockers.Add(MakeShareable(new FJsonValueObject(Blocker)));
    }
    
    // 3. Analyze Graph modules
    TArray<TSharedPtr<FJsonValue>> ConvertibleModules;
    TArray<TSharedPtr<FJsonValue>> UnsupportedModules;
    TSet<FString> FoundModuleNames;
    
    auto AnalyzeGraphModules = [&](UNiagaraScript* Script, const FString& ScriptType)
    {
        if (!Script) return;
        
        UNiagaraScriptSource* Source = Cast<UNiagaraScriptSource>(Script->GetLatestSource());
        UNiagaraGraph* Graph = Source ? Source->NodeGraph : nullptr;
        if (!Graph) return;
        
        for (UEdGraphNode* NodeBase : Graph->Nodes)
        {
            UNiagaraNode* Node = Cast<UNiagaraNode>(NodeBase);
            if (!Node) continue;
            
            FString NodeClass = Node->GetClass()->GetName();
            
            // Skip output/input/system nodes
            if (NodeClass.Contains(TEXT("Output")) || 
                NodeClass.Contains(TEXT("Input")) ||
                NodeClass.Contains(TEXT("System")) ||
                NodeClass.Contains(TEXT("Emitter")))
            {
                continue;
            }
            
            // Check if it's a module node
            if (!NodeClass.Contains(TEXT("Module"))) continue;
            
            FString ModuleName = Node->GetName();
            if (FoundModuleNames.Contains(ModuleName)) continue;
            FoundModuleNames.Add(ModuleName);
            
            // Find mapping
            const FStandardModuleMapping* Mapping = FindStandardModuleMapping(NodeClass);
            
            TSharedPtr<FJsonObject> ModuleInfo = MakeShareable(new FJsonObject);
            ModuleInfo->SetStringField(TEXT("name"), ModuleName);
            ModuleInfo->SetStringField(TEXT("node_class"), NodeClass);
            ModuleInfo->SetStringField(TEXT("script"), ScriptType);
            
            if (Mapping)
            {
                ModuleInfo->SetStringField(TEXT("stateless_module"), Mapping->DisplayName);
                ModuleInfo->SetBoolField(TEXT("supported"), Mapping->bIsSupported);
                if (Mapping->bIsSupported)
                {
                    ConvertibleModules.Add(MakeShareable(new FJsonValueObject(ModuleInfo)));
                    FoundModules.FindOrAdd(Mapping->DisplayName) = true;
                }
                else
                {
                    ModuleInfo->SetStringField(TEXT("reason"), TEXT("Module not supported in Stateless mode"));
                    UnsupportedModules.Add(MakeShareable(new FJsonValueObject(ModuleInfo)));
                }
            }
            else
            {
                ModuleInfo->SetBoolField(TEXT("supported"), false);
                ModuleInfo->SetStringField(TEXT("reason"), TEXT("No matching Stateless module found"));
                UnsupportedModules.Add(MakeShareable(new FJsonValueObject(ModuleInfo)));
            }
        }
    };
    
    // Analyze spawn and update script graphs
    if (EmitterData->SpawnScriptProps.Script)
    {
        AnalyzeGraphModules(EmitterData->SpawnScriptProps.Script, TEXT("spawn"));
    }
    if (EmitterData->UpdateScriptProps.Script)
    {
        AnalyzeGraphModules(EmitterData->UpdateScriptProps.Script, TEXT("update"));
    }
    
    // 4. Also analyze script parameters (for value migration)
    auto AnalyzeScriptParams = [&](UNiagaraScript* Script, const FString& ScriptType)
    {
        if (!Script) return;
        
        const FNiagaraParameterStore& ParamStore = Script->RapidIterationParameters;
        TArrayView<const FNiagaraVariableWithOffset> ParamVariables = ParamStore.ReadParameterVariables();
        
        for (const FNiagaraVariableWithOffset& ParamWithOffset : ParamVariables)
        {
            FString ParamName = ParamWithOffset.GetName().ToString();
            
            // Skip system parameters
            if (IsSystemParameter(ParamName))
            {
                continue;
            }
            
            // Find matching Stateless module
            const FStatelessModuleMapping* Mapping = FindStatelessMapping(ParamName);
            
            TSharedPtr<FJsonObject> ParamInfo = MakeShareable(new FJsonObject);
            ParamInfo->SetStringField(TEXT("name"), ParamName);
            ParamInfo->SetStringField(TEXT("script"), ScriptType);
            ParamInfo->SetStringField(TEXT("type"), ParamWithOffset.GetType().GetName());
            
            if (Mapping)
            {
                ParamInfo->SetStringField(TEXT("stateless_module"), Mapping->DisplayName);
                ParamInfo->SetBoolField(TEXT("convertible"), true);
                ConvertibleParams.Add(MakeShareable(new FJsonValueObject(ParamInfo)));
            }
            else
            {
                ParamInfo->SetBoolField(TEXT("convertible"), false);
                ParamInfo->SetStringField(TEXT("reason"), TEXT("No matching Stateless module"));
                UnsupportedParams.Add(MakeShareable(new FJsonValueObject(ParamInfo)));
            }
        }
    };
    
    if (EmitterData->SpawnScriptProps.Script)
    {
        AnalyzeScriptParams(EmitterData->SpawnScriptProps.Script, TEXT("spawn"));
    }
    if (EmitterData->UpdateScriptProps.Script)
    {
        AnalyzeScriptParams(EmitterData->UpdateScriptProps.Script, TEXT("update"));
    }
    
    // Build result
    Result->SetBoolField(TEXT("success"), true);
    Result->SetStringField(TEXT("emitter_name"), EmitterName);
    Result->SetStringField(TEXT("current_mode"), TEXT("Standard"));
    
    // Compatibility assessment
    bool bHasBlockers = Blockers.Num() > 0;
    bool bHasUnsupportedModules = UnsupportedModules.Num() > 0;
    bool bCanConvert = !bHasBlockers && ConvertibleModules.Num() > 0 && !bHasUnsupportedModules;
    
    Result->SetBoolField(TEXT("can_convert"), bCanConvert);
    Result->SetBoolField(TEXT("has_blockers"), bHasBlockers);
    
    // Convertible modules needed
    TArray<TSharedPtr<FJsonValue>> RequiredModules;
    for (auto& Pair : FoundModules)
    {
        RequiredModules.Add(MakeShareable(new FJsonValueString(Pair.Key)));
    }
    Result->SetArrayField(TEXT("required_stateless_modules"), RequiredModules);
    
    // Module analysis results
    Result->SetArrayField(TEXT("convertible_modules"), ConvertibleModules);
    Result->SetNumberField(TEXT("convertible_module_count"), ConvertibleModules.Num());
    Result->SetArrayField(TEXT("unsupported_modules"), UnsupportedModules);
    Result->SetNumberField(TEXT("unsupported_module_count"), UnsupportedModules.Num());
    
    // Parameter analysis results
    Result->SetArrayField(TEXT("convertible_parameters"), ConvertibleParams);
    Result->SetNumberField(TEXT("convertible_parameter_count"), ConvertibleParams.Num());
    Result->SetArrayField(TEXT("unsupported_parameters"), UnsupportedParams);
    Result->SetNumberField(TEXT("unsupported_parameter_count"), UnsupportedParams.Num());
    
    Result->SetArrayField(TEXT("blockers"), Blockers);
    
    // Summary message
    FString Message;
    if (bCanConvert)
    {
        Message = FString::Printf(TEXT("Convertible: %d modules and %d parameters can be migrated to Stateless"), 
            ConvertibleModules.Num(), ConvertibleParams.Num());
    }
    else if (bHasBlockers)
    {
        Message = FString::Printf(TEXT("Not convertible: %d blocker(s) found"), Blockers.Num());
    }
    else if (bHasUnsupportedModules)
    {
        Message = FString::Printf(TEXT("Not convertible: %d unsupported module(s)"), UnsupportedModules.Num());
    }
    else
    {
        Message = TEXT("Not convertible: No convertible modules found");
    }
    Result->SetStringField(TEXT("message"), Message);
    
    return Result;
}

// ============================================================================
// Graph Form Tools - Read/Update Niagara Script Graphs
// ============================================================================

TSharedPtr<FJsonObject> FEpicUnrealMCPNiagaraCommands::HandleGetNiagaraGraph(const TSharedPtr<FJsonObject>& Params)
{
    TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
    
    // Method 1: Direct script path for standalone assets
    FString ScriptPath;
    bool bUseStandaloneScript = Params->TryGetStringField(TEXT("script_path"), ScriptPath);
    
    // Method 2: Locate script within System's Emitter
    FString AssetPath;
    FString EmitterName;
    FString ScriptType = TEXT("spawn");
    
    if (!bUseStandaloneScript)
    {
        if (!Params->TryGetStringField(TEXT("asset_path"), AssetPath))
        {
            Result->SetBoolField(TEXT("success"), false);
            Result->SetStringField(TEXT("error"), TEXT("Must provide either (asset_path + emitter) or script_path"));
            return Result;
        }
        
        if (!Params->TryGetStringField(TEXT("emitter"), EmitterName))
        {
            Result->SetBoolField(TEXT("success"), false);
            Result->SetStringField(TEXT("error"), TEXT("Must provide either (asset_path + emitter) or script_path"));
            return Result;
        }
        
        Params->TryGetStringField(TEXT("script"), ScriptType);
    }
    
    // Optional: filter by module name
    FString ModuleName;
    Params->TryGetStringField(TEXT("module"), ModuleName);
    
    // Get the script
    UNiagaraScript* Script = nullptr;
    FString SourceLocation;  // For result metadata
    
    if (bUseStandaloneScript)
    {
        // Load standalone script asset
        Script = LoadObject<UNiagaraScript>(nullptr, *ScriptPath);
        if (!Script)
        {
            Result->SetBoolField(TEXT("success"), false);
            Result->SetStringField(TEXT("error"), FString::Printf(TEXT("Failed to load Niagara script: %s"), *ScriptPath));
            return Result;
        }
        SourceLocation = ScriptPath;
    }
    else
    {
        // Load script from System's Emitter
        UNiagaraSystem* NiagaraSystem = LoadNiagaraSystemAsset(AssetPath);
        if (!NiagaraSystem)
        {
            Result->SetBoolField(TEXT("success"), false);
            Result->SetStringField(TEXT("error"), FString::Printf(TEXT("Failed to load Niagara system: %s"), *AssetPath));
            return Result;
        }
        
        FNiagaraEmitterHandle* Handle = FindEmitterHandle(NiagaraSystem, EmitterName);
        if (!Handle)
        {
            Result->SetBoolField(TEXT("success"), false);
            Result->SetStringField(TEXT("error"), FString::Printf(TEXT("Emitter not found: %s"), *EmitterName));
            return Result;
        }
        
        FVersionedNiagaraEmitterData* EmitterData = Handle->GetEmitterData();
        if (!EmitterData)
        {
            Result->SetBoolField(TEXT("success"), false);
            Result->SetStringField(TEXT("error"), TEXT("Failed to get emitter data"));
            return Result;
        }
        
        if (ScriptType.ToLower() == TEXT("spawn"))
        {
            Script = EmitterData->SpawnScriptProps.Script;
        }
        else if (ScriptType.ToLower() == TEXT("update"))
        {
            Script = EmitterData->UpdateScriptProps.Script;
        }
        
        if (!Script)
        {
            Result->SetBoolField(TEXT("success"), false);
            Result->SetStringField(TEXT("error"), FString::Printf(TEXT("Script not found: %s"), *ScriptType));
            return Result;
        }
        
        SourceLocation = FString::Printf(TEXT("%s:%s:%s"), *AssetPath, *EmitterName, *ScriptType);
    }
    
    // Extract graph using helper function
    return ExtractGraphFromScript(Script, ModuleName);
}

TSharedPtr<FJsonObject> FEpicUnrealMCPNiagaraCommands::ExtractGraphFromScript(UNiagaraScript* Script, const FString& ModuleFilter)
{
    TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
    
    if (!Script)
    {
        Result->SetBoolField(TEXT("success"), false);
        Result->SetStringField(TEXT("error"), TEXT("Script is null"));
        return Result;
    }
    
    // Get script metadata
    Result->SetBoolField(TEXT("success"), true);
    Result->SetStringField(TEXT("script_name"), Script->GetName());
    
    // Get script usage
    FString UsageStr = TEXT("Unknown");
    switch (Script->GetUsage())
    {
        case ENiagaraScriptUsage::Function: UsageStr = TEXT("Function"); break;
        case ENiagaraScriptUsage::Module: UsageStr = TEXT("Module"); break;
        case ENiagaraScriptUsage::DynamicInput: UsageStr = TEXT("DynamicInput"); break;
        case ENiagaraScriptUsage::ParticleSpawnScript: UsageStr = TEXT("ParticleSpawn"); break;
        case ENiagaraScriptUsage::ParticleUpdateScript: UsageStr = TEXT("ParticleUpdate"); break;
        case ENiagaraScriptUsage::EmitterSpawnScript: UsageStr = TEXT("EmitterSpawn"); break;
        case ENiagaraScriptUsage::EmitterUpdateScript: UsageStr = TEXT("EmitterUpdate"); break;
        case ENiagaraScriptUsage::SystemSpawnScript: UsageStr = TEXT("SystemSpawn"); break;
        case ENiagaraScriptUsage::SystemUpdateScript: UsageStr = TEXT("SystemUpdate"); break;
        case ENiagaraScriptUsage::ParticleEventScript: UsageStr = TEXT("ParticleEvent"); break;
        default: break;
    }
    Result->SetStringField(TEXT("usage"), UsageStr);
    
    // Get description
    Result->SetStringField(TEXT("description"), Script->GetDescription(FGuid()).ToString());
    
    UNiagaraScriptSource* Source = Cast<UNiagaraScriptSource>(Script->GetLatestSource());
    UNiagaraGraph* Graph = Source ? Source->NodeGraph : nullptr;
    if (!Graph)
    {
        Result->SetBoolField(TEXT("has_graph"), false);
        return Result;
    }
    
    Result->SetBoolField(TEXT("has_graph"), true);
    
    // Build node ID map
    TMap<UEdGraphNode*, FString> NodeIdMap;
    int32 NodeIndex = 0;
    for (UEdGraphNode* NodeBase : Graph->Nodes)
    {
        if (NodeBase)
        {
            FString NodeId = FString::Printf(TEXT("Node_%d"), NodeIndex++);
            NodeIdMap.Add(NodeBase, NodeId);
        }
    }
    
    // Build nodes array
    TArray<TSharedPtr<FJsonValue>> NodesArray;
    TArray<TSharedPtr<FJsonValue>> ConnectionsArray;
    
    for (UEdGraphNode* NodeBase : Graph->Nodes)
    {
        UNiagaraNode* Node = Cast<UNiagaraNode>(NodeBase);
        if (!Node) continue;
        
        // Optional: filter by module name
        if (!ModuleFilter.IsEmpty())
        {
            FString NodeName = Node->GetName();
            if (!NodeName.Contains(ModuleFilter))
            {
                continue;
            }
        }
        
        // Get node details
        TSharedPtr<FJsonObject> NodeJson = GetNodeDetails(Node);
        FString NodeId = NodeIdMap[Node];
        NodeJson->SetStringField(TEXT("node_id"), NodeId);
        NodesArray.Add(MakeShareable(new FJsonValueObject(NodeJson)));
        
        // Get connections from this node
        TArray<TSharedPtr<FJsonValue>> NodeConnections = GetNodeConnections(Node, NodeIdMap);
        for (const auto& Conn : NodeConnections)
        {
            ConnectionsArray.Add(Conn);
        }
    }
    
    Result->SetArrayField(TEXT("nodes"), NodesArray);
    Result->SetNumberField(TEXT("node_count"), NodesArray.Num());
    Result->SetArrayField(TEXT("connections"), ConnectionsArray);
    Result->SetNumberField(TEXT("connection_count"), ConnectionsArray.Num());
    
    // Get rapid iteration parameters
    TArray<TSharedPtr<FJsonValue>> ParametersArray;
    const FNiagaraParameterStore& ParamStore = Script->RapidIterationParameters;
    TArrayView<const FNiagaraVariableWithOffset> ParamVariables = ParamStore.ReadParameterVariables();
    
    for (const FNiagaraVariableWithOffset& ParamWithOffset : ParamVariables)
    {
        TSharedPtr<FJsonObject> ParamJson = MakeShareable(new FJsonObject);
        ParamJson->SetStringField(TEXT("name"), ParamWithOffset.GetName().ToString());
        ParamJson->SetStringField(TEXT("type"), ParamWithOffset.GetType().GetName());
        ParametersArray.Add(MakeShareable(new FJsonValueObject(ParamJson)));
    }
    
    Result->SetArrayField(TEXT("parameters"), ParametersArray);
    Result->SetNumberField(TEXT("parameter_count"), ParametersArray.Num());
    
    return Result;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPNiagaraCommands::GetNodeDetails(UNiagaraNode* Node)
{
    TSharedPtr<FJsonObject> NodeJson = MakeShareable(new FJsonObject);
    
    if (!Node)
    {
        return NodeJson;
    }
    
    // Basic info
    FString NodeClass = Node->GetClass()->GetName();
    NodeClass.RemoveFromStart(TEXT("NiagaraNode"));
    NodeJson->SetStringField(TEXT("type"), NodeClass);
    NodeJson->SetStringField(TEXT("name"), Node->GetName());
    NodeJson->SetStringField(TEXT("class"), Node->GetClass()->GetName());
    
    // Position
    NodeJson->SetNumberField(TEXT("pos_x"), Node->NodePosX);
    NodeJson->SetNumberField(TEXT("pos_y"), Node->NodePosY);
    
    // Node-specific info
    FString NodeType = Node->GetClass()->GetName();
    
    // Input node - parameter input
    if (UNiagaraNodeInput* InputNode = Cast<UNiagaraNodeInput>(Node))
    {
        NodeJson->SetStringField(TEXT("usage"), TEXT("input"));
        NodeJson->SetStringField(TEXT("input_name"), InputNode->Input.GetName().ToString());
        NodeJson->SetStringField(TEXT("input_type"), InputNode->Input.GetType().GetName());
        
        // Note: Default value extraction removed due to UE 5.7 API changes
        // FNiagaraVariable::GetVars() no longer available
    }
    // Output node
    else if (UNiagaraNodeOutput* OutputNode = Cast<UNiagaraNodeOutput>(Node))
    {
        NodeJson->SetStringField(TEXT("usage"), TEXT("output"));
    }
    // CustomHlsl node (must check before FunctionCall since CustomHlsl inherits from it)
    else if (UNiagaraNodeCustomHlsl* CustomHlslNode = Cast<UNiagaraNodeCustomHlsl>(Node))
    {
        NodeJson->SetStringField(TEXT("usage"), TEXT("custom_hlsl"));
        
        // Get HLSL code using reflection (CustomHlsl is private in MinimalAPI class)
        if (FStrProperty* CustomHlslProperty = CastField<FStrProperty>(CustomHlslNode->GetClass()->FindPropertyByName(TEXT("CustomHlsl"))))
        {
            FString CustomHlslValue = CustomHlslProperty->GetPropertyValue_InContainer(CustomHlslNode);
            NodeJson->SetStringField(TEXT("hlsl_code"), CustomHlslValue);
        }
        
        // Get script usage name using enum reflection
        UEnum* ScriptUsageEnum = StaticEnum<ENiagaraScriptUsage>();
        if (ScriptUsageEnum)
        {
            FString ScriptUsageName = ScriptUsageEnum->GetNameStringByValue((int64)CustomHlslNode->ScriptUsage);
            NodeJson->SetStringField(TEXT("script_usage"), ScriptUsageName);
        }
    }
    // Function call node
    else if (UNiagaraNodeFunctionCall* FuncNode = Cast<UNiagaraNodeFunctionCall>(Node))
    {
        NodeJson->SetStringField(TEXT("usage"), TEXT("function_call"));
        
        // Get function name
        if (FuncNode->FunctionScript)
        {
            NodeJson->SetStringField(TEXT("function_name"), FuncNode->FunctionScript->GetName());
            NodeJson->SetStringField(TEXT("function_path"), FuncNode->FunctionScript->GetPathName());
        }
        else
        {
            NodeJson->SetStringField(TEXT("function_name"), FuncNode->GetName());
        }
    }
    // Operator node
    else if (UNiagaraNodeOp* OpNode = Cast<UNiagaraNodeOp>(Node))
    {
        NodeJson->SetStringField(TEXT("usage"), TEXT("operator"));
        NodeJson->SetStringField(TEXT("operator"), OpNode->OpName.ToString());
    }
    // Generic module node
    else if (NodeType.Contains(TEXT("Module")))
    {
        NodeJson->SetStringField(TEXT("usage"), TEXT("module"));
    }
    
    // Get input pins info
    TArray<TSharedPtr<FJsonValue>> InputPinsArray;
    for (UEdGraphPin* Pin : Node->Pins)
    {
        if (Pin && Pin->Direction == EGPD_Input && !Pin->bHidden)
        {
            TSharedPtr<FJsonObject> PinJson = MakeShareable(new FJsonObject);
            PinJson->SetStringField(TEXT("name"), Pin->PinName.ToString());
            PinJson->SetStringField(TEXT("type"), Pin->PinType.PinCategory.ToString());
            PinJson->SetBoolField(TEXT("connected"), Pin->LinkedTo.Num() > 0);
            InputPinsArray.Add(MakeShareable(new FJsonValueObject(PinJson)));
        }
    }
    if (InputPinsArray.Num() > 0)
    {
        NodeJson->SetArrayField(TEXT("input_pins"), InputPinsArray);
    }
    
    // Get output pins info
    TArray<TSharedPtr<FJsonValue>> OutputPinsArray;
    for (UEdGraphPin* Pin : Node->Pins)
    {
        if (Pin && Pin->Direction == EGPD_Output && !Pin->bHidden)
        {
            TSharedPtr<FJsonObject> PinJson = MakeShareable(new FJsonObject);
            PinJson->SetStringField(TEXT("name"), Pin->PinName.ToString());
            PinJson->SetStringField(TEXT("type"), Pin->PinType.PinCategory.ToString());
            PinJson->SetNumberField(TEXT("connections"), Pin->LinkedTo.Num());
            OutputPinsArray.Add(MakeShareable(new FJsonValueObject(PinJson)));
        }
    }
    if (OutputPinsArray.Num() > 0)
    {
        NodeJson->SetArrayField(TEXT("output_pins"), OutputPinsArray);
    }
    
    return NodeJson;
}

TArray<TSharedPtr<FJsonValue>> FEpicUnrealMCPNiagaraCommands::GetNodeConnections(UNiagaraNode* Node, const TMap<UEdGraphNode*, FString>& NodeIdMap)
{
    TArray<TSharedPtr<FJsonValue>> Connections;
    
    if (!Node)
    {
        return Connections;
    }
    
    FString SourceNodeId = NodeIdMap.FindRef(Node);
    if (SourceNodeId.IsEmpty())
    {
        return Connections;
    }
    
    // Get output pins and their connections
    for (UEdGraphPin* OutputPin : Node->Pins)
    {
        if (!OutputPin || OutputPin->Direction != EGPD_Output || OutputPin->bHidden) continue;
        
        for (UEdGraphPin* ConnectedPin : OutputPin->LinkedTo)
        {
            if (!ConnectedPin) continue;
            
            UEdGraphNode* TargetNode = ConnectedPin->GetOwningNode();
            if (!TargetNode) continue;
            
            const FString* TargetNodeId = NodeIdMap.Find(TargetNode);
            if (!TargetNodeId) continue;
            
            TSharedPtr<FJsonObject> ConnJson = MakeShareable(new FJsonObject);
            ConnJson->SetStringField(TEXT("source_node"), SourceNodeId);
            ConnJson->SetStringField(TEXT("source_pin"), OutputPin->PinName.ToString());
            ConnJson->SetStringField(TEXT("target_node"), *TargetNodeId);
            ConnJson->SetStringField(TEXT("target_pin"), ConnectedPin->PinName.ToString());
            
            Connections.Add(MakeShareable(new FJsonValueObject(ConnJson)));
        }
    }
    
    return Connections;
}

// ============================================================================
// Graph Update Operations
// ============================================================================

TSharedPtr<FJsonObject> FEpicUnrealMCPNiagaraCommands::HandleUpdateNiagaraGraph(const TSharedPtr<FJsonObject>& Params)
{
    TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);

    // Method 1: Direct script path for standalone assets
    FString ScriptPath;
    bool bUseStandaloneScript = Params->TryGetStringField(TEXT("script_path"), ScriptPath);

    // Method 2: Locate script within System's Emitter
    FString AssetPath;
    FString EmitterName;
    FString ScriptType = TEXT("spawn");

    if (!bUseStandaloneScript)
    {
        if (!Params->TryGetStringField(TEXT("asset_path"), AssetPath))
        {
            Result->SetBoolField(TEXT("success"), false);
            Result->SetStringField(TEXT("error"), TEXT("Must provide either (asset_path + emitter) or script_path"));
            return Result;
        }

        if (!Params->TryGetStringField(TEXT("emitter"), EmitterName))
        {
            Result->SetBoolField(TEXT("success"), false);
            Result->SetStringField(TEXT("error"), TEXT("Must provide either (asset_path + emitter) or script_path"));
            return Result;
        }

        Params->TryGetStringField(TEXT("script"), ScriptType);
    }

    const TArray<TSharedPtr<FJsonValue>>* OperationsArray;
    if (!Params->TryGetArrayField(TEXT("operations"), OperationsArray))
    {
        Result->SetBoolField(TEXT("success"), false);
        Result->SetStringField(TEXT("error"), TEXT("Missing required parameter: operations"));
        return Result;
    }

    // Get the script
    UNiagaraScript* Script = nullptr;
    UNiagaraSystem* NiagaraSystem = nullptr;
    FGuid EmitterHandleId;  // Store emitter ID for module operations
    
    if (bUseStandaloneScript)
    {
        Script = LoadObject<UNiagaraScript>(nullptr, *ScriptPath);
        if (!Script)
        {
            Result->SetBoolField(TEXT("success"), false);
            Result->SetStringField(TEXT("error"), FString::Printf(TEXT("Failed to load Niagara script: %s"), *ScriptPath));
            return Result;
        }
        Result->SetStringField(TEXT("script_path"), ScriptPath);
    }
    else
    {
        NiagaraSystem = LoadNiagaraSystemAsset(AssetPath);
        if (!NiagaraSystem)
        {
            Result->SetBoolField(TEXT("success"), false);
            Result->SetStringField(TEXT("error"), FString::Printf(TEXT("Failed to load Niagara system: %s"), *AssetPath));
            return Result;
        }

        FNiagaraEmitterHandle* Handle = FindEmitterHandle(NiagaraSystem, EmitterName);
        if (!Handle)
        {
            Result->SetBoolField(TEXT("success"), false);
            Result->SetStringField(TEXT("error"), FString::Printf(TEXT("Emitter not found: %s"), *EmitterName));
            return Result;
        }
        
        EmitterHandleId = Handle->GetId();  // Save for later module operations

        FVersionedNiagaraEmitterData* EmitterData = Handle->GetEmitterData();
        if (!EmitterData)
        {
            Result->SetBoolField(TEXT("success"), false);
            Result->SetStringField(TEXT("error"), TEXT("Failed to get emitter data"));
            return Result;
        }

        if (ScriptType.ToLower() == TEXT("spawn"))
        {
            Script = EmitterData->SpawnScriptProps.Script;
        }
        else if (ScriptType.ToLower() == TEXT("update"))
        {
            Script = EmitterData->UpdateScriptProps.Script;
        }

        if (!Script)
        {
            Result->SetBoolField(TEXT("success"), false);
            Result->SetStringField(TEXT("error"), FString::Printf(TEXT("Script not found: %s"), *ScriptType));
            return Result;
        }

        Result->SetStringField(TEXT("asset_path"), AssetPath);
        Result->SetStringField(TEXT("emitter_name"), EmitterName);
        Result->SetStringField(TEXT("script_type"), ScriptType);
    }

    // Get graph source
    UNiagaraScriptSource* Source = Cast<UNiagaraScriptSource>(Script->GetLatestSource());
    if (!Source)
    {
        Result->SetBoolField(TEXT("success"), false);
        Result->SetStringField(TEXT("error"), TEXT("Failed to get script source"));
        return Result;
    }

    UNiagaraGraph* Graph = Source->NodeGraph;
    if (!Graph)
    {
        Result->SetBoolField(TEXT("success"), false);
        Result->SetStringField(TEXT("error"), TEXT("Failed to get Niagara graph"));
        return Result;
    }

    TArray<TSharedPtr<FJsonValue>> ResultsArray;
    int32 SuccessCount = 0;
    int32 FailCount = 0;

    for (const TSharedPtr<FJsonValue>& OpValue : *OperationsArray)
    {
        TSharedPtr<FJsonObject> Op = OpValue->AsObject();
        if (!Op.IsValid()) continue;

        FString Action = Op->GetStringField(TEXT("action"));
        TSharedPtr<FJsonObject> OpResult = MakeShareable(new FJsonObject);

        if (Action == TEXT("add_module"))
        {
            FString ModulePath;
            if (!Op->TryGetStringField(TEXT("module_path"), ModulePath))
            {
                OpResult->SetBoolField(TEXT("success"), false);
                OpResult->SetStringField(TEXT("error"), TEXT("Missing module_path"));
                FailCount++;
            }
            else
            {
                UNiagaraScript* ModuleScript = LoadObject<UNiagaraScript>(nullptr, *ModulePath);
                if (!ModuleScript)
                {
                    OpResult->SetBoolField(TEXT("success"), false);
                    OpResult->SetStringField(TEXT("error"), FString::Printf(TEXT("Failed to load module: %s"), *ModulePath));
                    FailCount++;
                }
                else
                {
                    ENiagaraScriptUsage ScriptUsage = Script->GetUsage();
                    UNiagaraNodeOutput* OutputNode = nullptr;

                    for (UEdGraphNode* NodeBase : Graph->Nodes)
                    {
                        UNiagaraNodeOutput* Node = Cast<UNiagaraNodeOutput>(NodeBase);
                        if (Node && Node->GetUsage() == ScriptUsage)
                        {
                            OutputNode = Node;
                            break;
                        }
                    }

                    if (!OutputNode)
                    {
                        OpResult->SetBoolField(TEXT("success"), false);
                        OpResult->SetStringField(TEXT("error"), TEXT("Failed to find output node"));
                        FailCount++;
                    }
                    else
                    {
                        UNiagaraNodeFunctionCall* NewModule = FNiagaraStackGraphUtilities::AddScriptModuleToStack(
                            ModuleScript,
                            *OutputNode,
                            INDEX_NONE,
                            FString(),
                            FGuid()
                        );

                        if (NewModule)
                        {
                            OpResult->SetBoolField(TEXT("success"), true);
                            OpResult->SetStringField(TEXT("action"), TEXT("add_module"));
                            OpResult->SetStringField(TEXT("module_path"), ModulePath);
                            OpResult->SetStringField(TEXT("module_name"), NewModule->GetName());
                            SuccessCount++;
                            Script->Modify();
                        }
                        else
                        {
                            OpResult->SetBoolField(TEXT("success"), false);
                            OpResult->SetStringField(TEXT("error"), TEXT("Failed to add module to stack"));
                            FailCount++;
                        }
                    }
                }
            }
        }
        else if (Action == TEXT("remove_module"))
        {
            FString ModuleName;
            if (Op->TryGetStringField(TEXT("module_name"), ModuleName))
            {
                bool bFound = false;
                for (UEdGraphNode* NodeBase : Graph->Nodes)
                {
                    UNiagaraNodeFunctionCall* FuncNode = Cast<UNiagaraNodeFunctionCall>(NodeBase);
                    if (FuncNode && FuncNode->GetName().Contains(ModuleName))
                    {
                        Script->Modify();
                        FuncNode->Modify();

                        // Find input and output pins for the ParameterMap chain
                        UEdGraphPin* InputMapPin = nullptr;
                        UEdGraphPin* OutputMapPin = nullptr;
                        
                        for (UEdGraphPin* Pin : FuncNode->Pins)
                        {
                            if (Pin->GetName() == TEXT("InputMap") && Pin->Direction == EGPD_Input)
                            {
                                InputMapPin = Pin;
                            }
                            else if (Pin->GetName() == TEXT("OutputMap") && Pin->Direction == EGPD_Output)
                            {
                                OutputMapPin = Pin;
                            }
                        }

                        // Reconnect the chain: previous output -> next input
                        if (InputMapPin && OutputMapPin)
                        {
                            // Get the connected source pin (from previous module)
                            TArray<UEdGraphPin*> ConnectedInputPins = InputMapPin->LinkedTo;
                            
                            // Get all pins connected to our output (next modules)
                            TArray<UEdGraphPin*> ConnectedOutputPins = OutputMapPin->LinkedTo;
                            
                            // Break all connections
                            InputMapPin->BreakAllPinLinks();
                            OutputMapPin->BreakAllPinLinks();
                            
                            // Reconnect: previous module's output -> next module's input
                            for (UEdGraphPin* SourcePin : ConnectedInputPins)
                            {
                                for (UEdGraphPin* TargetPin : ConnectedOutputPins)
                                {
                                    SourcePin->MakeLinkTo(TargetPin);
                                }
                            }
                        }
                        else
                        {
                            // Fallback: just break all links
                            for (UEdGraphPin* Pin : FuncNode->Pins)
                            {
                                if (Pin)
                                {
                                    Pin->BreakAllPinLinks();
                                }
                            }
                        }

                        Graph->RemoveNode(FuncNode);

                        OpResult->SetBoolField(TEXT("success"), true);
                        OpResult->SetStringField(TEXT("action"), TEXT("remove_module"));
                        OpResult->SetStringField(TEXT("module_name"), ModuleName);
                        SuccessCount++;
                        bFound = true;
                        break;
                    }
                }

                if (!bFound)
                {
                    OpResult->SetBoolField(TEXT("success"), false);
                    OpResult->SetStringField(TEXT("error"), FString::Printf(TEXT("Module not found: %s"), *ModuleName));
                    FailCount++;
                }
            }
            else
            {
                OpResult->SetBoolField(TEXT("success"), false);
                OpResult->SetStringField(TEXT("error"), TEXT("Missing module_name"));
                FailCount++;
            }
        }
        else if (Action == TEXT("set_parameter"))
        {
            FString ParamName;
            if (!Op->TryGetStringField(TEXT("name"), ParamName))
            {
                OpResult->SetBoolField(TEXT("success"), false);
                OpResult->SetStringField(TEXT("error"), TEXT("Missing parameter name"));
                FailCount++;
            }
            else
            {
                TSharedPtr<FJsonValue> ValueJson = Op->TryGetField(TEXT("value"));
                if (ValueJson.IsValid())
                {
                    float FloatValue;
                    if (ValueJson->TryGetNumber(FloatValue))
                    {
                        FNiagaraVariable Var(FNiagaraTypeDefinition::GetFloatDef(), *ParamName);
                        Var.SetValue(FloatValue);
                        bool bAddIfMissing = true;
                        Script->RapidIterationParameters.SetParameterData(Var.GetData(), Var, bAddIfMissing);

                        OpResult->SetBoolField(TEXT("success"), true);
                        OpResult->SetStringField(TEXT("action"), TEXT("set_parameter"));
                        OpResult->SetStringField(TEXT("name"), ParamName);
                        SuccessCount++;
                        Script->Modify();
                    }
                    else
                    {
                        OpResult->SetBoolField(TEXT("success"), false);
                        OpResult->SetStringField(TEXT("error"), TEXT("Unsupported parameter value type"));
                        FailCount++;
                    }
                }
                else
                {
                    OpResult->SetBoolField(TEXT("success"), false);
                    OpResult->SetStringField(TEXT("error"), TEXT("Missing parameter value"));
                    FailCount++;
                }
            }
        }
        else
        {
            OpResult->SetBoolField(TEXT("success"), false);
            OpResult->SetStringField(TEXT("error"), FString::Printf(TEXT("Unknown action: %s"), *Action));
            FailCount++;
        }

        ResultsArray.Add(MakeShareable(new FJsonValueObject(OpResult)));
    }

    // Mark dirty
    if (NiagaraSystem)
    {
        NiagaraSystem->MarkPackageDirty();
    }
    else
    {
        Script->MarkPackageDirty();
    }

    Result->SetBoolField(TEXT("success"), FailCount == 0);
    Result->SetArrayField(TEXT("results"), ResultsArray);
    Result->SetNumberField(TEXT("success_count"), SuccessCount);
    Result->SetNumberField(TEXT("fail_count"), FailCount);

    return Result;
}

// ============================================================================
// Debug Tools - Compiled Code Inspection
// ============================================================================

TSharedPtr<FJsonObject> FEpicUnrealMCPNiagaraCommands::HandleGetNiagaraCompiledCode(const TSharedPtr<FJsonObject>& Params)
{
    TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
    
    // Get asset path
    FString AssetPath;
    if (!Params->TryGetStringField(TEXT("asset_path"), AssetPath))
    {
        Result->SetBoolField(TEXT("success"), false);
        Result->SetStringField(TEXT("error"), TEXT("Missing asset_path parameter"));
        return Result;
    }
    
    // Load Niagara System
    UNiagaraSystem* NiagaraSystem = LoadNiagaraSystemAsset(AssetPath);
    if (!NiagaraSystem)
    {
        Result->SetBoolField(TEXT("success"), false);
        Result->SetStringField(TEXT("error"), FString::Printf(TEXT("Failed to load Niagara System: %s"), *AssetPath));
        return Result;
    }
    
    // Get emitter name (required for embedded scripts)
    FString EmitterName;
    Params->TryGetStringField(TEXT("emitter"), EmitterName);
    
    // Get script type (spawn, update)
    FString ScriptType = TEXT("spawn");
    Params->TryGetStringField(TEXT("script"), ScriptType);
    
    // Find the script
    UNiagaraScript* Script = nullptr;
    
    if (EmitterName.IsEmpty())
    {
        // Try to get system-level script
        if (ScriptType == TEXT("system_spawn"))
        {
            Script = NiagaraSystem->GetSystemSpawnScript();
        }
        else if (ScriptType == TEXT("system_update"))
        {
            Script = NiagaraSystem->GetSystemUpdateScript();
        }
        else
        {
            Result->SetBoolField(TEXT("success"), false);
            Result->SetStringField(TEXT("error"), TEXT("For system scripts, use script='system_spawn' or 'system_update'"));
            return Result;
        }
    }
    else
    {
        // Find emitter handle
        FNiagaraEmitterHandle* Handle = FindEmitterHandle(NiagaraSystem, EmitterName);
        if (!Handle)
        {
            Result->SetBoolField(TEXT("success"), false);
            Result->SetStringField(TEXT("error"), FString::Printf(TEXT("Emitter not found: %s"), *EmitterName));
            return Result;
        }
        
        // Get emitter data (contains script references)
        FVersionedNiagaraEmitterData* EmitterData = Handle->GetEmitterData();
        if (!EmitterData)
        {
            Result->SetBoolField(TEXT("success"), false);
            Result->SetStringField(TEXT("error"), TEXT("Failed to get emitter data from handle"));
            return Result;
        }
        
        // Get script based on type
        if (ScriptType == TEXT("spawn"))
        {
            Script = EmitterData->SpawnScriptProps.Script;
        }
        else if (ScriptType == TEXT("update"))
        {
            Script = EmitterData->UpdateScriptProps.Script;
        }
        else if (ScriptType == TEXT("gpu_compute"))
        {
            // GPU compute script uses GetScript with specific usage
            Script = EmitterData->GetScript(ENiagaraScriptUsage::ParticleGPUComputeScript, FGuid());
        }
        else
        {
            Result->SetBoolField(TEXT("success"), false);
            Result->SetStringField(TEXT("error"), FString::Printf(TEXT("Unknown script type: %s. Use 'spawn', 'update', or 'gpu_compute'"), *ScriptType));
            return Result;
        }
    }
    
    if (!Script)
    {
        Result->SetBoolField(TEXT("success"), false);
        Result->SetStringField(TEXT("error"), TEXT("Script not found"));
        return Result;
    }
    
    // Extract compiled HLSL code
#if WITH_EDITORONLY_DATA
    // HLSL translations are stored in FNiagaraVMExecutableData
    const FNiagaraVMExecutableData& VMData = Script->GetVMExecutableData();
    FString HlslCpu = VMData.LastHlslTranslation;
    FString HlslGpu = VMData.LastHlslTranslationGPU;
    
    // Get compile errors if any
    TArray<FString> CompileErrors;
#if WITH_EDITOR
    if (Script->GetRenderThreadScript())
    {
        const TArray<FString>& Errors = Script->GetRenderThreadScript()->GetCompileErrors();
        for (const FString& Error : Errors)
        {
            CompileErrors.Add(Error);
        }
    }
#endif
    
    // Build result
    Result->SetBoolField(TEXT("success"), true);
    Result->SetStringField(TEXT("asset_path"), AssetPath);
    Result->SetStringField(TEXT("emitter_name"), EmitterName);
    Result->SetStringField(TEXT("script_type"), ScriptType);
    Result->SetStringField(TEXT("script_name"), Script->GetName());
    
    // HLSL outputs
    if (!HlslCpu.IsEmpty())
    {
        Result->SetStringField(TEXT("hlsl_cpu"), HlslCpu);
        Result->SetNumberField(TEXT("hlsl_cpu_length"), HlslCpu.Len());
    }
    else
    {
        Result->SetStringField(TEXT("hlsl_cpu"), TEXT(""));
        Result->SetNumberField(TEXT("hlsl_cpu_length"), 0);
    }
    
    if (!HlslGpu.IsEmpty())
    {
        Result->SetStringField(TEXT("hlsl_gpu"), HlslGpu);
        Result->SetNumberField(TEXT("hlsl_gpu_length"), HlslGpu.Len());
    }
    else
    {
        Result->SetStringField(TEXT("hlsl_gpu"), TEXT(""));
        Result->SetNumberField(TEXT("hlsl_gpu_length"), 0);
    }
    
    // Compile errors
    TArray<TSharedPtr<FJsonValue>> ErrorsArray;
    for (const FString& Error : CompileErrors)
    {
        ErrorsArray.Add(MakeShareable(new FJsonValueString(Error)));
    }
    Result->SetArrayField(TEXT("compile_errors"), ErrorsArray);
    Result->SetNumberField(TEXT("error_count"), CompileErrors.Num());
    
#else
    Result->SetBoolField(TEXT("success"), false);
    Result->SetStringField(TEXT("error"), TEXT("Compiled code inspection is only available in editor builds (WITH_EDITORONLY_DATA)"));
#endif
    
    return Result;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPNiagaraCommands::HandleGetNiagaraParticleAttributes(const TSharedPtr<FJsonObject>& Params)
{
    TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
    
#if WITH_EDITOR
    // Get component name from params
    FString ComponentName;
    if (!Params->TryGetStringField(TEXT("component_name"), ComponentName))
    {
        Result->SetBoolField(TEXT("success"), false);
        Result->SetStringField(TEXT("error"), TEXT("Missing component_name parameter"));
        return Result;
    }
    
    // Optional parameters
    FString EmitterName;
    Params->TryGetStringField(TEXT("emitter"), EmitterName);
    
    TArray<FString> AttributeNames;
    const TSharedPtr<FJsonObject>* AttrsObj;
    if (Params->TryGetObjectField(TEXT("attributes"), AttrsObj))
    {
        for (const auto& Pair : AttrsObj->Get()->Values)
        {
            AttributeNames.Add(Pair.Key);
        }
    }
    
    int32 FrameIndex = 0;
    Params->TryGetNumberField(TEXT("frame"), FrameIndex);
    
    // Find the Niagara component in the world
    UNiagaraComponent* NiagaraComponent = nullptr;
    
    // Search through all actors in the world
    for (TActorIterator<AActor> It(GWorld); It; ++It)
    {
        AActor* Actor = *It;
        if (!Actor) continue;
        
        // Get all Niagara components from this actor
        TArray<UNiagaraComponent*> Components;
        Actor->GetComponents<UNiagaraComponent>(Components);
        
        for (UNiagaraComponent* Comp : Components)
        {
            if (Comp && Comp->GetName() == ComponentName)
            {
                NiagaraComponent = Comp;
                break;
            }
        }
        
        if (NiagaraComponent)
            break;
    }
    
    if (!NiagaraComponent)
    {
        Result->SetBoolField(TEXT("success"), false);
        Result->SetStringField(TEXT("error"), FString::Printf(TEXT("Niagara component not found: %s"), *ComponentName));
        return Result;
    }
    
    // Get or create SimCache
    UNiagaraSimCache* SimCache = NiagaraComponent->GetSimCache();
    bool bCreatedCache = false;
    
    if (!SimCache)
    {
        // Create a new SimCache for capturing
        SimCache = NewObject<UNiagaraSimCache>(GetTransientPackage());
        if (!SimCache)
        {
            Result->SetBoolField(TEXT("success"), false);
            Result->SetStringField(TEXT("error"), TEXT("Failed to create SimCache"));
            return Result;
        }
        
        // Begin capturing
        FNiagaraSimCacheCreateParameters CreateParams = FNiagaraSimCacheCreateParameters::CreateForDebugging();
        if (!SimCache->BeginWrite(CreateParams, NiagaraComponent))
        {
            Result->SetBoolField(TEXT("success"), false);
            Result->SetStringField(TEXT("error"), TEXT("Failed to begin SimCache write"));
            return Result;
        }
        
        // Write current frame
        if (!SimCache->WriteFrame(NiagaraComponent))
        {
            Result->SetBoolField(TEXT("success"), false);
            Result->SetStringField(TEXT("error"), TEXT("Failed to write frame to SimCache"));
            return Result;
        }
        
        SimCache->EndWrite();
        bCreatedCache = true;
    }
    
    // Get num frames and validate frame index
    int32 NumFrames = SimCache->GetNumFrames();
    if (FrameIndex < 0 || FrameIndex >= NumFrames)
    {
        FrameIndex = FMath::Clamp(FrameIndex, 0, NumFrames - 1);
    }
    
    // Get emitter info
    TArray<FName> EmitterNames = SimCache->GetEmitterNames();
    
    // Build result
    Result->SetBoolField(TEXT("success"), true);
    Result->SetStringField(TEXT("component_name"), ComponentName);
    Result->SetNumberField(TEXT("num_frames"), NumFrames);
    Result->SetNumberField(TEXT("current_frame"), FrameIndex);
    
    // Emitter data
    TArray<TSharedPtr<FJsonValue>> EmittersArray;
    
    for (int32 EmitterIdx = 0; EmitterIdx < EmitterNames.Num(); ++EmitterIdx)
    {
        FName EmitterFName = EmitterNames[EmitterIdx];
        FString EmitterFNameStr = EmitterFName.ToString();
        
        // Skip if specific emitter requested and this isn't it
        if (!EmitterName.IsEmpty() && EmitterFNameStr != EmitterName)
        {
            continue;
        }
        
        TSharedPtr<FJsonObject> EmitterJson = MakeShareable(new FJsonObject);
        EmitterJson->SetStringField(TEXT("name"), EmitterFNameStr);
        
        int32 NumInstances = SimCache->GetEmitterNumInstances(EmitterIdx, FrameIndex);
        EmitterJson->SetNumberField(TEXT("particle_count"), NumInstances);
        
        // Get attribute list
        TArray<TSharedPtr<FJsonValue>> AttributesList;
        SimCache->ForEachEmitterAttribute(EmitterIdx, [&](const FNiagaraSimCacheVariable& Var) -> bool
        {
            TSharedPtr<FJsonObject> AttrJson = MakeShareable(new FJsonObject);
            FString VarName = Var.Variable.GetName().ToString();
            AttrJson->SetStringField(TEXT("name"), VarName);
            AttrJson->SetNumberField(TEXT("float_count"), Var.FloatCount);
            AttrJson->SetNumberField(TEXT("int_count"), Var.Int32Count);
            AttrJson->SetNumberField(TEXT("half_count"), Var.HalfCount);
            AttributesList.Add(MakeShareable(new FJsonValueObject(AttrJson)));
            return true;
        });
        EmitterJson->SetArrayField(TEXT("attributes"), AttributesList);
        
        // Read particle data
        if (NumInstances > 0)
        {
            TArray<TSharedPtr<FJsonValue>> ParticlesArray;
            
            // Read common attributes
            TArray<FVector> Positions;
            TArray<FLinearColor> Colors;
            TArray<float> Lifetimes;
            TArray<float> Ages;
            TArray<FVector> Velocities;
            
            SimCache->ReadPositionAttribute(Positions, FName("Position"), EmitterFName, true, FrameIndex);
            SimCache->ReadColorAttribute(Colors, FName("Color"), EmitterFName, FrameIndex);
            SimCache->ReadFloatAttribute(Lifetimes, FName("Lifetime"), EmitterFName, FrameIndex);
            SimCache->ReadFloatAttribute(Ages, FName("Age"), EmitterFName, FrameIndex);
            SimCache->ReadVectorAttribute(Velocities, FName("Velocity"), EmitterFName, FrameIndex);
            
            for (int32 i = 0; i < NumInstances; ++i)
            {
                TSharedPtr<FJsonObject> ParticleJson = MakeShareable(new FJsonObject);
                ParticleJson->SetNumberField(TEXT("index"), i);
                
                // Position
                if (Positions.IsValidIndex(i))
                {
                    TArray<TSharedPtr<FJsonValue>> PosArray;
                    PosArray.Add(MakeShareable(new FJsonValueNumber(Positions[i].X)));
                    PosArray.Add(MakeShareable(new FJsonValueNumber(Positions[i].Y)));
                    PosArray.Add(MakeShareable(new FJsonValueNumber(Positions[i].Z)));
                    ParticleJson->SetArrayField(TEXT("position"), PosArray);
                }
                
                // Color
                if (Colors.IsValidIndex(i))
                {
                    TArray<TSharedPtr<FJsonValue>> ColorArray;
                    ColorArray.Add(MakeShareable(new FJsonValueNumber(Colors[i].R)));
                    ColorArray.Add(MakeShareable(new FJsonValueNumber(Colors[i].G)));
                    ColorArray.Add(MakeShareable(new FJsonValueNumber(Colors[i].B)));
                    ColorArray.Add(MakeShareable(new FJsonValueNumber(Colors[i].A)));
                    ParticleJson->SetArrayField(TEXT("color"), ColorArray);
                }
                
                // Lifetime
                if (Lifetimes.IsValidIndex(i))
                {
                    ParticleJson->SetNumberField(TEXT("lifetime"), Lifetimes[i]);
                }
                
                // Age
                if (Ages.IsValidIndex(i))
                {
                    ParticleJson->SetNumberField(TEXT("age"), Ages[i]);
                }
                
                // Velocity
                if (Velocities.IsValidIndex(i))
                {
                    TArray<TSharedPtr<FJsonValue>> VelArray;
                    VelArray.Add(MakeShareable(new FJsonValueNumber(Velocities[i].X)));
                    VelArray.Add(MakeShareable(new FJsonValueNumber(Velocities[i].Y)));
                    VelArray.Add(MakeShareable(new FJsonValueNumber(Velocities[i].Z)));
                    ParticleJson->SetArrayField(TEXT("velocity"), VelArray);
                }
                
                ParticlesArray.Add(MakeShareable(new FJsonValueObject(ParticleJson)));
            }
            
            EmitterJson->SetArrayField(TEXT("particles"), ParticlesArray);
        }
        
        EmittersArray.Add(MakeShareable(new FJsonValueObject(EmitterJson)));
    }
    
    Result->SetArrayField(TEXT("emitters"), EmittersArray);
    
    // Clean up if we created the cache
    if (bCreatedCache && SimCache)
    {
        // Don't set the cache on the component since we just wanted to read
    }
    
#else
    Result->SetBoolField(TEXT("success"), false);
    Result->SetStringField(TEXT("error"), TEXT("Particle attribute inspection requires editor build"));
#endif
    
    return Result;
}
