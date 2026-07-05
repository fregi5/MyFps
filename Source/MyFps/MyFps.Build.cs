// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MyFps : ModuleRules
{
	public MyFps(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.Add(ModuleDirectory);

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "AIModule", "NavigationSystem", "UMG", "Sockets", "Niagara", "PhysicsCore" });
		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore", "Blutility" });
		PrivateIncludePaths.AddRange(new string[] { System.IO.Path.GetFullPath(Target.RelativeEnginePath) + "Source/Editor/Blutility/Private" });

	}
}
