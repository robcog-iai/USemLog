// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLRawDataWriterBson.h"
#include "SLRawDataAsyncWorker.h"

// Constr
FSLRawDataWriterBson::FSLRawDataWriterBson()
{
}

// Constr with Init
FSLRawDataWriterBson::FSLRawDataWriterBson(FSLRawDataAsyncWorker* InWorkerParent, const FString& LogDirectory, const FString& EpisodeId)
{
	Init(InWorkerParent, LogDirectory, EpisodeId);
}

// Destr
FSLRawDataWriterBson::~FSLRawDataWriterBson()
{
	if (FileHandle)
	{
		delete FileHandle;
	}
}

// Init
void FSLRawDataWriterBson::Init(FSLRawDataAsyncWorker* InWorkerParent, const FString& LogDirectory, const FString& EpisodeId)
{
	WorkerParent = InWorkerParent;
	SetFileHandle(LogDirectory, EpisodeId);
}

// Called to write the data
void FSLRawDataWriterBson::WriteData()
{
	// Bson root object
	TSharedPtr<FJsonObject> BsonRootObj = MakeShareable(new FJsonObject);

	// Bson array of entities
	TArray<TSharedPtr<FJsonValue>> BsonEntitiesArr;
	
	// Add actors data to the Bson array
	FSLRawDataWriterBson::AddActors(BsonEntitiesArr);

	// Add components data to the Bson array
	FSLRawDataWriterBson::AddComponents(BsonEntitiesArr);

	// Avoid appending empty entries
	if (BsonEntitiesArr.Num() > 0)
	{
		// Set timestamp
		BsonRootObj->SetNumberField("timestamp", WorkerParent->World->GetTimeSeconds());

		// Add actors to Bson root
		BsonRootObj->SetArrayField("entities", BsonEntitiesArr);

		// Write entry to file
		FSLRawDataWriterBson::WriteToFile(BsonRootObj);
	}
}

// Set the file handle for the logger
void FSLRawDataWriterBson::SetFileHandle(const FString& LogDirectory, const FString& InEpisodeId)
{
	const FString Filename = TEXT("RawData_") + InEpisodeId + TEXT(".Bson");
	FString EpisodesDirPath = FPaths::ProjectDir() + LogDirectory + TEXT("/Episodes/");
	FPaths::RemoveDuplicateSlashes(EpisodesDirPath);

	const FString FilePath = EpisodesDirPath + Filename;

	// Create logging directory path and the filehandle
	FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*EpisodesDirPath);
	FileHandle = FPlatformFileManager::Get().GetPlatformFile().OpenWrite(*FilePath, true);
}

// Add actors
void FSLRawDataWriterBson::AddActors(TArray<TSharedPtr<FJsonValue>>& OutBsonEntitiesArr)
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

				// Get current entry as Bson object
				TSharedPtr<FJsonObject> BsonActorEntry = FSLRawDataWriterBson::GetAsBsonEntry(
					RawDataActItr->Id, RawDataActItr->Class, CurrLoc, CurrQuat);

				// If actor is skeletal, save bones data as well
				if (ASkeletalMeshActor* SkelAct = Cast<ASkeletalMeshActor>(RawDataActItr->Entity))
				{
					// Bson array of bones
					TArray<TSharedPtr<FJsonValue>> BsonBonesArr;

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

						// Get current entry as Bson object
						TSharedPtr<FJsonObject> BsonBoneEntry = FSLRawDataWriterBson::GetAsBsonEntry(
							TEXT(""), BoneName.ToString(), CurrLoc, CurrQuat);

						// Add bone to Bson array
						BsonBonesArr.Add(MakeShareable(new FJsonValueObject(BsonBoneEntry)));
					}
					// Add bones to Bson actor
					BsonActorEntry->SetArrayField("bones", BsonBonesArr);
				}
				// Add entity to Bson array
				OutBsonEntitiesArr.Add(MakeShareable(new FJsonValueObject(BsonActorEntry)));
			}
		}
		else
		{
			RawDataActItr.RemoveCurrent();
		}
	}
}

// Add components
void FSLRawDataWriterBson::AddComponents(TArray<TSharedPtr<FJsonValue>>& OutBsonEntitiesArr)
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

				// Get current entry as Bson object
				TSharedPtr<FJsonObject> BsonCompEntry = FSLRawDataWriterBson::GetAsBsonEntry(
					RawDataCompItr->Id, RawDataCompItr->Class, CurrLoc, CurrQuat);

				// If comp is skeletal, save bones data as well
				if (USkeletalMeshComponent* SkelComp = Cast<USkeletalMeshComponent>(RawDataCompItr->Entity))
				{
					// Bson array of bones
					TArray<TSharedPtr<FJsonValue>> BsonBonesArr;

					// Get bone names
					TArray<FName> BoneNames;
					SkelComp->GetBoneNames(BoneNames);

					// Iterate through the bones of the skeletal mesh
					for (const auto& BoneName : BoneNames)
					{
						const FVector CurrLoc = SkelComp->GetBoneLocation(BoneName);
						const FQuat CurrQuat = SkelComp->GetBoneQuaternion(BoneName);

						// Get current entry as Bson object
						TSharedPtr<FJsonObject> BsonBoneEntry = FSLRawDataWriterBson::GetAsBsonEntry(
							TEXT(""), BoneName.ToString(), CurrLoc, CurrQuat);

						// Add bone to Bson array
						BsonBonesArr.Add(MakeShareable(new FJsonValueObject(BsonBoneEntry)));
					}
					// Add bones to Bson actor
					BsonCompEntry->SetArrayField("bones", BsonBonesArr);
				}
				// Add entity to Bson array
				OutBsonEntitiesArr.Add(MakeShareable(new FJsonValueObject(BsonCompEntry)));
			}
		}
		else
		{
			RawDataCompItr.RemoveCurrent();
		}
	}
}

// Get entry as Bson object
TSharedPtr<FJsonObject> FSLRawDataWriterBson::GetAsBsonEntry(const FString& InId,
	const FString& InClass,
	const FVector& InLoc,
	const FQuat& InQuat)
{
	// Switch to right handed ROS transformation
	const FVector ROSLoc = FConversions::UToROS(InLoc);
	const FQuat ROSQuat = FConversions::UToROS(InQuat);

	// New Bson entity object
	TSharedPtr<FJsonObject> BsonObj = MakeShareable(new FJsonObject);

	// Add "id" field if available (bones have no separate ids)
	if (!InId.IsEmpty())
	{
		BsonObj->SetStringField("id", InId);
	}

	// Add "class" field
	BsonObj->SetStringField("class", InClass);

	// Create and add "loc" field
	TSharedPtr<FJsonObject> LocObj = MakeShareable(new FJsonObject);
	LocObj->SetNumberField("x", ROSLoc.X);
	LocObj->SetNumberField("y", ROSLoc.Y);
	LocObj->SetNumberField("z", ROSLoc.Z);
	BsonObj->SetObjectField("loc", LocObj);

	// Create and add "rot" field
	TSharedPtr<FJsonObject> QuatObj = MakeShareable(new FJsonObject);
	QuatObj->SetNumberField("x", ROSQuat.X);
	QuatObj->SetNumberField("y", ROSQuat.Y);
	QuatObj->SetNumberField("z", ROSQuat.Z);
	QuatObj->SetNumberField("w", ROSQuat.W);
	BsonObj->SetObjectField("rot", QuatObj);

	return BsonObj;
}

// Write entry to file
void FSLRawDataWriterBson::WriteToFile(const TSharedPtr<FJsonObject>& InRootObj)
{
	// Transform to string
	FString BsonString;
	TSharedRef<TJsonWriter<>> BsonWriter = TJsonWriterFactory<>::Create(&BsonString);
	FJsonSerializer::Serialize(InRootObj.ToSharedRef(), BsonWriter);

	// Write to file
	if (FileHandle)
	{
		FileHandle->Write((const uint8*)TCHAR_TO_ANSI(*BsonString), BsonString.Len());
	}
}