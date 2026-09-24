/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

using UnrealBuildTool;

public class OculusXRMovementUtilityEditor : ModuleRules
{
    public OculusXRMovementUtilityEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "OculusXRMovementUtility",
                "Engine",
                "Slate",
                "SlateCore",
                "InputCore",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "AnimGraph",
                "AnimGraphRuntime",
                "BlueprintGraph",
                "UnrealEd",
                "DesktopPlatform",
                "PropertyEditor",
            }
        );
    }
}
