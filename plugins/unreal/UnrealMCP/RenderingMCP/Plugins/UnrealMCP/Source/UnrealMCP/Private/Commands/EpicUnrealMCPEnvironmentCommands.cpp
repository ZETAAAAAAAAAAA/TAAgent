#include "Commands/EpicUnrealMCPEnvironmentCommands.h"
#include "Commands/EpicUnrealMCPCommonUtils.h"

// Core UE includes
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Editor.h"
#include "EngineUtils.h"

// Reflection includes
#include "UObject/UObjectIterator.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/PropertyPortFlags.h"

// Actor includes
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Components/SceneComponent.h"
#include "Components/PrimitiveComponent.h"

// Light includes
#include "Engine/Light.h"
#include "Components/LightComponent.h"

// Mesh includes
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"

// Post Process
#include "Engine/PostProcessVolume.h"

// Viewport includes
#include "Slate/SceneViewport.h"
#include "LevelEditor.h"
#include "ILevelEditor.h"
#include "IAssetViewport.h"
#include "SLevelViewport.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Engine/GameViewportClient.h"
#include "Misc/FileHelper.h"
#include "Misc/Base64.h"

// ============================================================================
// Viewport Screenshot
// ============================================================================
TSharedPtr<FJsonObject> FEpicUnrealMCPEnvironmentCommands::HandleGetViewportScreenshot(const TSharedPtr<FJsonObject>& Params)
{
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

    FString OutputPath;
    if (!Params->TryGetStringField(TEXT("output_path"), OutputPath) || OutputPath.IsEmpty())
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("output_path parameter is required"));
    }

    FSceneViewport* TargetViewport = nullptr;
    FString ViewportType;
    
    if (GEditor)
    {
        if (GEditor->PlayWorld)
        {
            UWorld* PlayWorld = GEditor->PlayWorld;
            if (UGameViewportClient* GVC = PlayWorld->GetGameViewport())
            {
                TargetViewport = GVC->GetGameViewport();
                ViewportType = TEXT("PIE");
            }
        }
        
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
    
    TArray<FColor> PixelData;
    FReadSurfaceDataFlags ReadFlags(RCM_UNorm, CubeFace_MAX);
    ReadFlags.SetLinearToGamma(true);
    
    bool bReadSuccess = TargetViewport->ReadPixels(PixelData, ReadFlags);
    
    if (!bReadSuccess || PixelData.Num() == 0)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to read viewport pixels"));
    }
    
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

// ============================================================================
// Command Dispatcher
// ============================================================================
TSharedPtr<FJsonObject> FEpicUnrealMCPEnvironmentCommands::HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params)
{
    // Generic Actor Management
    if (CommandType == TEXT("spawn_actor"))
    {
        return HandleSpawnActor(Params);
    }
    else if (CommandType == TEXT("delete_actor"))
    {
        return HandleDeleteActor(Params);
    }
    else if (CommandType == TEXT("get_actors"))
    {
        return HandleGetActors(Params);
    }
    else if (CommandType == TEXT("set_actor_properties"))
    {
        return HandleSetActorProperties(Params);
    }
    else if (CommandType == TEXT("get_actor_properties"))
    {
        return HandleGetActorProperties(Params);
    }
    else if (CommandType == TEXT("get_viewport_screenshot"))
    {
        return HandleGetViewportScreenshot(Params);
    }
    // Legacy compatibility (deprecated)
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

// ============================================================================
// Reflection Helpers
// ============================================================================
UClass* FEpicUnrealMCPEnvironmentCommands::FindClassByName(const FString& ClassName)
{
    if (ClassName.IsEmpty())
    {
        return nullptr;
    }

    // Try direct lookup with various prefixes
    TArray<FString> PossibleNames;
    
    // Original name
    PossibleNames.Add(ClassName);
    
    // With A prefix (common for Actors)
    if (!ClassName.StartsWith(TEXT("A")))
    {
        PossibleNames.Add(TEXT("A") + ClassName);
    }
    
    // With U prefix (common for Components/Objects)
    if (!ClassName.StartsWith(TEXT("U")))
    {
        PossibleNames.Add(TEXT("U") + ClassName);
    }
    
    // Common variations
    if (ClassName == TEXT("PointLight") || ClassName == TEXT("point_light"))
    {
        PossibleNames.Add(TEXT("APointLight"));
    }
    else if (ClassName == TEXT("DirectionalLight") || ClassName == TEXT("directional_light"))
    {
        PossibleNames.Add(TEXT("ADirectionalLight"));
    }
    else if (ClassName == TEXT("SpotLight") || ClassName == TEXT("spot_light"))
    {
        PossibleNames.Add(TEXT("ASpotLight"));
    }
    else if (ClassName == TEXT("RectLight") || ClassName == TEXT("rect_light"))
    {
        PossibleNames.Add(TEXT("ARectLight"));
    }
    else if (ClassName == TEXT("StaticMeshActor") || ClassName == TEXT("mesh"))
    {
        PossibleNames.Add(TEXT("AStaticMeshActor"));
    }
    else if (ClassName == TEXT("Sphere"))
    {
        PossibleNames.Add(TEXT("AStaticMeshActor"));
    }
    else if (ClassName == TEXT("Cube") || ClassName == TEXT("Box"))
    {
        PossibleNames.Add(TEXT("AStaticMeshActor"));
    }
    else if (ClassName == TEXT("Plane"))
    {
        PossibleNames.Add(TEXT("AStaticMeshActor"));
    }
    else if (ClassName == TEXT("Cylinder"))
    {
        PossibleNames.Add(TEXT("AStaticMeshActor"));
    }
    else if (ClassName == TEXT("PostProcessVolume") || ClassName == TEXT("post_process_volume"))
    {
        PossibleNames.Add(TEXT("APostProcessVolume"));
    }

    // Try each possible name
    for (const FString& Name : PossibleNames)
    {
        UClass* FoundClass = FindObject<UClass>(nullptr, *Name);
        if (FoundClass)
        {
            return FoundClass;
        }
        
        // Try with /Script/Engine prefix
        FString EnginePath = TEXT("/Script/Engine.") + Name;
        FoundClass = FindObject<UClass>(nullptr, *EnginePath);
        if (FoundClass)
        {
            return FoundClass;
        }
    }

    // Fallback: iterate all classes (slower but catches more)
    for (TObjectIterator<UClass> It; It; ++It)
    {
        if (It->GetName() == ClassName || 
            It->GetName() == TEXT("A") + ClassName ||
            It->GetName() == TEXT("U") + ClassName)
        {
            return *It;
        }
    }

    return nullptr;
}

bool FEpicUnrealMCPEnvironmentCommands::SetPropertyByName(UObject* Object, const FString& PropertyName, 
    const TSharedPtr<FJsonValue>& Value, FString& OutError)
{
    if (!Object)
    {
        OutError = TEXT("Invalid object");
        return false;
    }

    if (!Value.IsValid())
    {
        OutError = TEXT("Invalid value");
        return false;
    }

    // Try to find property on the object itself
    FProperty* Property = Object->GetClass()->FindPropertyByName(*PropertyName);
    
    if (Property)
    {
        // Use CommonUtils for actual setting (it handles type conversion)
        return FEpicUnrealMCPCommonUtils::SetObjectProperty(Object, PropertyName, Value, OutError);
    }

    // Special handling for nested component properties (e.g., "intensity" on LightComponent)
    // Try to find a component that might have this property
    if (AActor* Actor = Cast<AActor>(Object))
    {
        // Common component property mappings
        TArray<TPair<FString, FString>> ComponentMappings;
        
        // Light properties
        if (PropertyName == TEXT("intensity") || PropertyName == TEXT("color") || 
            PropertyName == TEXT("cast_shadows") || PropertyName == TEXT("temperature") ||
            PropertyName == TEXT("attenuation_radius") || PropertyName == TEXT("outer_cone_angle") ||
            PropertyName == TEXT("inner_cone_angle") || PropertyName == TEXT("source_radius") ||
            PropertyName == TEXT("use_temperature"))
        {
            ComponentMappings.Add(TPair<FString, FString>(TEXT("LightComponent"), PropertyName));
            ComponentMappings.Add(TPair<FString, FString>(TEXT("PointLightComponent"), PropertyName));
            ComponentMappings.Add(TPair<FString, FString>(TEXT("SpotLightComponent"), PropertyName));
            ComponentMappings.Add(TPair<FString, FString>(TEXT("DirectionalLightComponent"), PropertyName));
            ComponentMappings.Add(TPair<FString, FString>(TEXT("RectLightComponent"), PropertyName));
        }
        // Mesh properties
        else if (PropertyName == TEXT("static_mesh") || PropertyName == TEXT("material") ||
                 PropertyName == TEXT("materials") || PropertyName == TEXT("override_materials"))
        {
            ComponentMappings.Add(TPair<FString, FString>(TEXT("StaticMeshComponent"), PropertyName));
            ComponentMappings.Add(TPair<FString, FString>(TEXT("MeshComponent"), PropertyName));
        }
        // Transform shortcuts
        else if (PropertyName == TEXT("location"))
        {
            // Handle location via SetActorLocation
            if (Value->Type == EJson::Object)
            {
                const TSharedPtr<FJsonObject>* LocObj;
                if (Value->TryGetObject(LocObj))
                {
                    double X = 0, Y = 0, Z = 0;
                    LocObj->Get()->TryGetNumberField(TEXT("x"), X);
                    LocObj->Get()->TryGetNumberField(TEXT("y"), Y);
                    LocObj->Get()->TryGetNumberField(TEXT("z"), Z);
                    Actor->SetActorLocation(FVector(X, Y, Z));
                    return true;
                }
            }
            else if (Value->Type == EJson::Array)
            {
                const TArray<TSharedPtr<FJsonValue>>* Arr;
                if (Value->TryGetArray(Arr) && Arr->Num() >= 3)
                {
                    FVector Loc((*Arr)[0]->AsNumber(), (*Arr)[1]->AsNumber(), (*Arr)[2]->AsNumber());
                    Actor->SetActorLocation(Loc);
                    return true;
                }
            }
        }
        else if (PropertyName == TEXT("rotation"))
        {
            // Handle rotation via SetActorRotation
            if (Value->Type == EJson::Object)
            {
                const TSharedPtr<FJsonObject>* RotObj;
                if (Value->TryGetObject(RotObj))
                {
                    double Pitch = 0, Yaw = 0, Roll = 0;
                    RotObj->Get()->TryGetNumberField(TEXT("pitch"), Pitch);
                    RotObj->Get()->TryGetNumberField(TEXT("yaw"), Yaw);
                    RotObj->Get()->TryGetNumberField(TEXT("roll"), Roll);
                    Actor->SetActorRotation(FRotator(Pitch, Yaw, Roll));
                    return true;
                }
            }
            else if (Value->Type == EJson::Array)
            {
                const TArray<TSharedPtr<FJsonValue>>* Arr;
                if (Value->TryGetArray(Arr) && Arr->Num() >= 3)
                {
                    FRotator Rot((*Arr)[0]->AsNumber(), (*Arr)[1]->AsNumber(), (*Arr)[2]->AsNumber());
                    Actor->SetActorRotation(Rot);
                    return true;
                }
            }
        }
        else if (PropertyName == TEXT("scale"))
        {
            // Handle scale via SetActorScale3D
            if (Value->Type == EJson::Object)
            {
                const TSharedPtr<FJsonObject>* ScaleObj;
                if (Value->TryGetObject(ScaleObj))
                {
                    double X = 1, Y = 1, Z = 1;
                    ScaleObj->Get()->TryGetNumberField(TEXT("x"), X);
                    ScaleObj->Get()->TryGetNumberField(TEXT("y"), Y);
                    ScaleObj->Get()->TryGetNumberField(TEXT("z"), Z);
                    Actor->SetActorScale3D(FVector(X, Y, Z));
                    return true;
                }
            }
            else if (Value->Type == EJson::Array)
            {
                const TArray<TSharedPtr<FJsonValue>>* Arr;
                if (Value->TryGetArray(Arr) && Arr->Num() >= 3)
                {
                    FVector Scale((*Arr)[0]->AsNumber(), (*Arr)[1]->AsNumber(), (*Arr)[2]->AsNumber());
                    Actor->SetActorScale3D(Scale);
                    return true;
                }
            }
        }

        // Try each component mapping
        for (const auto& Mapping : ComponentMappings)
        {
            UActorComponent* Component = FindActorComponent(Actor, Mapping.Key);
            if (Component)
            {
                // Try to set the property on the component
                if (FEpicUnrealMCPCommonUtils::SetObjectProperty(Component, Mapping.Value, Value, OutError))
                {
                    return true;
                }
            }
        }
    }

    OutError = FString::Printf(TEXT("Property '%s' not found on %s"), *PropertyName, *Object->GetName());
    return false;
}

TSharedPtr<FJsonValue> FEpicUnrealMCPEnvironmentCommands::GetPropertyAsJsonValue(UObject* Object, const FString& PropertyName)
{
    if (!Object)
    {
        return MakeShared<FJsonValueNull>();
    }

    FProperty* Property = Object->GetClass()->FindPropertyByName(*PropertyName);
    if (!Property)
    {
        return MakeShared<FJsonValueNull>();
    }

    void* PropertyAddr = Property->ContainerPtrToValuePtr<void>(Object);

    // Bool
    if (FBoolProperty* BoolProp = CastField<FBoolProperty>(Property))
    {
        return MakeShared<FJsonValueBoolean>(BoolProp->GetPropertyValue(PropertyAddr));
    }
    // Int
    else if (FIntProperty* IntProp = CastField<FIntProperty>(Property))
    {
        return MakeShared<FJsonValueNumber>(IntProp->GetPropertyValue(PropertyAddr));
    }
    // Float
    else if (FFloatProperty* FloatProp = CastField<FFloatProperty>(Property))
    {
        return MakeShared<FJsonValueNumber>(FloatProp->GetPropertyValue(PropertyAddr));
    }
    // Double
    else if (FDoubleProperty* DoubleProp = CastField<FDoubleProperty>(Property))
    {
        return MakeShared<FJsonValueNumber>(DoubleProp->GetPropertyValue(PropertyAddr));
    }
    // String/Text/Name
    else if (FStrProperty* StrProp = CastField<FStrProperty>(Property))
    {
        return MakeShared<FJsonValueString>(StrProp->GetPropertyValue(PropertyAddr));
    }
    else if (FTextProperty* TextProp = CastField<FTextProperty>(Property))
    {
        return MakeShared<FJsonValueString>(TextProp->GetPropertyValue(PropertyAddr).ToString());
    }
    else if (FNameProperty* NameProp = CastField<FNameProperty>(Property))
    {
        return MakeShared<FJsonValueString>(NameProp->GetPropertyValue(PropertyAddr).ToString());
    }
    // Vector
    else if (FStructProperty* StructProp = CastField<FStructProperty>(Property))
    {
        if (StructProp->Struct == TBaseStructure<FVector>::Get())
        {
            FVector* Vec = (FVector*)PropertyAddr;
            TArray<TSharedPtr<FJsonValue>> Arr;
            Arr.Add(MakeShared<FJsonValueNumber>(Vec->X));
            Arr.Add(MakeShared<FJsonValueNumber>(Vec->Y));
            Arr.Add(MakeShared<FJsonValueNumber>(Vec->Z));
            return MakeShared<FJsonValueArray>(Arr);
        }
        else if (StructProp->Struct == TBaseStructure<FRotator>::Get())
        {
            FRotator* Rot = (FRotator*)PropertyAddr;
            TArray<TSharedPtr<FJsonValue>> Arr;
            Arr.Add(MakeShared<FJsonValueNumber>(Rot->Pitch));
            Arr.Add(MakeShared<FJsonValueNumber>(Rot->Yaw));
            Arr.Add(MakeShared<FJsonValueNumber>(Rot->Roll));
            return MakeShared<FJsonValueArray>(Arr);
        }
        else if (StructProp->Struct == TBaseStructure<FColor>::Get())
        {
            FColor* Col = (FColor*)PropertyAddr;
            TArray<TSharedPtr<FJsonValue>> Arr;
            Arr.Add(MakeShared<FJsonValueNumber>(Col->R));
            Arr.Add(MakeShared<FJsonValueNumber>(Col->G));
            Arr.Add(MakeShared<FJsonValueNumber>(Col->B));
            Arr.Add(MakeShared<FJsonValueNumber>(Col->A));
            return MakeShared<FJsonValueArray>(Arr);
        }
        else if (StructProp->Struct == TBaseStructure<FLinearColor>::Get())
        {
            FLinearColor* Col = (FLinearColor*)PropertyAddr;
            TArray<TSharedPtr<FJsonValue>> Arr;
            Arr.Add(MakeShared<FJsonValueNumber>(Col->R));
            Arr.Add(MakeShared<FJsonValueNumber>(Col->G));
            Arr.Add(MakeShared<FJsonValueNumber>(Col->B));
            Arr.Add(MakeShared<FJsonValueNumber>(Col->A));
            return MakeShared<FJsonValueArray>(Arr);
        }
    }
    // Enum
    else if (FEnumProperty* EnumProp = CastField<FEnumProperty>(Property))
    {
        int64 Value = EnumProp->GetUnderlyingProperty()->GetSignedIntPropertyValue(PropertyAddr);
        return MakeShared<FJsonValueNumber>(Value);
    }
    else if (FByteProperty* ByteProp = CastField<FByteProperty>(Property))
    {
        if (ByteProp->GetIntPropertyEnum())
        {
            return MakeShared<FJsonValueNumber>(ByteProp->GetPropertyValue(PropertyAddr));
        }
    }

    return MakeShared<FJsonValueString>(TEXT("(unsupported type)"));
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEnvironmentCommands::GetAllPropertiesAsJson(UObject* Object)
{
    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    
    if (!Object)
    {
        return Result;
    }

    for (TFieldIterator<FProperty> PropIt(Object->GetClass()); PropIt; ++PropIt)
    {
        FProperty* Property = *PropIt;
        if (Property->HasAnyPropertyFlags(CPF_Edit | CPF_BlueprintVisible))
        {
            FString PropName = Property->GetName();
            TSharedPtr<FJsonValue> PropValue = GetPropertyAsJsonValue(Object, PropName);
            if (!PropValue->IsNull())
            {
                Result->SetField(PropName, PropValue);
            }
        }
    }

    return Result;
}

UActorComponent* FEpicUnrealMCPEnvironmentCommands::FindActorComponent(AActor* Actor, const FString& ComponentPattern)
{
    if (!Actor || ComponentPattern.IsEmpty())
    {
        return nullptr;
    }

    TArray<UActorComponent*> Components;
    Actor->GetComponents<UActorComponent>(Components);

    for (UActorComponent* Comp : Components)
    {
        if (Comp && Comp->GetClass()->GetName().Contains(ComponentPattern))
        {
            return Comp;
        }
    }

    return nullptr;
}

// ============================================================================
// Generic Actor Management (Refactored)
// ============================================================================
TSharedPtr<FJsonObject> FEpicUnrealMCPEnvironmentCommands::HandleSpawnActor(const TSharedPtr<FJsonObject>& Params)
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

    // Get actor class name (required)
    FString ActorClassName;
    if (!Params->TryGetStringField(TEXT("actor_class"), ActorClassName))
    {
        // Legacy: try "actor_type" 
        if (!Params->TryGetStringField(TEXT("actor_type"), ActorClassName))
        {
            return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'actor_class' parameter"));
        }
    }

    // Find the class
    UClass* ActorClass = FindClassByName(ActorClassName);
    if (!ActorClass)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(
            FString::Printf(TEXT("Could not find actor class: %s"), *ActorClassName));
    }

    // Verify it's an AActor derived class
    if (!ActorClass->IsChildOf(AActor::StaticClass()))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(
            FString::Printf(TEXT("Class %s is not an Actor class"), *ActorClass->GetName()));
    }

    // Get name (optional)
    FString ActorName;
    Params->TryGetStringField(TEXT("name"), ActorName);

    // Get transform
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

    // Spawn the actor
    FActorSpawnParameters SpawnParams;
    if (!ActorName.IsEmpty())
    {
        SpawnParams.Name = FName(*ActorName);
        SpawnParams.NameMode = FActorSpawnParameters::ESpawnActorNameMode::Requested;
    }

    AActor* SpawnedActor = World->SpawnActor<AActor>(ActorClass, Location, Rotation, SpawnParams);

    if (!SpawnedActor)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to spawn actor"));
    }

    SpawnedActor->SetActorScale3D(Scale);

    // Handle special initialization for known actor types
    // This allows spawning a Sphere with automatic mesh assignment
    if (ActorClassName == TEXT("Sphere") || ActorClassName == TEXT("sphere"))
    {
        if (AStaticMeshActor* MeshActor = Cast<AStaticMeshActor>(SpawnedActor))
        {
            UStaticMesh* SphereMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
            if (SphereMesh && MeshActor->GetStaticMeshComponent())
            {
                MeshActor->GetStaticMeshComponent()->SetStaticMesh(SphereMesh);
            }
        }
    }
    else if (ActorClassName == TEXT("Cube") || ActorClassName == TEXT("cube") || ActorClassName == TEXT("Box"))
    {
        if (AStaticMeshActor* MeshActor = Cast<AStaticMeshActor>(SpawnedActor))
        {
            UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
            if (CubeMesh && MeshActor->GetStaticMeshComponent())
            {
                MeshActor->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
            }
        }
    }
    else if (ActorClassName == TEXT("Plane") || ActorClassName == TEXT("plane"))
    {
        if (AStaticMeshActor* MeshActor = Cast<AStaticMeshActor>(SpawnedActor))
        {
            UStaticMesh* PlaneMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
            if (PlaneMesh && MeshActor->GetStaticMeshComponent())
            {
                MeshActor->GetStaticMeshComponent()->SetStaticMesh(PlaneMesh);
            }
        }
    }
    else if (ActorClassName == TEXT("Cylinder"))
    {
        if (AStaticMeshActor* MeshActor = Cast<AStaticMeshActor>(SpawnedActor))
        {
            UStaticMesh* CylinderMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
            if (CylinderMesh && MeshActor->GetStaticMeshComponent())
            {
                MeshActor->GetStaticMeshComponent()->SetStaticMesh(CylinderMesh);
            }
        }
    }
    else if (ActorClassName == TEXT("PostProcessVolume") || ActorClassName == TEXT("post_process_volume"))
    {
        if (APostProcessVolume* PPVolume = Cast<APostProcessVolume>(SpawnedActor))
        {
            PPVolume->bUnbound = true;
            // Initialize default Lookdev settings
            FPostProcessSettings& Settings = PPVolume->Settings;
            Settings.bOverride_AutoExposureMethod = true;
            Settings.AutoExposureMethod = AEM_Manual;
            Settings.bOverride_AutoExposureBias = true;
            Settings.AutoExposureBias = 0.0f;
            Settings.bOverride_BloomIntensity = true;
            Settings.BloomIntensity = 0.0f;
            Settings.bOverride_VignetteIntensity = true;
            Settings.VignetteIntensity = 0.0f;
            Settings.bOverride_AmbientOcclusionIntensity = true;
            Settings.AmbientOcclusionIntensity = 0.0f;
            PPVolume->bEnabled = true;
        }
    }

    // Apply properties from the "properties" object
    const TSharedPtr<FJsonObject>* PropertiesObj;
    if (Params->TryGetObjectField(TEXT("properties"), PropertiesObj))
    {
        TArray<FString> FailedProperties;
        for (const auto& Pair : PropertiesObj->Get()->Values)
        {
            FString Error;
            if (!SetPropertyByName(SpawnedActor, Pair.Key, Pair.Value, Error))
            {
                FailedProperties.Add(Pair.Key + TEXT(": ") + Error);
            }
        }
        
        if (FailedProperties.Num() > 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to set %d properties on spawned actor: %s"), 
                FailedProperties.Num(), *FString::Join(FailedProperties, TEXT("; ")));
        }
    }

    GEditor->RedrawLevelEditingViewports();

    // Build response
    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetBoolField(TEXT("success"), true);
    ResultObj->SetStringField(TEXT("name"), SpawnedActor->GetName());
    ResultObj->SetStringField(TEXT("actor_class"), ActorClass->GetName());
    ResultObj->SetStringField(TEXT("actor_path"), SpawnedActor->GetPathName());

    TSharedPtr<FJsonObject> LocationResult = MakeShared<FJsonObject>();
    LocationResult->SetNumberField(TEXT("x"), Location.X);
    LocationResult->SetNumberField(TEXT("y"), Location.Y);
    LocationResult->SetNumberField(TEXT("z"), Location.Z);
    ResultObj->SetObjectField(TEXT("location"), LocationResult);

    TSharedPtr<FJsonObject> RotationResult = MakeShared<FJsonObject>();
    RotationResult->SetNumberField(TEXT("pitch"), Rotation.Pitch);
    RotationResult->SetNumberField(TEXT("yaw"), Rotation.Yaw);
    RotationResult->SetNumberField(TEXT("roll"), Rotation.Roll);
    ResultObj->SetObjectField(TEXT("rotation"), RotationResult);

    TSharedPtr<FJsonObject> ScaleResult = MakeShared<FJsonObject>();
    ScaleResult->SetNumberField(TEXT("x"), Scale.X);
    ScaleResult->SetNumberField(TEXT("y"), Scale.Y);
    ScaleResult->SetNumberField(TEXT("z"), Scale.Z);
    ResultObj->SetObjectField(TEXT("scale"), ScaleResult);

    return ResultObj;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEnvironmentCommands::HandleDeleteActor(const TSharedPtr<FJsonObject>& Params)
{
    if (!GEditor)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Editor not available"));
    }

    FString ActorName;
    if (!Params->TryGetStringField(TEXT("name"), ActorName))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'name' parameter"));
    }

    UWorld* World = GEditor->GetEditorWorldContext().World();
    if (!World)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("No active world found"));
    }

    int32 DeletedCount = 0;
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        if (It->GetName() == ActorName)
        {
            It->Destroy();
            DeletedCount++;
            break;
        }
    }

    if (DeletedCount == 0)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(
            FString::Printf(TEXT("Actor not found: %s"), *ActorName));
    }

    GEditor->RedrawLevelEditingViewports();

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetBoolField(TEXT("success"), true);
    ResultObj->SetStringField(TEXT("deleted_actor"), ActorName);
    ResultObj->SetNumberField(TEXT("deleted_count"), DeletedCount);

    return ResultObj;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEnvironmentCommands::HandleGetActors(const TSharedPtr<FJsonObject>& Params)
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

    FString FilterClass;
    Params->TryGetStringField(TEXT("actor_class"), FilterClass);

    // Optional: get detailed properties
    bool bDetailed = false;
    Params->TryGetBoolField(TEXT("detailed"), bDetailed);

    TArray<TSharedPtr<FJsonValue>> ActorsArray;
    int32 Count = 0;

    for (TActorIterator<AActor> It(World); It; ++It)
    {
        AActor* Actor = *It;
        
        if (!FilterClass.IsEmpty())
        {
            FString ClassName = Actor->GetClass()->GetName();
            if (!ClassName.Contains(FilterClass) && 
                !ClassName.Contains(TEXT("A") + FilterClass) &&
                !Actor->GetClass()->IsChildOf(FindClassByName(FilterClass)))
            {
                continue;
            }
        }

        TSharedPtr<FJsonObject> ActorObj = MakeShared<FJsonObject>();
        ActorObj->SetStringField(TEXT("name"), Actor->GetName());
        ActorObj->SetStringField(TEXT("class"), Actor->GetClass()->GetName());
        ActorObj->SetStringField(TEXT("path"), Actor->GetPathName());

        FVector Loc = Actor->GetActorLocation();
        TSharedPtr<FJsonObject> LocObj = MakeShared<FJsonObject>();
        LocObj->SetNumberField(TEXT("x"), Loc.X);
        LocObj->SetNumberField(TEXT("y"), Loc.Y);
        LocObj->SetNumberField(TEXT("z"), Loc.Z);
        ActorObj->SetObjectField(TEXT("location"), LocObj);

        if (bDetailed)
        {
            TSharedPtr<FJsonObject> Properties = GetAllPropertiesAsJson(Actor);
            ActorObj->SetObjectField(TEXT("properties"), Properties);
        }

        ActorsArray.Add(MakeShared<FJsonValueObject>(ActorObj));
        Count++;
    }

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetBoolField(TEXT("success"), true);
    ResultObj->SetArrayField(TEXT("actors"), ActorsArray);
    ResultObj->SetNumberField(TEXT("count"), Count);

    return ResultObj;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEnvironmentCommands::HandleSetActorProperties(const TSharedPtr<FJsonObject>& Params)
{
    if (!GEditor)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Editor not available"));
    }

    FString ActorName;
    if (!Params->TryGetStringField(TEXT("name"), ActorName))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'name' parameter"));
    }

    const TSharedPtr<FJsonObject>* PropertiesObj;
    if (!Params->TryGetObjectField(TEXT("properties"), PropertiesObj))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'properties' parameter"));
    }

    UWorld* World = GEditor->GetEditorWorldContext().World();
    if (!World)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("No active world found"));
    }

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

    TArray<FString> ModifiedProperties;
    TArray<FString> FailedProperties;

    for (const auto& Pair : PropertiesObj->Get()->Values)
    {
        FString Error;
        if (SetPropertyByName(TargetActor, Pair.Key, Pair.Value, Error))
        {
            ModifiedProperties.Add(Pair.Key);
        }
        else
        {
            FailedProperties.Add(Pair.Key + TEXT(": ") + Error);
        }
    }

    GEditor->RedrawLevelEditingViewports();

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetBoolField(TEXT("success"), true);
    ResultObj->SetStringField(TEXT("actor_name"), ActorName);
    ResultObj->SetNumberField(TEXT("modified_count"), ModifiedProperties.Num());
    ResultObj->SetNumberField(TEXT("failed_count"), FailedProperties.Num());
    
    TArray<TSharedPtr<FJsonValue>> ModifiedArray;
    for (const FString& Prop : ModifiedProperties)
    {
        ModifiedArray.Add(MakeShared<FJsonValueString>(Prop));
    }
    ResultObj->SetArrayField(TEXT("modified_properties"), ModifiedArray);

    if (FailedProperties.Num() > 0)
    {
        TArray<TSharedPtr<FJsonValue>> FailedArray;
        for (const FString& Fail : FailedProperties)
        {
            FailedArray.Add(MakeShared<FJsonValueString>(Fail));
        }
        ResultObj->SetArrayField(TEXT("failed_properties"), FailedArray);
    }

    return ResultObj;
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEnvironmentCommands::HandleGetActorProperties(const TSharedPtr<FJsonObject>& Params)
{
    if (!GEditor)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Editor not available"));
    }

    FString ActorName;
    if (!Params->TryGetStringField(TEXT("name"), ActorName))
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'name' parameter"));
    }

    // Optional: specific properties to get
    TArray<FString> RequestedProperties;
    const TArray<TSharedPtr<FJsonValue>>* PropsArray;
    if (Params->TryGetArrayField(TEXT("properties"), PropsArray))
    {
        for (const auto& Val : *PropsArray)
        {
            RequestedProperties.Add(Val->AsString());
        }
    }

    UWorld* World = GEditor->GetEditorWorldContext().World();
    if (!World)
    {
        return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("No active world found"));
    }

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

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetBoolField(TEXT("success"), true);
    ResultObj->SetStringField(TEXT("name"), TargetActor->GetName());
    ResultObj->SetStringField(TEXT("class"), TargetActor->GetClass()->GetName());

    // Transform
    FVector Loc = TargetActor->GetActorLocation();
    TSharedPtr<FJsonObject> LocObj = MakeShared<FJsonObject>();
    LocObj->SetNumberField(TEXT("x"), Loc.X);
    LocObj->SetNumberField(TEXT("y"), Loc.Y);
    LocObj->SetNumberField(TEXT("z"), Loc.Z);
    ResultObj->SetObjectField(TEXT("location"), LocObj);

    FRotator Rot = TargetActor->GetActorRotation();
    TSharedPtr<FJsonObject> RotObj = MakeShared<FJsonObject>();
    RotObj->SetNumberField(TEXT("pitch"), Rot.Pitch);
    RotObj->SetNumberField(TEXT("yaw"), Rot.Yaw);
    RotObj->SetNumberField(TEXT("roll"), Rot.Roll);
    ResultObj->SetObjectField(TEXT("rotation"), RotObj);

    FVector Scale = TargetActor->GetActorScale3D();
    TSharedPtr<FJsonObject> ScaleObj = MakeShared<FJsonObject>();
    ScaleObj->SetNumberField(TEXT("x"), Scale.X);
    ScaleObj->SetNumberField(TEXT("y"), Scale.Y);
    ScaleObj->SetNumberField(TEXT("z"), Scale.Z);
    ResultObj->SetObjectField(TEXT("scale"), ScaleObj);

    // Properties
    TSharedPtr<FJsonObject> PropertiesObj = MakeShared<FJsonObject>();
    
    if (RequestedProperties.Num() > 0)
    {
        // Get specific properties
        for (const FString& PropName : RequestedProperties)
        {
            TSharedPtr<FJsonValue> PropValue = GetPropertyAsJsonValue(TargetActor, PropName);
            if (!PropValue->IsNull())
            {
                PropertiesObj->SetField(PropName, PropValue);
            }
            else
            {
                // Try component properties
                UActorComponent* Comp = FindActorComponent(TargetActor, PropName);
                if (Comp)
                {
                    PropertiesObj->SetObjectField(PropName, GetAllPropertiesAsJson(Comp));
                }
            }
        }
    }
    else
    {
        // Get all editable properties
        PropertiesObj = GetAllPropertiesAsJson(TargetActor);
    }
    
    ResultObj->SetObjectField(TEXT("properties"), PropertiesObj);

    return ResultObj;
}

// ============================================================================
// Legacy Compatibility (Deprecated)
// ============================================================================
TSharedPtr<FJsonObject> FEpicUnrealMCPEnvironmentCommands::HandleCreateLight(const TSharedPtr<FJsonObject>& Params)
{
    // Convert to spawn_actor
    FString LightType;
    if (Params->TryGetStringField(TEXT("light_type"), LightType))
    {
        TSharedPtr<FJsonObject> NewParams = MakeShared<FJsonObject>(*Params);
        NewParams->SetStringField(TEXT("actor_class"), LightType);
        return HandleSpawnActor(NewParams);
    }
    return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("create_light is deprecated, use spawn_actor with actor_class"));
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEnvironmentCommands::HandleSetLightProperties(const TSharedPtr<FJsonObject>& Params)
{
    return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("set_light_properties is deprecated, use set_actor_properties instead"));
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEnvironmentCommands::HandleGetLights(const TSharedPtr<FJsonObject>& Params)
{
    TSharedPtr<FJsonObject> NewParams = MakeShared<FJsonObject>(*Params);
    NewParams->SetStringField(TEXT("actor_class"), TEXT("Light"));
    return HandleGetActors(NewParams);
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEnvironmentCommands::HandleDeleteLight(const TSharedPtr<FJsonObject>& Params)
{
    return HandleDeleteActor(Params);
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEnvironmentCommands::HandleCreatePostProcessVolume(const TSharedPtr<FJsonObject>& Params)
{
    TSharedPtr<FJsonObject> NewParams = MakeShared<FJsonObject>(*Params);
    NewParams->SetStringField(TEXT("actor_class"), TEXT("PostProcessVolume"));
    return HandleSpawnActor(NewParams);
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEnvironmentCommands::HandleSetPostProcessSettings(const TSharedPtr<FJsonObject>& Params)
{
    return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("set_post_process_settings is deprecated, use set_actor_properties instead"));
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEnvironmentCommands::HandleSpawnBasicActor(const TSharedPtr<FJsonObject>& Params)
{
    // Convert actor_type to actor_class
    FString ActorType;
    if (Params->TryGetStringField(TEXT("actor_type"), ActorType))
    {
        TSharedPtr<FJsonObject> NewParams = MakeShared<FJsonObject>(*Params);
        NewParams->SetStringField(TEXT("actor_class"), ActorType);
        return HandleSpawnActor(NewParams);
    }
    return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("spawn_basic_actor is deprecated, use spawn_actor with actor_class"));
}

TSharedPtr<FJsonObject> FEpicUnrealMCPEnvironmentCommands::HandleSetActorMaterial(const TSharedPtr<FJsonObject>& Params)
{
    return FEpicUnrealMCPCommonUtils::CreateErrorResponse(TEXT("set_actor_material is deprecated, use set_actor_properties with material property instead"));
}
