// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLContactPublisher.h"
#include "SLOverlapArea.h"

// UUtils
#include "Ids.h"

// Default constructor
FSLContactPublisher::FSLContactPublisher(USLOverlapArea* InSLOverlapArea)
{
	Parent = InSLOverlapArea;
}

// Init
void FSLContactPublisher::Init()
{
	//Parent->OnBeginSLOverlap.AddRaw(this, &FSLContactPublisher::OnSLOverlapBegin);
	//Parent->OnEndSLOverlap.AddRaw(this, &FSLContactPublisher::OnSLOverlapEnd);
	Parent->OnBeginSLOverlap2.AddRaw(this, &FSLContactPublisher::OnSLOverlapBegin);
	Parent->OnEndSLOverlap2.AddRaw(this, &FSLContactPublisher::OnSLOverlapEnd);
}


// Terminate listener, finish and publish remaining events
void FSLContactPublisher::Finish(float EndTime)
{
	FSLContactPublisher::FinishAndPublishAllEvents(EndTime);
}

//// Start new contact event
//void FSLContactPublisher::StartAndAddEvent(
//	const uint32 InOtherId,
//	const FString& InOtherSemId,
//	const FString& InOtherSemClass,
//	float StartTime)
//{
//	// Start a semantic contact event
//	TSharedPtr<FSLContactEvent> ContactEvent = MakeShareable(new FSLContactEvent(
//		FIds::NewGuidInBase64Url(), StartTime,
//		Parent->OwnerId, Parent->OwnerSemId, Parent->OwnerSemClass,
//		InOtherId, InOtherSemId, InOtherSemClass));
//	// Add event to the pending contacts array
//	StartedEvents.Emplace(ContactEvent);
//}
void FSLContactPublisher::StartAndAddNewEvent(const FSLOverlapResult& InResult)
{
	UE_LOG(LogTemp, Warning, TEXT(">> %s::%d"), TEXT(__FUNCTION__), __LINE__);

	// Start a semantic contact event
	TSharedPtr<FSLContactEvent> ContactEvent = MakeShareable(new FSLContactEvent(
		FIds::PairEncodeCantor(InResult.Id, Parent->OwnerId),
		FIds::NewGuidInBase64Url(),
		InResult.TriggerTime,
		Parent->OwnerId, Parent->OwnerSemId, Parent->OwnerSemClass,
		InResult.Id, InResult.SemId, InResult.SemClass));
	// Add event to the pending contacts array
	StartedEvents.Emplace(ContactEvent);
}

// Publish finished event
bool FSLContactPublisher::FinishAndPublishEvent(const uint32 InOtherId, float EndTime)
{
	UE_LOG(LogTemp, Warning, TEXT(">> %s::%d"), TEXT(__FUNCTION__), __LINE__);

	// Use iterator to be able to remove the entry from the array
	for (auto EventItr(StartedEvents.CreateIterator()); EventItr; ++EventItr)
	{
		if ((*EventItr)->Obj2Id == InOtherId)
		{
			UE_LOG(LogTemp, Warning, TEXT(">> %s::%d  MATCH "), TEXT(__FUNCTION__), __LINE__);

			// Set end time and publish event
			(*EventItr)->End = EndTime;
			OnSemanticContactEvent.ExecuteIfBound(*EventItr);
			// Remove event from the pending list
			EventItr.RemoveCurrent();
			return true;
		}
	}
	return false;
}

// Terminate and publish pending contact events (this usually is called at end play)
void FSLContactPublisher::FinishAndPublishAllEvents(float EndTime)
{
	UE_LOG(LogTemp, Warning, TEXT(">> %s::%d"), TEXT(__FUNCTION__), __LINE__);
	// Finish contact events
	for (auto& Ev : StartedEvents)
	{
		// Set end time and publish event
		Ev->End = EndTime;
		OnSemanticContactEvent.ExecuteIfBound(Ev);
	}
	StartedEvents.Empty();
}


// Event called when a semantic overlap event begins
void FSLContactPublisher::OnSLOverlapBegin(const FSLOverlapResult& SemanticOverlapBeginResult)
{
	UE_LOG(LogTemp, Warning, TEXT(">> %s::%d"), TEXT(__FUNCTION__), __LINE__);

	FSLContactPublisher::StartAndAddNewEvent(SemanticOverlapBeginResult);
}
//void FSLContactPublisher::OnSLOverlapBegin(UStaticMeshComponent* OtherStaticMeshComp,
//	const uint32 OtherId,
//	const FString& OtherSemId,
//	const FString& OtherSemClass,
//	float StartTime,
//	bool bIsSLOverlapArea)
//{
//	FSLContactPublisher::StartAndAddEvent(OtherId, OtherSemId, OtherSemClass, StartTime);
//}

// Event called when a semantic overlap event ends
void FSLContactPublisher::OnSLOverlapEnd(const FSLOverlapResult& SemanticOverlapBeginResult)
{
	UE_LOG(LogTemp, Warning, TEXT(">> %s::%d"), TEXT(__FUNCTION__), __LINE__);
	FSLContactPublisher::FinishAndPublishEvent(SemanticOverlapBeginResult.Id, SemanticOverlapBeginResult.TriggerTime);
}
//void FSLContactPublisher::OnSLOverlapEnd(const uint32 OtherId,
//	const FString& SemOtherSemId,
//	const FString& OtherSemClass,
//	float EndTime,
//	bool bIsSLOverlapArea)
//{
//	FSLContactPublisher::FinishAndPublishEvent(OtherId, EndTime);
//}