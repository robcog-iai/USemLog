// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

using UnrealBuildTool;

public class USemLog : ModuleRules
{
	// Set the given preprocessor as a public definition with  0 or 1 (check as private and public module)
	private void SetDependencyPreprocessorDefinition(string ModuleName, string PreprocessorDefinition)
	{
		string Result = PrivateDependencyModuleNames.Find(DependencyName => DependencyName.Equals(ModuleName));
		if (string.IsNullOrEmpty(Result))
		{
			Result = PublicDependencyModuleNames.Find(DependencyName => DependencyName.Equals(ModuleName));
			if (string.IsNullOrEmpty(Result))
			{
				PublicDefinitions.Add(PreprocessorDefinition + "=0");
			}
            else
            {
				PublicDefinitions.Add(PreprocessorDefinition + "=1");
			}
			
		}
		else
		{
			PublicDefinitions.Add(PreprocessorDefinition + "=1");
		}
	}
	
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
                "UROSBridge",               // SL_WITH_ROSBRIDGE
				//"MongoC",					// SL_WITH_LIBMONGO_C
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
				"Landscape", "AIModule", // whitelisted actors when setting the world to visual only
				"UTags",
				"UIds",
				"UConversions",			// SL_WITH_ROS_CONVERSIONS
				"UMCGrasp",				// SL_WITH_MC_GRASP
				"USlicingLogic",		    // SL_WITH_SLICING	
                "UROSBridge",               // SL_WITH_ROSBRIDGE
				//"MongoCxx",			    // SL_WITH_LIBMONGO_CXX
				//"MongoC",					// SL_WITH_LIBMONGO_C	// !!! Needs to be in the PublicDependencyModuleNames
				//"SRanipal",			    // SL_WITH_EYE_TRACKING
				//"Boost",				    // SL_WITH_BOOST
				//"UViz", "UMongoQA",	    // SL_WITH_DATA_VIS
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

		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

		// Check included dependencies and set preprocessor flags accordingly
		SetDependencyPreprocessorDefinition("UConversions", "SL_WITH_ROS_CONVERSIONS");
		SetDependencyPreprocessorDefinition("UMCGrasp", "SL_WITH_MC_GRASP");
		SetDependencyPreprocessorDefinition("MongoC", "SL_WITH_LIBMONGO_C");
		SetDependencyPreprocessorDefinition("MongoCxx", "SL_WITH_LIBMONGO_CXX");
		SetDependencyPreprocessorDefinition("SRanipal", "SL_WITH_EYE_TRACKING");
		SetDependencyPreprocessorDefinition("USlicingLogic", "SL_WITH_SLICING");
        SetDependencyPreprocessorDefinition("UROSBridge", "SL_WITH_ROSBRIDGE");
		
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
		
		string UViz = PrivateDependencyModuleNames.Find(DependencyName => DependencyName.Equals("UViz"));
		string UMongoQA = PrivateDependencyModuleNames.Find(DependencyName => DependencyName.Equals("UMongoQA"));
		if (string.IsNullOrEmpty(UViz) || string.IsNullOrEmpty(UMongoQA))
		{
			PublicDefinitions.Add("SL_WITH_DATA_VIS=0");
		}
		else
		{
			PublicDefinitions.Add("SL_WITH_DATA_VIS=1");
		}
	}
}
