using UnrealBuildTool;

public class InkPot : ModuleRules
{
	public InkPot(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[] 
            {
			}
		);
		
		PrivateIncludePaths.AddRange(
			new string[] 
            {
			}
		);
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
                "Core",
                "CoreUObject",
                "Engine",

                "Settings",

                "InkPlusPlus",
            }
        );
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
            }
        );
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
			);
		
		if (Target.bBuildEditor == true)
		{
			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"UnrealEd",
				}
			);
		}
	}
}