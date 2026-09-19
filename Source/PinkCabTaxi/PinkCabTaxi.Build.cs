using UnrealBuildTool;

public class PinkCabTaxi : ModuleRules
{
    public PinkCabTaxi(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "PinkCabCore", "PinkCabEconomy", "PinkCabVehicle", "PinkCabWorld" });
    }
}
