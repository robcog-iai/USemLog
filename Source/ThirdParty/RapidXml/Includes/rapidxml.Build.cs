// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class RapidXML : ModuleRules
{
	public RapidXML(TargetInfo Target)
	{
		Type = ModuleType.External;

        PublicIncludePaths.AddRange(
            new string[] {
                "rapidxml"
				// ... add public include paths required here ...
			}
            );
    }
}
