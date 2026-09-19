using UnrealBuildTool;

public class PinkCabVehicle : ModuleRules
{
    public PinkCabVehicle(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "ChaosVehicles", "PhysicsCore", "PinkCabCore" });
    }
}
