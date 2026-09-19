using UnrealBuildTool;

public class PinkCabPersistence : ModuleRules
{
    public PinkCabPersistence(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "PinkCabCore", "PinkCabVehicle", "PinkCabTaxi", "PinkCabEconomy", "PinkCabWorld", "PinkCabTraffic" });
    }
}
