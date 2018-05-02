// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLRawDataWriterJson.h"
#include "Animation/SkeletalMeshActor.h"
#include "HAL/PlatformFilemanager.h"
#include "Conversions.h"

// Constr
FSLRawDataWriterJson::FSLRawDataWriterJson()
{
}

// Constr with Init
FSLRawDataWriterJson::FSLRawDataWriterJson(FSLRawDataAsyncWorker* InWorkerParent, const FString& LogDirectory, const FString& EpisodeId)
{
	Init(InWorkerParent, LogDirectory, EpisodeId);
}

// Destr
FSLRawDataWriterJson::~FSLRawDataWriterJson()
{
	if (FileHandle)
	{
		delete FileHandle;
	}
}

// Init
void FSLRawDataWriterJson::Init(FSLRawDataAsyncWorker* InWorkerParent, const FString& LogDirectory, const FString& EpisodeId)
{
	WorkerParent = InWorkerParent;
	SetFileHandle(LogDirectory, EpisodeId);
}

// Called to write the data
void FSLRawDataWriterJson::WriteData()
{
	// Json root object
	TSharedPtr<FJsonObject> JsonRootObj = MakeShareable(new FJsonObject);

	// Json array of entities
	TArray<TSharedPtr<FJsonValue>> JsonEntitiesArr;
	
	// Add actors data to the json array
	FSLRawDataWriterJson::AddActors(JsonEntitiesArr);

	// Add components data to the json array
	FSLRawDataWriterJson::AddComponents(JsonEntitiesArr);

	// Avoid appending empty entries
	if (JsonEntitiesArr.Num() > 0)
	{
		// Set timestamp
		JsonRootObj->SetNumberField("timestamp", WorkerParent->World->GetTimeSeconds());

		// Add actors to Json root
		JsonRootObj->SetArrayField("entities", JsonEntitiesArr);

		// Write entry to file
		FSLRawDataWriterJson::WriteToFile(JsonRootObj);
	}
}

// Set the file handle for the logger
void FSLRawDataWriterJson::SetFileHandle(const FString& LogDirectory, const FString& InEpisodeId)
{
	const FString Filename = TEXT("RawData_") + InEpisodeId + TEXT(".json");
	FString EpisodesDirPath = FPaths::ProjectDir() + LogDirectory + TEXT("/Episodes/");
	FPaths::RemoveDuplicateSlashes(EpisodesDirPath);

	const FString FilePath = EpisodesDirPath + Filename;

	// Create logging directory path and the filehandle
	FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*EpisodesDirPath);
	FileHandle = FPlatformFileManager::Get().GetPlatformFile().OpenWrite(*FilePath, true);
}

// Add actors
void FSLRawDataWriterJson::AddActors(TArray<TSharedPtr<FJsonValue>>& OutJsonEntitiesArr)
{
	// Iterate actors
	for (auto RawDataActItr(WorkerParent->RawDataActors.CreateIterator()); RawDataActItr; ++RawDataActItr)
	{
		// Check if pointer is valid
		if (RawDataActItr->Entity.IsValid(/*false, true*/))
		{
			// Check if the entity moved more than the threshold
			const FVector CurrLoc = RawDataActItr->Entity->GetActorLocation();
			const FQuat CurrQuat = RawDataActItr->Entity->GetActorQuat();
			if (FVector::DistSquared(CurrLoc, RawDataActItr->PrevLoc) > WorkerParent->DistanceSquaredThreshold)
			{
				// Update prev location
				RawDataActItr->PrevLoc = CurrLoc;

				// Get current entry as json object
				TSharedPtr<FJsonObject> JsonActorEntry = FSLRawDataWriterJson::GetAsJsonEntry(
					RawDataActItr->Id, RawDataActItr->Class, CurrLoc, CurrQuat);

				// If actor is skeletal, save bones data as well
				if (ASkeletalMeshActor* SkelAct = Cast<ASkeletalMeshActor>(RawDataActItr->Entity))
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
						TSharedPtr<FJsonObject> JsonBoneEntry = FSLRawDataWriterJson::GetAsJsonEntry(
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
			RawDataActItr.RemoveCurrent();
		}
	}
}

// Add components
void FSLRawDataWriterJson::AddComponents(TArray<TSharedPtr<FJsonValue>>& OutJsonEntitiesArr)
{
	// Iterate components
	for (auto RawDataCompItr(WorkerParent->RawDataComponents.CreateIterator()); RawDataCompItr; ++RawDataCompItr)
	{
		if (RawDataCompItr->Entity.IsValid(/*false, true*/))
		{
			// Check if the entity moved more than the threshold
			const FVector CurrLoc = RawDataCompItr->Entity->GetComponentLocation();
			const FQuat CurrQuat = RawDataCompItr->Entity->GetComponentQuat();
			if (FVector::DistSquared(CurrLoc, RawDataCompItr->PrevLoc) > WorkerParent->DistanceSquaredThreshold)
			{
				// Update prev location
				RawDataCompItr->PrevLoc = CurrLoc;

				// Get current entry as json object
				TSharedPtr<FJsonObject> JsonCompEntry = FSLRawDataWriterJson::GetAsJsonEntry(
					RawDataCompItr->Id, RawDataCompItr->Class, CurrLoc, CurrQuat);

				// If comp is skeletal, save bones data as well
				if (USkeletalMeshComponent* SkelComp = Cast<USkeletalMeshComponent>(RawDataCompItr->Entity))
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
						TSharedPtr<FJsonObject> JsonBoneEntry = FSLRawDataWriterJson::GetAsJsonEntry(
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
			RawDataCompItr.RemoveCurrent();
		}
	}
}

// Get entry as json object
TSharedPtr<FJsonObject> FSLRawDataWriterJson::GetAsJsonEntry(const FString& InId,
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
void FSLRawDataWriterJson::WriteToFile(const TSharedPtr<FJsonObject>& InRootObj)
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