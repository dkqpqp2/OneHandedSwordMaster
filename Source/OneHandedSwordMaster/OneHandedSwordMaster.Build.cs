// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class OneHandedSwordMaster : ModuleRules
{
	public OneHandedSwordMaster(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
	}
}
