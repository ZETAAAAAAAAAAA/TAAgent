#pragma once

#include "CoreMinimal.h"
#include "Json.h"

/**
 * Handler class for Editor-related MCP commands
 * Handles viewport control, actor manipulation, and level management
 */
class UNREALMCP_API FEpicUnrealMCPEditorCommands
{
public:
    	FEpicUnrealMCPEditorCommands();

    // Handle editor commands
    TSharedPtr<FJsonObject> HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params);

private:
    // Actor manipulation commands
    TSharedPtr<FJsonObject> HandleGetActorsInLevel(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleFindActorsByName(const TSharedPtr<FJsonObject>& Params);
    
    // Deprecated: Use FEpicUnrealMCPEnvironmentCommands::HandleSpawnActor instead (reflection-based)
    UE_DEPRECATED(5.0, "Use FEpicUnrealMCPEnvironmentCommands::HandleSpawnActor instead")
    TSharedPtr<FJsonObject> HandleSpawnActor(const TSharedPtr<FJsonObject>& Params);
    
    // Deprecated: Use FEpicUnrealMCPEnvironmentCommands::HandleDeleteActor instead
    UE_DEPRECATED(5.0, "Use FEpicUnrealMCPEnvironmentCommands::HandleDeleteActor instead")
    TSharedPtr<FJsonObject> HandleDeleteActor(const TSharedPtr<FJsonObject>& Params);
    
    TSharedPtr<FJsonObject> HandleSetActorTransform(const TSharedPtr<FJsonObject>& Params);

    // Blueprint actor spawning
    TSharedPtr<FJsonObject> HandleSpawnBlueprintActor(const TSharedPtr<FJsonObject>& Params);

    // FBX import
    TSharedPtr<FJsonObject> HandleImportFBX(const TSharedPtr<FJsonObject>& Params);

    // ============================================================================
    // Generic Asset Management (通用资产操作 - Refactored)
    // ============================================================================

    /** Create any asset by type using reflection */
    TSharedPtr<FJsonObject> HandleCreateAsset(const TSharedPtr<FJsonObject>& Params);

    /** Delete asset by path */
    TSharedPtr<FJsonObject> HandleDeleteAsset(const TSharedPtr<FJsonObject>& Params);

    /** Set asset properties using reflection */
    TSharedPtr<FJsonObject> HandleSetAssetProperties(const TSharedPtr<FJsonObject>& Params);

    /** Get asset properties using reflection */
    TSharedPtr<FJsonObject> HandleGetAssetProperties(const TSharedPtr<FJsonObject>& Params);

    /** Batch create multiple assets */
    TSharedPtr<FJsonObject> HandleBatchCreateAssets(const TSharedPtr<FJsonObject>& Params);

    /** Batch set properties on multiple assets */
    TSharedPtr<FJsonObject> HandleBatchSetAssetsProperties(const TSharedPtr<FJsonObject>& Params);

    // ============================================================================
    // Reflection Helpers for Assets
    // ============================================================================

    /** Find asset class by type name */
    static UClass* FindAssetClassByName(const FString& TypeName);

    /** Set property on UObject (asset or actor) */
    static bool SetUObjectProperty(UObject* Object, const FString& PropertyName, const TSharedPtr<FJsonValue>& Value, FString& OutError);

    /** Get property from UObject as JsonValue */
    static TSharedPtr<FJsonValue> GetUObjectPropertyAsJson(UObject* Object, const FString& PropertyName);

    /** Get all properties from UObject as JSON */
    static TSharedPtr<FJsonObject> GetAllUObjectPropertiesAsJson(UObject* Object);
}; 