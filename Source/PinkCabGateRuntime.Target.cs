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

        // Installed UE builds cannot use TargetBuildEnvironment.Unique. UBT's
        // supported installed-engine escape hatch is bOverrideBuildEnvironment;
        // the target remains monolithic so project runtime code is linked into
        // the executable instead of loaded as UnrealEditor project DLLs.
        // Dedicated development runtime used by CI/human gates. It executes the
        // exact uncooked project Content tree without hosting project code inside
        // UnrealEditor, so Windows Smart App Control never needs to trust/load
        // UnrealEditor-PinkCab*.dll for the fast development lane.
        bOverrideBuildEnvironment = true;
        LinkType = TargetLinkType.Monolithic;
        bBuildRequiresCookedData = false;
        // Keep this target deliberately runtime-only. Editor/developer tool flags
        // pull UnrealEd/MovieSceneTools into a Game target and defeat the SAC-safe
        // separation. Runtime acceptance uses production telemetry instead.
        bBuildWithEditorOnlyData = false;
    }
}
