// Copyright Epic Games, Inc. All Rights Reserved.


using System.IO;
using UnrealBuildTool;

public class MetaMovementSDK_Utility : ModuleRules
{
    public MetaMovementSDK_Utility(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PrivateIncludePaths.AddRange(
            new string[] {
		        // ... add other private include paths required here ...
		        Path.Combine(ModuleDirectory, "Private")
            }
        );

        PrivateIncludePaths.AddRange(
            new string[] {
		        // ... add other private include paths required here ...
		        Path.Combine(ModuleDirectory, "Public")
            }
        );

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
				// ... add other public dependencies that you statically link with here ...
			}
            );


        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
				// ... add private dependencies that you statically link with here ...
			}
            );


        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
				// ... add any modules that your module loads dynamically here ...
			}
            );

        string PluginRoot = Path.GetFullPath(Path.Combine(ModuleDirectory, "..", ".."));

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            string DllPath = Path.Combine(PluginRoot, "Binaries", "Win64", "MetaMovementSDK_Utility.dll");
            string LibPath = Path.Combine(PluginRoot, "Binaries", "Win64", "MetaMovementSDK_Utility.lib");

            PublicAdditionalLibraries.Add(LibPath);
            RuntimeDependencies.Add(DllPath);
            PublicDelayLoadDLLs.Add("MetaMovementSDK_Utility.dll");
        }

        if (Target.Platform == UnrealTargetPlatform.Android)
        {
            string AndroidLibPath = Path.Combine(PluginRoot, "Binaries", "Android", "libs", "arm64-v8a");
            PublicRuntimeLibraryPaths.Add(AndroidLibPath);
            PublicAdditionalLibraries.Add(Path.Combine(AndroidLibPath, "libMetaMovementSDK_Utility.so"));

            // MetaMovementSDK_Utility.xml is responsible for copying over the .so library
            AdditionalPropertiesForReceipt.Add("AndroidPlugin",
                Path.Combine(PluginRoot, "MetaMovementSDK_Utility.xml"));
        }

        if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            string MacArchFolder = Target.Architecture == UnrealArch.Arm64 ? "Mac-ARM" : "Mac-x86-64";
            string DylibPath = Path.Combine(PluginRoot, "Binaries", MacArchFolder, "MetaMovementSDK_Utility.dylib");

            PublicAdditionalLibraries.Add(DylibPath);
            RuntimeDependencies.Add(DylibPath);
            PublicDelayLoadDLLs.Add("MetaMovementSDK_Utility.dylib");
        }

        if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            string LinuxLibPath = Path.Combine(PluginRoot, "Binaries", "Linux");
            string SoPath = Path.Combine(LinuxLibPath, "MetaMovementSDK_Utility.so");

            PublicRuntimeLibraryPaths.Add(LinuxLibPath);
            PublicAdditionalLibraries.Add(SoPath);
            RuntimeDependencies.Add(SoPath);
        }
    }
}
