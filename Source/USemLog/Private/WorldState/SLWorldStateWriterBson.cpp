// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "WorldState/SLWorldStateWriterBson.h"
#include "Animation/SkeletalMeshActor.h"
#include "HAL/PlatformFilemanager.h"
#include "Conversions.h"

// Constr
FSLWorldStateWriterBson::FSLWorldStateWriterBson()
{
	bIsInit = false;
}

// Init constructor
FSLWorldStateWriterBson::FSLWorldStateWriterBson(const FSLWorldStateWriterParams& InParams)
{
	bIsInit = false;
	FSLWorldStateWriterBson::Init(InParams);
}

// Destr
FSLWorldStateWriterBson::~FSLWorldStateWriterBson()
{
	if (FileHandle)
	{
		delete FileHandle;
	}
}

// Init
void FSLWorldStateWriterBson::Init(const FSLWorldStateWriterParams& InParams)
{
	MinLinearDistanceSquared = InParams.LinearDistanceSquared;
	MinAngularDistance = InParams.AngularDistance;
	bIsInit = FSLWorldStateWriterBson::SetFileHandle(InParams.Location, InParams.EpisodeId);
}

// Finish
void FSLWorldStateWriterBson::Finish()
{
	if (bIsInit)
	{
		bIsInit = false;
	}
}

// Called to write the data
void FSLWorldStateWriterBson::Write(TArray<TSLItemState<AActor>>& NonSkeletalActorPool,
	TArray<TSLItemState<ASLSkeletalMeshActor>>& SkeletalActorPool,
	TArray<TSLItemState<USceneComponent>>& NonSkeletalComponentPool,
	float Timestamp)
{
#if SL_WITH_LIBMONGO

#endif //SL_WITH_LIBMONGO
}

// Set the file handle for the logger
bool FSLWorldStateWriterBson::SetFileHandle(const FString& LogDirectory, const FString& InEpisodeId)
{
	const FString Filename = InEpisodeId + TEXT("_WS.bson");
	FString EpisodesDirPath = FPaths::ProjectDir() + LogDirectory + TEXT("/Episodes/");
	FPaths::RemoveDuplicateSlashes(EpisodesDirPath);

	const FString FilePath = EpisodesDirPath + Filename;

	// Create logging directory path and the filehandle
	FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*EpisodesDirPath);
	FileHandle = FPlatformFileManager::Get().GetPlatformFile().OpenWrite(*FilePath, true);

	return FileHandle != nullptr;
}
#if SL_WITH_LIBMONGO
//// Add actors
//void FSLWorldStateWriterBson::AddActors(bson_t& OutBsonEntitiesArr)
//{
//	// Iterate actors
//	for (auto WorldStateActItr(WorkerParent->WorldStateActors.CreateIterator()); WorldStateActItr; ++WorldStateActItr)
//	{
//		// Check if pointer is valid
//		if (WorldStateActItr->Entity.IsValid(/*false, true*/))
//		{
//			// Check if the entity moved more than the threshold
//			const FVector CurrLoc = WorldStateActItr->Entity->GetActorLocation();
//			const FQuat CurrQuat = WorldStateActItr->Entity->GetActorQuat();
//			if (FVector::DistSquared(CurrLoc, WorldStateActItr->PrevLoc) > WorkerParent->LinearDistanceSquared)
//			{
//				// Update prev location
//				WorldStateActItr->PrevLoc = CurrLoc;
//
//				// Get current entry as Bson object
//				bson_t BsonActorEntry = FSLWorldStateWriterBson::GetAsBsonEntry(
//					WorldStateActItr->Id, WorldStateActItr->Class, CurrLoc, CurrQuat);
//				
//				// If actor is skeletal, save bones data as well
//				if (ASkeletalMeshActor* SkelAct = Cast<ASkeletalMeshActor>(WorldStateActItr->Entity))
//				{
//					// Bson array of bones
//					bson_t BsonBonesArr;
//					bson_init(&BsonBonesArr);
//
//					// automatic key generation 
//					const char* key;
//					char keybuf[16];
//					size_t keylen;
//					int keynum = 0;
//
//					// Get skeletal mesh component
//					USkeletalMeshComponent* SkelComp = SkelAct->GetSkeletalMeshComponent();
//
//					// Get bone names
//					TArray<FName> BoneNames;
//					SkelComp->GetBoneNames(BoneNames);
//
//					BSON_APPEND_ARRAY_BEGIN(&BsonActorEntry, "bones", &BsonBonesArr);
//
//					// Iterate through the bones of the skeletal mesh
//					for (const auto& BoneName : BoneNames)
//					{
//						const FVector CurrLoc = SkelComp->GetBoneLocation(BoneName);
//						const FQuat CurrQuat = SkelComp->GetBoneQuaternion(BoneName);
//
//						// Get current entry as Bson object
//						bson_t BsonBoneEntry = FSLWorldStateWriterBson::GetAsBsonEntry(
//							TEXT(""), BoneName.ToString(), CurrLoc, CurrQuat);
//						
//						keylen = bson_uint32_to_string(keynum, &key, keybuf, sizeof keybuf);
//						bson_append_document(&BsonBonesArr, key, keylen, &BsonBoneEntry);
//						keynum++;						
//
//					}
//					bson_append_array_end(&BsonActorEntry, &BsonBonesArr);					
//																		
//				}
//				// Add entity to Bson array
//				//OutBsonEntitiesArr.Add();
//				bson_t BsonActorEntrydocument;
//				bson_init(&BsonActorEntrydocument);
//				BSON_APPEND_DOCUMENT(&BsonActorEntrydocument, "document", &BsonActorEntry);
//				bson_concat(&OutBsonEntitiesArr, &BsonActorEntrydocument);
//			}
//		}
//		else
//		{
//			WorldStateActItr.RemoveCurrent();
//		}
//	}
//}
//
//// Add components
//void FSLWorldStateWriterBson::AddComponents(bson_t& OutBsonEntitiesArr)
//{
//	// Iterate components
//	for (auto WorldStateCompItr(WorkerParent->WorldStateComponents.CreateIterator()); WorldStateCompItr; ++WorldStateCompItr)
//	{
//		if (WorldStateCompItr->Entity.IsValid(/*false, true*/))
//		{
//			// Check if the entity moved more than the threshold
//			const FVector CurrLoc = WorldStateCompItr->Entity->GetComponentLocation();
//			const FQuat CurrQuat = WorldStateCompItr->Entity->GetComponentQuat();
//			if (FVector::DistSquared(CurrLoc, WorldStateCompItr->PrevLoc) > WorkerParent->LinearDistanceSquared)
//			{
//				// Update prev location
//				WorldStateCompItr->PrevLoc = CurrLoc;
//
//				// Get current entry as Bson object
//				bson_t BsonCompEntry = FSLWorldStateWriterBson::GetAsBsonEntry(
//					WorldStateCompItr->Id, WorldStateCompItr->Class, CurrLoc, CurrQuat);
//				
//				// If comp is skeletal, save bones data as well
//				if (USkeletalMeshComponent* SkelComp = Cast<USkeletalMeshComponent>(WorldStateCompItr->Entity))
//				{
//					// Bson array of bones
//					bson_t BsonBonesArr;
//					bson_init(&BsonBonesArr);
//
//					// automatic key generation 
//					const char* key;
//					char keybuf[16];
//					size_t keylen;
//					int keynum = 0;
//
//					// Get bone names
//					TArray<FName> BoneNames;
//					SkelComp->GetBoneNames(BoneNames);
//
//					BSON_APPEND_ARRAY_BEGIN(&BsonCompEntry, "bones", &BsonBonesArr);
//
//					// Iterate through the bones of the skeletal mesh
//					for (const auto& BoneName : BoneNames)
//					{
//						const FVector CurrLoc = SkelComp->GetBoneLocation(BoneName);
//						const FQuat CurrQuat = SkelComp->GetBoneQuaternion(BoneName);
//
//						// Get current entry as Bson object
//						bson_t BsonBoneEntry = FSLWorldStateWriterBson::GetAsBsonEntry(
//							TEXT(""), BoneName.ToString(), CurrLoc, CurrQuat);
//
//
//						keylen = bson_uint32_to_string(keynum, &key, keybuf, sizeof keybuf);
//						bson_append_document(&BsonBonesArr, key, keylen, &BsonBoneEntry);
//						keynum++;
//						
//					}
//					bson_append_array_end(&BsonCompEntry, &BsonBonesArr);
//
//				}
//				// Add entity to Bson array
//				bson_t BsonCompEntrydocument;
//				bson_init(&BsonCompEntrydocument);
//				BSON_APPEND_DOCUMENT(&BsonCompEntrydocument, "document", &BsonCompEntry);
//				bson_concat(&OutBsonEntitiesArr, &BsonCompEntrydocument);
//			}
//		}
//		else
//		{
//			WorldStateCompItr.RemoveCurrent();
//		}
//	}
//}
//
//// Get entry as Bson object
//bson_t FSLWorldStateWriterBson::GetAsBsonEntry(const FString& InId,
//	const FString& InClass,
//	const FVector& InLoc,
//	const FQuat& InQuat)
//{
//	// Switch to right handed ROS transformation
//	const FVector ROSLoc = FConversions::UToROS(InLoc);
//	const FQuat ROSQuat = FConversions::UToROS(InQuat);
//
//	// New Bson entity object
//	bson_t BsonObj;
//	bson_init(&BsonObj);
//
//	// Add "id" field if available (bones have no separate ids)
//	if (!InId.IsEmpty())
//	{
//		BSON_APPEND_UTF8(&BsonObj, "id", TCHAR_TO_UTF8(*InId));
//	}
//
//	// Add "class" field
//	BSON_APPEND_UTF8(&BsonObj, "class", TCHAR_TO_UTF8(*InClass));
//
//	// Create and add "loc" field
//	bson_t loc;
//	bson_init(&loc);
//	BSON_APPEND_DOUBLE(&loc, "x", ROSLoc.X);
//	BSON_APPEND_DOUBLE(&loc, "y", ROSLoc.Y);
//	BSON_APPEND_DOUBLE(&loc, "z", ROSLoc.Z);
//	BSON_APPEND_DOCUMENT(&BsonObj, "loc", &loc);
//
//	// Create and add "rot" field
//	bson_t rot;
//	bson_init(&rot);
//	BSON_APPEND_DOUBLE(&rot, "x", ROSQuat.X);
//	BSON_APPEND_DOUBLE(&rot, "y", ROSQuat.Y);
//	BSON_APPEND_DOUBLE(&rot, "z", ROSQuat.Z);
//	BSON_APPEND_DOCUMENT(&BsonObj, "rot", &rot);
//
//	return BsonObj;
//}
//
//// Write entry to file
//void FSLWorldStateWriterBson::WriteData(uint8* memorybuffer, int64 bufferlen)
//{
//
//	if (FileHandle)
//	{
//		FileHandle->Write(memorybuffer, bufferlen);
//	}
//}
#endif //SL_WITH_LIBMONGO
