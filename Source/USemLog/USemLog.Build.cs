// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

using UnrealBuildTool;

public class USemLog : ModuleRules
{
	public USemLog(ReadOnlyTargetRules Target) : base(Target)
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
				"USemLogOwl",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"Json",
				"JsonUtilities",
				"UTags",
				"UIds",
				"UConversions",
				"UMCGrasp",			// SL_WITH_MC_GRASP
				"libmongo"			// SL_WITH_LIBMONGO
				// ... add private dependencies that you statically link with here ...	
			}
			);

		// TODO
		// SL Vision currently only works in developer mode
		// (https://docs.unrealengine.com/en-us/Programming/UnrealBuildSystem/TargetFiles)
		if (Target.Type == TargetRules.TargetType.Editor)
		//if(Target.Type == TargetRules.TargetType.Program)
		{
			PrivateDependencyModuleNames.Add("USemLogVision");			
			PublicDefinitions.Add("SL_WITH_SLVIS=1");
		}
		else
		{
			PublicDefinitions.Add("SL_WITH_SLVIS=0");
		}

		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

		// Check included dependencies and set preprocessor flags accordingly
		string UMCGrasp = PrivateDependencyModuleNames.Find(DependencyName => DependencyName.Equals("UMCGrasp"));
		if (string.IsNullOrEmpty(UMCGrasp))
		{
			PublicDefinitions.Add("SL_WITH_MC_GRASP=0");
		}
		else
		{
			PublicDefinitions.Add("SL_WITH_MC_GRASP=1");
		}

		string libmongo = PrivateDependencyModuleNames.Find(DependencyName => DependencyName.Equals("libmongo"));
		if (string.IsNullOrEmpty(libmongo))
		{
			PublicDefinitions.Add("SL_WITH_LIBMONGO=0");
		}
		else
		{
			PublicDefinitions.Add("SL_WITH_LIBMONGO=0");
			
			// Needed to ignore various warnings from libmongo
			bEnableUndefinedIdentifierWarnings = false;
			bEnableExceptions = true;
		}
	}
}
