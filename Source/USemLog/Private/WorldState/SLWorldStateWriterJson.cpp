// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "WorldState/SLWorldStateWriterJson.h"
#include "Animation/SkeletalMeshActor.h"
#include "HAL/PlatformFilemanager.h"
#include "Conversions.h"

// Constructor
FSLWorldStateWriterJson::FSLWorldStateWriterJson(float DistanceStepSize, float RotationStepSize,
	const FString& Location, const FString& EpisodeId) :
	ISLWorldStateWriter(DistanceStepSize, RotationStepSize)
{
	FSLWorldStateWriterJson::SetFileHandle(Location, EpisodeId);
}

// Destr
FSLWorldStateWriterJson::~FSLWorldStateWriterJson()
{
	if (FileHandle)
	{
		delete FileHandle;
	}
}

// Called to write the data (it also removes invalid item -- e.g. deleted ones)
void FSLWorldStateWriterJson::Write(TArray<TSLItemState<AActor>>& NonSkeletalActorPool,
	TArray<TSLItemState<ASLSkeletalMeshActor>>& SkeletalActorPool,
	TArray<TSLItemState<USceneComponent>>& NonSkeletalComponentPool,
	float Timestamp)
{
	// Json root object
	TSharedPtr<FJsonObject> JsonRootObj = MakeShareable(new FJsonObject);

	// Out Json array of entities
	TArray<TSharedPtr<FJsonValue>> JsonEntitiesArr;
	
	// Add non skeletal actors data to the json array
	FSLWorldStateWriterJson::AddNonSkeletalActors(NonSkeletalActorPool, JsonEntitiesArr);

	// Add skeletal actors
	FSLWorldStateWriterJson::AddSkeletalActors(SkeletalActorPool, JsonEntitiesArr);

	// Add non skeletal components data to the json array
	FSLWorldStateWriterJson::AddNonSkeletalComponents(NonSkeletalComponentPool, JsonEntitiesArr);

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
}

// Set the file handle for the logger
void FSLWorldStateWriterJson::SetFileHandle(const FString& LogDirectory, const FString& InEpisodeId)
{
	const FString Filename = InEpisodeId + TEXT("_WS.json");
	FString EpisodesDirPath = FPaths::ProjectDir() + LogDirectory + TEXT("/Episodes/");
	FPaths::RemoveDuplicateSlashes(EpisodesDirPath);

	const FString FilePath = EpisodesDirPath + Filename;

	// Create logging directory path and the filehandle
	FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*EpisodesDirPath);
	FileHandle = FPlatformFileManager::Get().GetPlatformFile().OpenWrite(*FilePath, true);
}

// Add non skeletal actors to a json format
void FSLWorldStateWriterJson::AddNonSkeletalActors(TArray<TSLItemState<AActor>>& NonSkeletalActorPool,
	TArray<TSharedPtr<FJsonValue>>& OutJsonEntitiesArr)
{
	// Iterate items
	for (auto Itr(NonSkeletalActorPool.CreateIterator()); Itr; ++Itr)
	{
		// Check if pointer is valid
		if (Itr->Entity.IsValid(/*false, true*/))
		{
			// Check if the entity moved more than the threshold since the last logging
			const FVector CurrLoc = Itr->Entity->GetActorLocation();
			const FQuat CurrQuat = Itr->Entity->GetActorQuat();

			const float Distance = FVector::DistSquared(CurrLoc, Itr->PrevLoc);

			if (FVector::DistSquared(CurrLoc, Itr->PrevLoc) > DistanceStepSizeSquared ||
				CurrQuat.AngularDistance(Itr->PrevQuat))
			{
				// Update prev state
				Itr->PrevLoc = CurrLoc;
				Itr->PrevQuat = CurrQuat;

				// Get current entry as json object
				TSharedPtr<FJsonObject> JsonEntry = FSLWorldStateWriterJson::GetAsJsonEntry2(
					TMap<FString, FString>{ {"id", Itr->Item.Id}, { "class", Itr->Item.Class } },
					CurrLoc, CurrQuat);

				// Add entity to json array
				OutJsonEntitiesArr.Add(MakeShareable(new FJsonValueObject(JsonEntry)));
			}
		}
		else
		{
			Itr.RemoveCurrent();
			FSLMappings::GetInstance()->RemoveItem(Itr->Entity.Get());
		}
	}
}

// Add semantically annotated skeletal actors to a json format
void FSLWorldStateWriterJson::AddSkeletalActors(TArray<TSLItemState<ASLSkeletalMeshActor>>& SkeletalActorPool,
	TArray<TSharedPtr<FJsonValue>>& OutJsonEntitiesArr)
{
	// Iterate items
	for (auto Itr(SkeletalActorPool.CreateIterator()); Itr; ++Itr)
	{
		// Check if pointer is valid
		if (Itr->Entity.IsValid(/*false, true*/))
		{
			// Check if the entity moved more than the threshold since the last logging
			const FVector CurrLoc = Itr->Entity->GetActorLocation();
			const FQuat CurrQuat = Itr->Entity->GetActorQuat();

			const float Distance = FVector::DistSquared(CurrLoc, Itr->PrevLoc);

			if (FVector::DistSquared(CurrLoc, Itr->PrevLoc) > DistanceStepSizeSquared ||
				CurrQuat.AngularDistance(Itr->PrevQuat))
			{
				// Update prev state
				Itr->PrevLoc = CurrLoc;
				Itr->PrevQuat = CurrQuat;

				// Get current entry as json object
				TSharedPtr<FJsonObject> JsonEntry = FSLWorldStateWriterJson::GetAsJsonEntry2(
					TMap<FString, FString>{ {"id", Itr->Item.Id}, { "class", Itr->Item.Class } },
					CurrLoc, CurrQuat);
				
				// Json array of bones
				TArray<TSharedPtr<FJsonValue>> JsonBonesArr;

				// Check is the skeletal actor component is valid and has a class mapping of the bone
				if (USLSkeletalMapDataAsset* SkelMapDataAsset = Itr->Entity->GetSkeletalMapDataAsset())
				{
					if (USkeletalMeshComponent* SkelComp = Itr->Entity->GetSkeletalMeshComponent())
					{
						// Iterate through the bones of the skeletal mesh
						for (const auto& Pair : SkelMapDataAsset->BoneClassMap)
						{
							const FVector CurrLoc = SkelComp->GetBoneLocation(Pair.Key);
							const FQuat CurrQuat = SkelComp->GetBoneQuaternion(Pair.Key);

							// Get current entry as json object
							TSharedPtr<FJsonObject> JsonBoneEntry = FSLWorldStateWriterJson::GetAsJsonEntry2(
								TMap<FString, FString>{ {"bone", Pair.Key.ToString()}, { "class", Pair.Value } },
								CurrLoc, CurrQuat);

							// Add bone to Json array
							JsonBonesArr.Add(MakeShareable(new FJsonValueObject(JsonBoneEntry)));
						}
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
			FSLMappings::GetInstance()->RemoveItem(Itr->Entity.Get());
		}
	}
}

// Add non skeletal components
void FSLWorldStateWriterJson::AddNonSkeletalComponents(TArray<TSLItemState<USceneComponent>>& NonSkeletalComponentPool,
	TArray<TSharedPtr<FJsonValue>>& OutJsonEntitiesArr)
{
	// Iterate items
	for (auto Itr(NonSkeletalComponentPool.CreateIterator()); Itr; ++Itr)
	{
		// Check if pointer is valid
		if (Itr->Entity.IsValid(/*false, true*/))
		{
			// Check if the entity moved more than the threshold since the last logging
			const FVector CurrLoc = Itr->Entity->GetComponentLocation();
			const FQuat CurrQuat = Itr->Entity->GetComponentQuat();

			const float Distance = FVector::DistSquared(CurrLoc, Itr->PrevLoc);

			if (FVector::DistSquared(CurrLoc, Itr->PrevLoc) > DistanceStepSizeSquared ||
				CurrQuat.AngularDistance(Itr->PrevQuat))
			{
				// Update prev state
				Itr->PrevLoc = CurrLoc;
				Itr->PrevQuat = CurrQuat;

				// Get current entry as json object
				TSharedPtr<FJsonObject> JsonEntry = FSLWorldStateWriterJson::GetAsJsonEntry2(
					TMap<FString, FString>{ {"id", Itr->Item.Id}, { "class", Itr->Item.Class } },
					CurrLoc, CurrQuat);

				// Add entity to json array
				OutJsonEntitiesArr.Add(MakeShareable(new FJsonValueObject(JsonEntry)));
			}
		}
		else
		{
			Itr.RemoveCurrent();
			FSLMappings::GetInstance()->RemoveItem(Itr->Entity.Get());
		}
	}
}

// Get entry as json object
TSharedPtr<FJsonObject> FSLWorldStateWriterJson::GetAsJsonEntry(const FString& InId,
	const FString& InClass,
	const FVector& InLoc,
	const FQuat& InQuat)
{
	// Switch to right handed ROS transformation
	const FVector ROSLoc = FConversions::UToROS(InLoc);
	const FQuat ROSQuat = FConversions::UToROS(InQuat);

	// New json entity object
	TSharedPtr<FJsonObject> JsonObj = MakeShareable(new FJsonObject);

	// Add "id" field if available (bones have no separate ids)
	if (!InId.IsEmpty())
	{
		JsonObj->SetStringField("id", InId);
	}

	// Add "class" field
	JsonObj->SetStringField("class", InClass);

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

// Get entry as json object
TSharedPtr<FJsonObject> FSLWorldStateWriterJson::GetAsJsonEntry2(const TMap<FString, FString>& InKeyValMap,
	const FVector& InLoc, const FQuat& InQuat)
{
	// New json entity object
	TSharedPtr<FJsonObject> JsonObj = MakeShareable(new FJsonObject);

	// Add key values
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