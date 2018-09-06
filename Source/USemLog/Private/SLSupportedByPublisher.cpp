// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLSupportedByPublisher.h"
#include "SLOverlapArea.h"

// UUtils
#include "Ids.h"

#define SL_SB_VERT_SPEED_TH 0.5f // Supported by event vertical speed threshold
#define SL_SB_UPDATE_RATE_CB 0.3f // Update rate for the timer callback

// Default constructor
FSLSupportedByPublisher::FSLSupportedByPublisher(USLOverlapArea* InSLOverlapArea)
{
	Parent = InSLOverlapArea;
}

// Init
void FSLSupportedByPublisher::Init()
{
	Parent->OnBeginSLOverlap.AddRaw(this, &FSLSupportedByPublisher::OnSLOverlapBegin);
	Parent->OnEndSLOverlap.AddRaw(this, &FSLSupportedByPublisher::OnSLOverlapEnd);

	// Start timer for checking the supported by event candidates
	//TimerDelegateNextTick.BindUObject(this, &USLOverlapArea::NextTickCb);
	//GetWorld()->GetTimerManager().SetTimerForNextTick(TimerDelegateNextTick);
	//FTimerDelegate TD;
	//TD.BindLambda([this] {UE_LOG(LogTemp, Warning, TEXT(">> %s::%d LAMBDA TS:%f"), TEXT(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds()); });
	//GetWorld()->GetTimerManager().SetTimerForNextTick(TD);

	// Start timer (will be directly paused if no candidates are available)
	TimerDelegate.BindRaw(this, &FSLSupportedByPublisher::CheckCandidatesTimerCb);
	Parent->GetWorld()->GetTimerManager().SetTimer(TimerHandle,
		TimerDelegate, SL_SB_UPDATE_RATE_CB, true);
}


// Terminate listener, finish and publish remaining events
void FSLSupportedByPublisher::Finish(float EndTime)
{
	FSLSupportedByPublisher::FinishAndPublishStartedEvents(EndTime);
	Parent->GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
}


// Add supported by candidate (other)
void FSLSupportedByPublisher::AddCandidate(UStaticMeshComponent* OtherStaticMeshComp,
	uint32 OtherId,
	const FString& OtherSemId,
	const FString& OtherSemClass,
	bool bCanOnlyBeSupporting)
{
	// Add the other as a potential supported by event candidate
	FSLSupportedByCandidateData SBCandidate;
	SBCandidate.Id = OtherId;
	SBCandidate.SemId = OtherSemId;
	SBCandidate.SemClass = OtherSemClass;
	SBCandidate.StaticMeshComp = OtherStaticMeshComp;
	SBCandidate.bCanOnlyBeSupporting = bCanOnlyBeSupporting;

	// Add the created candidate
	Candidates.Emplace(SBCandidate);
}

// Check if other obj is a supported by candidate
bool FSLSupportedByPublisher::IsACandidate(const uint32 InOtherId, bool bRemoveIfFound)
{
	// Use iterator to be able to remove the entry from the array
	for (auto CandidateItr(Candidates.CreateIterator()); CandidateItr; ++CandidateItr)
	{
		if ((*CandidateItr).Id == InOtherId)
		{
			if (bRemoveIfFound)
			{
				// Remove candidate from the list
				CandidateItr.RemoveCurrent();
			}
			return true; // Found
		}
	}
	return false; // Not in list
}

// Check candidates vertical relative speed
void FSLSupportedByPublisher::CheckCandidatesTimerCb()
{
	float StartTime = Parent->GetWorld()->GetTimeSeconds();

	UE_LOG(LogTemp, Warning, TEXT(">> %s::%d CANDIDATES: "), TEXT(__FUNCTION__), __LINE__);

	for (auto CandidateItr(Candidates.CreateIterator()); CandidateItr; ++CandidateItr)
	{
		// Get relative vertical speed
		const float RelVerticalSpeed = FMath::Abs(Parent->OwnerStaticMeshComp->GetComponentVelocity().Z -
			CandidateItr->StaticMeshComp->GetComponentVelocity().Z);

		UE_LOG(LogTemp, Warning, TEXT("\t \t %s, Vel[Outer,Other]=[%s,%s], RelVerticalSpeed=%f"),
			*(*CandidateItr).SemId,
			*Parent->OwnerStaticMeshComp->GetComponentVelocity().ToString(),
			*(*CandidateItr).StaticMeshComp->GetComponentVelocity().ToString(),
			RelVerticalSpeed);

		if (RelVerticalSpeed < SL_SB_VERT_SPEED_TH)
		{
			if (CandidateItr->bCanOnlyBeSupporting)
			{
				StartedEvents.Emplace(MakeShareable(new FSLSupportedByEvent(
					FIds::NewGuidInBase64Url(), StartTime,
					Parent->OwnerId, Parent->OwnerSemId, Parent->OwnerSemClass, // supported
					CandidateItr->Id, CandidateItr->SemId, CandidateItr->SemClass))); // supporting

			}
			else // Check which is supporting and which is supported
			{
				// TODO simple height comparison for now
				if (Parent->OwnerStaticMeshComp->GetComponentLocation().Z >
					CandidateItr->StaticMeshComp->GetComponentLocation().Z)
				{
					StartedEvents.Emplace(MakeShareable(new FSLSupportedByEvent(
						FIds::NewGuidInBase64Url(), StartTime,
						Parent->OwnerId, Parent->OwnerSemId, Parent->OwnerSemClass, // supported
						CandidateItr->Id, CandidateItr->SemId, CandidateItr->SemClass))); // supporting
				}
				else
				{
					StartedEvents.Emplace(MakeShareable(new FSLSupportedByEvent(
						FIds::NewGuidInBase64Url(), StartTime,
						CandidateItr->Id, CandidateItr->SemId, CandidateItr->SemClass, // supported
						Parent->OwnerId, Parent->OwnerSemId, Parent->OwnerSemClass))); // supporting
				}
			}
			// Remove candidate, it is now part of a started event
			CandidateItr.RemoveCurrent();
		}
	}

	// Pause timer
	if (Candidates.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT(">> %s::%d No candidates, pause timer"), TEXT(__FUNCTION__), __LINE__);
		Parent->GetWorld()->GetTimerManager().PauseTimer(TimerHandle);
	}
}

// Publish finished event
bool FSLSupportedByPublisher::FinishAndPublishEvent(const uint32 InOtherId, float EndTime)
{
	// Use iterator to be able to remove the entry from the array
	for (auto EventItr(StartedEvents.CreateIterator()); EventItr; ++EventItr)
	{
		if (((*EventItr)->SupportingObjId == InOtherId) && ((*EventItr)->SupportedObjId == Parent->OwnerId) ||
			((*EventItr)->SupportedObjId == InOtherId) && ((*EventItr)->SupportingObjId == Parent->OwnerId))
		{
			// Set end time and publish event
			(*EventItr)->End = EndTime;
			OnSupportedByEvent.ExecuteIfBound(*EventItr);
			// Remove event from the pending list
			EventItr.RemoveCurrent();
			return true;
		}
	}
	return false;
}

// Terminate and publish pending contact events (this usually is called at end play)
void FSLSupportedByPublisher::FinishAndPublishStartedEvents(float EndTime)
{
	// Finish contact events
	for (auto& Ev : StartedEvents)
	{
		// Set end time and publish event
		Ev->End = EndTime;
		OnSupportedByEvent.ExecuteIfBound(Ev);
	}
	StartedEvents.Empty();
}

// Event called when a semantic overlap event begins
void FSLSupportedByPublisher::OnSLOverlapBegin(UStaticMeshComponent* OtherStaticMeshComp,
	uint32 OtherId,
	const FString& OtherSemId,
	const FString& OtherSemClass,
	float StartTime,
	bool bIsSLOverlapArea)
{
	FSLSupportedByPublisher::AddCandidate(OtherStaticMeshComp, OtherId, OtherSemId, OtherSemClass, !bIsSLOverlapArea);
	
	// If paused, unpause timer
	if (Parent->GetWorld()->GetTimerManager().IsTimerPaused(TimerHandle))
	{
		UE_LOG(LogTemp, Warning, TEXT(">> %s::%d UnPauseTimer"), TEXT(__FUNCTION__), __LINE__);
		Parent->GetWorld()->GetTimerManager().UnPauseTimer(TimerHandle);
	}
}

// Event called when a semantic overlap event ends
void FSLSupportedByPublisher::OnSLOverlapEnd(const uint32 OtherId,
	const FString& SemOtherSemId,
	const FString& OtherSemClass,
	float EndTime,
	bool bIsSLOverlapArea)
{
	// Check if other is still a candidate, or the event can be finished and published
	/*if (!IsASupportedByCandidate(OtherId))
	{
		FinishAndPublishSupportedByEvent(OtherId);
	}*/
	FSLSupportedByPublisher::FinishAndPublishEvent(OtherId, EndTime);
}


//void USLOverlapArea::NextTickCb()
//{
//	UE_LOG(LogTemp, Warning, TEXT(">> %s::%d TS:%f"), TEXT(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds());
//}


//// Check for supported by event
//bool USLContactTrigger::IsASupportedByEvent(UStaticMeshComponent* InOtherComp1, UStaticMeshComponent* InOuterComp2)
//{
//	UE_LOG(LogTemp, Error, TEXT(">> %s::%d TS=%f"), TEXT(__FUNCTION__), __LINE__, GetWorld()->GetTimeSeconds());
//
//	UE_LOG(LogTemp, Error, TEXT(">> %s::%d Check isSupportedBy [C1,C2]=[%s,%s]"),
//		TEXT(__FUNCTION__), __LINE__, *InOtherComp1->GetOwner()->GetName(), *InOuterComp2->GetOwner()->GetName());
//
//	UE_LOG(LogTemp, Error, TEXT(">> %s::%d Velocities: Ac1Comp1=[%s::%s] and Ac2Comp2=[%s::%s]"),
//		TEXT(__FUNCTION__), __LINE__,
//		*InOtherComp1->GetOwner()->GetVelocity().ToString(), *InOtherComp1->GetComponentVelocity().ToString(),
//		*InOuterComp2->GetOwner()->GetVelocity().ToString(), *InOuterComp2->GetComponentVelocity().ToString());
//
//	//OuterMeshComp->SetGenerateOverlapEvents(false);
//	SetGenerateOverlapEvents(false);
//
//	FHitResult MoveHit;
//	OuterMeshComp->MoveComponent(FVector(0.f, 0.f, -0.5f), OuterMeshComp->GetComponentQuat(), true, &MoveHit,
//		EMoveComponentFlags::MOVECOMP_DisableBlockingOverlapDispatch, ETeleportType::TeleportPhysics);
//
//	UE_LOG(LogTemp, Warning, TEXT(">> %s::%d *** MoveHit: \n %s \n ***"),
//		TEXT(__FUNCTION__), __LINE__, *MoveHit.ToString());
//
//	FHitResult MoveHit2;
//	OuterMeshComp->MoveComponent(FVector(0.f, 0.f, 1.f), OuterMeshComp->GetComponentQuat(), true, &MoveHit2,
//		EMoveComponentFlags::MOVECOMP_DisableBlockingOverlapDispatch, ETeleportType::TeleportPhysics);
//
//	UE_LOG(LogTemp, Warning, TEXT(">> %s::%d ***  MoveHit2: \n %s \n ***"),
//		TEXT(__FUNCTION__), __LINE__, *MoveHit2.ToString());
//
//	if (!GetWorld()->GetTimerManager().IsTimerActive(TimerHandle))
//	{
//		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &USLContactTrigger::TimerCallbackSupportedBy, 0.1f, false);
//	}
//
//	
//
//
//	//OuterMeshComp->SetGenerateOverlapEvents(true);
//	//SetGenerateOverlapEvents(true);
//
//	//// BLUE
//	//FVector Comp1Loc = InOtherComp1->GetComponentLocation();
//	//FQuat Comp1Quat = InOtherComp1->GetComponentQuat();
//
//	//// RED
//	//FVector Comp2Loc = InOuterComp2->GetComponentLocation();
//	//FQuat Comp2Quat = InOuterComp2->GetComponentQuat();
//
//
//
//	////FVector C1Ext = InOtherComp1->GetCollisionShape().GetExtent();
//	////FVector C2Ext = InOuterComp2->GetCollisionShape().GetExtent();
//
//	////DrawDebugBox(GetWorld(), InOtherComp1->GetComponentLocation(), C1Ext, FColor::Red, true, 1000.f, (uint8)'\000', 0.5f);
//	////DrawDebugBox(GetWorld(), InOuterComp2->GetComponentLocation(), C2Ext, FColor::Blue, true, 1000.f, (uint8)'\000', 0.75f);
//
//	//for (const auto& B : InOuterComp2->GetStaticMesh()->BodySetup->AggGeom.BoxElems)
//	//{
//	//	UE_LOG(LogTemp, Warning, TEXT(">> %s::%d \n \t B : %s"),
//	//		TEXT(__FUNCTION__), __LINE__, *B.Center.ToString());
//	//	DrawDebugBox(GetWorld(), Comp1Loc + B.Center, FVector(B.X, B.Y, B.Z), FColor::Blue, true, 1000.f, (uint8)'\000', 0.75f);
//	//}
//
//	//for (const auto& B : InOuterComp2->GetStaticMesh()->BodySetup->AggGeom.ConvexElems)
//	//{
//	//	UE_LOG(LogTemp, Warning, TEXT(">> %s::%d \n \t C : %s"),
//	//		TEXT(__FUNCTION__), __LINE__, *B.ElemBox.ToString());
//	//	DrawDebugBox(GetWorld(), Comp1Loc + B.ElemBox.GetCenter(), B.ElemBox.GetExtent(), FColor::Blue, true, 1000.f, (uint8)'\000', 0.75f);
//	//}
//
//	//for (const auto& B : InOtherComp1->GetStaticMesh()->BodySetup->AggGeom.BoxElems)
//	//{
//	//	UE_LOG(LogTemp, Warning, TEXT(">> %s::%d \n \t OthB : %s"),
//	//		TEXT(__FUNCTION__), __LINE__, *B.Center.ToString());
//	//	DrawDebugBox(GetWorld(), Comp2Loc + B.Center, FVector(B.X, B.Y, B.Z), FColor::Red, true, 1000.f, (uint8)'\000', 0.75f);
//	//}
//
//	//for (const auto& B : InOtherComp1->GetStaticMesh()->BodySetup->AggGeom.ConvexElems)
//	//{
//	//	UE_LOG(LogTemp, Warning, TEXT(">> %s::%d \n \t OthC : %s"),
//	//		TEXT(__FUNCTION__), __LINE__, *B.ElemBox.ToString());
//	//	DrawDebugBox(GetWorld(), Comp2Loc + B.ElemBox.GetCenter(), B.ElemBox.GetExtent(), FColor::Red, true, 1000.f, (uint8)'\000', 0.75f);
//	//}
//
//	//GetWorld()->DebugDrawTraceTag = "ABC";
//	////FBodySetupShapeIterator::
//	////auto Channel = UEngineTypes::ConvertToCollisionChannel(EObjectTypeQuery::);
//
//
//	//FCollisionQueryParams Param;
//	//Param.TraceTag = "ABC";
//	//FCollisionResponseParams RParam;
//	//RParam.CollisionResponse.Visibility;
//	//
//
//
//	//TArray<FHitResult> OutHits1;
//	//GetWorld()->LineTraceMultiByChannel(OutHits1,
//	//	Comp1Loc, Comp2Loc, ECollisionChannel::ECC_Visibility, Param);
//	//	
//
//	//for (const auto& H : OutHits1)
//	//{
//	//	UE_LOG(LogTemp, Warning, TEXT(">> %s::%d \n \t H: %s"),
//	//		TEXT(__FUNCTION__), __LINE__, *H.ToString());
//	//}
//
//	//
//
//
//	
//	//// BLUE
//	//FVector Comp1Loc = InComp1->GetComponentLocation();
//	//FQuat Comp1Quat = InComp1->GetComponentQuat();
//
//	//// RED
//	//FVector Comp2Loc = InComp2->GetComponentLocation();
//	//FQuat Comp2Quat = InComp2->GetComponentQuat();
//
//	//// BLUE->RED
//	//FHitResult HitResult12;
//	//InComp1->SweepComponent(HitResult12, Comp1Loc, Comp2Loc, Comp1Quat, InComp1->GetCollisionShape());
//	//// RED->BLUE
//	//FHitResult HitResult21;
//	//InComp2->SweepComponent(HitResult21, Comp2Loc, Comp1Loc, Comp2Quat, InComp2->GetCollisionShape());
//
//	//UE_LOG(LogTemp, Warning, TEXT(">> %s::%d \n \t Sweep12 = %s \n \t Sweep21 = %s \n Sweep done!"),
//	//	TEXT(__FUNCTION__), __LINE__, *HitResult12.ToString(), *HitResult21.ToString());
//
//	//// GREEN 1-2 Trace START-END
//	//DrawDebugLine(GetWorld(), HitResult12.TraceStart, HitResult12.TraceEnd, FColor::Green, true, 1000.f, (uint8)'\000', 0.5f);
//
//	//// Yellow 2-1 Trace START-END
//	//DrawDebugLine(GetWorld(), HitResult21.TraceStart, HitResult12.TraceEnd, FColor::Green, true, 1000.f, (uint8)'\000', 0.75f);
//
//
//	//// CYAN Normal
//	//DrawDebugDirectionalArrow(GetWorld(), HitResult12.ImpactPoint, HitResult12.ImpactPoint + HitResult12.ImpactNormal,
//	//	2.f, FColor::Cyan, true, 1000.f, (uint8)'\000', 1.0f);
//
//	////// YELLOW Two components lines
//	////DrawDebugLine(GetWorld(), Comp1Loc, Comp2Loc, FColor::Yellow, true, 1000.f, (uint8)'\000', 1.f);
//
//	//// BLUE Component location
//	//DrawDebugPoint(GetWorld(), Comp1Loc, 2.f, FColor::Blue, true, 1000.f);
//
//	//// RED Component location
//	//DrawDebugPoint(GetWorld(), Comp2Loc, 2.f, FColor::Red, true, 1000.f);
//
//	return true;
//}

