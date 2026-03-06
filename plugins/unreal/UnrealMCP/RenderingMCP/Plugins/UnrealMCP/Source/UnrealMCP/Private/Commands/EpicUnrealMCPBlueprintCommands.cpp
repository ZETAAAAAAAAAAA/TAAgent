#include "Commands/EpicUnrealMCPBlueprintCommands.h"
#include "Commands/EpicUnrealMCPCommonUtils.h"
#include "Commands/EpicUnrealMCPEnvironmentCommands.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/StaticMesh.h"
#include "Factories/BlueprintFactory.h"
#include "EdGraphSchema_K2.h"
#include "K2Node_Event.h"
#include "K2Node_VariableGet.h"
#include "K2Node_VariableSet.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceConstant.h"
#include "MaterialExpressionIO.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionConstant2Vector.h"
#include "Materials/MaterialExpressionConstant3Vector.h"
#include "Materials/MaterialExpressionConstant4Vector.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "Materials/MaterialExpressionAdd.h"
#include "Materials/MaterialExpressionSubtract.h"
#include "Materials/MaterialExpressionDivide.h"
#include "Materials/MaterialExpressionLinearInterpolate.h"
#include "Materials/MaterialExpressionClamp.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Materials/MaterialExpressionTextureCoordinate.h"
#include "Materials/MaterialExpressionTime.h"
#include "Materials/MaterialExpressionSine.h"
#include "Materials/MaterialExpressionCosine.h"
#include "Materials/MaterialExpressionPanner.h"
#include "Materials/MaterialExpressionRotator.h"
#include "Materials/MaterialExpressionPower.h"
#include "Materials/MaterialExpressionAbs.h"
#include "Materials/MaterialExpressionFloor.h"
#include "Materials/MaterialExpressionCeil.h"
#include "Materials/MaterialExpressionFrac.h"
#include "Materials/MaterialExpressionComponentMask.h"
#include "Materials/MaterialExpressionAppendVector.h"
#include "Materials/MaterialExpressionVertexColor.h"
#include "Materials/MaterialExpressionWorldPosition.h"
#include "Materials/MaterialExpressionObjectPositionWS.h"
#include "Materials/MaterialExpressionVertexNormalWS.h"
#include "Materials/MaterialExpressionFresnel.h"
#include "Materials/MaterialExpressionDepthFade.h"
#include "Materials/MaterialExpressionCameraPositionWS.h"
#include "Materials/MaterialExpressionCameraVectorWS.h"
#include "Materials/MaterialExpressionDistance.h"
#include "Materials/MaterialExpressionPixelDepth.h"
#include "Materials/MaterialExpressionSceneDepth.h"
#include "Materials/MaterialExpressionSceneTexture.h"
#include "Materials/MaterialExpressionCustom.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Materials/MaterialExpressionTextureObjectParameter.h"
#include "Materials/MaterialExpressionTextureSampleParameter2D.h"
#include "Materials/MaterialExpressionStaticBoolParameter.h"
#include "Materials/MaterialFunction.h"
#include "Materials/MaterialExpressionFunctionInput.h"
#include "Materials/MaterialExpressionFunctionOutput.h"
#include "Materials/MaterialExpressionTransform.h"
#include "Materials/MaterialExpressionDotProduct.h"
#include "Materials/MaterialExpressionCrossProduct.h"
#include "Materials/MaterialExpressionOneMinus.h"
#include "Materials/MaterialExpressionNormalize.h"
#include "Materials/MaterialExpressionSaturate.h"
#include "Materials/MaterialExpressionReflectionVectorWS.h"
#include "Materials/MaterialExpressionVertexTangentWS.h"
#include "Materials/MaterialExpressionDesaturation.h"
#include "Materials/MaterialExpressionDDX.h"
#include "Materials/MaterialExpressionDDY.h"
#include "Materials/MaterialExpressionTextureObject.h"
#include "Materials/MaterialExpressionIf.h"
#include "Materials/MaterialExpressionParticleSubUV.h"
#include "Materials/MaterialExpressionLightVector.h"
#include "Materials/MaterialExpressionViewSize.h"
#include "Materials/MaterialExpressionPreSkinnedPosition.h"
#include "Materials/MaterialExpressionPreSkinnedNormal.h"
#include "Materials/MaterialExpressionSquareRoot.h"
#include "Materials/MaterialExpressionMin.h"
#include "Materials/MaterialExpressionMax.h"
#include "Materials/MaterialExpressionRound.h"
#include "Materials/MaterialExpressionSign.h"
#include "Materials/MaterialExpressionStep.h"
#include "Materials/MaterialExpressionSmoothStep.h"
#include "Materials/MaterialExpressionInverseLinearInterpolate.h"
#include "Materials/MaterialExpressionSubtract.h"
#include "Materials/MaterialExpressionStaticSwitch.h"
#include "Materials/MaterialExpressionDynamicParameter.h"
#include "Materials/MaterialExpressionCurveAtlasRowParameter.h"
#include "Materials/MaterialExpressionReroute.h"
#include "Materials/MaterialExpressionComment.h"
#include "Materials/MaterialExpressionMaterialFunctionCall.h"
#include "Materials/MaterialExpressionLandscapeLayerBlend.h"
#include "Materials/MaterialExpressionNormalize.h"
#include "Materials/MaterialExpressionObjectOrientation.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Factories/MaterialFactoryNew.h"
#include "Factories/TextureFactory.h"
#include "Engine/Engine.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Engine/SimpleConstructionScript.h"
#include "Engine/SCS_Node.h"
#include "UObject/Field.h"
#include "UObject/FieldPath.h"
#include "EditorAssetLibrary.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "TextureResource.h"
#include "AssetToolsModule.h"
#include "AssetImportTask.h"
#include "UObject/Package.h"

FEpicUnrealMCPBlueprintCommands::FEpicUnrealMCPBlueprintCommands()
{
}

TSharedPtr<FJsonObject> FEpicUnrealMCPBlueprintCommands::HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params)
{
    if (CommandType == TEXT("create_blueprint"))
    {
        return HandleCreateBlueprint(Params);
    }
    else if (CommandType == TEXT("add_component_to_blueprint"))
    {
        return HandleAddComponentToBlueprint(Params);
    }
    else if (CommandType == TEXT("set_physics_properties"))
    {
        return HandleSetPhysicsProperties(Params);
    }
    else if (CommandType == TEXT("compile_blueprint"))
    {
        return HandleCompileBlueprint(Params);
    }
    else if (CommandType == TEXT("set_static_mesh_properties"))
    {
        return HandleSetStaticMeshProperties(Params);
    }
    else if (CommandType == TEXT("spawn_blueprint_actor"))
    {
        return HandleSpawnBlueprintActor(Params);
    }
    else if (CommandType == TEXT("set_mesh_material_color"))
    {
        return HandleSetMeshMaterialColor(Params);
    }
    // Material management commands
    else if (CommandType == TEXT("create_material"))
    {
        return HandleCreateMaterial(Params);
    }
    else if (CommandType == TEXT("apply_material_to_actor"))
    {
        return HandleApplyMaterialToActor(Params);
    }
    else if (CommandType == TEXT("apply_material_to_blueprint"))
    {
        return HandleApplyMaterialToBlueprint(Params);
    }
    else if (CommandType == TEXT("get_actor_material_info"))
    {
        return HandleGetActorMaterialInfo(Params);
    }
    else if (CommandType == TEXT("get_blueprint_material_info"))
    {
        return HandleGetBlueprintMaterialInfo(Params);
    }
    // Material expression commands
    else if (CommandType == TEXT("add_material_expression"))
    {
        return HandleAddMaterialExpression(Params);
    }
    else if (CommandType == TEXT("connect_material_nodes"))
    {
        return HandleConnectMaterialNodes(Params);
    }
    else if (CommandType == TEXT("build_material_graph"))
    {
        return HandleBuildMaterialGraph(Params);
    }
    else if (CommandType == TEXT("set_material_properties"))
    {
        return HandleSetMaterialProperties(Params);
    }
    else if (CommandType == TEXT("compile_material"))
    {
        return HandleCompileMaterial(Params);
    }
    else if (CommandType == TEXT("get_material_graph"))
    {
        return HandleGetMaterialGraph(Params);
    }
    else if (CommandType == TEXT("create_material_function"))
    {
        return HandleCreateMaterialFunction(Params);
    }
    // Material Function commands
    else if (CommandType == TEXT("get_material_function_content"))
    {
        return HandleGetMaterialFunctionContent(Params);
    }
    // Generic asset listing
    else if (CommandType == TEXT("get_assets"))
    {
        return HandleGetAssets(Params);
    }
    // Texture import command
    else if (CommandType == TEXT("import_texture"))
    {
        return HandleImportTexture(Params);
    }
    // Static mesh asset commands
    else if (CommandType == TEXT("set_static_mesh_asset_properties"))
    {
        return HandleSetStaticMeshAssetProperties(Params);
    }
    // Blueprint analysis commands
    else if (CommandType == TEXT("read_blueprint_content"))
    {
        return HandleReadBlueprintContent(Params);
    }
    else if (CommandType == TEXT("analyze_blueprint_graph"))
    {
        return HandleAnalyzeBlueprintGraph(Params);
    }
    else if (CommandType == TEXT("get_blueprint_variable_details"))
    {
        return HandleGetBlueprintVariableDetails(Params);
    }
    else if (CommandType == TEXT("get_blueprint_function_details"))
    {
        return HandleGetBlueprintFunctionDetails(Params);
    }
    // Viewport and Actor commands - delegated to EnvironmentCommands
    else if (CommandType == TEXT("get_viewport_screenshot"))
    {
        return FEpicUnrealMCPEnvironmentCommands::HandleGetViewportScreenshot(Params);
    }
    else if (CommandType == TEXT("spawn_actor"))
    {
        return FEpicUnrealMCPEnvironmentCommands::HandleSpawnActor(Params);
    }
    else if (CommandType == TEXT("delete_actor"))
    {
        return FEpicUnrealMCPEnvironmentCommands::HandleDeleteActor(Params);
    }
    else if (CommandType == TEXT("get_actors"))
    {
        return FEpicUnrealMCPEnvironmentCommands::HandleGetActors(Params);
    }
    else if (CommandType == TEXT("set_actor_properties"))
    {
        return FEpicUnrealMCPEnvironmentCommands::HandleSetActorProperties(Params);
    }
    else if (CommandType == TEXT("get_actor_properties"))
    {
        return FEpicUnrealMCPEnvironmentCommands::HandleGetActorProperties(Params);
    }
    // Legacy compatibility - these are deprecated but kept for backward compatibility
PRAGMA_DISABLE_DEPRECATION_WARNINGS
    else if (CommandType == TEXT("create_light"))
    {
        return FEpicUnrealMCPEnvironmentCommands::HandleCreateLight(Params);
    }
    else if (CommandType == TEXT("set_light_properties"))
    {
        return FEpicUnrealMCPEnvironmentCommands::HandleSetLightProperties(Params);
    }
    else if (CommandType == TEXT("get_lights"))
    {
        return FEpicUnrealMCPEnvironmentCommands::HandleGetLights(Params);
    }
    else if (CommandType == TEXT("delete_light"))
    {
        return FEpicUnrealMCPEnvironmentCommands::HandleDeleteLight(Params);
    }
    else if (CommandType == TEXT("create_post_process_volume"))
    {
        return FEpicUnrealMCPEnvironmentCommands::HandleCreatePostProcessVolume(Params);
    }
    else if (CommandType == TEXT("set_post_process_settings"))
    {
        return FEpicUnrealMCPEnvironmentCommands::HandleSetPostProcessSettings(Params);
    }
    else if (CommandType == TEXT("spawn_basic_actor"))
    {
        return FEpicUnrealMCPEnvironmentCommands::HandleSpawnBasicActor(Params);
    }
    else if (CommandType == TEXT("set_actor_material"))
    {
        return FEpicUnrealMCPEnvironmentCommands::HandleSetActorMaterial(Params);
    }
PRAGMA_ENABLE_DEPRECATION_WARNINGS

    return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Unknown blueprint command: %s"), *CommandType));
}

TSharedPtr<FJsonObject> FEpicUnrealMCPBlueprintCommands::HandleCreateBlueprint(const TSharedPtr<FJsonObject>& Params)
{
    // Get required parameters
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("name"), BlueprintName))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'name' parameter"));
    }

    // Check if blueprint already exists
    FString PackagePath = TEXT("/Game/Blueprints/");
    FString AssetName = BlueprintName;
    if (UEditorAssetLibrary::DoesAssetExist(PackagePath + AssetName))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint already exists: %s"), *BlueprintName));
    }

    // Create the blueprint factory
    UBlueprintFactory* Factory = NewObject<UBlueprintFactory>();
    
    // Handle parent class
    FString ParentClass;
    Params->TryGetStringField(TEXT("parent_class"), ParentClass);
    
    // Default to Actor if no parent class specified
    UClass* SelectedParentClass = AActor::StaticClass();
    
    // Try to find the specified parent class
    if (!ParentClass.IsEmpty())
    {
        FString ClassName = ParentClass;
        if (!ClassName.StartsWith(TEXT("A")))
        {
            ClassName = TEXT("A") + ClassName;
        }
        
        // First try direct StaticClass lookup for common classes
        UClass* FoundClass = nullptr;
        if (ClassName == TEXT("APawn"))
        {
            FoundClass = APawn::StaticClass();
        }
        else if (ClassName == TEXT("AActor"))
        {
            FoundClass = AActor::StaticClass();
        }
        else
        {
            // Try loading the class using LoadClass which is more reliable than FindObject
            const FString ClassPath = FString::Printf(TEXT("/Script/Engine.%s"), *ClassName);
            FoundClass = LoadClass<AActor>(nullptr, *ClassPath);
            
            if (!FoundClass)
            {
                // Try alternate paths if not found
                const FString GameClassPath = FString::Printf(TEXT("/Script/Game.%s"), *ClassName);
                FoundClass = LoadClass<AActor>(nullptr, *GameClassPath);
            }
        }

        if (FoundClass)
        {
            SelectedParentClass = FoundClass;
            UE_LOG(LogTemp, Log, TEXT("Successfully set parent class to '%s'"), *ClassName);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Could not find specified parent class '%s' at paths: /Script/Engine.%s or /Script/Game.%s, defaulting to AActor"), 
                *ClassName, *ClassName, *ClassName);
        }
    }
    
    Factory->ParentClass = SelectedParentClass;

    // Create the blueprint
    UPackage* Package = CreatePackage(*(PackagePath + AssetName));
    UBlueprint* NewBlueprint = Cast<UBlueprint>(Factory->FactoryCreateNew(UBlueprint::StaticClass(), Package, *AssetName, RF_Standalone | RF_Public, nullptr, GWarn));

    if (NewBlueprint)
    {
        // Notify the asset registry
        FAssetRegistryModule::AssetCreated(NewBlueprint);

        // Mark the package dirty
        Package->MarkPackageDirty();

        TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
        ResultObj->SetStringField(TEXT("name"), AssetName);
        ResultObj->SetStringField(TEXT("path"), PackagePath + AssetName);
        return ResultObj;
    }

    return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create blueprint"));
}

TSharedPtr<FJsonObject> FEpicUnrealMCPBlueprintCommands::HandleAddComponentToBlueprint(const TSharedPtr<FJsonObject>& Params)
{
    // Get required parameters
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    FString ComponentType;
    if (!Params->TryGetStringField(TEXT("component_type"), ComponentType))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'type' parameter"));
    }

    FString ComponentName;
    if (!Params->TryGetStringField(TEXT("component_name"), ComponentName))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'name' parameter"));
    }

    // Find the blueprint
    UBlueprint* Blueprint = FEpicUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
    if (!Blueprint)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }

    // Create the component - dynamically find the component class by name
    UClass* ComponentClass = nullptr;

    // Try to find the class with exact name first
    ComponentClass = FindObject<UClass>(nullptr, *ComponentType);
    
    // If not found, try with "Component" suffix
    if (!ComponentClass && !ComponentType.EndsWith(TEXT("Component")))
    {
        FString ComponentTypeWithSuffix = ComponentType + TEXT("Component");
        ComponentClass = FindObject<UClass>(nullptr, *ComponentTypeWithSuffix);
    }
    
    // If still not found, try with "U" prefix
    if (!ComponentClass && !ComponentType.StartsWith(TEXT("U")))
    {
        FString ComponentTypeWithPrefix = TEXT("U") + ComponentType;
        ComponentClass = FindObject<UClass>(nullptr, *ComponentTypeWithPrefix);
        
        // Try with both prefix and suffix
        if (!ComponentClass && !ComponentType.EndsWith(TEXT("Component")))
        {
            FString ComponentTypeWithBoth = TEXT("U") + ComponentType + TEXT("Component");
            ComponentClass = FindObject<UClass>(nullptr, *ComponentTypeWithBoth);
        }
    }
    
    // Verify that the class is a valid component type
    if (!ComponentClass || !ComponentClass->IsChildOf(UActorComponent::StaticClass()))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Unknown component type: %s"), *ComponentType));
    }

    // Add the component to the blueprint
    USCS_Node* NewNode = Blueprint->SimpleConstructionScript->CreateNode(ComponentClass, *ComponentName);
    if (NewNode)
    {
        // Set transform if provided
        USceneComponent* SceneComponent = Cast<USceneComponent>(NewNode->ComponentTemplate);
        if (SceneComponent)
        {
            if (Params->HasField(TEXT("location")))
            {
                SceneComponent->SetRelativeLocation(FEpicUnrealMCPCommonUtils::GetVectorFromJson(Params, TEXT("location")));
            }
            if (Params->HasField(TEXT("rotation")))
            {
                SceneComponent->SetRelativeRotation(FEpicUnrealMCPCommonUtils::GetRotatorFromJson(Params, TEXT("rotation")));
            }
            if (Params->HasField(TEXT("scale")))
            {
                SceneComponent->SetRelativeScale3D(FEpicUnrealMCPCommonUtils::GetVectorFromJson(Params, TEXT("scale")));
            }
        }

        // Add to root if no parent specified
        Blueprint->SimpleConstructionScript->AddNode(NewNode);

        // Compile the blueprint
        FKismetEditorUtilities::CompileBlueprint(Blueprint);

        TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
        ResultObj->SetStringField(TEXT("component_name"), ComponentName);
        ResultObj->SetStringField(TEXT("component_type"), ComponentType);
        return ResultObj;
    }

    return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to add component to blueprint"));
}

TSharedPtr<FJsonObject> FEpicUnrealMCPBlueprintCommands::HandleSetPhysicsProperties(const TSharedPtr<FJsonObject>& Params)
{
    // Get required parameters
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    FString ComponentName;
    if (!Params->TryGetStringField(TEXT("component_name"), ComponentName))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'component_name' parameter"));
    }

    // Find the blueprint
    UBlueprint* Blueprint = FEpicUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
    if (!Blueprint)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }

    // Find the component
    USCS_Node* ComponentNode = nullptr;
    for (USCS_Node* Node : Blueprint->SimpleConstructionScript->GetAllNodes())
    {
        if (Node && Node->GetVariableName().ToString() == ComponentName)
        {
            ComponentNode = Node;
            break;
        }
    }

    if (!ComponentNode)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Component not found: %s"), *ComponentName));
    }

    UPrimitiveComponent* PrimComponent = Cast<UPrimitiveComponent>(ComponentNode->ComponentTemplate);
    if (!PrimComponent)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Component is not a primitive component"));
    }

    // Set physics properties
    if (Params->HasField(TEXT("simulate_physics")))
    {
        PrimComponent->SetSimulatePhysics(Params->GetBoolField(TEXT("simulate_physics")));
    }

    if (Params->HasField(TEXT("mass")))
    {
        float Mass = Params->GetNumberField(TEXT("mass"));
        // In UE5.5, use proper overrideMass instead of just scaling
        PrimComponent->SetMassOverrideInKg(NAME_None, Mass);
        UE_LOG(LogTemp, Display, TEXT("Set mass for component %s to %f kg"), *ComponentName, Mass);
    }

    if (Params->HasField(TEXT("linear_damping")))
    {
        PrimComponent->SetLinearDamping(Params->GetNumberField(TEXT("linear_damping")));
    }

    if (Params->HasField(TEXT("angular_damping")))
    {
        PrimComponent->SetAngularDamping(Params->GetNumberField(TEXT("angular_damping")));
    }

    // Mark the blueprint as modified
    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetStringField(TEXT("component"), ComponentName);
    return ResultObj;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPBlueprintCommands::HandleCompileBlueprint(const TSharedPtr<FJsonObject>& Params)
{
    // Get required parameters
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    // Find the blueprint
    UBlueprint* Blueprint = FEpicUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
    if (!Blueprint)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }

    // Compile the blueprint
    FKismetEditorUtilities::CompileBlueprint(Blueprint);

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetStringField(TEXT("name"), BlueprintName);
    ResultObj->SetBoolField(TEXT("compiled"), true);
    return ResultObj;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPBlueprintCommands::HandleSpawnBlueprintActor(const TSharedPtr<FJsonObject>& Params)
{
    UE_LOG(LogTemp, Warning, TEXT("HandleSpawnBlueprintActor: Starting blueprint actor spawn"));
    
    // Get required parameters
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        UE_LOG(LogTemp, Error, TEXT("HandleSpawnBlueprintActor: Missing blueprint_name parameter"));
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    FString ActorName;
    if (!Params->TryGetStringField(TEXT("actor_name"), ActorName))
    {
        UE_LOG(LogTemp, Error, TEXT("HandleSpawnBlueprintActor: Missing actor_name parameter"));
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'actor_name' parameter"));
    }

    UE_LOG(LogTemp, Warning, TEXT("HandleSpawnBlueprintActor: Looking for blueprint '%s'"), *BlueprintName);

    // Find the blueprint
    UBlueprint* Blueprint = FEpicUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
    if (!Blueprint)
    {
        UE_LOG(LogTemp, Error, TEXT("HandleSpawnBlueprintActor: Blueprint not found: %s"), *BlueprintName);
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }

    UE_LOG(LogTemp, Warning, TEXT("HandleSpawnBlueprintActor: Blueprint found, getting transform parameters"));

    // Get transform parameters
    FVector Location(0.0f, 0.0f, 0.0f);
    FRotator Rotation(0.0f, 0.0f, 0.0f);

    if (Params->HasField(TEXT("location")))
    {
        Location = FEpicUnrealMCPCommonUtils::GetVectorFromJson(Params, TEXT("location"));
        UE_LOG(LogTemp, Warning, TEXT("HandleSpawnBlueprintActor: Location set to (%f, %f, %f)"), Location.X, Location.Y, Location.Z);
    }
    if (Params->HasField(TEXT("rotation")))
    {
        Rotation = FEpicUnrealMCPCommonUtils::GetRotatorFromJson(Params, TEXT("rotation"));
        UE_LOG(LogTemp, Warning, TEXT("HandleSpawnBlueprintActor: Rotation set to (%f, %f, %f)"), Rotation.Pitch, Rotation.Yaw, Rotation.Roll);
    }

    UE_LOG(LogTemp, Warning, TEXT("HandleSpawnBlueprintActor: Getting editor world"));

    // Spawn the actor
    UWorld* World = GEditor->GetEditorWorldContext().World();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("HandleSpawnBlueprintActor: Failed to get editor world"));
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to get editor world"));
    }

    UE_LOG(LogTemp, Warning, TEXT("HandleSpawnBlueprintActor: Creating spawn transform"));

    FTransform SpawnTransform;
    SpawnTransform.SetLocation(Location);
    SpawnTransform.SetRotation(FQuat(Rotation));

    // Add a small delay to allow the engine to process the newly compiled class
    FPlatformProcess::Sleep(0.2f);

    UE_LOG(LogTemp, Warning, TEXT("HandleSpawnBlueprintActor: About to spawn actor from blueprint '%s' with GeneratedClass: %s"), 
           *BlueprintName, Blueprint->GeneratedClass ? *Blueprint->GeneratedClass->GetName() : TEXT("NULL"));

    AActor* NewActor = World->SpawnActor<AActor>(Blueprint->GeneratedClass, SpawnTransform);
    
    UE_LOG(LogTemp, Warning, TEXT("HandleSpawnBlueprintActor: SpawnActor completed, NewActor: %s"), 
           NewActor ? *NewActor->GetName() : TEXT("NULL"));
    
    if (NewActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("HandleSpawnBlueprintActor: Setting actor label to '%s'"), *ActorName);
        NewActor->SetActorLabel(*ActorName);
        
        UE_LOG(LogTemp, Warning, TEXT("HandleSpawnBlueprintActor: About to convert actor to JSON"));
        TSharedPtr<FJsonObject> Result = FEpicUnrealMCPCommonUtils::ActorToJsonObject(NewActor, true);
        
        UE_LOG(LogTemp, Warning, TEXT("HandleSpawnBlueprintActor: JSON conversion completed, returning result"));
        return Result;
    }

    UE_LOG(LogTemp, Error, TEXT("HandleSpawnBlueprintActor: Failed to spawn blueprint actor"));
    return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to spawn blueprint actor"));
}

TSharedPtr<FJsonObject> FEpicUnrealMCPBlueprintCommands::HandleSetStaticMeshProperties(const TSharedPtr<FJsonObject>& Params)
{
    // Get required parameters
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    FString ComponentName;
    if (!Params->TryGetStringField(TEXT("component_name"), ComponentName))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'component_name' parameter"));
    }

    // Find the blueprint
    UBlueprint* Blueprint = FEpicUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
    if (!Blueprint)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }

    // Find the component
    USCS_Node* ComponentNode = nullptr;
    for (USCS_Node* Node : Blueprint->SimpleConstructionScript->GetAllNodes())
    {
        if (Node && Node->GetVariableName().ToString() == ComponentName)
        {
            ComponentNode = Node;
            break;
        }
    }

    if (!ComponentNode)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Component not found: %s"), *ComponentName));
    }

    UStaticMeshComponent* MeshComponent = Cast<UStaticMeshComponent>(ComponentNode->ComponentTemplate);
    if (!MeshComponent)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Component is not a static mesh component"));
    }

    // Set static mesh properties
    if (Params->HasField(TEXT("static_mesh")))
    {
        FString MeshPath = Params->GetStringField(TEXT("static_mesh"));
        UStaticMesh* Mesh = Cast<UStaticMesh>(UEditorAssetLibrary::LoadAsset(MeshPath));
        if (Mesh)
        {
            MeshComponent->SetStaticMesh(Mesh);
        }
    }

    if (Params->HasField(TEXT("material")))
    {
        FString MaterialPath = Params->GetStringField(TEXT("material"));
        UMaterialInterface* Material = Cast<UMaterialInterface>(UEditorAssetLibrary::LoadAsset(MaterialPath));
        if (Material)
        {
            MeshComponent->SetMaterial(0, Material);
        }
    }

    // Mark the blueprint as modified
    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetStringField(TEXT("component"), ComponentName);
    return ResultObj;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPBlueprintCommands::HandleSetMeshMaterialColor(const TSharedPtr<FJsonObject>& Params)
{
    // Get required parameters
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    FString ComponentName;
    if (!Params->TryGetStringField(TEXT("component_name"), ComponentName))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'component_name' parameter"));
    }

    // Find the blueprint
    UBlueprint* Blueprint = FEpicUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
    if (!Blueprint)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }

    // Find the component
    USCS_Node* ComponentNode = nullptr;
    for (USCS_Node* Node : Blueprint->SimpleConstructionScript->GetAllNodes())
    {
        if (Node && Node->GetVariableName().ToString() == ComponentName)
        {
            ComponentNode = Node;
            break;
        }
    }

    if (!ComponentNode)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Component not found: %s"), *ComponentName));
    }

    // Try to cast to StaticMeshComponent or PrimitiveComponent
    UPrimitiveComponent* PrimComponent = Cast<UPrimitiveComponent>(ComponentNode->ComponentTemplate);
    if (!PrimComponent)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Component is not a primitive component"));
    }

    // Get color parameter
    TArray<float> ColorArray;
    const TArray<TSharedPtr<FJsonValue>>* ColorJsonArray;
    if (!Params->TryGetArrayField(TEXT("color"), ColorJsonArray) || ColorJsonArray->Num() != 4)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("'color' must be an array of 4 float values [R, G, B, A]"));
    }

    for (const TSharedPtr<FJsonValue>& Value : *ColorJsonArray)
    {
        ColorArray.Add(FMath::Clamp(Value->AsNumber(), 0.0f, 1.0f));
    }

    FLinearColor Color(ColorArray[0], ColorArray[1], ColorArray[2], ColorArray[3]);

    // Get material slot index
    int32 MaterialSlot = 0;
    if (Params->HasField(TEXT("material_slot")))
    {
        MaterialSlot = Params->GetIntegerField(TEXT("material_slot"));
    }

    // Get parameter name
    FString ParameterName = TEXT("BaseColor");
    Params->TryGetStringField(TEXT("parameter_name"), ParameterName);

    // Get or create material
    UMaterialInterface* Material = nullptr;
    
    // Check if a specific material path was provided
    FString MaterialPath;
    if (Params->TryGetStringField(TEXT("material_path"), MaterialPath))
    {
        Material = Cast<UMaterialInterface>(UEditorAssetLibrary::LoadAsset(MaterialPath));
        if (!Material)
        {
            return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Failed to load material: %s"), *MaterialPath));
        }
    }
    else
    {
        // Use existing material on the component
        Material = PrimComponent->GetMaterial(MaterialSlot);
        if (!Material)
        {
            // Try to use a default material
            Material = Cast<UMaterialInterface>(UEditorAssetLibrary::LoadAsset(TEXT("/Engine/BasicShapes/BasicShapeMaterial")));
            if (!Material)
            {
                return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("No material found on component and failed to load default material"));
            }
        }
    }

    // Create a dynamic material instance
    UMaterialInstanceDynamic* DynMaterial = UMaterialInstanceDynamic::Create(Material, PrimComponent);
    if (!DynMaterial)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create dynamic material instance"));
    }

    // Set the color parameter
    DynMaterial->SetVectorParameterValue(*ParameterName, Color);

    // Apply the material to the component
    PrimComponent->SetMaterial(MaterialSlot, DynMaterial);

    // Mark the blueprint as modified
    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

    // Log success
    UE_LOG(LogTemp, Log, TEXT("Successfully set material color on component %s: R=%f, G=%f, B=%f, A=%f"), 
        *ComponentName, Color.R, Color.G, Color.B, Color.A);

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetStringField(TEXT("component"), ComponentName);
    ResultObj->SetNumberField(TEXT("material_slot"), MaterialSlot);
    ResultObj->SetStringField(TEXT("parameter_name"), ParameterName);
    
    TArray<TSharedPtr<FJsonValue>> ColorResultArray;
    ColorResultArray.Add(MakeShared<FJsonValueNumber>(Color.R));
    ColorResultArray.Add(MakeShared<FJsonValueNumber>(Color.G));
    ColorResultArray.Add(MakeShared<FJsonValueNumber>(Color.B));
    ColorResultArray.Add(MakeShared<FJsonValueNumber>(Color.A));
    ResultObj->SetArrayField(TEXT("color"), ColorResultArray);
    
    ResultObj->SetBoolField(TEXT("success"), true);
    return ResultObj;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPBlueprintCommands::HandleGetAvailableMaterials(const TSharedPtr<FJsonObject>& Params)
{
    // Get parameters - make search path completely dynamic
    FString SearchPath;
    if (!Params->TryGetStringField(TEXT("search_path"), SearchPath))
    {
        // Default to empty string to search everywhere
        SearchPath = TEXT("");
    }
    
    bool bIncludeEngineMaterials = true;
    if (Params->HasField(TEXT("include_engine_materials")))
    {
        bIncludeEngineMaterials = Params->GetBoolField(TEXT("include_engine_materials"));
    }

    // Get asset registry module
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
    IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

    // Create filter for materials
    FARFilter Filter;
    Filter.ClassPaths.Add(UMaterialInterface::StaticClass()->GetClassPathName());
    Filter.ClassPaths.Add(UMaterial::StaticClass()->GetClassPathName());
    Filter.ClassPaths.Add(UMaterialInstanceConstant::StaticClass()->GetClassPathName());
    Filter.ClassPaths.Add(UMaterialInstanceDynamic::StaticClass()->GetClassPathName());
    
    // Add search paths dynamically
    if (!SearchPath.IsEmpty())
    {
        // Ensure the path starts with /
        if (!SearchPath.StartsWith(TEXT("/")))
        {
            SearchPath = TEXT("/") + SearchPath;
        }
        // Ensure the path ends with / for proper directory search
        if (!SearchPath.EndsWith(TEXT("/")))
        {
            SearchPath += TEXT("/");
        }
        Filter.PackagePaths.Add(*SearchPath);
        UE_LOG(LogTemp, Log, TEXT("Searching for materials in: %s"), *SearchPath);
    }
    else
    {
        // Search in common game content locations
        Filter.PackagePaths.Add(TEXT("/Game/"));
        UE_LOG(LogTemp, Log, TEXT("Searching for materials in all game content"));
    }
    
    if (bIncludeEngineMaterials)
    {
        Filter.PackagePaths.Add(TEXT("/Engine/"));
        UE_LOG(LogTemp, Log, TEXT("Including Engine materials in search"));
    }
    
    Filter.bRecursivePaths = true;

    // Get assets from registry
    TArray<FAssetData> AssetDataArray;
    AssetRegistry.GetAssets(Filter, AssetDataArray);
    
    UE_LOG(LogTemp, Log, TEXT("Asset registry found %d materials"), AssetDataArray.Num());

    // Also try manual search using EditorAssetLibrary for more comprehensive results
    TArray<FString> AllAssetPaths;
    if (!SearchPath.IsEmpty())
    {
        AllAssetPaths = UEditorAssetLibrary::ListAssets(SearchPath, true, false);
    }
    else
    {
        AllAssetPaths = UEditorAssetLibrary::ListAssets(TEXT("/Game/"), true, false);
    }
    
    // Filter for materials from the manual search
    for (const FString& AssetPath : AllAssetPaths)
    {
        if (AssetPath.Contains(TEXT("Material")) && !AssetPath.Contains(TEXT(".uasset")))
        {
            UObject* Asset = UEditorAssetLibrary::LoadAsset(AssetPath);
            if (Asset && Asset->IsA<UMaterialInterface>())
            {
                // Check if we already have this asset from registry search
                bool bAlreadyFound = false;
                for (const FAssetData& ExistingData : AssetDataArray)
                {
                    if (ExistingData.GetObjectPathString() == AssetPath)
                    {
                        bAlreadyFound = true;
                        break;
                    }
                }
                
                if (!bAlreadyFound)
                {
                    // Create FAssetData manually for this asset
                    FAssetData ManualAssetData(Asset);
                    AssetDataArray.Add(ManualAssetData);
                }
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Total materials found after manual search: %d"), AssetDataArray.Num());

    // Convert to JSON
    TArray<TSharedPtr<FJsonValue>> MaterialArray;
    for (const FAssetData& AssetData : AssetDataArray)
    {
        TSharedPtr<FJsonObject> MaterialObj = MakeShared<FJsonObject>();
        MaterialObj->SetStringField(TEXT("name"), AssetData.AssetName.ToString());
        MaterialObj->SetStringField(TEXT("path"), AssetData.GetObjectPathString());
        MaterialObj->SetStringField(TEXT("package"), AssetData.PackageName.ToString());
        MaterialObj->SetStringField(TEXT("class"), AssetData.AssetClassPath.ToString());
        
        MaterialArray.Add(MakeShared<FJsonValueObject>(MaterialObj));
        
        UE_LOG(LogTemp, Verbose, TEXT("Found material: %s at %s"), *AssetData.AssetName.ToString(), *AssetData.GetObjectPathString());
    }

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetArrayField(TEXT("materials"), MaterialArray);
    ResultObj->SetNumberField(TEXT("count"), MaterialArray.Num());
    ResultObj->SetStringField(TEXT("search_path_used"), SearchPath.IsEmpty() ? TEXT("/Game/") : SearchPath);
    
    return ResultObj;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPBlueprintCommands::HandleGetAssets(const TSharedPtr<FJsonObject>& Params)
{
    // Get search path (optional, defaults to /Game/)
    FString SearchPath;
    if (!Params->TryGetStringField(TEXT("path"), SearchPath))
    {
        SearchPath = TEXT("/Game/");
    }
    
    // Get optional asset class filter
    FString AssetClass;
    Params->TryGetStringField(TEXT("asset_class"), AssetClass);
    
    // Get optional name filter
    FString NameFilter;
    Params->TryGetStringField(TEXT("name_filter"), NameFilter);

    // Get asset registry module
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
    IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

    // Build asset data filter
    FARFilter Filter;
    Filter.PackagePaths.Add(*SearchPath);
    Filter.bRecursivePaths = true;
    Filter.bRecursiveClasses = true;
    
    // Map asset class names to UE class paths
    if (!AssetClass.IsEmpty())
    {
        FString LowerClass = AssetClass.ToLower();
        if (LowerClass == TEXT("material"))
        {
            Filter.ClassPaths.Add(UMaterial::StaticClass()->GetClassPathName());
            Filter.ClassPaths.Add(UMaterialInstance::StaticClass()->GetClassPathName());
        }
        else if (LowerClass == TEXT("materialfunction") || LowerClass == TEXT("material_function"))
        {
            Filter.ClassPaths.Add(FTopLevelAssetPath(TEXT("/Script/Engine.MaterialFunction")));
        }
        else if (LowerClass == TEXT("texture"))
        {
            Filter.ClassPaths.Add(UTexture::StaticClass()->GetClassPathName());
        }
        else if (LowerClass == TEXT("staticmesh") || LowerClass == TEXT("static_mesh"))
        {
            Filter.ClassPaths.Add(UStaticMesh::StaticClass()->GetClassPathName());
        }
        else if (LowerClass == TEXT("blueprint"))
        {
            Filter.ClassPaths.Add(FTopLevelAssetPath(TEXT("/Script/Engine.Blueprint")));
        }
    }

    TArray<FAssetData> AssetDataList;
    AssetRegistry.GetAssets(Filter, AssetDataList);

    TArray<TSharedPtr<FJsonValue>> AssetsArray;
    
    for (const FAssetData& AssetData : AssetDataList)
    {
        FString AssetName = AssetData.AssetName.ToString();
        
        // Apply name filter if specified
        if (!NameFilter.IsEmpty() && !AssetName.Contains(NameFilter))
        {
            continue;
        }
        
        TSharedPtr<FJsonObject> AssetObj = MakeShared<FJsonObject>();
        AssetObj->SetStringField(TEXT("name"), AssetName);
        AssetObj->SetStringField(TEXT("path"), AssetData.GetObjectPathString());
        AssetObj->SetStringField(TEXT("class"), AssetData.AssetClassPath.ToString());
        AssetObj->SetStringField(TEXT("package"), AssetData.PackageName.ToString());
        
        AssetsArray.Add(MakeShared<FJsonValueObject>(AssetObj));
    }

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetArrayField(TEXT("assets"), AssetsArray);
    ResultObj->SetNumberField(TEXT("count"), AssetsArray.Num());
    ResultObj->SetStringField(TEXT("path"), SearchPath);
    if (!AssetClass.IsEmpty())
    {
        ResultObj->SetStringField(TEXT("asset_class"), AssetClass);
    }
    ResultObj->SetBoolField(TEXT("success"), true);

    return ResultObj;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPBlueprintCommands::HandleApplyMaterialToActor(const TSharedPtr<FJsonObject>& Params)
{
    // Get required parameters
    FString ActorName;
    if (!Params->TryGetStringField(TEXT("actor_name"), ActorName))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'actor_name' parameter"));
    }

    FString MaterialPath;
    if (!Params->TryGetStringField(TEXT("material_path"), MaterialPath))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'material_path' parameter"));
    }

    int32 MaterialSlot = 0;
    if (Params->HasField(TEXT("material_slot")))
    {
        MaterialSlot = Params->GetIntegerField(TEXT("material_slot"));
    }

    // Find the actor
    AActor* TargetActor = nullptr;
    UWorld* World = GEditor->GetEditorWorldContext().World();
    if (!World)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to get editor world"));
    }
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetName() == ActorName)
        {
            TargetActor = Actor;
            break;
        }
    }

    if (!TargetActor)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Actor not found: %s"), *ActorName));
    }

    // Load the material
    UMaterialInterface* Material = Cast<UMaterialInterface>(UEditorAssetLibrary::LoadAsset(MaterialPath));
    if (!Material)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Failed to load material: %s"), *MaterialPath));
    }

    // Find mesh components and apply material
    TArray<UStaticMeshComponent*> MeshComponents;
    TargetActor->GetComponents<UStaticMeshComponent>(MeshComponents);
    
    bool bAppliedToAny = false;
    for (UStaticMeshComponent* MeshComp : MeshComponents)
    {
        if (MeshComp)
        {
            MeshComp->SetMaterial(MaterialSlot, Material);
            bAppliedToAny = true;
        }
    }

    if (!bAppliedToAny)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("No mesh components found on actor"));
    }

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetStringField(TEXT("actor_name"), ActorName);
    ResultObj->SetStringField(TEXT("material_path"), MaterialPath);
    ResultObj->SetNumberField(TEXT("material_slot"), MaterialSlot);
    ResultObj->SetBoolField(TEXT("success"), true);
    
    return ResultObj;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPBlueprintCommands::HandleApplyMaterialToBlueprint(const TSharedPtr<FJsonObject>& Params)
{
    // Get required parameters
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    FString ComponentName;
    if (!Params->TryGetStringField(TEXT("component_name"), ComponentName))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'component_name' parameter"));
    }

    FString MaterialPath;
    if (!Params->TryGetStringField(TEXT("material_path"), MaterialPath))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'material_path' parameter"));
    }

    int32 MaterialSlot = 0;
    if (Params->HasField(TEXT("material_slot")))
    {
        MaterialSlot = Params->GetIntegerField(TEXT("material_slot"));
    }

    // Find the blueprint
    UBlueprint* Blueprint = FEpicUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
    if (!Blueprint)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }

    // Find the component
    USCS_Node* ComponentNode = nullptr;
    for (USCS_Node* Node : Blueprint->SimpleConstructionScript->GetAllNodes())
    {
        if (Node && Node->GetVariableName().ToString() == ComponentName)
        {
            ComponentNode = Node;
            break;
        }
    }

    if (!ComponentNode)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Component not found: %s"), *ComponentName));
    }

    UPrimitiveComponent* PrimComponent = Cast<UPrimitiveComponent>(ComponentNode->ComponentTemplate);
    if (!PrimComponent)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Component is not a primitive component"));
    }

    // Load the material
    UMaterialInterface* Material = Cast<UMaterialInterface>(UEditorAssetLibrary::LoadAsset(MaterialPath));
    if (!Material)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Failed to load material: %s"), *MaterialPath));
    }

    // Apply the material
    PrimComponent->SetMaterial(MaterialSlot, Material);

    // Mark the blueprint as modified
    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetStringField(TEXT("blueprint_name"), BlueprintName);
    ResultObj->SetStringField(TEXT("component_name"), ComponentName);
    ResultObj->SetStringField(TEXT("material_path"), MaterialPath);
    ResultObj->SetNumberField(TEXT("material_slot"), MaterialSlot);
    ResultObj->SetBoolField(TEXT("success"), true);
    
    return ResultObj;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPBlueprintCommands::HandleGetActorMaterialInfo(const TSharedPtr<FJsonObject>& Params)
{
    // Get required parameters
    FString ActorName;
    if (!Params->TryGetStringField(TEXT("actor_name"), ActorName))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'actor_name' parameter"));
    }

    // Find the actor
    AActor* TargetActor = nullptr;
    UWorld* World = GEditor->GetEditorWorldContext().World();
    if (!World)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to get editor world"));
    }
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetName() == ActorName)
        {
            TargetActor = Actor;
            break;
        }
    }

    if (!TargetActor)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Actor not found: %s"), *ActorName));
    }

    // Get mesh components and their materials
    TArray<UStaticMeshComponent*> MeshComponents;
    TargetActor->GetComponents<UStaticMeshComponent>(MeshComponents);
    
    TArray<TSharedPtr<FJsonValue>> MaterialSlots;
    
    for (UStaticMeshComponent* MeshComp : MeshComponents)
    {
        if (MeshComp)
        {
            for (int32 i = 0; i < MeshComp->GetNumMaterials(); i++)
            {
                TSharedPtr<FJsonObject> SlotInfo = MakeShared<FJsonObject>();
                SlotInfo->SetNumberField(TEXT("slot"), i);
                SlotInfo->SetStringField(TEXT("component"), MeshComp->GetName());
                
                UMaterialInterface* Material = MeshComp->GetMaterial(i);
                if (Material)
                {
                    SlotInfo->SetStringField(TEXT("material_name"), Material->GetName());
                    SlotInfo->SetStringField(TEXT("material_path"), Material->GetPathName());
                    SlotInfo->SetStringField(TEXT("material_class"), Material->GetClass()->GetName());
                }
                else
                {
                    SlotInfo->SetStringField(TEXT("material_name"), TEXT("None"));
                    SlotInfo->SetStringField(TEXT("material_path"), TEXT(""));
                    SlotInfo->SetStringField(TEXT("material_class"), TEXT(""));
                }
                
                MaterialSlots.Add(MakeShared<FJsonValueObject>(SlotInfo));
            }
        }
    }

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetStringField(TEXT("actor_name"), ActorName);
    ResultObj->SetArrayField(TEXT("material_slots"), MaterialSlots);
    ResultObj->SetNumberField(TEXT("total_slots"), MaterialSlots.Num());
    
    return ResultObj;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPBlueprintCommands::HandleGetBlueprintMaterialInfo(const TSharedPtr<FJsonObject>& Params)
{
    // Get required parameters
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    FString ComponentName;
    if (!Params->TryGetStringField(TEXT("component_name"), ComponentName))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'component_name' parameter"));
    }

    // Find the blueprint
    UBlueprint* Blueprint = FEpicUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
    if (!Blueprint)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }

    // Find the component
    USCS_Node* ComponentNode = nullptr;
    for (USCS_Node* Node : Blueprint->SimpleConstructionScript->GetAllNodes())
    {
        if (Node && Node->GetVariableName().ToString() == ComponentName)
        {
            ComponentNode = Node;
            break;
        }
    }

    if (!ComponentNode)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Component not found: %s"), *ComponentName));
    }

    UStaticMeshComponent* MeshComponent = Cast<UStaticMeshComponent>(ComponentNode->ComponentTemplate);
    if (!MeshComponent)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Component is not a static mesh component"));
    }

    // Get material slot information
    TArray<TSharedPtr<FJsonValue>> MaterialSlots;
    int32 NumMaterials = 0;
    
    // Check if we have a static mesh assigned
    UStaticMesh* StaticMesh = MeshComponent->GetStaticMesh();
    if (StaticMesh)
    {
        NumMaterials = StaticMesh->GetNumSections(0); // Get number of material slots for LOD 0
        
        for (int32 i = 0; i < NumMaterials; i++)
        {
            TSharedPtr<FJsonObject> SlotInfo = MakeShared<FJsonObject>();
            SlotInfo->SetNumberField(TEXT("slot"), i);
            SlotInfo->SetStringField(TEXT("component"), ComponentName);
            
            UMaterialInterface* Material = MeshComponent->GetMaterial(i);
            if (Material)
            {
                SlotInfo->SetStringField(TEXT("material_name"), Material->GetName());
                SlotInfo->SetStringField(TEXT("material_path"), Material->GetPathName());
                SlotInfo->SetStringField(TEXT("material_class"), Material->GetClass()->GetName());
            }
            else
            {
                SlotInfo->SetStringField(TEXT("material_name"), TEXT("None"));
                SlotInfo->SetStringField(TEXT("material_path"), TEXT(""));
                SlotInfo->SetStringField(TEXT("material_class"), TEXT(""));
            }
            
            MaterialSlots.Add(MakeShared<FJsonValueObject>(SlotInfo));
        }
    }
    else
    {
        // If no static mesh is assigned, we can't determine material slots
        UE_LOG(LogTemp, Warning, TEXT("No static mesh assigned to component %s in blueprint %s"), *ComponentName, *BlueprintName);
    }

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetStringField(TEXT("blueprint_name"), BlueprintName);
    ResultObj->SetStringField(TEXT("component_name"), ComponentName);
    ResultObj->SetArrayField(TEXT("material_slots"), MaterialSlots);
    ResultObj->SetNumberField(TEXT("total_slots"), MaterialSlots.Num());
    ResultObj->SetBoolField(TEXT("has_static_mesh"), StaticMesh != nullptr);
    
    return ResultObj;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPBlueprintCommands::HandleReadBlueprintContent(const TSharedPtr<FJsonObject>& Params)
{
    // Get required parameters
    FString BlueprintPath;
    if (!Params->TryGetStringField(TEXT("blueprint_path"), BlueprintPath))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_path' parameter"));
    }

    // Get optional parameters
    bool bIncludeEventGraph = true;
    bool bIncludeFunctions = true;
    bool bIncludeVariables = true;
    bool bIncludeComponents = true;
    bool bIncludeInterfaces = true;

    Params->TryGetBoolField(TEXT("include_event_graph"), bIncludeEventGraph);
    Params->TryGetBoolField(TEXT("include_functions"), bIncludeFunctions);
    Params->TryGetBoolField(TEXT("include_variables"), bIncludeVariables);
    Params->TryGetBoolField(TEXT("include_components"), bIncludeComponents);
    Params->TryGetBoolField(TEXT("include_interfaces"), bIncludeInterfaces);

    // Load the blueprint
    UBlueprint* Blueprint = Cast<UBlueprint>(UEditorAssetLibrary::LoadAsset(BlueprintPath));
    if (!Blueprint)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Failed to load blueprint: %s"), *BlueprintPath));
    }

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetStringField(TEXT("blueprint_path"), BlueprintPath);
    ResultObj->SetStringField(TEXT("blueprint_name"), Blueprint->GetName());
    ResultObj->SetStringField(TEXT("parent_class"), Blueprint->ParentClass ? Blueprint->ParentClass->GetName() : TEXT("None"));

    // Include variables if requested
    if (bIncludeVariables)
    {
        TArray<TSharedPtr<FJsonValue>> VariableArray;
        for (const FBPVariableDescription& Variable : Blueprint->NewVariables)
        {
            TSharedPtr<FJsonObject> VarObj = MakeShared<FJsonObject>();
            VarObj->SetStringField(TEXT("name"), Variable.VarName.ToString());
            VarObj->SetStringField(TEXT("type"), Variable.VarType.PinCategory.ToString());
            VarObj->SetStringField(TEXT("default_value"), Variable.DefaultValue);
            VarObj->SetBoolField(TEXT("is_editable"), (Variable.PropertyFlags & CPF_Edit) != 0);
            VariableArray.Add(MakeShared<FJsonValueObject>(VarObj));
        }
        ResultObj->SetArrayField(TEXT("variables"), VariableArray);
    }

    // Include functions if requested
    if (bIncludeFunctions)
    {
        TArray<TSharedPtr<FJsonValue>> FunctionArray;
        for (UEdGraph* Graph : Blueprint->FunctionGraphs)
        {
            if (Graph)
            {
                TSharedPtr<FJsonObject> FuncObj = MakeShared<FJsonObject>();
                FuncObj->SetStringField(TEXT("name"), Graph->GetName());
                FuncObj->SetStringField(TEXT("graph_type"), TEXT("Function"));
                
                // Count nodes in function
                int32 NodeCount = Graph->Nodes.Num();
                FuncObj->SetNumberField(TEXT("node_count"), NodeCount);
                
                FunctionArray.Add(MakeShared<FJsonValueObject>(FuncObj));
            }
        }
        ResultObj->SetArrayField(TEXT("functions"), FunctionArray);
    }

    // Include event graph if requested
    if (bIncludeEventGraph)
    {
        TSharedPtr<FJsonObject> EventGraphObj = MakeShared<FJsonObject>();
        
        // Find the main event graph
        for (UEdGraph* Graph : Blueprint->UbergraphPages)
        {
            if (Graph && Graph->GetName() == TEXT("EventGraph"))
            {
                EventGraphObj->SetStringField(TEXT("name"), Graph->GetName());
                EventGraphObj->SetNumberField(TEXT("node_count"), Graph->Nodes.Num());
                
                // Get basic node information
                TArray<TSharedPtr<FJsonValue>> NodeArray;
                for (UEdGraphNode* Node : Graph->Nodes)
                {
                    if (Node)
                    {
                        TSharedPtr<FJsonObject> NodeObj = MakeShared<FJsonObject>();
                        NodeObj->SetStringField(TEXT("name"), Node->GetName());
                        NodeObj->SetStringField(TEXT("class"), Node->GetClass()->GetName());
                        NodeObj->SetStringField(TEXT("title"), Node->GetNodeTitle(ENodeTitleType::FullTitle).ToString());
                        NodeArray.Add(MakeShared<FJsonValueObject>(NodeObj));
                    }
                }
                EventGraphObj->SetArrayField(TEXT("nodes"), NodeArray);
                break;
            }
        }
        
        ResultObj->SetObjectField(TEXT("event_graph"), EventGraphObj);
    }

    // Include components if requested
    if (bIncludeComponents)
    {
        TArray<TSharedPtr<FJsonValue>> ComponentArray;
        if (Blueprint->SimpleConstructionScript)
        {
            for (USCS_Node* Node : Blueprint->SimpleConstructionScript->GetAllNodes())
            {
                if (Node && Node->ComponentTemplate)
                {
                    TSharedPtr<FJsonObject> CompObj = MakeShared<FJsonObject>();
                    CompObj->SetStringField(TEXT("name"), Node->GetVariableName().ToString());
                    CompObj->SetStringField(TEXT("class"), Node->ComponentTemplate->GetClass()->GetName());
                    CompObj->SetBoolField(TEXT("is_root"), Node == Blueprint->SimpleConstructionScript->GetDefaultSceneRootNode());
                    ComponentArray.Add(MakeShared<FJsonValueObject>(CompObj));
                }
            }
        }
        ResultObj->SetArrayField(TEXT("components"), ComponentArray);
    }

    // Include interfaces if requested
    if (bIncludeInterfaces)
    {
        TArray<TSharedPtr<FJsonValue>> InterfaceArray;
        for (const FBPInterfaceDescription& Interface : Blueprint->ImplementedInterfaces)
        {
            TSharedPtr<FJsonObject> InterfaceObj = MakeShared<FJsonObject>();
            InterfaceObj->SetStringField(TEXT("name"), Interface.Interface ? Interface.Interface->GetName() : TEXT("Unknown"));
            InterfaceArray.Add(MakeShared<FJsonValueObject>(InterfaceObj));
        }
        ResultObj->SetArrayField(TEXT("interfaces"), InterfaceArray);
    }

    ResultObj->SetBoolField(TEXT("success"), true);
    return ResultObj;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPBlueprintCommands::HandleAnalyzeBlueprintGraph(const TSharedPtr<FJsonObject>& Params)
{
    // Get required parameters
    FString BlueprintPath;
    if (!Params->TryGetStringField(TEXT("blueprint_path"), BlueprintPath))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_path' parameter"));
    }

    FString GraphName = TEXT("EventGraph");
    Params->TryGetStringField(TEXT("graph_name"), GraphName);

    // Get optional parameters
    bool bIncludeNodeDetails = true;
    bool bIncludePinConnections = true;
    bool bTraceExecutionFlow = true;

    Params->TryGetBoolField(TEXT("include_node_details"), bIncludeNodeDetails);
    Params->TryGetBoolField(TEXT("include_pin_connections"), bIncludePinConnections);
    Params->TryGetBoolField(TEXT("trace_execution_flow"), bTraceExecutionFlow);

    // Load the blueprint
    UBlueprint* Blueprint = Cast<UBlueprint>(UEditorAssetLibrary::LoadAsset(BlueprintPath));
    if (!Blueprint)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Failed to load blueprint: %s"), *BlueprintPath));
    }

    // Find the specified graph
    UEdGraph* TargetGraph = nullptr;
    
    // Check event graphs first
    for (UEdGraph* Graph : Blueprint->UbergraphPages)
    {
        if (Graph && Graph->GetName() == GraphName)
        {
            TargetGraph = Graph;
            break;
        }
    }
    
    // Check function graphs if not found
    if (!TargetGraph)
    {
        for (UEdGraph* Graph : Blueprint->FunctionGraphs)
        {
            if (Graph && Graph->GetName() == GraphName)
            {
                TargetGraph = Graph;
                break;
            }
        }
    }

    if (!TargetGraph)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Graph not found: %s"), *GraphName));
    }

    TSharedPtr<FJsonObject> GraphData = MakeShared<FJsonObject>();
    GraphData->SetStringField(TEXT("graph_name"), TargetGraph->GetName());
    GraphData->SetStringField(TEXT("graph_type"), TargetGraph->GetClass()->GetName());

    // Analyze nodes
    TArray<TSharedPtr<FJsonValue>> NodeArray;
    TArray<TSharedPtr<FJsonValue>> ConnectionArray;

    for (UEdGraphNode* Node : TargetGraph->Nodes)
    {
        if (Node)
        {
            TSharedPtr<FJsonObject> NodeObj = MakeShared<FJsonObject>();
            NodeObj->SetStringField(TEXT("name"), Node->GetName());
            NodeObj->SetStringField(TEXT("class"), Node->GetClass()->GetName());
            NodeObj->SetStringField(TEXT("title"), Node->GetNodeTitle(ENodeTitleType::FullTitle).ToString());

            if (bIncludeNodeDetails)
            {
                NodeObj->SetNumberField(TEXT("pos_x"), Node->NodePosX);
                NodeObj->SetNumberField(TEXT("pos_y"), Node->NodePosY);
                NodeObj->SetBoolField(TEXT("can_rename"), Node->bCanRenameNode);
            }

            // Include pin information if requested
            if (bIncludePinConnections)
            {
                TArray<TSharedPtr<FJsonValue>> PinArray;
                for (UEdGraphPin* Pin : Node->Pins)
                {
                    if (Pin)
                    {
                        TSharedPtr<FJsonObject> PinObj = MakeShared<FJsonObject>();
                        PinObj->SetStringField(TEXT("name"), Pin->PinName.ToString());
                        PinObj->SetStringField(TEXT("type"), Pin->PinType.PinCategory.ToString());
                        PinObj->SetStringField(TEXT("direction"), Pin->Direction == EGPD_Input ? TEXT("Input") : TEXT("Output"));
                        PinObj->SetNumberField(TEXT("connections"), Pin->LinkedTo.Num());
                        
                        // Record connections for this pin
                        for (UEdGraphPin* LinkedPin : Pin->LinkedTo)
                        {
                            if (LinkedPin && LinkedPin->GetOwningNode())
                            {
                                TSharedPtr<FJsonObject> ConnObj = MakeShared<FJsonObject>();
                                ConnObj->SetStringField(TEXT("from_node"), Pin->GetOwningNode()->GetName());
                                ConnObj->SetStringField(TEXT("from_pin"), Pin->PinName.ToString());
                                ConnObj->SetStringField(TEXT("to_node"), LinkedPin->GetOwningNode()->GetName());
                                ConnObj->SetStringField(TEXT("to_pin"), LinkedPin->PinName.ToString());
                                ConnectionArray.Add(MakeShared<FJsonValueObject>(ConnObj));
                            }
                        }
                        
                        PinArray.Add(MakeShared<FJsonValueObject>(PinObj));
                    }
                }
                NodeObj->SetArrayField(TEXT("pins"), PinArray);
            }

            NodeArray.Add(MakeShared<FJsonValueObject>(NodeObj));
        }
    }

    GraphData->SetArrayField(TEXT("nodes"), NodeArray);
    GraphData->SetArrayField(TEXT("connections"), ConnectionArray);

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetStringField(TEXT("blueprint_path"), BlueprintPath);
    ResultObj->SetObjectField(TEXT("graph_data"), GraphData);
    ResultObj->SetBoolField(TEXT("success"), true);

    return ResultObj;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPBlueprintCommands::HandleGetBlueprintVariableDetails(const TSharedPtr<FJsonObject>& Params)
{
    // Get required parameters
    FString BlueprintPath;
    if (!Params->TryGetStringField(TEXT("blueprint_path"), BlueprintPath))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_path' parameter"));
    }

    FString VariableName;
    bool bSpecificVariable = Params->TryGetStringField(TEXT("variable_name"), VariableName);

    // Load the blueprint
    UBlueprint* Blueprint = Cast<UBlueprint>(UEditorAssetLibrary::LoadAsset(BlueprintPath));
    if (!Blueprint)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Failed to load blueprint: %s"), *BlueprintPath));
    }

    TArray<TSharedPtr<FJsonValue>> VariableArray;

    for (const FBPVariableDescription& Variable : Blueprint->NewVariables)
    {
        // If looking for specific variable, skip others
        if (bSpecificVariable && Variable.VarName.ToString() != VariableName)
        {
            continue;
        }

        TSharedPtr<FJsonObject> VarObj = MakeShared<FJsonObject>();
        VarObj->SetStringField(TEXT("name"), Variable.VarName.ToString());
        VarObj->SetStringField(TEXT("type"), Variable.VarType.PinCategory.ToString());
        VarObj->SetStringField(TEXT("sub_category"), Variable.VarType.PinSubCategory.ToString());
        VarObj->SetStringField(TEXT("default_value"), Variable.DefaultValue);
        VarObj->SetStringField(TEXT("friendly_name"), Variable.FriendlyName.IsEmpty() ? Variable.VarName.ToString() : Variable.FriendlyName);
        
        // Get tooltip from metadata (VarTooltip doesn't exist in UE 5.5)
        FString TooltipValue;
        if (Variable.HasMetaData(FBlueprintMetadata::MD_Tooltip))
        {
            TooltipValue = Variable.GetMetaData(FBlueprintMetadata::MD_Tooltip);
        }
        VarObj->SetStringField(TEXT("tooltip"), TooltipValue);
        
        VarObj->SetStringField(TEXT("category"), Variable.Category.ToString());

        // Property flags
        VarObj->SetBoolField(TEXT("is_editable"), (Variable.PropertyFlags & CPF_Edit) != 0);
        VarObj->SetBoolField(TEXT("is_blueprint_visible"), (Variable.PropertyFlags & CPF_BlueprintVisible) != 0);
        VarObj->SetBoolField(TEXT("is_editable_in_instance"), (Variable.PropertyFlags & CPF_DisableEditOnInstance) == 0);
        VarObj->SetBoolField(TEXT("is_config"), (Variable.PropertyFlags & CPF_Config) != 0);

        // Replication
        VarObj->SetNumberField(TEXT("replication"), (int32)Variable.ReplicationCondition);

        VariableArray.Add(MakeShared<FJsonValueObject>(VarObj));
    }

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetStringField(TEXT("blueprint_path"), BlueprintPath);
    
    if (bSpecificVariable)
    {
        ResultObj->SetStringField(TEXT("variable_name"), VariableName);
        if (VariableArray.Num() > 0)
        {
            ResultObj->SetObjectField(TEXT("variable"), VariableArray[0]->AsObject());
        }
        else
        {
            return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Variable not found: %s"), *VariableName));
        }
    }
    else
    {
        ResultObj->SetArrayField(TEXT("variables"), VariableArray);
        ResultObj->SetNumberField(TEXT("variable_count"), VariableArray.Num());
    }

    ResultObj->SetBoolField(TEXT("success"), true);
    return ResultObj;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPBlueprintCommands::HandleGetBlueprintFunctionDetails(const TSharedPtr<FJsonObject>& Params)
{
    // Get required parameters
    FString BlueprintPath;
    if (!Params->TryGetStringField(TEXT("blueprint_path"), BlueprintPath))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_path' parameter"));
    }

    FString FunctionName;
    bool bSpecificFunction = Params->TryGetStringField(TEXT("function_name"), FunctionName);

    bool bIncludeGraph = true;
    Params->TryGetBoolField(TEXT("include_graph"), bIncludeGraph);

    // Load the blueprint
    UBlueprint* Blueprint = Cast<UBlueprint>(UEditorAssetLibrary::LoadAsset(BlueprintPath));
    if (!Blueprint)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Failed to load blueprint: %s"), *BlueprintPath));
    }

    TArray<TSharedPtr<FJsonValue>> FunctionArray;

    for (UEdGraph* Graph : Blueprint->FunctionGraphs)
    {
        if (!Graph) continue;

        // If looking for specific function, skip others
        if (bSpecificFunction && Graph->GetName() != FunctionName)
        {
            continue;
        }

        TSharedPtr<FJsonObject> FuncObj = MakeShared<FJsonObject>();
        FuncObj->SetStringField(TEXT("name"), Graph->GetName());
        FuncObj->SetStringField(TEXT("graph_type"), TEXT("Function"));

        // Get function signature from graph
        TArray<TSharedPtr<FJsonValue>> InputPins;
        TArray<TSharedPtr<FJsonValue>> OutputPins;

        // Find function entry and result nodes
        for (UEdGraphNode* Node : Graph->Nodes)
        {
            if (Node)
            {
                if (Node->GetClass()->GetName().Contains(TEXT("FunctionEntry")))
                {
                    // Process input parameters
                    for (UEdGraphPin* Pin : Node->Pins)
                    {
                        if (Pin && Pin->Direction == EGPD_Output && Pin->PinName != TEXT("then"))
                        {
                            TSharedPtr<FJsonObject> PinObj = MakeShared<FJsonObject>();
                            PinObj->SetStringField(TEXT("name"), Pin->PinName.ToString());
                            PinObj->SetStringField(TEXT("type"), Pin->PinType.PinCategory.ToString());
                            InputPins.Add(MakeShared<FJsonValueObject>(PinObj));
                        }
                    }
                }
                else if (Node->GetClass()->GetName().Contains(TEXT("FunctionResult")))
                {
                    // Process output parameters
                    for (UEdGraphPin* Pin : Node->Pins)
                    {
                        if (Pin && Pin->Direction == EGPD_Input && Pin->PinName != TEXT("exec"))
                        {
                            TSharedPtr<FJsonObject> PinObj = MakeShared<FJsonObject>();
                            PinObj->SetStringField(TEXT("name"), Pin->PinName.ToString());
                            PinObj->SetStringField(TEXT("type"), Pin->PinType.PinCategory.ToString());
                            OutputPins.Add(MakeShared<FJsonValueObject>(PinObj));
                        }
                    }
                }
            }
        }

        FuncObj->SetArrayField(TEXT("input_parameters"), InputPins);
        FuncObj->SetArrayField(TEXT("output_parameters"), OutputPins);
        FuncObj->SetNumberField(TEXT("node_count"), Graph->Nodes.Num());

        // Include graph details if requested
        if (bIncludeGraph)
        {
            TArray<TSharedPtr<FJsonValue>> NodeArray;
            for (UEdGraphNode* Node : Graph->Nodes)
            {
                if (Node)
                {
                    TSharedPtr<FJsonObject> NodeObj = MakeShared<FJsonObject>();
                    NodeObj->SetStringField(TEXT("name"), Node->GetName());
                    NodeObj->SetStringField(TEXT("class"), Node->GetClass()->GetName());
                    NodeObj->SetStringField(TEXT("title"), Node->GetNodeTitle(ENodeTitleType::FullTitle).ToString());
                    NodeArray.Add(MakeShared<FJsonValueObject>(NodeObj));
                }
            }
            FuncObj->SetArrayField(TEXT("graph_nodes"), NodeArray);
        }

        FunctionArray.Add(MakeShared<FJsonValueObject>(FuncObj));
    }

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetStringField(TEXT("blueprint_path"), BlueprintPath);
    
    if (bSpecificFunction)
    {
        ResultObj->SetStringField(TEXT("function_name"), FunctionName);
        if (FunctionArray.Num() > 0)
        {
            ResultObj->SetObjectField(TEXT("function"), FunctionArray[0]->AsObject());
        }
        else
        {
            return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Function not found: %s"), *FunctionName));
        }
    }
    else
    {
        ResultObj->SetArrayField(TEXT("functions"), FunctionArray);
        ResultObj->SetNumberField(TEXT("function_count"), FunctionArray.Num());
    }

    ResultObj->SetBoolField(TEXT("success"), true);
    return ResultObj;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPBlueprintCommands::HandleCreateMaterial(const TSharedPtr<FJsonObject>& Params)
{
    // Get required parameters
    FString MaterialName;
    if (!Params->TryGetStringField(TEXT("name"), MaterialName))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'name' parameter"));
    }

    // Check if material already exists
    FString PackagePath = TEXT("/Game/Materials/");
    FString AssetName = MaterialName;
    if (UEditorAssetLibrary::DoesAssetExist(PackagePath + AssetName))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Material already exists: %s"), *MaterialName));
    }

    // Create the package
    UPackage* Package = CreatePackage(*(PackagePath + AssetName));
    if (!Package)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create package for material"));
    }

    // Create the material using factory
    UMaterialFactoryNew* MaterialFactory = NewObject<UMaterialFactoryNew>();
    UMaterial* NewMaterial = Cast<UMaterial>(MaterialFactory->FactoryCreateNew(
        UMaterial::StaticClass(), Package, *AssetName, RF_Public | RF_Standalone, nullptr, GWarn));
    
    if (!NewMaterial)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create material object"));
    }

    // Add to asset registry
    FAssetRegistryModule::AssetCreated(NewMaterial);
    Package->MarkPackageDirty();

    UE_LOG(LogTemp, Log, TEXT("Successfully created empty material: %s at %s"), *MaterialName, *(PackagePath + AssetName));

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetStringField(TEXT("name"), AssetName);
    ResultObj->SetStringField(TEXT("path"), PackagePath + AssetName);
    ResultObj->SetBoolField(TEXT("success"), true);
    
    return ResultObj;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPBlueprintCommands::HandleCreateMaterialFunction(const TSharedPtr<FJsonObject>& Params)
{
    // Get required parameters
    FString FunctionName;
    if (!Params->TryGetStringField(TEXT("name"), FunctionName))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'name' parameter"));
    }

    // Get optional path parameter
    FString PackagePath = TEXT("/Game/MaterialFunctions/");
    FString CustomPath;
    if (Params->TryGetStringField(TEXT("path"), CustomPath))
    {
        PackagePath = CustomPath;
        if (!PackagePath.EndsWith(TEXT("/")))
        {
            PackagePath += TEXT("/");
        }
    }

    FString AssetName = FunctionName;
    if (UEditorAssetLibrary::DoesAssetExist(PackagePath + AssetName))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Material Function already exists: %s"), *FunctionName));
    }

    // Create the package
    UPackage* Package = CreatePackage(*(PackagePath + AssetName));
    if (!Package)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create package for material function"));
    }

    // Create the material function
    UMaterialFunction* NewFunction = NewObject<UMaterialFunction>(Package, *AssetName, RF_Public | RF_Standalone);
    
    if (!NewFunction)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create material function object"));
    }

    // Set description if provided
    FString Description;
    if (Params->TryGetStringField(TEXT("description"), Description))
    {
        NewFunction->Description = Description;
    }

    // Add to asset registry
    FAssetRegistryModule::AssetCreated(NewFunction);
    Package->MarkPackageDirty();

    UE_LOG(LogTemp, Log, TEXT("Successfully created material function: %s at %s"), *FunctionName, *(PackagePath + AssetName));

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetStringField(TEXT("name"), AssetName);
    ResultObj->SetStringField(TEXT("path"), PackagePath + AssetName);
    ResultObj->SetBoolField(TEXT("success"), true);
    
    return ResultObj;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPBlueprintCommands::HandleAddMaterialExpression(const TSharedPtr<FJsonObject>& Params)
{
    // Get material name
    FString MaterialName;
    if (!Params->TryGetStringField(TEXT("material_name"), MaterialName))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'material_name' parameter"));
    }

    // Get expression type
    FString ExpressionType;
    if (!Params->TryGetStringField(TEXT("expression_type"), ExpressionType))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'expression_type' parameter"));
    }

    // Find or load the material
    FString MaterialPath = MaterialName.StartsWith(TEXT("/")) ? MaterialName : FString::Printf(TEXT("/Game/Materials/%s"), *MaterialName);
    UMaterial* Material = Cast<UMaterial>(UEditorAssetLibrary::LoadAsset(MaterialPath));
    if (!Material)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Material not found: %s"), *MaterialPath));
    }

    // Get position
    int32 PosX = 0, PosY = 0;
    Params->TryGetNumberField(TEXT("pos_x"), PosX);
    Params->TryGetNumberField(TEXT("pos_y"), PosY);

    // Check if we're updating an existing node
    FString ExistingNodeId;
    bool bIsUpdate = Params->TryGetStringField(TEXT("node_id"), ExistingNodeId);
    
    UMaterialExpression* ExistingExpression = nullptr;
    if (bIsUpdate)
    {
        ExistingExpression = FindMaterialExpressionById(Material, ExistingNodeId);
        if (!ExistingExpression)
        {
            return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Node not found: %s"), *ExistingNodeId));
        }
    }

    // Create expression based on type (or update existing)
    UMaterialExpression* NewExpression = nullptr;
    FString NodeId;

    // Handle update mode - modify existing expression
    if (bIsUpdate && ExistingExpression)
    {
        // Update existing expression values based on type
        if (UMaterialExpressionConstant* ConstScalar = Cast<UMaterialExpressionConstant>(ExistingExpression))
        {
            float Value = 0.0f;
            Params->TryGetNumberField(TEXT("value"), Value);
            ConstScalar->R = Value;
        }
        else if (UMaterialExpressionConstant2Vector* Const2Vec = Cast<UMaterialExpressionConstant2Vector>(ExistingExpression))
        {
            const TArray<TSharedPtr<FJsonValue>>* ValueArray = nullptr;
            if (Params->TryGetArrayField(TEXT("value"), ValueArray) && ValueArray && ValueArray->Num() >= 2)
            {
                Const2Vec->R = (*ValueArray)[0]->AsNumber();
                Const2Vec->G = (*ValueArray)[1]->AsNumber();
            }
        }
        else if (UMaterialExpressionConstant3Vector* Const3Vec = Cast<UMaterialExpressionConstant3Vector>(ExistingExpression))
        {
            const TArray<TSharedPtr<FJsonValue>>* ValueArray = nullptr;
            if (Params->TryGetArrayField(TEXT("value"), ValueArray) && ValueArray && ValueArray->Num() >= 3)
            {
                Const3Vec->Constant.R = (*ValueArray)[0]->AsNumber();
                Const3Vec->Constant.G = (*ValueArray)[1]->AsNumber();
                Const3Vec->Constant.B = (*ValueArray)[2]->AsNumber();
            }
        }
        else if (UMaterialExpressionConstant4Vector* Const4Vec = Cast<UMaterialExpressionConstant4Vector>(ExistingExpression))
        {
            const TArray<TSharedPtr<FJsonValue>>* ValueArray = nullptr;
            if (Params->TryGetArrayField(TEXT("value"), ValueArray) && ValueArray && ValueArray->Num() >= 4)
            {
                Const4Vec->Constant.R = (*ValueArray)[0]->AsNumber();
                Const4Vec->Constant.G = (*ValueArray)[1]->AsNumber();
                Const4Vec->Constant.B = (*ValueArray)[2]->AsNumber();
                Const4Vec->Constant.A = (*ValueArray)[3]->AsNumber();
            }
        }
        else if (UMaterialExpressionScalarParameter* ScalarParam = Cast<UMaterialExpressionScalarParameter>(ExistingExpression))
        {
            float Value = 0.0f;
            Params->TryGetNumberField(TEXT("value"), Value);
            ScalarParam->DefaultValue = Value;
        }
        else if (UMaterialExpressionVectorParameter* VectorParam = Cast<UMaterialExpressionVectorParameter>(ExistingExpression))
        {
            const TArray<TSharedPtr<FJsonValue>>* ValueArray = nullptr;
            if (Params->TryGetArrayField(TEXT("value"), ValueArray) && ValueArray && ValueArray->Num() >= 4)
            {
                VectorParam->DefaultValue.R = (*ValueArray)[0]->AsNumber();
                VectorParam->DefaultValue.G = (*ValueArray)[1]->AsNumber();
                VectorParam->DefaultValue.B = (*ValueArray)[2]->AsNumber();
                VectorParam->DefaultValue.A = (*ValueArray)[3]->AsNumber();
            }
        }
        // Update position if provided
        if (Params->HasField(TEXT("pos_x")))
        {
            ExistingExpression->MaterialExpressionEditorX = PosX;
        }
        if (Params->HasField(TEXT("pos_y")))
        {
            ExistingExpression->MaterialExpressionEditorY = PosY;
        }
        
        NewExpression = ExistingExpression;
    }
    else
    {
        // Create new expression
        if (ExpressionType == TEXT("Constant"))
        {
            UMaterialExpressionConstant* NewConst = NewObject<UMaterialExpressionConstant>(Material);
            float Value = 0.0f;
            Params->TryGetNumberField(TEXT("value"), Value);
            NewConst->R = Value;
            NewExpression = NewConst;
        }
        else if (ExpressionType == TEXT("Constant2Vector"))
        {
            UMaterialExpressionConstant2Vector* NewConst2 = NewObject<UMaterialExpressionConstant2Vector>(Material);
            const TArray<TSharedPtr<FJsonValue>>* ValueArray = nullptr;
            if (Params->TryGetArrayField(TEXT("value"), ValueArray) && ValueArray && ValueArray->Num() >= 2)
            {
                NewConst2->R = (*ValueArray)[0]->AsNumber();
                NewConst2->G = (*ValueArray)[1]->AsNumber();
            }
            NewExpression = NewConst2;
        }
        else if (ExpressionType == TEXT("Constant3Vector"))
        {
            UMaterialExpressionConstant3Vector* NewConst3 = NewObject<UMaterialExpressionConstant3Vector>(Material);
            const TArray<TSharedPtr<FJsonValue>>* ValueArray = nullptr;
            if (Params->TryGetArrayField(TEXT("value"), ValueArray) && ValueArray && ValueArray->Num() >= 3)
            {
                NewConst3->Constant.R = (*ValueArray)[0]->AsNumber();
                NewConst3->Constant.G = (*ValueArray)[1]->AsNumber();
                NewConst3->Constant.B = (*ValueArray)[2]->AsNumber();
            }
            NewExpression = NewConst3;
        }
        else if (ExpressionType == TEXT("Constant4Vector"))
        {
            UMaterialExpressionConstant4Vector* NewConst4 = NewObject<UMaterialExpressionConstant4Vector>(Material);
            const TArray<TSharedPtr<FJsonValue>>* ValueArray = nullptr;
            if (Params->TryGetArrayField(TEXT("value"), ValueArray) && ValueArray && ValueArray->Num() >= 4)
            {
                NewConst4->Constant.R = (*ValueArray)[0]->AsNumber();
                NewConst4->Constant.G = (*ValueArray)[1]->AsNumber();
                NewConst4->Constant.B = (*ValueArray)[2]->AsNumber();
                NewConst4->Constant.A = (*ValueArray)[3]->AsNumber();
            }
            NewExpression = NewConst4;
        }
        else if (ExpressionType == TEXT("Multiply"))
        {
            NewExpression = NewObject<UMaterialExpressionMultiply>(Material);
        }
        else if (ExpressionType == TEXT("Add"))
        {
            NewExpression = NewObject<UMaterialExpressionAdd>(Material);
        }
        else if (ExpressionType == TEXT("Subtract"))
        {
            NewExpression = NewObject<UMaterialExpressionSubtract>(Material);
        }
        else if (ExpressionType == TEXT("Divide"))
        {
            NewExpression = NewObject<UMaterialExpressionDivide>(Material);
        }
        else if (ExpressionType == TEXT("Lerp"))
        {
            NewExpression = NewObject<UMaterialExpressionLinearInterpolate>(Material);
        }
        else if (ExpressionType == TEXT("Clamp"))
        {
            NewExpression = NewObject<UMaterialExpressionClamp>(Material);
        }
        else if (ExpressionType == TEXT("TextureSample"))
        {
            UMaterialExpressionTextureSample* TexExpr = NewObject<UMaterialExpressionTextureSample>(Material);
            FString TexturePath;
            if (Params->TryGetStringField(TEXT("texture"), TexturePath))
            {
                UTexture* Texture = Cast<UTexture>(UEditorAssetLibrary::LoadAsset(TexturePath));
                if (Texture)
                {
                    TexExpr->Texture = Texture;
                }
            }
            NewExpression = TexExpr;
        }
        else if (ExpressionType == TEXT("TextureCoordinate"))
        {
            NewExpression = NewObject<UMaterialExpressionTextureCoordinate>(Material);
        }
        else if (ExpressionType == TEXT("Time"))
        {
            NewExpression = NewObject<UMaterialExpressionTime>(Material);
        }
        else if (ExpressionType == TEXT("Sine"))
        {
            NewExpression = NewObject<UMaterialExpressionSine>(Material);
        }
        else if (ExpressionType == TEXT("Cosine"))
        {
            NewExpression = NewObject<UMaterialExpressionCosine>(Material);
        }
        else if (ExpressionType == TEXT("Panner"))
        {
            NewExpression = NewObject<UMaterialExpressionPanner>(Material);
        }
        else if (ExpressionType == TEXT("Rotator"))
        {
            NewExpression = NewObject<UMaterialExpressionRotator>(Material);
        }
        else if (ExpressionType == TEXT("Power"))
        {
            NewExpression = NewObject<UMaterialExpressionPower>(Material);
        }
        else if (ExpressionType == TEXT("Abs"))
        {
            NewExpression = NewObject<UMaterialExpressionAbs>(Material);
        }
        else if (ExpressionType == TEXT("Floor"))
        {
            NewExpression = NewObject<UMaterialExpressionFloor>(Material);
        }
        else if (ExpressionType == TEXT("Ceil"))
        {
            NewExpression = NewObject<UMaterialExpressionCeil>(Material);
        }
        else if (ExpressionType == TEXT("Frac"))
    {
        NewExpression = NewObject<UMaterialExpressionFrac>(Material);
    }
    else if (ExpressionType == TEXT("ComponentMask"))
    {
        UMaterialExpressionComponentMask* MaskExpr = NewObject<UMaterialExpressionComponentMask>(Material);
        // Default to R channel only for dissolve effect
        MaskExpr->R = true;
        MaskExpr->G = false;
        MaskExpr->B = false;
        MaskExpr->A = false;
        // Override with provided values
        const TArray<TSharedPtr<FJsonValue>>* MaskArray = nullptr;
        if (Params->TryGetArrayField(TEXT("mask"), MaskArray) && MaskArray && MaskArray->Num() >= 4)
        {
            MaskExpr->R = (*MaskArray)[0]->AsBool();
            MaskExpr->G = (*MaskArray)[1]->AsBool();
            MaskExpr->B = (*MaskArray)[2]->AsBool();
            MaskExpr->A = (*MaskArray)[3]->AsBool();
        }
        NewExpression = MaskExpr;
    }
    else if (ExpressionType == TEXT("AppendVector"))
    {
        NewExpression = NewObject<UMaterialExpressionAppendVector>(Material);
    }
    else if (ExpressionType == TEXT("VertexColor"))
    {
        NewExpression = NewObject<UMaterialExpressionVertexColor>(Material);
    }
    else if (ExpressionType == TEXT("WorldPosition"))
    {
        NewExpression = NewObject<UMaterialExpressionWorldPosition>(Material);
    }
    else if (ExpressionType == TEXT("ObjectPosition"))
    {
        NewExpression = NewObject<UMaterialExpressionObjectPositionWS>(Material);
    }
    else if (ExpressionType == TEXT("VertexNormal"))
    {
        NewExpression = NewObject<UMaterialExpressionVertexNormalWS>(Material);
    }
    else if (ExpressionType == TEXT("Fresnel"))
    {
        NewExpression = NewObject<UMaterialExpressionFresnel>(Material);
    }
    else if (ExpressionType == TEXT("DepthFade"))
    {
        NewExpression = NewObject<UMaterialExpressionDepthFade>(Material);
    }
    else if (ExpressionType == TEXT("CameraPosition"))
    {
        NewExpression = NewObject<UMaterialExpressionCameraPositionWS>(Material);
    }
    else if (ExpressionType == TEXT("CameraVector"))
    {
        NewExpression = NewObject<UMaterialExpressionCameraVectorWS>(Material);
    }
    else if (ExpressionType == TEXT("Distance"))
    {
        NewExpression = NewObject<UMaterialExpressionDistance>(Material);
    }
    else if (ExpressionType == TEXT("PixelDepth"))
    {
        NewExpression = NewObject<UMaterialExpressionPixelDepth>(Material);
    }
    else if (ExpressionType == TEXT("SceneDepth"))
    {
        NewExpression = NewObject<UMaterialExpressionSceneDepth>(Material);
    }
    else if (ExpressionType == TEXT("Custom") || ExpressionType == TEXT("CustomHLSL"))
    {
        UMaterialExpressionCustom* CustomExpr = nullptr;
        
        // Check if updating existing node
        if (bIsUpdate && ExistingExpression)
        {
            CustomExpr = Cast<UMaterialExpressionCustom>(ExistingExpression);
            if (!CustomExpr)
            {
                return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Existing node is not a Custom expression"));
            }
        }
        else
        {
            CustomExpr = NewObject<UMaterialExpressionCustom>(Material);
        }

        // Set HLSL code
        FString Code;
        if (Params->TryGetStringField(TEXT("code"), Code))
        {
            CustomExpr->Code = Code;
        }

        // Set description
        FString Description;
        if (Params->TryGetStringField(TEXT("description"), Description))
        {
            CustomExpr->Description = Description;
        }
        else if (!bIsUpdate)
        {
            CustomExpr->Description = TEXT("Custom");
        }

        // Set output type
        FString OutputType;
        if (Params->TryGetStringField(TEXT("output_type"), OutputType))
        {
            if (OutputType == TEXT("Float1"))
                CustomExpr->OutputType = ECustomMaterialOutputType::CMOT_Float1;
            else if (OutputType == TEXT("Float2"))
                CustomExpr->OutputType = ECustomMaterialOutputType::CMOT_Float2;
            else if (OutputType == TEXT("Float3"))
                CustomExpr->OutputType = ECustomMaterialOutputType::CMOT_Float3;
            else if (OutputType == TEXT("Float4"))
                CustomExpr->OutputType = ECustomMaterialOutputType::CMOT_Float4;
        }

        // Set inputs (input pin names)
        const TArray<TSharedPtr<FJsonValue>>* InputsArray = nullptr;
        if (Params->TryGetArrayField(TEXT("inputs"), InputsArray) && InputsArray)
        {
            // When updating, preserve existing input connections
            TMap<FName, FExpressionInput*> ExistingConnections;
            if (bIsUpdate)
            {
                for (int32 i = 0; i < CustomExpr->Inputs.Num(); i++)
                {
                    FName InputName = CustomExpr->Inputs[i].InputName;
                    FExpressionInput* InputPtr = CustomExpr->GetInput(i);
                    if (InputPtr && InputPtr->Expression)
                    {
                        ExistingConnections.Add(InputName, new FExpressionInput(*InputPtr));
                    }
                }
            }

            CustomExpr->Inputs.Empty();
            for (const TSharedPtr<FJsonValue>& InputValue : *InputsArray)
            {
                FCustomInput NewInput;
                if (InputValue->Type == EJson::String)
                {
                    NewInput.InputName = FName(*InputValue->AsString());
                }
                else if (InputValue->Type == EJson::Object)
                {
                    TSharedPtr<FJsonObject> InputObj = InputValue->AsObject();
                    FString InputName;
                    if (InputObj->TryGetStringField(TEXT("name"), InputName))
                    {
                        NewInput.InputName = FName(*InputName);
                    }
                }
                CustomExpr->Inputs.Add(NewInput);

                // Restore connection if this input existed before
                if (bIsUpdate)
                {
                    FExpressionInput** SavedConnection = ExistingConnections.Find(NewInput.InputName);
                    if (SavedConnection && *SavedConnection)
                    {
                        int32 InputIndex = CustomExpr->Inputs.Num() - 1;
                        FExpressionInput* CurrentInput = CustomExpr->GetInput(InputIndex);
                        if (CurrentInput)
                        {
                            CurrentInput->Expression = (*SavedConnection)->Expression;
                            CurrentInput->OutputIndex = (*SavedConnection)->OutputIndex;
                            CurrentInput->Mask = (*SavedConnection)->Mask;
                            CurrentInput->MaskR = (*SavedConnection)->MaskR;
                            CurrentInput->MaskG = (*SavedConnection)->MaskG;
                            CurrentInput->MaskB = (*SavedConnection)->MaskB;
                            CurrentInput->MaskA = (*SavedConnection)->MaskA;
                        }
                    }
                }
            }

            // Clean up saved connections
            for (auto& Pair : ExistingConnections)
            {
                delete Pair.Value;
            }
        }

        NewExpression = CustomExpr;
    }
    else if (ExpressionType == TEXT("ScalarParameter"))
    {
        UMaterialExpressionScalarParameter* ScalarParam = nullptr;
        
        // Check if updating existing node
        if (bIsUpdate && ExistingExpression)
        {
            ScalarParam = Cast<UMaterialExpressionScalarParameter>(ExistingExpression);
            if (!ScalarParam)
            {
                return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Existing node is not a ScalarParameter"));
            }
        }
        else
        {
            ScalarParam = NewObject<UMaterialExpressionScalarParameter>(Material);
        }

        // Set parameter name
        FString ParamName;
        if (Params->TryGetStringField(TEXT("parameter_name"), ParamName))
        {
            ScalarParam->ParameterName = FName(*ParamName);
        }
        else if (!bIsUpdate)
        {
            ScalarParam->ParameterName = FName(TEXT("Parameter"));
        }

        // Set default value - support both single number and array format
        float DefaultValue = 0.0f;
        const TArray<TSharedPtr<FJsonValue>>* ValueArray = nullptr;
        if (Params->TryGetArrayField(TEXT("value"), ValueArray) && ValueArray && ValueArray->Num() > 0)
        {
            // Array format: [value]
            DefaultValue = (*ValueArray)[0]->AsNumber();
            ScalarParam->DefaultValue = DefaultValue;
        }
        else if (Params->TryGetNumberField(TEXT("value"), DefaultValue))
        {
            // Single number format
            ScalarParam->DefaultValue = DefaultValue;
        }

        // Set group
        FString Group;
        if (Params->TryGetStringField(TEXT("group"), Group))
        {
            ScalarParam->Group = FName(*Group);
        }

        NewExpression = ScalarParam;
    }
    else if (ExpressionType == TEXT("VectorParameter"))
    {
        UMaterialExpressionVectorParameter* VectorParam = nullptr;
        
        // Check if updating existing node
        if (bIsUpdate && ExistingExpression)
        {
            VectorParam = Cast<UMaterialExpressionVectorParameter>(ExistingExpression);
            if (!VectorParam)
            {
                return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Existing node is not a VectorParameter"));
            }
        }
        else
        {
            VectorParam = NewObject<UMaterialExpressionVectorParameter>(Material);
        }

        // Set parameter name
        FString ParamName;
        if (Params->TryGetStringField(TEXT("parameter_name"), ParamName))
        {
            VectorParam->ParameterName = FName(*ParamName);
        }
        else if (!bIsUpdate)
        {
            VectorParam->ParameterName = FName(TEXT("Parameter"));
        }

        // Set default value (RGBA)
        const TArray<TSharedPtr<FJsonValue>>* ValueArray = nullptr;
        if (Params->TryGetArrayField(TEXT("value"), ValueArray) && ValueArray && ValueArray->Num() >= 4)
        {
            VectorParam->DefaultValue.R = (*ValueArray)[0]->AsNumber();
            VectorParam->DefaultValue.G = (*ValueArray)[1]->AsNumber();
            VectorParam->DefaultValue.B = (*ValueArray)[2]->AsNumber();
            VectorParam->DefaultValue.A = (*ValueArray)[3]->AsNumber();
        }
        else if (Params->TryGetArrayField(TEXT("value"), ValueArray) && ValueArray && ValueArray->Num() >= 3)
        {
            VectorParam->DefaultValue.R = (*ValueArray)[0]->AsNumber();
            VectorParam->DefaultValue.G = (*ValueArray)[1]->AsNumber();
            VectorParam->DefaultValue.B = (*ValueArray)[2]->AsNumber();
            VectorParam->DefaultValue.A = 1.0f;
        }

        // Set group
        FString Group;
        if (Params->TryGetStringField(TEXT("group"), Group))
        {
            VectorParam->Group = FName(*Group);
        }

        NewExpression = VectorParam;
    }
    else if (ExpressionType == TEXT("TextureObjectParameter"))
    {
        UMaterialExpressionTextureObjectParameter* TexParam = NewObject<UMaterialExpressionTextureObjectParameter>(Material);

        // Set parameter name
        FString ParamName;
        if (Params->TryGetStringField(TEXT("parameter_name"), ParamName))
        {
            TexParam->ParameterName = FName(*ParamName);
        }
        else
        {
            TexParam->ParameterName = FName(TEXT("Texture"));
        }

        // Set default texture
        FString TexturePath;
        if (Params->TryGetStringField(TEXT("texture"), TexturePath))
        {
            UTexture* Texture = Cast<UTexture>(UEditorAssetLibrary::LoadAsset(TexturePath));
            if (Texture)
            {
                TexParam->Texture = Texture;
            }
        }

        // Set group
        FString Group;
        if (Params->TryGetStringField(TEXT("group"), Group))
        {
            TexParam->Group = FName(*Group);
        }

        NewExpression = TexParam;
    }
    else if (ExpressionType == TEXT("TextureSampleParameter2D"))
    {
        UMaterialExpressionTextureSampleParameter2D* TexParam = NewObject<UMaterialExpressionTextureSampleParameter2D>(Material);

        // Set parameter name
        FString ParamName;
        if (Params->TryGetStringField(TEXT("parameter_name"), ParamName))
        {
            TexParam->ParameterName = FName(*ParamName);
        }
        else
        {
            TexParam->ParameterName = FName(TEXT("Texture"));
        }

        // Set default texture
        FString TexturePath;
        if (Params->TryGetStringField(TEXT("texture"), TexturePath))
        {
            UTexture* Texture = Cast<UTexture>(UEditorAssetLibrary::LoadAsset(TexturePath));
            if (Texture)
            {
                TexParam->Texture = Texture;
            }
        }
        else
        {
            TexParam->SetDefaultTexture();
        }

        // Set group
        FString Group;
        if (Params->TryGetStringField(TEXT("group"), Group))
        {
            TexParam->Group = FName(*Group);
        }

        // Set sampler type (Color, Normal, Grayscale, Alpha, etc.)
        FString SamplerTypeStr;
        if (Params->TryGetStringField(TEXT("sampler_type"), SamplerTypeStr))
        {
            // EMaterialSamplerType enum values (based on UE5 source)
            // Color=0, Normal=1, Grayscale=2, Alpha=3, LinearColor=4, LinearGrayscale=5
            // DistanceFieldFont=6, VirtualColor=7, VirtualNormal=8, VirtualGrayscale=9, VirtualAlpha=10, External=11
            if (SamplerTypeStr == TEXT("Color"))
            {
                TexParam->SamplerType = (EMaterialSamplerType)0;
            }
            else if (SamplerTypeStr == TEXT("Normal"))
            {
                TexParam->SamplerType = (EMaterialSamplerType)1;
            }
            else if (SamplerTypeStr == TEXT("Grayscale"))
            {
                TexParam->SamplerType = (EMaterialSamplerType)2;
            }
            else if (SamplerTypeStr == TEXT("Alpha"))
            {
                TexParam->SamplerType = (EMaterialSamplerType)3;
            }
            else if (SamplerTypeStr == TEXT("LinearColor"))
            {
                TexParam->SamplerType = (EMaterialSamplerType)4;
            }
            else if (SamplerTypeStr == TEXT("LinearGrayscale"))
            {
                TexParam->SamplerType = (EMaterialSamplerType)5;
            }
            else if (SamplerTypeStr == TEXT("DistanceFieldFont"))
            {
                TexParam->SamplerType = (EMaterialSamplerType)6;
            }
            else if (SamplerTypeStr == TEXT("VirtualColor"))
            {
                TexParam->SamplerType = (EMaterialSamplerType)7;
            }
            else if (SamplerTypeStr == TEXT("VirtualNormal"))
            {
                TexParam->SamplerType = (EMaterialSamplerType)8;
            }
            else if (SamplerTypeStr == TEXT("VirtualGrayscale"))
            {
                TexParam->SamplerType = (EMaterialSamplerType)9;
            }
            else if (SamplerTypeStr == TEXT("VirtualAlpha"))
            {
                TexParam->SamplerType = (EMaterialSamplerType)10;
            }
            else if (SamplerTypeStr == TEXT("External"))
            {
                TexParam->SamplerType = (EMaterialSamplerType)11;
            }
        }

        NewExpression = TexParam;
    }
    else if (ExpressionType == TEXT("StaticBoolParameter"))
    {
        UMaterialExpressionStaticBoolParameter* BoolParam = NewObject<UMaterialExpressionStaticBoolParameter>(Material);

        // Set parameter name
        FString ParamName;
        if (Params->TryGetStringField(TEXT("parameter_name"), ParamName))
        {
            BoolParam->ParameterName = FName(*ParamName);
        }
        else
        {
            BoolParam->ParameterName = FName(TEXT("Boolean"));
        }

        // Set default value
        bool DefaultValue = false;
        if (Params->TryGetBoolField(TEXT("value"), DefaultValue))
        {
            BoolParam->DefaultValue = DefaultValue ? 1 : 0;
        }

        // Set group
        FString Group;
        if (Params->TryGetStringField(TEXT("group"), Group))
        {
            BoolParam->Group = FName(*Group);
        }

        NewExpression = BoolParam;
    }
    // ==================== NEW EXPRESSIONS ====================
    else if (ExpressionType == TEXT("Transform"))
    {
        UMaterialExpressionTransform* TransformExpr = NewObject<UMaterialExpressionTransform>(Material);
        
        // Set source type: Tangent(0), Local(1), World(2), View(3), Camera(4), Instance(6)
        int32 SourceType = 0;
        if (Params->TryGetNumberField(TEXT("source"), SourceType))
        {
            TransformExpr->TransformSourceType = (EMaterialVectorCoordTransformSource)SourceType;
        }
        
        // Set destination type: Tangent(0), Local(1), World(2), View(3), Camera(4), Instance(6)
        int32 DestType = 2; // Default to World
        if (Params->TryGetNumberField(TEXT("destination"), DestType))
        {
            TransformExpr->TransformType = (EMaterialVectorCoordTransform)DestType;
        }
        
        NewExpression = TransformExpr;
    }
    else if (ExpressionType == TEXT("DotProduct"))
    {
        NewExpression = NewObject<UMaterialExpressionDotProduct>(Material);
    }
    else if (ExpressionType == TEXT("CrossProduct"))
    {
        NewExpression = NewObject<UMaterialExpressionCrossProduct>(Material);
    }
    else if (ExpressionType == TEXT("OneMinus"))
    {
        NewExpression = NewObject<UMaterialExpressionOneMinus>(Material);
    }
    else if (ExpressionType == TEXT("Normalize"))
    {
        NewExpression = NewObject<UMaterialExpressionNormalize>(Material);
    }
    else if (ExpressionType == TEXT("Saturate"))
    {
        NewExpression = NewObject<UMaterialExpressionSaturate>(Material);
    }
    else if (ExpressionType == TEXT("ReflectionVector"))
    {
        NewExpression = NewObject<UMaterialExpressionReflectionVectorWS>(Material);
    }
    else if (ExpressionType == TEXT("VertexTangent"))
    {
        NewExpression = NewObject<UMaterialExpressionVertexTangentWS>(Material);
    }
    else if (ExpressionType == TEXT("Desaturation"))
    {
        NewExpression = NewObject<UMaterialExpressionDesaturation>(Material);
    }
    else if (ExpressionType == TEXT("DDX"))
    {
        NewExpression = NewObject<UMaterialExpressionDDX>(Material);
    }
    else if (ExpressionType == TEXT("DDY"))
    {
        NewExpression = NewObject<UMaterialExpressionDDY>(Material);
    }
    else if (ExpressionType == TEXT("TextureObject"))
    {
        UMaterialExpressionTextureObject* TexObj = NewObject<UMaterialExpressionTextureObject>(Material);
        FString TexturePath;
        if (Params->TryGetStringField(TEXT("texture"), TexturePath))
        {
            UTexture* Texture = Cast<UTexture>(UEditorAssetLibrary::LoadAsset(TexturePath));
            if (Texture)
            {
                TexObj->Texture = Texture;
            }
        }
        NewExpression = TexObj;
    }
    else if (ExpressionType == TEXT("If"))
    {
        NewExpression = NewObject<UMaterialExpressionIf>(Material);
    }
    else if (ExpressionType == TEXT("ParticleSubUV"))
    {
        UMaterialExpressionParticleSubUV* SubUV = NewObject<UMaterialExpressionParticleSubUV>(Material);
        FString TexturePath;
        if (Params->TryGetStringField(TEXT("texture"), TexturePath))
        {
            UTexture* Texture = Cast<UTexture>(UEditorAssetLibrary::LoadAsset(TexturePath));
            if (Texture)
            {
                SubUV->Texture = Texture;
            }
        }
        NewExpression = SubUV;
    }
    else if (ExpressionType == TEXT("LightVector"))
    {
        NewExpression = NewObject<UMaterialExpressionLightVector>(Material);
    }
    else if (ExpressionType == TEXT("ViewSize"))
    {
        NewExpression = NewObject<UMaterialExpressionViewSize>(Material);
    }
    else if (ExpressionType == TEXT("PreSkinnedPosition"))
    {
        NewExpression = NewObject<UMaterialExpressionPreSkinnedPosition>(Material);
    }
    else if (ExpressionType == TEXT("PreSkinnedNormal"))
    {
        NewExpression = NewObject<UMaterialExpressionPreSkinnedNormal>(Material);
    }
    else if (ExpressionType == TEXT("SquareRoot") || ExpressionType == TEXT("Sqrt"))
    {
        NewExpression = NewObject<UMaterialExpressionSquareRoot>(Material);
    }
    else if (ExpressionType == TEXT("Min"))
    {
        NewExpression = NewObject<UMaterialExpressionMin>(Material);
    }
    else if (ExpressionType == TEXT("Max"))
    {
        NewExpression = NewObject<UMaterialExpressionMax>(Material);
    }
    else if (ExpressionType == TEXT("Round"))
    {
        NewExpression = NewObject<UMaterialExpressionRound>(Material);
    }
    else if (ExpressionType == TEXT("Sign"))
    {
        NewExpression = NewObject<UMaterialExpressionSign>(Material);
    }
    else if (ExpressionType == TEXT("Step"))
    {
        NewExpression = NewObject<UMaterialExpressionStep>(Material);
    }
    else if (ExpressionType == TEXT("SmoothStep"))
    {
        NewExpression = NewObject<UMaterialExpressionSmoothStep>(Material);
    }
    else if (ExpressionType == TEXT("InverseLerp"))
    {
        NewExpression = NewObject<UMaterialExpressionInverseLinearInterpolate>(Material);
    }
    else if (ExpressionType == TEXT("StaticSwitch"))
    {
        UMaterialExpressionStaticSwitch* StaticSwitch = NewObject<UMaterialExpressionStaticSwitch>(Material);
        bool DefaultValue = false;
        if (Params->TryGetBoolField(TEXT("value"), DefaultValue))
        {
            StaticSwitch->DefaultValue = DefaultValue;
        }
        NewExpression = StaticSwitch;
    }
    else if (ExpressionType == TEXT("Reroute"))
    {
        NewExpression = NewObject<UMaterialExpressionReroute>(Material);
    }
    else if (ExpressionType == TEXT("Comment"))
    {
        UMaterialExpressionComment* Comment = NewObject<UMaterialExpressionComment>(Material);
        FString CommentText;
        if (Params->TryGetStringField(TEXT("text"), CommentText))
        {
            Comment->Text = CommentText;
        }
        int32 SizeX = 400, SizeY = 100;
        Params->TryGetNumberField(TEXT("size_x"), SizeX);
        Params->TryGetNumberField(TEXT("size_y"), SizeY);
        Comment->SizeX = SizeX;
        Comment->SizeY = SizeY;
        NewExpression = Comment;
    }
    else
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Unknown expression type: %s"), *ExpressionType));
    }
    }

    if (!NewExpression)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create material expression"));
    }

    // Set position
    NewExpression->MaterialExpressionEditorX = PosX;
    NewExpression->MaterialExpressionEditorY = PosY;
    NewExpression->Material = Material;

    // Add to material only if creating new (not updating)
    if (!bIsUpdate)
    {
        Material->GetExpressionCollection().AddExpression(NewExpression);
    }
    Material->MarkPackageDirty();

    // Generate or reuse node ID
    if (bIsUpdate)
    {
        NodeId = ExistingNodeId;
    }
    else
    {
        NodeId = FString::Printf(TEXT("Expr_%s_%d"), *ExpressionType, NewExpression->GetUniqueID());
    }

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetStringField(TEXT("node_id"), NodeId);
    ResultObj->SetStringField(TEXT("expression_type"), ExpressionType);
    ResultObj->SetNumberField(TEXT("pos_x"), PosX);
    ResultObj->SetNumberField(TEXT("pos_y"), PosY);
    ResultObj->SetBoolField(TEXT("updated"), bIsUpdate);
    ResultObj->SetBoolField(TEXT("success"), true);

    return ResultObj;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPBlueprintCommands::HandleConnectMaterialNodes(const TSharedPtr<FJsonObject>& Params)
{
    // Get material name
    FString MaterialName;
    if (!Params->TryGetStringField(TEXT("material_name"), MaterialName))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'material_name' parameter"));
    }

    // Find or load the material
    FString MaterialPath = MaterialName.StartsWith(TEXT("/")) ? MaterialName : FString::Printf(TEXT("/Game/Materials/%s"), *MaterialName);
    UMaterial* Material = Cast<UMaterial>(UEditorAssetLibrary::LoadAsset(MaterialPath));
    if (!Material)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Material not found: %s"), *MaterialPath));
    }

    // Get source and target info
    FString SourceNodeId, SourceOutput;
    FString TargetNodeId, TargetInput;
    
    Params->TryGetStringField(TEXT("source_node"), SourceNodeId);
    Params->TryGetStringField(TEXT("source_output"), SourceOutput);
    Params->TryGetStringField(TEXT("target_node"), TargetNodeId);
    Params->TryGetStringField(TEXT("target_input"), TargetInput);

    // Handle connection to material property (BaseColor, Normal, etc.)
    if (TargetNodeId == TEXT("Material") || TargetNodeId.IsEmpty())
    {
        // Connect to material property
        UMaterialExpression* SourceExpr = FindMaterialExpressionById(Material, SourceNodeId);
        if (!SourceExpr)
        {
            return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Source expression not found: %s"), *SourceNodeId));
        }

        // Map property name to actual property
        FExpressionInput* PropertyInput = GetMaterialPropertyInput(Material, TargetInput);
        if (!PropertyInput)
        {
            return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Unknown material property: %s"), *TargetInput));
        }

        PropertyInput->Expression = SourceExpr;
        PropertyInput->OutputIndex = 0;
        Material->MarkPackageDirty();

        TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
        ResultObj->SetStringField(TEXT("source_node"), SourceNodeId);
        ResultObj->SetStringField(TEXT("target_property"), TargetInput);
        ResultObj->SetBoolField(TEXT("success"), true);
        return ResultObj;
    }

    // Handle node-to-node connection
    UMaterialExpression* SourceExpr = FindMaterialExpressionById(Material, SourceNodeId);
    UMaterialExpression* TargetExpr = FindMaterialExpressionById(Material, TargetNodeId);

    if (!SourceExpr || !TargetExpr)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Source or target expression not found"));
    }

    // Find the input on target expression
    FExpressionInput* InputPtr = GetExpressionInputByName(TargetExpr, TargetInput);
    if (!InputPtr)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Input not found: %s"), *TargetInput));
    }

    // Connect
    InputPtr->Expression = SourceExpr;
    InputPtr->OutputIndex = 0;
    Material->MarkPackageDirty();

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetStringField(TEXT("source_node"), SourceNodeId);
    ResultObj->SetStringField(TEXT("target_node"), TargetNodeId);
    ResultObj->SetStringField(TEXT("target_input"), TargetInput);
    ResultObj->SetBoolField(TEXT("success"), true);

    return ResultObj;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPBlueprintCommands::HandleSetMaterialProperties(const TSharedPtr<FJsonObject>& Params)
{
    // Get material name
    FString MaterialName;
    if (!Params->TryGetStringField(TEXT("material_name"), MaterialName))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'material_name' parameter"));
    }

    // Find or load the material
    FString MaterialPath = MaterialName.StartsWith(TEXT("/")) ? MaterialName : FString::Printf(TEXT("/Game/Materials/%s"), *MaterialName);
    UMaterial* Material = Cast<UMaterial>(UEditorAssetLibrary::LoadAsset(MaterialPath));
    if (!Material)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Material not found: %s"), *MaterialPath));
    }

    TArray<FString> UpdatedProperties;

    // Shading Model
    FString ShadingModel;
    if (Params->TryGetStringField(TEXT("shading_model"), ShadingModel))
    {
        if (ShadingModel == TEXT("Unlit"))
            Material->SetShadingModel(EMaterialShadingModel::MSM_Unlit);
        else if (ShadingModel == TEXT("DefaultLit"))
            Material->SetShadingModel(EMaterialShadingModel::MSM_DefaultLit);
        else if (ShadingModel == TEXT("Subsurface"))
            Material->SetShadingModel(EMaterialShadingModel::MSM_Subsurface);
        else if (ShadingModel == TEXT("TwoSidedFoliage"))
            Material->SetShadingModel(EMaterialShadingModel::MSM_TwoSidedFoliage);
        UpdatedProperties.Add(TEXT("shading_model"));
    }

    // Blend Mode
    FString BlendMode;
    if (Params->TryGetStringField(TEXT("blend_mode"), BlendMode))
    {
        if (BlendMode == TEXT("Opaque"))
            Material->BlendMode = EBlendMode::BLEND_Opaque;
        else if (BlendMode == TEXT("Masked"))
            Material->BlendMode = EBlendMode::BLEND_Masked;
        else if (BlendMode == TEXT("Translucent"))
            Material->BlendMode = EBlendMode::BLEND_Translucent;
        else if (BlendMode == TEXT("Additive"))
            Material->BlendMode = EBlendMode::BLEND_Additive;
        UpdatedProperties.Add(TEXT("blend_mode"));
    }

    // Two Sided
    bool bTwoSided;
    if (Params->TryGetBoolField(TEXT("two_sided"), bTwoSided))
    {
        Material->TwoSided = bTwoSided ? 1 : 0;
        UpdatedProperties.Add(TEXT("two_sided"));
    }

    // Material Domain
    FString MaterialDomain;
    if (Params->TryGetStringField(TEXT("material_domain"), MaterialDomain))
    {
        // Use integer values directly for compatibility
        // MD_Surface=0, MD_DeferredDecal=1, MD_LightFunction=2, MD_Volume=3, MD_PostProcess=4, MD_UserInterface=5
        if (MaterialDomain == TEXT("Surface"))
            Material->MaterialDomain = (EMaterialDomain)0;
        else if (MaterialDomain == TEXT("DeferredDecal"))
            Material->MaterialDomain = (EMaterialDomain)1;
        else if (MaterialDomain == TEXT("LightFunction"))
            Material->MaterialDomain = (EMaterialDomain)2;
        else if (MaterialDomain == TEXT("Volume"))
            Material->MaterialDomain = (EMaterialDomain)3;
        else if (MaterialDomain == TEXT("PostProcess"))
            Material->MaterialDomain = (EMaterialDomain)4;
        else if (MaterialDomain == TEXT("UserInterface") || MaterialDomain == TEXT("UI"))
            Material->MaterialDomain = (EMaterialDomain)5;
        UpdatedProperties.Add(TEXT("material_domain"));
    }

    Material->MarkPackageDirty();

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetArrayField(TEXT("updated_properties"), 
        TArray<TSharedPtr<FJsonValue>>());
    ResultObj->SetBoolField(TEXT("success"), true);

    return ResultObj;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPBlueprintCommands::HandleCompileMaterial(const TSharedPtr<FJsonObject>& Params)
{
    // Get material name
    FString MaterialName;
    if (!Params->TryGetStringField(TEXT("material_name"), MaterialName))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'material_name' parameter"));
    }

    // Find or load the material
    FString MaterialPath = MaterialName.StartsWith(TEXT("/")) ? MaterialName : FString::Printf(TEXT("/Game/Materials/%s"), *MaterialName);
    UMaterial* Material = Cast<UMaterial>(UEditorAssetLibrary::LoadAsset(MaterialPath));
    if (!Material)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Material not found: %s"), *MaterialPath));
    }

    // Force recompile
    Material->ForceRecompileForRendering();

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetStringField(TEXT("material_name"), MaterialName);
    ResultObj->SetBoolField(TEXT("compiled"), true);
    ResultObj->SetBoolField(TEXT("success"), true);

    return ResultObj;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPBlueprintCommands::HandleGetMaterialExpressions(const TSharedPtr<FJsonObject>& Params)
{
    // Get material name
    FString MaterialName;
    if (!Params->TryGetStringField(TEXT("material_name"), MaterialName))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'material_name' parameter"));
    }

    // Find or load the material
    FString MaterialPath = MaterialName.StartsWith(TEXT("/")) ? MaterialName : FString::Printf(TEXT("/Game/Materials/%s"), *MaterialName);
    UMaterial* Material = Cast<UMaterial>(UEditorAssetLibrary::LoadAsset(MaterialPath));
    if (!Material)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Material not found: %s"), *MaterialPath));
    }

    TArray<TSharedPtr<FJsonValue>> ExpressionsArray;
    const TArray<UMaterialExpression*>& Expressions = Material->GetExpressionCollection().Expressions;

    for (UMaterialExpression* Expr : Expressions)
    {
        if (Expr)
        {
            TSharedPtr<FJsonObject> ExprObj = MakeShared<FJsonObject>();
            FString TypeName = Expr->GetClass()->GetName();
            // Remove "MaterialExpression" prefix for cleaner type name
            TypeName.ReplaceInline(TEXT("MaterialExpression"), TEXT(""));
            
            // Generate node_id
            FString NodeId = FString::Printf(TEXT("Expr_%s_%d"), *TypeName, Expr->GetUniqueID());
            
            ExprObj->SetStringField(TEXT("node_id"), NodeId);
            ExprObj->SetStringField(TEXT("type"), TypeName);
            ExprObj->SetNumberField(TEXT("pos_x"), Expr->MaterialExpressionEditorX);
            ExprObj->SetNumberField(TEXT("pos_y"), Expr->MaterialExpressionEditorY);
            ExprObj->SetStringField(TEXT("desc"), Expr->Desc);
            
            // Add parameter-specific info
            if (UMaterialExpressionScalarParameter* ScalarParam = Cast<UMaterialExpressionScalarParameter>(Expr))
            {
                ExprObj->SetStringField(TEXT("parameter_name"), ScalarParam->ParameterName.ToString());
                ExprObj->SetNumberField(TEXT("default_value"), ScalarParam->DefaultValue);
                ExprObj->SetStringField(TEXT("group"), ScalarParam->Group.ToString());
            }
            else if (UMaterialExpressionVectorParameter* VectorParam = Cast<UMaterialExpressionVectorParameter>(Expr))
            {
                ExprObj->SetStringField(TEXT("parameter_name"), VectorParam->ParameterName.ToString());
                TArray<TSharedPtr<FJsonValue>> DefaultValue;
                DefaultValue.Add(MakeShared<FJsonValueNumber>(VectorParam->DefaultValue.R));
                DefaultValue.Add(MakeShared<FJsonValueNumber>(VectorParam->DefaultValue.G));
                DefaultValue.Add(MakeShared<FJsonValueNumber>(VectorParam->DefaultValue.B));
                DefaultValue.Add(MakeShared<FJsonValueNumber>(VectorParam->DefaultValue.A));
                ExprObj->SetArrayField(TEXT("default_value"), DefaultValue);
                ExprObj->SetStringField(TEXT("group"), VectorParam->Group.ToString());
            }
            else if (UMaterialExpressionTextureSampleParameter2D* TexParam = Cast<UMaterialExpressionTextureSampleParameter2D>(Expr))
            {
                ExprObj->SetStringField(TEXT("parameter_name"), TexParam->ParameterName.ToString());
                ExprObj->SetStringField(TEXT("group"), TexParam->Group.ToString());
                if (TexParam->Texture)
                {
                    ExprObj->SetStringField(TEXT("default_texture"), TexParam->Texture->GetPathName());
                }
            }
            else if (UMaterialExpressionTextureObjectParameter* TexObjParam = Cast<UMaterialExpressionTextureObjectParameter>(Expr))
            {
                ExprObj->SetStringField(TEXT("parameter_name"), TexObjParam->ParameterName.ToString());
                ExprObj->SetStringField(TEXT("group"), TexObjParam->Group.ToString());
                // TextureObjectParameter uses the same Texture property
                if (TexObjParam->Texture)
                {
                    ExprObj->SetStringField(TEXT("default_texture"), TexObjParam->Texture->GetPathName());
                }
            }
            else if (UMaterialExpressionCustom* CustomExpr = Cast<UMaterialExpressionCustom>(Expr))
            {
                ExprObj->SetStringField(TEXT("description"), CustomExpr->Description);
                // Output type
                FString OutputTypeStr;
                switch (CustomExpr->OutputType)
                {
                    case ECustomMaterialOutputType::CMOT_Float1: OutputTypeStr = TEXT("Float1"); break;
                    case ECustomMaterialOutputType::CMOT_Float2: OutputTypeStr = TEXT("Float2"); break;
                    case ECustomMaterialOutputType::CMOT_Float3: OutputTypeStr = TEXT("Float3"); break;
                    case ECustomMaterialOutputType::CMOT_Float4: OutputTypeStr = TEXT("Float4"); break;
                    default: OutputTypeStr = TEXT("Float1"); break;
                }
                ExprObj->SetStringField(TEXT("output_type"), OutputTypeStr);
                // Input names
                TArray<TSharedPtr<FJsonValue>> InputNames;
                for (const FCustomInput& Input : CustomExpr->Inputs)
                {
                    InputNames.Add(MakeShared<FJsonValueString>(Input.InputName.ToString()));
                }
                ExprObj->SetArrayField(TEXT("inputs"), InputNames);
            }
            // MaterialFunctionCall - extract referenced function path
            else if (UMaterialExpressionMaterialFunctionCall* FuncCall = Cast<UMaterialExpressionMaterialFunctionCall>(Expr))
            {
                if (FuncCall->MaterialFunction)
                {
                    ExprObj->SetStringField(TEXT("function_path"), FuncCall->MaterialFunction->GetPathName());
                    ExprObj->SetStringField(TEXT("function_name"), FuncCall->MaterialFunction->GetName());
                }
                // Add function inputs info
                TArray<TSharedPtr<FJsonValue>> FunctionInputs;
                for (const FFunctionExpressionInput& FuncInput : FuncCall->FunctionInputs)
                {
                    TSharedPtr<FJsonObject> InputObj = MakeShared<FJsonObject>();
                    // Use ExpressionInputId which is the GUID identifier
                    InputObj->SetStringField(TEXT("input_id"), FuncInput.ExpressionInputId.ToString());
                    if (FuncInput.ExpressionInput)
                    {
                        InputObj->SetStringField(TEXT("input_name"), FuncInput.ExpressionInput->InputName.ToString());
                    }
                    FunctionInputs.Add(MakeShared<FJsonValueObject>(InputObj));
                }
                ExprObj->SetArrayField(TEXT("function_inputs"), FunctionInputs);
                // Add function outputs info
                TArray<TSharedPtr<FJsonValue>> FunctionOutputs;
                for (const FFunctionExpressionOutput& FuncOutput : FuncCall->FunctionOutputs)
                {
                    TSharedPtr<FJsonObject> OutputObj = MakeShared<FJsonObject>();
                    // Use ExpressionOutputId which is the GUID identifier
                    OutputObj->SetStringField(TEXT("output_id"), FuncOutput.ExpressionOutputId.ToString());
                    if (FuncOutput.ExpressionOutput)
                    {
                        OutputObj->SetStringField(TEXT("output_name"), FuncOutput.ExpressionOutput->OutputName.ToString());
                    }
                    FunctionOutputs.Add(MakeShared<FJsonValueObject>(OutputObj));
                }
                ExprObj->SetArrayField(TEXT("function_outputs"), FunctionOutputs);
            }
            // TextureSample - extract texture info
            else if (UMaterialExpressionTextureSample* TexSample = Cast<UMaterialExpressionTextureSample>(Expr))
            {
                if (TexSample->Texture)
                {
                    ExprObj->SetStringField(TEXT("texture_path"), TexSample->Texture->GetPathName());
                    ExprObj->SetStringField(TEXT("texture_name"), TexSample->Texture->GetName());
                }
                // Sampler type
                FString SamplerTypeStr;
                switch (TexSample->SamplerType)
                {
                    case SAMPLERTYPE_Color: SamplerTypeStr = TEXT("Color"); break;
                    case SAMPLERTYPE_LinearColor: SamplerTypeStr = TEXT("LinearColor"); break;
                    case SAMPLERTYPE_Normal: SamplerTypeStr = TEXT("Normal"); break;
                    case SAMPLERTYPE_Alpha: SamplerTypeStr = TEXT("Alpha"); break;
                    case SAMPLERTYPE_Grayscale: SamplerTypeStr = TEXT("Grayscale"); break;
                    default: SamplerTypeStr = TEXT("Color"); break;
                }
                ExprObj->SetStringField(TEXT("sampler_type"), SamplerTypeStr);
            }
            
            ExpressionsArray.Add(MakeShared<FJsonValueObject>(ExprObj));
        }
    }

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetArrayField(TEXT("expressions"), ExpressionsArray);
    ResultObj->SetNumberField(TEXT("count"), ExpressionsArray.Num());
    ResultObj->SetBoolField(TEXT("success"), true);

    return ResultObj;
}

// Helper function to find expression by ID
UMaterialExpression* FEpicUnrealMCPBlueprintCommands::FindMaterialExpressionById(UMaterial* Material, const FString& NodeId)
{
    if (!Material || NodeId.IsEmpty()) return nullptr;

    // Try to parse node ID format: Expr_Type_UniqueId
    TArray<FString> Parts;
    NodeId.ParseIntoArray(Parts, TEXT("_"), false);
    
    if (Parts.Num() >= 3)
    {
        // Last part should be the unique ID
        int32 UniqueId = FCString::Atoi(*Parts.Last());
        
        const TArray<UMaterialExpression*>& Expressions = Material->GetExpressionCollection().Expressions;
        for (UMaterialExpression* Expr : Expressions)
        {
            if (Expr && Expr->GetUniqueID() == UniqueId)
            {
                return Expr;
            }
        }
    }

    return nullptr;
}

// Helper function to get expression input by name
FExpressionInput* FEpicUnrealMCPBlueprintCommands::GetExpressionInputByName(UMaterialExpression* Expression, const FString& InputName)
{
    if (!Expression || InputName.IsEmpty()) return nullptr;

    // Common input names mapping
    FString LowerInputName = InputName.ToLower();
    
    // Try to find by iterating through inputs
    int32 InputIndex = 0;
    FExpressionInput* Input = Expression->GetInput(InputIndex);
    while (Input)
    {
        FString Name = Expression->GetInputName(InputIndex).ToString().ToLower();
        if (Name == LowerInputName)
        {
            return Input;
        }
        InputIndex++;
        Input = Expression->GetInput(InputIndex);
    }

    // Handle common cases directly
    if (UMaterialExpressionMultiply* Multiply = Cast<UMaterialExpressionMultiply>(Expression))
    {
        if (LowerInputName == TEXT("a")) return &Multiply->A;
        if (LowerInputName == TEXT("b")) return &Multiply->B;
    }
    else if (UMaterialExpressionAdd* Add = Cast<UMaterialExpressionAdd>(Expression))
    {
        if (LowerInputName == TEXT("a")) return &Add->A;
        if (LowerInputName == TEXT("b")) return &Add->B;
    }
    else if (UMaterialExpressionSubtract* Sub = Cast<UMaterialExpressionSubtract>(Expression))
    {
        if (LowerInputName == TEXT("a")) return &Sub->A;
        if (LowerInputName == TEXT("b")) return &Sub->B;
    }
    else if (UMaterialExpressionDivide* Div = Cast<UMaterialExpressionDivide>(Expression))
    {
        if (LowerInputName == TEXT("a")) return &Div->A;
        if (LowerInputName == TEXT("b")) return &Div->B;
    }
    else if (UMaterialExpressionLinearInterpolate* Lerp = Cast<UMaterialExpressionLinearInterpolate>(Expression))
    {
        if (LowerInputName == TEXT("a")) return &Lerp->A;
        if (LowerInputName == TEXT("b")) return &Lerp->B;
        if (LowerInputName == TEXT("alpha")) return &Lerp->Alpha;
    }
    else if (UMaterialExpressionClamp* Clamp = Cast<UMaterialExpressionClamp>(Expression))
    {
        if (LowerInputName == TEXT("input")) return &Clamp->Input;
    }
    else if (UMaterialExpressionPanner* Panner = Cast<UMaterialExpressionPanner>(Expression))
    {
        if (LowerInputName == TEXT("coordinate")) return &Panner->Coordinate;
        if (LowerInputName == TEXT("time")) return &Panner->Time;
    }
    else if (UMaterialExpressionRotator* Rotator = Cast<UMaterialExpressionRotator>(Expression))
    {
        if (LowerInputName == TEXT("coordinate")) return &Rotator->Coordinate;
        if (LowerInputName == TEXT("time")) return &Rotator->Time;
    }
    else if (UMaterialExpressionPower* Power = Cast<UMaterialExpressionPower>(Expression))
    {
        if (LowerInputName == TEXT("base")) return &Power->Base;
        if (LowerInputName == TEXT("exponent")) return &Power->Exponent;
    }
    else if (UMaterialExpressionTextureSample* TexSample = Cast<UMaterialExpressionTextureSample>(Expression))
    {
        if (LowerInputName == TEXT("coordinates")) return &TexSample->Coordinates;
    }

    return nullptr;
}

// Helper function to get material property input
FExpressionInput* FEpicUnrealMCPBlueprintCommands::GetMaterialPropertyInput(UMaterial* Material, const FString& PropertyName)
{
    if (!Material || PropertyName.IsEmpty()) return nullptr;

    FString LowerPropName = PropertyName.ToLower();

    // Map property names to EMaterialProperty
    EMaterialProperty MaterialProperty = MP_MAX;

    if (LowerPropName == TEXT("basecolor") || LowerPropName == TEXT("base_color"))
        MaterialProperty = MP_BaseColor;
    else if (LowerPropName == TEXT("metallic"))
        MaterialProperty = MP_Metallic;
    else if (LowerPropName == TEXT("specular"))
        MaterialProperty = MP_Specular;
    else if (LowerPropName == TEXT("roughness"))
        MaterialProperty = MP_Roughness;
    else if (LowerPropName == TEXT("normal"))
        MaterialProperty = MP_Normal;
    else if (LowerPropName == TEXT("worldpositionoffset") || LowerPropName == TEXT("world_position_offset"))
        MaterialProperty = MP_WorldPositionOffset;
    else if (LowerPropName == TEXT("emissivecolor") || LowerPropName == TEXT("emissive_color"))
        MaterialProperty = MP_EmissiveColor;
    else if (LowerPropName == TEXT("opacity"))
        MaterialProperty = MP_Opacity;
    else if (LowerPropName == TEXT("opacitymask") || LowerPropName == TEXT("opacity_mask"))
        MaterialProperty = MP_OpacityMask;
    else if (LowerPropName == TEXT("ambientocclusion") || LowerPropName == TEXT("ambient_occlusion"))
        MaterialProperty = MP_AmbientOcclusion;
    else if (LowerPropName == TEXT("refraction"))
        MaterialProperty = MP_Refraction;
    else if (LowerPropName == TEXT("pixeldepthoffset") || LowerPropName == TEXT("pixel_depth_offset"))
        MaterialProperty = MP_PixelDepthOffset;
    else if (LowerPropName == TEXT("subsurfacecolor") || LowerPropName == TEXT("subsurface_color"))
        MaterialProperty = MP_SubsurfaceColor;
    else
        return nullptr;

    return Material->GetExpressionInputForProperty(MaterialProperty);
}

TSharedPtr<FJsonObject> FEpicUnrealMCPBlueprintCommands::HandleImportTexture(const TSharedPtr<FJsonObject>& Params)
{
    // Get required parameters
    FString SourceFilePath;
    if (!Params->TryGetStringField(TEXT("source_path"), SourceFilePath))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'source_path' parameter"));
    }

    FString TextureName;
    if (!Params->TryGetStringField(TEXT("name"), TextureName))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'name' parameter"));
    }

    // Get optional parameters
    FString DestinationPath = TEXT("/Game/Textures/");
    FString CustomPath;
    if (Params->TryGetStringField(TEXT("destination_path"), CustomPath))
    {
        DestinationPath = CustomPath;
        if (!DestinationPath.EndsWith(TEXT("/")))
        {
            DestinationPath += TEXT("/");
        }
    }

    bool bDeleteSource = false;
    if (Params->TryGetBoolField(TEXT("delete_source"), bDeleteSource) && bDeleteSource)
    {
        // Will delete after successful import
    }

    // Check if source file exists
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    if (!PlatformFile.FileExists(*SourceFilePath))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Source file does not exist: %s"), *SourceFilePath));
    }

    // Check if texture already exists
    FString AssetPath = DestinationPath + TextureName;
    if (UEditorAssetLibrary::DoesAssetExist(AssetPath))
    {
        // Delete existing asset
        UEditorAssetLibrary::DeleteAsset(AssetPath);
        UE_LOG(LogTemp, Log, TEXT("Deleted existing texture: %s"), *AssetPath);
    }

    // Import the texture using AssetTools
    UTexture2D* ImportedTexture = nullptr;

    // Import using asset tools
    IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
    TArray<UObject*> ImportedAssets = AssetTools.ImportAssets(TArray<FString>{SourceFilePath}, DestinationPath);

    if (ImportedAssets.Num() > 0)
    {
        ImportedTexture = Cast<UTexture2D>(ImportedAssets[0]);
    }

    if (!ImportedTexture)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Failed to import texture from: %s"), *SourceFilePath));
    }

    // Rename if needed
    if (ImportedTexture->GetName() != TextureName)
    {
        FString PackagePath = FPaths::GetPath(ImportedTexture->GetOutermost()->GetName());
        TArray<FAssetRenameData> RenameData;
        RenameData.Add(FAssetRenameData(ImportedTexture, PackagePath, TextureName));
        AssetTools.RenameAssets(RenameData);
    }

    // Get the final asset path
    FString FinalAssetPath;
    if (ImportedTexture)
    {
        FinalAssetPath = ImportedTexture->GetPathName();
        // Remove the asset name suffix (e.g., ".T_MyTexture")
        int32 DotIndex;
        if (FinalAssetPath.FindChar('.', DotIndex))
        {
            FinalAssetPath = FinalAssetPath.Left(DotIndex);
        }
    }

    // Delete source file if requested
    if (bDeleteSource)
    {
        if (PlatformFile.DeleteFile(*SourceFilePath))
        {
            UE_LOG(LogTemp, Log, TEXT("Deleted source file: %s"), *SourceFilePath);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to delete source file: %s"), *SourceFilePath);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Successfully imported texture: %s from %s"), *TextureName, *SourceFilePath);

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetStringField(TEXT("name"), TextureName);
    ResultObj->SetStringField(TEXT("path"), FinalAssetPath);
    ResultObj->SetStringField(TEXT("source_path"), SourceFilePath);
    ResultObj->SetBoolField(TEXT("deleted_source"), bDeleteSource);
    ResultObj->SetBoolField(TEXT("success"), true);

    return ResultObj;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPBlueprintCommands::HandleSetStaticMeshAssetProperties(const TSharedPtr<FJsonObject>& Params)
{
    // Get required parameters
    FString MeshPath;
    if (!Params->TryGetStringField(TEXT("mesh_path"), MeshPath))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'mesh_path' parameter"));
    }

    // Load the static mesh asset
    UStaticMesh* StaticMesh = Cast<UStaticMesh>(UEditorAssetLibrary::LoadAsset(MeshPath));
    if (!StaticMesh)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Failed to load static mesh: %s"), *MeshPath));
    }

    TArray<TSharedPtr<FJsonValue>> UpdatedProperties;
    StaticMesh->Modify();

    // Set materials (array of {slot, material_path})
    const TArray<TSharedPtr<FJsonValue>>* MaterialsArray = nullptr;
    if (Params->TryGetArrayField(TEXT("materials"), MaterialsArray) && MaterialsArray)
    {
        TArray<FStaticMaterial>& StaticMaterials = StaticMesh->GetStaticMaterials();
        
        for (const TSharedPtr<FJsonValue>& MaterialValue : *MaterialsArray)
        {
            TSharedPtr<FJsonObject> MatObj = MaterialValue->AsObject();
            if (!MatObj) continue;

            int32 Slot = 0;
            if (MatObj->TryGetNumberField(TEXT("slot"), Slot))
            {
                FString MaterialPath;
                if (MatObj->TryGetStringField(TEXT("material_path"), MaterialPath))
                {
                    UMaterialInterface* Material = Cast<UMaterialInterface>(UEditorAssetLibrary::LoadAsset(MaterialPath));
                    if (Material)
                    {
                        if (Slot >= 0 && Slot < StaticMaterials.Num())
                        {
                            StaticMaterials[Slot].MaterialInterface = Material;
                            UpdatedProperties.Add(MakeShared<FJsonValueString>(FString::Printf(TEXT("material_slot_%d"), Slot)));
                        }
                    }
                }
            }
        }
    }

    // Mark package dirty
    StaticMesh->MarkPackageDirty();

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetStringField(TEXT("mesh_path"), MeshPath);
    ResultObj->SetArrayField(TEXT("updated_properties"), UpdatedProperties);
    ResultObj->SetBoolField(TEXT("success"), true);

    return ResultObj;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPBlueprintCommands::HandleGetMaterialFunctions(const TSharedPtr<FJsonObject>& Params)
{
    // Get search path (optional, defaults to /Engine/Functions/)
    FString SearchPath;
    if (!Params->TryGetStringField(TEXT("path"), SearchPath))
    {
        SearchPath = TEXT("/Engine/Functions/");
    }
    
    // Get optional filter string
    FString NameFilter;
    Params->TryGetStringField(TEXT("name_filter"), NameFilter);

    // Use asset registry to find material functions
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

    // Build asset data filter
    FARFilter Filter;
    Filter.PackagePaths.Add(*SearchPath);
    Filter.bRecursivePaths = true;
    Filter.bRecursiveClasses = true;
    Filter.ClassPaths.Add(FTopLevelAssetPath(TEXT("/Script/Engine.MaterialFunction")));

    TArray<FAssetData> AssetDataList;
    AssetRegistry.GetAssets(Filter, AssetDataList);

    TArray<TSharedPtr<FJsonValue>> FunctionsArray;
    
    for (const FAssetData& AssetData : AssetDataList)
    {
        FString AssetName = AssetData.AssetName.ToString();
        FString AssetPath = AssetData.GetObjectPathString();
        
        // Apply name filter if specified
        if (!NameFilter.IsEmpty() && !AssetName.Contains(NameFilter))
        {
            continue;
        }
        
        TSharedPtr<FJsonObject> FunctionObj = MakeShared<FJsonObject>();
        FunctionObj->SetStringField(TEXT("name"), AssetName);
        FunctionObj->SetStringField(TEXT("path"), AssetPath);
        FunctionObj->SetStringField(TEXT("package"), AssetData.PackageName.ToString());
        
        FunctionsArray.Add(MakeShared<FJsonValueObject>(FunctionObj));
    }

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetArrayField(TEXT("material_functions"), FunctionsArray);
    ResultObj->SetNumberField(TEXT("count"), FunctionsArray.Num());
    ResultObj->SetStringField(TEXT("search_path_used"), SearchPath);
    ResultObj->SetBoolField(TEXT("success"), true);

    return ResultObj;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPBlueprintCommands::HandleGetMaterialFunctionContent(const TSharedPtr<FJsonObject>& Params)
{
    // Get material function path
    FString FunctionPath;
    if (!Params->TryGetStringField(TEXT("function_path"), FunctionPath))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'function_path' parameter"));
    }

    // Load the material function
    UMaterialFunction* MaterialFunction = Cast<UMaterialFunction>(UEditorAssetLibrary::LoadAsset(FunctionPath));
    if (!MaterialFunction)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Material Function not found: %s"), *FunctionPath));
    }

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetStringField(TEXT("name"), MaterialFunction->GetName());
    ResultObj->SetStringField(TEXT("path"), FunctionPath);
    ResultObj->SetStringField(TEXT("description"), MaterialFunction->Description);

    // Get inputs and outputs from expression nodes
    TArray<TSharedPtr<FJsonValue>> InputsArray;
    TArray<TSharedPtr<FJsonValue>> OutputsArray;
    
    const TArray<UMaterialExpression*>& Expressions = MaterialFunction->GetExpressionCollection().Expressions;
    
    for (UMaterialExpression* Expr : Expressions)
    {
        if (!Expr) continue;
        
        // Check for Function Input
        if (UMaterialExpressionFunctionInput* InputExpr = Cast<UMaterialExpressionFunctionInput>(Expr))
        {
            TSharedPtr<FJsonObject> InputObj = MakeShared<FJsonObject>();
            InputObj->SetStringField(TEXT("name"), InputExpr->InputName.ToString());
            InputObj->SetStringField(TEXT("description"), InputExpr->Description);
            InputObj->SetStringField(TEXT("id"), InputExpr->Id.ToString());
            
            // Determine input type from InputType property
            FString InputType;
            switch (InputExpr->InputType)
            {
                case FunctionInput_Vector2: InputType = TEXT("Vector2"); break;
                case FunctionInput_Vector3: InputType = TEXT("Vector3"); break;
                case FunctionInput_Vector4: InputType = TEXT("Vector4"); break;
                case FunctionInput_Scalar: InputType = TEXT("Scalar"); break;
                case FunctionInput_Bool: InputType = TEXT("Bool"); break;
                default: InputType = TEXT("Unknown"); break;
            }
            InputObj->SetStringField(TEXT("type"), InputType);
            
            InputsArray.Add(MakeShared<FJsonValueObject>(InputObj));
        }
        // Check for Function Output
        else if (UMaterialExpressionFunctionOutput* OutputExpr = Cast<UMaterialExpressionFunctionOutput>(Expr))
        {
            TSharedPtr<FJsonObject> OutputObj = MakeShared<FJsonObject>();
            OutputObj->SetStringField(TEXT("name"), OutputExpr->OutputName.ToString());
            OutputObj->SetStringField(TEXT("description"), OutputExpr->Description);
            OutputObj->SetStringField(TEXT("id"), OutputExpr->Id.ToString());
            OutputsArray.Add(MakeShared<FJsonValueObject>(OutputObj));
        }
    }
    
    ResultObj->SetArrayField(TEXT("inputs"), InputsArray);
    ResultObj->SetArrayField(TEXT("outputs"), OutputsArray);

    // Get all expressions (including non-input/output nodes)
    TArray<TSharedPtr<FJsonValue>> ExpressionsArray;
    
    for (UMaterialExpression* Expr : Expressions)
    {
        if (!Expr) continue;
        
        TSharedPtr<FJsonObject> ExprObj = MakeShared<FJsonObject>();
        
        // Get expression type
        FString ExprType = Expr->GetClass()->GetName();
        // Remove "MaterialExpression" prefix if present
        if (ExprType.StartsWith(TEXT("MaterialExpression")))
        {
            ExprType = ExprType.Mid(19); // len("MaterialExpression") = 19
        }
        ExprObj->SetStringField(TEXT("type"), ExprType);
        ExprObj->SetStringField(TEXT("name"), Expr->GetName());
        
        // Position
        ExprObj->SetNumberField(TEXT("pos_x"), Expr->MaterialExpressionEditorX);
        ExprObj->SetNumberField(TEXT("pos_y"), Expr->MaterialExpressionEditorY);
        
        // Description
        if (!Expr->Desc.IsEmpty())
        {
            ExprObj->SetStringField(TEXT("desc"), Expr->Desc);
        }

        // Add type-specific information
        if (UMaterialExpressionFunctionInput* InputExpr = Cast<UMaterialExpressionFunctionInput>(Expr))
        {
            ExprObj->SetStringField(TEXT("input_name"), InputExpr->InputName.ToString());
            ExprObj->SetStringField(TEXT("input_id"), InputExpr->Id.ToString());
            ExprObj->SetStringField(TEXT("description"), InputExpr->Description);
        }
        else if (UMaterialExpressionFunctionOutput* OutputExpr = Cast<UMaterialExpressionFunctionOutput>(Expr))
        {
            ExprObj->SetStringField(TEXT("output_name"), OutputExpr->OutputName.ToString());
            ExprObj->SetStringField(TEXT("output_id"), OutputExpr->Id.ToString());
            ExprObj->SetStringField(TEXT("description"), OutputExpr->Description);
        }
        else if (UMaterialExpressionScalarParameter* ScalarParam = Cast<UMaterialExpressionScalarParameter>(Expr))
        {
            ExprObj->SetStringField(TEXT("parameter_name"), ScalarParam->ParameterName.ToString());
            ExprObj->SetNumberField(TEXT("default_value"), ScalarParam->DefaultValue);
        }
        else if (UMaterialExpressionVectorParameter* VectorParam = Cast<UMaterialExpressionVectorParameter>(Expr))
        {
            ExprObj->SetStringField(TEXT("parameter_name"), VectorParam->ParameterName.ToString());
            TArray<TSharedPtr<FJsonValue>> DefaultValue;
            DefaultValue.Add(MakeShared<FJsonValueNumber>(VectorParam->DefaultValue.R));
            DefaultValue.Add(MakeShared<FJsonValueNumber>(VectorParam->DefaultValue.G));
            DefaultValue.Add(MakeShared<FJsonValueNumber>(VectorParam->DefaultValue.B));
            DefaultValue.Add(MakeShared<FJsonValueNumber>(VectorParam->DefaultValue.A));
            ExprObj->SetArrayField(TEXT("default_value"), DefaultValue);
        }
        else if (UMaterialExpressionConstant* ConstExpr = Cast<UMaterialExpressionConstant>(Expr))
        {
            ExprObj->SetNumberField(TEXT("value"), ConstExpr->R);
        }
        else if (UMaterialExpressionConstant2Vector* Const2Expr = Cast<UMaterialExpressionConstant2Vector>(Expr))
        {
            TArray<TSharedPtr<FJsonValue>> ValueArray;
            ValueArray.Add(MakeShared<FJsonValueNumber>(Const2Expr->R));
            ValueArray.Add(MakeShared<FJsonValueNumber>(Const2Expr->G));
            ExprObj->SetArrayField(TEXT("value"), ValueArray);
        }
        else if (UMaterialExpressionConstant3Vector* Const3Expr = Cast<UMaterialExpressionConstant3Vector>(Expr))
        {
            TArray<TSharedPtr<FJsonValue>> ValueArray;
            ValueArray.Add(MakeShared<FJsonValueNumber>(Const3Expr->Constant.R));
            ValueArray.Add(MakeShared<FJsonValueNumber>(Const3Expr->Constant.G));
            ValueArray.Add(MakeShared<FJsonValueNumber>(Const3Expr->Constant.B));
            ExprObj->SetArrayField(TEXT("value"), ValueArray);
        }
        else if (UMaterialExpressionConstant4Vector* Const4Expr = Cast<UMaterialExpressionConstant4Vector>(Expr))
        {
            TArray<TSharedPtr<FJsonValue>> ValueArray;
            ValueArray.Add(MakeShared<FJsonValueNumber>(Const4Expr->Constant.R));
            ValueArray.Add(MakeShared<FJsonValueNumber>(Const4Expr->Constant.G));
            ValueArray.Add(MakeShared<FJsonValueNumber>(Const4Expr->Constant.B));
            ValueArray.Add(MakeShared<FJsonValueNumber>(Const4Expr->Constant.A));
            ExprObj->SetArrayField(TEXT("value"), ValueArray);
        }
        else if (UMaterialExpressionCustom* CustomExpr = Cast<UMaterialExpressionCustom>(Expr))
        {
            ExprObj->SetStringField(TEXT("code"), CustomExpr->Code);
            ExprObj->SetStringField(TEXT("output_type"), 
                CustomExpr->OutputType == ECustomMaterialOutputType::CMOT_Float1 ? TEXT("Float1") :
                CustomExpr->OutputType == ECustomMaterialOutputType::CMOT_Float2 ? TEXT("Float2") :
                CustomExpr->OutputType == ECustomMaterialOutputType::CMOT_Float3 ? TEXT("Float3") :
                CustomExpr->OutputType == ECustomMaterialOutputType::CMOT_Float4 ? TEXT("Float4") : TEXT("Unknown"));
            
            // Add inputs
            TArray<TSharedPtr<FJsonValue>> CustomInputs;
            for (const FCustomInput& CustomInput : CustomExpr->Inputs)
            {
                TSharedPtr<FJsonObject> CustomInputObj = MakeShared<FJsonObject>();
                CustomInputObj->SetStringField(TEXT("input_name"), CustomInput.InputName.ToString());
                CustomInputs.Add(MakeShared<FJsonValueObject>(CustomInputObj));
            }
            ExprObj->SetArrayField(TEXT("inputs"), CustomInputs);
        }
        
        ExpressionsArray.Add(MakeShared<FJsonValueObject>(ExprObj));
    }
    
    ResultObj->SetArrayField(TEXT("expressions"), ExpressionsArray);
    ResultObj->SetNumberField(TEXT("expression_count"), ExpressionsArray.Num());
    
    // Build connection map for expressions
    TMap<UMaterialExpression*, FString> ExprToNodeId;
    for (UMaterialExpression* Expr : Expressions)
    {
        if (Expr)
        {
            FString TypeName = Expr->GetClass()->GetName();
            TypeName.ReplaceInline(TEXT("MaterialExpression"), TEXT(""));
            FString NodeId = FString::Printf(TEXT("Expr_%s_%d"), *TypeName, Expr->GetUniqueID());
            ExprToNodeId.Add(Expr, NodeId);
        }
    }
    
    // Get all connections between expressions
    TArray<TSharedPtr<FJsonValue>> ConnectionsArray;
    
    for (UMaterialExpression* Expr : Expressions)
    {
        if (!Expr) continue;
        
        FString SourceNodeId = ExprToNodeId[Expr];
        
        // Get inputs by iterating through GetInput
        int32 InputIndex = 0;
        FExpressionInput* Input = Expr->GetInput(InputIndex);
        while (Input)
        {
            if (Input->Expression)
            {
                FString* TargetNodeId = ExprToNodeId.Find(Input->Expression);
                if (TargetNodeId)
                {
                    TSharedPtr<FJsonObject> ConnObj = MakeShared<FJsonObject>();
                    ConnObj->SetStringField(TEXT("from"), *TargetNodeId);
                    ConnObj->SetStringField(TEXT("to"), SourceNodeId);
                    ConnObj->SetStringField(TEXT("from_output"), FString::Printf(TEXT("Output_%d"), Input->OutputIndex));
                    ConnObj->SetStringField(TEXT("to_input"), FString::Printf(TEXT("Input_%d"), InputIndex));
                    
                    ConnectionsArray.Add(MakeShared<FJsonValueObject>(ConnObj));
                }
            }
            InputIndex++;
            Input = Expr->GetInput(InputIndex);
        }
        
        // Handle specific expression types with named inputs
        auto AddConnection = [&](UMaterialExpression* SourceExpr, const FString& OutputName, const FString& InputName)
        {
            if (SourceExpr)
            {
                FString* FromNodeId = ExprToNodeId.Find(SourceExpr);
                if (FromNodeId)
                {
                    TSharedPtr<FJsonObject> ConnObj = MakeShared<FJsonObject>();
                    ConnObj->SetStringField(TEXT("from"), *FromNodeId);
                    ConnObj->SetStringField(TEXT("to"), SourceNodeId);
                    ConnObj->SetStringField(TEXT("from_output"), OutputName);
                    ConnObj->SetStringField(TEXT("to_input"), InputName);
                    
                    ConnectionsArray.Add(MakeShared<FJsonValueObject>(ConnObj));
                }
            }
        };
        
        // Handle common expression types
        if (UMaterialExpressionMultiply* Multiply = Cast<UMaterialExpressionMultiply>(Expr))
        {
            if (Multiply->A.Expression) AddConnection(Multiply->A.Expression, FString::Printf(TEXT("Output_%d"), Multiply->A.OutputIndex), TEXT("A"));
            if (Multiply->B.Expression) AddConnection(Multiply->B.Expression, FString::Printf(TEXT("Output_%d"), Multiply->B.OutputIndex), TEXT("B"));
        }
        else if (UMaterialExpressionAdd* Add = Cast<UMaterialExpressionAdd>(Expr))
        {
            if (Add->A.Expression) AddConnection(Add->A.Expression, FString::Printf(TEXT("Output_%d"), Add->A.OutputIndex), TEXT("A"));
            if (Add->B.Expression) AddConnection(Add->B.Expression, FString::Printf(TEXT("Output_%d"), Add->B.OutputIndex), TEXT("B"));
        }
        else if (UMaterialExpressionSubtract* Sub = Cast<UMaterialExpressionSubtract>(Expr))
        {
            if (Sub->A.Expression) AddConnection(Sub->A.Expression, FString::Printf(TEXT("Output_%d"), Sub->A.OutputIndex), TEXT("A"));
            if (Sub->B.Expression) AddConnection(Sub->B.Expression, FString::Printf(TEXT("Output_%d"), Sub->B.OutputIndex), TEXT("B"));
        }
        else if (UMaterialExpressionDivide* Div = Cast<UMaterialExpressionDivide>(Expr))
        {
            if (Div->A.Expression) AddConnection(Div->A.Expression, FString::Printf(TEXT("Output_%d"), Div->A.OutputIndex), TEXT("A"));
            if (Div->B.Expression) AddConnection(Div->B.Expression, FString::Printf(TEXT("Output_%d"), Div->B.OutputIndex), TEXT("B"));
        }
        else if (UMaterialExpressionLinearInterpolate* Lerp = Cast<UMaterialExpressionLinearInterpolate>(Expr))
        {
            if (Lerp->A.Expression) AddConnection(Lerp->A.Expression, FString::Printf(TEXT("Output_%d"), Lerp->A.OutputIndex), TEXT("A"));
            if (Lerp->B.Expression) AddConnection(Lerp->B.Expression, FString::Printf(TEXT("Output_%d"), Lerp->B.OutputIndex), TEXT("B"));
            if (Lerp->Alpha.Expression) AddConnection(Lerp->Alpha.Expression, FString::Printf(TEXT("Output_%d"), Lerp->Alpha.OutputIndex), TEXT("Alpha"));
        }
        else if (UMaterialExpressionTextureSample* TexSample = Cast<UMaterialExpressionTextureSample>(Expr))
        {
            if (TexSample->Coordinates.Expression) AddConnection(TexSample->Coordinates.Expression, FString::Printf(TEXT("Output_%d"), TexSample->Coordinates.OutputIndex), TEXT("Coordinates"));
        }
        else if (UMaterialExpressionFunctionOutput* FuncOutput = Cast<UMaterialExpressionFunctionOutput>(Expr))
        {
            if (FuncOutput->A.Expression) AddConnection(FuncOutput->A.Expression, FString::Printf(TEXT("Output_%d"), FuncOutput->A.OutputIndex), TEXT("Input"));
        }
        else if (UMaterialExpressionComponentMask* Mask = Cast<UMaterialExpressionComponentMask>(Expr))
        {
            if (Mask->Input.Expression) AddConnection(Mask->Input.Expression, FString::Printf(TEXT("Output_%d"), Mask->Input.OutputIndex), TEXT("Input"));
        }
        else if (UMaterialExpressionOneMinus* OneMinus = Cast<UMaterialExpressionOneMinus>(Expr))
        {
            if (OneMinus->Input.Expression) AddConnection(OneMinus->Input.Expression, FString::Printf(TEXT("Output_%d"), OneMinus->Input.OutputIndex), TEXT("Input"));
        }
        else if (UMaterialExpressionSaturate* Saturate = Cast<UMaterialExpressionSaturate>(Expr))
        {
            if (Saturate->Input.Expression) AddConnection(Saturate->Input.Expression, FString::Printf(TEXT("Output_%d"), Saturate->Input.OutputIndex), TEXT("Input"));
        }
        else if (UMaterialExpressionAbs* AbsExpr = Cast<UMaterialExpressionAbs>(Expr))
        {
            if (AbsExpr->Input.Expression) AddConnection(AbsExpr->Input.Expression, FString::Printf(TEXT("Output_%d"), AbsExpr->Input.OutputIndex), TEXT("Input"));
        }
        else if (UMaterialExpressionCeil* Ceil = Cast<UMaterialExpressionCeil>(Expr))
        {
            if (Ceil->Input.Expression) AddConnection(Ceil->Input.Expression, FString::Printf(TEXT("Output_%d"), Ceil->Input.OutputIndex), TEXT("Input"));
        }
        else if (UMaterialExpressionFloor* Floor = Cast<UMaterialExpressionFloor>(Expr))
        {
            if (Floor->Input.Expression) AddConnection(Floor->Input.Expression, FString::Printf(TEXT("Output_%d"), Floor->Input.OutputIndex), TEXT("Input"));
        }
        else if (UMaterialExpressionFrac* Frac = Cast<UMaterialExpressionFrac>(Expr))
        {
            if (Frac->Input.Expression) AddConnection(Frac->Input.Expression, FString::Printf(TEXT("Output_%d"), Frac->Input.OutputIndex), TEXT("Input"));
        }
        else if (UMaterialExpressionSine* Sine = Cast<UMaterialExpressionSine>(Expr))
        {
            if (Sine->Input.Expression) AddConnection(Sine->Input.Expression, FString::Printf(TEXT("Output_%d"), Sine->Input.OutputIndex), TEXT("Input"));
        }
        else if (UMaterialExpressionCosine* Cosine = Cast<UMaterialExpressionCosine>(Expr))
        {
            if (Cosine->Input.Expression) AddConnection(Cosine->Input.Expression, FString::Printf(TEXT("Output_%d"), Cosine->Input.OutputIndex), TEXT("Input"));
        }
        else if (UMaterialExpressionPower* Power = Cast<UMaterialExpressionPower>(Expr))
        {
            if (Power->Base.Expression) AddConnection(Power->Base.Expression, FString::Printf(TEXT("Output_%d"), Power->Base.OutputIndex), TEXT("Base"));
            if (Power->Exponent.Expression) AddConnection(Power->Exponent.Expression, FString::Printf(TEXT("Output_%d"), Power->Exponent.OutputIndex), TEXT("Exponent"));
        }
        else if (UMaterialExpressionDotProduct* DotProd = Cast<UMaterialExpressionDotProduct>(Expr))
        {
            if (DotProd->A.Expression) AddConnection(DotProd->A.Expression, FString::Printf(TEXT("Output_%d"), DotProd->A.OutputIndex), TEXT("A"));
            if (DotProd->B.Expression) AddConnection(DotProd->B.Expression, FString::Printf(TEXT("Output_%d"), DotProd->B.OutputIndex), TEXT("B"));
        }
        else if (UMaterialExpressionCrossProduct* CrossProd = Cast<UMaterialExpressionCrossProduct>(Expr))
        {
            if (CrossProd->A.Expression) AddConnection(CrossProd->A.Expression, FString::Printf(TEXT("Output_%d"), CrossProd->A.OutputIndex), TEXT("A"));
            if (CrossProd->B.Expression) AddConnection(CrossProd->B.Expression, FString::Printf(TEXT("Output_%d"), CrossProd->B.OutputIndex), TEXT("B"));
        }
        else if (UMaterialExpressionNormalize* Normalize = Cast<UMaterialExpressionNormalize>(Expr))
        {
            if (Normalize->VectorInput.Expression) AddConnection(Normalize->VectorInput.Expression, FString::Printf(TEXT("Output_%d"), Normalize->VectorInput.OutputIndex), TEXT("VectorInput"));
        }
        else if (UMaterialExpressionAppendVector* Append = Cast<UMaterialExpressionAppendVector>(Expr))
        {
            if (Append->A.Expression) AddConnection(Append->A.Expression, FString::Printf(TEXT("Output_%d"), Append->A.OutputIndex), TEXT("A"));
            if (Append->B.Expression) AddConnection(Append->B.Expression, FString::Printf(TEXT("Output_%d"), Append->B.OutputIndex), TEXT("B"));
        }
        else if (UMaterialExpressionPanner* Panner = Cast<UMaterialExpressionPanner>(Expr))
        {
            if (Panner->Coordinate.Expression) AddConnection(Panner->Coordinate.Expression, FString::Printf(TEXT("Output_%d"), Panner->Coordinate.OutputIndex), TEXT("Coordinate"));
            if (Panner->Time.Expression) AddConnection(Panner->Time.Expression, FString::Printf(TEXT("Output_%d"), Panner->Time.OutputIndex), TEXT("Time"));
        }
        else if (UMaterialExpressionRotator* Rotator = Cast<UMaterialExpressionRotator>(Expr))
        {
            if (Rotator->Coordinate.Expression) AddConnection(Rotator->Coordinate.Expression, FString::Printf(TEXT("Output_%d"), Rotator->Coordinate.OutputIndex), TEXT("Coordinate"));
            if (Rotator->Time.Expression) AddConnection(Rotator->Time.Expression, FString::Printf(TEXT("Output_%d"), Rotator->Time.OutputIndex), TEXT("Time"));
        }
        else if (UMaterialExpressionDesaturation* Desat = Cast<UMaterialExpressionDesaturation>(Expr))
        {
            if (Desat->Input.Expression) AddConnection(Desat->Input.Expression, FString::Printf(TEXT("Output_%d"), Desat->Input.OutputIndex), TEXT("Input"));
            if (Desat->Fraction.Expression) AddConnection(Desat->Fraction.Expression, FString::Printf(TEXT("Output_%d"), Desat->Fraction.OutputIndex), TEXT("Fraction"));
        }
        else if (UMaterialExpressionMaterialFunctionCall* FuncCall = Cast<UMaterialExpressionMaterialFunctionCall>(Expr))
        {
            for (int32 FuncInputIdx = 0; FuncInputIdx < FuncCall->FunctionInputs.Num(); FuncInputIdx++)
            {
                const FFunctionExpressionInput& FuncInput = FuncCall->FunctionInputs[FuncInputIdx];
                if (FuncInput.Input.Expression)
                {
                    FString InputName = FuncInput.ExpressionInput ? FuncInput.ExpressionInput->InputName.ToString() : FString::Printf(TEXT("Input_%d"), FuncInputIdx);
                    AddConnection(FuncInput.Input.Expression, FString::Printf(TEXT("Output_%d"), FuncInput.Input.OutputIndex), InputName);
                }
            }
        }
    }
    
    ResultObj->SetArrayField(TEXT("connections"), ConnectionsArray);
    ResultObj->SetNumberField(TEXT("connection_count"), ConnectionsArray.Num());
    ResultObj->SetBoolField(TEXT("success"), true);

    return ResultObj;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPBlueprintCommands::HandleGetMaterialProperties(const TSharedPtr<FJsonObject>& Params)
{
    // Get material name
    FString MaterialName;
    if (!Params->TryGetStringField(TEXT("material_name"), MaterialName))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'material_name' parameter"));
    }

    // Find or load the material
    FString MaterialPath = MaterialName.StartsWith(TEXT("/")) ? MaterialName : FString::Printf(TEXT("/Game/Materials/%s"), *MaterialName);
    UMaterial* Material = Cast<UMaterial>(UEditorAssetLibrary::LoadAsset(MaterialPath));
    if (!Material)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Material not found: %s"), *MaterialPath));
    }

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    
    // Blend Mode
    FString BlendModeStr;
    switch (Material->BlendMode)
    {
        case BLEND_Opaque: BlendModeStr = TEXT("Opaque"); break;
        case BLEND_Masked: BlendModeStr = TEXT("Masked"); break;
        case BLEND_Translucent: BlendModeStr = TEXT("Translucent"); break;
        case BLEND_Additive: BlendModeStr = TEXT("Additive"); break;
        case BLEND_Modulate: BlendModeStr = TEXT("Modulate"); break;
        case BLEND_AlphaComposite: BlendModeStr = TEXT("AlphaComposite"); break;
        default: BlendModeStr = FString::Printf(TEXT("Unknown(%d)"), (int32)Material->BlendMode); break;
    }
    ResultObj->SetStringField(TEXT("blend_mode"), BlendModeStr);

    // Shading Model - use GetShadingModels() for UE5.7 compatibility
    FMaterialShadingModelField ShadingModels = Material->GetShadingModels();
    TArray<FString> ShadingModelNames;
    
    if (ShadingModels.HasShadingModel(MSM_Unlit)) ShadingModelNames.Add(TEXT("Unlit"));
    if (ShadingModels.HasShadingModel(MSM_DefaultLit)) ShadingModelNames.Add(TEXT("DefaultLit"));
    if (ShadingModels.HasShadingModel(MSM_Subsurface)) ShadingModelNames.Add(TEXT("Subsurface"));
    if (ShadingModels.HasShadingModel(MSM_PreintegratedSkin)) ShadingModelNames.Add(TEXT("PreintegratedSkin"));
    if (ShadingModels.HasShadingModel(MSM_ClearCoat)) ShadingModelNames.Add(TEXT("ClearCoat"));
    if (ShadingModels.HasShadingModel(MSM_SubsurfaceProfile)) ShadingModelNames.Add(TEXT("SubsurfaceProfile"));
    if (ShadingModels.HasShadingModel(MSM_TwoSidedFoliage)) ShadingModelNames.Add(TEXT("TwoSidedFoliage"));
    if (ShadingModels.HasShadingModel(MSM_Hair)) ShadingModelNames.Add(TEXT("Hair"));
    if (ShadingModels.HasShadingModel(MSM_Cloth)) ShadingModelNames.Add(TEXT("Cloth"));
    if (ShadingModels.HasShadingModel(MSM_Eye)) ShadingModelNames.Add(TEXT("Eye"));
    if (ShadingModels.HasShadingModel(MSM_SingleLayerWater)) ShadingModelNames.Add(TEXT("SingleLayerWater"));
    if (ShadingModels.HasShadingModel(MSM_ThinTranslucent)) ShadingModelNames.Add(TEXT("ThinTranslucent"));
    if (ShadingModels.HasShadingModel(MSM_Strata)) ShadingModelNames.Add(TEXT("Strata"));
    
    if (ShadingModelNames.Num() == 0)
    {
        ShadingModelNames.Add(TEXT("DefaultLit"));
    }
    
    TArray<TSharedPtr<FJsonValue>> ShadingModelsArray;
    for (const FString& ModelName : ShadingModelNames)
    {
        ShadingModelsArray.Add(MakeShared<FJsonValueString>(ModelName));
    }
    ResultObj->SetArrayField(TEXT("shading_models"), ShadingModelsArray);
    // Primary shading model for backward compatibility
    ResultObj->SetStringField(TEXT("shading_model"), ShadingModelNames[0]);

    // Two Sided
    ResultObj->SetBoolField(TEXT("two_sided"), Material->TwoSided ? true : false);

    // Material Domain
    FString MaterialDomainStr;
    switch (Material->MaterialDomain)
    {
        case MD_Surface: MaterialDomainStr = TEXT("Surface"); break;
        case MD_DeferredDecal: MaterialDomainStr = TEXT("DeferredDecal"); break;
        case MD_LightFunction: MaterialDomainStr = TEXT("LightFunction"); break;
        case MD_Volume: MaterialDomainStr = TEXT("Volume"); break;
        case MD_PostProcess: MaterialDomainStr = TEXT("PostProcess"); break;
        case MD_UI: MaterialDomainStr = TEXT("UserInterface"); break;
        default: MaterialDomainStr = TEXT("Surface"); break;
    }
    ResultObj->SetStringField(TEXT("material_domain"), MaterialDomainStr);

    // Additional properties
    ResultObj->SetBoolField(TEXT("is_masked"), Material->IsMasked());
    ResultObj->SetBoolField(TEXT("is_blend_mode_masked"), Material->GetBlendMode() == BLEND_Masked);
    
    // Dithered LOD transition
    ResultObj->SetBoolField(TEXT("dithered_lod_transition"), Material->DitheredLODTransition);
    
    // Dither opacity mask
    ResultObj->SetBoolField(TEXT("dither_opacity_mask"), Material->DitherOpacityMask ? true : false);

    // Output opacity mask
    if (Material->IsMasked())
    {
        ResultObj->SetNumberField(TEXT("opacity_mask_clip_value"), Material->GetOpacityMaskClipValue());
    }

    // Cast ray traced shadow
    ResultObj->SetBoolField(TEXT("cast_ray_traced_shadows"), Material->bCastRayTracedShadows);

    ResultObj->SetStringField(TEXT("material_name"), Material->GetName());
    ResultObj->SetStringField(TEXT("material_path"), Material->GetPathName());
    ResultObj->SetBoolField(TEXT("success"), true);

    return ResultObj;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPBlueprintCommands::HandleGetMaterialConnections(const TSharedPtr<FJsonObject>& Params)
{
    // Get material name
    FString MaterialName;
    if (!Params->TryGetStringField(TEXT("material_name"), MaterialName))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'material_name' parameter"));
    }

    // Find or load the material
    FString MaterialPath = MaterialName.StartsWith(TEXT("/")) ? MaterialName : FString::Printf(TEXT("/Game/Materials/%s"), *MaterialName);
    UMaterial* Material = Cast<UMaterial>(UEditorAssetLibrary::LoadAsset(MaterialPath));
    if (!Material)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Material not found: %s"), *MaterialPath));
    }

    // Map from expression pointer to node_id
    TMap<UMaterialExpression*, FString> ExprToNodeId;
    const TArray<UMaterialExpression*>& Expressions = Material->GetExpressionCollection().Expressions;
    
    // Build the expression to node_id map
    for (UMaterialExpression* Expr : Expressions)
    {
        if (Expr)
        {
            FString TypeName = Expr->GetClass()->GetName();
            TypeName.ReplaceInline(TEXT("MaterialExpression"), TEXT(""));
            FString NodeId = FString::Printf(TEXT("Expr_%s_%d"), *TypeName, Expr->GetUniqueID());
            ExprToNodeId.Add(Expr, NodeId);
        }
    }

    TArray<TSharedPtr<FJsonValue>> NodeConnectionsArray;
    
    // Iterate through all expressions and get their input connections
    for (UMaterialExpression* Expr : Expressions)
    {
        if (!Expr) continue;
        
        FString NodeId = ExprToNodeId[Expr];
        TSharedPtr<FJsonObject> NodeConnObj = MakeShared<FJsonObject>();
        NodeConnObj->SetStringField(TEXT("node_id"), NodeId);
        
        TSharedPtr<FJsonObject> InputsObj = MakeShared<FJsonObject>();
        
        // Get inputs by iterating through GetInput
        int32 InputIndex = 0;
        FExpressionInput* Input = Expr->GetInput(InputIndex);
        while (Input)
        {
            FString InputName = Expr->GetInputName(InputIndex).ToString();
            TSharedPtr<FJsonObject> InputConnObj = MakeShared<FJsonObject>();
            
            if (Input->Expression)
            {
                FString* ConnectedNodeId = ExprToNodeId.Find(Input->Expression);
                if (ConnectedNodeId)
                {
                    InputConnObj->SetStringField(TEXT("connected_node"), *ConnectedNodeId);
                    InputConnObj->SetNumberField(TEXT("output_index"), Input->OutputIndex);
                    
                    // Output name if available
                    FString OutputName;
                    TArray<FExpressionOutput>& Outputs = Input->Expression->GetOutputs();
                    if (Input->OutputIndex >= 0 && Input->OutputIndex < Outputs.Num())
                    {
                        OutputName = Outputs[Input->OutputIndex].OutputName.ToString();
                        InputConnObj->SetStringField(TEXT("output_name"), OutputName);
                    }
                }
            }
            
            if (InputConnObj->HasField(TEXT("connected_node")))
            {
                InputsObj->SetObjectField(InputName, InputConnObj);
            }
            
            InputIndex++;
            Input = Expr->GetInput(InputIndex);
        }
        
        // Also handle specific expression types that have named inputs
        // (fallback for GetInput not working for some types)
        auto AddInputConnection = [&](const FString& InputName, FExpressionInput* ExprInput) {
            if (ExprInput && ExprInput->Expression)
            {
                TSharedPtr<FJsonObject> InputConnObj = MakeShared<FJsonObject>();
                FString* ConnectedNodeId = ExprToNodeId.Find(ExprInput->Expression);
                if (ConnectedNodeId)
                {
                    InputConnObj->SetStringField(TEXT("connected_node"), *ConnectedNodeId);
                    InputConnObj->SetNumberField(TEXT("output_index"), ExprInput->OutputIndex);
                    
                    FString OutputName;
                    TArray<FExpressionOutput>& Outputs = ExprInput->Expression->GetOutputs();
                    if (ExprInput->OutputIndex >= 0 && ExprInput->OutputIndex < Outputs.Num())
                    {
                        OutputName = Outputs[ExprInput->OutputIndex].OutputName.ToString();
                        InputConnObj->SetStringField(TEXT("output_name"), OutputName);
                    }
                    InputsObj->SetObjectField(InputName, InputConnObj);
                }
            }
        };
        
        // Handle specific types
        if (UMaterialExpressionMultiply* Multiply = Cast<UMaterialExpressionMultiply>(Expr))
        {
            AddInputConnection(TEXT("A"), &Multiply->A);
            AddInputConnection(TEXT("B"), &Multiply->B);
        }
        else if (UMaterialExpressionAdd* Add = Cast<UMaterialExpressionAdd>(Expr))
        {
            AddInputConnection(TEXT("A"), &Add->A);
            AddInputConnection(TEXT("B"), &Add->B);
        }
        else if (UMaterialExpressionSubtract* Sub = Cast<UMaterialExpressionSubtract>(Expr))
        {
            AddInputConnection(TEXT("A"), &Sub->A);
            AddInputConnection(TEXT("B"), &Sub->B);
        }
        else if (UMaterialExpressionDivide* Div = Cast<UMaterialExpressionDivide>(Expr))
        {
            AddInputConnection(TEXT("A"), &Div->A);
            AddInputConnection(TEXT("B"), &Div->B);
        }
        else if (UMaterialExpressionLinearInterpolate* Lerp = Cast<UMaterialExpressionLinearInterpolate>(Expr))
        {
            AddInputConnection(TEXT("A"), &Lerp->A);
            AddInputConnection(TEXT("B"), &Lerp->B);
            AddInputConnection(TEXT("Alpha"), &Lerp->Alpha);
        }
        else if (UMaterialExpressionClamp* Clamp = Cast<UMaterialExpressionClamp>(Expr))
        {
            AddInputConnection(TEXT("Input"), &Clamp->Input);
        }
        else if (UMaterialExpressionPanner* Panner = Cast<UMaterialExpressionPanner>(Expr))
        {
            AddInputConnection(TEXT("Coordinate"), &Panner->Coordinate);
            AddInputConnection(TEXT("Time"), &Panner->Time);
        }
        else if (UMaterialExpressionRotator* Rotator = Cast<UMaterialExpressionRotator>(Expr))
        {
            AddInputConnection(TEXT("Coordinate"), &Rotator->Coordinate);
            AddInputConnection(TEXT("Time"), &Rotator->Time);
        }
        else if (UMaterialExpressionPower* Power = Cast<UMaterialExpressionPower>(Expr))
        {
            AddInputConnection(TEXT("Base"), &Power->Base);
            AddInputConnection(TEXT("Exponent"), &Power->Exponent);
        }
        else if (UMaterialExpressionTextureSample* TexSample = Cast<UMaterialExpressionTextureSample>(Expr))
        {
            AddInputConnection(TEXT("Coordinates"), &TexSample->Coordinates);
        }
        else if (UMaterialExpressionTextureCoordinate* TexCoord = Cast<UMaterialExpressionTextureCoordinate>(Expr))
        {
            // TextureCoordinate has no inputs
        }
        else if (UMaterialExpressionComponentMask* Mask = Cast<UMaterialExpressionComponentMask>(Expr))
        {
            AddInputConnection(TEXT("Input"), &Mask->Input);
        }
        else if (UMaterialExpressionAbs* AbsExpr = Cast<UMaterialExpressionAbs>(Expr))
        {
            AddInputConnection(TEXT("Input"), &AbsExpr->Input);
        }
        else if (UMaterialExpressionOneMinus* OneMinus = Cast<UMaterialExpressionOneMinus>(Expr))
        {
            AddInputConnection(TEXT("Input"), &OneMinus->Input);
        }
        else if (UMaterialExpressionSaturate* Saturate = Cast<UMaterialExpressionSaturate>(Expr))
        {
            AddInputConnection(TEXT("Input"), &Saturate->Input);
        }
        else if (UMaterialExpressionCeil* Ceil = Cast<UMaterialExpressionCeil>(Expr))
        {
            AddInputConnection(TEXT("Input"), &Ceil->Input);
        }
        else if (UMaterialExpressionFloor* Floor = Cast<UMaterialExpressionFloor>(Expr))
        {
            AddInputConnection(TEXT("Input"), &Floor->Input);
        }
        else if (UMaterialExpressionFrac* Frac = Cast<UMaterialExpressionFrac>(Expr))
        {
            AddInputConnection(TEXT("Input"), &Frac->Input);
        }
        else if (UMaterialExpressionSine* Sine = Cast<UMaterialExpressionSine>(Expr))
        {
            AddInputConnection(TEXT("Input"), &Sine->Input);
        }
        else if (UMaterialExpressionCosine* Cosine = Cast<UMaterialExpressionCosine>(Expr))
        {
            AddInputConnection(TEXT("Input"), &Cosine->Input);
        }
        else if (UMaterialExpressionNormalize* Normalize = Cast<UMaterialExpressionNormalize>(Expr))
        {
            AddInputConnection(TEXT("Input"), &Normalize->VectorInput);
        }
        else if (UMaterialExpressionSquareRoot* Sqrt = Cast<UMaterialExpressionSquareRoot>(Expr))
        {
            AddInputConnection(TEXT("Input"), &Sqrt->Input);
        }
        else if (UMaterialExpressionMin* Min = Cast<UMaterialExpressionMin>(Expr))
        {
            AddInputConnection(TEXT("A"), &Min->A);
            AddInputConnection(TEXT("B"), &Min->B);
        }
        else if (UMaterialExpressionMax* Max = Cast<UMaterialExpressionMax>(Expr))
        {
            AddInputConnection(TEXT("A"), &Max->A);
            AddInputConnection(TEXT("B"), &Max->B);
        }
        else if (UMaterialExpressionRound* Round = Cast<UMaterialExpressionRound>(Expr))
        {
            AddInputConnection(TEXT("Input"), &Round->Input);
        }
        else if (UMaterialExpressionSign* Sign = Cast<UMaterialExpressionSign>(Expr))
        {
            AddInputConnection(TEXT("Input"), &Sign->Input);
        }
        else if (UMaterialExpressionFresnel* Fresnel = Cast<UMaterialExpressionFresnel>(Expr))
        {
            AddInputConnection(TEXT("Normal"), &Fresnel->Normal);
        }
        else if (UMaterialExpressionDepthFade* DepthFade = Cast<UMaterialExpressionDepthFade>(Expr))
        {
            AddInputConnection(TEXT("Opacity"), &DepthFade->InOpacity);
            AddInputConnection(TEXT("FadeDistance"), &DepthFade->FadeDistance);
        }
        else if (UMaterialExpressionPixelDepth* PixelDepth = Cast<UMaterialExpressionPixelDepth>(Expr))
        {
            // No inputs
        }
        else if (UMaterialExpressionSceneDepth* SceneDepth = Cast<UMaterialExpressionSceneDepth>(Expr))
        {
            AddInputConnection(TEXT("Input"), &SceneDepth->Input);
        }
        else if (UMaterialExpressionWorldPosition* WorldPos = Cast<UMaterialExpressionWorldPosition>(Expr))
        {
            // No inputs
        }
        else if (UMaterialExpressionObjectPositionWS* ObjPos = Cast<UMaterialExpressionObjectPositionWS>(Expr))
        {
            // No inputs
        }
        else if (UMaterialExpressionCameraPositionWS* CamPos = Cast<UMaterialExpressionCameraPositionWS>(Expr))
        {
            // No inputs
        }
        else if (UMaterialExpressionCameraVectorWS* CamVec = Cast<UMaterialExpressionCameraVectorWS>(Expr))
        {
            // No inputs
        }
        else if (UMaterialExpressionReflectionVectorWS* ReflVec = Cast<UMaterialExpressionReflectionVectorWS>(Expr))
        {
            // No inputs
        }
        else if (UMaterialExpressionLightVector* LightVec = Cast<UMaterialExpressionLightVector>(Expr))
        {
            // No inputs
        }
        else if (UMaterialExpressionDistance* Dist = Cast<UMaterialExpressionDistance>(Expr))
        {
            AddInputConnection(TEXT("A"), &Dist->A);
            AddInputConnection(TEXT("B"), &Dist->B);
        }
        else if (UMaterialExpressionDesaturation* Desat = Cast<UMaterialExpressionDesaturation>(Expr))
        {
            AddInputConnection(TEXT("Input"), &Desat->Input);
            AddInputConnection(TEXT("Fraction"), &Desat->Fraction);
        }
        else if (UMaterialExpressionLandscapeLayerBlend* LayerBlend = Cast<UMaterialExpressionLandscapeLayerBlend>(Expr))
        {
            // Handle LayerBlend layers - use LayerInput member
            for (int32 LayerIdx = 0; LayerIdx < LayerBlend->Layers.Num(); LayerIdx++)
            {
                FString LayerInputName = FString::Printf(TEXT("Layer_%d"), LayerIdx);
                AddInputConnection(LayerInputName, &LayerBlend->Layers[LayerIdx].LayerInput);
            }
        }
        else if (UMaterialExpressionStaticBoolParameter* StaticBool = Cast<UMaterialExpressionStaticBoolParameter>(Expr))
        {
            // Has no inputs typically
        }
        else if (UMaterialExpressionStaticSwitch* StaticSwitch = Cast<UMaterialExpressionStaticSwitch>(Expr))
        {
            AddInputConnection(TEXT("A"), &StaticSwitch->A);
            AddInputConnection(TEXT("B"), &StaticSwitch->B);
        }
        else if (UMaterialExpressionIf* IfExpr = Cast<UMaterialExpressionIf>(Expr))
        {
            AddInputConnection(TEXT("A"), &IfExpr->A);
            AddInputConnection(TEXT("B"), &IfExpr->B);
            AddInputConnection(TEXT("AGreaterThanB"), &IfExpr->AGreaterThanB);
            AddInputConnection(TEXT("AEqualsB"), &IfExpr->AEqualsB);
            AddInputConnection(TEXT("ALessThanB"), &IfExpr->ALessThanB);
        }
        else if (UMaterialExpressionMaterialFunctionCall* FuncCall = Cast<UMaterialExpressionMaterialFunctionCall>(Expr))
        {
            for (int32 FuncInputIdx = 0; FuncInputIdx < FuncCall->FunctionInputs.Num(); FuncInputIdx++)
            {
                FString InputName;
                if (FuncCall->FunctionInputs[FuncInputIdx].ExpressionInput)
                {
                    InputName = FuncCall->FunctionInputs[FuncInputIdx].ExpressionInput->InputName.ToString();
                }
                else
                {
                    InputName = FString::Printf(TEXT("Input_%d"), FuncInputIdx);
                }
                AddInputConnection(InputName, &FuncCall->FunctionInputs[FuncInputIdx].Input);
            }
        }
        
        if (InputsObj->Values.Num() > 0)
        {
            NodeConnObj->SetObjectField(TEXT("inputs"), InputsObj);
        }
        
        NodeConnectionsArray.Add(MakeShared<FJsonValueObject>(NodeConnObj));
    }

    // Get property connections
    TSharedPtr<FJsonObject> PropertyConnectionsObj = MakeShared<FJsonObject>();
    
    auto AddPropertyConnection = [&](const FString& PropertyName, EMaterialProperty MaterialProperty) {
        FExpressionInput* PropertyInput = Material->GetExpressionInputForProperty(MaterialProperty);
        if (PropertyInput && PropertyInput->Expression)
        {
            TSharedPtr<FJsonObject> PropConnObj = MakeShared<FJsonObject>();
            FString* ConnectedNodeId = ExprToNodeId.Find(PropertyInput->Expression);
            if (ConnectedNodeId)
            {
                PropConnObj->SetStringField(TEXT("node_id"), *ConnectedNodeId);
                PropConnObj->SetNumberField(TEXT("output_index"), PropertyInput->OutputIndex);
                
                FString OutputName;
                TArray<FExpressionOutput>& Outputs = PropertyInput->Expression->GetOutputs();
                if (PropertyInput->OutputIndex >= 0 && PropertyInput->OutputIndex < Outputs.Num())
                {
                    OutputName = Outputs[PropertyInput->OutputIndex].OutputName.ToString();
                    PropConnObj->SetStringField(TEXT("output_name"), OutputName);
                }
                PropertyConnectionsObj->SetObjectField(PropertyName, PropConnObj);
            }
        }
    };

    AddPropertyConnection(TEXT("BaseColor"), MP_BaseColor);
    AddPropertyConnection(TEXT("Metallic"), MP_Metallic);
    AddPropertyConnection(TEXT("Specular"), MP_Specular);
    AddPropertyConnection(TEXT("Roughness"), MP_Roughness);
    AddPropertyConnection(TEXT("Normal"), MP_Normal);
    AddPropertyConnection(TEXT("WorldPositionOffset"), MP_WorldPositionOffset);
    AddPropertyConnection(TEXT("EmissiveColor"), MP_EmissiveColor);
    AddPropertyConnection(TEXT("Opacity"), MP_Opacity);
    AddPropertyConnection(TEXT("OpacityMask"), MP_OpacityMask);
    AddPropertyConnection(TEXT("AmbientOcclusion"), MP_AmbientOcclusion);
    AddPropertyConnection(TEXT("Refraction"), MP_Refraction);
    AddPropertyConnection(TEXT("PixelDepthOffset"), MP_PixelDepthOffset);
    AddPropertyConnection(TEXT("SubsurfaceColor"), MP_SubsurfaceColor);
    // MP_TangentColor not available in UE5.7

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetArrayField(TEXT("node_connections"), NodeConnectionsArray);
    ResultObj->SetObjectField(TEXT("property_connections"), PropertyConnectionsObj);
    ResultObj->SetNumberField(TEXT("node_count"), Expressions.Num());
    ResultObj->SetStringField(TEXT("material_name"), Material->GetName());
    ResultObj->SetBoolField(TEXT("success"), true);

    return ResultObj;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPBlueprintCommands::HandleGetMaterialGraph(const TSharedPtr<FJsonObject>& Params)
{
    // Get material name
    FString MaterialName;
    if (!Params->TryGetStringField(TEXT("material_name"), MaterialName))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'material_name' parameter"));
    }

    // Find or load the material
    FString MaterialPath = MaterialName.StartsWith(TEXT("/")) ? MaterialName : FString::Printf(TEXT("/Game/Materials/%s"), *MaterialName);
    UMaterial* Material = Cast<UMaterial>(UEditorAssetLibrary::LoadAsset(MaterialPath));
    if (!Material)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Material not found: %s"), *MaterialPath));
    }

    // Build expression to node_id map
    TMap<UMaterialExpression*, FString> ExprToNodeId;
    const TArray<UMaterialExpression*>& Expressions = Material->GetExpressionCollection().Expressions;
    
    // Build nodes array (from expressions)
    TArray<TSharedPtr<FJsonValue>> NodesArray;
    for (UMaterialExpression* Expr : Expressions)
    {
        if (Expr)
        {
            TSharedPtr<FJsonObject> NodeObj = MakeShared<FJsonObject>();
            FString TypeName = Expr->GetClass()->GetName();
            TypeName.ReplaceInline(TEXT("MaterialExpression"), TEXT(""));
            FString NodeId = FString::Printf(TEXT("Expr_%s_%d"), *TypeName, Expr->GetUniqueID());
            ExprToNodeId.Add(Expr, NodeId);
            
            NodeObj->SetStringField(TEXT("node_id"), NodeId);
            NodeObj->SetStringField(TEXT("type"), TypeName);
            NodeObj->SetNumberField(TEXT("pos_x"), Expr->MaterialExpressionEditorX);
            NodeObj->SetNumberField(TEXT("pos_y"), Expr->MaterialExpressionEditorY);
            NodeObj->SetStringField(TEXT("desc"), Expr->Desc);
            
            NodesArray.Add(MakeShared<FJsonValueObject>(NodeObj));
        }
    }

    // Build connections array
    TArray<TSharedPtr<FJsonValue>> ConnectionsArray;
    
    for (UMaterialExpression* Expr : Expressions)
    {
        if (!Expr) continue;
        
        FString TargetNodeId = ExprToNodeId[Expr];
        
        // Get inputs
        int32 InputIndex = 0;
        FExpressionInput* Input = Expr->GetInput(InputIndex);
        while (Input)
        {
            if (Input->Expression)
            {
                FString* SourceNodeId = ExprToNodeId.Find(Input->Expression);
                if (SourceNodeId)
                {
                    TSharedPtr<FJsonObject> ConnObj = MakeShared<FJsonObject>();
                    ConnObj->SetStringField(TEXT("source_node"), *SourceNodeId);
                    ConnObj->SetStringField(TEXT("target_node"), TargetNodeId);
                    ConnObj->SetNumberField(TEXT("output_index"), Input->OutputIndex);
                    
                    // Get output name
                    TArray<FExpressionOutput>& Outputs = Input->Expression->GetOutputs();
                    if (Input->OutputIndex >= 0 && Input->OutputIndex < Outputs.Num())
                    {
                        ConnObj->SetStringField(TEXT("output_name"), Outputs[Input->OutputIndex].OutputName.ToString());
                    }
                    
                    ConnectionsArray.Add(MakeShared<FJsonValueObject>(ConnObj));
                }
            }
            InputIndex++;
            Input = Expr->GetInput(InputIndex);
        }
    }

    // Get property connections
    TSharedPtr<FJsonObject> PropertyConnectionsObj = MakeShared<FJsonObject>();
    
    auto AddPropertyConnection = [&](const FString& PropertyName, EMaterialProperty MaterialProperty) {
        FExpressionInput* PropertyInput = Material->GetExpressionInputForProperty(MaterialProperty);
        if (PropertyInput && PropertyInput->Expression)
        {
            TSharedPtr<FJsonObject> PropConnObj = MakeShared<FJsonObject>();
            FString* ConnectedNodeId = ExprToNodeId.Find(PropertyInput->Expression);
            if (ConnectedNodeId)
            {
                PropConnObj->SetStringField(TEXT("node_id"), *ConnectedNodeId);
                PropConnObj->SetNumberField(TEXT("output_index"), PropertyInput->OutputIndex);
                PropertyConnectionsObj->SetObjectField(PropertyName, PropConnObj);
            }
        }
    };

    AddPropertyConnection(TEXT("BaseColor"), MP_BaseColor);
    AddPropertyConnection(TEXT("Metallic"), MP_Metallic);
    AddPropertyConnection(TEXT("Specular"), MP_Specular);
    AddPropertyConnection(TEXT("Roughness"), MP_Roughness);
    AddPropertyConnection(TEXT("Normal"), MP_Normal);
    AddPropertyConnection(TEXT("EmissiveColor"), MP_EmissiveColor);
    AddPropertyConnection(TEXT("Opacity"), MP_Opacity);
    AddPropertyConnection(TEXT("OpacityMask"), MP_OpacityMask);
    AddPropertyConnection(TEXT("AmbientOcclusion"), MP_AmbientOcclusion);

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetArrayField(TEXT("nodes"), NodesArray);
    ResultObj->SetArrayField(TEXT("connections"), ConnectionsArray);
    ResultObj->SetObjectField(TEXT("property_connections"), PropertyConnectionsObj);
    ResultObj->SetNumberField(TEXT("node_count"), NodesArray.Num());
    ResultObj->SetNumberField(TEXT("connection_count"), ConnectionsArray.Num());
    ResultObj->SetBoolField(TEXT("success"), true);

    return ResultObj;
}

// ============================================================================
// Batch Material Graph Builder
// ============================================================================
TSharedPtr<FJsonObject> FEpicUnrealMCPBlueprintCommands::HandleBuildMaterialGraph(const TSharedPtr<FJsonObject>& Params)
{
    // Get material name
    FString MaterialName;
    if (!Params->TryGetStringField(TEXT("material_name"), MaterialName))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'material_name' parameter"));
    }

    // Find or load the material
    FString MaterialPath = MaterialName.StartsWith(TEXT("/")) ? MaterialName : FString::Printf(TEXT("/Game/Materials/%s"), *MaterialName);
    UMaterial* Material = Cast<UMaterial>(UEditorAssetLibrary::LoadAsset(MaterialPath));
    if (!Material)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Material not found: %s"), *MaterialPath));
    }

    // Track created nodes for connection mapping
    TMap<FString, UMaterialExpression*> NodeIdToExpression;
    TArray<TSharedPtr<FJsonValue>> CreatedNodesArray;
    int32 NodeCount = 0;
    int32 ConnectionCount = 0;

    // Process nodes array
    const TArray<TSharedPtr<FJsonValue>>* NodesArray = nullptr;
    if (Params->TryGetArrayField(TEXT("nodes"), NodesArray) && NodesArray)
    {
        for (const TSharedPtr<FJsonValue>& NodeValue : *NodesArray)
        {
            if (!NodeValue.IsValid()) continue;
            
            TSharedPtr<FJsonObject> NodeObj = NodeValue->AsObject();
            if (!NodeObj.IsValid()) continue;

            // Get node ID
            FString NodeId;
            if (!NodeObj->TryGetStringField(TEXT("id"), NodeId))
            {
                continue; // Skip nodes without ID
            }

            // Get expression type
            FString ExpressionType;
            if (!NodeObj->TryGetStringField(TEXT("type"), ExpressionType))
            {
                continue;
            }

            // Get position
            int32 PosX = 0, PosY = 0;
            NodeObj->TryGetNumberField(TEXT("pos_x"), PosX);
            NodeObj->TryGetNumberField(TEXT("pos_y"), PosY);

            // Create expression based on type
            UMaterialExpression* NewExpression = nullptr;

            if (ExpressionType == TEXT("Constant"))
            {
                UMaterialExpressionConstant* Const = NewObject<UMaterialExpressionConstant>(Material);
                float Value = 0.0f;
                NodeObj->TryGetNumberField(TEXT("value"), Value);
                Const->R = Value;
                NewExpression = Const;
            }
            else if (ExpressionType == TEXT("Constant2Vector"))
            {
                UMaterialExpressionConstant2Vector* Const2 = NewObject<UMaterialExpressionConstant2Vector>(Material);
                const TArray<TSharedPtr<FJsonValue>>* ValueArray = nullptr;
                if (NodeObj->TryGetArrayField(TEXT("value"), ValueArray) && ValueArray && ValueArray->Num() >= 2)
                {
                    Const2->R = (*ValueArray)[0]->AsNumber();
                    Const2->G = (*ValueArray)[1]->AsNumber();
                }
                NewExpression = Const2;
            }
            else if (ExpressionType == TEXT("Constant3Vector"))
            {
                UMaterialExpressionConstant3Vector* Const3 = NewObject<UMaterialExpressionConstant3Vector>(Material);
                const TArray<TSharedPtr<FJsonValue>>* ValueArray = nullptr;
                if (NodeObj->TryGetArrayField(TEXT("value"), ValueArray) && ValueArray && ValueArray->Num() >= 3)
                {
                    Const3->Constant.R = (*ValueArray)[0]->AsNumber();
                    Const3->Constant.G = (*ValueArray)[1]->AsNumber();
                    Const3->Constant.B = (*ValueArray)[2]->AsNumber();
                }
                NewExpression = Const3;
            }
            else if (ExpressionType == TEXT("Constant4Vector"))
            {
                UMaterialExpressionConstant4Vector* Const4 = NewObject<UMaterialExpressionConstant4Vector>(Material);
                const TArray<TSharedPtr<FJsonValue>>* ValueArray = nullptr;
                if (NodeObj->TryGetArrayField(TEXT("value"), ValueArray) && ValueArray && ValueArray->Num() >= 4)
                {
                    Const4->Constant.R = (*ValueArray)[0]->AsNumber();
                    Const4->Constant.G = (*ValueArray)[1]->AsNumber();
                    Const4->Constant.B = (*ValueArray)[2]->AsNumber();
                    Const4->Constant.A = (*ValueArray)[3]->AsNumber();
                }
                NewExpression = Const4;
            }
            else if (ExpressionType == TEXT("Multiply"))
            {
                NewExpression = NewObject<UMaterialExpressionMultiply>(Material);
            }
            else if (ExpressionType == TEXT("Add"))
            {
                NewExpression = NewObject<UMaterialExpressionAdd>(Material);
            }
            else if (ExpressionType == TEXT("Subtract"))
            {
                NewExpression = NewObject<UMaterialExpressionSubtract>(Material);
            }
            else if (ExpressionType == TEXT("Divide"))
            {
                NewExpression = NewObject<UMaterialExpressionDivide>(Material);
            }
            else if (ExpressionType == TEXT("Lerp"))
            {
                NewExpression = NewObject<UMaterialExpressionLinearInterpolate>(Material);
            }
            else if (ExpressionType == TEXT("Clamp"))
            {
                NewExpression = NewObject<UMaterialExpressionClamp>(Material);
            }
            else if (ExpressionType == TEXT("TextureSample"))
            {
                UMaterialExpressionTextureSample* TexExpr = NewObject<UMaterialExpressionTextureSample>(Material);
                FString TexturePath;
                if (NodeObj->TryGetStringField(TEXT("texture"), TexturePath))
                {
                    UTexture* Texture = Cast<UTexture>(UEditorAssetLibrary::LoadAsset(TexturePath));
                    if (Texture)
                    {
                        TexExpr->Texture = Texture;
                    }
                }
                NewExpression = TexExpr;
            }
            else if (ExpressionType == TEXT("ScalarParameter"))
            {
                UMaterialExpressionScalarParameter* ScalarParam = NewObject<UMaterialExpressionScalarParameter>(Material);
                FString ParamName;
                if (NodeObj->TryGetStringField(TEXT("parameter_name"), ParamName))
                {
                    ScalarParam->ParameterName = FName(*ParamName);
                }
                float DefaultValue = 0.0f;
                if (NodeObj->TryGetNumberField(TEXT("value"), DefaultValue))
                {
                    ScalarParam->DefaultValue = DefaultValue;
                }
                FString Group;
                if (NodeObj->TryGetStringField(TEXT("group"), Group))
                {
                    ScalarParam->Group = FName(*Group);
                }
                NewExpression = ScalarParam;
            }
            else if (ExpressionType == TEXT("VectorParameter"))
            {
                UMaterialExpressionVectorParameter* VectorParam = NewObject<UMaterialExpressionVectorParameter>(Material);
                FString ParamName;
                if (NodeObj->TryGetStringField(TEXT("parameter_name"), ParamName))
                {
                    VectorParam->ParameterName = FName(*ParamName);
                }
                const TArray<TSharedPtr<FJsonValue>>* ValueArray = nullptr;
                if (NodeObj->TryGetArrayField(TEXT("value"), ValueArray) && ValueArray && ValueArray->Num() >= 4)
                {
                    VectorParam->DefaultValue.R = (*ValueArray)[0]->AsNumber();
                    VectorParam->DefaultValue.G = (*ValueArray)[1]->AsNumber();
                    VectorParam->DefaultValue.B = (*ValueArray)[2]->AsNumber();
                    VectorParam->DefaultValue.A = (*ValueArray)[3]->AsNumber();
                }
                FString Group;
                if (NodeObj->TryGetStringField(TEXT("group"), Group))
                {
                    VectorParam->Group = FName(*Group);
                }
                NewExpression = VectorParam;
            }
            else if (ExpressionType == TEXT("TextureCoordinate"))
            {
                NewExpression = NewObject<UMaterialExpressionTextureCoordinate>(Material);
            }
            else if (ExpressionType == TEXT("Time"))
            {
                NewExpression = NewObject<UMaterialExpressionTime>(Material);
            }
            else if (ExpressionType == TEXT("VertexNormal"))
            {
                NewExpression = NewObject<UMaterialExpressionVertexNormalWS>(Material);
            }
            else if (ExpressionType == TEXT("WorldPosition"))
            {
                NewExpression = NewObject<UMaterialExpressionWorldPosition>(Material);
            }
            else if (ExpressionType == TEXT("OneMinus"))
            {
                NewExpression = NewObject<UMaterialExpressionOneMinus>(Material);
            }
            else if (ExpressionType == TEXT("Saturate"))
            {
                NewExpression = NewObject<UMaterialExpressionSaturate>(Material);
            }
            else if (ExpressionType == TEXT("Normalize"))
            {
                NewExpression = NewObject<UMaterialExpressionNormalize>(Material);
            }
            else if (ExpressionType == TEXT("DotProduct"))
            {
                NewExpression = NewObject<UMaterialExpressionDotProduct>(Material);
            }
            else if (ExpressionType == TEXT("CrossProduct"))
            {
                NewExpression = NewObject<UMaterialExpressionCrossProduct>(Material);
            }
            else if (ExpressionType == TEXT("ComponentMask"))
            {
                UMaterialExpressionComponentMask* Mask = NewObject<UMaterialExpressionComponentMask>(Material);
                bool bR = false, bG = false, bB = false, bA = false;
                NodeObj->TryGetBoolField(TEXT("mask_r"), bR);
                NodeObj->TryGetBoolField(TEXT("mask_g"), bG);
                NodeObj->TryGetBoolField(TEXT("mask_b"), bB);
                NodeObj->TryGetBoolField(TEXT("mask_a"), bA);
                Mask->R = bR;
                Mask->G = bG;
                Mask->B = bB;
                Mask->A = bA;
                NewExpression = Mask;
            }
            else if (ExpressionType == TEXT("AppendVector"))
            {
                NewExpression = NewObject<UMaterialExpressionAppendVector>(Material);
            }
            else if (ExpressionType == TEXT("Fresnel"))
            {
                NewExpression = NewObject<UMaterialExpressionFresnel>(Material);
            }
            else if (ExpressionType == TEXT("Power"))
            {
                NewExpression = NewObject<UMaterialExpressionPower>(Material);
            }
            else if (ExpressionType == TEXT("Panner"))
            {
                NewExpression = NewObject<UMaterialExpressionPanner>(Material);
            }
            else if (ExpressionType == TEXT("Rotator"))
            {
                NewExpression = NewObject<UMaterialExpressionRotator>(Material);
            }
            else if (ExpressionType == TEXT("Sine"))
            {
                NewExpression = NewObject<UMaterialExpressionSine>(Material);
            }
            else if (ExpressionType == TEXT("Cosine"))
            {
                NewExpression = NewObject<UMaterialExpressionCosine>(Material);
            }
            else if (ExpressionType == TEXT("Abs"))
            {
                NewExpression = NewObject<UMaterialExpressionAbs>(Material);
            }
            else if (ExpressionType == TEXT("Floor"))
            {
                NewExpression = NewObject<UMaterialExpressionFloor>(Material);
            }
            else if (ExpressionType == TEXT("Ceil"))
            {
                NewExpression = NewObject<UMaterialExpressionCeil>(Material);
            }
            else if (ExpressionType == TEXT("Frac"))
            {
                NewExpression = NewObject<UMaterialExpressionFrac>(Material);
            }
            else if (ExpressionType == TEXT("SquareRoot"))
            {
                NewExpression = NewObject<UMaterialExpressionSquareRoot>(Material);
            }
            else if (ExpressionType == TEXT("Desaturation"))
            {
                NewExpression = NewObject<UMaterialExpressionDesaturation>(Material);
            }
            else if (ExpressionType == TEXT("ReflectionVector"))
            {
                NewExpression = NewObject<UMaterialExpressionReflectionVectorWS>(Material);
            }
            else if (ExpressionType == TEXT("CameraPosition"))
            {
                NewExpression = NewObject<UMaterialExpressionCameraPositionWS>(Material);
            }
            else if (ExpressionType == TEXT("CameraVector"))
            {
                NewExpression = NewObject<UMaterialExpressionCameraVectorWS>(Material);
            }

            if (NewExpression)
            {
                // Set position
                NewExpression->MaterialExpressionEditorX = PosX;
                NewExpression->MaterialExpressionEditorY = PosY;

                // Add to material
                Material->GetExpressionCollection().AddExpression(NewExpression);

                // Store in map
                NodeIdToExpression.Add(NodeId, NewExpression);

                // Track created node
                TSharedPtr<FJsonObject> CreatedNodeObj = MakeShared<FJsonObject>();
                CreatedNodeObj->SetStringField(TEXT("id"), NodeId);
                CreatedNodeObj->SetStringField(TEXT("type"), ExpressionType);
                FString ActualNodeId = FString::Printf(TEXT("Expr_%s_%d"), *ExpressionType, NewExpression->GetUniqueID());
                CreatedNodeObj->SetStringField(TEXT("node_id"), ActualNodeId);
                CreatedNodesArray.Add(MakeShared<FJsonValueObject>(CreatedNodeObj));

                NodeCount++;
            }
        }
    }

    // Process connections array
    const TArray<TSharedPtr<FJsonValue>>* ConnectionsArray = nullptr;
    if (Params->TryGetArrayField(TEXT("connections"), ConnectionsArray) && ConnectionsArray)
    {
        for (const TSharedPtr<FJsonValue>& ConnValue : *ConnectionsArray)
        {
            if (!ConnValue.IsValid()) continue;
            
            TSharedPtr<FJsonObject> ConnObj = ConnValue->AsObject();
            if (!ConnObj.IsValid()) continue;

            FString SourceId, TargetId;
            if (!ConnObj->TryGetStringField(TEXT("source"), SourceId) || 
                !ConnObj->TryGetStringField(TEXT("target"), TargetId))
            {
                continue;
            }

            FString SourceOutput = TEXT("Output");
            ConnObj->TryGetStringField(TEXT("source_output"), SourceOutput);

            FString TargetInput;
            ConnObj->TryGetStringField(TEXT("target_input"), TargetInput);

            // Handle connection to material property
            if (TargetId == TEXT("Material"))
            {
                UMaterialExpression** SourceExpr = NodeIdToExpression.Find(SourceId);
                if (SourceExpr && *SourceExpr)
                {
                    FExpressionInput* PropertyInput = GetMaterialPropertyInput(Material, TargetInput);
                    if (PropertyInput)
                    {
                        PropertyInput->Expression = *SourceExpr;
                        PropertyInput->OutputIndex = 0;
                        ConnectionCount++;
                    }
                }
            }
            else
            {
                // Handle node-to-node connection
                UMaterialExpression** SourceExpr = NodeIdToExpression.Find(SourceId);
                UMaterialExpression** TargetExpr = NodeIdToExpression.Find(TargetId);

                if (SourceExpr && TargetExpr && *SourceExpr && *TargetExpr)
                {
                    FExpressionInput* InputPtr = GetExpressionInputByName(*TargetExpr, TargetInput);
                    if (InputPtr)
                    {
                        InputPtr->Expression = *SourceExpr;
                        InputPtr->OutputIndex = (SourceOutput == TEXT("Output")) ? 0 : 0; // Default to 0
                        ConnectionCount++;
                    }
                }
            }
        }
    }

    // Process material properties if provided
    TSharedPtr<FJsonObject> PropertiesObj = Params->GetObjectField(TEXT("properties"));
    if (PropertiesObj.IsValid())
    {
        // Shading Model
        FString ShadingModel;
        if (PropertiesObj->TryGetStringField(TEXT("shading_model"), ShadingModel))
        {
            if (ShadingModel == TEXT("Unlit"))
                Material->SetShadingModel(EMaterialShadingModel::MSM_Unlit);
            else if (ShadingModel == TEXT("DefaultLit"))
                Material->SetShadingModel(EMaterialShadingModel::MSM_DefaultLit);
            else if (ShadingModel == TEXT("Subsurface"))
                Material->SetShadingModel(EMaterialShadingModel::MSM_Subsurface);
            else if (ShadingModel == TEXT("TwoSidedFoliage"))
                Material->SetShadingModel(EMaterialShadingModel::MSM_TwoSidedFoliage);
        }

        // Blend Mode
        FString BlendMode;
        if (PropertiesObj->TryGetStringField(TEXT("blend_mode"), BlendMode))
        {
            if (BlendMode == TEXT("Opaque"))
                Material->BlendMode = EBlendMode::BLEND_Opaque;
            else if (BlendMode == TEXT("Masked"))
                Material->BlendMode = EBlendMode::BLEND_Masked;
            else if (BlendMode == TEXT("Translucent"))
                Material->BlendMode = EBlendMode::BLEND_Translucent;
            else if (BlendMode == TEXT("Additive"))
                Material->BlendMode = EBlendMode::BLEND_Additive;
        }

        // Two Sided
        bool bTwoSided;
        if (PropertiesObj->TryGetBoolField(TEXT("two_sided"), bTwoSided))
        {
            Material->TwoSided = bTwoSided ? 1 : 0;
        }

        // Material Domain
        FString MaterialDomain;
        if (PropertiesObj->TryGetStringField(TEXT("material_domain"), MaterialDomain))
        {
            if (MaterialDomain == TEXT("Surface"))
                Material->MaterialDomain = (EMaterialDomain)0;
            else if (MaterialDomain == TEXT("DeferredDecal"))
                Material->MaterialDomain = (EMaterialDomain)1;
            else if (MaterialDomain == TEXT("LightFunction"))
                Material->MaterialDomain = (EMaterialDomain)2;
            else if (MaterialDomain == TEXT("PostProcess"))
                Material->MaterialDomain = (EMaterialDomain)4;
        }
    }

    // Mark material as dirty
    Material->MarkPackageDirty();

    // Compile if requested
    bool bShouldCompile = true;
    Params->TryGetBoolField(TEXT("compile"), bShouldCompile);
    if (bShouldCompile)
    {
        Material->ForceRecompileForRendering();
    }

    // Build result
    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetStringField(TEXT("material_name"), MaterialName);
    ResultObj->SetNumberField(TEXT("node_count"), NodeCount);
    ResultObj->SetNumberField(TEXT("connection_count"), ConnectionCount);
    ResultObj->SetArrayField(TEXT("nodes"), CreatedNodesArray);
    ResultObj->SetBoolField(TEXT("compiled"), bShouldCompile);
    ResultObj->SetBoolField(TEXT("success"), true);

    return ResultObj;
}