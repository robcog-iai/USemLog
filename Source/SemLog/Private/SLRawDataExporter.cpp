// Fill out your copyright notice in the Description page of Project Settings.

#include "SemLogPrivatePCH.h"
#include "SLRawDataExporter.h"


// Set default values
FSLRawDataExporter::FSLRawDataExporter(
	const float DistThreshSqr,
	const TArray<ASLItem*>& DynamicItems,
	const TArray<ASLItem*>& StaticItems,
	const TMap<ASkeletalMeshActor*, FString>& SkelActPtrToUniqNameMap,
	const TPair<USceneComponent*, FString> CamToUniqName,
	const FString Path)
{
	// Get platform file and init file handle
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	RawFileHandle = MakeShareable(PlatformFile.OpenWrite(*Path, true, true));

	// Set the camera to be loggeds
	CameraToUniqueName = CamToUniqName;
	CameraPrevLoc = FVector(0);

	// Init items we want to log
	FSLRawDataExporter::InitItemsToLog(
		DynamicItems,
		StaticItems,
		SkelActPtrToUniqNameMap);
}

// Destructor
FSLRawDataExporter::~FSLRawDataExporter()
{
	RawFileHandle.Reset();
}

// Initialize items to log
void FSLRawDataExporter::InitItemsToLog(
	const TArray<ASLItem*>& DynamicItems,
	const TArray<ASLItem*>& StaticItems,
	const TMap<ASkeletalMeshActor*, FString>& SkelActPtrToUniqNameMap)

{
	UE_LOG(SemLogRaw, Log, TEXT(" ** Raw data logger: "));

	for (const auto ItemItr : DynamicItems)
	{
		DynamicItemsStructArr.Add(ItemRawStruct(ItemItr));
		UE_LOG(SemLogRaw, Log, TEXT("\t %s --> %s [Dynamic]"),
			*ItemItr->GetName(), *ItemItr->GetUniqueName());
	}

	if (CameraToUniqueName.Key)
	{
		// Camera component
		UE_LOG(SemLogRaw, Log, TEXT("\t%s --> %s [Dynamic]"),
			*CameraToUniqueName.Key->GetName(), *CameraToUniqueName.Value);
	}

	for (const auto SkelActPtrToUniqNameItr : SkelActPtrToUniqNameMap)
	{
		SkelActStructArr.Add(
			SkelRawStruct(SkelActPtrToUniqNameItr.Key, SkelActPtrToUniqNameItr.Value));
		UE_LOG(SemLogRaw, Log, TEXT(" \t% s --> %s [Skeletal]"),
			*SkelActPtrToUniqNameItr.Key->GetName(), *SkelActPtrToUniqNameItr.Value);
	}

	// Local copy of the static items
	StaticItemsArr = StaticItems;
	for (const auto ItemItr : StaticItems)
	{
		UE_LOG(SemLogRaw, Log, TEXT("\t %s --> %s [Static]"),
			*ItemItr->GetName(), *ItemItr->GetUniqueName());
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
	for (auto& SkelActStructItr : SkelActStructArr)
	{
		// Get component current location
		const FVector CurrCompLocation = SkelActStructItr.SkelMeshComp->GetActorLocation();

		// Squared distance between the current and the previous pose
		const float DistSqr = FVector::DistSquared(CurrCompLocation, SkelActStructItr.PrevLoc);

		// Save data if distance larger than threshold
		if (DistSqr > DistanceThresholdSquared)
		{
			// Get a local pointer of the skeletal mesh
			USkeletalMeshComponent* CurrSkelMesh = SkelActStructItr.SkelMeshComp->GetSkeletalMeshComponent();
			// Update previous location
			SkelActStructItr.PrevLoc = CurrCompLocation;
			// Json actor object with name location and rotation
			TSharedPtr<FJsonObject> JsonActorObj = FSLRawDataExporter::CreateNameLocRotJsonObject(
				SkelActStructItr.UniqueName, CurrCompLocation * 0.01, CurrSkelMesh->GetComponentQuat());

			// Json array of bones
			TArray< TSharedPtr<FJsonValue> > JsonBoneArr;
			// Get bone names
			TArray<FName> BoneNames;
			CurrSkelMesh->GetBoneNames(BoneNames);

			// Iterate through the bones of the skeletal mesh
			for (const auto BoneName : BoneNames)
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

			// Add actor to Json array
			JsonActorArr.Add(MakeShareable(new FJsonValueObject(JsonActorObj)));
		}
	}

	// Iterate through dynamic items
	for (auto& DynamicItemStructItr : DynamicItemsStructArr)
	{
		// Get component current location
		const FVector CurrActLocation = DynamicItemStructItr.Item->GetActorLocation();

		// Squared distance between the current and the previous pose
		const float DistSqr = FVector::DistSquared(CurrActLocation, DynamicItemStructItr.PrevLoc);

		// Save data if distance larger than threshold
		if (DistSqr > DistanceThresholdSquared)
		{
			// Get a local pointer of the skeletal mesh actor
			AStaticMeshActor* CurrStaticMeshAct = DynamicItemStructItr.Item;
			// Update previous location
			DynamicItemStructItr.PrevLoc = CurrActLocation;

			// Json actor object with name location and rotation
			TSharedPtr<FJsonObject> JsonActorObj = FSLRawDataExporter::CreateNameLocRotJsonObject(
				DynamicItemStructItr.Item->GetUniqueName(), CurrActLocation * 0.01, CurrStaticMeshAct->GetActorQuat());

			// Add actor to Json array
			JsonActorArr.Add(MakeShareable(new FJsonValueObject(JsonActorObj)));
		}
	}

	// Log the camera component
	if(CameraToUniqueName.Key)
	{
		// Get component current location
		const FVector CurrCameraLocation = CameraToUniqueName.Key->GetComponentLocation();

		// Squared distance between the current and the previous pose
		const float DistSqr = FVector::DistSquared(CurrCameraLocation, CameraPrevLoc);

		// Save data if distance larger than threshold
		if (DistSqr > DistanceThresholdSquared)
		{
			// Update previous location
			CameraPrevLoc = CurrCameraLocation;

			// Json actor object with name location and rotation
			TSharedPtr<FJsonObject> JsonActorObj = FSLRawDataExporter::CreateNameLocRotJsonObject(
				CameraToUniqueName.Value, CurrCameraLocation * 0.01, CameraToUniqueName.Key->GetComponentQuat());
			// Add actor to Json array
			JsonActorArr.Add(MakeShareable(new FJsonValueObject(JsonActorObj)));
		}
	}


	// Check if static map actors need to be logged
	if (StaticItemsArr.Num() > 0)
	{
		// Iterate the static map actors (done only once)
		for (const auto StaticItemsItr : StaticItemsArr)
		{
			// Json actor object with name location and rotation
			TSharedPtr<FJsonObject> JsonActorObj = FSLRawDataExporter::CreateNameLocRotJsonObject(
				StaticItemsItr->GetUniqueName(), StaticItemsItr->GetActorLocation() * 0.01, StaticItemsItr->GetActorQuat());

			// Add actor to Json array
			JsonActorArr.Add(MakeShareable(new FJsonValueObject(JsonActorObj)));
		}
		// Empty array, only needs to be logged once;
		StaticItemsArr.Empty();
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

