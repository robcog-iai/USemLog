// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Viz/Legacy/SLVizMarker.h"
#include "Viz/SLVizAssets.h"
#include "Viz/SLVizMarkerManager.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

// Constructor
USLVizMarker::USLVizMarker()
{
	PrimaryComponentTick.bCanEverTick = false;
	bSelectable = false;
	LoadAssets();
	LoadAssetsContainer();
	SkeletalMesh = nullptr;
	CurrentVisualType = ESLVizVisualType::NONE;
}

// Create visual at pose
void USLVizMarker::SetVisual(const FSLVizMarkerVisualParams& VisualParams)
{
	// Clear any previously set visuals (mesh/materials)
	Reset();

	// Check visual type
	if (VisualParams.Type != ESLVizMarkerType::Clone)
	{
		Scale = VisualParams.Scale;

		SetStaticMesh(GetPrimitiveMarkerMesh(VisualParams.Type));
		SetCollisionEnabled(ECollisionEnabled::NoCollision);

		UMaterialInstanceDynamic* DynMat = VisualParams.bUnlit ? UMaterialInstanceDynamic::Create(MaterialUnlit, NULL) : UMaterialInstanceDynamic::Create(MaterialLit, NULL);
		DynMat->SetVectorParameterValue(FName("Color"), VisualParams.Color);
		SetMaterial(0, DynMat);
		CurrentVisualType = ESLVizVisualType::Static;
	}
	else
	{
		if (VisualParams.SMC)
		{
			Scale = FVector::OneVector;

			SetStaticMesh(VisualParams.SMC->GetStaticMesh());
			SetCollisionEnabled(ECollisionEnabled::NoCollision);

			if (!VisualParams.bUseCloneOriginalMaterial)
			{
				UMaterialInstanceDynamic* DynMat = VisualParams.bUnlit ? UMaterialInstanceDynamic::Create(MaterialUnlit, NULL) : UMaterialInstanceDynamic::Create(MaterialLit, NULL);
				DynMat->SetVectorParameterValue(FName("Color"), VisualParams.Color);
				SetMaterial(0, DynMat);
			}

			CurrentVisualType = ESLVizVisualType::Static;
		}
		else if (VisualParams.SkelMC)
		{
			//SkeletalMesh = DuplicateObject<USkeletalMesh>(SkMC->SkeletalMesh, this);
			SkeletalMesh = VisualParams.SkelMC->SkeletalMesh;

			if (VisualParams.MaterialIndexes.Num() == 0)
			{
				if (VisualParams.bUseCloneOriginalMaterial)
				{
					for (int32 MatIdx = 0; MatIdx < VisualParams.SkelMC->GetNumMaterials(); ++MatIdx)
					{
						SkeletalMaterials.Emplace(MatIdx, VisualParams.SkelMC->GetMaterial(MatIdx));
					}
				}
				else
				{
					UMaterialInstanceDynamic* DynMat = VisualParams.bUnlit ? UMaterialInstanceDynamic::Create(MaterialUnlit, NULL) : UMaterialInstanceDynamic::Create(MaterialLit, NULL);
					DynMat->SetVectorParameterValue(FName("Color"), VisualParams.Color);
					for (int32 MatIdx = 0; MatIdx < VisualParams.SkelMC->GetNumMaterials(); ++MatIdx)
					{
						SkeletalMaterials.Emplace(MatIdx, DynMat);
					}
				}
			}
			else if (VisualParams.MaterialIndexes.Num() == 1)
			{
				if (VisualParams.MaterialIndexes[0] >= VisualParams.SkelMC->GetNumMaterials())
				{
					UE_LOG(LogTemp, Error, TEXT("%s::%d Invalid MaterialIndex=%d .."), *FString(__FUNCTION__), __LINE__, VisualParams.MaterialIndexes[0]);
					return;
				}

				if (VisualParams.bUseCloneOriginalMaterial)
				{
					for (int32 MatIdx = 0; MatIdx < VisualParams.SkelMC->GetNumMaterials(); ++MatIdx)
					{
						MatIdx != VisualParams.MaterialIndexes[0] ? SkeletalMaterials.Emplace(MatIdx, MaterialInvisible) 
							: SkeletalMaterials.Emplace(MatIdx, VisualParams.SkelMC->GetMaterial(MatIdx));
					}
				}
				else
				{
					UMaterialInstanceDynamic* DynMat = VisualParams.bUnlit ? UMaterialInstanceDynamic::Create(MaterialUnlit, NULL) : UMaterialInstanceDynamic::Create(MaterialLit, NULL);
					DynMat->SetVectorParameterValue(FName("Color"), VisualParams.Color);

					for (int32 MatIdx = 0; MatIdx < VisualParams.SkelMC->GetNumMaterials(); ++MatIdx)
					{
						MatIdx != VisualParams.MaterialIndexes[0] ? SkeletalMaterials.Emplace(MatIdx, MaterialInvisible)
							: SkeletalMaterials.Emplace(MatIdx, DynMat);
					}
				}
			}
			else
			{
				if (VisualParams.bUseCloneOriginalMaterial)
				{
					for (int32 MatIdx = 0; MatIdx < VisualParams.SkelMC->GetNumMaterials(); ++MatIdx)
					{
						!VisualParams.MaterialIndexes.Contains(MatIdx) ? SkeletalMaterials.Emplace(MatIdx, MaterialInvisible)
							: SkeletalMaterials.Emplace(MatIdx, VisualParams.SkelMC->GetMaterial(MatIdx));
					}
				}
				else
				{
					UMaterialInstanceDynamic* DynMat = VisualParams.bUnlit ? UMaterialInstanceDynamic::Create(MaterialUnlit, NULL) : UMaterialInstanceDynamic::Create(MaterialLit, NULL);
					DynMat->SetVectorParameterValue(FName("Color"), VisualParams.Color);

					for (int32 MatIdx = 0; MatIdx < VisualParams.SkelMC->GetNumMaterials(); ++MatIdx)
					{
						!VisualParams.MaterialIndexes.Contains(MatIdx) ? SkeletalMaterials.Emplace(MatIdx, MaterialInvisible)
							: SkeletalMaterials.Emplace(MatIdx, DynMat);
					}
				}
			}

			CurrentVisualType = ESLVizVisualType::Skeletal;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Unsuported visual type.."), *FString(__FUNCTION__), __LINE__);
			return;
		}
	}
}

// Create marker at location
void USLVizMarker::Init(ESLVizMarkerType Type, const FVector& InScale, const FLinearColor& Color, bool bUnlit)
{
	// Clear any previously set visuals (mesh/materials)
	Reset();
	Scale = InScale;
	SetStaticMesh(GetPrimitiveMarkerMesh(Type));
	SetCollisionEnabled(ECollisionEnabled::NoCollision);

	UMaterialInstanceDynamic* DynMat = bUnlit ? UMaterialInstanceDynamic::Create(MaterialUnlit, NULL) : UMaterialInstanceDynamic::Create(MaterialLit, NULL);
	DynMat->SetVectorParameterValue(FName("Color"), Color);
	SetMaterial(0, DynMat);
	CurrentVisualType = ESLVizVisualType::Static;
}

// Set the visuals of the marker from the static mesh 
void USLVizMarker::Init(UStaticMeshComponent* SMC)
{
	// Clear any previously set visuals (mesh/materials)
	Reset();

	Scale = FVector(1.f);	
	SetStaticMesh(SMC->GetStaticMesh());	
	SetCollisionEnabled(ECollisionEnabled::NoCollision);

	CurrentVisualType = ESLVizVisualType::Static;
}

// Set the visuals of the marker from the static mesh component with custom color
void USLVizMarker::Init(UStaticMeshComponent* SMC, const FLinearColor& Color, bool bUnlit)
{
	// Clear any previously set visuals (mesh/materials)
	Reset();

	Scale = FVector(1.f);
	SetStaticMesh(SMC->GetStaticMesh());
	SetCollisionEnabled(ECollisionEnabled::NoCollision);

	UMaterialInstanceDynamic* DynMat = bUnlit ? UMaterialInstanceDynamic::Create(MaterialUnlit, NULL) : UMaterialInstanceDynamic::Create(MaterialLit, NULL);
	DynMat->SetVectorParameterValue(FName("Color"), Color);
	for (int32 MatIdx = 0; MatIdx < SMC->GetNumMaterials(); ++MatIdx)
	{
		SetMaterial(MatIdx, DynMat);
	}

	CurrentVisualType = ESLVizVisualType::Static;
}

// Set the visuals of the marker from the skeletal mesh component with its original colors
void USLVizMarker::Init(USkeletalMeshComponent* SkMC)
{
	// Clear any previously set visuals (mesh/materials)
	Reset();

	//SkeletalMesh = DuplicateObject<USkeletalMesh>(SkMC->SkeletalMesh, this);
	SkeletalMesh = SkMC->SkeletalMesh;

	for (int32 MatIdx = 0; MatIdx < SkMC->GetNumMaterials(); ++MatIdx)
	{
		SkeletalMaterials.Emplace(MatIdx, SkMC->GetMaterial(MatIdx));
	}

	CurrentVisualType = ESLVizVisualType::Skeletal;
}

// Set the visuals of the marker from the skeletal mesh component with custom color
void USLVizMarker::Init(USkeletalMeshComponent* SkMC, const FLinearColor& Color, bool bUnlit)
{
	// Clear any previously set visuals (mesh/materials)
	Reset();

	//SkeletalMesh = DuplicateObject<USkeletalMesh>(SkMC->SkeletalMesh, this);
	SkeletalMesh = SkMC->SkeletalMesh;

	UMaterialInstanceDynamic* DynMat = bUnlit ? UMaterialInstanceDynamic::Create(MaterialUnlit, NULL) : UMaterialInstanceDynamic::Create(MaterialLit, NULL);
	DynMat->SetVectorParameterValue(FName("Color"), Color);
	for (int32 MatIdx = 0; MatIdx < SkMC->GetNumMaterials(); ++MatIdx)
	{
		SkeletalMaterials.Emplace(MatIdx, DynMat);
	}

	//for (auto M : SkeletalMesh->Materials)
	//{
	//	M = DynMat;
	//}

	CurrentVisualType = ESLVizVisualType::Skeletal;
}

// Set the visuals of the marker from the bone (material index) skeletal mesh component with its original colors
void USLVizMarker::Init(USkeletalMeshComponent* SkMC, int32 MaterialIndex)
{
	if (MaterialIndex >= SkMC->GetNumMaterials())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Invalid MaterialIndex=%d .."), *FString(__FUNCTION__), __LINE__);
		return;
	}

	// Clear any previously set visuals (mesh/materials)
	Reset();

	//SkeletalMesh = DuplicateObject<USkeletalMesh>(SkMC->SkeletalMesh, this);
	SkeletalMesh = SkMC->SkeletalMesh;

	//int32 MatIdx = 0;
	//for (auto M : SkeletalMesh->Materials)
	//{
	//	if (MatIdx != MaterialIndex)
	//	{
	//		M = MaterialInvisible;
	//	}		
	//	MatIdx++;
	//}

	for (int32 MatIdx = 0; MatIdx < SkMC->GetNumMaterials(); ++MatIdx)
	{
		if (MatIdx != MaterialIndex)
		{
			SkeletalMaterials.Emplace(MatIdx, MaterialInvisible);
		}
		else
		{
			SkeletalMaterials.Emplace(MatIdx, SkMC->GetMaterial(MatIdx));
		}
	}

	CurrentVisualType = ESLVizVisualType::Skeletal;
}

// Set the visuals of the marker from the bone (material index) skeletal mesh component with custom color
void USLVizMarker::Init(USkeletalMeshComponent* SkMC, int32 MaterialIndex, const FLinearColor& Color, bool bUnlit)
{
	if (MaterialIndex >= SkMC->GetNumMaterials())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Invalid MaterialIndex=%d .."), *FString(__FUNCTION__), __LINE__);
		return;
	}

	// Clear any previously set visuals (mesh/materials)
	Reset();

	//SkeletalMesh = DuplicateObject<USkeletalMesh>(SkMC->SkeletalMesh, this);
	SkeletalMesh = SkMC->SkeletalMesh;

	UMaterialInstanceDynamic* DynMat = bUnlit ? UMaterialInstanceDynamic::Create(MaterialUnlit, NULL) : UMaterialInstanceDynamic::Create(MaterialLit, NULL);
	DynMat->SetVectorParameterValue(FName("Color"), Color);

	//int32 MatIdx = 0;
	//for (auto M : SkeletalMesh->Materials)
	//{
	//	if (MatIdx != MaterialIndex)
	//	{
	//		M = MaterialInvisible;
	//	}
	//	else
	//	{
	//		M = DynMat;
	//	}
	//	MatIdx++;
	//}

	for (int32 MatIdx = 0; MatIdx < SkMC->GetNumMaterials(); ++MatIdx)
	{
		if (MatIdx != MaterialIndex)
		{
			SkeletalMaterials.Emplace(MatIdx, MaterialInvisible);
		}
		else
		{
			SkeletalMaterials.Emplace(MatIdx, DynMat);
		}
	}

	CurrentVisualType = ESLVizVisualType::Skeletal;
}

// Set the visuals of the marker from the bones (material indexes) skeletal mesh component with its original colors
void USLVizMarker::Init(USkeletalMeshComponent* SkMC, TArray<int32>& MaterialIndexes)
{
	// Clear any previously set visuals (mesh/materials)
	Reset();

	//SkeletalMesh = DuplicateObject<USkeletalMesh>(SkMC->SkeletalMesh, this);
	SkeletalMesh = SkMC->SkeletalMesh;

	//int32 MatIdx = 0;
	//for (auto M : SkeletalMesh->Materials)
	//{
	//	if (!MaterialSlots.Contains(MatIdx))
	//	{
	//		M = MaterialInvisible;
	//	}
	//	MatIdx++;
	//}

	for (int32 MatIdx = 0; MatIdx < SkMC->GetNumMaterials(); ++MatIdx)
	{
		if (!MaterialIndexes.Contains(MatIdx))
		{
			SkeletalMaterials.Emplace(MatIdx, MaterialInvisible);
		}
		else
		{
			SkeletalMaterials.Emplace(MatIdx, SkMC->GetMaterial(MatIdx));
		}
	}

	CurrentVisualType = ESLVizVisualType::Skeletal;
}

// Set the visuals of the marker from the bones (material indexes) skeletal mesh component with custom color
void USLVizMarker::Init(USkeletalMeshComponent* SkMC, TArray<int32>& MaterialIndexes, const FLinearColor& Color, bool bUnlit)
{
	// Clear any previously set visuals (mesh/materials)
	Reset();

	//SkeletalMesh = DuplicateObject<USkeletalMesh>(SkMC->SkeletalMesh, this);
	SkeletalMesh = SkMC->SkeletalMesh;

	UMaterialInstanceDynamic* DynMat = bUnlit ? UMaterialInstanceDynamic::Create(MaterialUnlit, NULL) : UMaterialInstanceDynamic::Create(MaterialLit, NULL);
	DynMat->SetVectorParameterValue(FName("Color"), Color);

	//int32 MatIdx = 0;
	//for (auto M : SkeletalMesh->Materials)
	//{
	//	if (!MaterialSlots.Contains(MatIdx))
	//	{
	//		M = MaterialInvisible;
	//	}
	//	else
	//	{
	//		M = DynMat;
	//	}
	//	MatIdx++;
	//}

	for (int32 MatIdx = 0; MatIdx < SkMC->GetNumMaterials(); ++MatIdx)
	{
		if (!MaterialIndexes.Contains(MatIdx))
		{
			SkeletalMaterials.Emplace(MatIdx, MaterialInvisible);
		}
		else
		{
			SkeletalMaterials.Emplace(MatIdx, DynMat);
		}
	}

	CurrentVisualType = ESLVizVisualType::Skeletal;
}

// Add instances at location
void USLVizMarker::Add(const FVector& Location)
{
	if (CurrentVisualType == ESLVizVisualType::Static)
	{
		AddInstance(FTransform(FQuat::Identity, Location, Scale));
	}
	else if (CurrentVisualType == ESLVizVisualType::Skeletal)
	{
		UPoseableMeshComponent* SkelInstance = CreateNewSkeletalInstance();
		SkelInstance->SetWorldLocation(Location);
		SkeletalInstances.Emplace(SkelInstance);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Marker is not initialized.."), *FString(__FUNCTION__), __LINE__);
	}
}

// Add instances at pose
void USLVizMarker::Add(const FTransform& Pose)
{
	if (CurrentVisualType == ESLVizVisualType::Static)
	{
		AddInstance(FTransform(Pose.GetRotation(), Pose.GetLocation(), Scale));
	}
	else if (CurrentVisualType == ESLVizVisualType::Skeletal)
	{		
		UPoseableMeshComponent* SkelInstance = CreateNewSkeletalInstance();
		SkelInstance->SetWorldTransform(Pose);
		SkeletalInstances.Emplace(SkelInstance);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Marker is not initialized.."), *FString(__FUNCTION__), __LINE__);
	}
}

// Add instances with the locations
void USLVizMarker::Add(const TArray<FVector>& Locations)
{
	if (CurrentVisualType == ESLVizVisualType::Static)
	{
		for (const auto& L : Locations)
		{
			FTransform T(L);
			T.SetScale3D(Scale);
			AddInstance(T);
		}
	}
	else if (CurrentVisualType == ESLVizVisualType::Skeletal)
	{
		for (const auto& L : Locations)
		{
			UPoseableMeshComponent* SkelInstance = CreateNewSkeletalInstance();
			SkelInstance->SetWorldLocation(L);
			SkeletalInstances.Emplace(SkelInstance);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Marker is not initialized.."), *FString(__FUNCTION__), __LINE__);
	}
}

// Add instances with the poses
void USLVizMarker::Add(const TArray<FTransform>& Poses)
{
	if (CurrentVisualType == ESLVizVisualType::Static)
	{
		for (auto P : Poses)
		{
			P.SetScale3D(Scale);
			AddInstance(P);
		}
	}
	else if (CurrentVisualType == ESLVizVisualType::Skeletal)
	{
		for (const auto& P : Poses)
		{
			UPoseableMeshComponent* SkelInstance = CreateNewSkeletalInstance();
			SkelInstance->SetWorldTransform(P);
			SkeletalInstances.Emplace(SkelInstance);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Marker is not initialized.."), *FString(__FUNCTION__), __LINE__);
	}
}

// Add skeletal pose
void USLVizMarker::Add(TPair<FTransform, TMap<FString, FTransform>>& SkeletalPose)
{
	if (CurrentVisualType == ESLVizVisualType::Static)
	{
		AddInstance(SkeletalPose.Key);
	}
	else if (CurrentVisualType == ESLVizVisualType::Skeletal)
	{
		UPoseableMeshComponent* SkelInstance = CreateNewSkeletalInstance();
		SkelInstance->SetWorldTransform(SkeletalPose.Key);
		for (const auto& BonePose : SkeletalPose.Value)
		{
			SkelInstance->SetBoneTransformByName(*BonePose.Key, BonePose.Value, EBoneSpaces::WorldSpace);
		}
		SkeletalInstances.Emplace(SkelInstance);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Marker is not initialized.."), *FString(__FUNCTION__), __LINE__);
	}
}

// Add skeletal poses
void USLVizMarker::Add(const TArray<TPair<FTransform, TMap<FString, FTransform>>>& SkeletalPoses)
{
	if (CurrentVisualType == ESLVizVisualType::Static)
	{
		for (auto SkeletalPose : SkeletalPoses)
		{
			AddInstance(SkeletalPose.Key);
		}
	}
	else if (CurrentVisualType == ESLVizVisualType::Skeletal)
	{
		for (const auto& SkeletalPose : SkeletalPoses)
		{
			UPoseableMeshComponent* SkelInstance = CreateNewSkeletalInstance();
			SkelInstance->SetWorldTransform(SkeletalPose.Key);
			for (const auto& BonePose : SkeletalPose.Value)
			{
				SkelInstance->SetBoneTransformByName(*BonePose.Key, BonePose.Value, EBoneSpaces::WorldSpace);
			}
			SkeletalInstances.Emplace(SkelInstance);			
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Marker is not initialized.."), *FString(__FUNCTION__), __LINE__);
	}
}

// Destroy dynamically created components first
void USLVizMarker::DestroyComponent(bool bPromoteChildren/*= false*/)
{
	// TODO add delegate for the notifying the manager
	for (const auto& SkInst : SkeletalInstances)
	{
		if (SkInst->IsValidLowLevel())
		{
			SkInst->DestroyComponent();
		}
	}
	Super::DestroyComponent(bPromoteChildren);
}

// Clear any previously set visuals (mesh/materials)
void USLVizMarker::Reset()
{
	// Reset any previously initialized visual markers
	if (CurrentVisualType == ESLVizVisualType::Static)
	{
		ClearInstances();
		EmptyOverrideMaterials();
	}
	else if (CurrentVisualType == ESLVizVisualType::Skeletal)
	{
		for (auto SkI : SkeletalInstances)
		{
			SkI->DestroyComponent();
		}
		SkeletalInstances.Empty();
		SkeletalMesh->ConditionalBeginDestroy();
		SkeletalMesh = nullptr;
		//SkeletalMaterials.Empty();
	}
	CurrentVisualType = ESLVizVisualType::NONE;
}

// Create and register a new poseable mesh component
UPoseableMeshComponent* USLVizMarker::CreateNewSkeletalInstance()
{
	UPoseableMeshComponent* PMC = NewObject<UPoseableMeshComponent>(this);
	PMC->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PMC->SetSkeletalMesh(SkeletalMesh);
	PMC->bPerBoneMotionBlur = false;
	PMC->bHasMotionBlurVelocityMeshes = false;
	for (const auto MatPair : SkeletalMaterials)
	{
		PMC->SetMaterial(MatPair.Key, MatPair.Value);
	}
	PMC->AttachToComponent(this, FAttachmentTransformRules::KeepWorldTransform);
	PMC->RegisterComponent();	
	return PMC;
}

// Load marker mesh and material assets
void USLVizMarker::LoadAssets()
{
	/* Meshes */
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshBoxAsset(TEXT("StaticMesh'/USemLog/Viz/SM_Box1m.SM_Box1m'"));
	MeshBox = MeshBoxAsset.Object;
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshSphereAsset(TEXT("StaticMesh'/USemLog/Viz/SM_Sphere1m.SM_Sphere1m'"));
	MeshSphere = MeshSphereAsset.Object;
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshCylinderAsset(TEXT("StaticMesh'/USemLog/Viz/SM_Cylinder1m.SM_Cylinder1m'"));
	MeshCylinder = MeshCylinderAsset.Object;
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshArrowAsset(TEXT("StaticMesh'/USemLog/Viz/SM_Arrow1m.SM_Arrow1m'"));
	MeshArrow = MeshArrowAsset.Object;
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAxisAsset(TEXT("StaticMesh'/USemLog/Viz/SM_Axis1m.SM_Axis1m'"));
	MeshAxis = MeshAxisAsset.Object;

	/* Materials */
	static ConstructorHelpers::FObjectFinder<UMaterial>MaterialLitAsset(TEXT("Material'/USemLog/Viz/M_MarkerDynamicColorLit.M_MarkerDynamicColorLit'"));
	MaterialLit = MaterialLitAsset.Object;
	static ConstructorHelpers::FObjectFinder<UMaterial>MaterialUnlitAsset(TEXT("Material'/USemLog/Viz/M_MarkerDynamicColorUnlit.M_MarkerDynamicColorUnlit'"));
	MaterialUnlit = MaterialUnlitAsset.Object;
	static ConstructorHelpers::FObjectFinder<UMaterial>MaterialInvisibleAsset(TEXT("Material'/USemLog/Viz/M_MarkerInvisible.M_MarkerInvisible'"));
	MaterialInvisible = MaterialInvisibleAsset.Object;
}

// Load assets container
bool USLVizMarker::LoadAssetsContainer()
{
	static ConstructorHelpers::FObjectFinder<USLVizAssets>VizAssetsContainerAsset(AssetsContainerPath);
	if (VizAssetsContainerAsset.Succeeded())
	{
		VizAssetsContainer = VizAssetsContainerAsset.Object;

		// Check if all assets in the container are set
		bool RetVal = true;

		/* Meshes */
		if (VizAssetsContainer->MeshBox == nullptr) {
			UE_LOG(LogTemp, Error, TEXT("%s::%d Assets container MeshBox is NULL.."), *FString(__FUNCTION__), __LINE__);
			RetVal = false;
		}
		if (VizAssetsContainer->MeshSphere == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Assets container MeshSphere is NULL.."), *FString(__FUNCTION__), __LINE__);
			RetVal = false;
		}
		if (VizAssetsContainer->MeshCylinder == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Assets container MeshCylinder is NULL.."), *FString(__FUNCTION__), __LINE__);
			RetVal = false;
		}
		if (VizAssetsContainer->MeshArrow == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Assets container MeshArrow is NULL.."), *FString(__FUNCTION__), __LINE__);
			RetVal = false;
		}
		if (VizAssetsContainer->MeshAxis == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Assets container MeshAxis is NULL.."), *FString(__FUNCTION__), __LINE__);
			RetVal = false;
		}

		/* Materials */
		if (VizAssetsContainer->MaterialLit == nullptr) {
			UE_LOG(LogTemp, Error, TEXT("%s::%d Assets container MaterialLit is NULL.."), *FString(__FUNCTION__), __LINE__);
			RetVal = false;
		}
		if (VizAssetsContainer->MaterialUnlit == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Assets container MaterialUnlit is NULL.."), *FString(__FUNCTION__), __LINE__);
			RetVal = false;
		}
		if (VizAssetsContainer->MaterialInvisible == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Assets container MaterialInvisible is NULL.."), *FString(__FUNCTION__), __LINE__);
			RetVal = false;
		}

		return RetVal;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not find the assets container at Path=%s.."),
			*FString(__FUNCTION__), __LINE__, AssetsContainerPath);
		return false;
	}
}

// Get the marker static mesh from its type
UStaticMesh * USLVizMarker::GetPrimitiveMarkerMesh(ESLVizMarkerType Type) const
{
	switch (Type)
	{
	case ESLVizMarkerType::Box:
		return MeshBox;
	case ESLVizMarkerType::Sphere:
		return MeshSphere;
	case ESLVizMarkerType::Cylinder:
		return MeshCylinder;
	case ESLVizMarkerType::Arrow:
		return MeshArrow;
	case ESLVizMarkerType::Axis:
		return MeshAxis;
	default:
		return MeshBox;
	}
}
