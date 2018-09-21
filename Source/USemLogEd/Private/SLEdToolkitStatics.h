// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once
#include "CoreMinimal.h"
#include "PlatformFilemanager.h"
#include "Engine/Selection.h"
#include "FileManager.h"
#include "FileHelper.h"
#include "EngineUtils.h"
#include "Ids.h"
#include "Tags.h"
#include "SLSemanticMapWriter.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "PhysicsEngine/PhysicsConstraintActor.h"
//#include "SLMap.h"
//#include "SLRuntimeManager.h"
//#include "SLLevelInfo.h"

struct FSLEdToolkitStatics
{
	// TODO avoid using static functions, see other examples
	//static bool bOverwriteSemanticMap;

	static bool AreActorsSelected()
	{
		return GEditor->GetSelectedActors()->Num() != 0;
	}

	static FReply GenerateSemanticMap()
	{
		//FSLSemanticMapWriter SemMapWriter;

		////SemMapWriter.Generate(
		////	GEditor->GetEditorWorldContext().World(), EMapTemplateType::Default);

		//SemMapWriter.Generate(
		//	GEditor->GetEditorWorldContext().World(), EMapTemplateType::IAISupermarket);

		//if (SemMapWriter.WriteToFile())
		//{
		//	UE_LOG(LogTemp, Error, TEXT("[%s][%d]"), TEXT(__FUNCTION__), __LINE__);
		//}

		FSLSemanticMapWriter SemMapWriter;
		SemMapWriter.WriteToFile(GEditor->GetEditorWorldContext().World(),
			EOwlSemanticMapTemplate::IAIKitchen, TEXT("SemLog"), TEXT("SemanticMap"));

		return FReply::Handled();
	}


	static bool NoRuntimeManagerInTheWorld()
	{
		//if (TActorIterator<ASLRuntimeManager>(GEditor->GetEditorWorldContext().World()))
		//{
		//	return false;
		//}
		return true;		
	}

	static FReply AddRuntimeManager()
	{
		//GEditor->GetEditorWorldContext().World()->SpawnActor(ASLRuntimeManager::StaticClass());
		return FReply::Handled();
	}

	static bool NoLevelInfoInTheWorld()
	{
		//if (TActorIterator<ASLLevelInfo>(GEditor->GetEditorWorldContext().World()))
		//{
		//	return false;
		//}
		return true;
	}

	static FReply AddLevelinfo()
	{
		//GEditor->GetEditorWorldContext().World()->SpawnActor(ASLLevelInfo::StaticClass());
		return FReply::Handled();
	}

	static FReply GenerateNewIds()
	{
		for (TActorIterator<AActor> ActItr(GEditor->GetEditorWorldContext().World()); ActItr; ++ActItr)
		{
			int32 TagIndex = FTags::GetTagTypeIndex(*ActItr, "SemLog");
			if (TagIndex != INDEX_NONE)
			{
				FTags::AddKeyValuePair(
					ActItr->Tags[TagIndex], "Id", FIds::NewGuidInBase64Url());
			}

			// Check component tags as well
			for (const auto& CompItr : ActItr->GetComponents())
			{
				int32 TagIndex = FTags::GetTagTypeIndex(CompItr, "SemLog");
				if (TagIndex != INDEX_NONE)
				{
					FTags::AddKeyValuePair(
						CompItr->ComponentTags[TagIndex], "Id", FIds::NewGuidInBase64());
				}
			}
		}
		return FReply::Handled();
	}

	static FReply ClearIds()
	{
		FTags::RemoveAllKeyValuePairs(GEditor->GetEditorWorldContext().World(), "SemLog", "Id");
		return FReply::Handled();
	}

	static FReply TagSemanticConstraints()
	{
		// Check constraint components
		for (TObjectIterator<UPhysicsConstraintComponent> ConstrItr; ConstrItr; ++ConstrItr)
		{
			// Check if constraint is not already tagged
			if (!FTags::HasType(*ConstrItr, "SemLog") &&
				ConstrItr->ConstraintActor1 != nullptr &&
				ConstrItr->ConstraintActor2 != nullptr)
			{
				// Check if constrained actors are tagged with a class
				if (FTags::HasKey(ConstrItr->ConstraintActor1, "SemLog", "Class") &&
					FTags::HasKey(ConstrItr->ConstraintActor2, "SemLog", "Class"))
				{
					const FString ConstraintTag = FString("SemLog;Id," + FIds::NewGuidInBase64Url() + ";");
					ConstrItr->ComponentTags.Add(*ConstraintTag);
				}
			}
		}

		//// Check constraint actors
		//for (TActorIterator<APhysicsConstraintActor> ConstrActItr(GEditor->GetEditorWorldContext().World()); ConstrActItr; ++ConstrActItr)
		//{
		//	// Check if constraint is not already tagged
		//	if (!FTags::HasType(*ConstrActItr, "SemLog") &&
		//		ConstrActItr->GetConstraintComp()->ConstraintActor1 != nullptr &&
		//		ConstrActItr->GetConstraintComp()->ConstraintActor2 != nullptr)
		//	{
		//		UE_LOG(LogTemp, Error, TEXT("ConstrActItr is: %s"), *ConstrActItr->GetName());
		//		// Check if constrained actors are tagged with a class
		//		if (FTags::HasKey(ConstrActItr->GetConstraintComp()->ConstraintActor1, "SemLog", "Class") &&
		//			FTags::HasKey(ConstrActItr->GetConstraintComp()->ConstraintActor2, "SemLog", "Class"))
		//		{
		//			UE_LOG(LogTemp, Error, TEXT("\t Add tag ConstrActItr is: %s"), *ConstrActItr->GetName());
		//			const FString ConstraintTag = "SemLog;Id," + FIds::NewGuidInBase64Url() + ";";
		//			ConstrActItr->Tags.Add(*ConstraintTag);
		//		}
		//	}
		//}
		return FReply::Handled();
	}

	static FReply TagSemanticClasses()
	{

		for (TActorIterator<AStaticMeshActor> ActItr(GEditor->GetEditorWorldContext().World()); ActItr; ++ActItr)
		{
			// Check if actor is not already tagged
			if (!FTags::HasKey(*ActItr, "SemLog", "Class"))
			{
				// Check if any of the components are tagged
				TArray<UStaticMeshComponent*> StaticComps;
				ActItr->GetComponents<UStaticMeshComponent>(StaticComps);
				
				bool bComponentTagged = false;
				bool bTagComponent = false;
				FString MeshName;
				for (auto& StaticMeshComponent : StaticComps)
				{
					MeshName = StaticMeshComponent->GetStaticMesh()->GetFullName();
					int32 Index;
					// Remove path information
					MeshName.FindLastChar('.', Index);
					MeshName.RemoveAt(0, Index + 1);

					// Remove SM
					MeshName.RemoveFromStart(TEXT("SM_"));					
					
					if (FTags::HasKey(StaticMeshComponent, "SemLog", "Class"))
					{
						bComponentTagged = true;
					} 
					else if(FTags::HasType(StaticMeshComponent, "SemLog"))
					{
						bTagComponent = true;
					}

					if (bTagComponent)
					{
						// Tag component but not actor
						FTags::AddKeyValuePair(StaticMeshComponent, "SemLog", "Class", MeshName);
						FTags::AddKeyValuePair(StaticMeshComponent, "SemLog", "Mobility", "Dynamic");
					} 
					else if (!bComponentTagged) 
					{
						// Tag actor not his component
						FTags::AddKeyValuePair(*ActItr, "SemLog", "Class", MeshName);
						FTags::AddKeyValuePair(*ActItr, "SemLog", "Mobility", "Dynamic");
					}
				}

				
			}

		}

		return FReply::Handled();
	}

	static FReply ClearClasses()
	{
		for (TActorIterator<AStaticMeshActor> ActItr(GEditor->GetEditorWorldContext().World()); ActItr; ++ActItr)
		{
			FTags::RemoveKeyValuePair(*ActItr, "SemLog", "Class");

			TArray<UStaticMeshComponent*> StaticComps;
			ActItr->GetComponents<UStaticMeshComponent>(StaticComps);
			for (auto& StaticMeshComponent : StaticComps)
			{
				FTags::RemoveKeyValuePair(StaticMeshComponent, "SemLog", "Class");
			}
		}

		return FReply::Handled();
	}

	// Replace text in tags
	static FReply ReplaceText()
	{
		// What to replace
		const FString SearchText = "LogType";
		// With what
		const FString ReplaceText = "Mobility";

		for (TActorIterator<AActor> ActItr(GEditor->GetEditorWorldContext().World()); ActItr; ++ActItr)
		{
			for (auto& T : ActItr->Tags)
			{
				FString TagAsString = T.ToString();
				TagAsString.ReplaceInline(*SearchText, *ReplaceText);
				T = FName(*TagAsString);
			}
			// Iterate actor components
			TArray<UActorComponent*> Comps;
			ActItr->GetComponents<UActorComponent>(Comps);
			for (auto& C : Comps)
			{
				for (auto& T : C->ComponentTags)
				{
					FString TagAsString = T.ToString();
					TagAsString.ReplaceInline(*SearchText, *ReplaceText);
					T = FName(*TagAsString);
				}
			}
		}
		return FReply::Handled();
	}

	// Replace text in tags
	static FReply ClearAllTags()
	{
		for (TActorIterator<AActor> ActItr(GEditor->GetEditorWorldContext().World()); ActItr; ++ActItr)
		{
			ActItr->Tags.Empty();

			// Iterate actor components
			TArray<UActorComponent*> Comps;
			ActItr->GetComponents<UActorComponent>(Comps);
			for (auto& C : Comps)
			{
				C->ComponentTags.Empty();
			}
		}
		return FReply::Handled();
	}

	// Create semantic logs directory
	static bool SetupLoggingDirectory(const FString& DirectoryName)
	{
		// Get platform file
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

		// Create directory, return true if already exists
		return PlatformFile.CreateDirectory(*FPaths::ProjectDir().Append("/").Append(DirectoryName));
	}

	// Check if semantic map already exists
	static bool SemanticMapExists(const FString& Path)
	{
		return IFileManager::Get().FileExists(*Path);
	}

	// Delete semantic map
	static bool DeleteSemanticMap(const FString& Path)
	{
		return IFileManager::Get().Delete(*Path);
	}

	// Checkbox
	static void OnCheckedOverwriteSemanticMap(ECheckBoxState NewCheckedState)
	{
		//bOverwriteSemanticMap = (NewCheckedState == ECheckBoxState::Checked);
	}

	//// Generate semantic map button callback
	//static FReply GenerateSemanticMap()
	//{
	//	//UE_LOG(SemLogEd, Warning, TEXT("Game DIR: %s "), *FPaths::GameDir());

	//	// Let editor know that we're about to do something that we want to undo/redo
	//	//GEditor->BeginTransaction(LOCTEXT("SemLogGenSemMap", "Generate Semantic Map"));

	//	// Get editor world
	//	UWorld* EdWorld = GEditor->GetEditorWorldContext().World();

	//	// Iterate all actor
	//	for (TActorIterator<AActor> ActorItr(EdWorld); ActorItr; ++ActorItr)
	//	{
	//		// Flag checking if the actor has a semantic logger tag set
	//		bool bHasSemLogTag = false;

	//		// Iterate all the tags, check for keyword "SemLog:"
	//		for (const auto& TagItr : ActorItr->Tags)
	//		{
	//			// Copy of the current tag as FString, Tag ex: "SemLog:Class,Box;Runtime,Dynamic;Id,yg6D;"
	//			FString CurrTag = TagItr.ToString();

	//			// Check if tag is related to the semantic logger
	//			if (CurrTag.RemoveFromStart("SemLog:"))
	//			{
	//				// Tag state: "Class,Box;Runtime,Dynamic;Id,yg6D;"
	//				bHasSemLogTag = true;

	//				// Stop iteration if object is ignored
	//				if (CurrTag.Contains("IGNORE"))
	//				{
	//					UE_LOG(SemLogEd, Warning, TEXT("%s - IGNORED"), *ActorItr->GetName());
	//					break;
	//				}

	//				// Map to store the key value properties
	//				TMap<FString, FString> SemLogProperties;

	//				// Split on semicolon, Tag state "Class,Box" / "Runtime,Dynamic" / "Id,yg6D"
	//				FString CurrPair;
	//				while (CurrTag.Split(TEXT(";"), &CurrPair, &CurrTag))
	//				{
	//					// Split on comma, Tag state "Class" / "Box"
	//					FString CurrKey, CurrValue;
	//					if (CurrPair.Split(TEXT(","), &CurrKey, &CurrValue))
	//					{
	//						SemLogProperties.Add(CurrKey, CurrValue);
	//					}
	//				}

	//				// Output properties
	//				for (const auto & PropElem : SemLogProperties)
	//				{
	//					UE_LOG(SemLogEd, Display, TEXT("Key %s - %s"), *PropElem.Key, *PropElem.Value);
	//				}
	//			}
	//		}

	//		if (!bHasSemLogTag)
	//		{
	//			// Actor has no semantic logging related tag
	//			UE_LOG(SemLogEd, Error, TEXT("%s - NO SemLog TAG, added IGNORE TAG"), *ActorItr->GetName());
	//			ActorItr->Tags.Add(FName(TEXT("SemLog:IGNORE;")));
	//		}
	//	}
	//	
	//	// Close transaction
	//	//GEditor->EndTransaction();

	//	return FReply::Handled();
	//}

	//static bool IsAnythingSelected()
	//{			
	//	return GEditor->GetSelectedActors()->Num() != 0;
	//}

	//static FReply OnShowAllObjectsTagsButtonClick()
	//{
	//	// Let editor know that we're about to do something that we want to undo/redo
	//	GEditor->BeginTransaction(LOCTEXT("ShowAllObjectsTransactionName", "ShowAllObjects"));

	//	// For each selected actor
	//	for (TActorIterator<AActor> ActorItr(GEditor->GetEditorWorldContext().World()); ActorItr; ++ActorItr)
	//	{
	//		// Register actor in opened transaction (undo/redo)
	//		ActorItr->Modify();
	//		// Draw debug
	//		ActorItr->DrawDebugComponents();
	//		// Debug msg
	//		UE_LOG(SemLogEd, Warning, TEXT("All objects: %s"), *ActorItr->GetName());
	//	}			

	//	// We're done moving actors so close transaction
	//	GEditor->EndTransaction();

	//	return FReply::Handled();
	//}

	//static FReply OnShowSelectedObjectsTagsButtonClick()
	//{
	//	USelection* SelectedActors = GEditor->GetSelectedActors();

	//	// Let editor know that we're about to do something that we want to undo/redo
	//	GEditor->BeginTransaction(LOCTEXT("ShowSelectedObjectsTransactionName", "ShowSelectedObjects"));

	//	// For each selected actor
	//	for (FSelectionIterator Iter(*SelectedActors); Iter; ++Iter)
	//	{
	//		if (AActor* LevelActor = Cast<AActor>(*Iter))
	//		{
	//			// Register actor in opened transaction (undo/redo)
	//			LevelActor->Modify();
	//			// Draw debug
	//			LevelActor->DrawDebugComponents();
	//			// Debug msg
	//			UE_LOG(SemLogEd, Warning, TEXT("Selected objects: %s"), *LevelActor->GetName());
	//		}
	//	}

	//	// We're done moving actors so close transaction
	//	GEditor->EndTransaction();

	//	return FReply::Handled();
	//}

	//static FText GetTagsText()
	//{
	//	return LOCTEXT("TagsText", "etc.etc.\netc.etc");
	//}
};