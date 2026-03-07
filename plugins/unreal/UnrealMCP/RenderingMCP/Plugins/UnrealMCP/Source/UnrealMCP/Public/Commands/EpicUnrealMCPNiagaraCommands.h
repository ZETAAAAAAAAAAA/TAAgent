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
 * Design Note: Only provides tools that generic tools cannot handle.
 * - get_niagara_asset_details: Deep inspection of complex nested Niagara structure
 * - update_niagara_asset: Batch modification of emitters, renderers, parameters, etc.
 * 
 * For listing assets: use get_assets(asset_class="NiagaraSystem")
 * For creating/deleting: use generic create_asset/delete_asset
 * 
 * This follows the MCP design philosophy:
 * - Generic tools for common operations
 * - Domain-specific tools only for complex nested data
 */
class UNREALMCP_API FEpicUnrealMCPNiagaraCommands
{
public:
    FEpicUnrealMCPNiagaraCommands();

    TSharedPtr<FJsonObject> HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params);

private:
    // ============================================================================
    // Read Operations
    // ============================================================================
    
    /**
     * Get detailed information about a Niagara asset.
     * Supports selective data retrieval via detail_level and include parameters.
     */
    TSharedPtr<FJsonObject> HandleGetNiagaraAssetDetails(const TSharedPtr<FJsonObject>& Params);
    
    // ============================================================================
    // Update Operations
    // ============================================================================
    
    /**
     * Batch update a Niagara asset with multiple operations.
     * 
     * Operations format:
     * [
     *   {"target": "emitter", "name": "Flame", "action": "set_enabled", "value": false},
     *   {"target": "emitter", "name": "Flame", "action": "rename", "value": "BigFlame"},
     *   {"target": "renderer", "emitter": "Flame", "index": 0, "action": "set_enabled", "value": true},
     *   {"target": "parameter", "emitter": "Flame", "script": "spawn", "name": "SpawnRate", "value": 100.0},
     *   {"target": "sim_stage", "emitter": "Flame", "name": "Collision", "action": "set_enabled", "value": true},
     *   {"target": "emitter", "name": "Smoke", "action": "add", "template": "/Niagara/Templates/SimpleSmoke"},
     *   {"target": "emitter", "name": "OldEmitter", "action": "remove"}
     * ]
     */
    TSharedPtr<FJsonObject> HandleUpdateNiagaraAsset(const TSharedPtr<FJsonObject>& Params);
    
    /**
     * Analyze Standard emitter compatibility for Stateless conversion.
     * 
     * Checks if all modules in a Standard emitter have Stateless equivalents.
     * Returns conversion suggestions and compatibility report.
     */
    TSharedPtr<FJsonObject> HandleAnalyzeStatelessCompatibility(const TSharedPtr<FJsonObject>& Params);
    
    /**
     * Convert Standard emitter to Stateless mode.
     * 
     * Automatically converts compatible Standard emitter to Stateless,
     * migrating module parameters where possible.
     */
    TSharedPtr<FJsonObject> HandleConvertToStateless(const TSharedPtr<FJsonObject>& Params);
    
    /**
     * Get Niagara Module Graph nodes and connections.
     * Similar to get_material_graph for materials.
     * 
     * Returns all nodes in the graph with their:
     * - Node type and properties
     * - Input/Output pins
     * - Connections between nodes
     */
    TSharedPtr<FJsonObject> HandleGetNiagaraModuleGraph(const TSharedPtr<FJsonObject>& Params);
    
    // ============================================================================
    // Helper Functions - Read
    // ============================================================================
    
    TSharedPtr<FJsonObject> GetNiagaraSystemOverview(UNiagaraSystem* System);
    TSharedPtr<FJsonObject> GetEmitterDetails(FNiagaraEmitterHandle& Handle, UNiagaraSystem* System, 
        const TArray<FString>& IncludeSections);
    TSharedPtr<FJsonObject> GetScriptDetails(UNiagaraScript* Script);
    TSharedPtr<FJsonObject> GetRendererDetails(UNiagaraRendererProperties* Renderer);
    TSharedPtr<FJsonObject> GetSimulationStageDetails(UNiagaraSimulationStageBase* Stage);
    
    // Graph node helpers
    TSharedPtr<FJsonObject> GetNodeDetails(class UNiagaraNode* Node);
    TArray<TSharedPtr<FJsonValue>> GetNodeConnections(class UNiagaraNode* Node, const TMap<class UEdGraphNode*, FString>& NodeIdMap);
    
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
