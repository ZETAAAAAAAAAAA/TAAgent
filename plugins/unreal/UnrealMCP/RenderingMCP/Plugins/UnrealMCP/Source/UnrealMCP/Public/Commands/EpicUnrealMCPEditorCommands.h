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

    // Texture import
    TSharedPtr<FJsonObject> HandleImportTexture(const TSharedPtr<FJsonObject>& Params);

    // Texture properties
    TSharedPtr<FJsonObject> HandleSetTextureProperties(const TSharedPtr<FJsonObject>& Params);

    // Material instance
    TSharedPtr<FJsonObject> HandleCreateMaterialInstance(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleSetMaterialInstanceParameter(const TSharedPtr<FJsonObject>& Params);

    // Static mesh from data
    TSharedPtr<FJsonObject> HandleCreateStaticMeshFromData(const TSharedPtr<FJsonObject>& Params);
}; 