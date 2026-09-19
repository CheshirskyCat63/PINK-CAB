using UnrealBuildTool;

public class PinkCabEconomy : ModuleRules
{
    public PinkCabEconomy(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "PinkCabCore" });
    }
}
