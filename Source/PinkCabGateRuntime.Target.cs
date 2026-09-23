using UnrealBuildTool;
using System.Collections.Generic;

public class PinkCabGateRuntimeTarget : TargetRules
{
    public PinkCabGateRuntimeTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V7;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
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
            "PinkCabPersistence"
        });

        // Dedicated development runtime used by CI/human gates. It executes the
        // exact uncooked project Content tree without hosting project code inside
        // UnrealEditor, so Windows Smart App Control never needs to trust/load
        // UnrealEditor-PinkCab*.dll for the fast development lane.
        BuildEnvironment = TargetBuildEnvironment.Unique;
        LinkType = TargetLinkType.Monolithic;
        bBuildRequiresCookedData = false;
        bBuildWithEditorOnlyData = true;
        bBuildDeveloperTools = true;
        bForceBuildTargetPlatforms = true;
        bForceBuildShaderFormats = true;
        bForceCompileDevelopmentAutomationTests = true;
    }
}
