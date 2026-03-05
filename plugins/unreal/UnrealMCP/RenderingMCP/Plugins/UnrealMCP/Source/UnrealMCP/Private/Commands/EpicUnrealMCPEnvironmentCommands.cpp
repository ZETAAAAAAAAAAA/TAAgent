#include "Commands/EpicUnrealMCPEnvironmentCommands.h"
#include "Commands/EpicUnrealMCPCommonUtils.h"
#include "Slate/SceneViewport.h"
#include "LevelEditor.h"
#include "ILevelEditor.h"
#include "IAssetViewport.h"
#include "SLevelViewport.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "ImageUtils.h"
#include "Engine/GameViewportClient.h"
#include "Engine/Engine.h"
#include "Misc/FileHelper.h"

// Light includes
#include "Engine/DirectionalLight.h"
#include "Engine/PointLight.h"
#include "Engine/SpotLight.h"
#include "Engine/RectLight.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/RectLightComponent.h"
#include "EngineUtils.h"
#include "Components/LightComponent.h"

// Post Process Volume includes
#include "Engine/PostProcessVolume.h"
#include "GameFramework/WorldSettings.h"

// Actor spawning includes
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Editor.h"

// Viewport screenshot
TSharedPtr<FJsonObject> FEpicUnrealMCPEnvironmentCommands::HandleGetViewportScreenshot(const TSharedPtr<FJsonObject>& Params)
{
    // Get optional parameters
    FString Format = TEXT("png");
    Params->TryGetStringField(TEXT("format"), Format);
    
    int32 Quality = 85;
    if (Params->HasField(TEXT("quality")))
    {
        Quality = Params->GetIntegerField(TEXT("quality"));
        Quality = FMath::Clamp(Quality, 1, 100);
    }
    
    bool bIncludeUI = false;
    if (Params->HasField(TEXT("include_ui")))
    {
        bIncludeUI = Params->GetBoolField(TEXT("include_ui"));
    }

    // Get output path (required for saving to file)
    FString OutputPath;
    if (!Params->TryGetStringField(TEXT("output_path"), OutputPath) || OutputPath.IsEmpty())
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("output_path parameter is required"));
    }

    // Get the active viewport
    FSceneViewport* TargetViewport = nullptr;
    FString ViewportType;
    
    // Try to get editor viewport first
    if (GEditor)
    {
        // Check for PIE viewport
        if (GEditor->PlayWorld)
        {
            UWorld* PlayWorld = GEditor->PlayWorld;
            if (UGameViewportClient* GVC = PlayWorld->GetGameViewport())
            {
                TargetViewport = GVC->GetGameViewport();
                ViewportType = TEXT("PIE");
            }
        }
        
        // Fall back to level editor viewport
        if (!TargetViewport)
        {
            FLevelEditorModule* LevelEditorModule = FModuleManager::GetModulePtr<FLevelEditorModule>(TEXT("LevelEditor"));
            if (LevelEditorModule)
            {
                TSharedPtr<ILevelEditor> LevelEditor = LevelEditorModule->GetFirstLevelEditor();
                if (LevelEditor.IsValid())
                {
                    TSharedPtr<SLevelViewport> ActiveViewport = LevelEditor->GetActiveViewportInterface();
                    if (ActiveViewport.IsValid())
                    {
                        TSharedPtr<FSceneViewport> SceneViewport = ActiveViewport->GetSharedActiveViewport();
                        if (SceneViewport.IsValid())
                        {
                            TargetViewport = SceneViewport.Get();
                            ViewportType = TEXT("Editor");
                        }
                    }
                }
            }
        }
    }
    
    if (!TargetViewport)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("No active viewport found"));
    }
    
    FIntPoint ViewportSize = TargetViewport->GetSizeXY();
    if (ViewportSize.X <= 0 || ViewportSize.Y <= 0)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Invalid viewport size"));
    }
    
    // Read pixels from viewport
    TArray<FColor> PixelData;
    FReadSurfaceDataFlags ReadFlags(RCM_UNorm, CubeFace_MAX);
    ReadFlags.SetLinearToGamma(true);
    
    bool bReadSuccess = TargetViewport->ReadPixels(PixelData, ReadFlags);
    
    if (!bReadSuccess || PixelData.Num() == 0)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to read viewport pixels"));
    }
    
    // Encode image
    TArray<uint8> ImageData;
    EImageFormat ImageFormat = EImageFormat::PNG;
    if (Format == TEXT("jpg") || Format == TEXT("jpeg"))
    {
        ImageFormat = EImageFormat::JPEG;
    }
    else if (Format == TEXT("bmp"))
    {
        ImageFormat = EImageFormat::BMP;
    }
    
    IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
    TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(ImageFormat);
    
    if (!ImageWrapper.IsValid())
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create image wrapper"));
    }
    
    ImageWrapper->SetRaw(PixelData.GetData(), PixelData.Num() * sizeof(FColor), ViewportSize.X, ViewportSize.Y, ERGBFormat::BGRA, 8);
    ImageData = ImageWrapper->GetCompressed(Quality);
    
    if (ImageData.Num() == 0)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to compress image"));
    }
    
    // Save image to file
    bool bSaved = FFileHelper::SaveArrayToFile(ImageData, *OutputPath);
    
    if (!bSaved)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Failed to save image to: %s"), *OutputPath));
    }
    
    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetBoolField(TEXT("success"), true);
    ResultObj->SetStringField(TEXT("file_path"), OutputPath);
    ResultObj->SetStringField(TEXT("format"), Format);
    ResultObj->SetNumberField(TEXT("width"), ViewportSize.X);
    ResultObj->SetNumberField(TEXT("height"), ViewportSize.Y);
    ResultObj->SetNumberField(TEXT("size_bytes"), ImageData.Num());
    ResultObj->SetStringField(TEXT("viewport_type"), ViewportType);
    
    return ResultObj;
}

// Main command dispatcher
TSharedPtr<FJsonObject> FEpicUnrealMCPEnvironmentCommands::HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params)
{
    if (CommandType == TEXT("get_viewport_screenshot"))
    {
        return HandleGetViewportScreenshot(Params);
    }
    else if (CommandType == TEXT("create_light"))
    {
        return HandleCreateLight(Params);
    }
    else if (CommandType == TEXT("set_light_properties"))
    {
        return HandleSetLightProperties(Params);
    }
    else if (CommandType == TEXT("get_lights"))
    {
        return HandleGetLights(Params);
    }
    else if (CommandType == TEXT("delete_light"))
    {
        return HandleDeleteLight(Params);
    }
    else if (CommandType == TEXT("create_post_process_volume"))
    {
        return HandleCreatePostProcessVolume(Params);
    }
    else if (CommandType == TEXT("set_post_process_settings"))
    {
        return HandleSetPostProcessSettings(Params);
    }
    else if (CommandType == TEXT("spawn_basic_actor"))
    {
        return HandleSpawnBasicActor(Params);
    }
    else if (CommandType == TEXT("set_actor_material"))
    {
        return HandleSetActorMaterial(Params);
    }
    
    return FEpicUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Unknown environment command: %s"), *CommandType));
}

// Light management - TODO: Migrate from main file
TSharedPtr<FJsonObject> FEpicUnrealMCPEnvironmentCommands::HandleCreateLight(const TSharedPtr<FJsonObject>& Params)
{
    return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("create_light not yet migrated to EnvironmentCommands"));
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEnvironmentCommands::HandleSetLightProperties(const TSharedPtr<FJsonObject>& Params)
{
    return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("set_light_properties not yet migrated to EnvironmentCommands"));
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEnvironmentCommands::HandleGetLights(const TSharedPtr<FJsonObject>& Params)
{
    return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("get_lights not yet migrated to EnvironmentCommands"));
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEnvironmentCommands::HandleDeleteLight(const TSharedPtr<FJsonObject>& Params)
{
    return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("delete_light not yet migrated to EnvironmentCommands"));
}

// Post Process Volume
TSharedPtr<FJsonObject> FEpicUnrealMCPEnvironmentCommands::HandleCreatePostProcessVolume(const TSharedPtr<FJsonObject>& Params)
{
    if (!GEditor)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Editor not available"));
    }

    UWorld* World = GEditor->GetEditorWorldContext().World();
    if (!World)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("No active world found"));
    }

    // Get name (optional)
    FString VolumeName = TEXT("PostProcessVolume_Lookdev");
    Params->TryGetStringField(TEXT("name"), VolumeName);

    // Get location (optional, default to origin)
    FVector Location(0, 0, 0);
    const TSharedPtr<FJsonObject>* LocationObj;
    if (Params->TryGetObjectField(TEXT("location"), LocationObj))
    {
        double X = 0, Y = 0, Z = 0;
        LocationObj->Get()->TryGetNumberField(TEXT("x"), X);
        LocationObj->Get()->TryGetNumberField(TEXT("y"), Y);
        LocationObj->Get()->TryGetNumberField(TEXT("z"), Z);
        Location = FVector(X, Y, Z);
    }

    // Get scale (optional, default to large volume covering the scene)
    FVector Scale(1000, 1000, 1000);
    const TSharedPtr<FJsonObject>* ScaleObj;
    if (Params->TryGetObjectField(TEXT("scale"), ScaleObj))
    {
        double X = 1000, Y = 1000, Z = 1000;
        ScaleObj->Get()->TryGetNumberField(TEXT("x"), X);
        ScaleObj->Get()->TryGetNumberField(TEXT("y"), Y);
        ScaleObj->Get()->TryGetNumberField(TEXT("z"), Z);
        Scale = FVector(X, Y, Z);
    }

    // Spawn PostProcessVolume
    FActorSpawnParameters SpawnParams;
    SpawnParams.Name = FName(*VolumeName);
    SpawnParams.NameMode = FActorSpawnParameters::ESpawnActorNameMode::Requested;

    APostProcessVolume* PostProcessVolume = World->SpawnActor<APostProcessVolume>(APostProcessVolume::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);

    if (!PostProcessVolume)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to spawn PostProcessVolume"));
    }

    // Set scale
    PostProcessVolume->SetActorScale3D(Scale);

    // Enable unbound by default (affects entire scene)
    PostProcessVolume->bUnbound = true;

    // Initialize default Lookdev settings
    FPostProcessSettings& Settings = PostProcessVolume->Settings;
    
    // Exposure settings for Lookdev (EV100=0, Manual mode)
    Settings.bOverride_AutoExposureMethod = true;
    Settings.AutoExposureMethod = AEM_Manual;
    Settings.bOverride_AutoExposureBias = true;
    Settings.AutoExposureBias = 0.0f;
    // Note: CameraExposureValue is not available in UE 5.7, use AutoExposureBias instead

    // Disable effects that interfere with accurate metering
    Settings.bOverride_BloomIntensity = true;
    Settings.BloomIntensity = 0.0f;
    Settings.bOverride_VignetteIntensity = true;
    Settings.VignetteIntensity = 0.0f;
    Settings.bOverride_AmbientOcclusionIntensity = true;
    Settings.AmbientOcclusionIntensity = 0.0f;

    // Enable the volume
    PostProcessVolume->bEnabled = true;

    // Refresh the level
    GEditor->RedrawLevelEditingViewports();

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetBoolField(TEXT("success"), true);
    ResultObj->SetStringField(TEXT("name"), PostProcessVolume->GetName());
    ResultObj->SetStringField(TEXT("actor_path"), PostProcessVolume->GetPathName());

    // Location
    TSharedPtr<FJsonObject> LocationResult = MakeShared<FJsonObject>();
    LocationResult->SetNumberField(TEXT("x"), Location.X);
    LocationResult->SetNumberField(TEXT("y"), Location.Y);
    LocationResult->SetNumberField(TEXT("z"), Location.Z);
    ResultObj->SetObjectField(TEXT("location"), LocationResult);

    // Scale
    TSharedPtr<FJsonObject> ScaleResult = MakeShared<FJsonObject>();
    ScaleResult->SetNumberField(TEXT("x"), Scale.X);
    ScaleResult->SetNumberField(TEXT("y"), Scale.Y);
    ScaleResult->SetNumberField(TEXT("z"), Scale.Z);
    ResultObj->SetObjectField(TEXT("scale"), ScaleResult);

    ResultObj->SetBoolField(TEXT("unbound"), PostProcessVolume->bUnbound);

    return ResultObj;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEnvironmentCommands::HandleSetPostProcessSettings(const TSharedPtr<FJsonObject>& Params)
{
    if (!GEditor)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Editor not available"));
    }

    // Get volume name (optional, if not provided use first available)
    FString VolumeName;
    Params->TryGetStringField(TEXT("name"), VolumeName);

    UWorld* World = GEditor->GetEditorWorldContext().World();
    if (!World)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("No active world found"));
    }

    APostProcessVolume* TargetVolume = nullptr;

    // Find the volume
    if (!VolumeName.IsEmpty())
    {
        for (TActorIterator<APostProcessVolume> It(World); It; ++It)
        {
            if (It->GetName() == VolumeName)
            {
                TargetVolume = *It;
                break;
            }
        }
    }
    else
    {
        // Use first available
        for (TActorIterator<APostProcessVolume> It(World); It; ++It)
        {
            TargetVolume = *It;
            break;
        }
    }

    if (!TargetVolume)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(
            VolumeName.IsEmpty() 
                ? TEXT("No PostProcessVolume found in the level") 
                : FString::Printf(TEXT("PostProcessVolume not found: %s"), *VolumeName));
    }

    FPostProcessSettings& Settings = TargetVolume->Settings;

    // Exposure settings
    if (Params->HasField(TEXT("exposure_mode")))
    {
        FString ModeStr;
        Params->TryGetStringField(TEXT("exposure_mode"), ModeStr);
        Settings.bOverride_AutoExposureMethod = true;
        if (ModeStr == TEXT("manual"))
        {
            Settings.AutoExposureMethod = AEM_Manual;
        }
        else if (ModeStr == TEXT("auto"))
        {
            Settings.AutoExposureMethod = AEM_Histogram;
        }
    }

    if (Params->HasField(TEXT("exposure_bias")))
    {
        double Bias = 0.0;
        Params->TryGetNumberField(TEXT("exposure_bias"), Bias);
        Settings.bOverride_AutoExposureBias = true;
        Settings.AutoExposureBias = (float)Bias;
    }

    if (Params->HasField(TEXT("exposure_value")))
    {
        // In UE 5.7, CameraExposureValue is not available, map to AutoExposureBias
        double EV = 0.0;
        Params->TryGetNumberField(TEXT("exposure_value"), EV);
        Settings.bOverride_AutoExposureBias = true;
        Settings.AutoExposureBias = (float)EV;
    }

    // Effect toggles
    if (Params->HasField(TEXT("bloom_enabled")))
    {
        bool bEnabled = false;
        Params->TryGetBoolField(TEXT("bloom_enabled"), bEnabled);
        Settings.bOverride_BloomIntensity = true;
        Settings.BloomIntensity = bEnabled ? 1.0f : 0.0f;
    }

    if (Params->HasField(TEXT("vignette_enabled")))
    {
        bool bEnabled = false;
        Params->TryGetBoolField(TEXT("vignette_enabled"), bEnabled);
        Settings.bOverride_VignetteIntensity = true;
        Settings.VignetteIntensity = bEnabled ? 0.4f : 0.0f;
    }

    if (Params->HasField(TEXT("ao_enabled")))
    {
        bool bEnabled = false;
        Params->TryGetBoolField(TEXT("ao_enabled"), bEnabled);
        Settings.bOverride_AmbientOcclusionIntensity = true;
        Settings.AmbientOcclusionIntensity = bEnabled ? 1.0f : 0.0f;
    }

    // Unbound setting
    if (Params->HasField(TEXT("unbound")))
    {
        bool bUnbound = true;
        Params->TryGetBoolField(TEXT("unbound"), bUnbound);
        TargetVolume->bUnbound = bUnbound;
    }

    // Enabled setting
    if (Params->HasField(TEXT("enabled")))
    {
        bool bEnabled = true;
        Params->TryGetBoolField(TEXT("enabled"), bEnabled);
        TargetVolume->bEnabled = bEnabled;
    }

    GEditor->RedrawLevelEditingViewports();

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetBoolField(TEXT("success"), true);
    ResultObj->SetStringField(TEXT("name"), TargetVolume->GetName());
    ResultObj->SetNumberField(TEXT("exposure_bias"), Settings.AutoExposureBias);
    // CameraExposureValue not available in UE 5.7, use AutoExposureBias instead
    ResultObj->SetNumberField(TEXT("exposure_value"), Settings.AutoExposureBias);
    ResultObj->SetNumberField(TEXT("bloom_intensity"), Settings.BloomIntensity);
    ResultObj->SetNumberField(TEXT("vignette_intensity"), Settings.VignetteIntensity);
    ResultObj->SetBoolField(TEXT("unbound"), TargetVolume->bUnbound);
    ResultObj->SetBoolField(TEXT("enabled"), TargetVolume->bEnabled);

    return ResultObj;
}

// Actor Spawning
TSharedPtr<FJsonObject> FEpicUnrealMCPEnvironmentCommands::HandleSpawnBasicActor(const TSharedPtr<FJsonObject>& Params)
{
    if (!GEditor)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Editor not available"));
    }

    UWorld* World = GEditor->GetEditorWorldContext().World();
    if (!World)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("No active world found"));
    }

    // Get actor type (required)
    FString ActorType;
    if (!Params->TryGetStringField(TEXT("actor_type"), ActorType))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'actor_type' parameter"));
    }

    // Get name (optional)
    FString ActorName;
    Params->TryGetStringField(TEXT("name"), ActorName);

    // Get location (optional)
    FVector Location(0, 0, 0);
    const TSharedPtr<FJsonObject>* LocationObj;
    if (Params->TryGetObjectField(TEXT("location"), LocationObj))
    {
        double X = 0, Y = 0, Z = 0;
        LocationObj->Get()->TryGetNumberField(TEXT("x"), X);
        LocationObj->Get()->TryGetNumberField(TEXT("y"), Y);
        LocationObj->Get()->TryGetNumberField(TEXT("z"), Z);
        Location = FVector(X, Y, Z);
    }

    // Get rotation (optional)
    FRotator Rotation(0, 0, 0);
    const TSharedPtr<FJsonObject>* RotationObj;
    if (Params->TryGetObjectField(TEXT("rotation"), RotationObj))
    {
        double Pitch = 0, Yaw = 0, Roll = 0;
        RotationObj->Get()->TryGetNumberField(TEXT("pitch"), Pitch);
        RotationObj->Get()->TryGetNumberField(TEXT("yaw"), Yaw);
        RotationObj->Get()->TryGetNumberField(TEXT("roll"), Roll);
        Rotation = FRotator(Pitch, Yaw, Roll);
    }

    // Get scale (optional)
    FVector Scale(1, 1, 1);
    const TSharedPtr<FJsonObject>* ScaleObj;
    if (Params->TryGetObjectField(TEXT("scale"), ScaleObj))
    {
        double X = 1, Y = 1, Z = 1;
        ScaleObj->Get()->TryGetNumberField(TEXT("x"), X);
        ScaleObj->Get()->TryGetNumberField(TEXT("y"), Y);
        ScaleObj->Get()->TryGetNumberField(TEXT("z"), Z);
        Scale = FVector(X, Y, Z);
    }

    AActor* SpawnedActor = nullptr;

    FActorSpawnParameters SpawnParams;
    if (!ActorName.IsEmpty())
    {
        SpawnParams.Name = FName(*ActorName);
        SpawnParams.NameMode = FActorSpawnParameters::ESpawnActorNameMode::Requested;
    }

    // Handle different actor types
    if (ActorType == TEXT("StaticMeshActor") || ActorType == TEXT("mesh"))
    {
        AStaticMeshActor* MeshActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, Rotation, SpawnParams);
        
        if (MeshActor)
        {
            // Get mesh path (optional)
            FString MeshPath;
            if (Params->TryGetStringField(TEXT("mesh_path"), MeshPath) && !MeshPath.IsEmpty())
            {
                UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, *MeshPath);
                if (Mesh)
                {
                    MeshActor->GetStaticMeshComponent()->SetStaticMesh(Mesh);
                }
            }
            
            MeshActor->SetActorScale3D(Scale);
            SpawnedActor = MeshActor;
        }
    }
    else if (ActorType == TEXT("Sphere") || ActorType == TEXT("sphere"))
    {
        // Spawn sphere for lookdev material ball
        AStaticMeshActor* MeshActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, Rotation, SpawnParams);
        
        if (MeshActor)
        {
            UStaticMesh* SphereMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
            if (SphereMesh)
            {
                MeshActor->GetStaticMeshComponent()->SetStaticMesh(SphereMesh);
            }
            MeshActor->SetActorScale3D(Scale);
            SpawnedActor = MeshActor;
        }
    }
    else if (ActorType == TEXT("Cube") || ActorType == TEXT("cube") || ActorType == TEXT("Box"))
    {
        AStaticMeshActor* MeshActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, Rotation, SpawnParams);
        
        if (MeshActor)
        {
            UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
            if (CubeMesh)
            {
                MeshActor->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
            }
            MeshActor->SetActorScale3D(Scale);
            SpawnedActor = MeshActor;
        }
    }
    else if (ActorType == TEXT("Plane") || ActorType == TEXT("plane"))
    {
        AStaticMeshActor* MeshActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, Rotation, SpawnParams);
        
        if (MeshActor)
        {
            UStaticMesh* PlaneMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
            if (PlaneMesh)
            {
                MeshActor->GetStaticMeshComponent()->SetStaticMesh(PlaneMesh);
            }
            MeshActor->SetActorScale3D(Scale);
            SpawnedActor = MeshActor;
        }
    }
    else if (ActorType == TEXT("Cylinder"))
    {
        AStaticMeshActor* MeshActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, Rotation, SpawnParams);
        
        if (MeshActor)
        {
            UStaticMesh* CylinderMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
            if (CylinderMesh)
            {
                MeshActor->GetStaticMeshComponent()->SetStaticMesh(CylinderMesh);
            }
            MeshActor->SetActorScale3D(Scale);
            SpawnedActor = MeshActor;
        }
    }
    else
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(
            FString::Printf(TEXT("Unknown actor_type: %s. Supported: StaticMeshActor, Sphere, Cube, Plane, Cylinder"), *ActorType));
    }

    if (!SpawnedActor)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to spawn actor"));
    }

    GEditor->RedrawLevelEditingViewports();

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetBoolField(TEXT("success"), true);
    ResultObj->SetStringField(TEXT("name"), SpawnedActor->GetName());
    ResultObj->SetStringField(TEXT("actor_type"), ActorType);
    ResultObj->SetStringField(TEXT("actor_path"), SpawnedActor->GetPathName());

    // Location
    TSharedPtr<FJsonObject> LocationResult = MakeShared<FJsonObject>();
    LocationResult->SetNumberField(TEXT("x"), Location.X);
    LocationResult->SetNumberField(TEXT("y"), Location.Y);
    LocationResult->SetNumberField(TEXT("z"), Location.Z);
    ResultObj->SetObjectField(TEXT("location"), LocationResult);

    // Rotation
    TSharedPtr<FJsonObject> RotationResult = MakeShared<FJsonObject>();
    RotationResult->SetNumberField(TEXT("pitch"), Rotation.Pitch);
    RotationResult->SetNumberField(TEXT("yaw"), Rotation.Yaw);
    RotationResult->SetNumberField(TEXT("roll"), Rotation.Roll);
    ResultObj->SetObjectField(TEXT("rotation"), RotationResult);

    return ResultObj;
}

// Set Actor Material
TSharedPtr<FJsonObject> FEpicUnrealMCPEnvironmentCommands::HandleSetActorMaterial(const TSharedPtr<FJsonObject>& Params)
{
    if (!GEditor)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Editor not available"));
    }

    // Get actor name (required)
    FString ActorName;
    if (!Params->TryGetStringField(TEXT("actor_name"), ActorName))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'actor_name' parameter"));
    }

    // Get material path (required)
    FString MaterialPath;
    if (!Params->TryGetStringField(TEXT("material_path"), MaterialPath))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'material_path' parameter"));
    }

    UWorld* World = GEditor->GetEditorWorldContext().World();
    if (!World)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("No active world found"));
    }

    // Find the actor
    AActor* TargetActor = nullptr;
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        if (It->GetName() == ActorName)
        {
            TargetActor = *It;
            break;
        }
    }

    if (!TargetActor)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(
            FString::Printf(TEXT("Actor not found: %s"), *ActorName));
    }

    // Load the material
    UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, *MaterialPath);
    if (!Material)
    {
        // Try with /Game/ prefix
        if (!MaterialPath.StartsWith(TEXT("/")))
        {
            MaterialPath = FString::Printf(TEXT("/Game/%s"), *MaterialPath);
            Material = LoadObject<UMaterialInterface>(nullptr, *MaterialPath);
        }
    }

    if (!Material)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(
            FString::Printf(TEXT("Material not found: %s"), *MaterialPath));
    }

    // Apply material to the actor's mesh component
    int32 ComponentsModified = 0;

    // Check for StaticMeshComponent
    if (AStaticMeshActor* StaticMeshActor = Cast<AStaticMeshActor>(TargetActor))
    {
        if (UStaticMeshComponent* MeshComp = StaticMeshActor->GetStaticMeshComponent())
        {
            MeshComp->SetMaterial(0, Material);
            ComponentsModified++;
        }
    }
    else
    {
        // Try to find any mesh component
        TArray<UStaticMeshComponent*> MeshComponents;
        TargetActor->GetComponents<UStaticMeshComponent>(MeshComponents);
        
        for (UStaticMeshComponent* MeshComp : MeshComponents)
        {
            MeshComp->SetMaterial(0, Material);
            ComponentsModified++;
        }
    }

    if (ComponentsModified == 0)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(
            TEXT("Actor has no mesh component to apply material to"));
    }

    GEditor->RedrawLevelEditingViewports();

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetBoolField(TEXT("success"), true);
    ResultObj->SetStringField(TEXT("actor_name"), ActorName);
    ResultObj->SetStringField(TEXT("material_path"), MaterialPath);
    ResultObj->SetNumberField(TEXT("components_modified"), ComponentsModified);

    return ResultObj;
}
