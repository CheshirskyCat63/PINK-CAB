using UnrealBuildTool;

public class PinkCabCore : ModuleRules
{
    public PinkCabCore(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new string[] { "Core" });
    }
}
