// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLRawDataAsyncWorker.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"
#include "Animation/SkeletalMeshActor.h"
#include "Conversions.h"

// Constructor
FSLRawDataAsyncWorker::FSLRawDataAsyncWorker()
{
	UE_LOG(LogTemp, Error, TEXT("[%s][%d]"), TEXT(__FUNCTION__), __LINE__);
}

// Destructor
FSLRawDataAsyncWorker::~FSLRawDataAsyncWorker()
{
	UE_LOG(LogTemp, Error, TEXT("[%s][%d]"), TEXT(__FUNCTION__), __LINE__);
	if (FileHandle)
	{
		delete FileHandle;
	}
}

// Init worker, load models to log from world
void FSLRawDataAsyncWorker::Init(UWorld* InWorld, const float DistanceThreshold)
{
	// Bind function pointer to default
	LogToFunctionPointer = &FSLRawDataAsyncWorker::LogTo_Default;

	// Set pointer to world (access to current timestamp)
	World = InWorld;

	// Set the square of the distance threshold for objects to be logged
	DistanceSquaredThreshold = DistanceThreshold * DistanceThreshold;

	// Get all objects with the SemLog tag type 
	TMap<UObject*, TMap<FString, FString>> ObjsToKeyValuePairs =
		FTags::GetObjectsToKeyValuePairs(InWorld, TEXT("SemLog"));

	// Add static and dynamic objects with transform data
	for (const auto& ObjToKVP : ObjsToKeyValuePairs)
	{
		// Take into account only objects with an id and class value set
		if (ObjToKVP.Value.Contains("Id") && ObjToKVP.Value.Contains("Class"))
		{
			const FString Id = ObjToKVP.Value["Id"];
			const FString Class = ObjToKVP.Value["Class"];
			// Take into account only objects with transform data)
			if (AActor* ObjAsActor = Cast<AActor>(ObjToKVP.Key))
			{
				//RawDataActors.Add(FSLRawDataActor(TWeakObjectPtr<AActor>(ObjAsActor), Id));
				RawDataActors.Add(TSLRawDataEntity<AActor>(ObjAsActor, Id, Class));
			}
			else if (USceneComponent* ObjAsComp = Cast<USceneComponent>(ObjToKVP.Key))
			{
				RawDataComponents.Add(TSLRawDataEntity<USceneComponent>(ObjAsComp, Id, Class));
			}
		}
	}

	UE_LOG(LogTemp, Error, TEXT("[%s] Log message"), *(InWorld->GetName()));
}

// Log data to json file
void FSLRawDataAsyncWorker::SetLogToJson(const FString& InLogDirectory, const FString& InEpisodeId)
{
	SetFileHandle(InLogDirectory, InEpisodeId, TEXT("json"));
	LogToFunctionPointer = &FSLRawDataAsyncWorker::LogTo_Json;
}

// Log data to bson file
void FSLRawDataAsyncWorker::SetLogToBson(const FString& InLogDirectory, const FString& InEpisodeId)
{
	LogToFunctionPointer = &FSLRawDataAsyncWorker::LogTo_Bson;
}

// Log data to mongodb
void FSLRawDataAsyncWorker::SetLogToMongo(const FString& InLogDB, const FString& InEpisodeId, const FString& InMongoIP, uint16 MongoPort)
{
	LogToFunctionPointer = &FSLRawDataAsyncWorker::LogTo_Mongo;
}

// Remove all non-dynamic objects from arrays
void FSLRawDataAsyncWorker::RemoveAllNonDynamicObjects()
{
	// Remove static/invalid actors
	for (auto RawDataActItr(RawDataActors.CreateIterator()); RawDataActItr; ++RawDataActItr)
	{
		if (RawDataActItr->Entity.IsValid())
		{
			if (!FTags::HasKeyValuePair(RawDataActItr->Entity.Get(), "SemLog", "LogType", "Dynamic"))
			{
				RawDataActItr.RemoveCurrent();
			}
		}
		else
		{
			RawDataActItr.RemoveCurrent();
		}
	}
	RawDataActors.Shrink();

	// Remove static/invalid components
	for (auto RawDataCompItr(RawDataComponents.CreateIterator()); RawDataCompItr; ++RawDataCompItr)
	{
		if (RawDataCompItr->Entity.IsValid())
		{
			if (!FTags::HasKeyValuePair(RawDataCompItr->Entity.Get(), "SemLog", "LogType", "Dynamic"))
			{
				RawDataCompItr.RemoveCurrent();
			}
		}
		else
		{
			RawDataCompItr.RemoveCurrent();
		}
	}
	RawDataComponents.Shrink();
}

// Async work done here
void FSLRawDataAsyncWorker::DoWork()
{
	(this->*LogToFunctionPointer)();
}

// Needed by the engine API
FORCEINLINE TStatId FSLRawDataAsyncWorker::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(FSLRawDataAsyncWorker, STATGROUP_ThreadPoolAsyncTasks);
}

// Set the file handle for the logger
void FSLRawDataAsyncWorker::SetFileHandle(const FString& LogDirectory, const FString& InEpisodeId, const FString& InFileExt)
{
	const FString Filename = TEXT("RawData_") + InEpisodeId + TEXT(".") + InFileExt;
	FString EpisodesDirPath = FPaths::ProjectDir() + LogDirectory + TEXT("/Episodes/");
	FPaths::RemoveDuplicateSlashes(EpisodesDirPath);

	const FString FilePath = EpisodesDirPath + Filename;

	// Create logging directory path and the filehandle
	FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*EpisodesDirPath);
	FileHandle = FPlatformFileManager::Get().GetPlatformFile().OpenWrite(*FilePath, true);
}

// Log to default
void FSLRawDataAsyncWorker::LogTo_Default()
{

}

// Log to json
void FSLRawDataAsyncWorker::LogTo_Json()
{
	// Json root object
	TSharedPtr<FJsonObject> JsonRootObj = MakeShareable(new FJsonObject);

	// Json array of entities
	TArray<TSharedPtr<FJsonValue>> JsonEntitiesArr;

	// Iterate actors
	for (auto RawDataActItr(RawDataActors.CreateIterator()); RawDataActItr; ++RawDataActItr)
	{
		// Check if pointer is valid
		if (RawDataActItr->Entity.IsValid(/*false, true*/))
		{
			// Check if the entity moved more than the threshold
			const FVector CurrLoc = RawDataActItr->Entity->GetActorLocation();
			const FQuat CurrQuat = RawDataActItr->Entity->GetActorQuat();
			if (FVector::DistSquared(CurrLoc, RawDataActItr->PrevLoc) > DistanceSquaredThreshold)
			{
				// Update prev location
				RawDataActItr->PrevLoc = CurrLoc;

				// Get current entry as json object
				TSharedPtr<FJsonObject> JsonActorEntry = FSLRawDataAsyncWorker::GetAsJsonEntry(
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
						TSharedPtr<FJsonObject> JsonBoneEntry = FSLRawDataAsyncWorker::GetAsJsonEntry(
							TEXT(""), BoneName.ToString(), CurrLoc, CurrQuat);

						// Add bone to Json array
						JsonBonesArr.Add(MakeShareable(new FJsonValueObject(JsonBoneEntry)));
					}
					// Add bones to Json actor
					JsonActorEntry->SetArrayField("bones", JsonBonesArr);
				}
				// Add entity to json array
				JsonEntitiesArr.Add(MakeShareable(new FJsonValueObject(JsonActorEntry)));
			}
		}
		else
		{
			RawDataActItr.RemoveCurrent();
		}
	}

	// Iterate components
	for (auto RawDataCompItr(RawDataComponents.CreateIterator()); RawDataCompItr; ++RawDataCompItr)
	{
		if (RawDataCompItr->Entity.IsValid(/*false, true*/))
		{
			// Check if the entity moved more than the threshold
			const FVector CurrLoc = RawDataCompItr->Entity->GetComponentLocation();
			const FQuat CurrQuat = RawDataCompItr->Entity->GetComponentQuat();
			if (FVector::DistSquared(CurrLoc, RawDataCompItr->PrevLoc) > DistanceSquaredThreshold)
			{
				// Update prev location
				RawDataCompItr->PrevLoc = CurrLoc;

				// Get current entry as json object
				TSharedPtr<FJsonObject> JsonCompEntry = FSLRawDataAsyncWorker::GetAsJsonEntry(
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
						TSharedPtr<FJsonObject> JsonBoneEntry = FSLRawDataAsyncWorker::GetAsJsonEntry(
							TEXT(""), BoneName.ToString(), CurrLoc, CurrQuat);

						// Add bone to Json array
						JsonBonesArr.Add(MakeShareable(new FJsonValueObject(JsonBoneEntry)));
					}
					// Add bones to Json actor
					JsonCompEntry->SetArrayField("bones", JsonBonesArr);
				}
				// Add entity to json array
				JsonEntitiesArr.Add(MakeShareable(new FJsonValueObject(JsonCompEntry)));
			}
		}
		else
		{
			RawDataCompItr.RemoveCurrent();
		}
	}
	
	// Avoid appending empty entries
	if (JsonEntitiesArr.Num() > 0)
	{
		// Set timestamp
		JsonRootObj->SetNumberField("timestamp", World->GetTimeSeconds());

		// Add actors to Json root
		JsonRootObj->SetArrayField("entities", JsonEntitiesArr);

		// Transform to string
		FString JsonString;
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
		FJsonSerializer::Serialize(JsonRootObj.ToSharedRef(), Writer);

		// Write to file
		if (FileHandle)
		{
			FileHandle->Write((const uint8*)TCHAR_TO_ANSI(*JsonString), JsonString.Len());
		}
	}
}

// Log to bson
void FSLRawDataAsyncWorker::LogTo_Bson()
{

}

// Log to mongo
void FSLRawDataAsyncWorker::LogTo_Mongo()
{

}

// Get entry as json object
TSharedPtr<FJsonObject> FSLRawDataAsyncWorker::GetAsJsonEntry(const FString& InId,
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
