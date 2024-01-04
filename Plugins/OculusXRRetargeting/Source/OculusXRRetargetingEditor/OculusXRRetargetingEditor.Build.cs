/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

using UnrealBuildTool;

public class OculusXRRetargetingEditor : ModuleRules
{
	public OculusXRRetargetingEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"OculusXRRetargeting", "Core", "CoreUObject", "Engine", "InputCore", "IKRig", "IKRigDeveloper", "IKRigEditor",
				"AnimGraph", "AnimGraphRuntime", "OculusXRMovement"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"UnrealEd",
				"AnimGraph",
				"AnimGraphRuntime",
				"BlueprintGraph",
			}
		);
	}
}
