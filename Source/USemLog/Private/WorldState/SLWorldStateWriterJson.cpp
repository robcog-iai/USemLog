// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "WorldState/SLWorldStateWriterJson.h"
#include "Animation/SkeletalMeshActor.h"
#include "HAL/PlatformFilemanager.h"
#include "Conversions.h"

// Constructor
FSLWorldStateWriterJson::FSLWorldStateWriterJson()
{
	bIsInit = false;
}

// Init constructor
FSLWorldStateWriterJson::FSLWorldStateWriterJson(const FSLWorldStateWriterParams& InParams)
{
	bIsInit = false;
	FSLWorldStateWriterJson::Init(InParams);
}

// Destr
FSLWorldStateWriterJson::~FSLWorldStateWriterJson()
{
	FSLWorldStateWriterJson::Finish();
	if (FileHandle)
	{
		delete FileHandle;
	}
}

// Init
void FSLWorldStateWriterJson::Init(const FSLWorldStateWriterParams& InParams)
{
	LinDistSqMin = InParams.LinearDistanceSquared;
	AngDistMin = InParams.AngularDistance;
	bIsInit = SetFileHandle(InParams.TaskId, InParams.EpisodeId);
}


// Finish
void FSLWorldStateWriterJson::Finish()
{
	if (bIsInit)
	{
		bIsInit = false;
	}
}

// Called to write the data (it also removes invalid item -> e.g. deleted ones)
void FSLWorldStateWriterJson::Write(float Timestamp,
	TArray<TSLEntityPreviousPose<AActor>>& ActorEntities,
	TArray<TSLEntityPreviousPose<USceneComponent>>& ComponentEntities,
	TArray<TSLEntityPreviousPose<USLSkeletalDataComponent>>& SkeletalEntities,
	FSLGazeData& GazeData,
	bool bCheckAndRemoveInvalidEntities)
{
#if SL_WITH_JSON
	// Json root object
	TSharedPtr<FJsonObject> JsonRootObj = MakeShareable(new FJsonObject);

	// Out Json array of entities
	TArray<TSharedPtr<FJsonValue>> JsonEntitiesArr;

	// Add entities to json array
	FSLWorldStateWriterJson::AddActorEntities(ActorEntities, JsonEntitiesArr);
	FSLWorldStateWriterJson::AddComponentEntities(ComponentEntities, JsonEntitiesArr);
	FSLWorldStateWriterJson::AddSkeletalEntities(SkeletalEntities, JsonEntitiesArr);

	// Avoid appending empty entries
	if (JsonEntitiesArr.Num() > 0)
	{
		// Set timestamp
		JsonRootObj->SetNumberField("timestamp", Timestamp);

		// Add actors to Json root
		JsonRootObj->SetArrayField("entities", JsonEntitiesArr);

		// Write entry to file
		FSLWorldStateWriterJson::WriteToFile(JsonRootObj);
	}
#endif // SL_WITH_JSON
}

// Set the file handle for the logger
bool FSLWorldStateWriterJson::SetFileHandle(const FString& LogDirectory, const FString& InEpisodeId)
{
	const FString Filename = InEpisodeId + TEXT("_WS.json");
	FString EpisodesDirPath = FPaths::ProjectDir() + "/SemLog/" + LogDirectory + TEXT("/Episodes/");
	FPaths::RemoveDuplicateSlashes(EpisodesDirPath);

	const FString FilePath = EpisodesDirPath + Filename;

	// Create logging directory path and the filehandle
	FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*EpisodesDirPath);
	FileHandle = FPlatformFileManager::Get().GetPlatformFile().OpenWrite(*FilePath, true);

	return FileHandle != nullptr;
}

#if SL_WITH_JSON
// Get non skeletal actors as json array
void FSLWorldStateWriterJson::AddActorEntities(TArray<TSLEntityPreviousPose<AActor>>& ActorEntities,
	TArray<TSharedPtr<FJsonValue>>& OutJsonEntitiesArr)
{
	// Iterate items
	for (auto Itr(ActorEntities.CreateIterator()); Itr; ++Itr)
	{
		// Check if pointer is valid
		if (Itr->Obj.IsValid(/*false, true*/))
		{
			// Check if the entity moved more than the threshold since the last logging
			const FVector CurrLoc = Itr->Obj->GetActorLocation();
			const FQuat CurrQuat = Itr->Obj->GetActorQuat();

			if (FVector::DistSquared(CurrLoc, Itr->PrevLoc) > LinDistSqMin ||
				CurrQuat.AngularDistance(Itr->PrevQuat))
			{
				// Update prev state
				Itr->PrevLoc = CurrLoc;
				Itr->PrevQuat = CurrQuat;

				// Get current entry as json object
				TSharedPtr<FJsonObject> JsonEntry = FSLWorldStateWriterJson::GetAsJsonEntry(
					TMap<FString, FString>{ {"id", Itr->Entity.Id}, { "class", Itr->Entity.Class } },
					CurrLoc, CurrQuat);

				// Add entity to json array
				OutJsonEntitiesArr.Add(MakeShareable(new FJsonValueObject(JsonEntry)));
			}
		}
		else
		{
			Itr.RemoveCurrent();
			FSLEntitiesManager::GetInstance()->RemoveEntity(Itr->Obj.Get());
		}
	}
}

// Get non skeletal components as json array
void FSLWorldStateWriterJson::AddComponentEntities(TArray<TSLEntityPreviousPose<USceneComponent>>& ComponentEntities,
	TArray<TSharedPtr<FJsonValue>>& OutJsonEntitiesArr)
{
	// Iterate items
	for (auto Itr(ComponentEntities.CreateIterator()); Itr; ++Itr)
	{
		// Check if pointer is valid
		if (Itr->Obj.IsValid(/*false, true*/))
		{
			// Check if the entity moved more than the threshold since the last logging
			const FVector CurrLoc = Itr->Obj->GetComponentLocation();
			const FQuat CurrQuat = Itr->Obj->GetComponentQuat();

			if (FVector::DistSquared(CurrLoc, Itr->PrevLoc) > LinDistSqMin ||
				CurrQuat.AngularDistance(Itr->PrevQuat))
			{
				// Update prev state
				Itr->PrevLoc = CurrLoc;
				Itr->PrevQuat = CurrQuat;

				// Get current entry as json object
				TSharedPtr<FJsonObject> JsonEntry = FSLWorldStateWriterJson::GetAsJsonEntry(
					TMap<FString, FString>{ {"id", Itr->Entity.Id}, { "class", Itr->Entity.Class } },
					CurrLoc, CurrQuat);

				// Add entity to json array
				OutJsonEntitiesArr.Add(MakeShareable(new FJsonValueObject(JsonEntry)));
			}
		}
		else
		{
			Itr.RemoveCurrent();
			FSLEntitiesManager::GetInstance()->RemoveEntity(Itr->Obj.Get());
		}
	}
}

// Get skeletal actors as json array
void FSLWorldStateWriterJson::AddSkeletalEntities(TArray<TSLEntityPreviousPose<USLSkeletalDataComponent>>& SkeletalEntities,
	TArray<TSharedPtr<FJsonValue>>& OutJsonEntitiesArr)
{
	// Iterate items
	for (auto Itr(SkeletalEntities.CreateIterator()); Itr; ++Itr)
	{
		// Check if pointer is valid
		if (Itr->Obj.IsValid(/*false, true*/))
		{
			// Check if the entity moved more than the threshold since the last logging
			const FVector CurrLoc = Itr->Obj->GetComponentLocation();
			const FQuat CurrQuat = Itr->Obj->GetComponentQuat();

			if (FVector::DistSquared(CurrLoc, Itr->PrevLoc) > LinDistSqMin ||
				CurrQuat.AngularDistance(Itr->PrevQuat))
			{
				// Update prev state
				Itr->PrevLoc = CurrLoc;
				Itr->PrevQuat = CurrQuat;

				// Get current entry as json object
				TSharedPtr<FJsonObject> JsonEntry = FSLWorldStateWriterJson::GetAsJsonEntry(
					TMap<FString, FString>{ {"id", Itr->Entity.Id}, { "class", Itr->Entity.Class } },
					CurrLoc, CurrQuat);

				// Json array of bones
				TArray<TSharedPtr<FJsonValue>> JsonBonesArr;

				if (USkeletalMeshComponent* SkelComp = Itr->Obj->SkeletalMeshParent)
				{
					// Iterate through the bones of the skeletal mesh
					for (const auto& Pair : Itr->Obj->AllBonesData)
					{
						const FVector CurrBoneLoc = SkelComp->GetBoneLocation(Pair.Key);
						const FQuat CurrBoneQuat = SkelComp->GetBoneQuaternion(Pair.Key);

						// Get current entry as json object
						TMap<FString, FString> SemanticData;
						SemanticData.Add("bone", Pair.Key.ToString());
						if (!Pair.Value.Class.IsEmpty())
						{
							SemanticData.Add("class", Pair.Value.Class);
						}
						if (!Pair.Value.VisualMask.IsEmpty())
						{
							SemanticData.Add("mask_hex", Pair.Value.VisualMask);
						}

						TSharedPtr<FJsonObject> JsonBoneEntry = FSLWorldStateWriterJson::GetAsJsonEntry(
							SemanticData, CurrBoneLoc, CurrBoneQuat);

						// Add bone to Json array
						JsonBonesArr.Add(MakeShareable(new FJsonValueObject(JsonBoneEntry)));
					}
				}
				// Add bones to json entry
				JsonEntry->SetArrayField("bones", JsonBonesArr);

				// Add entity to json array
				OutJsonEntitiesArr.Add(MakeShareable(new FJsonValueObject(JsonEntry)));
			}
		}
		else
		{
			Itr.RemoveCurrent();
			FSLEntitiesManager::GetInstance()->RemoveEntity(Itr->Obj.Get());
		}
	}
}

// Get key value pairs as json entry
TSharedPtr<FJsonObject> FSLWorldStateWriterJson::GetAsJsonEntry(const TMap<FString, FString>& InKeyValMap,
	const FVector& InLoc, const FQuat& InQuat)
{
	// New json entity object
	TSharedPtr<FJsonObject> JsonObj = MakeShareable(new FJsonObject);

	// Add key values pairs
	for (const auto& Pair : InKeyValMap)
	{
		JsonObj->SetStringField(Pair.Key, Pair.Value);
	}

	// Switch to right handed ROS transformation
	const FVector ROSLoc = FConversions::UToROS(InLoc);
	const FQuat ROSQuat = FConversions::UToROS(InQuat);

	// Create and add "loc" field
	TSharedPtr<FJsonObject> LocObj = MakeShareable(new FJsonObject);
	LocObj->SetNumberField("x", ROSLoc.X);
	LocObj->SetNumberField("y", ROSLoc.Y);
	LocObj->SetNumberField("z", ROSLoc.Z);
	JsonObj->SetObjectField("loc", LocObj);

	// Create and add "rot" field
	TSharedPtr<FJsonObject> QuatObj = MakeShareable(new FJsonObject);
	QuatObj->SetNumberField("x", ROSQuat.X);
	QuatObj->SetNumberField("y", ROSQuat.Y);
	QuatObj->SetNumberField("z", ROSQuat.Z);
	QuatObj->SetNumberField("w", ROSQuat.W);
	JsonObj->SetObjectField("rot", QuatObj);

	return JsonObj;
}

// Write entry to file
void FSLWorldStateWriterJson::WriteToFile(const TSharedPtr<FJsonObject>& InRootObj)
{
	// Transform to string
	FString JsonString;
	TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(InRootObj.ToSharedRef(), JsonWriter);

	// Write to file
	if (FileHandle)
	{
		FileHandle->Write((const uint8*)TCHAR_TO_ANSI(*JsonString), JsonString.Len());
	}
}
#endif // SL_WITH_JSON