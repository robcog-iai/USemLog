// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

using UnrealBuildTool;

public class USemLog : ModuleRules
{
    // Set to false if you don't need eye tracking
    private bool WithEyeTracking { get { return true; } }

    public USemLog(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		//PrivatePCHHeaderFile = "Public/USemLog.h";
		//bEnforceIWYU = false;

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
				//"Json",
				//"JsonUtilities",
				"UTags",
				"UIds",
				"UConversions",
				"UMCGrasp",				// SL_WITH_MC_GRASP
				//"libmongo",
				//"SlicingLogic",		// SL_WITH_SLICING
				"MongoC",				// SL_WITH_LIBMONGO_C
				//"MongoCxx",			// SL_WITH_LIBMONGO_CXX
				"SRanipal",				// SL_WITH_EYE_TRACKING
				//"Boost",				// SL_WITH_BOOST
				// ... add private dependencies that you statically link with here ...
			}
			);
		
		// Avoiding depending on the editor when packaging
		if(Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"UnrealEd",
				}
				);
		}

		//// SL Vision currently only works in developer mode
		//// (https://docs.unrealengine.com/en-us/Programming/UnrealBuildSystem/TargetFiles)
		//if (Target.Type == TargetRules.TargetType.Editor)
		////if(Target.Type == TargetRules.TargetType.Program)
		//{
		//	PrivateDependencyModuleNames.Add("USemLogVision");
		//	PublicDefinitions.Add("SL_WITH_SLVIS=1");
		//}
		//else
		//{
		//	PublicDefinitions.Add("SL_WITH_SLVIS=0");
		//}

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

		string MongoC = PrivateDependencyModuleNames.Find(DependencyName => DependencyName.Equals("MongoC"));
		if (string.IsNullOrEmpty(MongoC))
		{
			PublicDefinitions.Add("SL_WITH_LIBMONGO_C=0");
		}
		else
		{
			PublicDefinitions.Add("SL_WITH_LIBMONGO_C=1");

			// Needed to ignore various warnings from libmongo
			bEnableUndefinedIdentifierWarnings = false;
			bEnableExceptions = true;
			//bUseRTTI = true;
		}

		string MongoCxx = PrivateDependencyModuleNames.Find(DependencyName => DependencyName.Equals("MongoCxx"));
		if (string.IsNullOrEmpty(MongoCxx))
		{
			PublicDefinitions.Add("SL_WITH_LIBMONGO_CXX=0");
		}
		else
		{
			PublicDefinitions.Add("SL_WITH_LIBMONGO_CXX=1");

			// Needed to ignore various warnings from libmongo
			bEnableUndefinedIdentifierWarnings = false;
			bEnableExceptions = true;
			//bUseRTTI = true;
		}

		string SRanipal = PrivateDependencyModuleNames.Find(DependencyName => DependencyName.Equals("SRanipal"));
		if (string.IsNullOrEmpty(SRanipal))
		{
			PublicDefinitions.Add("SL_WITH_EYE_TRACKING=0");
		}
		else
		{
			PublicDefinitions.Add("SL_WITH_EYE_TRACKING=1");
		}

		string SlicingLogic = PrivateDependencyModuleNames.Find(DependencyName => DependencyName.Equals("SlicingLogic"));
		if (string.IsNullOrEmpty(SlicingLogic))
		{
			PublicDefinitions.Add("SL_WITH_SLICING=0");
		}
		else
		{
			PublicDefinitions.Add("SL_WITH_SLICING=1");
		}
		
		string Boost = PrivateDependencyModuleNames.Find(DependencyName => DependencyName.Equals("Boost"));
		if (string.IsNullOrEmpty(Boost))
		{
			PublicDefinitions.Add("VIZ_WITH_BOOST=0");
		}
		else
		{
			PublicDefinitions.Add("VIZ_WITH_BOOST=1");
		}
		
		string Json = PrivateDependencyModuleNames.Find(DependencyName => DependencyName.Equals("Json"));
		string JsonUtil = PrivateDependencyModuleNames.Find(DependencyName => DependencyName.Equals("JsonUtilities"));
		if (string.IsNullOrEmpty(Json) || string.IsNullOrEmpty(JsonUtil))
		{
			PublicDefinitions.Add("SL_WITH_JSON=0");
		}
		else
		{
			PublicDefinitions.Add("SL_WITH_JSON=1");
		}
	}
}
