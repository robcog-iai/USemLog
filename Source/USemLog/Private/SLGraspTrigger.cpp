// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLGraspTrigger.h"
#include "SLMap.h"
//#include "SLOverlapArea.h"

// UUTils
#include "Tags.h"
#include "Ids.h"

// Default constructor
USLGraspTrigger::USLGraspTrigger()
{
}

// Setup pointers to outer, check if semantically annotated
bool USLGraspTrigger::Init(USkeletalMeshComponent* InHand)
{
	OwnerId = InHand->GetUniqueID();
	OwnerSemId = FSLMap::GetInstance()->GetSemanticId(OwnerId);
	OwnerSemClass = FSLMap::GetInstance()->GetSemanticClass(OwnerId);
	if (OwnerSemId.IsEmpty() || OwnerSemClass.IsEmpty())
	{
		SLGraspPub = MakeShareable(new FSLGraspPublisher(this));
		SLGraspPub->Init();
		return true;
	}
	return false;
}

// Finish trigger
void USLGraspTrigger::Finish(float Time)
{
	//	// Terminate and publish pending events
	if (SLGraspPub.IsValid())
	{
		SLGraspPub->Finish(Time);
	}
}

// Start grasp
void USLGraspTrigger::BeginGrasp(AStaticMeshActor* Other, float Time)
{
	// Check if other actor is semantically annotated
	const uint32 OtherId = Other->GetUniqueID();
	const FString OtherSemId = FSLMap::GetInstance()->GetSemanticId(OtherId);
	const FString OtherSemClass = FSLMap::GetInstance()->GetSemanticClass(OtherId);
	if (OtherSemId.IsEmpty() || OtherSemClass.IsEmpty())
	{
		// Broadcast the result
		FSLGraspResult GraspBeginResult(OtherId, OtherSemId, OtherSemClass, Time);
		UE_LOG(LogTemp, Warning, TEXT(">> %s::%d \t\t BEGIN SL GRASP"), TEXT(__FUNCTION__), __LINE__);
		OnBeginSLGrasp.Broadcast(GraspBeginResult);
	}
}

// End grasp
void USLGraspTrigger::EndGrasp(AStaticMeshActor* Other, float Time)
{
	UE_LOG(LogTemp, Warning, TEXT(">> %s::%d \t\t END SL GRASP"), TEXT(__FUNCTION__), __LINE__);
	OnEndSLGrasp.Broadcast(Other->GetUniqueID(), Time);
}
