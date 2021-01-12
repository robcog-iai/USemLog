// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "CV/SLCVQScene.h"
#include "Individuals/SLIndividualManager.h"
#include "Individuals/Type/SLVisibleIndividual.h"
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

	return InitSceneImpl(IndividualManager, MQManager);
}

// Public execute function
void USLCVQScene::ShowScene()
{
	for (const auto& ActPosePair : SceneActorPoses)
	{
		AActor* Actor = ActPosePair.Key;
		FTransform Pose = ActPosePair.Value;
		Actor->SetActorTransform(Pose);
		Actor->SetActorHiddenInGame(false);
	}
}

// Generate mask clones for the scene
void USLCVQScene::GenerateMaskClones(const TCHAR* MaterialPath, bool bUseInternalMask, FColor MaskColor)
{
	// Get the dynamic mask material
	UMaterial* DefaultMaskMaterial = LoadObject<UMaterial>(this, MaterialPath);
	if (!DefaultMaskMaterial)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s could not load default mask material.."),
			*FString(__func__), __LINE__, *GetName());
		return;
	}
	DefaultMaskMaterial->bUsedWithStaticLighting = true;
	DefaultMaskMaterial->bUsedWithSkeletalMesh = true;

	// Create a common dynamic mask material and set its color
	UMaterialInstanceDynamic* DynamicMaskMaterial = nullptr;
	if (!bUseInternalMask)
	{
		DynamicMaskMaterial = UMaterialInstanceDynamic::Create(DefaultMaskMaterial, GetTransientPackage());
		DynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"), MaskColor);
	}

	// 
	for (const auto& ActPosePair : SceneActorPoses)
	{
		if (USLBaseIndividual* BI = FSLIndividualUtils::GetIndividualObject(ActPosePair.Key))
		{
			if (auto VI = Cast<USLVisibleIndividual>(BI))
			{
				// Use the individual unique visual mask value for the mask
				if (bUseInternalMask)
				{
					DynamicMaskMaterial = UMaterialInstanceDynamic::Create(DefaultMaskMaterial, GetTransientPackage());
					DynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"), FColor::FromHex(VI->GetVisualMaskValue()));
				}

				//		// Make sure parent is a static mesh actor
				//		if (auto AsSMA = Cast<AStaticMeshActor>(VI->GetParentActor()))
				//		{
				//			FActorSpawnParameters Parameters;
				//			Parameters.Template = AsSMA;
				//			Parameters.Template->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
				//			//Parameters.Instigator = SMA->GetInstigator();
				//			Parameters.Name = FName(*(AsSMA->GetName() + TEXT("_MaskClone")));
				//			AStaticMeshActor* SMAClone = GetWorld()->SpawnActor<AStaticMeshActor>(AsSMA->GetClass(), Parameters);
				//			if (UStaticMeshComponent* SMC = SMAClone->GetStaticMeshComponent())
				//			{
				//				for (int32 MatIdx = 0; MatIdx < SMC->GetNumMaterials(); ++MatIdx)
				//				{
				//#if WITH_EDITOR	
				//					SMC->SetMaterial(MatIdx, DynamicMaskMaterial);
				//#endif
				//				}
				//			}
				//			SMAClone->SetActorHiddenInGame(true);
				//			IndividualsMaskClones.Add(VI, SMAClone);
				//		}
				//		else if (auto AsSkelMA = Cast<ASkeletalMeshActor>(VI->GetParentActor()))
				//		{
				//			//todo
				//		}

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
}

// Show mask values of the scenes
void USLCVQScene::ShowMaskMaterials()
{
	for (const auto& Pair : MaskClones)
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
	for (const auto& Pair : MaskClones)
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

// Hide scene
void USLCVQScene::HideScene()
{
	for (const auto& ActPosePair : SceneActorPoses)
	{
		AActor* Actor = ActPosePair.Key;
		Actor->SetActorHiddenInGame(true);
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

// Get the bounding sphere radius of the scene
float USLCVQScene::GetSphereBoundsRadius() const
{
	FBoxSphereBounds SphereBounds;
	for (const auto& ActPosePair : SceneActorPoses)
	{
		AActor* Actor = ActPosePair.Key;
		if (auto AsSMA = Cast<AStaticMeshActor>(Actor))
		{
			SphereBounds = SphereBounds + AsSMA->GetStaticMeshComponent()->Bounds;
		}
		else if (auto AsSkelMA = Cast<ASkeletalMeshActor>(Actor))
		{
			SphereBounds = SphereBounds + AsSkelMA->GetSkeletalMeshComponent()->Bounds;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is an unsupported actor type.."),
				*FString(__FUNCTION__), __LINE__, *Actor->GetName());
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

// Virtual implementation for the scene initialization
bool USLCVQScene::InitSceneImpl(ASLIndividualManager* IndividualManager, ASLMongoQueryManager* MQManager)
{
	// Iterate the scene actors, set their original world position
	for (const auto& Id : Ids)
	{
		if (auto Act = IndividualManager->GetIndividualActor(Id))
		{
			FTransform WorldPose = MQManager->GetIndividualPoseAt(Id, Timestamp);
			SceneActorPoses.Add(Act, WorldPose);
		}
	}

	// Calculate the centroid/barycenter of the scene
	FVector SceneCentroidLocation;
	for (const auto& ActPosePair : SceneActorPoses)
	{
		FTransform WorldPose = ActPosePair.Value;
		SceneCentroidLocation += WorldPose.GetLocation();
	}
	SceneCentroidLocation /= SceneActorPoses.Num();

	// Move scene to root
	for (auto& ActPosePair : SceneActorPoses)
	{
		ActPosePair.Value.AddToTranslation(-SceneCentroidLocation);
	}

	return SceneActorPoses.Num() > 0;
}
#endif // WITH_EDITOR

