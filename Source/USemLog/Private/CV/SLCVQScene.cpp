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

#if WITH_EDITOR
#include "Engine/Selection.h"
#include "Editor.h"
#include "Individuals/SLIndividualUtils.h"
#include "Individuals/Type/SLBaseIndividual.h"
#endif // WITH_EDITOR

// Set the scene actors and cache their relative transforms to the world root
bool USLCVQScene::InitScene(ASLIndividualManager* IndividualManager, ASLMongoQueryManager* MQManager)
{
	// Clear any previous data
	if (SceneActorPoses.Num() > 0 || StaticMaskClones.Num() > 0 || SkelMaskClones.Num() > 0)
	{
		SceneActorPoses.Empty();
		StaticMaskClones.Empty();
		SkelMaskClones.Empty();
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
		CurrSMA->SetActorTransform(CurrPose);
		CurrSMA->SetActorHiddenInGame(false);

		if (auto* CurrClone = StaticMaskClones.Find(CurrSMA))
		{
			(*CurrClone)->SetWorldTransform(CurrPose);
		}
	}

	// Move skeletal mesh actors (and mask clones) to the given poses
	for (const auto& SkelActPosePair : SceneSkelActorPoses)
	{
		ASkeletalMeshActor* CurrSkelMA = SkelActPosePair.Key;
		FTransform CurrSkelMAPose = SkelActPosePair.Value.Key;
		CurrSkelMA->SetActorTransform(CurrSkelMAPose);
		CurrSkelMA->SetActorHiddenInGame(false);

		// Apply orig clone bone poses
		TMap<int32, FTransform> CurrBonePoses = SkelActPosePair.Value.Value;
		if (auto* OrigClone = SkelOrigClones.Find(CurrSkelMA))
		{
			for (const auto& BonePosePair : CurrBonePoses)
			{
				const FName BoneName = (*OrigClone)->GetBoneName(BonePosePair.Key);
				(*OrigClone)->SetBoneTransformByName(BoneName, BonePosePair.Value, EBoneSpaces::WorldSpace);
			}
		}

		// Apply mask clone bone poses
		if (auto* MaskClone = SkelMaskClones.Find(CurrSkelMA))
		{
			for (const auto& BonePosePair : CurrBonePoses)
			{
				const FName BoneName = (*MaskClone)->GetBoneName(BonePosePair.Key);
				(*MaskClone)->SetBoneTransformByName(BoneName, BonePosePair.Value, EBoneSpaces::WorldSpace);
			}
		}
	}
}

// Hide scene
void USLCVQScene::HideScene()
{
	for (const auto& ActPosePair : SceneActorPoses)
	{
		ActPosePair.Key->SetActorHiddenInGame(true);
	}

	for (const auto& SkelActPosePair : SceneSkelActorPoses)
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

	// 
	for (const auto& ActPosePair : SceneActorPoses)
	{
		AActor* CurrActor = ActPosePair.Key;
		if (USLBaseIndividual* BI = FSLIndividualUtils::GetIndividualObject(CurrActor))
		{
			// Make sure individual is of type visible
			if (auto VI = Cast<USLVisibleIndividual>(BI))
			{
				// Make sure parent is a static or skeletal mesh actor
				if (auto AsSMA = Cast<AStaticMeshActor>(CurrActor))
				{
					// Check if the actor already has a clone
					for (const auto& Comp : AsSMA->GetComponentsByClass(UStaticMeshComponent::StaticClass()))
					{
						if (Comp->GetName().EndsWith("_CVQSceneMaskClone"))
						{
							// There is alrady a clone, add to array
							StaticMaskClones.Add(AsSMA, CastChecked<UStaticMeshComponent>(Comp));
							continue;
						}
					}

					// Duplicate/clone the static mesh component
					UStaticMeshComponent* OrigSMC = AsSMA->GetStaticMeshComponent();
					//UStaticMeshComponent* CloneSMC = NewObject<UStaticMeshComponent>(AsSMA);
					//CloneSMC->SetStaticMesh(OrigSMC->GetStaticMesh());
					UStaticMeshComponent* CloneSMC = DuplicateObject<UStaticMeshComponent>(OrigSMC, AsSMA,
						FName(*OrigSMC->GetName().Append("_CVQSceneMaskClone")));
					CloneSMC->SetCollisionEnabled(ECollisionEnabled::NoCollision);

					// Check if an individual mask needs to be created
					if (bUseIndividualMaskValue)
					{
						// Use the individual unique visual mask value for the mask
						DynamicMaskMaterial = UMaterialInstanceDynamic::Create(DefaultMaskMaterial, GetTransientPackage());
						DynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"), FColor::FromHex(VI->GetVisualMaskValue()));
					}

					// Apply the dynamic mask material to the mesh
					for (int32 MatIdx = 0; MatIdx < CloneSMC->GetNumMaterials(); ++MatIdx)
					{
						CloneSMC->SetMaterial(MatIdx, DynamicMaskMaterial);
					}

					// Register with actor
					AsSMA->AddOwnedComponent(CloneSMC);
					AsSMA->AddInstanceComponent(CloneSMC);
					CloneSMC->OnComponentCreated();
					CloneSMC->RegisterComponent();
					AsSMA->RerunConstructionScripts();

					// Attach to static mesh component (cannot be used since the world destroys attachments at one point)
					//CloneSMC->AttachToComponent(OrigSMC, FAttachmentTransformRules::SnapToTargetIncludingScale);
					//CloneSMC->SetRelativeLocationAndRotation(FVector::ZeroVector, FQuat::Identity);

					// Not visible by default
					CloneSMC->SetVisibility(false);

					// Add to map
					StaticMaskClones.Add(AsSMA, CloneSMC);
					
				}
				else if (auto AsSkelMA = Cast<ASkeletalMeshActor>(CurrActor))
				{
					// Check if the actor already has a clone
					for (const auto& Comp : AsSkelMA->GetComponentsByClass(UPoseableMeshComponent::StaticClass()))
					{
						if (Comp->GetName().EndsWith("_CVQSceneMaskClone"))
						{
							// There is alrady a clone, add to array
							SkelMaskClones.Add(AsSkelMA, CastChecked<UPoseableMeshComponent>(Comp));
							continue;
						}
					}

					// Create skeletal mesh clone
					FName NewComponentName = FName(*AsSkelMA->GetSkeletalMeshComponent()->GetName().Append("_CVQSceneMaskClone"));
					UPoseableMeshComponent* CloneSkelMC = NewObject<UPoseableMeshComponent>(AsSkelMA, NewComponentName);
					CloneSkelMC->SetSkeletalMesh(AsSkelMA->GetSkeletalMeshComponent()->SkeletalMesh);
					CloneSkelMC->SetCollisionEnabled(ECollisionEnabled::NoCollision);
					CloneSkelMC->bPerBoneMotionBlur = false;
					CloneSkelMC->bHasMotionBlurVelocityMeshes = false;

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
								CloneSkelMC->SetMaterial(BoneI->GetMaterialIndex(), DynamicMaskMaterial);
							}
						}
						else
						{
							UE_LOG(LogTemp, Error, TEXT("%s::%d skeletal actor %s does not have a skeletal individual representation.. "),
								*FString(__FUNCTION__), __LINE__, *AsSkelMA->GetName());
						}
					}
					else
					{
						for (int32 MatIdx = 0; MatIdx < CloneSkelMC->GetNumMaterials(); ++MatIdx)
						{
							CloneSkelMC->SetMaterial(MatIdx, DynamicMaskMaterial);
						}
					}

					// Register with actor
					AsSkelMA->AddOwnedComponent(CloneSkelMC);
					AsSkelMA->AddInstanceComponent(CloneSkelMC);
					CloneSkelMC->OnComponentCreated();
					CloneSkelMC->RegisterComponent();
					AsSkelMA->RerunConstructionScripts();

					// Not visible by default
					CloneSkelMC->SetVisibility(false);

					// Add to map
					SkelMaskClones.Add(AsSkelMA, CloneSkelMC);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("%s::%d Actor %s is not of type static or skeletal mesh, this should not happen.."),
						*FString(__FUNCTION__), __LINE__, *CurrActor->GetName());
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d %s does not have a visible individual.. this should not happen"),
					*FString(__FUNCTION__), __LINE__, *ActPosePair.Key->GetName());
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s does not have an individual.. this should not happen"),
				*FString(__FUNCTION__), __LINE__, *ActPosePair.Key->GetName());
		}
	}
	return StaticMaskClones.Num() > 0 || SkelMaskClones.Num() > 0;
}

// Show mask values of the scenes
void USLCVQScene::ShowMaskMaterials()
{
	for (const auto& Pair : StaticMaskClones)
	{
		Pair.Key->GetStaticMeshComponent()->SetVisibility(false);
		Pair.Value->SetVisibility(true);
	}

	for (const auto& Pair : SkelMaskClones)
	{
		Pair.Key->GetSkeletalMeshComponent()->SetVisibility(false);
		Pair.Value->SetVisibility(true);
	}
}

// Show original material
void USLCVQScene::ShowOriginalMaterials()
{
	for (const auto& Pair : StaticMaskClones)
	{
		Pair.Key->GetStaticMeshComponent()->SetVisibility(true);
		Pair.Value->SetVisibility(false);
	}

	for (const auto& Pair : SkelMaskClones)
	{
		Pair.Key->GetSkeletalMeshComponent()->SetVisibility(true);
		Pair.Value->SetVisibility(false);
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
	for (const auto& SkelMAPosePair : SceneSkelActorPoses)
	{
		// Get the original poseable mesh bounds
		if (auto* OrigClone = SkelOrigClones.Find(SkelMAPosePair.Key))
		{
			FBoxSphereBounds SMBounds = (*OrigClone)->Bounds;

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

	// Iterate the scene actors, cache their original world position,
	for (const auto& Id : Ids)
	{
		if (auto CurrActor = IndividualManager->GetIndividualActor(Id))
		{
			if (auto* AsSMA = Cast<AStaticMeshActor>(CurrActor))
			{
				// Cache the original world pose
				FTransform WorldPose = MQManager->GetIndividualPoseAt(Id, Timestamp);
				SceneActorPoses.Add(AsSMA, WorldPose);
			}
			else if (auto* AsSkelMA = Cast<ASkeletalMeshActor>(CurrActor))
			{
				TPair<FTransform, TMap<int32, FTransform>> SkelWorldPose = MQManager->GetSkeletalIndividualPoseAt(Id, Timestamp);
				SceneSkelActorPoses.Add(AsSkelMA, SkelWorldPose);
				AsSkelMA->GetSkeletalMeshComponent()->SetVisibility(false);


				/* Create a poseable mesh clone with the original materials */
				// Check if the actor already has a clone
				for (const auto& Comp : AsSkelMA->GetComponentsByClass(UPoseableMeshComponent::StaticClass()))
				{
					if (Comp->GetName().EndsWith("_CVQSceneOrigClone"))
					{
						// There is alrady a clone, add to array
						SkelOrigClones.Add(AsSkelMA, CastChecked<UPoseableMeshComponent>(Comp));
						continue;
					}
				}

				// Create a new skeletal mesh clone
				FName NewComponentName = FName(*AsSkelMA->GetSkeletalMeshComponent()->GetName().Append("_CVQSceneOrigClone"));
				UPoseableMeshComponent* CloneSkelMC = NewObject<UPoseableMeshComponent>(AsSkelMA, NewComponentName);
				CloneSkelMC->SetSkeletalMesh(AsSkelMA->GetSkeletalMeshComponent()->SkeletalMesh);
				CloneSkelMC->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				CloneSkelMC->bPerBoneMotionBlur = false;
				CloneSkelMC->bHasMotionBlurVelocityMeshes = false;

				// Register with actor
				AsSkelMA->AddOwnedComponent(CloneSkelMC);
				AsSkelMA->AddInstanceComponent(CloneSkelMC);
				CloneSkelMC->OnComponentCreated();
				CloneSkelMC->RegisterComponent();
				AsSkelMA->RerunConstructionScripts();

				// Add to map
				SkelOrigClones.Add(AsSkelMA, CloneSkelMC);
			}
		}
	}

	// Calculate the centroid/barycenter of the scene
	FVector SceneCentroidLocation;
	for (const auto& ActPosePair : SceneActorPoses)
	{
		FTransform WorldPose = ActPosePair.Value;
		SceneCentroidLocation += WorldPose.GetLocation();
	}
	int32 TotalNumBonePoses = 0;
	for (const auto& SkelActPosePair : SceneSkelActorPoses)
	{
		// Use only bone locations for the centroid calculation
		//FTransform WorldPose = SkelActPosePair.Value.Key;
		TMap<int32, FTransform> BonePoses = SkelActPosePair.Value.Value;
		TotalNumBonePoses += BonePoses.Num();
		for (auto BonePosePair : BonePoses)
		{
			SceneCentroidLocation += BonePosePair.Value.GetLocation();
		}
	}
	SceneCentroidLocation /= (SceneActorPoses.Num() + TotalNumBonePoses);

	// TODO needs testing
	//// Calculate centroid location
	//FVector SceneCentroidLocation;
	//FBoxSphereBounds SphereBounds(EForceInit::ForceInit);
	//// Add up static mesh bounds
	//for (const auto& SMAPosePair : SceneActorPoses)
	//{
	//	// Get the mesh bounds
	//	FBoxSphereBounds SMBounds = SMAPosePair.Key->GetStaticMeshComponent()->Bounds;

	//	// Set first value, or add the next ones
	//	if (SphereBounds.SphereRadius > 0.f)
	//	{
	//		SphereBounds = SphereBounds + SMBounds;
	//	}
	//	else
	//	{
	//		// First value
	//		SphereBounds = SMBounds;
	//	}
	//}
	//// Add up skeletal mesh bounds
	//for (const auto& SkelMAPosePair : SceneSkelActorPoses)
	//{
	//	// Get the original poseable mesh bounds
	//	if (auto* OrigClone = SkelOrigClones.Find(SkelMAPosePair.Key))
	//	{
	//		FBoxSphereBounds SMBounds = (*OrigClone)->Bounds;

	//		// Set first value, or add the next ones
	//		if (SphereBounds.SphereRadius > 0.f)
	//		{
	//			SphereBounds = SphereBounds + SMBounds;
	//		}
	//		else
	//		{
	//			// First value
	//			SphereBounds = SMBounds;
	//		}
	//	}
	//}
	//SceneCentroidLocation = SphereBounds.Origin;

	// Move scene to root
	for (auto& ActPosePair : SceneActorPoses)
	{
		ActPosePair.Value.AddToTranslation(-SceneCentroidLocation);
	}
	for (auto& SkelActPosePair : SceneSkelActorPoses)
	{
		// Moving the skel actor is probably not needed
		SkelActPosePair.Value.Key.AddToTranslation(-SceneCentroidLocation);
		
		// Moving the bones is needed
		for (auto& BonePosePair : SkelActPosePair.Value.Value)
		{
			BonePosePair.Value.AddToTranslation(-SceneCentroidLocation);
		}
	}

	return SceneActorPoses.Num() > 0 || SceneSkelActorPoses.Num() > 0;
}

