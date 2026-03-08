// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Json.h"

class UMaterial;
class UMaterialFunction;
class UMaterialExpression;
struct FExpressionInput;

/**
 * Handler class for Material-related MCP commands.
 * 
 * Design Note: Consolidates all Material operations in one place.
 * - Material creation and editing
 * - Material graph building and reading
 * - Material instance management
 * - Texture import and properties
 * 
 * This follows the MCP design philosophy:
 * - Domain-specific tools for complex nested data
 * - Single responsibility for Material domain
 */
class UNREALMCP_API FEpicUnrealMCPMaterialCommands
{
public:
    FEpicUnrealMCPMaterialCommands();

    TSharedPtr<FJsonObject> HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params);

private:
    // ============================================================================
    // Material Creation
    // ============================================================================
    
    TSharedPtr<FJsonObject> HandleCreateMaterial(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleCreateMaterialFunction(const TSharedPtr<FJsonObject>& Params);

    // ============================================================================
    // Material Graph Operations
    // ============================================================================
    
    /** Build entire material graph in one call (recommended) */
    TSharedPtr<FJsonObject> HandleBuildMaterialGraph(const TSharedPtr<FJsonObject>& Params);
    
    /** Get material or material function graph (nodes + connections) */
    TSharedPtr<FJsonObject> HandleGetMaterialGraph(const TSharedPtr<FJsonObject>& Params);
    
    /** Compile material to update shader */
    TSharedPtr<FJsonObject> HandleCompileMaterial(const TSharedPtr<FJsonObject>& Params);
    
    /** Set material properties (shading model, blend mode, etc.) */
    TSharedPtr<FJsonObject> HandleSetMaterialProperties(const TSharedPtr<FJsonObject>& Params);

    // ============================================================================
    // Material Instance Operations
    // ============================================================================
    
    TSharedPtr<FJsonObject> HandleCreateMaterialInstance(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleSetMaterialInstanceParameter(const TSharedPtr<FJsonObject>& Params);

    // ============================================================================
    // Texture Operations
    // ============================================================================
    
    TSharedPtr<FJsonObject> HandleImportTexture(const TSharedPtr<FJsonObject>& Params);

    // ============================================================================
    // Helper Functions
    // ============================================================================
    
    /** Find material expression by node ID */
    UMaterialExpression* FindMaterialExpressionById(UMaterial* Material, const FString& NodeId);
    
    /** Get expression input by name */
    FExpressionInput* GetExpressionInputByName(UMaterialExpression* Expression, const FString& InputName);
    
    /** Get material property input (BaseColor, Normal, etc.) */
    FExpressionInput* GetMaterialPropertyInput(UMaterial* Material, const FString& PropertyName);
};
