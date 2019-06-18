// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLContactShapeInterface.h"


// Called on overlap begin events
void ISLContactShapeInterface::OnOverlapBegin2(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	//// Ignore self overlaps (area with static mesh)
	//if (OtherActor == GetOwner())
	//{
	//	return;
	//}

	//// Check if the component or its outer is semantically annotated
	//FSLEntity OtherItem = FSLEntitiesManager::GetInstance()->GetEntity(OtherComp);
	//if (!OtherItem.IsSet())
	//{
	//	// Other not valid, check if its outer is semantically annotated
	//	OtherItem = FSLEntitiesManager::GetInstance()->GetEntity(OtherComp->GetOuter());
	//	if (!OtherItem.IsSet())
	//	{
	//		return;
	//	}
	//}

	//// Get the time of the event in second
	//float StartTime = GetWorld()->GetTimeSeconds();

	//// Check the type of the other component
	//if (UMeshComponent* OtherAsMeshComp = Cast<UMeshComponent>(OtherComp))
	//{
	//	// Broadcast begin of semantic overlap event
	//	FSLContactResult SemanticOverlapResult(SemanticOwner, OtherItem,
	//		StartTime, false, OwnerMeshComp, OtherAsMeshComp);
	//	OnBeginSLContact.Broadcast(SemanticOverlapResult);
	//}
	//else if (USLContactCapsule* OtherContactTrigger = Cast<USLContactCapsule>(OtherComp))
	//{
	//	// If both areas are trigger areas, they will both concurrently trigger overlap events.
	//	// To avoid this we consistently ignore one trigger event. This is chosen using
	//	// the unique ids of the overlapping actors (GetUniqueID), we compare the two values 
	//	// and consistently pick the event with a given (larger or smaller) value.
	//	// This allows us to be in sync with the overlap end event 
	//	// since the unique ids and the rule of ignoring the one event will not change
	//	// Filter out one of the trigger areas (compare unique ids)
	//	if (OtherItem.Obj->GetUniqueID() > SemanticOwner.Obj->GetUniqueID())
	//	{
	//		// Broadcast begin of semantic overlap event
	//		FSLContactResult SemanticOverlapResult(SemanticOwner, OtherItem,
	//			StartTime, true, OwnerMeshComp, OtherContactTrigger->OwnerMeshComp);
	//		OnBeginSLContact.Broadcast(SemanticOverlapResult);
	//	}
	//}
}

// Called on overlap end events
void ISLContactShapeInterface::OnOverlapEnd2(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	//// Ignore self overlaps (area with static mesh)
	//if (OtherActor == GetOwner())
	//{
	//	return;
	//}

	//// Check if the component or its outer is semantically annotated
	//FSLEntity OtherItem = FSLEntitiesManager::GetInstance()->GetEntity(OtherComp);
	//if (!OtherItem.IsSet())
	//{
	//	// Other not valid, check if its outer is semantically annotated
	//	OtherItem = FSLEntitiesManager::GetInstance()->GetEntity(OtherComp->GetOuter());
	//	if (!OtherItem.IsSet())
	//	{
	//		return;
	//	}
	//}

	//// Get the time of the event in second
	//float EndTime = GetWorld()->GetTimeSeconds();

	//// Check the type of the other component
	//if (UMeshComponent* OtherAsMeshComp = Cast<UMeshComponent>(OtherComp))
	//{
	//	// Broadcast end of semantic overlap event
	//	OnEndSLContact.Broadcast(SemanticOwner.Obj, OtherItem.Obj, EndTime);
	//}
	//else if (USLContactCapsule* OtherContactTrigger = Cast<USLContactCapsule>(OtherComp))
	//{
	//	// If both areas are trigger areas, they will both concurrently trigger overlap events.
	//	// To avoid this we consistently ignore one trigger event. This is chosen using
	//	// the unique ids of the overlapping actors (GetUniqueID), we compare the two values 
	//	// and consistently pick the event with a given (larger or smaller) value.
	//	// This allows us to be in sync with the overlap end event 
	//	// since the unique ids and the rule of ignoring the one event will not change
	//	// Filter out one of the trigger areas (compare unique ids)
	//	if (OtherItem.Obj->GetUniqueID() > SemanticOwner.Obj->GetUniqueID())
	//	{
	//		// Broadcast end of semantic overlap event
	//		OnEndSLContact.Broadcast(SemanticOwner.Obj, OtherItem.Obj, EndTime);
	//	}
	//}
}