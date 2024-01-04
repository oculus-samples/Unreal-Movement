// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class OculusXRRetargeting : ModuleRules
{
	public OculusXRRetargeting(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"IKRig",
				"OculusXRMovement"
			}
			);


		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
			}
			);
	}
}
