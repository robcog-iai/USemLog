// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "WorldState/SLWorldStateWriterJson.h"
#include "Animation/SkeletalMeshActor.h"
#include "HAL/PlatformFilemanager.h"
#include "Conversions.h"

// Constr
FSLWorldStateWriterJson::FSLWorldStateWriterJson()
{
}

// Constr with Init
FSLWorldStateWriterJson::FSLWorldStateWriterJson(FSLWorldStateAsyncWorker* InWorkerParent, const FString& LogDirectory, const FString& EpisodeId)
{
	FSLWorldStateWriterJson::Init(InWorkerParent, LogDirectory, EpisodeId);
}

// Destr
FSLWorldStateWriterJson::~FSLWorldStateWriterJson()
{
	if (FileHandle)
	{
		delete FileHandle;
	}
}

// Init
void FSLWorldStateWriterJson::Init(FSLWorldStateAsyncWorker* InWorkerParent, const FString& LogDirectory, const FString& EpisodeId)
{
	WorkerParent = InWorkerParent;
	SetFileHandle(LogDirectory, EpisodeId);
}

// Called to write the data
void FSLWorldStateWriterJson::WriteData()
{
	// Json root object
	TSharedPtr<FJsonObject> JsonRootObj = MakeShareable(new FJsonObject);

	// Json array of entities
	TArray<TSharedPtr<FJsonValue>> JsonEntitiesArr;
	
	// Add actors data to the json array
	FSLWorldStateWriterJson::AddActors(JsonEntitiesArr);

	// Add components data to the json array
	FSLWorldStateWriterJson::AddComponents(JsonEntitiesArr);

	// Avoid appending empty entries
	if (JsonEntitiesArr.Num() > 0)
	{
		// Set timestamp
		JsonRootObj->SetNumberField("timestamp", WorkerParent->World->GetTimeSeconds());

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

// Add actors
void FSLWorldStateWriterJson::AddActors(TArray<TSharedPtr<FJsonValue>>& OutJsonEntitiesArr)
{
	// Iterate actors
	for (auto WorldStateActItr(WorkerParent->WorldStateActors.CreateIterator()); WorldStateActItr; ++WorldStateActItr)
	{
		// Check if pointer is valid
		if (WorldStateActItr->Entity.IsValid(/*false, true*/))
		{
			// Check if the entity moved more than the threshold
			const FVector CurrLoc = WorldStateActItr->Entity->GetActorLocation();
			const FQuat CurrQuat = WorldStateActItr->Entity->GetActorQuat();
			if (FVector::DistSquared(CurrLoc, WorldStateActItr->PrevLoc) > WorkerParent->DistanceStepSizeSquared)
			{
				// Update prev location
				WorldStateActItr->PrevLoc = CurrLoc;

				// Get current entry as json object
				TSharedPtr<FJsonObject> JsonActorEntry = FSLWorldStateWriterJson::GetAsJsonEntry(
					WorldStateActItr->Id, WorldStateActItr->Class, CurrLoc, CurrQuat);

				// If actor is skeletal, save bones data as well
				if (ASkeletalMeshActor* SkelAct = Cast<ASkeletalMeshActor>(WorldStateActItr->Entity))
				{
					// Json array of bones
					TArray<TSharedPtr<FJsonValue>> JsonBonesArr;

					// Get skeletal mesh component
					USkeletalMeshComponent* SkelComp = SkelAct->GetSkeletalMeshComponent();

					// Get bone names
					TArray<FName> BoneNames;
					SkelComp->GetBoneNames(BoneNames);

					// Iterate through the bones of the skeletal mesh
					for (const auto& BoneName : BoneNames)
					{
						const FVector CurrLoc = SkelComp->GetBoneLocation(BoneName);
						const FQuat CurrQuat = SkelComp->GetBoneQuaternion(BoneName);

						// Get current entry as json object
						TSharedPtr<FJsonObject> JsonBoneEntry = FSLWorldStateWriterJson::GetAsJsonEntry(
							TEXT(""), BoneName.ToString(), CurrLoc, CurrQuat);

						// Add bone to Json array
						JsonBonesArr.Add(MakeShareable(new FJsonValueObject(JsonBoneEntry)));
					}
					// Add bones to Json actor
					JsonActorEntry->SetArrayField("bones", JsonBonesArr);
				}
				// Add entity to json array
				OutJsonEntitiesArr.Add(MakeShareable(new FJsonValueObject(JsonActorEntry)));
			}
		}
		else
		{
			WorldStateActItr.RemoveCurrent();
		}
	}
}

// Add components
void FSLWorldStateWriterJson::AddComponents(TArray<TSharedPtr<FJsonValue>>& OutJsonEntitiesArr)
{
	// Iterate components
	for (auto WorldStateCompItr(WorkerParent->WorldStateComponents.CreateIterator()); WorldStateCompItr; ++WorldStateCompItr)
	{
		if (WorldStateCompItr->Entity.IsValid(/*false, true*/))
		{
			// Check if the entity moved more than the threshold
			const FVector CurrLoc = WorldStateCompItr->Entity->GetComponentLocation();
			const FQuat CurrQuat = WorldStateCompItr->Entity->GetComponentQuat();
			if (FVector::DistSquared(CurrLoc, WorldStateCompItr->PrevLoc) > WorkerParent->DistanceStepSizeSquared)
			{
				// Update prev location
				WorldStateCompItr->PrevLoc = CurrLoc;

				// Get current entry as json object
				TSharedPtr<FJsonObject> JsonCompEntry = FSLWorldStateWriterJson::GetAsJsonEntry(
					WorldStateCompItr->Id, WorldStateCompItr->Class, CurrLoc, CurrQuat);

				// If comp is skeletal, save bones data as well
				if (USkeletalMeshComponent* SkelComp = Cast<USkeletalMeshComponent>(WorldStateCompItr->Entity))
				{
					// Json array of bones
					TArray<TSharedPtr<FJsonValue>> JsonBonesArr;

					// Get bone names
					TArray<FName> BoneNames;
					SkelComp->GetBoneNames(BoneNames);

					// Iterate through the bones of the skeletal mesh
					for (const auto& BoneName : BoneNames)
					{
						const FVector CurrLoc = SkelComp->GetBoneLocation(BoneName);
						const FQuat CurrQuat = SkelComp->GetBoneQuaternion(BoneName);

						// Get current entry as json object
						TSharedPtr<FJsonObject> JsonBoneEntry = FSLWorldStateWriterJson::GetAsJsonEntry(
							TEXT(""), BoneName.ToString(), CurrLoc, CurrQuat);

						// Add bone to Json array
						JsonBonesArr.Add(MakeShareable(new FJsonValueObject(JsonBoneEntry)));
					}
					// Add bones to Json actor
					JsonCompEntry->SetArrayField("bones", JsonBonesArr);
				}
				// Add entity to json array
				OutJsonEntitiesArr.Add(MakeShareable(new FJsonValueObject(JsonCompEntry)));
			}
		}
		else
		{
			WorldStateCompItr.RemoveCurrent();
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