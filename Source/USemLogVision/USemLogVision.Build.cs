// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

using UnrealBuildTool;

public class USemLogVision : ModuleRules
{
	public USemLogVision(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"RHI",
				"RenderCore",
				"UnrealEd",
				"UTags",
				"libmongo", //SLVIS_WITH_LIBMONGO
				//"MongoC",
				//"MongoCxx",
				// ... add private dependencies that you statically link with here ...	
			}
			);


		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

		string libmongo = PrivateDependencyModuleNames.Find(DependencyName => DependencyName.Equals("libmongo"));
		if (string.IsNullOrEmpty(libmongo))
		{
			PublicDefinitions.Add("SLVIS_WITH_LIBMONGO=0");
		}
		else
		{
			PublicDefinitions.Add("SLVIS_WITH_LIBMONGO=1");

			// Needed to ignore various warnings from libmongo
			bEnableUndefinedIdentifierWarnings = false;
			bEnableExceptions = true;
			//bUseRTTI = true;
		}
	}
}
