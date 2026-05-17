
using UnrealBuildTool;
using System.Collections.Generic;

public class LastDanceClientTarget : TargetRules
{
    public LastDanceClientTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Client;
        DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_4;
		ExtraModuleNames.Add("LastDance");
    }
}