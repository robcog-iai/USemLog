// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

using UnrealBuildTool;

public class USemLog : ModuleRules
{
	// Set the given preprocessor as a public definition with  0 or 1 (check as private and public module)
	private void SetDependencyPrepreocessorDefinition(string ModuleName, string PreprocessorDefinition)
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
				"MongoC",						// SL_WITH_LIBMONGO_C                
				//"UProtobuf", 					// SL_WITH_PROTO
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
				"Landscape",
				"WebSockets",
				"CinematicCamera",
				//"Landscape", "AIModule",	// whitelisted actors when setting the world to visual only
				//"UConversions",				// SL_WITH_ROS_CONVERSIONS
				"UMCGrasp",					// SL_WITH_MC_GRASP
				//"SRanipal",					// SL_WITH_EYE_TRACKING
				//"SlicingLogic",		    // SL_WITH_SLICING				
				//"MongoCxx",			    // SL_WITH_LIBMONGO_CXX				
				//"Boost",				    // SL_WITH_BOOST			
				// ... add private dependencies that you statically link with here ...
			}
			);

		// Avoiding depending on the editor when packaging
		if (Target.bBuildEditor)
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

		// Enable/disable various debug functions throughout the code
		PublicDefinitions.Add("SL_WITH_DEBUG=1");

		// Check included dependencies and set preprocessor flags accordingly
		SetDependencyPrepreocessorDefinition("UConversions", "SL_WITH_ROS_CONVERSIONS");
		SetDependencyPrepreocessorDefinition("UMCGrasp", "SL_WITH_MC_GRASP");
		SetDependencyPrepreocessorDefinition("MongoC", "SL_WITH_LIBMONGO_C");
		SetDependencyPrepreocessorDefinition("MongoCxx", "SL_WITH_LIBMONGO_CXX");
		SetDependencyPrepreocessorDefinition("SRanipal", "SL_WITH_EYE_TRACKING");
		SetDependencyPrepreocessorDefinition("SlicingLogic", "SL_WITH_SLICING");
		SetDependencyPrepreocessorDefinition("UProtobuf", "SL_WITH_PROTO");
		SetDependencyPrepreocessorDefinition("UROSBridge", "SL_WITH_ROSBRIDGE");

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
		if (string.IsNullOrEmpty(UViz))
        {
			UViz = PublicDependencyModuleNames.Find(DependencyName => DependencyName.Equals("UViz"));
		}
		string UMongoQA = PrivateDependencyModuleNames.Find(DependencyName => DependencyName.Equals("UMongoQA"));
		if(string.IsNullOrEmpty(UMongoQA))
        {
			UMongoQA = PublicDependencyModuleNames.Find(DependencyName => DependencyName.Equals("UMongoQA"));
		}
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
