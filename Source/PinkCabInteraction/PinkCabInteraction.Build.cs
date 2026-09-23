using UnrealBuildTool;

public class PinkCabInteraction : ModuleRules
{
    public PinkCabInteraction(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "PinkCabCore" });
    }
}
