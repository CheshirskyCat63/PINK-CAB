using UnrealBuildTool;
using System.Collections.Generic;

public class PinkCabTarget : TargetRules
{
    public PinkCabTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V7;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        ExtraModuleNames.AddRange(new string[] { "PinkCab", "PinkCabCore", "PinkCabInteraction", "PinkCabVehicle", "PinkCabEconomy", "PinkCabWorld", "PinkCabTraffic", "PinkCabTaxi", "PinkCabPersistence" });
    }
}
