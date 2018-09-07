// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLOverlapArea.h"
#include "SLMappings.h"
#include "DrawDebugHelpers.h"

// UUTils
#include "Tags.h"
#include "Ids.h"

#define SL_COLL_SCALE_FACTOR 1.03f
#define SL_COLL_TAGTYPE "SemLogColl"

// Default constructor
USLOverlapArea::USLOverlapArea()
{
	bListenForContactEvents = true;
	bListenForSupportedByEvents = true;
}

// Called at level startup
void USLOverlapArea::BeginPlay()
{
	Super::BeginPlay();

	// Check if component is ready for runtime
	if (Init())
	{
		// If objects are already overlapping at begin play, they will not be triggered
		// Here we do a manual overlap check and forward them to OnOverlapBegin
		TSet<UPrimitiveComponent*> CurrOverlappingComponents;
		GetOverlappingComponents(CurrOverlappingComponents);
		FHitResult Dummy;
		for (const auto& CompItr : CurrOverlappingComponents)
		{
			USLOverlapArea::OnOverlapBegin(
				this, CompItr->GetOwner(), CompItr, 0, false, Dummy);
		}

		// Bind event delegates
		OnComponentBeginOverlap.AddDynamic(this, &USLOverlapArea::OnOverlapBegin);
		OnComponentEndOverlap.AddDynamic(this, &USLOverlapArea::OnOverlapEnd);

		// Listen and publish semantic contact events
		if (bListenForContactEvents)
		{
			SLContactPub = MakeShareable(new FSLContactPublisher(this));
			SLContactPub->Init();
		}

		// Listen and publish supported by events
		if (bListenForSupportedByEvents)
		{
			SLSupportedByPub = MakeShareable(new FSLSupportedByPublisher(this));
			SLSupportedByPub->Init();
		}
	}
}

// Called when actor removed from game or game ended
void USLOverlapArea::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	float EndTime = GetWorld()->GetTimeSeconds();

	// Terminate and publish pending events
	if (SLContactPub.IsValid())
	{
		SLContactPub->Finish(EndTime);
	}

	if (SLSupportedByPub.IsValid())
	{
		SLSupportedByPub->Finish(EndTime);
	}
}

// Called after the C++ constructor and after the properties have been initialized
void  USLOverlapArea::PostInitProperties()
{
	Super::PostInitProperties();

	if (!ReadAndUpdateArea())
	{
		UpdateArea();
	}
	ShapeColor = FColor::Blue;
}

#if WITH_EDITOR
// Called when a property is changed in the editor
void USLOverlapArea::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property and member names
	FName PropertyName = PropertyChangedEvent.GetPropertyName();

	FName MemberPropertyName = (PropertyChangedEvent.MemberProperty != NULL) ? 
		PropertyChangedEvent.MemberProperty->GetFName() : NAME_None;

	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(USLOverlapArea, BoxExtent))
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
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(USLOverlapArea, RelativeLocation))
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
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(USLOverlapArea, RelativeRotation))
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
void USLOverlapArea::PostEditComponentMove(bool bFinished)
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
bool USLOverlapArea::ReadAndUpdateArea()
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
bool USLOverlapArea::UpdateArea()
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
		SaveArea();
		return true;
	}
	return false;
}

// Save values to tags
bool USLOverlapArea::SaveArea()
{
	const FTransform RelTransf = GetRelativeTransform();
	const FVector RelLoc = RelTransf.GetLocation();
	const FQuat RelQuat = RelTransf.GetRotation();

	TMap<FString, FString> KeyValMap;
	
	KeyValMap.Add("ExtX", FString::SanitizeFloat(BoxExtent.X));
	KeyValMap.Add("ExtY", FString::SanitizeFloat(BoxExtent.Y));
	KeyValMap.Add("ExtZ", FString::SanitizeFloat(BoxExtent.Z));
	
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
bool USLOverlapArea::Init()
{
	// Make sure outer is a static mesh actor
	if (AStaticMeshActor* CastToSMAct = Cast<AStaticMeshActor>(GetOwner()))
	{
		OwnerStaticMeshAct = CastToSMAct;
		OwnerId = OwnerStaticMeshAct->GetUniqueID();

		OwnerStaticMeshComp = OwnerStaticMeshAct->GetStaticMeshComponent();
		// Make sure it has a valid mesh component
		if (OwnerStaticMeshComp)
		{
			// Make sure there are no overlap events on the mesh as well
			// (these will be calculated on the contact listener)
			// TODO this might cause problems with grasping objects
			OwnerStaticMeshComp->SetGenerateOverlapEvents(false);

			// Init the semantic items content singleton
			if (!FSLMappings::GetInstance()->IsInit())
			{
				FSLMappings::GetInstance()->LoadData(GetWorld());
			}
			// Make sure it has a semantic unique id and class
			OwnerSemId = FSLMappings::GetInstance()->GetSemanticId(OwnerId);
			OwnerSemClass = FSLMappings::GetInstance()->GetSemanticClass(OwnerId);

			if (!OwnerSemId.IsEmpty() && !OwnerSemClass.IsEmpty())
			{
				return true;
			}
		}
	}
	return false;
}


// Called on overlap begin events
void USLOverlapArea::OnOverlapBegin(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	// Ignore self overlaps (area with static mesh)
	if (OtherActor == OwnerStaticMeshAct)
	{
		return;
	}

	// Make sure other is a static mesh actor with a valid static mesh component
	AStaticMeshActor* OtherSMAct = Cast<AStaticMeshActor>(OtherActor);
	UStaticMeshComponent* OtherSMComp = nullptr;
	// Ignore if other actor is not a static mesh actor, or does not have a static mesh component
	if (OtherSMAct)
	{
		OtherSMComp = OtherSMAct->GetStaticMeshComponent();
		if (OtherSMComp == nullptr)
		{
			return;
		}
	}
	else
	{
		return;
	}

	// Make sure other is semantically annotated
	const uint32 OtherId = OtherActor->GetUniqueID();
	const FString OtherSemId = FSLMappings::GetInstance()->GetSemanticId(OtherId);
	const FString OtherSemClass = FSLMappings::GetInstance()->GetSemanticClass(OtherId);
	if (OtherSemId.IsEmpty() || OtherSemClass.IsEmpty())
	{
		return;
	}

	// Set by default that the other component is not an semantic overlap area (this will be checked further)
	bool bOtherIsASemanticOverlapArea = false;
	// Get the time of the event in second
	float StartTime = GetWorld()->GetTimeSeconds();
	
	// If both areas are trigger areas, they will both concurrently trigger overlap events.
	// To avoid this we consistently ignore one trigger event. This is chosen using
	// the unique ids of the overlapping actors (GetUniqueID), we compare the two values 
	// and consistently pick the event with a given (larger or smaller) value.
	// This allows us to be in sync with the overlap end event 
	// since the unique ids and the rule of ignoring the one event will not change
	if (USLOverlapArea* OtherContactTrigger = Cast<USLOverlapArea>(OtherComp))
	{
		// Other is a semantic overlap component
		bOtherIsASemanticOverlapArea = true;

		// Filter out one of the trigger areas
		if (OtherId > OwnerId)
		{
			// Broadcast begin of semantic overlap event
			FSLOverlapResult SemanticOverlapResult(OtherId, OtherSemId, OtherSemClass, StartTime,
				bOtherIsASemanticOverlapArea, OtherSMAct, OtherSMComp);
			OnBeginSLOverlap.Broadcast(SemanticOverlapResult);
			return;
		}
	}

	// Broadcast begin of semantic overlap event
	FSLOverlapResult SemanticOverlapResult(OtherId, OtherSemId, OtherSemClass, StartTime,
		bOtherIsASemanticOverlapArea, OtherSMAct, OtherSMComp);
	OnBeginSLOverlap.Broadcast(SemanticOverlapResult);
}

// Called on overlap end events
void USLOverlapArea::OnOverlapEnd(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	// Ignore overlaps with itself
	if (OtherActor == OwnerStaticMeshAct)
	{
		return;
	}

	// Make sure other is a static mesh actor with a valid static mesh component
	AStaticMeshActor* OtherSMAct = Cast<AStaticMeshActor>(OtherActor);
	UStaticMeshComponent* OtherSMComp = nullptr;
	// Ignore if other actor is not a static mesh actor, or does not have a static mesh component
	if (OtherSMAct)
	{
		OtherSMComp = OtherSMAct->GetStaticMeshComponent();
		if (OtherSMComp == nullptr)
		{
			return;
		}
	}
	else
	{
		return;
	}

	// Check if other actor is semantically annotated
	const uint32 OtherId = OtherActor->GetUniqueID();
	const FString OtherSemId = FSLMappings::GetInstance()->GetSemanticId(OtherId);
	const FString OtherSemClass = FSLMappings::GetInstance()->GetSemanticClass(OtherId);
	if (OtherSemId.IsEmpty() || OtherSemClass.IsEmpty())
	{
		return;
	}

	// Set by default that the other component is not an semantic overlap area (this will be checked further)
	bool bOtherIsASemanticOverlapArea = false;
	// Get the time of the event in second
	float EndTime = GetWorld()->GetTimeSeconds();

	// If both areas are trigger areas, they will both concurrently trigger overlap events.
	// To avoid this we consistently ignore one trigger event. This is chosen using
	// the unique ids of the overlapping actors (GetUniqueID), we compare the two values 
	// and consistently pick the event with a given (larger or smaller) value.
	// This allows us to be in sync with the overlap end event 
	// since the unique ids and the rule of ignoring the one event will not change
	if (OtherComp->IsA(USLOverlapArea::StaticClass()))
	{
		// Other is a semantic overlap component
		bOtherIsASemanticOverlapArea = true;

		if (OtherId > OwnerId)
		{
			// Broadcast end of semantic overlap event
			FSLOverlapResult SemanticOverlapResult(OtherId, OtherSemId, OtherSemClass, EndTime,
				bOtherIsASemanticOverlapArea, OtherSMAct, OtherSMComp);
			OnEndSLOverlap.Broadcast(SemanticOverlapResult);
			return;
		}
	}

	// Broadcast end of semantic overlap event
	FSLOverlapResult SemanticOverlapResult(OtherId, OtherSemId, OtherSemClass, EndTime,
		bOtherIsASemanticOverlapArea, OtherSMAct, OtherSMComp);
	OnEndSLOverlap.Broadcast(SemanticOverlapResult);
}

//// Called on hit event
//void USLContactTrigger::OnHit(UPrimitiveComponent* HitComponent,
//	AActor* OtherActor,
//	UPrimitiveComponent* OtherComp,
//	FVector NormalImpulse,
//	const FHitResult& Hit)
//{
//	
//	UE_LOG(LogTemp, Warning, TEXT(">> %s::%d This-Other: %s-%s TS: %f"),
//		TEXT(__FUNCTION__), __LINE__,
//		*HitComponent->GetOwner()->GetName(), *OtherActor->GetName(),
//		GetWorld()->GetTimeSeconds());
//
//	UE_LOG(LogTemp, Warning, TEXT(">> %s::%d \n \t HIT: %s \n \t Normal Impulse: %s \n **"),
//		TEXT(__FUNCTION__), __LINE__, *Hit.ToString(), *NormalImpulse.ToString());
//}