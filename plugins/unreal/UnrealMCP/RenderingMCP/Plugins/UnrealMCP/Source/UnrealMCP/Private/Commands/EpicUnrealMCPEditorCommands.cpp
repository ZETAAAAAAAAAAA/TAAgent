#include "Commands/EpicUnrealMCPEditorCommands.h"
#include "Commands/EpicUnrealMCPCommonUtils.h"
#include "Editor.h"
#include "EditorViewportClient.h"
#include "LevelEditorViewport.h"
#include "ImageUtils.h"
#include "HighResScreenshot.h"
#include "Engine/GameViewportClient.h"
#include "Misc/FileHelper.h"
#include "GameFramework/Actor.h"
#include "Engine/Selection.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/PointLight.h"
#include "Engine/SpotLight.h"
#include "Camera/CameraActor.h"
#include "Components/StaticMeshComponent.h"
#include "EditorSubsystem.h"
#include "Subsystems/EditorActorSubsystem.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "EditorAssetLibrary.h"
#include "Commands/EpicUnrealMCPBlueprintCommands.h"
#include "AssetToolsModule.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Factories/FbxFactory.h"
#include "Factories/FbxImportUI.h"
#include "Factories/FbxStaticMeshImportData.h"
#include "AssetImportTask.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialInterface.h"
#include "Engine/Texture.h"
#include "Engine/Texture2D.h"
#include "Factories/TextureFactory.h"
#include "Engine/StaticMesh.h"
#include "StaticMeshAttributes.h"
#include "MeshDescription.h"
#include "MeshDescriptionBuilder.h"

FEpicUnrealMCPEditorCommands::FEpicUnrealMCPEditorCommands()
{
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEditorCommands::HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params)
{
    // Actor manipulation commands
    if (CommandType == TEXT("get_actors_in_level"))
    {
        return HandleGetActorsInLevel(Params);
    }
    else if (CommandType == TEXT("find_actors_by_name"))
    {
        return HandleFindActorsByName(Params);
    }
    else if (CommandType == TEXT("spawn_actor"))
    {
        return HandleSpawnActor(Params);
    }
    else if (CommandType == TEXT("delete_actor"))
    {
        return HandleDeleteActor(Params);
    }
    else if (CommandType == TEXT("set_actor_transform"))
    {
        return HandleSetActorTransform(Params);
    }
    // Blueprint actor spawning
    else if (CommandType == TEXT("spawn_blueprint_actor"))
    {
        return HandleSpawnBlueprintActor(Params);
    }
    // FBX import
    else if (CommandType == TEXT("import_fbx"))
    {
        return HandleImportFBX(Params);
    }
    // Texture import
    else if (CommandType == TEXT("import_texture"))
    {
        return HandleImportTexture(Params);
    }
    // Texture properties
    else if (CommandType == TEXT("set_texture_properties"))
    {
        return HandleSetTextureProperties(Params);
    }
    // Material instance
    else if (CommandType == TEXT("create_material_instance"))
    {
        return HandleCreateMaterialInstance(Params);
    }
    else if (CommandType == TEXT("set_material_instance_parameter"))
    {
        return HandleSetMaterialInstanceParameter(Params);
    }
    // Static mesh from data
    else if (CommandType == TEXT("create_static_mesh_from_data"))
    {
        return HandleCreateStaticMeshFromData(Params);
    }
    
    return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Unknown editor command: %s"), *CommandType));
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEditorCommands::HandleGetActorsInLevel(const TSharedPtr<FJsonObject>& Params)
{
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GWorld, AActor::StaticClass(), AllActors);
    
    TArray<TSharedPtr<FJsonValue>> ActorArray;
    for (AActor* Actor : AllActors)
    {
        if (Actor)
        {
            ActorArray.Add(FEpicUnrealMCPCommonUtils::ActorToJson(Actor));
        }
    }
    
    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetArrayField(TEXT("actors"), ActorArray);
    
    return ResultObj;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEditorCommands::HandleFindActorsByName(const TSharedPtr<FJsonObject>& Params)
{
    FString Pattern;
    if (!Params->TryGetStringField(TEXT("pattern"), Pattern))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'pattern' parameter"));
    }
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GWorld, AActor::StaticClass(), AllActors);
    
    TArray<TSharedPtr<FJsonValue>> MatchingActors;
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetName().Contains(Pattern))
        {
            MatchingActors.Add(FEpicUnrealMCPCommonUtils::ActorToJson(Actor));
        }
    }
    
    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetArrayField(TEXT("actors"), MatchingActors);
    
    return ResultObj;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEditorCommands::HandleSpawnActor(const TSharedPtr<FJsonObject>& Params)
{
    // Get required parameters
    FString ActorType;
    if (!Params->TryGetStringField(TEXT("type"), ActorType))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'type' parameter"));
    }

    // Get actor name (required parameter)
    FString ActorName;
    if (!Params->TryGetStringField(TEXT("name"), ActorName))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'name' parameter"));
    }

    // Get optional transform parameters
    FVector Location(0.0f, 0.0f, 0.0f);
    FRotator Rotation(0.0f, 0.0f, 0.0f);
    FVector Scale(1.0f, 1.0f, 1.0f);

    if (Params->HasField(TEXT("location")))
    {
        Location = FEpicUnrealMCPCommonUtils::GetVectorFromJson(Params, TEXT("location"));
    }
    if (Params->HasField(TEXT("rotation")))
    {
        Rotation = FEpicUnrealMCPCommonUtils::GetRotatorFromJson(Params, TEXT("rotation"));
    }
    if (Params->HasField(TEXT("scale")))
    {
        Scale = FEpicUnrealMCPCommonUtils::GetVectorFromJson(Params, TEXT("scale"));
    }

    // Create the actor based on type
    AActor* NewActor = nullptr;
    UWorld* World = GEditor->GetEditorWorldContext().World();

    if (!World)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to get editor world"));
    }

    // Check if an actor with this name already exists
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetName() == ActorName)
        {
            return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Actor with name '%s' already exists"), *ActorName));
        }
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Name = *ActorName;

    if (ActorType == TEXT("StaticMeshActor"))
    {
        AStaticMeshActor* NewMeshActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, Rotation, SpawnParams);
        if (NewMeshActor)
        {
            // Check for an optional static_mesh parameter to assign a mesh
            FString MeshPath;
            if (Params->TryGetStringField(TEXT("static_mesh"), MeshPath))
            {
                UStaticMesh* Mesh = Cast<UStaticMesh>(UEditorAssetLibrary::LoadAsset(MeshPath));
                if (Mesh)
                {
                    NewMeshActor->GetStaticMeshComponent()->SetStaticMesh(Mesh);
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("Could not find static mesh at path: %s"), *MeshPath);
                }
            }
        }
        NewActor = NewMeshActor;
    }
    else if (ActorType == TEXT("PointLight"))
    {
        NewActor = World->SpawnActor<APointLight>(APointLight::StaticClass(), Location, Rotation, SpawnParams);
    }
    else if (ActorType == TEXT("SpotLight"))
    {
        NewActor = World->SpawnActor<ASpotLight>(ASpotLight::StaticClass(), Location, Rotation, SpawnParams);
    }
    else if (ActorType == TEXT("DirectionalLight"))
    {
        NewActor = World->SpawnActor<ADirectionalLight>(ADirectionalLight::StaticClass(), Location, Rotation, SpawnParams);
    }
    else if (ActorType == TEXT("CameraActor"))
    {
        NewActor = World->SpawnActor<ACameraActor>(ACameraActor::StaticClass(), Location, Rotation, SpawnParams);
    }
    else
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Unknown actor type: %s"), *ActorType));
    }

    if (NewActor)
    {
        // Set scale (since SpawnActor only takes location and rotation)
        FTransform Transform = NewActor->GetTransform();
        Transform.SetScale3D(Scale);
        NewActor->SetActorTransform(Transform);

        // Return the created actor's details
        return FEpicUnrealMCPCommonUtils::ActorToJsonObject(NewActor, true);
    }

    return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create actor"));
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEditorCommands::HandleDeleteActor(const TSharedPtr<FJsonObject>& Params)
{
    FString ActorName;
    if (!Params->TryGetStringField(TEXT("name"), ActorName))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'name' parameter"));
    }

    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GWorld, AActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetName() == ActorName)
        {
            // Store actor info before deletion for the response
            TSharedPtr<FJsonObject> ActorInfo = FEpicUnrealMCPCommonUtils::ActorToJsonObject(Actor);
            
            // Delete the actor
            Actor->Destroy();
            
            TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
            ResultObj->SetObjectField(TEXT("deleted_actor"), ActorInfo);
            return ResultObj;
        }
    }
    
    return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Actor not found: %s"), *ActorName));
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEditorCommands::HandleSetActorTransform(const TSharedPtr<FJsonObject>& Params)
{
    // Get actor name
    FString ActorName;
    if (!Params->TryGetStringField(TEXT("name"), ActorName))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'name' parameter"));
    }

    // Find the actor
    AActor* TargetActor = nullptr;
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GWorld, AActor::StaticClass(), AllActors);
    
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

    // Get transform parameters
    FTransform NewTransform = TargetActor->GetTransform();

    if (Params->HasField(TEXT("location")))
    {
        NewTransform.SetLocation(FEpicUnrealMCPCommonUtils::GetVectorFromJson(Params, TEXT("location")));
    }
    if (Params->HasField(TEXT("rotation")))
    {
        NewTransform.SetRotation(FQuat(FEpicUnrealMCPCommonUtils::GetRotatorFromJson(Params, TEXT("rotation"))));
    }
    if (Params->HasField(TEXT("scale")))
    {
        NewTransform.SetScale3D(FEpicUnrealMCPCommonUtils::GetVectorFromJson(Params, TEXT("scale")));
    }

    // Set the new transform
    TargetActor->SetActorTransform(NewTransform);

    // Return updated actor info
    return FEpicUnrealMCPCommonUtils::ActorToJsonObject(TargetActor, true);
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEditorCommands::HandleSpawnBlueprintActor(const TSharedPtr<FJsonObject>& Params)
{
    // This function will now correctly call the implementation in BlueprintCommands
    FEpicUnrealMCPBlueprintCommands BlueprintCommands;
    return BlueprintCommands.HandleCommand(TEXT("spawn_blueprint_actor"), Params);
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEditorCommands::HandleImportFBX(const TSharedPtr<FJsonObject>& Params)
{
    // Get FBX file path
    FString FBXPath;
    if (!Params->TryGetStringField(TEXT("fbx_path"), FBXPath))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'fbx_path' parameter"));
    }

    // Check if file exists
    IFileManager& FileManager = IFileManager::Get();
    if (!FileManager.FileExists(*FBXPath))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("FBX file not found: %s"), *FBXPath));
    }

    // Get optional destination path (default to /Game/ImportedMeshes/)
    FString DestinationPath = TEXT("/Game/ImportedMeshes/");
    if (Params->HasField(TEXT("destination_path")))
    {
        Params->TryGetStringField(TEXT("destination_path"), DestinationPath);
    }

    // Get optional asset name (default to filename without extension)
    FString AssetName;
    if (Params->HasField(TEXT("asset_name")))
    {
        Params->TryGetStringField(TEXT("asset_name"), AssetName);
    }
    else
    {
        AssetName = FPaths::GetBaseFilename(FBXPath);
    }

    // Get optional spawn in level flag
    bool bSpawnInLevel = true;
    Params->TryGetBoolField(TEXT("spawn_in_level"), bSpawnInLevel);

    // Get optional location for spawned actor
    FVector SpawnLocation(0.0f, 0.0f, 0.0f);
    if (Params->HasField(TEXT("location")))
    {
        SpawnLocation = FEpicUnrealMCPCommonUtils::GetVectorFromJson(Params, TEXT("location"));
    }

    // Ensure destination path exists
    UEditorAssetLibrary::MakeDirectory(DestinationPath);

    // Get AssetTools module
    FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

    // Create FBX factory
    UFbxFactory* FbxFactory = NewObject<UFbxFactory>();
    FbxFactory->AddToRoot();

    // Create import UI with proper settings
    UFbxImportUI* ImportUI = NewObject<UFbxImportUI>(FbxFactory);
    ImportUI->bImportMesh = true;
    ImportUI->bImportAnimations = false;
    ImportUI->bImportMaterials = false;
    ImportUI->bImportTextures = false;
    ImportUI->bImportAsSkeletal = false;
    ImportUI->MeshTypeToImport = FBXIT_StaticMesh;
    ImportUI->bIsReimport = false;
    ImportUI->ReimportMesh = nullptr;
    ImportUI->bAllowContentTypeImport = true;
    ImportUI->bAutomatedImportShouldDetectType = false;
    ImportUI->bIsObjImport = false;
    
    // Create static mesh import data
    ImportUI->StaticMeshImportData = NewObject<UFbxStaticMeshImportData>(FbxFactory);
    ImportUI->StaticMeshImportData->bCombineMeshes = true;
    
    FbxFactory->ImportUI = ImportUI;
    FbxFactory->SetDetectImportTypeOnImport(false);

    // Create import task
    UAssetImportTask* Task = NewObject<UAssetImportTask>();
    Task->AddToRoot();
    Task->bAutomated = true;
    Task->bReplaceExisting = true;
    Task->bSave = true;
    Task->DestinationPath = DestinationPath;
    Task->DestinationName = AssetName;
    Task->Filename = FBXPath;
    Task->Factory = FbxFactory;
    Task->Options = ImportUI;
    
    FbxFactory->SetAssetImportTask(Task);
    
    // Execute import
    TArray<UAssetImportTask*> Tasks;
    Tasks.Add(Task);
    AssetToolsModule.Get().ImportAssetTasks(Tasks);

    // Get imported objects
    UObject* ImportedAsset = nullptr;
    for (const FString& AssetPath : Task->ImportedObjectPaths)
    {
        FAssetData AssetData = AssetRegistryModule.Get().GetAssetByObjectPath(FSoftObjectPath(AssetPath));
        ImportedAsset = AssetData.GetAsset();
        if (ImportedAsset)
        {
            break;
        }
    }

    // Cleanup
    Task->RemoveFromRoot();
    FbxFactory->RemoveFromRoot();

    if (!ImportedAsset)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Failed to import FBX: %s"), *FBXPath));
    }

    // Get the static mesh
    UStaticMesh* ImportedMesh = Cast<UStaticMesh>(ImportedAsset);

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    
    TArray<TSharedPtr<FJsonValue>> ImportedAssetPaths;
    ImportedAssetPaths.Add(MakeShared<FJsonValueString>(ImportedAsset->GetPathName()));
    ResultObj->SetArrayField(TEXT("imported_assets"), ImportedAssetPaths);

    // Optionally spawn actor in level
    if (bSpawnInLevel && ImportedMesh)
    {
        UWorld* World = GEditor->GetEditorWorldContext().World();
        if (World)
        {
            FActorSpawnParameters SpawnParams;
            SpawnParams.Name = *AssetName;
            
            AStaticMeshActor* NewActor = World->SpawnActor<AStaticMeshActor>(
                AStaticMeshActor::StaticClass(),
                SpawnLocation,
                FRotator::ZeroRotator,
                SpawnParams
            );
            
            if (NewActor)
            {
                NewActor->GetStaticMeshComponent()->SetStaticMesh(ImportedMesh);
                NewActor->SetActorLabel(AssetName);
                ResultObj->SetStringField(TEXT("spawned_actor"), NewActor->GetName());
                ResultObj->SetObjectField(TEXT("actor_info"), FEpicUnrealMCPCommonUtils::ActorToJsonObject(NewActor, true));
            }
        }
    }

    ResultObj->SetBoolField(TEXT("success"), true);
    ResultObj->SetStringField(TEXT("mesh_path"), ImportedMesh ? ImportedMesh->GetPathName() : TEXT(""));
    
    return ResultObj;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEditorCommands::HandleImportTexture(const TSharedPtr<FJsonObject>& Params)
{
    // Get source path
    FString SourcePath;
    if (!Params->TryGetStringField(TEXT("source_path"), SourcePath))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'source_path' parameter"));
    }

    // Check if file exists
    IFileManager& FileManager = IFileManager::Get();
    if (!FileManager.FileExists(*SourcePath))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Texture file not found: %s"), *SourcePath));
    }

    // Get optional name
    FString TextureName;
    if (Params->HasField(TEXT("name")))
    {
        Params->TryGetStringField(TEXT("name"), TextureName);
    }
    else
    {
        TextureName = FPaths::GetBaseFilename(SourcePath);
    }

    // Get optional destination path
    FString DestinationPath = TEXT("/Game/Textures/");
    if (Params->HasField(TEXT("destination_path")))
    {
        Params->TryGetStringField(TEXT("destination_path"), DestinationPath);
    }

    // Ensure destination path exists
    UEditorAssetLibrary::MakeDirectory(DestinationPath);

    // Get AssetTools module
    FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

    // Create texture factory
    UTextureFactory* TextureFactory = NewObject<UTextureFactory>();
    TextureFactory->AddToRoot();
    TextureFactory->bCreateMaterial = false;

    // Create import task
    UAssetImportTask* Task = NewObject<UAssetImportTask>();
    Task->AddToRoot();
    Task->bAutomated = true;
    Task->bReplaceExisting = true;
    Task->bSave = true;
    Task->DestinationPath = DestinationPath;
    Task->DestinationName = TextureName;
    Task->Filename = SourcePath;
    Task->Factory = TextureFactory;
    
    TextureFactory->SetAssetImportTask(Task);
    
    // Execute import
    TArray<UAssetImportTask*> Tasks;
    Tasks.Add(Task);
    AssetToolsModule.Get().ImportAssetTasks(Tasks);

    // Get imported objects
    UObject* ImportedTexture = nullptr;
    for (const FString& AssetPath : Task->ImportedObjectPaths)
    {
        FAssetData AssetData = AssetRegistryModule.Get().GetAssetByObjectPath(FSoftObjectPath(AssetPath));
        ImportedTexture = AssetData.GetAsset();
        if (ImportedTexture)
        {
            break;
        }
    }

    // Cleanup
    Task->RemoveFromRoot();
    TextureFactory->RemoveFromRoot();

    if (!ImportedTexture)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Failed to import texture: %s"), *SourcePath));
    }

    // Check for delete source option
    bool bDeleteSource = false;
    Params->TryGetBoolField(TEXT("delete_source"), bDeleteSource);
    if (bDeleteSource)
    {
        IFileManager::Get().Delete(*SourcePath);
    }

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetBoolField(TEXT("success"), true);
    ResultObj->SetStringField(TEXT("texture_path"), ImportedTexture ? ImportedTexture->GetPathName() : TEXT(""));

    return ResultObj;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEditorCommands::HandleSetTextureProperties(const TSharedPtr<FJsonObject>& Params)
{
    // Get texture path
    FString TexturePath;
    if (!Params->TryGetStringField(TEXT("texture_path"), TexturePath))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'texture_path' parameter"));
    }

    // Load texture
    UTexture* Texture = LoadObject<UTexture>(nullptr, *TexturePath);
    if (!Texture)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Failed to load texture: %s"), *TexturePath));
    }

    bool bModified = false;

    // Set sRGB property
    if (Params->HasField(TEXT("srgb")))
    {
        bool bSRGB = false;
        Params->TryGetBoolField(TEXT("srgb"), bSRGB);
        Texture->SRGB = bSRGB;
        bModified = true;
    }

    // Set compression settings
    if (Params->HasField(TEXT("compression_settings")))
    {
        FString CompressionSettingsStr;
        Params->TryGetStringField(TEXT("compression_settings"), CompressionSettingsStr);
        
        // Map string to enum (UE5.7 names)
        TextureCompressionSettings CompressionSettings = TC_Default;
        if (CompressionSettingsStr == TEXT("Default")) CompressionSettings = TC_Default;
        else if (CompressionSettingsStr == TEXT("NormalMap")) CompressionSettings = TC_Normalmap;
        else if (CompressionSettingsStr == TEXT("Masks")) CompressionSettings = TC_Masks;
        else if (CompressionSettingsStr == TEXT("Grayscale")) CompressionSettings = TC_Grayscale;
        else if (CompressionSettingsStr == TEXT("Displacementmap")) CompressionSettings = TC_Displacementmap;
        else if (CompressionSettingsStr == TEXT("VectorDisplacementmap")) CompressionSettings = TC_VectorDisplacementmap;
        else if (CompressionSettingsStr == TEXT("HDR")) CompressionSettings = TC_HDR;
        else if (CompressionSettingsStr == TEXT("UserInterface")) CompressionSettings = TC_EditorIcon;
        else if (CompressionSettingsStr == TEXT("Alpha")) CompressionSettings = TC_Alpha;
        else if (CompressionSettingsStr == TEXT("DistanceFieldFont")) CompressionSettings = TC_DistanceFieldFont;
        else if (CompressionSettingsStr == TEXT("HDRCompressed")) CompressionSettings = TC_HDR_Compressed;
        else if (CompressionSettingsStr == TEXT("BC7")) CompressionSettings = TC_BC7;
        else if (CompressionSettingsStr == TEXT("HalfFloat")) CompressionSettings = TC_HalfFloat;
        else if (CompressionSettingsStr == TEXT("SingleFloat")) CompressionSettings = TC_SingleFloat;
        else if (CompressionSettingsStr == TEXT("HDR_F32")) CompressionSettings = TC_HDR_F32;
        
        Texture->CompressionSettings = CompressionSettings;
        bModified = true;
    }

    // Set filter mode
    if (Params->HasField(TEXT("filter")))
    {
        FString FilterStr;
        Params->TryGetStringField(TEXT("filter"), FilterStr);
        
        TextureFilter Filter = TF_Default;
        if (FilterStr == TEXT("Default")) Filter = TF_Default;
        else if (FilterStr == TEXT("Nearest")) Filter = TF_Nearest;
        else if (FilterStr == TEXT("Bilinear")) Filter = TF_Bilinear;
        else if (FilterStr == TEXT("Trilinear")) Filter = TF_Trilinear;
        
        Texture->Filter = Filter;
        bModified = true;
    }

    // Set address mode (wrap settings) - only for Texture2D
    UTexture2D* Texture2D = Cast<UTexture2D>(Texture);
    if (Texture2D)
    {
        if (Params->HasField(TEXT("address_x")))
        {
            FString AddressStr;
            Params->TryGetStringField(TEXT("address_x"), AddressStr);
            
            TextureAddress Address = TA_Wrap;
            if (AddressStr == TEXT("Wrap")) Address = TA_Wrap;
            else if (AddressStr == TEXT("Clamp")) Address = TA_Clamp;
            else if (AddressStr == TEXT("Mirror")) Address = TA_Mirror;
            
            Texture2D->AddressX = Address;
            bModified = true;
        }
        if (Params->HasField(TEXT("address_y")))
        {
            FString AddressStr;
            Params->TryGetStringField(TEXT("address_y"), AddressStr);
            
            TextureAddress Address = TA_Wrap;
            if (AddressStr == TEXT("Wrap")) Address = TA_Wrap;
            else if (AddressStr == TEXT("Clamp")) Address = TA_Clamp;
            else if (AddressStr == TEXT("Mirror")) Address = TA_Mirror;
            
            Texture2D->AddressY = Address;
            bModified = true;
        }
    }

    // Save and reimport if modified
    if (bModified)
    {
        Texture->UpdateResource();
        Texture->MarkPackageDirty();
        UEditorAssetLibrary::SaveLoadedAsset(Texture);
    }

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetBoolField(TEXT("success"), true);
    ResultObj->SetStringField(TEXT("texture_path"), Texture->GetPathName());
    ResultObj->SetBoolField(TEXT("modified"), bModified);

    return ResultObj;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEditorCommands::HandleCreateMaterialInstance(const TSharedPtr<FJsonObject>& Params)
{
    // Get parent material path
    FString ParentMaterialPath;
    if (!Params->TryGetStringField(TEXT("parent_material"), ParentMaterialPath))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'parent_material' parameter"));
    }

    // Get optional name
    FString InstanceName;
    if (Params->HasField(TEXT("name")))
    {
        Params->TryGetStringField(TEXT("name"), InstanceName);
    }
    else
    {
        // Generate name from parent material
        FString ParentName = FPaths::GetBaseFilename(ParentMaterialPath);
        InstanceName = TEXT("MI_") + ParentName;
    }

    // Get optional destination path
    FString DestinationPath = TEXT("/Game/Materials/Instances/");
    if (Params->HasField(TEXT("destination_path")))
    {
        Params->TryGetStringField(TEXT("destination_path"), DestinationPath);
    }

    // Ensure destination path exists
    UEditorAssetLibrary::MakeDirectory(DestinationPath);

    // Load parent material
    UMaterialInterface* ParentMaterial = LoadObject<UMaterialInterface>(nullptr, *ParentMaterialPath);
    if (!ParentMaterial)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Failed to load parent material: %s"), *ParentMaterialPath));
    }

    // Create package
    FString PackageName = DestinationPath + InstanceName;
    UPackage* Package = CreatePackage(*PackageName);
    if (!Package)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create package"));
    }

    // Create material instance constant
    UMaterialInstanceConstant* MaterialInstance = NewObject<UMaterialInstanceConstant>(Package, *InstanceName, RF_Public | RF_Standalone);
    if (!MaterialInstance)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create material instance"));
    }

    // Set parent
    MaterialInstance->SetParentEditorOnly(ParentMaterial);
    MaterialInstance->PostLoad();

    // Notify asset registry
    FAssetRegistryModule::AssetCreated(MaterialInstance);
    Package->MarkPackageDirty();

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetBoolField(TEXT("success"), true);
    ResultObj->SetStringField(TEXT("material_instance_path"), MaterialInstance->GetPathName());

    return ResultObj;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEditorCommands::HandleSetMaterialInstanceParameter(const TSharedPtr<FJsonObject>& Params)
{
    // Get material instance path
    FString MaterialInstancePath;
    if (!Params->TryGetStringField(TEXT("material_instance"), MaterialInstancePath))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'material_instance' parameter"));
    }

    // Get parameter name
    FString ParameterName;
    if (!Params->TryGetStringField(TEXT("parameter_name"), ParameterName))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'parameter_name' parameter"));
    }

    // Get parameter type
    FString ParameterType;
    if (!Params->TryGetStringField(TEXT("parameter_type"), ParameterType))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'parameter_type' parameter"));
    }

    // Load material instance
    UMaterialInstanceConstant* MaterialInstance = LoadObject<UMaterialInstanceConstant>(nullptr, *MaterialInstancePath);
    if (!MaterialInstance)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Failed to load material instance: %s"), *MaterialInstancePath));
    }

    FName ParamName(*ParameterName);
    bool bSuccess = false;

    // Set parameter based on type
    if (ParameterType == TEXT("scalar") || ParameterType == TEXT("float"))
    {
        double Value = 0.0;
        if (Params->TryGetNumberField(TEXT("value"), Value))
        {
            MaterialInstance->SetScalarParameterValueEditorOnly(ParamName, static_cast<float>(Value));
            bSuccess = true;
        }
    }
    else if (ParameterType == TEXT("vector") || ParameterType == TEXT("color"))
    {
        const TSharedPtr<FJsonObject>* ValueObj;
        if (Params->TryGetObjectField(TEXT("value"), ValueObj))
        {
            FLinearColor Color(1.0f, 1.0f, 1.0f, 1.0f);  // Default white
            if ((*ValueObj)->HasField(TEXT("r")))
            {
                double R, G, B, A = 1.0;
                (*ValueObj)->TryGetNumberField(TEXT("r"), R);
                (*ValueObj)->TryGetNumberField(TEXT("g"), G);
                (*ValueObj)->TryGetNumberField(TEXT("b"), B);
                if ((*ValueObj)->HasField(TEXT("a")))
                {
                    (*ValueObj)->TryGetNumberField(TEXT("a"), A);
                }
                Color = FLinearColor(R, G, B, A);
            }
            else
            {
                // Try array format [r, g, b, a]
                const TArray<TSharedPtr<FJsonValue>>* ValueArray;
                if ((*ValueObj)->TryGetArrayField(TEXT("value"), ValueArray) && ValueArray->Num() >= 3)
                {
                    double R = (*ValueArray)[0]->AsNumber();
                    double G = (*ValueArray)[1]->AsNumber();
                    double B = (*ValueArray)[2]->AsNumber();
                    double A = ValueArray->Num() > 3 ? (*ValueArray)[3]->AsNumber() : 1.0;
                    Color = FLinearColor(R, G, B, A);
                }
            }
            MaterialInstance->SetVectorParameterValueEditorOnly(ParamName, Color);
            bSuccess = true;
        }
    }
    else if (ParameterType == TEXT("texture"))
    {
        FString TexturePath;
        if (Params->TryGetStringField(TEXT("value"), TexturePath))
        {
            UTexture* Texture = LoadObject<UTexture>(nullptr, *TexturePath);
            if (Texture)
            {
                MaterialInstance->SetTextureParameterValueEditorOnly(ParamName, Texture);
                bSuccess = true;
            }
            else
            {
                return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Failed to load texture: %s"), *TexturePath));
            }
        }
        else
        {
            return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'value' parameter for texture type"));
        }
    }
    else
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Unknown parameter type: %s"), *ParameterType));
    }

    if (!bSuccess)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to set parameter value"));
    }

    MaterialInstance->PostEditChange();
    MaterialInstance->MarkPackageDirty();

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetBoolField(TEXT("success"), true);
    ResultObj->SetStringField(TEXT("material_instance_path"), MaterialInstance->GetPathName());

    return ResultObj;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEditorCommands::HandleCreateStaticMeshFromData(const TSharedPtr<FJsonObject>& Params)
{
    // Get mesh name
    FString MeshName = TEXT("SM_ImportedMesh");
    if (Params->HasField(TEXT("name")))
    {
        Params->TryGetStringField(TEXT("name"), MeshName);
    }

    // Get destination path
    FString DestinationPath = TEXT("/Game/Meshes/");
    if (Params->HasField(TEXT("destination_path")))
    {
        Params->TryGetStringField(TEXT("destination_path"), DestinationPath);
    }

    // Get vertex data arrays
    const TArray<TSharedPtr<FJsonValue>>* PositionsArray;
    if (!Params->TryGetArrayField(TEXT("positions"), PositionsArray))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'positions' parameter"));
    }

    const TArray<TSharedPtr<FJsonValue>>* IndicesArray;
    if (!Params->TryGetArrayField(TEXT("indices"), IndicesArray))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'indices' parameter"));
    }

    // Optional arrays
    const TArray<TSharedPtr<FJsonValue>>* NormalsArray = nullptr;
    Params->TryGetArrayField(TEXT("normals"), NormalsArray);

    const TArray<TSharedPtr<FJsonValue>>* UVsArray = nullptr;
    Params->TryGetArrayField(TEXT("uvs"), UVsArray);

    const TArray<TSharedPtr<FJsonValue>>* TangentsArray = nullptr;
    Params->TryGetArrayField(TEXT("tangents"), TangentsArray);

    const TArray<TSharedPtr<FJsonValue>>* ColorsArray = nullptr;
    Params->TryGetArrayField(TEXT("colors"), ColorsArray);

    // Ensure destination path exists
    UEditorAssetLibrary::MakeDirectory(DestinationPath);

    // Create package
    FString PackageName = DestinationPath + MeshName;
    UPackage* Package = CreatePackage(*PackageName);
    if (!Package)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create package"));
    }

    // Create static mesh
    UStaticMesh* StaticMesh = NewObject<UStaticMesh>(Package, *MeshName, RF_Public | RF_Standalone);
    if (!StaticMesh)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create static mesh"));
    }

    // Get or create mesh description
    FMeshDescription* MeshDescription = StaticMesh->GetMeshDescription(0);
    if (!MeshDescription)
    {
        MeshDescription = StaticMesh->CreateMeshDescription(0);
    }

    // Initialize vertex attributes
    FStaticMeshAttributes Attributes(*MeshDescription);
    Attributes.Register();

    // Get attribute references
    TVertiveDataAttributesRef<FVector3f> VertexPositions = Attributes.GetVertexPositions();
    TVertiveDataAttributesRef<FVector3f> VertexNormals = Attributes.GetVertexInstanceNormals();
    TVertiveDataAttributesRef<FVector2f> VertexUVs = Attributes.GetVertexInstanceUVs();
    TVertiveDataAttributesRef<FVector4f> VertexTangents = Attributes.GetVertexInstanceTangents();
    TVertiveDataAttributesRef<FVector3f> VertexBinormalSigns = Attributes.GetVertexInstanceBinormalSigns();
    TVertiveDataAttributesRef<FColor> VertexColors = Attributes.GetVertexInstanceColors();

    // Parse positions
    int32 NumVertices = PositionsArray->Num();
    TArray<FVector3f> ParsedPositions;
    ParsedPositions.Reserve(NumVertices);
    for (const TSharedPtr<FJsonValue>& PosValue : *PositionsArray)
    {
        const TArray<TSharedPtr<FJsonValue>>* PosArray;
        if (PosValue->TryGetArray(PosArray) && PosArray->Num() >= 3)
        {
            float X = static_cast<float>((*PosArray)[0]->AsNumber());
            float Y = static_cast<float>((*PosArray)[1]->AsNumber());
            float Z = static_cast<float>((*PosArray)[2]->AsNumber());
            ParsedPositions.Add(FVector3f(X, Y, Z));
        }
    }

    // Parse indices
    TArray<uint32> ParsedIndices;
    ParsedIndices.Reserve(IndicesArray->Num());
    for (const TSharedPtr<FJsonValue>& IndexValue : *IndicesArray)
    {
        ParsedIndices.Add(static_cast<uint32>(IndexValue->AsNumber()));
    }

    // Parse normals
    TArray<FVector3f> ParsedNormals;
    if (NormalsArray)
    {
        ParsedNormals.Reserve(NormalsArray->Num());
        for (const TSharedPtr<FJsonValue>& NormalValue : *NormalsArray)
        {
            const TArray<TSharedPtr<FJsonValue>>* NormalArray;
            if (NormalValue->TryGetArray(NormalArray) && NormalArray->Num() >= 3)
            {
                float X = static_cast<float>((*NormalArray)[0]->AsNumber());
                float Y = static_cast<float>((*NormalArray)[1]->AsNumber());
                float Z = static_cast<float>((*NormalArray)[2]->AsNumber());
                ParsedNormals.Add(FVector3f(X, Y, Z));
            }
            else
            {
                ParsedNormals.Add(FVector3f(0.0f, 0.0f, 1.0f));
            }
        }
    }

    // Parse UVs
    TArray<FVector2f> ParsedUVs;
    if (UVsArray)
    {
        ParsedUVs.Reserve(UVsArray->Num());
        for (const TSharedPtr<FJsonValue>& UVValue : *UVsArray)
        {
            const TArray<TSharedPtr<FJsonValue>>* UVArray;
            if (UVValue->TryGetArray(UVArray) && UVArray->Num() >= 2)
            {
                float U = static_cast<float>((*UVArray)[0]->AsNumber());
                float V = static_cast<float>((*UVArray)[1]->AsNumber());
                ParsedUVs.Add(FVector2f(U, V));
            }
            else
            {
                ParsedUVs.Add(FVector2f(0.0f, 0.0f));
            }
        }
    }

    // Parse tangents
    TArray<FVector4f> ParsedTangents;
    if (TangentsArray)
    {
        ParsedTangents.Reserve(TangentsArray->Num());
        for (const TSharedPtr<FJsonValue>& TangentValue : *TangentsArray)
        {
            const TArray<TSharedPtr<FJsonValue>>* TangentArray;
            if (TangentValue->TryGetArray(TangentArray) && TangentArray->Num() >= 4)
            {
                float X = static_cast<float>((*TangentArray)[0]->AsNumber());
                float Y = static_cast<float>((*TangentArray)[1]->AsNumber());
                float Z = static_cast<float>((*TangentArray)[2]->AsNumber());
                float W = static_cast<float>((*TangentArray)[3]->AsNumber());
                ParsedTangents.Add(FVector4f(X, Y, Z, W));
            }
            else if (TangentValue->TryGetArray(TangentArray) && TangentArray->Num() >= 3)
            {
                float X = static_cast<float>((*TangentArray)[0]->AsNumber());
                float Y = static_cast<float>((*TangentArray)[1]->AsNumber());
                float Z = static_cast<float>((*TangentArray)[2]->AsNumber());
                ParsedTangents.Add(FVector4f(X, Y, Z, 1.0f));
            }
            else
            {
                ParsedTangents.Add(FVector4f(1.0f, 0.0f, 0.0f, 1.0f));
            }
        }
    }

    // Parse colors
    TArray<FColor> ParsedColors;
    if (ColorsArray)
    {
        ParsedColors.Reserve(ColorsArray->Num());
        for (const TSharedPtr<FJsonValue>& ColorValue : *ColorsArray)
        {
            const TArray<TSharedPtr<FJsonValue>>* ColorArray;
            if (ColorValue->TryGetArray(ColorArray) && ColorArray->Num() >= 4)
            {
                float R = static_cast<float>((*ColorArray)[0]->AsNumber());
                float G = static_cast<float>((*ColorArray)[1]->AsNumber());
                float B = static_cast<float>((*ColorArray)[2]->AsNumber());
                float A = static_cast<float>((*ColorArray)[3]->AsNumber());
                ParsedColors.Add(FLinearColor(R, G, B, A).ToFColor(true));
            }
            else
            {
                ParsedColors.Add(FColor::White);
            }
        }
    }

    // Create vertices
    TArray<FVertexID> VertexIDs;
    VertexIDs.Reserve(NumVertices);
    for (int32 i = 0; i < NumVertices; ++i)
    {
        FVertexID VertexID = MeshDescription->CreateVertex();
        VertexIDs.Add(VertexID);
        if (i < ParsedPositions.Num())
        {
            VertexPositions[VertexID] = ParsedPositions[i];
        }
    }

    // Create polygons (triangles)
    int32 NumTriangles = ParsedIndices.Num() / 3;
    for (int32 TriIdx = 0; TriIdx < NumTriangles; ++TriIdx)
    {
        int32 Idx0 = ParsedIndices[TriIdx * 3 + 0];
        int32 Idx1 = ParsedIndices[TriIdx * 3 + 1];
        int32 Idx2 = ParsedIndices[TriIdx * 3 + 2];

        if (Idx0 >= VertexIDs.Num() || Idx1 >= VertexIDs.Num() || Idx2 >= VertexIDs.Num())
        {
            continue; // Skip invalid triangle
        }

        // Create vertex instances for this triangle
        TArray<FVertexInstanceID> VertexInstanceIDs;
        VertexInstanceIDs.Add(MeshDescription->CreateVertexInstance(VertexIDs[Idx0]));
        VertexInstanceIDs.Add(MeshDescription->CreateVertexInstance(VertexIDs[Idx1]));
        VertexInstanceIDs.Add(MeshDescription->CreateVertexInstance(VertexIDs[Idx2]));

        // Set vertex instance attributes
        for (int32 i = 0; i < 3; ++i)
        {
            int32 VertIdx = ParsedIndices[TriIdx * 3 + i];
            FVertexInstanceID InstanceID = VertexInstanceIDs[i];

            // Normal
            if (VertIdx < ParsedNormals.Num())
            {
                VertexNormals[InstanceID] = ParsedNormals[VertIdx];
            }
            else
            {
                VertexNormals[InstanceID] = FVector3f(0.0f, 0.0f, 1.0f);
            }

            // UV
            if (VertIdx < ParsedUVs.Num())
            {
                VertexUVs[InstanceID] = ParsedUVs[VertIdx];
            }
            else
            {
                VertexUVs[InstanceID] = FVector2f(0.0f, 0.0f);
            }

            // Tangent
            if (VertIdx < ParsedTangents.Num())
            {
                FVector4f Tan = ParsedTangents[VertIdx];
                VertexTangents[InstanceID] = FVector3f(Tan.X, Tan.Y, Tan.Z);
                VertexBinormalSigns[InstanceID] = FVector3f(0.0f, Tan.W > 0 ? 1.0f : -1.0f, 0.0f);
            }
            else
            {
                VertexTangents[InstanceID] = FVector3f(1.0f, 0.0f, 0.0f);
                VertexBinormalSigns[InstanceID] = FVector3f(0.0f, 1.0f, 0.0f);
            }

            // Color
            if (VertIdx < ParsedColors.Num())
            {
                VertexColors[InstanceID] = ParsedColors[VertIdx];
            }
            else
            {
                VertexColors[InstanceID] = FColor::White;
            }
        }

        // Create polygon
        FPolygonID PolygonID = MeshDescription->CreatePolygon(MeshDescription->GetPolygonGroupIDs()[0], VertexInstanceIDs);
    }

    // Build static mesh
    StaticMesh->BuildFromMeshDescription(*MeshDescription);

    // Notify asset registry
    FAssetRegistryModule::AssetCreated(StaticMesh);
    Package->MarkPackageDirty();

    // Optionally spawn in level
    bool bSpawnInLevel = true;
    Params->TryGetBoolField(TEXT("spawn_in_level"), bSpawnInLevel);

    FVector SpawnLocation = FVector::ZeroVector;
    if (Params->HasField(TEXT("location")))
    {
        const TArray<TSharedPtr<FJsonValue>>* LocationArray;
        if (Params->TryGetArrayField(TEXT("location"), LocationArray) && LocationArray->Num() >= 3)
        {
            SpawnLocation = FVector(
                (*LocationArray)[0]->AsNumber(),
                (*LocationArray)[1]->AsNumber(),
                (*LocationArray)[2]->AsNumber()
            );
        }
    }

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetBoolField(TEXT("success"), true);
    ResultObj->SetStringField(TEXT("mesh_path"), StaticMesh->GetPathName());
    ResultObj->SetNumberField(TEXT("num_vertices"), NumVertices);
    ResultObj->SetNumberField(TEXT("num_triangles"), NumTriangles);

    if (bSpawnInLevel)
    {
        UWorld* World = GEditor->GetEditorWorldContext().World();
        if (World)
        {
            FActorSpawnParameters SpawnParams;
            SpawnParams.Name = *MeshName;
            
            AStaticMeshActor* NewActor = World->SpawnActor<AStaticMeshActor>(
                AStaticMeshActor::StaticClass(),
                SpawnLocation,
                FRotator::ZeroRotator,
                SpawnParams
            );
            
            if (NewActor)
            {
                NewActor->GetStaticMeshComponent()->SetStaticMesh(StaticMesh);
                NewActor->SetActorLabel(MeshName);
                
                ResultObj->SetStringField(TEXT("spawned_actor"), NewActor->GetName());
                ResultObj->SetObjectField(TEXT("actor_info"), FEpicUnrealMCPCommonUtils::ActorToJsonObject(NewActor, true));
            }
        }
    }

    return ResultObj;
}
