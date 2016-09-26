// Fill out your copyright notice in the Description page of Project Settings.

#include "SemLogPrivatePCH.h"
#include "SLRawDataExporter.h"

// Set default values
FSLRawDataExporter::FSLRawDataExporter(
	const float DistThreshSqr, 
	const FString Path,
	TMap<ASkeletalMeshActor*, FString> SkelActPtrToUniqNameMap,
	TMap<AStaticMeshActor*, FString> DynamicActPtrToUniqNameMap,
	TMap<AStaticMeshActor*, FString> StaticActPtrToUniqNameMap,
	TPair<USceneComponent*, FString> CamToUniqName) :
	DistanceThresholdSquared(DistThreshSqr)
{
	// Get platform file and init file handle
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();	
	RawFileHandle = MakeShareable(PlatformFile.OpenWrite(*Path, true, true));

	// Set the camera to be loggeds
	CameraToUniqueName = CamToUniqName;
	CameraPrevLoc = FVector(0);

	// Init items we want to log
	FSLRawDataExporter::InitItemsToLog(SkelActPtrToUniqNameMap,
		DynamicActPtrToUniqNameMap,
		StaticActPtrToUniqNameMap);
}

// Destructor
FSLRawDataExporter::~FSLRawDataExporter()
{
	RawFileHandle.Reset();
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

	// Iterate through the static mesh components
	for (auto& DynamicActStructItr : DynamicActStructArr)
	{
		// Get component current location
		const FVector CurrActLocation = DynamicActStructItr.StaticMeshAct->GetActorLocation();

		// Squared distance between the current and the previous pose
		const float DistSqr = FVector::DistSquared(CurrActLocation, DynamicActStructItr.PrevLoc);

		// Save data if distance larger than threshold
		if (DistSqr > DistanceThresholdSquared)
		{
			// Get a local pointer of the skeletal mesh actor
			AStaticMeshActor* CurrStaticMeshAct = DynamicActStructItr.StaticMeshAct;
			// Update previous location
			DynamicActStructItr.PrevLoc = CurrActLocation;

			// Json actor object with name location and rotation
			TSharedPtr<FJsonObject> JsonActorObj = FSLRawDataExporter::CreateNameLocRotJsonObject(
				DynamicActStructItr.UniqueName, CurrActLocation * 0.01, CurrStaticMeshAct->GetActorQuat());

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
	if (StaticActToUniqName.Num() > 0)
	{
		// Iterate the static map actors (done only once)
		for (auto& StaticActToUniqNameItr : StaticActToUniqName)
		{
			// Json actor object with name location and rotation
			TSharedPtr<FJsonObject> JsonActorObj = FSLRawDataExporter::CreateNameLocRotJsonObject(
				StaticActToUniqNameItr.Value, StaticActToUniqNameItr.Key->GetActorLocation() * 0.01, StaticActToUniqNameItr.Key->GetActorQuat());

			// Add actor to Json array
			JsonActorArr.Add(MakeShareable(new FJsonValueObject(JsonActorObj)));
		}
		// Empty array, only needs to be logged once;
		StaticActToUniqName.Empty();
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

// Initialize items to log
void FSLRawDataExporter::InitItemsToLog(
	const TMap<ASkeletalMeshActor*, FString>& SkelActPtrToUniqNameMap,
	const TMap<AStaticMeshActor*, FString>& DynamicActPtrToUniqNameMap,
	const TMap<AStaticMeshActor*, FString>& StaticActPtrToUniqNameMap)
{
	UE_LOG(SemLogRaw, Warning, TEXT(" *** "));
	UE_LOG(SemLogRaw, Warning, TEXT("Raw data logger: "));
	UE_LOG(SemLogRaw, Warning, TEXT("  Skeletal components:"));
	for (const auto SkelActPtrToUniqNameItr : SkelActPtrToUniqNameMap)
	{
		SkelActStructArr.Add(
			FSLSkelLogRawStruct(SkelActPtrToUniqNameItr.Key, SkelActPtrToUniqNameItr.Value));
		UE_LOG(SemLogRaw, Warning, TEXT("\t%s -> %s"),
			*SkelActPtrToUniqNameItr.Key->GetName(), *SkelActPtrToUniqNameItr.Value);
	}

	UE_LOG(SemLogRaw, Warning, TEXT("  Dynamic items:"));
	for (const auto DynamicActPtrToUniqNameItr : DynamicActPtrToUniqNameMap)
	{
		DynamicActStructArr.Add(
			FSLDynActLogRawStruct(DynamicActPtrToUniqNameItr.Key, DynamicActPtrToUniqNameItr.Value));
		UE_LOG(SemLogRaw, Warning, TEXT("\t%s -> %s"),
			*DynamicActPtrToUniqNameItr.Key->GetName(), *DynamicActPtrToUniqNameItr.Value);
	}

	if (CameraToUniqueName.Key)
	{
		// Camera component
		UE_LOG(SemLogRaw, Warning, TEXT("\t%s -> %s"),
			*CameraToUniqueName.Key->GetName(), *CameraToUniqueName.Value);
	}

	UE_LOG(SemLogRaw, Warning, TEXT("  Static map items (logged once):"));
	StaticActToUniqName = StaticActPtrToUniqNameMap;
	for (const auto StaticActPtrToUniqNameItr : StaticActPtrToUniqNameMap)
	{
		UE_LOG(SemLogRaw, Warning, TEXT("\t%s -> %s"),
			*StaticActPtrToUniqNameItr.Key->GetName(), *StaticActPtrToUniqNameItr.Value);
	}
}
