// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class UnrealMCP : ModuleRules
{
	public UnrealMCP(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDefinitions.Add("UNREALMCP_EXPORTS=1");

		PublicIncludePaths.AddRange(
			new string[] {
				System.IO.Path.Combine(ModuleDirectory, "Public"),
				System.IO.Path.Combine(ModuleDirectory, "Public/Commands"),
				System.IO.Path.Combine(ModuleDirectory, "Public/Commands/BlueprintGraph"),
				System.IO.Path.Combine(ModuleDirectory, "Public/Commands/BlueprintGraph/Nodes")
			}
		);

		PrivateIncludePaths.AddRange(
			new string[] {
				System.IO.Path.Combine(ModuleDirectory, "Private"),
				System.IO.Path.Combine(ModuleDirectory, "Private/Commands"),
				System.IO.Path.Combine(ModuleDirectory, "Private/Commands/BlueprintGraph"),
				System.IO.Path.Combine(ModuleDirectory, "Private/Commands/BlueprintGraph/Nodes")
			}
		);
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"InputCore",
				"Networking",
				"Sockets",
				"HTTP",
				"Json",
				"JsonUtilities",
				"DeveloperSettings",
				"PhysicsCore",
				"UnrealEd",           // For Blueprint editing
				"BlueprintGraph",     // For K2Node classes (F15-F22)
				"KismetCompiler",     // For Blueprint compilation (F15-F22)
				"Niagara",            // For Niagara particle systems
				"NiagaraShader"       // For Stateless Niagara (NiagaraStatelessSimulationShader.h)
			}
		);
		
		// UE 5.7+ Stateless Niagara support - add Internal include paths
		// NiagaraStatelessSimulationShader.h is in NiagaraShader/Internal/Stateless/
		string NiagaraShaderPath = System.IO.Path.Combine(ModuleDirectory, "..", "..", "..", "..", "..", "..", "Engine", "Plugins", "FX", "Niagara", "Source", "NiagaraShader");
		if (!System.IO.Directory.Exists(NiagaraShaderPath))
		{
			NiagaraShaderPath = "E:/UE/UE_5.7/Engine/Plugins/FX/Niagara/Source/NiagaraShader";
		}
		string NiagaraPath = System.IO.Path.Combine(ModuleDirectory, "..", "..", "..", "..", "..", "..", "Engine", "Plugins", "FX", "Niagara", "Source", "Niagara");
		if (!System.IO.Directory.Exists(NiagaraPath))
		{
			NiagaraPath = "E:/UE/UE_5.7/Engine/Plugins/FX/Niagara/Source/Niagara";
		}
		
		// Add Internal include paths for Stateless API
		if (System.IO.Directory.Exists(System.IO.Path.Combine(NiagaraShaderPath, "Internal")))
		{
			PublicIncludePaths.Add(System.IO.Path.Combine(NiagaraShaderPath, "Internal"));
		}
		if (System.IO.Directory.Exists(System.IO.Path.Combine(NiagaraPath, "Internal")))
		{
			PublicIncludePaths.Add(System.IO.Path.Combine(NiagaraPath, "Internal"));
		}
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"EditorScriptingUtilities",
				"EditorSubsystem",
				"Slate",
				"SlateCore",
				"Kismet",
				"Projects",
				"AssetRegistry",
				"MeshDescription",       // For FMeshDescription
				"StaticMeshDescription", // For FStaticMeshAttributes
				"Landscape",             // For UMaterialExpressionLandscapeLayerBlend
				"ImageWrapper",          // For image encoding (PNG/JPEG)
				"LevelEditor"            // For editor viewport access
			}
		);
		
		if (Target.bBuildEditor == true)
		{
			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"PropertyEditor",      // For property editing
					"ToolMenus",           // For editor UI
					"BlueprintEditorLibrary", // For Blueprint utilities
					"MaterialEditor",      // For material creation and editing
					"NiagaraEditor"        // For Niagara asset editing (AddEmitterToSystem)
				}
			);


		}
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
		);
	}
} 