// Fill out your copyright notice in the Description page of Project Settings.

#include "SemLogPrivatePCH.h"
#include "SLUtils.h"
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

	// Iterate through the actors and their semlog info
	for (const auto ActToSemLogItr : ActToSemLogInfo)
	{
		// Local copy of the actor, unique name and log type
		const AActor* CurrAct = ActToSemLogItr.Key;
		const FString ActUniqueName = ActToUniqueName[CurrAct];
		FString LogType = FSLUtils::GetPairArrayValue(ActToSemLogItr.Value, TEXT("LogType"));

		if (LogType.Equals("Dynamic"))
		{
			// Store the actors and their unique names in the local array for frequent update logging
			RawExpActArr.Add(RawExpActStruct(CurrAct, ActUniqueName));
			UE_LOG(SemLogRaw, Log, TEXT("\t %s --> %s [Dynamic]"),
				*CurrAct->GetName(), *ActUniqueName);

			// TODO create a function to write the data, use this in the update as well
		}
		else if (LogType.Equals("Static"))
		{
			// Write the position of the actors only once, no storing required
			UE_LOG(SemLogRaw, Log, TEXT("\t %s --> %s [Static]"),
				*CurrAct->GetName(), *ActUniqueName);

			// TODO create a function to write the data
		}
		else
		{
			UE_LOG(SemLogRaw, Error, TEXT(" !! %s has no LogType, ignored by the RawDataExporter."),
				*CurrAct->GetName());
		}
	}
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
		// Get a local pointer of the actor
		const AActor* CurrAct = RawExpActItr.Actor;
		// Get component current location
		const FVector CurrActLocation = CurrAct->GetActorLocation();
		// Write data if distance larger than threshold
		if (FVector::DistSquared(CurrActLocation, RawExpActItr.PrevLoc) > DistanceThresholdSquared)
		{
			// Update previous location
			RawExpActItr.PrevLoc = CurrActLocation;

			// Json actor object with name location and rotation
			TSharedPtr<FJsonObject> JsonActorObj = FSLRawDataExporter::CreateNameLocRotJsonObject(
				RawExpActItr.UniqueName, CurrActLocation * 0.01, CurrAct->GetActorQuat());

			// Add actor to Json array
			JsonActorArr.Add(MakeShareable(new FJsonValueObject(JsonActorObj)));

			// TODO check if skeletal component

			// Check if skeletal component

			//// Get a local pointer of the skeletal mesh
			//USkeletalMeshComponent* CurrSkelMesh = RawExpActItr.Actor->GetSkeletalMeshComponent();
			//// Update previous location
			//SkelActStructItr.PrevLoc = CurrCompLocation;
			//// Json actor object with name location and rotation
			//TSharedPtr<FJsonObject> JsonActorObj = FSLRawDataExporter::CreateNameLocRotJsonObject(
			//	SkelActStructItr.UniqueName, CurrCompLocation * 0.01, CurrSkelMesh->GetComponentQuat());

			//// Json array of bones
			//TArray< TSharedPtr<FJsonValue> > JsonBoneArr;
			//// Get bone names
			//TArray<FName> BoneNames;
			//CurrSkelMesh->GetBoneNames(BoneNames);

			//// Iterate through the bones of the skeletal mesh
			//for (const auto BoneName : BoneNames)
			//{
			//	// TODO black voodo magic crashes, bug report, crashes if this is not called before
			//	CurrSkelMesh->GetBoneQuaternion(BoneName);

			//	// Json bone object with name location and rotation
			//	TSharedPtr<FJsonObject> JsonBoneObj = FSLRawDataExporter::CreateNameLocRotJsonObject(
			//		BoneName.ToString(), CurrSkelMesh->GetBoneLocation(BoneName) * 0.01,
			//		CurrSkelMesh->GetBoneQuaternion(BoneName));

			//	// Add bone to Json array
			//	JsonBoneArr.Add(MakeShareable(new FJsonValueObject(JsonBoneObj)));
			//}
			//// Add bones to Json actor
			//JsonActorObj->SetArrayField("bones", JsonBoneArr);

			//// Add actor to Json array
			//JsonActorArr.Add(MakeShareable(new FJsonValueObject(JsonActorObj)));




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

