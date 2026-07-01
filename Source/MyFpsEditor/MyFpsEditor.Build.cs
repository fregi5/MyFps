// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MyFpsEditor : ModuleRules
{
	public MyFpsEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"UMG",
			"Blutility",
			"MyFps"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Slate",
			"SlateCore",
			"UnrealEd"
		});

		PublicIncludePaths.AddRange(new string[]
		{
			"MyFps"
		});
	}
}
