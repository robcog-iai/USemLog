// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "VizQ/SLVizQMarkerArray.h"
#include "VizQ/SLVizQMarker.h"
#include "Knowrob/SLKnowrobManager.h"
#include "Mongo/SLMongoQueryManager.h"
#include "Viz/SLVizManager.h"

#if WITH_EDITOR
#include "Engine/Selection.h"
#include "Editor.h"
#include "Individuals/SLIndividualUtils.h"
#include "Individuals/Type/SLBaseIndividual.h"
#endif // WITH_EDITOR

#if WITH_EDITOR
// Called when a property is changed in the editor
void USLVizQMarkerArray::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property name
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ?
		PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(USLVizQMarkerArray, bRemoveButton))
	{
		bRemoveButton = false;
		if (IsReadyForManualExecution())
		{
			for (const auto& MarkerId : MarkerIds)
			{
				KnowrobManager->GetVizManager()->RemoveMarker(MarkerId);
			}
		}
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLVizQMarkerArray, bRemoveAllButton))
	{
		bRemoveAllButton = false;
		if (IsReadyForManualExecution())
		{
			KnowrobManager->GetVizManager()->RemoveAllMarkers();
		}
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLVizQMarkerArray, bCalcTimelineDurationButton))
	{
		bCalcTimelineDurationButton = false;
		TimelineParams.Duration = EndTime - StartTime;
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLVizQMarkerArray, bSyncWithChildrenButton))
	{
		bSyncWithChildrenButton = false;
		for (const auto& C : Children)
		{
			if (auto MAC = Cast<USLVizQMarkerArray>(C))
			{
				MAC->Task = Task;
				MAC->Episode = Episode;
				MAC->StartTime = StartTime;
				MAC->EndTime = EndTime;
				MAC->DeltaT = DeltaT;
				MAC->TimelineParams = TimelineParams;
			}
			else if (auto MC = Cast<USLVizQMarker>(C))
			{
				MC->Task = Task;
				MC->Episode = Episode;
				MC->StartTime = StartTime;
				MC->EndTime = EndTime;
				MC->DeltaT = DeltaT;
				MC->TimelineParams = TimelineParams;
			}
		}
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLVizQMarkerArray, bAddSelectedButton))
	{
		bAddSelectedButton = false;
		if (bOverwrite)
		{
			Individuals.Empty();
		}
		for (FSelectionIterator It(GEditor->GetSelectedActorIterator()); It; ++It)
		{
			AActor* SelectedActor = CastChecked<AActor>(*It);
			if (USLBaseIndividual* BI = FSLIndividualUtils::GetIndividualObject(SelectedActor))
			{
				if (BI->IsLoaded())
				{
					bEnsureUniqueness ? Individuals.AddUnique(BI->GetIdValue()) : Individuals.Add(BI->GetIdValue());
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
		// Resync
		MarkerIds.Empty();
		for (const auto Individual : Individuals)
		{
			MarkerIds.Add(MarkerIdPrefix + Individual);
		}		
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLVizQMarkerArray, bRemoveSelectedButton))
	{
		bRemoveSelectedButton = false;
		for (FSelectionIterator It(GEditor->GetSelectedActorIterator()); It; ++It)
		{
			AActor* SelectedActor = CastChecked<AActor>(*It);
			if (USLBaseIndividual* BI = FSLIndividualUtils::GetIndividualObject(SelectedActor))
			{
				if (BI->IsLoaded())
				{
					Individuals.Remove(BI->GetIdValue());
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
		// Resync
		MarkerIds.Empty();
		for (const auto Individual : Individuals)
		{
			MarkerIds.Add(MarkerIdPrefix + Individual);
		}
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLVizQMarkerArray, bSyncMarkerAndIndividualsButton))
	{
		bSyncMarkerAndIndividualsButton = false;
		MarkerIds.Empty();
		for (const auto Individual : Individuals)
		{	
			MarkerIds.Add(MarkerIdPrefix + Individual);
		}
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLVizQMarkerArray, MarkerIdPrefix))
	{
		MarkerIds.Empty();
		for (const auto Individual : Individuals)
		{
			MarkerIds.Add(MarkerIdPrefix + Individual);
		}
	}
}
#endif // WITH_EDITOR

// Virtual implementation of the execute function
void USLVizQMarkerArray::ExecuteImpl(ASLKnowrobManager* KRManager)
{
	ASLVizManager* VizManager = KRManager->GetVizManager();
	ASLMongoQueryManager* MongoQueryManager = KRManager->GetMongoQueryManager();

	if (MarkerIds.Num() != Individuals.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d MarkerIds.Num() != Individuals.Num().."), *FString(__FUNCTION__), __LINE__);
		return;
	}

	int32 ViewIdx = 0;
	for (const auto& MarkerId : MarkerIds)
	{
		const FString Individual = Individuals[ViewIdx];
		ViewIdx++;

		/* Skeletal */
		if (MeshType == ESLVizQMarkerArrayMeshType::SkeletalMesh)
		{
			// Pose/trajectory data
			TArray<TPair<FTransform, TMap<int32, FTransform>>> SkeletalPoses;

			// Read data as pose or trajectory
			if (Type == ESLVizQMarkerArrayType::Pose)
			{
				SkeletalPoses.Add(MongoQueryManager->GetSkeletalIndividualPoseAt(Task, Episode, Individual,
					StartTime));
			}
			else if (EndTime > 0 && EndTime > StartTime)
			{
				SkeletalPoses = MongoQueryManager->GetSkeletalIndividualTrajectory(Task, Episode, Individual,
					StartTime, EndTime, DeltaT);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d EndTime is not valid.."), *FString(__FUNCTION__), __LINE__);
				return;
			}

			if (SkeletalPoses.Num() == 0)
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d query resulted in 0 poses.. make sure %s is skeletal.."), *FString(__FUNCTION__), __LINE__);
				return;
			}

			// Draw marker as static or timeline
			if (Type != ESLVizQMarkerArrayType::Timeline)
			{
				if (bUseOriginalColor)
				{
					VizManager->CreateSkeletalMeshMarker(MarkerId, SkeletalPoses, Individual);
				}
				else
				{
					VizManager->CreateSkeletalMeshMarker(MarkerId, SkeletalPoses, Individual,
						Color, MaterialType);
				}
			}
			else
			{
				if (bUseOriginalColor)
				{
					VizManager->CreateSkeletalMeshMarkerTimeline(MarkerId, SkeletalPoses, Individual,
						TimelineParams);
				}
				else
				{
					VizManager->CreateSkeletalMeshMarkerTimeline(MarkerId, SkeletalPoses, Individual,
						Color, MaterialType,
						TimelineParams);
				}
			}
		}

		/* Static mesh */
		else
		{
			// Pose/trajectory data
			TArray<FTransform> Poses;

			// Read data as pose or trajectory
			if (Type == ESLVizQMarkerArrayType::Pose)
			{
				Poses.Add(MongoQueryManager->GetIndividualPoseAt(Task, Episode, Individual,
					StartTime));
			}
			else if (EndTime > 0 && EndTime > StartTime)
			{
				Poses = MongoQueryManager->GetIndividualTrajectory(Task, Episode, Individual,
					StartTime, EndTime, DeltaT);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d EndTime is not valid.."), *FString(__FUNCTION__), __LINE__);
				return;
			}

			if (Poses.Num() == 0)
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d query resulted in 0 poses.."), *FString(__FUNCTION__), __LINE__);
				return;
			}

			// Draw marker as static or timeline
			if (MeshType == ESLVizQMarkerArrayMeshType::Primitive)
			{
				if (Type != ESLVizQMarkerArrayType::Timeline)
				{
					VizManager->CreatePrimitiveMarker(MarkerId, Poses, PrimitiveType, Size,
						Color, MaterialType);
				}
				else
				{
					VizManager->CreatePrimitiveMarkerTimeline(MarkerId, Poses, PrimitiveType,
						Size, Color, MaterialType,
						TimelineParams);
				}
			}
			else if (MeshType == ESLVizQMarkerArrayMeshType::StaticMesh)
			{
				if (Type != ESLVizQMarkerArrayType::Timeline)
				{
					if (bUseOriginalColor)
					{
						VizManager->CreateStaticMeshMarker(MarkerId, Poses, Individual);
					}
					else
					{
						VizManager->CreateStaticMeshMarker(MarkerId, Poses, Individual,
							Color, MaterialType);
					}
				}
				else
				{
					if (bUseOriginalColor)
					{
						VizManager->CreateStaticMeshMarkerTimeline(MarkerId, Poses, Individual,
							TimelineParams);
					}
					else
					{
						VizManager->CreateStaticMeshMarkerTimeline(MarkerId, Poses, Individual,
							Color, MaterialType,
							TimelineParams);
					}
				}
			}
		}
	}
}
