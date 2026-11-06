using UnrealBuildTool;

public class LastDanceEditor : ModuleRules
{
	public LastDanceEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
			"LastDance",
			"AssetDefinition",
			"GraphEditor",
			"PropertyEditor",
			"Slate",
			"SlateCore",
			"UnrealEd"
		});
	}
}
