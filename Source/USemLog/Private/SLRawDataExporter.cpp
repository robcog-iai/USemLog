// Fill out your copyright notice in the Description page of Project Settings.

#include "USemLogPrivatePCH.h"
#include "Animation/SkeletalMeshActor.h"
#include "SLUtils.h"
#include "Animation/SkeletalMeshActor.h"
#include "SLRawDataExporter.h"


// Set default values
FSLRawDataExporter::FSLRawDataExporter(const float DistThresh, const FString Path)
{
	// Square distance threshold (faster vector distance comparison)
	DistanceThresholdSquared = DistThresh * DistThresh;

	// Get platform file and init file handle
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	RawFileHandle = MakeShareable(PlatformFile.OpenWrite(*Path, true, true));
}

// Destructor
FSLRawDataExporter::~FSLRawDataExporter()
{
	RawFileHandle.Reset();
}

// Initialize items to log
void FSLRawDataExporter::WriteInit(
	const TMap<AActor*, FString>& ActToUniqueName,
	const TMap<AActor*, TArray<TPair<FString, FString>>>& ActToSemLogInfo,
	const float Timestamp)

{
	UE_LOG(SemLogRaw, Log, TEXT(" ** Init raw data logger: "));

	// Json root object
	TSharedPtr<FJsonObject> JsonRootObj = MakeShareable(new FJsonObject);
	// Set timestamp
	JsonRootObj->SetNumberField("timestamp", Timestamp);
	// Json array of actors
	TArray< TSharedPtr<FJsonValue> > JsonActorArr;

	// Iterate through the actors and their semlog info
	for (const auto& ActToSemLogItr : ActToSemLogInfo)
	{
		// Local copy of the actor, unique name and log type
		AActor* CurrAct = ActToSemLogItr.Key;
		const FString ActUniqueName = ActToUniqueName[CurrAct];
		const FString LoggingType = FSLUtils::GetPairArrayValue(ActToSemLogItr.Value, TEXT("LogType"));

		if (LoggingType.Equals("Dynamic"))
		{
			// Create local actor structure
			RawExpActStruct CurrActStruct(CurrAct, ActUniqueName);
			// Store the actors and their unique names in the local array for frequent update logging
			RawExpActArr.Add(CurrActStruct);
			// Add the current actor to the json array
			FSLRawDataExporter::AddActorToJsonArray(CurrActStruct, JsonActorArr);

			UE_LOG(SemLogRaw, Log, TEXT("\t %s --> %s [Dynamic]"),
				*CurrAct->GetName(), *ActUniqueName);
		}
		else if (LoggingType.Equals("Static"))
		{
			// Create local actor structure
			RawExpActStruct CurrActStruct(CurrAct, ActUniqueName);
			// Add the current actor to the json array
			FSLRawDataExporter::AddActorToJsonArray(CurrActStruct, JsonActorArr);

			// Write the position of the actors only once, no storing required
			UE_LOG(SemLogRaw, Log, TEXT("\t %s --> %s [Static]"),
				*CurrAct->GetName(), *ActUniqueName);
		}
		else
		{
			UE_LOG(SemLogRaw, Error, TEXT(" !! %s has no LogType, ignored by the RawDataExporter."),
				*CurrAct->GetName());
		}
	}

	// Add actors to Json root
	JsonRootObj->SetArrayField("actors", JsonActorArr);

	// Transform to string
	FString JsonOutputString;
	TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&JsonOutputString);
	FJsonSerializer::Serialize(JsonRootObj.ToSharedRef(), Writer);

	// Write string to file
	RawFileHandle->Write((const uint8*)TCHAR_TO_ANSI(*JsonOutputString), JsonOutputString.Len());
}


// Update grasping
void FSLRawDataExporter::Update(const float Timestamp)
{
	// Json root object
	TSharedPtr<FJsonObject> JsonRootObj = MakeShareable(new FJsonObject);
	// Set timestamp
	JsonRootObj->SetNumberField("timestamp", Timestamp);
	// Json array of actors
	TArray< TSharedPtr<FJsonValue> > JsonActorArr;

	// Iterate through the skeletal mesh components
	for (auto& RawExpActItr : RawExpActArr)
	{
		// Add the current actor to the json array
		FSLRawDataExporter::AddActorToJsonArray(RawExpActItr, JsonActorArr);
	}

	// Add actors to Json root
	JsonRootObj->SetArrayField("actors", JsonActorArr);

	// Transform to string
	FString JsonOutputString;
	TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&JsonOutputString);
	FJsonSerializer::Serialize(JsonRootObj.ToSharedRef(), Writer);

	// Write string to file
	RawFileHandle->Write((const uint8*)TCHAR_TO_ANSI(*JsonOutputString), JsonOutputString.Len());
}

// Create Json object with a 3d location
FORCEINLINE TSharedPtr<FJsonObject> FSLRawDataExporter::CreateLocationJsonObject(const FVector Location)
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
FORCEINLINE TSharedPtr<FJsonObject> FSLRawDataExporter::CreateRotationJsonObject(const FQuat Rotation)
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
FORCEINLINE TSharedPtr<FJsonObject> FSLRawDataExporter::CreateNameLocRotJsonObject(const FString Name, const FVector Location, const FQuat Rotation)
{
	// Json  actor object
	TSharedPtr<FJsonObject> JsonObj = MakeShareable(new FJsonObject);
	// Add fields
	JsonObj->SetStringField("name", Name);
	JsonObj->SetObjectField("pos", FSLRawDataExporter::CreateLocationJsonObject(Location));
	JsonObj->SetObjectField("rot", FSLRawDataExporter::CreateRotationJsonObject(Rotation));

	return JsonObj;
}

// Add the actors raw data to the json array
FORCEINLINE void FSLRawDataExporter::AddActorToJsonArray(RawExpActStruct& RawActStruct, TArray<TSharedPtr<FJsonValue>>& JsonArray)
{
	// Get a local pointer of the actor
	AActor* CurrAct = RawActStruct.Actor;
	// Get component current location
	const FVector CurrActLocation = CurrAct->GetActorLocation();
	// Write data if distance larger than threshold
	if (FVector::DistSquared(CurrActLocation, RawActStruct.PrevLoc) > DistanceThresholdSquared)
	{
		// Update previous location
		RawActStruct.PrevLoc = CurrActLocation;

		// Json actor object with name location and rotation
		TSharedPtr<FJsonObject> JsonActorObj = FSLRawDataExporter::CreateNameLocRotJsonObject(
			RawActStruct.UniqueName, CurrActLocation * 0.01, CurrAct->GetActorQuat());

		// Check if actor is skeletal
		if (CurrAct->IsA(ASkeletalMeshActor::StaticClass()))
		{
			// Cast and get the skeletal mesh component
			USkeletalMeshComponent* CurrSkelMesh = Cast<ASkeletalMeshActor>(CurrAct)->GetSkeletalMeshComponent();

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
				TSharedPtr<FJsonObject> JsonBoneObj = FSLRawDataExporter::CreateNameLocRotJsonObject(
					BoneName.ToString(), CurrSkelMesh->GetBoneLocation(BoneName) * 0.01,
					CurrSkelMesh->GetBoneQuaternion(BoneName));

				// Add bone to Json array
				JsonBoneArr.Add(MakeShareable(new FJsonValueObject(JsonBoneObj)));
			}
			// Add bones to Json actor
			JsonActorObj->SetArrayField("bones", JsonBoneArr);
		}

		// Add actor to Json array
		JsonArray.Add(MakeShareable(new FJsonValueObject(JsonActorObj)));
	}
}