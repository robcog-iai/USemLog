// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "VizQ/SLVizQMarker.h"
#include "VizQ/SLVizQMarkerArray.h"
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
void USLVizQMarker::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property name
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ?
		PropertyChangedEvent.Property->GetFName() : NAME_None;

	/* Manual interaction */
	if (PropertyName == GET_MEMBER_NAME_CHECKED(USLVizQMarker, bRemoveButton))
	{
		bRemoveButton = false;
		if (IsReadyForManualExecution())
		{
			KnowrobManager->GetVizManager()->RemoveMarker(MarkerId);
		}
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLVizQMarker, bRemoveAllButton))
	{
		bRemoveAllButton = false;
		if (IsReadyForManualExecution())
		{
			KnowrobManager->GetVizManager()->RemoveAllMarkers();
		}
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLVizQMarker, bCalcTimelineDurationButton))
	{
		bCalcTimelineDurationButton = false;
		TimelineParams.Duration = EndTime - StartTime;
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(USLVizQMarker, bSyncWithChildrenButton))
	{
		bSyncWithChildrenButton = false;
		for (const auto& C : Children)
		{
			if (auto MC = Cast<USLVizQMarker>(C))
			{
				MC->Task = Task;
				MC->Episode = Episode;
				MC->StartTime = StartTime;
				MC->EndTime = EndTime;
				MC->DeltaT = DeltaT;
				MC->TimelineParams = TimelineParams;
				MC->MarkerIdPrefix = MarkerIdPrefix;
			}
			else if (auto MAC = Cast<USLVizQMarkerArray>(C))
			{
				MAC->Task = Task;
				MAC->Episode = Episode;
				MAC->StartTime = StartTime;
				MAC->EndTime = EndTime;
				MAC->DeltaT = DeltaT;
				MAC->TimelineParams = TimelineParams;
				MAC->MarkerIdPrefix = MarkerIdPrefix;
			}
		}
	}
	/* Editor interaction */
	if (PropertyName == GET_MEMBER_NAME_CHECKED(USLVizQMarker, bAddSelectedButton))
	{
		bAddSelectedButton = false;
		for (FSelectionIterator It(GEditor->GetSelectedActorIterator()); It; ++It)
		{
			AActor* SelectedActor = CastChecked<AActor>(*It);
			if (USLBaseIndividual* BI = FSLIndividualUtils::GetIndividualObject(SelectedActor))
			{
				if (BI->IsLoaded())
				{
					Individual = BI->GetIdValue();
					return;
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
		// Sync
		MarkerId = MarkerIdPrefix + Individual;
	}
	if (PropertyName == GET_MEMBER_NAME_CHECKED(USLVizQMarker, MarkerIdPrefix))
	{
		MarkerId = MarkerIdPrefix + Individual;
	}
}
#endif // WITH_EDITOR

// Virtual implementation of the execute function
void USLVizQMarker::ExecuteImpl(ASLKnowrobManager* KRManager)
{
	ASLVizManager* VizManager = KRManager->GetVizManager();
	ASLMongoQueryManager* MongoQueryManager = KRManager->GetMongoQueryManager();

	/* Skeletal */
	if (MeshType == ESLVizQMarkerMeshType::SkeletalMesh)
	{
		// Pose/trajectory data
		TArray<TPair<FTransform, TMap<int32, FTransform>>> SkeletalPoses;

		// Read data as pose or trajectory
		if (Type == ESLVizQMarkerType::Pose)
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
		if (Type != ESLVizQMarkerType::Timeline)
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
		if (Type == ESLVizQMarkerType::Pose)
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
		if (MeshType == ESLVizQMarkerMeshType::Primitive)
		{
			if (Type != ESLVizQMarkerType::Timeline)
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
		else if (MeshType == ESLVizQMarkerMeshType::StaticMesh)
		{
			if (Type != ESLVizQMarkerType::Timeline)
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
