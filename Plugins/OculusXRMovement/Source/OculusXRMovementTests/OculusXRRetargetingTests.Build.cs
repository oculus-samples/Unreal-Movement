using UnrealBuildTool;

public class OculusXRRetargetingTests : ModuleRules
{
    public OculusXRRetargetingTests(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PrivateDependencyModuleNames.AddRange(
            new[]
            {
                "Core",
                "OculusXRRetargeting",
                "OculusXRMovement"
            }
        );
    }
}
