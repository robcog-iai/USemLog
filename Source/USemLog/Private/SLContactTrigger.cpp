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

	// Check if initialization is successful
	if (RuntimeInit())
	{
		// Bind event delegates
		OnComponentBeginOverlap.AddDynamic(this, &USLContactTrigger::OnOverlapBegin);
		OnComponentEndOverlap.AddDynamic(this, &USLContactTrigger::OnOverlapEnd);
	}
}

// Called when actor removed from game or game ended
void USLContactTrigger::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	// Terminate and publish pending contact events
	FinishRemainingPendingEvents();
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
		OuterUniqueId = OuterMeshAct->GetUniqueID();

		OuterMeshComp = OuterMeshAct->GetStaticMeshComponent();
		// Make sure it has a valid mesh component
		if (OuterMeshComp)
		{
			// Make sure there are no overlap events on the mesh as well
			// (these will be calculated on the contact listener)
			// TODO this will cause problems with grasping objects
			OuterMeshComp->SetGenerateOverlapEvents(false);

			// Make sure it has a semantic unique id
			OuterSemLogId = FTags::GetValue(OuterMeshAct, "SemLog", "Id");
			if (!OuterSemLogId.IsEmpty())
			{
				return true;
			}
		}
	}
	return false;
}

// Start new contact event
void USLContactTrigger::AddNewPendingContactEvent(const FString& InOtherSemLogId)
{
	// Start a semantic contact event
	TSharedPtr<FSLContactEvent> Event = MakeShareable(new FSLContactEvent);
	Event->Id = FIds::NewGuidInBase64Url();
	Event->Obj1Id = OuterSemLogId;
	Event->Obj2Id = InOtherSemLogId;
	Event->Start = GetWorld()->GetTimeSeconds();
	// Add event to the pending contacts array
	PendingContactEvents.Emplace(Event);
}

// Publish finished event
bool USLContactTrigger::PublishFinishedContactEvent(const FString& InOtherSemLogId)
{
	// Use iterator to be able to remove the entry from the array
	for (auto EventItr(PendingContactEvents.CreateIterator()); EventItr; ++EventItr)
	{
		if ((*EventItr)->Obj2Id.Equals(InOtherSemLogId))
		{
			// Set end time
			(*EventItr)->End = GetWorld()->GetTimeSeconds();
			// Publish event
			OnSemanticContactEvent.ExecuteIfBound(*EventItr);
			// Remove event from the pending list
			EventItr.RemoveCurrent();
			return true;
		}
	}
	return false;
}

// Terminate and publish pending contact events (this usually is called at end play)
void USLContactTrigger::FinishRemainingPendingEvents()
{
	const float EndTime = GetWorld()->GetTimeSeconds();
	for (auto& Ev : PendingContactEvents)
	{
		// Set end time for the event
		Ev->End = EndTime;
		// Publish event
		OnSemanticContactEvent.ExecuteIfBound(Ev);
	}
	PendingContactEvents.Empty();
}

// Called on overlap begin events
void USLContactTrigger::OnOverlapBegin(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Error, TEXT("[%s][%d] !! OVERLAP BEGIN !! ME - OTHER : %s - %s"),
		TEXT(__FUNCTION__), __LINE__, *OuterMeshAct->GetName(), *OtherActor->GetName());

	// Ignore overlaps with itself
	if (OtherActor == OuterMeshAct)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s][%d] Ignoring self collision - ABORT"), TEXT(__FUNCTION__), __LINE__);
		return;
	}

	// Check if other actor is semantically annotated
	const uint32 OtherUniqueId = OtherActor->GetUniqueID();
	const FString OtherSemLogId = FSLMappings::GetInstance()->GetSemLogId(OtherUniqueId);
	if (OtherSemLogId.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("[%s][%d] Other is not semantically annotated - ABORT"), TEXT(__FUNCTION__), __LINE__);
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
		if (OtherUniqueId > OuterUniqueId)
		{
			UE_LOG(LogTemp, Warning, TEXT("[%s][%d] Add contact **TriggerArea** OtherId: %s"),
				TEXT(__FUNCTION__), __LINE__, *OtherSemLogId);

			// Create a new contact event
			AddNewPendingContactEvent(OtherSemLogId);
			return;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[%s][%d] Other is a trigger area, avoiding double contact trigger - ABORT"), TEXT(__FUNCTION__), __LINE__);
			return;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[%s][%d] Add contact SM OtherId: %s"),
		TEXT(__FUNCTION__), __LINE__, *OtherSemLogId);
	// Create a new contact event
	AddNewPendingContactEvent(OtherSemLogId);
}

// Called on overlap end events
void USLContactTrigger::OnOverlapEnd(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	UE_LOG(LogTemp, Error, TEXT("[%s][%d] !! OVERLAP END !! ME - OTHER : %s - %s"),
		TEXT(__FUNCTION__), __LINE__, *OuterMeshAct->GetName(), *OtherActor->GetName());

	// Ignore overlaps with itself
	if (OtherActor == OuterMeshAct)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s][%d] Ignoring self collision - ABORT"), TEXT(__FUNCTION__), __LINE__);
		return;
	}

	// Check if other actor is semantically annotated
	const uint32 OtherUniqueId = OtherActor->GetUniqueID();
	const FString OtherSemLogId = FSLMappings::GetInstance()->GetSemLogId(OtherUniqueId);
	if (OtherSemLogId.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("[%s][%d] Other is not semantically annotated - ABORT"), TEXT(__FUNCTION__), __LINE__);
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
		if (OtherUniqueId > OuterUniqueId)
		{
			UE_LOG(LogTemp, Warning, TEXT("[%s][%d] Finish contact **TriggerArea** OtherId: %s"),
				TEXT(__FUNCTION__), __LINE__, *OtherSemLogId);

			// Publish finished contact event
			if (!PublishFinishedContactEvent(OtherSemLogId))
			{
				UE_LOG(LogTemp, Warning, TEXT("[%s][%d] Cold not finish the semantic event OtherId: %s"),
					TEXT(__FUNCTION__), __LINE__, *OtherSemLogId);
			}
			return;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[%s][%d] Other is a trigger area, avoiding double contact trigger - ABORT"), TEXT(__FUNCTION__), __LINE__);
			return;
		}
	}

	// Publish finished contact event
	if (!PublishFinishedContactEvent(OtherSemLogId))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s][%d] Cold not finish the semantic event OtherSemLogId=%s"),
			TEXT(__FUNCTION__), __LINE__, *OtherSemLogId);
	}
}
