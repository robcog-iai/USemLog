// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLContactTrigger.h"
#include "SLMappings.h"

// UUTils
#include "Tags.h"
#include "Ids.h"

#define SL_COLL_SCALE_FACTOR 1.04f
#define SL_COLL_TAGTYPE "SemLogColl"

// Default constructor
USLContactTrigger::USLContactTrigger()
{
}

// Called at level startup
void USLContactTrigger::BeginPlay()
{
	Super::BeginPlay();

	// Check if component is ready for runtime
	if (RuntimeInit())
	{
		// If objects are already overlapping at begin play, they will not be triggered
		// Here we do a manual overlap check and forward them to OnOverlapBegin
		TSet<UPrimitiveComponent*> CurrOverlappingComponents;
		GetOverlappingComponents(CurrOverlappingComponents);
		FHitResult Dummy;
		for (const auto& CompItr : CurrOverlappingComponents)
		{
			USLContactTrigger::OnOverlapBegin(
				this, CompItr->GetOwner(), CompItr, 0, false, Dummy);
		}

		// Bind event delegates
		OnComponentBeginOverlap.AddDynamic(this, &USLContactTrigger::OnOverlapBegin);
		OnComponentEndOverlap.AddDynamic(this, &USLContactTrigger::OnOverlapEnd);
	}
}

// Called when actor removed from game or game ended
void USLContactTrigger::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	// Terminate and publish pending events
	FinishAndPublishAllStartedEvents();
}

// Called after the C++ constructor and after the properties have been initialized
void  USLContactTrigger::PostInitProperties()
{
	Super::PostInitProperties();

	if (!LoadAndApplyTriggerAreaSize())
	{
		CalculateAndApplyTriggerAreaSize();
	}
	ShapeColor = FColor::Blue;
}

#if WITH_EDITOR
// Called when a property is changed in the editor
void USLContactTrigger::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property and member names
	FName PropertyName = PropertyChangedEvent.GetPropertyName();

	FName MemberPropertyName = (PropertyChangedEvent.MemberProperty != NULL) ? 
		PropertyChangedEvent.MemberProperty->GetFName() : NAME_None;

	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(USLContactTrigger, BoxExtent))
	{
		if (PropertyName == FName("X"))
		{
			FTags::AddKeyValuePair(GetOuter(), SL_COLL_TAGTYPE, "ExtX",
				FString::SanitizeFloat(BoxExtent.X));
		}
		else if (PropertyName == FName("Y"))
		{
			FTags::AddKeyValuePair(GetOuter(), SL_COLL_TAGTYPE, "ExtY",
				FString::SanitizeFloat(BoxExtent.Y));
		}
		else if (PropertyName == FName("Z"))
		{
			FTags::AddKeyValuePair(GetOuter(), SL_COLL_TAGTYPE, "ExtY",
				FString::SanitizeFloat(BoxExtent.Y));
		}
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(USLContactTrigger, RelativeLocation))
	{
		if (PropertyName == FName("X"))
		{
			FTags::AddKeyValuePair(GetOuter(), SL_COLL_TAGTYPE, "LocX",
				FString::SanitizeFloat(RelativeLocation.X));
		}
		else if (PropertyName == FName("Y"))
		{
			FTags::AddKeyValuePair(GetOuter(), SL_COLL_TAGTYPE, "LocY",
				FString::SanitizeFloat(RelativeLocation.Y));
		}
		else if (PropertyName == FName("Z"))
		{
			FTags::AddKeyValuePair(GetOuter(), SL_COLL_TAGTYPE, "LocZ",
				FString::SanitizeFloat(RelativeLocation.Y));
		}
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(USLContactTrigger, RelativeRotation))
	{
		const FQuat RelQuat = GetRelativeTransform().GetRotation();
		TMap<FString, FString> KeyValMap;
		KeyValMap.Add("QuatW", FString::SanitizeFloat(RelQuat.W));
		KeyValMap.Add("QuatX", FString::SanitizeFloat(RelQuat.X));
		KeyValMap.Add("QuatY", FString::SanitizeFloat(RelQuat.Y));
		KeyValMap.Add("QuatZ", FString::SanitizeFloat(RelQuat.Z));
		FTags::AddKeyValuePairs(GetOuter(), SL_COLL_TAGTYPE, KeyValMap);
	}
}

// Called when this component is moved in the editor
void USLContactTrigger::PostEditComponentMove(bool bFinished)
{
	// Update tags with the new transform
	const FTransform RelTransf = GetRelativeTransform();
	const FVector RelLoc = RelTransf.GetLocation();
	const FQuat RelQuat = RelTransf.GetRotation();

	TMap<FString, FString> KeyValMap;

	KeyValMap.Add("LocX", FString::SanitizeFloat(RelLoc.X));
	KeyValMap.Add("LocY", FString::SanitizeFloat(RelLoc.Y));
	KeyValMap.Add("LocZ", FString::SanitizeFloat(RelLoc.Z));

	KeyValMap.Add("QuatW", FString::SanitizeFloat(RelQuat.W));
	KeyValMap.Add("QuatX", FString::SanitizeFloat(RelQuat.X));
	KeyValMap.Add("QuatY", FString::SanitizeFloat(RelQuat.Y));
	KeyValMap.Add("QuatZ", FString::SanitizeFloat(RelQuat.Z));

	FTags::AddKeyValuePairs(GetOuter(), SL_COLL_TAGTYPE, KeyValMap);
}
#endif // WITH_EDITOR

// Read values from tags
bool USLContactTrigger::LoadAndApplyTriggerAreaSize()
{
	TMap<FString, FString> TagKeyValMap = 
		FTags::GetKeyValuePairs(GetOuter(), SL_COLL_TAGTYPE);

	if (TagKeyValMap.Num() == 0){return false;}

	FVector BoxExt;
	if (FString* ValPtr = TagKeyValMap.Find("ExtX")) { BoxExt.X = FCString::Atof(**ValPtr); }
	else { return false; }
	if (FString* ValPtr = TagKeyValMap.Find("ExtY")) { BoxExt.Y = FCString::Atof(**ValPtr); }
	else { return false; }
	if (FString* ValPtr = TagKeyValMap.Find("ExtZ")) { BoxExt.Z = FCString::Atof(**ValPtr); }
	else { return false; }

	SetBoxExtent(BoxExt);

	FVector RelLoc;
	if (FString* ValPtr = TagKeyValMap.Find("LocX")) { RelLoc.X = FCString::Atof(**ValPtr); }
	else { return false; }
	if (FString* ValPtr = TagKeyValMap.Find("LocY")) { RelLoc.Y = FCString::Atof(**ValPtr); }
	else { return false; }
	if (FString* ValPtr = TagKeyValMap.Find("LocZ")) { RelLoc.Z = FCString::Atof(**ValPtr); }
	else { return false; }

	FQuat RelQuat;
	if (FString* ValPtr = TagKeyValMap.Find("QuatW")) { RelQuat.W = FCString::Atof(**ValPtr); }
	else { return false; }
	if (FString* ValPtr = TagKeyValMap.Find("QuatX")) { RelQuat.X = FCString::Atof(**ValPtr); }
	else { return false; }
	if (FString* ValPtr = TagKeyValMap.Find("QuatY")) { RelQuat.Y = FCString::Atof(**ValPtr); }
	else { return false; }
	if (FString* ValPtr = TagKeyValMap.Find("QuatZ")) { RelQuat.Z = FCString::Atof(**ValPtr); }
	else { return false; }

	SetRelativeTransform(FTransform(RelQuat, RelLoc));
	return true;
}

// Calculate trigger area size
bool USLContactTrigger::CalculateAndApplyTriggerAreaSize()
{
	// Get the static mesh component
	if (AStaticMeshActor* OuterAsSMAct = Cast<AStaticMeshActor>(GetOuter()))
	{
		UStaticMeshComponent* SMComp = OuterAsSMAct->GetStaticMeshComponent();

		// Apply parameters to the contact listener area
		SetBoxExtent(SMComp->Bounds.BoxExtent * SL_COLL_SCALE_FACTOR);
		// Apply its location
		FTransform BoundsTransf(FQuat::Identity, SMComp->Bounds.Origin);
		BoundsTransf.SetToRelativeTransform(SMComp->GetComponentTransform());
		SetRelativeTransform(BoundsTransf);

		// Save calculated data
		StoreTriggerAreaSize(GetRelativeTransform(), BoxExtent);
		return true;
	}
	return false;
}

// Save values to tags
bool USLContactTrigger::StoreTriggerAreaSize(const FTransform& InTransform, const FVector& InBoxExtent)
{
	const FVector RelLoc = InTransform.GetLocation();
	const FQuat RelQuat = InTransform.GetRotation();

	TMap<FString, FString> KeyValMap;
	
	KeyValMap.Add("ExtX", FString::SanitizeFloat(InBoxExtent.X));
	KeyValMap.Add("ExtY", FString::SanitizeFloat(InBoxExtent.Y));
	KeyValMap.Add("ExtZ", FString::SanitizeFloat(InBoxExtent.Z));
	
	KeyValMap.Add("LocX", FString::SanitizeFloat(RelLoc.X));
	KeyValMap.Add("LocY", FString::SanitizeFloat(RelLoc.Y));
	KeyValMap.Add("LocZ", FString::SanitizeFloat(RelLoc.Z));

	KeyValMap.Add("QuatW", FString::SanitizeFloat(RelQuat.W));
	KeyValMap.Add("QuatX", FString::SanitizeFloat(RelQuat.X));
	KeyValMap.Add("QuatY", FString::SanitizeFloat(RelQuat.Y));
	KeyValMap.Add("QuatZ", FString::SanitizeFloat(RelQuat.Z));
	
	return FTags::AddKeyValuePairs(GetOuter(), SL_COLL_TAGTYPE, KeyValMap);
}

// Setup pointers to outer, check if semantically annotated
bool USLContactTrigger::RuntimeInit()
{
	// Make sure outer is a static mesh actor
	if (AStaticMeshActor* OuterMeshAsAct = Cast<AStaticMeshActor>(GetOuter()))
	{
		OuterMeshAct = OuterMeshAsAct;
		OuterId = OuterMeshAct->GetUniqueID();

		OuterMeshComp = OuterMeshAct->GetStaticMeshComponent();
		// Make sure it has a valid mesh component
		if (OuterMeshComp)
		{
			// Make sure there are no overlap events on the mesh as well
			// (these will be calculated on the contact listener)
			// TODO this will cause problems with grasping objects
			OuterMeshComp->SetGenerateOverlapEvents(false);

			// Init the semantic items content singleton
			if (!FSLMappings::GetInstance()->IsInit())
			{
				FSLMappings::GetInstance()->LoadData(GetWorld());
			}
			// Make sure it has a semantic unique id and class
			OuterSemId = FSLMappings::GetInstance()->GetSemanticId(OuterId);
			OuterClass = FSLMappings::GetInstance()->GetSemanticClass(OuterId);

			if (!OuterSemId.IsEmpty() && !OuterClass.IsEmpty())
			{
				return true;
			}
		}
	}
	return false;
}

// Start new contact event
void USLContactTrigger::AddStartedContactEvent(
	const uint32 InOtherUniqueId,
	const FString& InOtherSemId,
	const FString& InOtherClass)
{
	// Start a semantic contact event
	TSharedPtr<FSLContactEvent> ContactEvent = MakeShareable(new FSLContactEvent);
	ContactEvent->Id = FIds::NewGuidInBase64Url();
	ContactEvent->Obj1UniqueId = OuterId;
	ContactEvent->Obj1Id = OuterSemId;
	ContactEvent->Obj1Class = OuterClass;
	ContactEvent->Obj2UniqueId = InOtherUniqueId;
	ContactEvent->Obj2Id = InOtherSemId;
	ContactEvent->Obj2Class = InOtherClass;
	ContactEvent->Start = GetWorld()->GetTimeSeconds();
	// Add event to the pending contacts array
	StartedContactEvents.Emplace(ContactEvent);
}

// Start new supported by event
void USLContactTrigger::AddStartedSupportedByEvent(
	const uint32 InOtherUniqueId,
	const FString& InOtherSemId,
	const FString& InOtherClass)
{
	// Start a semantic contact event
	TSharedPtr<FSLSupportedByEvent> SupportedByEvent = MakeShareable(new FSLSupportedByEvent);
	SupportedByEvent->Id = FIds::NewGuidInBase64Url();
	SupportedByEvent->Obj1UniqueId = OuterId;
	SupportedByEvent->Obj1Id = OuterSemId;
	SupportedByEvent->Obj1Class = OuterClass;
	SupportedByEvent->Obj2UniqueId = InOtherUniqueId;
	SupportedByEvent->Obj2Id = InOtherSemId;
	SupportedByEvent->Obj2Class = InOtherClass;
	SupportedByEvent->Start = GetWorld()->GetTimeSeconds();
	// Add event to the pending contacts array
	StartedSupportedByEvents.Emplace(SupportedByEvent);
}

// Publish finished event
bool USLContactTrigger::FinishAndPublishContactEvent(const FString& InOtherSemId)
{
	// Use iterator to be able to remove the entry from the array
	for (auto EventItr(StartedContactEvents.CreateIterator()); EventItr; ++EventItr)
	{
		if ((*EventItr)->Obj2Id.Equals(InOtherSemId))
		{
			// Set end time and publish event
			(*EventItr)->End = GetWorld()->GetTimeSeconds();
			OnSemanticContactEvent.ExecuteIfBound(*EventItr);
			// Remove event from the pending list
			EventItr.RemoveCurrent();
			return true;
		}
	}
	return false;
}

// Publish finished event
bool USLContactTrigger::FinishAndPublishSupportedByEvent(const FString& InOtherSemId)
{
	// Use iterator to be able to remove the entry from the array
	for (auto EventItr(StartedSupportedByEvents.CreateIterator()); EventItr; ++EventItr)
	{
		if ((*EventItr)->Obj2Id.Equals(InOtherSemId))
		{
			// Set end time and publish event
			(*EventItr)->End = GetWorld()->GetTimeSeconds();			
			OnSemanticSupportedByEvent.ExecuteIfBound(*EventItr);
			// Remove event from the pending list
			EventItr.RemoveCurrent();
			return true;
		}
	}
	return false;
}

// Terminate and publish pending contact events (this usually is called at end play)
void USLContactTrigger::FinishAndPublishAllStartedEvents()
{
	const float EndTime = GetWorld()->GetTimeSeconds();

	// Finish contact events
	for (auto& Ev : StartedContactEvents)
	{
		// Set end time and publish event
		Ev->End = EndTime;
		OnSemanticContactEvent.ExecuteIfBound(Ev);
	}
	StartedContactEvents.Empty();

	// Finish supported by events
	for (auto& Ev : StartedSupportedByEvents)
	{
		// Set end time and publish event
		Ev->End = EndTime;
		OnSemanticSupportedByEvent.ExecuteIfBound(Ev);
	}
	StartedSupportedByEvents.Empty();
}

// Called on overlap begin events
void USLContactTrigger::OnOverlapBegin(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	// Ignore self overlaps (area with static mesh)
	if (OtherActor == OuterMeshAct)
	{
		return;
	}

	// Check if other actor is semantically annotated
	const uint32 OtherUniqueId = OtherActor->GetUniqueID();
	const FString OtherSemId = FSLMappings::GetInstance()->GetSemanticId(OtherUniqueId);
	const FString OtherClass = FSLMappings::GetInstance()->GetSemanticClass(OtherUniqueId);
	if (OtherSemId.IsEmpty() || OtherClass.IsEmpty())
	{
		return;
	}
	
	// If both areas are trigger areas, they will both concurrently trigger overlap events.
	// To avoid this we consistently ignore one trigger event. This is chosen using
	// the unique ids of the overlapping actors (GetUniqueID), we compare the two values 
	// and consistently pick the event with a given (larger or smaller) value.
	// This allows us to be in sync with the overlap end event 
	// since the unique ids and the rule of ignoring the one event will not change
	if (OtherComp->IsA(USLContactTrigger::StaticClass()))
	{
		if (OtherUniqueId > OuterId)
		{
			// Create a new contact event
			AddStartedContactEvent(OtherUniqueId, OtherSemId, OtherClass);
			AddStartedSupportedByEvent(OtherUniqueId, OtherSemId, OtherClass);
			return;
		}
	}

	// Create a new contact event
	AddStartedContactEvent(OtherUniqueId, OtherSemId, OtherClass);
	AddStartedSupportedByEvent(OtherUniqueId, OtherSemId, OtherClass);
}

// Called on overlap end events
void USLContactTrigger::OnOverlapEnd(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	// Ignore overlaps with itself
	if (OtherActor == OuterMeshAct)
	{
		return;
	}

	// Check if other actor is semantically annotated
	const uint32 OtherUniqueId = OtherActor->GetUniqueID();
	const FString OtherSemId = FSLMappings::GetInstance()->GetSemanticId(OtherUniqueId);
	const FString OtherClass = FSLMappings::GetInstance()->GetSemanticClass(OtherUniqueId);
	if (OtherSemId.IsEmpty() || OtherClass.IsEmpty())
	{
		return;
	}

	// If both areas are trigger areas, they will both concurrently trigger overlap events.
	// To avoid this we consistently ignore one trigger event. This is chosen using
	// the unique ids of the overlapping actors (GetUniqueID), we compare the two values 
	// and consistently pick the event with a given (larger or smaller) value.
	// This allows us to be in sync with the overlap end event 
	// since the unique ids and the rule of ignoring the one event will not change
	if (OtherComp->IsA(USLContactTrigger::StaticClass()))
	{
		if (OtherUniqueId > OuterId)
		{
			// Publish finished contact event
			FinishAndPublishContactEvent(OtherSemId);
			FinishAndPublishSupportedByEvent(OtherSemId);
			return;
		}
	}

	// Publish finished contact event
	FinishAndPublishContactEvent(OtherSemId);
	FinishAndPublishSupportedByEvent(OtherSemId);
}
