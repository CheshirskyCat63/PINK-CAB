using UnrealBuildTool;
using System.Collections.Generic;

public class PinkCabCommandletTarget : TargetRules
{
    public PinkCabCommandletTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V7;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        bOverrideBuildEnvironment = true;
        LinkType = TargetLinkType.Monolithic;
        ExtraModuleNames.AddRange(new string[]
        {
            "PinkCab",
            "PinkCabCore",
            "PinkCabInteraction",
            "PinkCabVehicle",
            "PinkCabEconomy",
            "PinkCabWorld",
            "PinkCabTraffic",
            "PinkCabTaxi",
            "PinkCabPersistence",
            "PinkCabTests"
        });
    }
}
