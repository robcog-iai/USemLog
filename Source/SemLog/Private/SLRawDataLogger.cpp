// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLRawDataLogger.h"
#include "Animation/SkeletalMeshActor.h"
#include "TagStatics.h"


// Constructor
USLRawDataLogger::USLRawDataLogger()
{
	// Default values
	bIsInit = false;
	bLogToFile = false;
	bBroadcastData = false;
}

// Destructor
USLRawDataLogger::~USLRawDataLogger()
{
	if (FileHandle)
	{
		delete FileHandle;
	}
}

// Init logger
bool USLRawDataLogger::Init(UWorld* InWorld, const float DistanceThreshold)
{
	// Set the world
	World = InWorld;

	// Calculate the squared distance threshold (faster comparisons)
	SquaredDistanceThreshold = DistanceThreshold * DistanceThreshold;

	// Logger initialized
	bIsInit = (World != nullptr);
	return bIsInit;
}

// Set file handle for appending log data to file every update
void USLRawDataLogger::InitFileHandle(const FString EpisodeId, const FString LogDirectoryPath)
{
	// Create file handle to incrementally append json logs to file
	const FString Filename = "RawData_" + EpisodeId + ".json";
	const FString EpisodesDirPath = LogDirectoryPath.EndsWith("/") ?
		(LogDirectoryPath + "Episodes/") : (LogDirectoryPath + "/Episodes/");
	const FString FilePath = EpisodesDirPath + Filename;

	// Create logging directory path and the filehandle
	FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*EpisodesDirPath);
	FileHandle = FPlatformFileManager::Get().GetPlatformFile().OpenWrite(*FilePath, true);

	bLogToFile = (FileHandle != nullptr);
}

// Allow broadcasting the data as events
void USLRawDataLogger::InitBroadcaster()
{
	bBroadcastData = true;
}

// Log dynamic and static entities to file
void USLRawDataLogger::LogFirstEntry()
{
	// Get the dynamic entities data as json
	FString FristEntryJsonOutputString;
	if (USLRawDataLogger::GetAllEntitiesAsJson(FristEntryJsonOutputString))
	{
		// Append json to file 
		if (bLogToFile)
		{
			USLRawDataLogger::InsertJsonContentToFile(FristEntryJsonOutputString);
		}

		// Broadcast json
		if (bBroadcastData)
		{
			USLRawDataLogger::BroadcastJsonContent(FristEntryJsonOutputString);
		}
	}
}

// Log dynamic entities
void USLRawDataLogger::LogDynamicEntities()
{
	// Get the dynamic entities data as json
	FString DynamicJsonOutputString;
	if (USLRawDataLogger::GetDynamicEntitiesAsJson(DynamicJsonOutputString))
	{
		// Append json to file 
		if (bLogToFile)
		{
			USLRawDataLogger::InsertJsonContentToFile(DynamicJsonOutputString);
		}

		// Broadcast json
		if (bBroadcastData)
		{
			USLRawDataLogger::BroadcastJsonContent(DynamicJsonOutputString);
		}
	}
}

// Add new dynamic entity for logging
void USLRawDataLogger::AddNewDynamicEntity(AActor* Actor)
{
	int32 TagIndex = FTagStatics::GetTagTypeIndex(Actor, "SemLog");
	if (TagIndex != INDEX_NONE)
	{
		const FString Id = FTagStatics::GetKeyValue(Actor->Tags[TagIndex], "Id");
		const FString Class = FTagStatics::GetKeyValue(Actor->Tags[TagIndex], "Class");
		if (!Id.IsEmpty() && !Class.IsEmpty())
		{
			// Location is init automatically to -INF
			const FString UniqueName = Class + "_" + Id;
			FUniqueNameAndLocation UniqueNameAndInitLoc(UniqueName);

			// Store the UniqueName and the Location of the dynamic entity
			DynamicActorsWithData.Add(Actor,
				FUniqueNameAndLocation(UniqueName, Actor->GetActorLocation()));
		}
	}
}

// Remove dynamic entity from logging
void USLRawDataLogger::RemoveDynamicEntity(AActor* Actor)
{
	DynamicActorsWithData.Remove(Actor);
}

// Get the dynamic and static entities as json string
bool USLRawDataLogger::GetAllEntitiesAsJson(FString& FirstJsonEntry)
{
	if (!bIsInit)
	{
		return false;
	}

	// Create Json root object
	TSharedPtr<FJsonObject> JsonRootObj = MakeShareable(new FJsonObject);
	// Set timestamp
	JsonRootObj->SetNumberField("timestamp", World->GetTimeSeconds());
	// Json array of actors
	TArray< TSharedPtr<FJsonValue> > JsonActorArr;

	// Log static entities (logged only once at init)
	TArray<AActor*> StaticActors = FTagStatics::GetActorsWithKeyValuePair(
		World, "SemLog", "Runtime", "Static");

	for (const auto& ActItr : StaticActors)
	{
		int32 TagIndex = FTagStatics::GetTagTypeIndex(ActItr, "SemLog");
		if (TagIndex != INDEX_NONE)
		{
			const FString Id = FTagStatics::GetKeyValue(ActItr->Tags[TagIndex], "Id");
			const FString Class = FTagStatics::GetKeyValue(ActItr->Tags[TagIndex], "Class");
			if (!Id.IsEmpty() && !Class.IsEmpty())
			{
				// Location is init automatically to -INF
				FUniqueNameAndLocation UniqueNameAndInitLoc(Class + "_" + Id);
				USLRawDataLogger::AddActorToJsonArray(JsonActorArr, ActItr, UniqueNameAndInitLoc);
			}
		}
	}

	// Get all static components and cast them to USceneComponent
	TArray<UActorComponent*> StaticActorComponents = FTagStatics::GetComponentsWithKeyValuePair(
		World, "SemLog", "Runtime", "Static");
	// Array to cast to
	TArray<USceneComponent*> StaticSceneComponents;
	for (const auto& StaticActCompItr : StaticActorComponents)
	{
		if (StaticActCompItr->IsA(USceneComponent::StaticClass()))
		{
			StaticSceneComponents.Emplace(Cast<USceneComponent>(StaticActCompItr));
		}
	}

	for (const auto& CompItr : StaticSceneComponents)
	{
		int32 TagIndex = FTagStatics::GetTagTypeIndex(CompItr, "SemLog");
		if (TagIndex != INDEX_NONE)
		{
			const FString Id = FTagStatics::GetKeyValue(CompItr->ComponentTags[TagIndex], "Id");
			const FString Class = FTagStatics::GetKeyValue(CompItr->ComponentTags[TagIndex], "Class");
			if (!Id.IsEmpty() && !Class.IsEmpty())
			{
				FUniqueNameAndLocation UniqueNameAndInitLoc(Class + "_" + Id);
				USLRawDataLogger::AddComponentToJsonArray(JsonActorArr, CompItr, UniqueNameAndInitLoc);
			}
		}
	}

	// Setup and log dynamic entities
	TArray<AActor*> DynamicActors = FTagStatics::GetActorsWithKeyValuePair(
		World, "SemLog", "Runtime", "Dynamic");

	for (const auto& DynActItr : DynamicActors)
	{
		int32 TagIndex = FTagStatics::GetTagTypeIndex(DynActItr, "SemLog");
		if (TagIndex != INDEX_NONE)
		{
			const FString Id = FTagStatics::GetKeyValue(DynActItr->Tags[TagIndex], "Id");
			const FString Class = FTagStatics::GetKeyValue(DynActItr->Tags[TagIndex], "Class");
			if (!Id.IsEmpty() && !Class.IsEmpty())
			{
				// Location is init automatically to -INF
				const FString UniqueName = Class + "_" + Id;
				FUniqueNameAndLocation UniqueNameAndInitLoc(UniqueName);
				USLRawDataLogger::AddActorToJsonArray(JsonActorArr, DynActItr, UniqueNameAndInitLoc);

				// Store the UniqueName and the Location of the dynamic entity
				DynamicActorsWithData.Add(DynActItr,
					FUniqueNameAndLocation(UniqueName, DynActItr->GetActorLocation()));
			}
		}
	}

	// Get all static components and cast them to USceneComponent
	TArray<UActorComponent*> DynamicActorComponents = FTagStatics::GetComponentsWithKeyValuePair(
		World, "SemLog", "Runtime", "Dynamic");
	// Array to cast to
	TArray<USceneComponent*> DynamicSceneComponents;
	for (const auto& DynamicActCompItr : DynamicActorComponents)
	{
		if (DynamicActCompItr->IsA(USceneComponent::StaticClass()))
		{
			DynamicSceneComponents.Emplace(Cast<USceneComponent>(DynamicActCompItr));
		}
	}

	for (const auto& DynCompItr : DynamicSceneComponents)
	{
		int32 TagIndex = FTagStatics::GetTagTypeIndex(DynCompItr, "SemLog");
		if (TagIndex != INDEX_NONE)
		{
			const FString Id = FTagStatics::GetKeyValue(DynCompItr->ComponentTags[TagIndex], "Id");
			const FString Class = FTagStatics::GetKeyValue(DynCompItr->ComponentTags[TagIndex], "Class");
			if (!Id.IsEmpty() && !Class.IsEmpty())
			{
				// Location is init automatically to -INF
				const FString UniqueName = Class + "_" + Id;
				FUniqueNameAndLocation UniqueNameAndInitLoc(UniqueName);
				USLRawDataLogger::AddComponentToJsonArray(JsonActorArr, DynCompItr, UniqueNameAndInitLoc);

				// Store the UniqueName and the Location of the dynamic entity
				DynamicComponentsWithData.Add(DynCompItr,
					FUniqueNameAndLocation(UniqueName, DynCompItr->GetComponentLocation()));
			}
		}
	}

	// Add actors to Json root
	JsonRootObj->SetArrayField("actors", JsonActorArr);

	// Transform to string
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&FirstJsonEntry);
	FJsonSerializer::Serialize(JsonRootObj.ToSharedRef(), Writer);

	return (!FirstJsonEntry.IsEmpty());
}

// Get logged dynamic entities as json string
bool USLRawDataLogger::GetDynamicEntitiesAsJson(FString& DynamicJsonEntry)
{
	// Create Json root object
	// Json root object
	TSharedPtr<FJsonObject> JsonRootObj = MakeShareable(new FJsonObject);

	// Json array of actors
	TArray<TSharedPtr<FJsonValue>> JsonActorArr;

	// Iterate and log dynamic actors
	for (auto& ActWithDataItr : DynamicActorsWithData)
	{
		USLRawDataLogger::AddActorToJsonArray(JsonActorArr,
			ActWithDataItr.Key, ActWithDataItr.Value);
	}

	// Iterate and log dynamic components
	for (auto& CompWithDataItr : DynamicComponentsWithData)
	{
		USLRawDataLogger::AddComponentToJsonArray(JsonActorArr,
			CompWithDataItr.Key, CompWithDataItr.Value);
	}

	// Avoid appending empty entries
	if (JsonActorArr.Num() > 0)
	{
		// Set timestamp
		JsonRootObj->SetNumberField("timestamp", World->GetTimeSeconds());

		// Add actors to Json root
		JsonRootObj->SetArrayField("actors", JsonActorArr);

		// Transform to string
		FString JsonOutputString;
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&DynamicJsonEntry);
		FJsonSerializer::Serialize(JsonRootObj.ToSharedRef(), Writer);

		return true;
	}

	// No dynamic entities to be logged
	return false;
}

// Append string to the file
bool USLRawDataLogger::InsertJsonContentToFile(const FString& JsonString)
{
	if (!FileHandle)
	{
		return false;
	}
	// Write string to file
	return FileHandle->Write((const uint8*)TCHAR_TO_ANSI(*JsonString), JsonString.Len());
}

// Broadcast json content
void USLRawDataLogger::BroadcastJsonContent(const FString& JsonString)
{
	OnNewData.Broadcast(JsonString);
}

// Create Json object with a 3d location
FORCEINLINE TSharedPtr<FJsonObject> USLRawDataLogger::CreateLocationJsonObject(const FVector& Location)
{
	// Json location object
	TSharedPtr<FJsonObject> JsonObj = MakeShareable(new FJsonObject);
	// Add fields
	JsonObj->SetNumberField("x", Location.X);
	JsonObj->SetNumberField("y", -Location.Y); // left to right handed
	JsonObj->SetNumberField("z", Location.Z);

	return JsonObj;
}

// Create Json object with a 3d rotation as quaternion 
FORCEINLINE TSharedPtr<FJsonObject> USLRawDataLogger::CreateRotationJsonObject(const FQuat& Rotation)
{
	// Json rotation object
	TSharedPtr<FJsonObject> JsonObj = MakeShareable(new FJsonObject);
	// Add fields
	JsonObj->SetNumberField("w", Rotation.W);
	JsonObj->SetNumberField("x", -Rotation.X); // left to right handed
	JsonObj->SetNumberField("y", Rotation.Y);
	JsonObj->SetNumberField("z", -Rotation.Z); // left to right handed

	return JsonObj;
}

// Create Json object with name location and rotation
FORCEINLINE TSharedPtr<FJsonObject> USLRawDataLogger::CreateNameLocRotJsonObject(
	const FString& Name, const FVector& Location, const FQuat& Rotation)
{
	// Json  actor object
	TSharedPtr<FJsonObject> JsonObj = MakeShareable(new FJsonObject);
	// Add fields
	JsonObj->SetStringField("name", Name);
	JsonObj->SetObjectField("pos", USLRawDataLogger::CreateLocationJsonObject(Location));
	JsonObj->SetObjectField("rot", USLRawDataLogger::CreateRotationJsonObject(Rotation));

	return JsonObj;
}

// Add the actors raw data to the json array
void USLRawDataLogger::AddActorToJsonArray(
	TArray<TSharedPtr<FJsonValue>>& OutJsonArray,
	AActor* Actor,
	FUniqueNameAndLocation &UniqueNameAndLocation)
{
	// Get entity current location
	const FVector CurrLocation = Actor->GetActorLocation();
	// Write raw data if distance larger than threshold
	if (FVector::DistSquared(CurrLocation, UniqueNameAndLocation.Location) > SquaredDistanceThreshold)
	{
		// Update previous location
		UniqueNameAndLocation.Location = CurrLocation;

		// Json actor object with name location and rotation
		TSharedPtr<FJsonObject> JsonActorObj = USLRawDataLogger::CreateNameLocRotJsonObject(
			UniqueNameAndLocation.UniqueName, CurrLocation * 0.01f, Actor->GetActorQuat());

		// Check if actor is skeletal
		if (Actor->IsA(ASkeletalMeshActor::StaticClass()))
		{
			// Cast and get the skeletal mesh component
			USkeletalMeshComponent* CurrSkelMesh = Cast<ASkeletalMeshActor>(Actor)->GetSkeletalMeshComponent();

			// Json array of bones
			TArray<TSharedPtr<FJsonValue>> JsonBoneArr;

			// Get bone names
			TArray<FName> BoneNames;
			CurrSkelMesh->GetBoneNames(BoneNames);

			// Iterate through the bones of the skeletal mesh
			for (const auto& BoneName : BoneNames)
			{
				// TODO black voodo magic crashes, bug report, crashes if this is not called before
				CurrSkelMesh->GetBoneQuaternion(BoneName);

				// Json bone object with name location and rotation
				TSharedPtr<FJsonObject> JsonBoneObj = USLRawDataLogger::CreateNameLocRotJsonObject(
					BoneName.ToString(), CurrSkelMesh->GetBoneLocation(BoneName) * 0.01f,
					CurrSkelMesh->GetBoneQuaternion(BoneName));

				// Add bone to Json array
				JsonBoneArr.Add(MakeShareable(new FJsonValueObject(JsonBoneObj)));
			}
			// Add bones to Json actor
			JsonActorObj->SetArrayField("bones", JsonBoneArr);
		}

		// Add actor to Json array
		OutJsonArray.Add(MakeShareable(new FJsonValueObject(JsonActorObj)));
	}
}

// Add component's data to the json array
void USLRawDataLogger::AddComponentToJsonArray(
	TArray<TSharedPtr<FJsonValue>>& OutJsonArray,
	USceneComponent* Component,
	FUniqueNameAndLocation &UniqueNameAndLocation)
{
	// Get entity current location
	const FVector CurrLocation = Component->GetComponentLocation();
	// Write raw data if distance larger than threshold
	if (FVector::DistSquared(CurrLocation, UniqueNameAndLocation.Location) > SquaredDistanceThreshold)
	{
		// Update previous location
		UniqueNameAndLocation.Location = CurrLocation;

		// Json actor object with name location and rotation
		TSharedPtr<FJsonObject> JsonActorObj = USLRawDataLogger::CreateNameLocRotJsonObject(
			UniqueNameAndLocation.UniqueName, CurrLocation * 0.01f, Component->GetComponentQuat());

		// Add actor to Json array
		OutJsonArray.Add(MakeShareable(new FJsonValueObject(JsonActorObj)));
	}
}