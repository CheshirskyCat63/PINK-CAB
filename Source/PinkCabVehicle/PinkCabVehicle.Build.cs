using UnrealBuildTool;

public class PinkCabVehicle : ModuleRules
{
    public PinkCabVehicle(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivateDependencyModuleNames.Add("Chaos");
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "ChaosVehicles", "ChaosVehiclesCore", "PhysicsCore", "PinkCabCore" });
    }
}
