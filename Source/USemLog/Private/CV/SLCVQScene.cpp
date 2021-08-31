// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
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
	// Non-offseted poses, to re-calculate bounds
	// Redundant call with InitSceneImpl, required otherwise the bounds are not updated
	ApplyPoses();

	// Add osset to the cached poses
	FVector SceneOrigin = CalcSceneOrigin();
	AddOffsetToScene(-SceneOrigin);

	// Apply offseted (moved to 0,0,0) poes
	ApplyPoses();
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
#if ENGINE_MINOR_VERSION > 23 || ENGINE_MAJOR_VERSION > 4
				TArray<UActorComponent*> Components;
				CurrSMA->GetComponents(UStaticMeshComponent::StaticClass(), Components);
				for (const auto& Comp : Components)
				{
					if (Comp->GetName().EndsWith("_CVQSceneMaskClone"))
					{
						// There is alrady a clone, add to array
						StaticMaskClones.Add(CurrSMA, CastChecked<UStaticMeshComponent>(Comp));
						continue;
					}
				}
#else
				for (const auto& Comp : CurrSMA->GetComponentsByClass(UStaticMeshComponent::StaticClass()))
				{
					if (Comp->GetName().EndsWith("_CVQSceneMaskClone"))
					{
						// There is alrady a clone, add to array
						StaticMaskClones.Add(CurrSMA, CastChecked<UStaticMeshComponent>(Comp));
						continue;
					}
				}
#endif

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
		UPoseableMeshComponent* PoseableMeshComp = SkelMAPosePair.Key->GetPoseableMeshComponent();
		FBoxSphereBounds PoseableMBounds = PoseableMeshComp->Bounds;

		// Set first value, or add the next ones
		if (SphereBounds.SphereRadius > 0.f)
		{
			SphereBounds = SphereBounds + PoseableMBounds;
		}
		else
		{
			// First value
			SphereBounds = PoseableMBounds;
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
	
	// Dummy call, for some reason it is required otherwise the skeletal bounding boxes
	// are not updated in the show scene
	ApplyPoses();

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
			}
			else if (auto* AsSkelMA = Cast<ASkeletalMeshActor>(CurrActor))
			{
				// Store ep memory skel pose
				TPair<FTransform, TMap<int32, FTransform>> EpMemSkelPose = MQManager->GetSkeletalIndividualPoseAt(Id, Timestamp);

				// Name of the poseable mesh
				const FString PoseableActorName = AsSkelMA->GetName() + TEXT("_CVQSceneClone");

				// Check if the actor already has a clone
				for (TActorIterator<ASkeletalMeshActor>Iter(IndividualManager->GetWorld()); Iter; ++Iter)
				{
					if (!(*Iter)->IsPendingKillOrUnreachable())
					{
						if ((*Iter)->GetName().Equals(PoseableActorName)) {
							continue;
						}
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

				// Hide by default
				//PoseableCloneAct->SetActorHiddenInGame(true);

#if SL_WITH_DEBUG && ENABLE_DRAW_DEBUG
				if (ActiveWorld && !ActiveWorld->IsPendingKillOrUnreachable())
				{
					// Sem map location
					// Orig
					//USkeletalMeshComponent* SkelMeshComp = AsSkelMA->GetSkeletalMeshComponent();
					//DrawDebugSphere(ActiveWorld, AsSkelMA->GetActorLocation(), 3.f, 4, FColor::Blue, true);
					//DrawDebugSphere(ActiveWorld, SkelMeshComp->GetComponentLocation(), 5.f, 8, FColor::Blue, true);
					//for (int32 BIdx = 0; BIdx < SkelMeshComp->GetNumBones(); BIdx++)
					//{
					//	FVector CurrBoneLocation = SkelMeshComp->GetBoneTransform(BIdx).GetLocation();
					//	DrawDebugPoint(ActiveWorld, CurrBoneLocation, 5.f, FColor::Blue, true);
					//	DrawDebugLine(ActiveWorld, CurrBoneLocation, AsSkelMA->GetActorLocation(), FColor::Blue, true);
					//}
					//DrawDebugSphere(ActiveWorld, SkelMeshComp->Bounds.Origin, SkelMeshComp->Bounds.SphereRadius, 16, FColor::Blue, true);
					//DrawDebugSphere(ActiveWorld, SkelMeshComp->Bounds.Origin, 5.f, 4, FColor::Blue, true);
					//DrawDebugLine(ActiveWorld, SkelMeshComp->Bounds.Origin, AsSkelMA->GetActorLocation(), FColor::Yellow, true);

					// Clone
					//UPoseableMeshComponent* PoseableMeshComp = PoseableCloneAct->GetPoseableMeshComponent();
					//DrawDebugSphere(ActiveWorld, PoseableCloneAct->GetActorLocation(), 3.f, 4, FColor::Magenta, true);
					//DrawDebugSphere(ActiveWorld, PoseableMeshComp->GetComponentLocation(), 5.f, 8, FColor::Magenta, true);
					//for (int32 BIdx = 0; BIdx < PoseableMeshComp->GetNumBones(); BIdx++)
					//{
					//	FVector CurrBoneLocation = PoseableMeshComp->GetBoneTransform(BIdx).GetLocation();
					//	DrawDebugPoint(ActiveWorld, CurrBoneLocation, 5.f, FColor::Magenta, true);
					//	DrawDebugLine(ActiveWorld, CurrBoneLocation, PoseableCloneAct->GetActorLocation(), FColor::Magenta, true);
					//}
					//DrawDebugSphere(ActiveWorld, PoseableMeshComp->Bounds.Origin, PoseableMeshComp->Bounds.SphereRadius, 16, FColor::Magenta, true);
					//DrawDebugSphere(ActiveWorld, PoseableMeshComp->Bounds.Origin, 5.f, 4, FColor::Magenta, true);
					//DrawDebugLine(ActiveWorld, PoseableMeshComp->Bounds.Origin, PoseableCloneAct->GetActorLocation(), FColor::Yellow, true);

					// Ep mem location
					//DrawDebugSphere(ActiveWorld, EpMemSkelPose.Key.GetLocation(), 3.f, 4, FColor::White, true);
					//for (const auto& BonePosePair : EpMemSkelPose.Value)
					//{
					//	FVector CurrBoneLocation = BonePosePair.Value.GetLocation();
					//	DrawDebugPoint(ActiveWorld, CurrBoneLocation, 5.f, FColor::White, true);
					//	DrawDebugLine(ActiveWorld, CurrBoneLocation, EpMemSkelPose.Key.GetLocation(), FColor::White, true);
					//}
				}
#endif // SL_WITH_DEBUG && ENABLE_DRAW_DEBUG
			}
		}
	}
	return SceneActorPoses.Num() > 0 || ScenePoseableActorPoses.Num() > 0;
}

// Calculate scene center pose
FVector USLCVQScene::CalcSceneOrigin()
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
	for (const auto& SkelActPosePair : ScenePoseableActorPoses)
	{
		// Get the original poseable mesh bounds
		ASLPoseableMeshActor* PoseableCloneAct = SkelActPosePair.Key;
		UPoseableMeshComponent* PoseableMeshComp = PoseableCloneAct->GetPoseableMeshComponent();
		TPair<FTransform, TMap<int32, FTransform>> CurrEpMemSkelPose = SkelActPosePair.Value;
			
		// Move clone to episodic memory location (easier to calculate the bound values)
		PoseableCloneAct->SetSkeletalPose(CurrEpMemSkelPose);
		PoseableMeshComp->UpdateBounds();
		FBoxSphereBounds PoseableMBounds = PoseableMeshComp->Bounds;

		// Set first value, or add the next ones
		if (SphereBounds.SphereRadius > 0.f)
		{
			SphereBounds = SphereBounds + PoseableMBounds;
		}
		else
		{
			// First value
			SphereBounds = PoseableMBounds;
		}
	}
	return SphereBounds.Origin;
}

// Move scene to root (scan) location (0,0,0)
void USLCVQScene::AddOffsetToScene(FVector Offset)
{
	// Move scene to root
	for (auto& ActPosePair : SceneActorPoses)
	{
		ActPosePair.Value.AddToTranslation(Offset);
	}

	for (auto& SkelActPosePair : ScenePoseableActorPoses)
	{
		// Reference to the skel actor and bone pose world pose
		TPair<FTransform, TMap<int32, FTransform>>& SkelPoseRef = SkelActPosePair.Value;

		// Moving the skel actor is probably not needed
		SkelPoseRef.Key.AddToTranslation(Offset);

		// Moving the bones is needed
		for (auto& BonePosePair : SkelPoseRef.Value)
		{
			BonePosePair.Value.AddToTranslation(Offset);
		}
	}
}

// Apply cached poses to the scene
void USLCVQScene::ApplyPoses()
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
		CurrSkelMA->SetActorHiddenInGame(false);

		// Set actor pose and bone pose
		CurrSkelMA->SetSkeletalPose(SkelActPosePair.Value);
	}
}

// 
FVector USLCVQScene::DummyCalcSceneOriginRed()
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
	for (const auto& SkelActPosePair : ScenePoseableActorPoses)
	{
		// Get the original poseable mesh bounds
		ASLPoseableMeshActor* PoseableCloneAct = SkelActPosePair.Key;
		UPoseableMeshComponent* PoseableMeshComp = PoseableCloneAct->GetPoseableMeshComponent();
		TPair<FTransform, TMap<int32, FTransform>> CurrEpMemSkelPose = SkelActPosePair.Value;

		// Move clone to episodic memory location (easier to calculate the bound values)
		PoseableCloneAct->SetSkeletalPose(CurrEpMemSkelPose);
		PoseableMeshComp->UpdateBounds();
		FBoxSphereBounds PoseableMBounds = PoseableMeshComp->Bounds;

#if SL_WITH_DEBUG && ENABLE_DRAW_DEBUG
		if (ActiveWorld && !ActiveWorld->IsPendingKillOrUnreachable())
		{
			// Clone
			for (int32 BIdx = 0; BIdx < PoseableMeshComp->GetNumBones(); BIdx++)
			{
				FVector CurrBoneLocation = PoseableMeshComp->GetBoneTransform(BIdx).GetLocation();
				DrawDebugPoint(ActiveWorld, CurrBoneLocation, 8.f, FColor::Red, true);
				DrawDebugLine(ActiveWorld, CurrBoneLocation, PoseableCloneAct->GetActorLocation(), FColor::Red, true);
			}
			DrawDebugSphere(ActiveWorld, PoseableMBounds.Origin, PoseableMBounds.SphereRadius, 16, FColor::Red, true);
			DrawDebugSphere(ActiveWorld, PoseableMBounds.Origin, 3.f, 6, FColor::Red, true);
		}
#endif // SL_WITH_DEBUG && ENABLE_DRAW_DEBUG

		// Set first value, or add the next ones
		if (SphereBounds.SphereRadius > 0.f)
		{
			SphereBounds = SphereBounds + PoseableMBounds;
		}
		else
		{
			// First value
			SphereBounds = PoseableMBounds;
		}
	}

#if SL_WITH_DEBUG && ENABLE_DRAW_DEBUG
	if (ActiveWorld && !ActiveWorld->IsPendingKillOrUnreachable())
	{
		DrawDebugSphere(ActiveWorld, SphereBounds.Origin, SphereBounds.SphereRadius, 32, FColor::Red, true);
		DrawDebugSphere(ActiveWorld, SphereBounds.Origin, 3.f, 16, FColor::Red, true);
	}
#endif // SL_WITH_DEBUG && ENABLE_DRAW_DEBUG

	return SphereBounds.Origin;
}

// 
FVector USLCVQScene::DummyCalcSceneOriginYellow()
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
	for (const auto& SkelActPosePair : ScenePoseableActorPoses)
	{
		// Get the original poseable mesh bounds
		ASLPoseableMeshActor* PoseableCloneAct = SkelActPosePair.Key;
		UPoseableMeshComponent* PoseableMeshComp = PoseableCloneAct->GetPoseableMeshComponent();
		TPair<FTransform, TMap<int32, FTransform>> CurrEpMemSkelPose = SkelActPosePair.Value;

		// Move clone to episodic memory location (easier to calculate the bound values)
		PoseableCloneAct->SetSkeletalPose(CurrEpMemSkelPose);
		PoseableMeshComp->UpdateBounds();
		FBoxSphereBounds PoseableMBounds = PoseableMeshComp->Bounds;

#if SL_WITH_DEBUG && ENABLE_DRAW_DEBUG
		if (ActiveWorld && !ActiveWorld->IsPendingKillOrUnreachable())
		{
			// Clone
			for (int32 BIdx = 0; BIdx < PoseableMeshComp->GetNumBones(); BIdx++)
			{
				FVector CurrBoneLocation = PoseableMeshComp->GetBoneTransform(BIdx).GetLocation();
				DrawDebugPoint(ActiveWorld, CurrBoneLocation, 8.f, FColor::Yellow, true);
				DrawDebugLine(ActiveWorld, CurrBoneLocation, PoseableCloneAct->GetActorLocation(), FColor::Yellow, true);
			}
			DrawDebugSphere(ActiveWorld, PoseableMBounds.Origin, PoseableMBounds.SphereRadius, 16, FColor::Yellow, true);
			DrawDebugSphere(ActiveWorld, PoseableMBounds.Origin, 3.f, 6, FColor::Yellow, true);
		}
#endif // SL_WITH_DEBUG && ENABLE_DRAW_DEBUG

		// Set first value, or add the next ones
		if (SphereBounds.SphereRadius > 0.f)
		{
			SphereBounds = SphereBounds + PoseableMBounds;
		}
		else
		{
			// First value
			SphereBounds = PoseableMBounds;
		}
	}

#if SL_WITH_DEBUG && ENABLE_DRAW_DEBUG
	if (ActiveWorld && !ActiveWorld->IsPendingKillOrUnreachable())
	{
		DrawDebugSphere(ActiveWorld, SphereBounds.Origin, SphereBounds.SphereRadius, 32, FColor::Yellow, true);
		DrawDebugSphere(ActiveWorld, SphereBounds.Origin, 3.f, 16, FColor::Yellow, true);
	}
#endif // SL_WITH_DEBUG && ENABLE_DRAW_DEBUG

	return SphereBounds.Origin;
}

// 
FVector USLCVQScene::DummyCalcSceneOriginGreen()
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
	for (const auto& SkelActPosePair : ScenePoseableActorPoses)
	{
		// Get the original poseable mesh bounds
		ASLPoseableMeshActor* PoseableCloneAct = SkelActPosePair.Key;
		UPoseableMeshComponent* PoseableMeshComp = PoseableCloneAct->GetPoseableMeshComponent();
		TPair<FTransform, TMap<int32, FTransform>> CurrEpMemSkelPose = SkelActPosePair.Value;

		// Move clone to episodic memory location (easier to calculate the bound values)
		PoseableCloneAct->SetSkeletalPose(CurrEpMemSkelPose);
		PoseableMeshComp->UpdateBounds();
		FBoxSphereBounds PoseableMBounds = PoseableMeshComp->Bounds;

#if SL_WITH_DEBUG && ENABLE_DRAW_DEBUG
		if (ActiveWorld && !ActiveWorld->IsPendingKillOrUnreachable())
		{
			// Clone
			for (int32 BIdx = 0; BIdx < PoseableMeshComp->GetNumBones(); BIdx++)
			{
				FVector CurrBoneLocation = PoseableMeshComp->GetBoneTransform(BIdx).GetLocation();
				DrawDebugPoint(ActiveWorld, CurrBoneLocation, 8.f, FColor::Green, true);
				DrawDebugLine(ActiveWorld, CurrBoneLocation, PoseableCloneAct->GetActorLocation(), FColor::Green, true);
			}
			DrawDebugSphere(ActiveWorld, PoseableMBounds.Origin, PoseableMBounds.SphereRadius, 16, FColor::Green, true);
			DrawDebugSphere(ActiveWorld, PoseableMBounds.Origin, 3.f, 6, FColor::Green, true);
		}
#endif // SL_WITH_DEBUG && ENABLE_DRAW_DEBUG

		// Set first value, or add the next ones
		if (SphereBounds.SphereRadius > 0.f)
		{
			SphereBounds = SphereBounds + PoseableMBounds;
		}
		else
		{
			// First value
			SphereBounds = PoseableMBounds;
		}
	}

#if SL_WITH_DEBUG && ENABLE_DRAW_DEBUG
	if (ActiveWorld && !ActiveWorld->IsPendingKillOrUnreachable())
	{
		DrawDebugSphere(ActiveWorld, SphereBounds.Origin, SphereBounds.SphereRadius, 32, FColor::Green, true);
		DrawDebugSphere(ActiveWorld, SphereBounds.Origin, 3.f, 16, FColor::Green, true);
	}
#endif // SL_WITH_DEBUG && ENABLE_DRAW_DEBUG

	return SphereBounds.Origin;
}
