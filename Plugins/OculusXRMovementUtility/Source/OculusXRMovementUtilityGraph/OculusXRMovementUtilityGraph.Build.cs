/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

using UnrealBuildTool;

public class OculusXRMovementUtilityGraph : ModuleRules
{
    public OculusXRMovementUtilityGraph(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "OculusXRMovementUtility",
                "OculusXRMovement",
                "MetaMovementSDK_Utility",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "AnimGraph",
                "AnimGraphRuntime",
                "BlueprintGraph",
                "Slate",
                "SlateCore",
                "DesktopPlatform",
                "UnrealEd",
                "AssetTools",
                "KismetCompiler",
            }
        );
    }
}
