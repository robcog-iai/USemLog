// Fill out your copyright notice in the Description page of Project Settings.

using System.IO;

namespace UnrealBuildTool.Rules
{
	public class SemLog : ModuleRules
	{
		// PATH HELPERS
		private string ThirdPartyPath
		{
			get { return Path.Combine(ModuleDirectory, "../../ThirdParty/"); }
		}
	
		public SemLog(TargetInfo Target)
		{
			PublicIncludePaths.AddRange(
				new string[] {
					"SemLog/Public",
					// ... add public include paths required here ...
				}
				);

			PrivateIncludePaths.AddRange(
				new string[] {
					"SemLog/Private",
					Path.Combine(ThirdPartyPath, "RapidXml", "Includes"),
					// ... add other private include paths required here ...
				}
				);

			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
					"CoreUObject",
					"Engine",
					"InputCore",
					"Json",
					"JsonUtilities",
					// ... add other public dependencies that you statically link with here ...
				}
				);

			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					// ... add private dependencies that you statically link with here ...
				}
				);

			DynamicallyLoadedModuleNames.AddRange(
				new string[]
				{
					// ... add any modules that your module loads dynamically here ...
				}
				);
		}
	}
}