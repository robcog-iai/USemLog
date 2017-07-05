// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLRawData.h"
#include "Animation/SkeletalMeshActor.h"
#include "TagStatics.h"
//#include "PlatformFilemanager.h"
//#include "FileManager.h"
//#include "FileHelper.h"

// Constructor
USLRawData::USLRawData()
{
	// Default values
	bIsInit = false;
	bLogToFile = false;
}

// Destructor
USLRawData::~USLRawData()
{
	if (FileHandle)
	{
		delete FileHandle;
	}
}

// Init logger
bool USLRawData::Init(UWorld* InWorld, const float DistanceThreshold)
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
void USLRawData::SetLogToFile(const FString EpisodeId, const FString LogDirectoryPath)
{
	// Create filehandle to incrementally append json logs to file
	const FString Filename = "RawData_" + EpisodeId + ".json";
	const FString EpisodesDirPath = LogDirectoryPath.EndsWith("/") ?
		(LogDirectoryPath + "Episodes/") : (LogDirectoryPath + "/Episodes/");
	const FString FilePath = EpisodesDirPath + Filename;

	// Create logging directory path and the filehandle
	FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*EpisodesDirPath);
	FileHandle = FPlatformFileManager::Get().GetPlatformFile().OpenWrite(*FilePath, true);

	bLogToFile = (FileHandle != nullptr);
}

// Log dynamic and static entities to file
void USLRawData::FirstLog()
{
	// String to store the json entry
	FString JsonOutputString;
	USLRawData::GetFirstJsonEntry(JsonOutputString);

	// Append to file if set
	if (bLogToFile)
	{
		USLRawData::AddToFile(JsonOutputString);
	}
}

// Get the dynamic and static entities as json string
bool USLRawData::GetFirstJsonEntry(FString& FirstJsonEntry)
{
	if (!bIsInit)
	{
		return false;
	}

	// Create Json root object
	// Json root object
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
		const FString Id = FTagStatics::GetKeyValue(ActItr, "SemLog", "Id");
		if (!Id.IsEmpty())
		{
			const FString UniqueName = FTagStatics::GetKeyValue(ActItr, "SemLog", "Class") + "_" + Id;
			FVector VirtualPreviousLocation(-99999.9f);

			USLRawData::AddActorToJsonArray(
				JsonActorArr, ActItr, UniqueName, VirtualPreviousLocation);
		}
	}

	//TArray<USceneComponent*> StaticComponents = FTagStatics::GetActorsWithKeyValuePair(
	//	World, "SemLog", "Runtime", "Static");

	//for (const auto& CompItr : StaticComponents)
	//{
	//	const FString Id = FTagStatics::GetKeyValue(CompItr, "SemLog", "Id");
	//	if (!Id.IsEmpty())
	//	{
	//		const FString UniqueName = FTagStatics::GetKeyValue(CompItr, "SemLog", "Class") + "_" + Id;
	//		FVector VirtualPreviousLocation(-99999.9f);

	//		USLRawData::AddComponentToJsonArray(
	//			JsonActorArr, CompItr, UniqueName, VirtualPreviousLocation);
	//	}
	//}

	// Setup and log dynamic entities
	TArray<AActor*> DynamicActors = FTagStatics::GetActorsWithKeyValuePair(
		World, "SemLog", "Runtime", "Dynamic");

	for (const auto& DynActItr : DynamicActors)
	{
		const FString Id = FTagStatics::GetKeyValue(DynActItr, "SemLog", "Id");
		if (!Id.IsEmpty())
		{
			const FString UniqueName = FTagStatics::GetKeyValue(DynActItr, "SemLog", "Class") + "_" + Id;
			FVector VirtualPreviousLocation(-99999.9f);
			USLRawData::AddActorToJsonArray(
				JsonActorArr, DynActItr, UniqueName, VirtualPreviousLocation);

			// Store the UniqueName and the Location of the dynamic entity
			DynamicActorsWithData.Add(DynActItr,
				FUniqueNameAndLocation(UniqueName, DynActItr->GetActorLocation()));
		}
	}

	//TArray<USceneComponent*> DynamicComponents = FTagStatics::GetActorsWithKeyValuePair(
	//	World, "SemLog", "Runtime", "Dynamic");

	//for (const auto& DynCompItr : DynamicComponents)
	//{
	//	const FString Id = FTagStatics::GetKeyValue(DynCompItr, "SemLog", "Id");
	//	if (!Id.IsEmpty())
	//	{
	//		const FString UniqueName = FTagStatics::GetKeyValue(DynCompItr, "SemLog", "Class") + "_" + Id;
	//		FVector VirtualPreviousLocation(-99999.9f);
	//		USLRawData::AddComponentToJsonArray(
	//			JsonActorArr, DynCompItr, UniqueName, VirtualPreviousLocation);

	//		// Store the UniqueName and the Location of the dynamic entity
	//		DynamicComponentsWithData.Add(DynCompItr,
	//			FUniqueNameAndLocation(UniqueName, DynCompItr->GetComponentLocation()));
	//	}
	//}

	// Add actors to Json root
	JsonRootObj->SetArrayField("actors", JsonActorArr);

	// Transform to string
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&FirstJsonEntry);
	FJsonSerializer::Serialize(JsonRootObj.ToSharedRef(), Writer);

	return (!FirstJsonEntry.IsEmpty());
}

// Log dynamic entities
void USLRawData::LogDynamic()
{
	// String to store the json entry
	FString DynamicJsonOutputString;
	USLRawData::GetDynamicJsonEntry(DynamicJsonOutputString);

	// Append to file if set
	if (bLogToFile)
	{
		USLRawData::AddToFile(DynamicJsonOutputString);
	}
}

// Get logged dynamic entities as json string
bool USLRawData::GetDynamicJsonEntry(FString& DynamicJsonEntry)
{
	// Create Json root object
	// Json root object
	TSharedPtr<FJsonObject> JsonRootObj = MakeShareable(new FJsonObject);

	// Json array of actors
	TArray< TSharedPtr<FJsonValue> > JsonActorArr;

	// Iterate and log dynamic actors
	for (auto& ActWithDataItr : DynamicActorsWithData)
	{
		USLRawData::AddActorToJsonArray(JsonActorArr,
			ActWithDataItr.Key, ActWithDataItr.Value.UniqueName, ActWithDataItr.Value.Location);
	}

	// Iterate and log dynamic components
	for (auto& CompWithDataItr : DynamicComponentsWithData)
	{
		USLRawData::AddComponentToJsonArray(JsonActorArr,
			CompWithDataItr.Key, CompWithDataItr.Value.UniqueName, CompWithDataItr.Value.Location);
	}

	// Avoid appending emtpy entries
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
bool USLRawData::AddToFile(FString& JsonString)
{
	if (!FileHandle)
	{
		return false;
	}
	// Write string to file
	return FileHandle->Write((const uint8*)TCHAR_TO_ANSI(*JsonString), JsonString.Len());
}

// Create Json object with a 3d location
FORCEINLINE TSharedPtr<FJsonObject> USLRawData::CreateLocationJsonObject(const FVector& Location)
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
FORCEINLINE TSharedPtr<FJsonObject> USLRawData::CreateRotationJsonObject(const FQuat& Rotation)
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
FORCEINLINE TSharedPtr<FJsonObject> USLRawData::CreateNameLocRotJsonObject(
	const FString& Name, const FVector& Location, const FQuat& Rotation)
{
	// Json  actor object
	TSharedPtr<FJsonObject> JsonObj = MakeShareable(new FJsonObject);
	// Add fields
	JsonObj->SetStringField("name", Name);
	JsonObj->SetObjectField("pos", USLRawData::CreateLocationJsonObject(Location));
	JsonObj->SetObjectField("rot", USLRawData::CreateRotationJsonObject(Rotation));

	return JsonObj;
}

// Add the actors raw data to the json array
void USLRawData::AddActorToJsonArray(
	TArray<TSharedPtr<FJsonValue>>& OutJsonArray,
	AActor* Actor,
	const FString& UniqueName,
	FVector& PreviousLocation)
{
	// Get entity current location
	const FVector CurrLocation = Actor->GetActorLocation();
	// Write raw data if distance larger than threshold
	if (FVector::DistSquared(CurrLocation, PreviousLocation) > SquaredDistanceThreshold)
	{
		// Update previous location
		PreviousLocation = CurrLocation;

		// Json actor object with name location and rotation
		TSharedPtr<FJsonObject> JsonActorObj = USLRawData::CreateNameLocRotJsonObject(
			UniqueName, CurrLocation * 0.01f, Actor->GetActorQuat());

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
				TSharedPtr<FJsonObject> JsonBoneObj = USLRawData::CreateNameLocRotJsonObject(
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
void USLRawData::AddComponentToJsonArray(
	TArray<TSharedPtr<FJsonValue>>& OutJsonArray,
	USceneComponent* Component,
	const FString& UniqueName,
	FVector& PreviousLocation)
{
	// Get entity current location
	const FVector CurrLocation = Component->GetComponentLocation();
	// Write raw data if distance larger than threshold
	if (FVector::DistSquared(CurrLocation, PreviousLocation) > SquaredDistanceThreshold)
	{
		// Update previous location
		PreviousLocation = CurrLocation;

		// Json actor object with name location and rotation
		TSharedPtr<FJsonObject> JsonActorObj = USLRawData::CreateNameLocRotJsonObject(
			UniqueName, CurrLocation * 0.01f, Component->GetComponentQuat());

		// Add actor to Json array
		OutJsonArray.Add(MakeShareable(new FJsonValueObject(JsonActorObj)));
	}
}