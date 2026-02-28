// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class OneHandedSwordMaster : ModuleRules
{
	public OneHandedSwordMaster(ReadOnlyTargetRules Target) : base(Target)
	{
		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", 
			"CoreUObject", 
			"Engine",
			"InputCore", 
			"EnhancedInput", 
			"AIModule",
			"GameplayTasks",
			"NavigationSystem"
		});
	}
}
