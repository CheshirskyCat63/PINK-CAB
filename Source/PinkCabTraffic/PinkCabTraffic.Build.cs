using UnrealBuildTool;

public class PinkCabTraffic : ModuleRules
{
    public PinkCabTraffic(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "PinkCabCore", "PinkCabWorld" });
    }
}
