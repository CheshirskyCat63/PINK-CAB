using UnrealBuildTool;

public class PinkCabWorld : ModuleRules
{
    public PinkCabWorld(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "PinkCabCore" });
    }
}
