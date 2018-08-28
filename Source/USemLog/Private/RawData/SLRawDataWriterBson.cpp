// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "RawData/SLRawDataWriterBson.h"
#include "Animation/SkeletalMeshActor.h"
#include "HAL/PlatformFilemanager.h"
#include "Conversions.h"

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
#if WITH_LIBMONGO
	// Bson root object
	bson_t* BsonRootObj=bson_new();

	// Bson Array of entities
	bson_t BsonEntitiesArr;
	bson_init(&BsonEntitiesArr);
	
	// Add actors data to the bson array
	FSLRawDataWriterBson::AddActors(BsonEntitiesArr);
	
	// Add components data to the bson array
	FSLRawDataWriterBson::AddComponents(BsonEntitiesArr);

	// Avoid appending empty entries
	if (sizeof(BsonEntitiesArr)>0)
	{	
		// Bson Write Buffer
		bson_writer_t * BsonWriter;
		uint8_t* buf = NULL;
		size_t buflen = 0;

		// create bson writer
		BsonWriter = bson_writer_new(&buf, &buflen, 0, bson_realloc_ctx, NULL);

		// begin writing with bson writer
		bool r = bson_writer_begin(BsonWriter, &BsonRootObj);

		// set timestamp
		BSON_APPEND_DOUBLE(BsonRootObj, "timestamp", WorkerParent->World->GetTimeSeconds());
		
		// add entity array to root oject
		BSON_APPEND_ARRAY(BsonRootObj, "entities", &BsonEntitiesArr);

				
		// finish writing into memory
		bson_writer_end(BsonWriter);

		// write memory to file
		FSLRawDataWriterBson::WriteData((uint8*)buf, (int64)buflen);

		//free bson writer buffer
		bson_free(buf);		
	}
#endif //WITH_LIBMONGO
}

// Set the file handle for the logger
void FSLRawDataWriterBson::SetFileHandle(const FString& LogDirectory, const FString& InEpisodeId)
{
	const FString Filename = TEXT("RawData_") + InEpisodeId + TEXT(".bson");
	FString EpisodesDirPath = FPaths::ProjectDir() + LogDirectory + TEXT("/Episodes/");
	FPaths::RemoveDuplicateSlashes(EpisodesDirPath);

	const FString FilePath = EpisodesDirPath + Filename;

	// Create logging directory path and the filehandle
	FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*EpisodesDirPath);
	FileHandle = FPlatformFileManager::Get().GetPlatformFile().OpenWrite(*FilePath, true);
}
#if WITH_LIBMONGO
// Add actors
void FSLRawDataWriterBson::AddActors(bson_t& OutBsonEntitiesArr)
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
				bson_t BsonActorEntry = FSLRawDataWriterBson::GetAsBsonEntry(
					RawDataActItr->Id, RawDataActItr->Class, CurrLoc, CurrQuat);
				
				// If actor is skeletal, save bones data as well
				if (ASkeletalMeshActor* SkelAct = Cast<ASkeletalMeshActor>(RawDataActItr->Entity))
				{
					// Bson array of bones
					bson_t BsonBonesArr;
					bson_init(&BsonBonesArr);

					// automatic key generation 
					const char* key;
					char keybuf[16];
					size_t keylen;
					int keynum = 0;

					// Get skeletal mesh component
					USkeletalMeshComponent* SkelComp = SkelAct->GetSkeletalMeshComponent();

					// Get bone names
					TArray<FName> BoneNames;
					SkelComp->GetBoneNames(BoneNames);

					BSON_APPEND_ARRAY_BEGIN(&BsonActorEntry, "bones", &BsonBonesArr);

					// Iterate through the bones of the skeletal mesh
					for (const auto& BoneName : BoneNames)
					{
						const FVector CurrLoc = SkelComp->GetBoneLocation(BoneName);
						const FQuat CurrQuat = SkelComp->GetBoneQuaternion(BoneName);

						// Get current entry as Bson object
						bson_t BsonBoneEntry = FSLRawDataWriterBson::GetAsBsonEntry(
							TEXT(""), BoneName.ToString(), CurrLoc, CurrQuat);
						
						keylen = bson_uint32_to_string(keynum, &key, keybuf, sizeof keybuf);
						bson_append_document(&BsonBonesArr, key, keylen, &BsonBoneEntry);
						keynum++;						

					}
					bson_append_array_end(&BsonActorEntry, &BsonBonesArr);					
																		
				}
				// Add entity to Bson array
				//OutBsonEntitiesArr.Add();
				bson_t BsonActorEntrydocument;
				bson_init(&BsonActorEntrydocument);
				BSON_APPEND_DOCUMENT(&BsonActorEntrydocument, "document", &BsonActorEntry);
				bson_concat(&OutBsonEntitiesArr, &BsonActorEntrydocument);
			}
		}
		else
		{
			RawDataActItr.RemoveCurrent();
		}
	}
}

// Add components
void FSLRawDataWriterBson::AddComponents(bson_t& OutBsonEntitiesArr)
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
				bson_t BsonCompEntry = FSLRawDataWriterBson::GetAsBsonEntry(
					RawDataCompItr->Id, RawDataCompItr->Class, CurrLoc, CurrQuat);
				
				// If comp is skeletal, save bones data as well
				if (USkeletalMeshComponent* SkelComp = Cast<USkeletalMeshComponent>(RawDataCompItr->Entity))
				{
					// Bson array of bones
					bson_t BsonBonesArr;
					bson_init(&BsonBonesArr);

					// automatic key generation 
					const char* key;
					char keybuf[16];
					size_t keylen;
					int keynum = 0;

					// Get bone names
					TArray<FName> BoneNames;
					SkelComp->GetBoneNames(BoneNames);

					BSON_APPEND_ARRAY_BEGIN(&BsonCompEntry, "bones", &BsonBonesArr);

					// Iterate through the bones of the skeletal mesh
					for (const auto& BoneName : BoneNames)
					{
						const FVector CurrLoc = SkelComp->GetBoneLocation(BoneName);
						const FQuat CurrQuat = SkelComp->GetBoneQuaternion(BoneName);

						// Get current entry as Bson object
						bson_t BsonBoneEntry = FSLRawDataWriterBson::GetAsBsonEntry(
							TEXT(""), BoneName.ToString(), CurrLoc, CurrQuat);


						keylen = bson_uint32_to_string(keynum, &key, keybuf, sizeof keybuf);
						bson_append_document(&BsonBonesArr, key, keylen, &BsonBoneEntry);
						keynum++;
						
					}
					bson_append_array_end(&BsonCompEntry, &BsonBonesArr);

				}
				// Add entity to Bson array
				bson_t BsonCompEntrydocument;
				bson_init(&BsonCompEntrydocument);
				BSON_APPEND_DOCUMENT(&BsonCompEntrydocument, "document", &BsonCompEntry);
				bson_concat(&OutBsonEntitiesArr, &BsonCompEntrydocument);
			}
		}
		else
		{
			RawDataCompItr.RemoveCurrent();
		}
	}
}

// Get entry as Bson object
bson_t FSLRawDataWriterBson::GetAsBsonEntry(const FString& InId,
	const FString& InClass,
	const FVector& InLoc,
	const FQuat& InQuat)
{
	// Switch to right handed ROS transformation
	const FVector ROSLoc = FConversions::UToROS(InLoc);
	const FQuat ROSQuat = FConversions::UToROS(InQuat);

	// New Bson entity object
	bson_t BsonObj;
	bson_init(&BsonObj);

	// Add "id" field if available (bones have no separate ids)
	if (!InId.IsEmpty())
	{
		BSON_APPEND_UTF8(&BsonObj, "id", TCHAR_TO_UTF8(*InId));
	}

	// Add "class" field
	BSON_APPEND_UTF8(&BsonObj, "class", TCHAR_TO_UTF8(*InClass));

	// Create and add "loc" field
	bson_t loc;
	bson_init(&loc);
	BSON_APPEND_DOUBLE(&loc, "x", ROSLoc.X);
	BSON_APPEND_DOUBLE(&loc, "y", ROSLoc.Y);
	BSON_APPEND_DOUBLE(&loc, "z", ROSLoc.Z);
	BSON_APPEND_DOCUMENT(&BsonObj, "loc", &loc);

	// Create and add "rot" field
	bson_t rot;
	bson_init(&rot);
	BSON_APPEND_DOUBLE(&rot, "x", ROSQuat.X);
	BSON_APPEND_DOUBLE(&rot, "y", ROSQuat.Y);
	BSON_APPEND_DOUBLE(&rot, "z", ROSQuat.Z);
	BSON_APPEND_DOCUMENT(&BsonObj, "rot", &rot);

	return BsonObj;
}

// Write entry to file
void FSLRawDataWriterBson::WriteData(uint8* memorybuffer, int64 bufferlen)
{

	if (FileHandle)
	{
		FileHandle->Write(memorybuffer, bufferlen);
	}
}
#endif //WITH_LIBMONGO
