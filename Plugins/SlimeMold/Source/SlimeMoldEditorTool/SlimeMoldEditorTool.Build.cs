// Some copyright should be here...

using UnrealBuildTool;

public class SlimeMoldEditorTool : ModuleRules
{
	public SlimeMoldEditorTool(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
            }
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"EditorFramework",
				"EditorStyle",
				"UnrealEd",
				"InteractiveToolsFramework",
				"EditorInteractiveToolsFramework",
				"SlimeMold",
			}
			);

	}
}
