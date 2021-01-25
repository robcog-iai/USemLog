// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "CV/SLCVQScene.h"
#include "Individuals/SLIndividualManager.h"
#include "Individuals/Type/SLVisibleIndividual.h"
#include "Individuals/Type/SLSkeletalIndividual.h"
#include "Individuals/Type/SLBoneIndividual.h"
#include "Mongo/SLMongoQueryManager.h"
#include "Engine/StaticMeshActor.h"
#include "Animation/SkeletalMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"

#include "Skeletal/SLPoseableMeshActorWithMask.h"
#include "EngineUtils.h"

#if WITH_EDITOR
#include "Engine/Selection.h"
#include "Editor.h"
#include "Individuals/SLIndividualUtils.h"
#include "Individuals/Type/SLBaseIndividual.h"
#endif // WITH_EDITOR

#if SL_WITH_DEBUG
#include "DrawDebugHelpers.h"
#endif // SL_WITH_DEBUG

// Set the scene actors and cache their relative transforms to the world root
bool USLCVQScene::InitScene(ASLIndividualManager* IndividualManager, ASLMongoQueryManager* MQManager)
{
	// Clear any previous data
	if (SceneActorPoses.Num() > 0 
		|| PoseableMeshCloneOfMap.Num() > 0
		|| ScenePoseableActorPoses.Num() > 0
		|| StaticMaskClones.Num() > 0 
		)
	{
		SceneActorPoses.Empty();
		PoseableMeshCloneOfMap.Empty();
		ScenePoseableActorPoses.Empty();
		StaticMaskClones.Empty();
	}

	if (bIgnore)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is set to be ignored, skipping execution.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return false;
	}

	if (!IndividualManager || !IndividualManager->IsValidLowLevel() || IndividualManager->IsPendingKillOrUnreachable() || !IndividualManager->IsLoaded())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %'s individual manager is not valid/loaded, aborting execution.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return false;
	}

	return InitSceneImpl(IndividualManager, MQManager);
}

// Public execute function
void USLCVQScene::ShowScene()
{
	// Move static mesh actors (and mask clones) to the given poses
	for (const auto& ActPosePair : SceneActorPoses)
	{
		AStaticMeshActor* CurrSMA = ActPosePair.Key;
		FTransform CurrPose = ActPosePair.Value;
		CurrSMA->SetActorHiddenInGame(false);

		// Set the actor with the original materials in its location
		CurrSMA->SetActorTransform(CurrPose);
		
		// Set the clones in their locations
		if (auto* CurrClone = StaticMaskClones.Find(CurrSMA))
		{
			(*CurrClone)->SetWorldTransform(CurrPose);
		}
	}

	for (const auto& SkelActPosePair : ScenePoseableActorPoses)
	{
		ASLPoseableMeshActorWithMask* CurrSkelMA = SkelActPosePair.Key;
		FTransform CurrSkelMAPose = SkelActPosePair.Value.Key;
		CurrSkelMA->SetActorHiddenInGame(false);

		// Set actor pose and bone pose
		CurrSkelMA->SetSkeletalPose(SkelActPosePair.Value);
	}
}

// Hide scene
void USLCVQScene::HideScene()
{
	for (const auto& ActPosePair : SceneActorPoses)
	{
		ActPosePair.Key->SetActorHiddenInGame(true);
	}

	for (const auto& SkelActPosePair : ScenePoseableActorPoses)
	{
		SkelActPosePair.Key->SetActorHiddenInGame(true);
	}
}

// Generate mask clones for the scene
bool USLCVQScene::GenerateMaskClones(const TCHAR* MaterialPath, bool bUseIndividualMaskValue, FColor MaskColor)
{
	// Get the dynamic mask material
	UMaterial* DefaultMaskMaterial = LoadObject<UMaterial>(this, MaterialPath);
	if (!DefaultMaskMaterial)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s could not load default mask material.."),
			*FString(__func__), __LINE__, *GetName());
		return false;
	}
	DefaultMaskMaterial->bUsedWithStaticLighting = true;
	DefaultMaskMaterial->bUsedWithSkeletalMesh = true;

	// Create a common dynamic mask material and set its color
	UMaterialInstanceDynamic* DynamicMaskMaterial = nullptr;
	if (!bUseIndividualMaskValue)
	{
		DynamicMaskMaterial = UMaterialInstanceDynamic::Create(DefaultMaskMaterial, GetTransientPackage());
		DynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"), MaskColor);
	}

	// Clone static meshes
	for (const auto& ActPosePair : SceneActorPoses)
	{
		AStaticMeshActor* CurrSMA = ActPosePair.Key;
		if (USLBaseIndividual* BI = FSLIndividualUtils::GetIndividualObject(CurrSMA))
		{
			// Make sure individual is of type visible
			if (auto VI = Cast<USLVisibleIndividual>(BI))
			{
				// Check if the actor already has a clone
				for (const auto& Comp : CurrSMA->GetComponentsByClass(UStaticMeshComponent::StaticClass()))
				{
					if (Comp->GetName().EndsWith("_CVQSceneMaskClone"))
					{
						// There is alrady a clone, add to array
						StaticMaskClones.Add(CurrSMA, CastChecked<UStaticMeshComponent>(Comp));
						continue;
					}
				}

				// Duplicate/clone the static mesh component
				UStaticMeshComponent* OrigSMC = CurrSMA->GetStaticMeshComponent();
				//UStaticMeshComponent* CloneSMC = NewObject<UStaticMeshComponent>(CurrSMA);
				//CloneSMC->SetStaticMesh(OrigSMC->GetStaticMesh());
				UStaticMeshComponent* MaskCloneSMC = DuplicateObject<UStaticMeshComponent>(OrigSMC, CurrSMA,
					FName(*OrigSMC->GetName().Append("_CVQSceneMaskClone")));
				MaskCloneSMC->SetSimulatePhysics(false);
				MaskCloneSMC->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				//MaskCloneSMC->AttachToComponent(OrigSMC, FAttachmentTransformRules::SnapToTargetIncludingScale);
				//MaskCloneSMC->SetWorldTransform(OrigSMC->GetComponentTransform());
				//MaskCloneSMC->SetRelativeTransform(FTransform::Identity);
				//MaskCloneSMC->SetRelativeLocationAndRotation(FVector::ZeroVector, FQuat::Identity);

				// Check if an individual mask needs to be created
				if (bUseIndividualMaskValue)
				{
					// Use the individual unique visual mask value for the mask
					DynamicMaskMaterial = UMaterialInstanceDynamic::Create(DefaultMaskMaterial, GetTransientPackage());
					DynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"), FColor::FromHex(VI->GetVisualMaskValue()));
				}

				// Apply the dynamic mask material to the mesh
				for (int32 MatIdx = 0; MatIdx < MaskCloneSMC->GetNumMaterials(); ++MatIdx)
				{
					MaskCloneSMC->SetMaterial(MatIdx, DynamicMaskMaterial);
				}

				// Register with actor
				CurrSMA->AddOwnedComponent(MaskCloneSMC);
				CurrSMA->AddInstanceComponent(MaskCloneSMC);
				MaskCloneSMC->OnComponentCreated();
				MaskCloneSMC->RegisterComponent();
				CurrSMA->RerunConstructionScripts();

				// Not visible by default
				MaskCloneSMC->SetVisibility(false);

				// Add to map
				StaticMaskClones.Add(CurrSMA, MaskCloneSMC);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d %s does not have a visible individual.. this should not happen"),
					*FString(__FUNCTION__), __LINE__, *CurrSMA->GetName());
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s does not have an individual.. this should not happen"),
				*FString(__FUNCTION__), __LINE__, *CurrSMA->GetName());
		}
	}
	
	// Setup skeletal masks
	for (const auto& SkelActPosePair : ScenePoseableActorPoses)
	{
		ASLPoseableMeshActorWithMask* CurrPoseableClone = SkelActPosePair.Key;
		if (auto* CurrSkelMA = PoseableMeshCloneOfMap.Find(CurrPoseableClone))
		{
			if (USLBaseIndividual* BI = FSLIndividualUtils::GetIndividualObject(*CurrSkelMA))
			{
				// Make sure individual is of type visible
				if (auto VI = Cast<USLVisibleIndividual>(BI))
				{
					// Check if an individual mask needs to be created
					if (bUseIndividualMaskValue)
					{
						if (auto SkelI = Cast<USLSkeletalIndividual>(BI))
						{
							for (const auto& BoneI : SkelI->GetBoneIndividuals())
							{
								DynamicMaskMaterial = UMaterialInstanceDynamic::Create(DefaultMaskMaterial, GetTransientPackage());
								DynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"),
									FColor::FromHex(BoneI->GetVisualMaskValue()));
								CurrPoseableClone->SetCustomMaterial(BoneI->GetMaterialIndex(), DynamicMaskMaterial);
							}
						}
						else
						{
							UE_LOG(LogTemp, Error, TEXT("%s::%d skeletal actor %s does not have a skeletal individual representation.. "),
								*FString(__FUNCTION__), __LINE__, *(*CurrSkelMA)->GetName());
						}
					}
					else
					{
						CurrPoseableClone->SetCustomMaterial(DynamicMaskMaterial);
					}
				}
			}
		}
	}

	return StaticMaskClones.Num() > 0 || ScenePoseableActorPoses.Num() > 0;
}

// Show mask values of the scenes
void USLCVQScene::ShowMaskMaterials()
{
	for (const auto& SMPair : StaticMaskClones)
	{
		SMPair.Key->GetStaticMeshComponent()->SetVisibility(false);
		SMPair.Value->SetVisibility(true);
	}

	for (const auto& PoseablePair : ScenePoseableActorPoses)
	{
		PoseablePair.Key->ShowMask();
	}
}

// Show original material
void USLCVQScene::ShowOriginalMaterials()
{
	for (const auto& SMPair : StaticMaskClones)
	{
		SMPair.Key->GetStaticMeshComponent()->SetVisibility(true);
		SMPair.Value->SetVisibility(false);
	}

	for (const auto& PoseablePair : ScenePoseableActorPoses)
	{
		PoseablePair.Key->ShowOriginal();
	}
}

// Get the scene name
FString USLCVQScene::GetSceneName()
{
	if (SceneName.IsEmpty())
	{
		SceneName = GetName();
	}
	return SceneName;
}

// Get the bounding sphere radius of the applied scene
float USLCVQScene::GetAppliedSceneSphereBoundsRadius() const
{
	FBoxSphereBounds SphereBounds(EForceInit::ForceInit);

	// Add up static mesh bounds
	for (const auto& SMAPosePair : SceneActorPoses)
	{
		// Get the mesh bounds
		FBoxSphereBounds SMBounds = SMAPosePair.Key->GetStaticMeshComponent()->Bounds;

		// Set first value, or add the next ones
		if (SphereBounds.SphereRadius > 0.f)
		{
			SphereBounds = SphereBounds + SMBounds;
		}
		else
		{
			// First value
			SphereBounds = SMBounds;
		}
	}

	// Add up skeletal mesh bounds
	for (const auto& SkelMAPosePair : ScenePoseableActorPoses)
	{
		// Get the original poseable mesh bounds		
		UPoseableMeshComponent* OrigClone = SkelMAPosePair.Key->GetPoseableMeshComponent();
		FBoxSphereBounds SMBounds = OrigClone->Bounds;

		// Set first value, or add the next ones
		if (SphereBounds.SphereRadius > 0.f)
		{
			SphereBounds = SphereBounds + SMBounds;
		}
		else
		{
			// First value
			SphereBounds = SMBounds;
		}
	}

	return SphereBounds.SphereRadius;
}

#if WITH_EDITOR
// Called when a property is changed in the editor
void USLCVQScene::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (bIgnore)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is set to be ignored, ignoring property change events.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	// Get the changed property name
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ?
		PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(USLCVQScene, bAddSelectedButton))
	{
		bAddSelectedButton = false;
		if (bOverwrite)
		{
			Ids.Empty();
		}
		for (FSelectionIterator It(GEditor->GetSelectedActorIterator()); It; ++It)
		{
			AActor* SelectedActor = CastChecked<AActor>(*It);
			
			if (SelectedActor->IsA(AStaticMeshActor::StaticClass()) ||
				SelectedActor->IsA(ASkeletalMeshActor::StaticClass()))
			{
				if (USLBaseIndividual* BI = FSLIndividualUtils::GetIndividualObject(SelectedActor))
				{
					if (BI->IsLoaded())
					{
						bEnsureUniqueness ? Ids.AddUnique(BI->GetIdValue()) : Ids.Add(BI->GetIdValue());
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's individual is not loaded.."),
							*FString(__FUNCTION__), __LINE__, *SelectedActor->GetName());
					}
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("%s::%d %s has no individual representation.."),
						*FString(__FUNCTION__), __LINE__, *SelectedActor->GetName());
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is not a skeletal or static mesh actor, skipping.."),
					*FString(__FUNCTION__), __LINE__, *SelectedActor->GetName());
			}
		}
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLCVQScene, bRemoveSelectedButton))
	{
		bRemoveSelectedButton = false;
		for (FSelectionIterator It(GEditor->GetSelectedActorIterator()); It; ++It)
		{
			AActor* SelectedActor = CastChecked<AActor>(*It);
			if (USLBaseIndividual* BI = FSLIndividualUtils::GetIndividualObject(SelectedActor))
			{
				if (BI->IsLoaded())
				{
					Ids.Remove(BI->GetIdValue());
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's individual is not loaded.."),
						*FString(__FUNCTION__), __LINE__, *SelectedActor->GetName());
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d %s has no individual representation.."),
					*FString(__FUNCTION__), __LINE__, *SelectedActor->GetName());
			}
		}
	}
}
#endif // WITH_EDITOR

// Virtual implementation for the scene initialization
bool USLCVQScene::InitSceneImpl(ASLIndividualManager* IndividualManager, ASLMongoQueryManager* MQManager)
{
	if (!MQManager || !MQManager->IsValidLowLevel() || MQManager->IsPendingKillOrUnreachable())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %'s mongo query manager is not valid, aborting execution.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return false;
	}

	if (!MQManager->IsConnected())
	{
		if (!MQManager->Connect(MongoIp, MongoPort))
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %'s mongo query manager could not connect to %s::%s, aborting execution.."),
				*FString(__FUNCTION__), __LINE__, *GetName(), *MongoIp, *FString::FromInt(MongoPort));
			return false;
		}
	}

	if (!MQManager->SetTask(Task) || !MQManager->SetEpisode(Episode))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %'s mongo query manager could not set task %s or episode %s, aborting execution.."),
			*FString(__FUNCTION__), __LINE__, *GetName(), *Task, *Episode);
		return false;
	}

#if SL_WITH_DEBUG
	ActiveWorld = IndividualManager->GetWorld();
#endif // SL_WITH_DEBUG

	if (!SetSceneActors(IndividualManager, MQManager))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %'s could not set up scene actors, aborting execution.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return false;
	}

	// Get the center location of the scene
	FVector SceneCenterLocation = CalcSceneCenterPose();

	// Move scene to root (scan) location (0,0,0) using calculated offset
	MoveSceneToRootLocation(SceneCenterLocation);	

	return true;
}

// Iterate ids, set up scene actors
bool USLCVQScene::SetSceneActors(ASLIndividualManager* IndividualManager, ASLMongoQueryManager* MQManager)
{
	// Iterate the scene actors, cache their original world position,
	for (const auto& Id : Ids)
	{
		if (auto CurrActor = IndividualManager->GetIndividualActor(Id))
		{
			if (auto* AsSMA = Cast<AStaticMeshActor>(CurrActor))
			{
				// Cache the episodic memory world pose
				FTransform EpMemPose = MQManager->GetIndividualPoseAt(Id, Timestamp);
				SceneActorPoses.Add(AsSMA, EpMemPose);

#if SL_WITH_DEBUG && ENABLE_DRAW_DEBUG
				UE_LOG(LogTemp, Error, TEXT("%s::%d Actor %s:\n (RED)SemMapLoc=%s; (GREEN)EpMemLoc=%s;"),
					*FString(__FUNCTION__), __LINE__, *AsSMA->GetName(),
					*AsSMA->GetActorLocation().ToString(),
					*EpMemPose.GetLocation().ToString());

				if (ActiveWorld && !ActiveWorld->IsPendingKillOrUnreachable())
				{
					DrawDebugPoint(ActiveWorld, AsSMA->GetActorLocation(), 10.f, FColor::Red, true);
					DrawDebugPoint(ActiveWorld, EpMemPose.GetLocation(), 10.f, FColor::Green, true);
					DrawDebugLine(ActiveWorld, AsSMA->GetActorLocation(), EpMemPose.GetLocation(), FColor::Yellow, true);
				}
#endif // SL_WITH_DEBUG && ENABLE_DRAW_DEBUG
			}
			else if (auto* AsSkelMA = Cast<ASkeletalMeshActor>(CurrActor))
			{
				// Store ep memory skel pose
				TPair<FTransform, TMap<int32, FTransform>> EpMemSkelPose = MQManager->GetSkeletalIndividualPoseAt(Id, Timestamp);
				//SceneSkelActorPoses.Add(AsSkelMA, EpMemSkelPose);

				// Name of the poseable mesh
				const FString PoseableActorName = AsSkelMA->GetName() + TEXT("_CVQSceneClone");

				// Check if the actor already has a clone
				for (TActorIterator<ASkeletalMeshActor>Iter(IndividualManager->GetWorld()); Iter; ++Iter)
				{
					if (!(*Iter)->IsPendingKillOrUnreachable())
					{
						(*Iter)->GetName().Equals(PoseableActorName);
						continue;
					}
				}

				// Create a clone actor of the skeletal actor
				FActorSpawnParameters SpawnParams;

				SpawnParams.Name = FName(*PoseableActorName);
				auto* PoseableCloneAct = IndividualManager->GetWorld()->SpawnActor<ASLPoseableMeshActorWithMask>(SpawnParams);
#if WITH_EDITOR
				PoseableCloneAct->SetActorLabel(PoseableActorName);
#endif // WITH_EDITOR
				PoseableCloneAct->SetSkeletalMeshAndPose(AsSkelMA);
				ScenePoseableActorPoses.Add(PoseableCloneAct, EpMemSkelPose);

				// Keep a mapping to the original actor
				PoseableMeshCloneOfMap.Add(PoseableCloneAct, AsSkelMA);

#if SL_WITH_DEBUG && ENABLE_DRAW_DEBUG
				if (ActiveWorld && !ActiveWorld->IsPendingKillOrUnreachable())
				{
					// Sem map location
					USkeletalMeshComponent* SkelMeshComp = AsSkelMA->GetSkeletalMeshComponent();
					DrawDebugPoint(ActiveWorld, AsSkelMA->GetActorLocation(), 10.f, FColor::Red, true);
					DrawDebugSphere(ActiveWorld, SkelMeshComp->GetComponentLocation(), 5.f, 8, FColor::Red, true);
					for (int32 BIdx = 0; BIdx < SkelMeshComp->GetNumBones(); BIdx++)
					{
						FVector CurrBoneLocation = SkelMeshComp->GetBoneTransform(BIdx).GetLocation();
						DrawDebugPoint(ActiveWorld, CurrBoneLocation, 5.f, FColor::Red, true);
						DrawDebugLine(ActiveWorld, CurrBoneLocation, AsSkelMA->GetActorLocation(), FColor::Red, true);
					}
					
					// Ep mem location
					DrawDebugPoint(ActiveWorld, EpMemSkelPose.Key.GetLocation(), 20.f, FColor::Green, true);
					for (const auto& BonePosePair : EpMemSkelPose.Value)
					{
						FVector CurrBoneLocation = BonePosePair.Value.GetLocation();
						DrawDebugPoint(ActiveWorld, CurrBoneLocation, 5.f, FColor::Green, true);
						DrawDebugLine(ActiveWorld, CurrBoneLocation, EpMemSkelPose.Key.GetLocation(), FColor::Red, true);
					}
				}
#endif // SL_WITH_DEBUG && ENABLE_DRAW_DEBUG
			}
		}
	}

#if SL_WITH_DEBUG && ENABLE_DRAW_DEBUG
	if (ActiveWorld && !ActiveWorld->IsPendingKillOrUnreachable())
	{
		DrawDebugPoint(ActiveWorld, FVector::ZeroVector, 25.f, FColor::White, true);
	}
#endif // SL_WITH_DEBUG && ENABLE_DRAW_DEBUG

	return SceneActorPoses.Num() > 0 || ScenePoseableActorPoses.Num() > 0;
}

// Calculate scene center pose
FVector USLCVQScene::CalcSceneCenterPose()
{
	// Calculate centroid location
	FBoxSphereBounds SphereBounds(EForceInit::ForceInit);
	// Add up static mesh bounds
	for (const auto& SMAPosePair : SceneActorPoses)
	{
		AStaticMeshActor* CurrSMA = SMAPosePair.Key;
		FTransform CurrEpMemPose = SMAPosePair.Value;

		// Get the mesh bounds
		FBoxSphereBounds SMBounds = CurrSMA->GetStaticMeshComponent()->Bounds;

#if SL_WITH_DEBUG && ENABLE_DRAW_DEBUG
		if (ActiveWorld && !ActiveWorld->IsPendingKillOrUnreachable())
		{
			DrawDebugSphere(ActiveWorld, SMBounds.Origin, SMBounds.SphereRadius, 16, FColor::Red, true);
			DrawDebugSphere(ActiveWorld, CurrEpMemPose.GetLocation(), SMBounds.SphereRadius, 16, FColor::Green, true);
		}
#endif // SL_WITH_DEBUG && ENABLE_DRAW_DEBUG

		// Offset the bounds origin to the episodic memory location
		SMBounds.Origin = CurrEpMemPose.GetLocation();

		// Set first value, or add the next ones
		if (SphereBounds.SphereRadius > 0.f)
		{
			SphereBounds = SphereBounds + SMBounds;
		}
		else
		{
			// First value
			SphereBounds = SMBounds;
		}
	}
	// Add up skeletal mesh bounds
	for (const auto& SkelMAPosePair : ScenePoseableActorPoses)
	{
		// Get the original poseable mesh bounds
		UPoseableMeshComponent* OrigClone = SkelMAPosePair.Key->GetPoseableMeshComponent();
		FBoxSphereBounds SkelMBounds = OrigClone->Bounds;

#if SL_WITH_DEBUG && ENABLE_DRAW_DEBUG
		if (ActiveWorld && !ActiveWorld->IsPendingKillOrUnreachable())
		{
			DrawDebugSphere(ActiveWorld, SkelMBounds.Origin, SkelMBounds.SphereRadius, 16, FColor::Red, true);
			DrawDebugSphere(ActiveWorld, OrigClone->GetComponentLocation(), SkelMBounds.SphereRadius, 16, FColor::Green, true);
		}
#endif // SL_WITH_DEBUG && ENABLE_DRAW_DEBUG

		// The skeletal meshes do not have their root pose in the center of the mesh,
		// we then need to apply this offset when moving the origin of the bounds to the ep memory location
		FVector SkelCenterOffset = OrigClone->GetComponentLocation() - SkelMBounds.Origin;

		UE_LOG(LogTemp, Error, TEXT("%s::%d %s::%s's SkelCenterOffset=%s;"),
			*FString(__FUNCTION__), __LINE__,
			*GetName(),
			*SkelMAPosePair.Key->GetName(),
			*SkelCenterOffset.ToString());

		// Offset the bounds origin to the episodic memory location
		if (auto SkelPose = ScenePoseableActorPoses.Find(SkelMAPosePair.Key))
		{
			SkelMBounds.Origin = (*SkelPose).Key.GetLocation() - SkelCenterOffset;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s this should not happen, %s should be in the scene.."),
				*FString(__FUNCTION__), __LINE__, *GetName(), *SkelMAPosePair.Key->GetName());
		}

		UE_LOG(LogTemp, Error, TEXT("%s::%d %s::%s's SkelMBounds.Origin=%s;"),
			*FString(__FUNCTION__), __LINE__,
			*GetName(),
			*SkelMAPosePair.Key->GetName(),
			*SkelMBounds.Origin.ToString());

		// Set first value, or add the next ones
		if (SphereBounds.SphereRadius > 0.f)
		{
			SphereBounds = SphereBounds + SkelMBounds;
		}
		else
		{
			// First value
			SphereBounds = SkelMBounds;
		}
	}

#if SL_WITH_DEBUG && ENABLE_DRAW_DEBUG
	if (ActiveWorld && !ActiveWorld->IsPendingKillOrUnreachable())
	{
		DrawDebugSphere(ActiveWorld, FVector::ZeroVector, SphereBounds.SphereRadius, 16, FColor::White, true);
	}
#endif // SL_WITH_DEBUG && ENABLE_DRAW_DEBUG

	return SphereBounds.Origin;
}

// Move scene to root (scan) location (0,0,0)
void USLCVQScene::MoveSceneToRootLocation(FVector Offset)
{
	// Move scene to root
	for (auto& ActPosePair : SceneActorPoses)
	{
		ActPosePair.Value.AddToTranslation(-Offset);
	}

	for (auto& SkelActPosePair : ScenePoseableActorPoses)
	{
		// Reference to the skel actor and bone pose world pose
		TPair<FTransform, TMap<int32, FTransform>>& SkelPoseRef = SkelActPosePair.Value;

		// Moving the skel actor is probably not needed
		SkelPoseRef.Key.AddToTranslation(-Offset);

		// Moving the bones is needed
		for (auto& BonePosePair : SkelPoseRef.Value)
		{
			BonePosePair.Value.AddToTranslation(-Offset);
		}
	}
}

