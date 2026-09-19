using UnrealBuildTool;

public class PinkCabTests : ModuleRules
{
    public PinkCabTests(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Core", "CoreUObject", "ApplicationCore", "Engine", "InputCore", "UnrealEd", "PinkCab",
            "ChaosVehicles", "PhysicsCore"
        });
    }
}
