// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

using UnrealBuildTool;

public class USemLog : ModuleRules
{
	public USemLog(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		bEnableUndefinedIdentifierWarnings = false;

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
				"UMCGrasp",			// WITH_MC_GRASP
				//"libmongo"		// WITH_LIBMONGO , TODO 4.20 has issues with libmongo
				// ... add private dependencies that you statically link with here ...	
			}
			);

		// SL Vision currently works in developer mode (https://docs.unrealengine.com/en-us/Programming/UnrealBuildSystem/TargetFiles)
		if(Target.Type == TargetRules.TargetType.Editor)
		//if(Target.Type == TargetRules.TargetType.Program)
		{
			PrivateDependencyModuleNames.Add("USemLogVision");			
			PublicDefinitions.Add("WITH_SL_VIS=1");
		}

		// 4.20 has issues with libmongo, this flag will ignore the mongo code
		PublicDefinitions.Add("WITH_LIBMONGO=0");

		// Include listening for physics based movement events
		PublicDefinitions.Add("WITH_MC_GRASP=1");

		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
