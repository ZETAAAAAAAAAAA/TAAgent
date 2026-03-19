// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class AssetValidation : ModuleRules
{
	public AssetValidation(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"Niagara",
				"InputCore",
				"ApplicationCore",
				"RenderCore",
				"RHI",
				"ImageWrapper"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"UnrealEd",
				"Slate",
				"SlateCore",
				"EditorStyle",
				"AssetRegistry",
				"AssetTools",
				"ToolMenus",
				"LevelEditor",
				"Projects"
			}
		);
	}
}
