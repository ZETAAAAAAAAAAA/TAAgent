#pragma once

#include "CoreMinimal.h"
#include "Json.h"

class UClass;
class AActor;
class UActorComponent;

/**
 * Generic Actor Management and Environment Commands
 * 
 * Design Philosophy:
 * - 5 generic tools for all actor operations
 * - Actor type configurations stored in skill.md
 * - Property matching: match -> modify, no match -> ignore
 * 
 * Refactored: Uses UE Reflection System for universal property access
 * - No hardcoded actor types or properties
 * - Supports any UPROPERTY at runtime
 * - New actor types work without code changes
 */
class FEpicUnrealMCPEnvironmentCommands
{
public:
    // Main command dispatcher
    static TSharedPtr<FJsonObject> HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params);
    
    // Viewport screenshot
    static TSharedPtr<FJsonObject> HandleGetViewportScreenshot(const TSharedPtr<FJsonObject>& Params);
    
    // ============================================================================
    // Generic Actor Management (5 Core Tools - Refactored with Reflection)
    // ============================================================================
    
    /** Spawn any actor by class name. Uses reflection to find class dynamically. */
    static TSharedPtr<FJsonObject> HandleSpawnActor(const TSharedPtr<FJsonObject>& Params);
    
    /** Delete actor by name. */
    static TSharedPtr<FJsonObject> HandleDeleteActor(const TSharedPtr<FJsonObject>& Params);
    
    /** List all actors with optional class filter. */
    static TSharedPtr<FJsonObject> HandleGetActors(const TSharedPtr<FJsonObject>& Params);
    
    /** Set actor properties using reflection. Matches any UPROPERTY by name. */
    static TSharedPtr<FJsonObject> HandleSetActorProperties(const TSharedPtr<FJsonObject>& Params);
    
    /** Get actor properties using reflection. Reads all UPROPERTY values. */
    static TSharedPtr<FJsonObject> HandleGetActorProperties(const TSharedPtr<FJsonObject>& Params);
    
    // ============================================================================
    // Reflection-based Property Helpers
    // ============================================================================
    
    /** Find UClass by name (supports A, U prefixes and partial matching) */
    static UClass* FindClassByName(const FString& ClassName);
    
    /** Set a property on UObject using reflection (Actor or Component) */
    static bool SetPropertyByName(UObject* Object, const FString& PropertyName, const TSharedPtr<FJsonValue>& Value, FString& OutError);
    
    /** Get a property value from UObject as JsonValue */
    static TSharedPtr<FJsonValue> GetPropertyAsJsonValue(UObject* Object, const FString& PropertyName);
    
    /** Get all UPROPERTIES from an object as JSON object */
    static TSharedPtr<FJsonObject> GetAllPropertiesAsJson(UObject* Object);
    
    /** Find component on actor by class or name pattern */
    static UActorComponent* FindActorComponent(AActor* Actor, const FString& ComponentPattern);
    
    // ============================================================================
    // Legacy Compatibility (Deprecated - Use Generic Tools Instead)
    // ============================================================================
    UE_DEPRECATED(5.0, "Use spawn_actor with actor_class instead")
    static TSharedPtr<FJsonObject> HandleCreateLight(const TSharedPtr<FJsonObject>& Params);
    
    UE_DEPRECATED(5.0, "Use set_actor_properties instead")
    static TSharedPtr<FJsonObject> HandleSetLightProperties(const TSharedPtr<FJsonObject>& Params);
    
    UE_DEPRECATED(5.0, "Use get_actors with filter instead")
    static TSharedPtr<FJsonObject> HandleGetLights(const TSharedPtr<FJsonObject>& Params);
    
    UE_DEPRECATED(5.0, "Use delete_actor instead")
    static TSharedPtr<FJsonObject> HandleDeleteLight(const TSharedPtr<FJsonObject>& Params);
    
    UE_DEPRECATED(5.0, "Use spawn_actor with actor_class=PostProcessVolume instead")
    static TSharedPtr<FJsonObject> HandleCreatePostProcessVolume(const TSharedPtr<FJsonObject>& Params);
    
    UE_DEPRECATED(5.0, "Use set_actor_properties instead")
    static TSharedPtr<FJsonObject> HandleSetPostProcessSettings(const TSharedPtr<FJsonObject>& Params);
    
    UE_DEPRECATED(5.0, "Use spawn_actor with actor_class=StaticMeshActor and properties instead")
    static TSharedPtr<FJsonObject> HandleSpawnBasicActor(const TSharedPtr<FJsonObject>& Params);
    
    UE_DEPRECATED(5.0, "Use set_actor_properties with material property instead")
    static TSharedPtr<FJsonObject> HandleSetActorMaterial(const TSharedPtr<FJsonObject>& Params);
};
