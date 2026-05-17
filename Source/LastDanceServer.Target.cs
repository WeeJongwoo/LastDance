
using UnrealBuildTool;
using System.Collections.Generic;

public class LastDanceServerTarget : TargetRules
{
    public LastDanceServerTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Server;
        DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_4;
		ExtraModuleNames.Add("LastDance");
    }
}