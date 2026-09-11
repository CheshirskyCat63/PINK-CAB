using UnrealBuildTool;
using System.Collections.Generic;

public class PinkCabEditorTarget : TargetRules
{
    public PinkCabEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V7;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        ExtraModuleNames.AddRange(new string[] { "PinkCab", "PinkCabTests" });
    }
}
