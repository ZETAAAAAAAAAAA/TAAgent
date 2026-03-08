// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Json.h"

class UNiagaraSystem;
class UNiagaraEmitter;
class UNiagaraScript;
class UNiagaraRendererProperties;
class UNiagaraSimulationStageBase;

// UE 5.7 Stateless Niagara forward declarations
class UNiagaraStatelessEmitter;
class UNiagaraStatelessModule;

// Forward declarations for Niagara types
struct FNiagaraEmitterHandle;
struct FVersionedNiagaraEmitterData;

/**
 * Handler class for Niagara-related MCP commands.
 * 
 * Design Philosophy: 4 tools following "Minimal Tool Set" principle.
 * 
 * Graph Form Tools:
 * - get_niagara_graph: Read Niagara script graphs (embedded or standalone)
 * - update_niagara_graph: Update Niagara script graphs
 * 
 * Emitter Form Tools:
 * - get_niagara_emitter: Read Emitter hierarchy and properties
 * - update_niagara_emitter: Batch update Emitter structure
 * 
 * For listing assets: use get_assets(asset_class="NiagaraSystem")
 * For creating/deleting: use generic create_asset/delete_asset
 */
class UNREALMCP_API FEpicUnrealMCPNiagaraCommands
{
public:
    FEpicUnrealMCPNiagaraCommands();

    TSharedPtr<FJsonObject> HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params);

private:
    // ============================================================================
    // Graph Form Tools - Read/Update Niagara Script Graphs
    // ============================================================================
    
    /**
     * Get Niagara Graph nodes and connections.
     * 
     * Unified interface for reading graphs from:
     * 1. Scripts within a Niagara System (specify asset_path + emitter + script)
     * 2. Standalone Niagara Script assets (specify script_path only)
     */
    TSharedPtr<FJsonObject> HandleGetNiagaraGraph(const TSharedPtr<FJsonObject>& Params);
    
    /**
     * Update Niagara Graph with operations.
     * 
     * Operations: add_module, remove_module, set_parameter, connect, disconnect
     */
    TSharedPtr<FJsonObject> HandleUpdateNiagaraGraph(const TSharedPtr<FJsonObject>& Params);
    
    // ============================================================================
    // Emitter Form Tools - Read/Update Emitter Hierarchy
    // ============================================================================
    
    /**
     * Get Niagara Emitter structure and properties.
     * 
     * Supports:
     * - detail_level: "overview" or "full"
     * - include: scripts, renderers, parameters, stateless_analysis
     */
    TSharedPtr<FJsonObject> HandleGetNiagaraEmitter(const TSharedPtr<FJsonObject>& Params);
    
    /**
     * Batch update Niagara Emitter with multiple operations.
     * 
     * Operations:
     * - emitter: set_enabled, rename, add, remove, convert_to_stateless
     * - renderer: set_enabled
     * - parameter: set value
     * - sim_stage: set_enabled
     * - stateless_module: set_property
     */
    TSharedPtr<FJsonObject> HandleUpdateNiagaraEmitter(const TSharedPtr<FJsonObject>& Params);
    
    // ============================================================================
    // Debug Tools - Compiled Code Inspection
    // ============================================================================
    
    /**
     * Get compiled HLSL code from Niagara scripts.
     * 
     * Returns generated HLSL code for CPU (VM) and GPU (Compute Shader) scripts.
     * Useful for debugging and understanding Niagara execution.
     * 
     * Input: asset_path + emitter + script (like get_niagara_graph)
     * Output: hlsl_cpu, hlsl_gpu, compile_errors
     */
    TSharedPtr<FJsonObject> HandleGetNiagaraCompiledCode(const TSharedPtr<FJsonObject>& Params);
    
    // ============================================================================
    // Helper Functions - Graph
    // ============================================================================
    
    // Core graph extraction (shared between embedded and standalone scripts)
    TSharedPtr<FJsonObject> ExtractGraphFromScript(UNiagaraScript* Script, const FString& ModuleFilter = TEXT(""));
    
    // Graph node helpers
    TSharedPtr<FJsonObject> GetNodeDetails(class UNiagaraNode* Node);
    TArray<TSharedPtr<FJsonValue>> GetNodeConnections(class UNiagaraNode* Node, const TMap<class UEdGraphNode*, FString>& NodeIdMap);
    
    // ============================================================================
    // Helper Functions - Emitter
    // ============================================================================
    
    TSharedPtr<FJsonObject> GetNiagaraSystemOverview(UNiagaraSystem* System);
    TSharedPtr<FJsonObject> GetEmitterDetails(FNiagaraEmitterHandle& Handle, UNiagaraSystem* System, 
        const TArray<FString>& IncludeSections);
    TSharedPtr<FJsonObject> GetScriptDetails(UNiagaraScript* Script);
    TSharedPtr<FJsonObject> GetRendererDetails(UNiagaraRendererProperties* Renderer);
    TSharedPtr<FJsonObject> GetSimulationStageDetails(UNiagaraSimulationStageBase* Stage);
    
    // Stateless analysis helper
    TSharedPtr<FJsonObject> AnalyzeStatelessCompatibility(FNiagaraEmitterHandle& Handle);
    
    // ============================================================================
    // Helper Functions - Update
    // ============================================================================
    
    TSharedPtr<FJsonObject> ProcessEmitterOperation(UNiagaraSystem* System, const TSharedPtr<FJsonObject>& Op);
    TSharedPtr<FJsonObject> ProcessRendererOperation(UNiagaraSystem* System, const TSharedPtr<FJsonObject>& Op);
    TSharedPtr<FJsonObject> ProcessParameterOperation(UNiagaraSystem* System, const TSharedPtr<FJsonObject>& Op);
    TSharedPtr<FJsonObject> ProcessSimStageOperation(UNiagaraSystem* System, const TSharedPtr<FJsonObject>& Op);
    
    // UE 5.7 Stateless module operations
    TSharedPtr<FJsonObject> ProcessStatelessModuleOperation(UNiagaraSystem* System, const TSharedPtr<FJsonObject>& Op);
    
    // Find emitter handle by name
    FNiagaraEmitterHandle* FindEmitterHandle(UNiagaraSystem* System, const FString& EmitterName);
    
    // ============================================================================
    // Utility Functions
    // ============================================================================
    
    TArray<FString> ParseIncludeSections(const TSharedPtr<FJsonObject>& Params);
    bool ShouldInclude(const TArray<FString>& IncludeSections, const FString& Section);
    UNiagaraSystem* LoadNiagaraSystemAsset(const FString& AssetPath);
};
