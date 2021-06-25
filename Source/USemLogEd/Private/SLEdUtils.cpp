// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLEdUtils.h"
#include "EngineUtils.h"
#include "AssetRegistryModule.h" // material search for InstancedStaticMesh

// SL
#include "Editor/SLSemanticMapWriter.h"

#include "Runtime/SLLoggerManager.h"

#include "Individuals/SLIndividualManager.h"
#include "Individuals/SLIndividualInfoManager.h"
#include "Individuals/SLIndividualUtils.h"
#include "Individuals/Type/SLBaseIndividual.h"

#include "Engine/StaticMeshActor.h"
#include "Materials/Material.h"

// Utils
#include "Utils/SLTagIO.h"
#include "Utils/SLUuid.h"

// Write the semantic map
void FSLEdUtils::WriteSemanticMap(UWorld* World, bool bOverwrite)
{
	FString TaskId;
	for (TActorIterator<ASLLoggerManager> ActItr(World); ActItr; ++ActItr)
	{
		TaskId = *ActItr->GetTaskId();
		break;
	}
	if(TaskId.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not find the semantic manager to read the task id, set to default.."),
			*FString(__func__), __LINE__);
		TaskId = "DefaultTaskId";
	}
	
	FSLSemanticMapWriterParams Params;
	Params.Id = TaskId;
	Params.TemplateType = ESLOwlSemanticMapTemplate::IAIKitchen;
	Params.Level = World->GetMapName();
	Params.DirectoryPath = TEXT("SL/") + TaskId;
	Params.bOverwrite = bOverwrite;

	// Generate map and write to file
	FSLSemanticMapWriter SemMapWriter;
	SemMapWriter.WriteToFile(World, Params);
}


/* Managers */
//// Get the semantic individual manager from the world, add new if none are available
//ASLIndividualManager* FSLEdUtils::GetOrCreateNewIndividualManager(UWorld* World, bool bCreateNew)
//{
//	int32 ActNum = 0;
//	ASLIndividualManager* Manager = nullptr;
//	for (TActorIterator<ASLIndividualManager> ActItr(World); ActItr; ++ActItr)
//	{
//		Manager = *ActItr;
//		ActNum++;
//	}
//	if (ActNum > 1)
//	{
//		UE_LOG(LogTemp, Warning, TEXT("%s::%d There are %ld individual managers in the world, the should only be one.."),
//			*FString(__FUNCTION__), __LINE__, ActNum);
//	}
//	else if(ActNum == 0 && bCreateNew)
//	{
//		UE_LOG(LogTemp, Warning, TEXT("%s::%d There are no individual managers in the world, spawning one.."),
//			*FString(__FUNCTION__), __LINE__);
//		FActorSpawnParameters Params;
//		//Params.Name = FName(TEXT("SL_IndividualManager"));
//		Manager = World->SpawnActor<ASLIndividualManager>(Params);
//#if WITH_EDITOR
//		Manager->SetActorLabel(TEXT("SL_IndividualManager"));
//#endif // WITH_EDITOR
//		World->MarkPackageDirty();
//	}
//	return Manager;
//}

//// Get the vis info manager form the world, add new one if none are available
//ASLIndividualInfoManager* FSLEdUtils::GetOrCreateNewVisualInfoManager(UWorld* World, bool bCreateNew)
//{
//	int32 ActNum = 0;
//	ASLIndividualInfoManager* Manager = nullptr;
//	for (TActorIterator<ASLIndividualInfoManager> ActItr(World); ActItr; ++ActItr)
//	{
//		Manager = *ActItr;
//		ActNum++;
//	}
//	if (ActNum > 1)
//	{
//		UE_LOG(LogTemp, Warning, TEXT("%s::%d There are %ld individual visual info managers in the world, the should only be one.."),
//			*FString(__FUNCTION__), __LINE__, ActNum);
//	}
//	else if (ActNum == 0 && bCreateNew)
//	{
//		UE_LOG(LogTemp, Warning, TEXT("%s::%d There are no individual visual info managers in the world, spawning one.."),
//			*FString(__FUNCTION__), __LINE__);
//		FActorSpawnParameters Params;
//		//Params.Name = FName(TEXT("SL_IndividualVisualInfoManager"));
//		Manager = World->SpawnActor<ASLIndividualInfoManager>(Params);		
//#if WITH_EDITOR
//		Manager->SetActorLabel(TEXT("SL_IndividualVisualInfoManager"));
//#endif // WITH_EDITOR
//		World->MarkPackageDirty();
//	}
//	return Manager;
//}

	// Log id values 
void FSLEdUtils::LogIds(UWorld* World)
{
	FString LogIdsString;
	int32 Num = 0;
	for (TActorIterator<AActor> ActItr(World); ActItr; ++ActItr)
	{
		if (USLBaseIndividual* BI = FSLIndividualUtils::GetIndividualObject(*ActItr))
		{
			LogIdsString.Append(BI->GetIdValue()).Append(";");
			Num++;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no individual.. "), *FString(__FUNCTION__), __LINE__, *ActItr->GetName());
		}
	}
	UE_LOG(LogTemp, Log, TEXT("%s::%d\tNum=%d;\tString:\n\n%s\n\n"),
		*FString(__FUNCTION__), __LINE__, Num, *LogIdsString);
}

void FSLEdUtils::LogIds(const TArray<AActor*>& Actors)
{
	FString LogIdsString;
	int32 Num = 0;
	for (const auto& Act : Actors)
	{
		if (USLBaseIndividual* BI = FSLIndividualUtils::GetIndividualObject(Act))
		{
			LogIdsString.Append(BI->GetIdValue()).Append(",");
			Num++;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no individual.. "), *FString(__FUNCTION__), __LINE__, *Act->GetName());
		}
	}
	UE_LOG(LogTemp, Log, TEXT("%s::%d\tNum=%d;\tString:\n\n%s\n\n"),
		*FString(__FUNCTION__), __LINE__, Num, *LogIdsString);
}

// Remove all tag keys
bool FSLEdUtils::RemoveTagKey(UWorld* World, const FString& TagType, const FString& TagKey)
{
	return FSLTagIO::RemoveWorldKVPairs(World, TagType, TagKey);
}

bool FSLEdUtils::RemoveTagKey(const TArray<AActor*>& Actors, const FString& TagType, const FString& TagKey)
{
	bool bMarkDirty = false;
	for (const auto& Act : Actors)
	{
		bMarkDirty = FSLTagIO::RemoveKVPair(Act, TagType, TagKey) || bMarkDirty;
	}
	return bMarkDirty;
}


// Remove all tags of the "SemLog" type
bool FSLEdUtils::RemoveTagType(UWorld* World, const FString& TagType)
{
	bool bMarkDirty = false;
	for (TActorIterator<AActor> ActItr(World); ActItr; ++ActItr)
	{
		int32 Pos = INDEX_NONE;
		if (FSLTagIO::HasType(*ActItr, TagType, &Pos))
		{
			ActItr->Modify();
			ActItr->Tags.RemoveAt(Pos);
			bMarkDirty = true;
		}
	}
	return bMarkDirty;
}

bool FSLEdUtils::RemoveTagType(const TArray<AActor*>& Actors, const FString& TagType)
{
	bool bMarkDirty = false;
	for (const auto& Act : Actors)
	{
		int32 Pos = INDEX_NONE;
		if (FSLTagIO::HasType(Act, TagType, &Pos))
		{
			Act->Modify();
			Act->Tags.RemoveAt(Pos);
			bMarkDirty = true;
		}
	}
	return bMarkDirty;
}


// Add semantic monitor components to the actors
bool FSLEdUtils::AddSemanticMonitorComponents(UWorld* World, bool bOverwrite)
{
	return false;
	//// Iterate only static mesh actors
	//for (TActorIterator<AStaticMeshActor> ActItr(GEditor->GetEditorWorldContext().World()); ActItr; ++ActItr)
	//{
	//	// Continue only if a valid mesh component is available
	//	if (UStaticMeshComponent* SMC = ActItr->GetStaticMeshComponent())
	//	{
	//		// Ignore if actor is not tagged
	//		if (FTags::HasKey(*ActItr, "SemLog", "Class"))
	//		{
	//			// Continue if no previous components are created
	//			TArray<USLContactMonitorBox*> Comps;
	//			ActItr->GetComponents<USLContactMonitorBox>(Comps);
	//			//if (Comps.Num() == 0)
	//			//{
	//			//	USLContactMonitorBox* Comp = NewObject<USLContactMonitorBox>(*ActItr);
	//			//	Comp->RegisterComponent();
	//			//	/*FTransform T;
	//			//	ActItr->AddComponent("USLContactMonitorBox", false, T, USLContactMonitorBox::StaticClass());*/
	//			//}
	//		}
	//	}
	//}
}

bool FSLEdUtils::AddSemanticMonitorComponents(const TArray<AActor*>& Actors, bool bOverwrite)
{
	return false;
}


// Enable overlaps on actors
bool FSLEdUtils::EnableOverlaps(UWorld* World)
{
	// TODO called on individual init
	bool bMarkDirty = false;
	for (TActorIterator<AStaticMeshActor> ActItr(World); ActItr; ++ActItr)
	{
		if (UStaticMeshComponent* SMC = ActItr->GetStaticMeshComponent())
		{
			if (!SMC->GetGenerateOverlapEvents())
			{
				ActItr->Modify();
				SMC->SetGenerateOverlapEvents(true);
				bMarkDirty = true;
			}
		}
	}
	return bMarkDirty;
}

bool FSLEdUtils::EnableOverlaps(const TArray<AActor*>& Actors)
{
	bool bMarkDirty = false;
	for (const auto& Act : Actors)
	{
		if (AStaticMeshActor* SMA = Cast<AStaticMeshActor>(Act))
		{
			if (UStaticMeshComponent* SMC = SMA->GetStaticMeshComponent())
			{
				if (!SMC->GetGenerateOverlapEvents())
				{
					Act->Modify();
					SMC->SetGenerateOverlapEvents(true);
					bMarkDirty = true;
				}
			}
		}
	}
	return bMarkDirty;
}


// Enable all materials for instanced static mesh rendering
void FSLEdUtils::EnableAllMaterialsForInstancedStaticMesh()
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	TArray<FAssetData> AllAsset;
	AssetRegistryModule.Get().GetAssetsByPath(TEXT("/Game/"), AllAsset, true, false);
	
	// TODO can be optimized to only search for materials:
	//FARFilter Filter;
	//Filter.Classes.Add(UStaticMesh::StaticClass());
	//Filter.PackagePaths.Add("/Game/Meshes");
	//AssetRegistryModule.Get().GetAssets(Filter, AssetData);

	for (FAssetData Data : AllAsset)
	{
		if (Data.AssetClass.ToString().Equals(TEXT("Material")))
		{
			UMaterial* Material = Cast<UMaterial>(Data.GetAsset());
			if (!Material->bUsedWithInstancedStaticMeshes)
			{
				Material->bUsedWithInstancedStaticMeshes = true;
				Data.GetPackage()->MarkPackageDirty();
				UE_LOG(LogTemp, Error, TEXT("%s::%d Material: %s is enabled for instanced static mesh.."), *FString(__func__), __LINE__, *Data.GetPackage()->GetFName().ToString());
			}
		}
	}

}

