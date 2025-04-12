// Some copyright should be here...

using UnrealBuildTool;

public class SlimeMold : ModuleRules
{
	public SlimeMold(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"InputCore",
			}
			);
	}
}
