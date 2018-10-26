// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLOverlapArea.h"
#include "SLMappings.h"
//#include "DrawDebugHelpers.h"

// UUTils
#include "Tags.h"
#include "Ids.h"

#define SL_COLL_SCALE_FACTOR 1.03f
#define SL_COLL_SCALE_SIZE 0.25f
#define SL_COLL_TAGTYPE "SemLogColl"

// Default constructor
USLOverlapArea::USLOverlapArea()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// State flags
	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;

	// TODO Start externally by default
	bStartAtBeginPlay = true;

	// Events flags
	bListenForContactEvents = true;
	bListenForSupportedByEvents = true;
}

// Destructor
USLOverlapArea::~USLOverlapArea()
{
	if (!bIsFinished)
	{
		USLOverlapArea::Finish();
	}
}

// Called at level startup
void USLOverlapArea::BeginPlay()
{
	Super::BeginPlay();

	if (bStartAtBeginPlay)
	{
		USLOverlapArea::Init();
		USLOverlapArea::Start();
	}
}

// Called when actor removed from game or game ended
void USLOverlapArea::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (!bIsFinished)
	{
		USLOverlapArea::Finish();
	}
}

// Setup pointers to outer, check if semantically annotated
void USLOverlapArea::Init()
{
	if (!bIsInit)
	{
		// TODO add case where owner is a component (e.g. instead of using get owner, use outer)
		// Make sure it has a semantic unique id and class
		OwnerId = GetOwner()->GetUniqueID();
		// Init the semantic items mappings singleton
		if (!FSLMappings::GetInstance()->IsInit())
		{
			FSLMappings::GetInstance()->LoadData(GetWorld());
		}
		OwnerSemId = FSLMappings::GetInstance()->GetSemanticId(OwnerId);
		OwnerSemClass = FSLMappings::GetInstance()->GetSemanticClass(OwnerId);
		// Check that owner is semantically annotated
		if (OwnerSemId.IsEmpty() || OwnerSemClass.IsEmpty())
		{
			// Not init
			return;
		}

		// Make sure the mesh (static/skeletal) component is valid
		if (AStaticMeshActor* CastToSMAct = Cast<AStaticMeshActor>(GetOwner()))
		{
			OwnerMeshComp = CastToSMAct->GetStaticMeshComponent();
		}
		else if (ASkeletalMeshActor* CastToSkelAct = Cast<ASkeletalMeshActor>(GetOwner()))
		{
			OwnerMeshComp = CastToSkelAct->GetSkeletalMeshComponent();
		}

		if (OwnerMeshComp)
		{
			// Make sure there are no overlap events on the mesh as well
			// (these will be calculated on the contact listener)
			// TODO this might cause problems with grasping objects
			OwnerMeshComp->SetGenerateOverlapEvents(false);

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

			// Mark as initialized
			bIsInit = true;
		}
		else
		{
			// Not init
			return;
		}
	}
}

// Start overlap events, trigger currently overlapping objects
void USLOverlapArea::Start()
{
	if (!bIsStarted && bIsInit)
	{
		// Broadcast currently overlapping components
		USLOverlapArea::TriggerInitialOverlaps();

		// Bind future overlapping event delegates
		OnComponentBeginOverlap.AddDynamic(this, &USLOverlapArea::OnOverlapBegin);
		OnComponentEndOverlap.AddDynamic(this, &USLOverlapArea::OnOverlapEnd);

		// Mark as started
		bIsStarted = true;
	}
}

// Stop publishing overlap events
void USLOverlapArea::Finish()
{
	if (bIsStarted || bIsInit)
	{
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

		// Mark as finished
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
	}
}

#if WITH_EDITOR
// Called after the C++ constructor and after the properties have been initialized
void USLOverlapArea::PostInitProperties()
{
	Super::PostInitProperties();

	if (!LoadAreaParameters())
	{
		CalculateAreaParameters();
	}
	ShapeColor = FColor::Blue;
}

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

// Read values from tags
bool USLOverlapArea::LoadAreaParameters()
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
bool USLOverlapArea::CalculateAreaParameters()
{
	// Get the static mesh component
	if (AStaticMeshActor* OuterAsSMAct = Cast<AStaticMeshActor>(GetOuter()))
	{
		UStaticMeshComponent* SMComp = OuterAsSMAct->GetStaticMeshComponent();

		// Apply parameters to the contact listener area
		//SetBoxExtent(SMComp->Bounds.BoxExtent * SL_COLL_SCALE_FACTOR);
		SetBoxExtent(SMComp->Bounds.BoxExtent + FVector(SL_COLL_SCALE_SIZE));
		// Apply its location
		FTransform BoundsTransf(FQuat::Identity, SMComp->Bounds.Origin);
		BoundsTransf.SetToRelativeTransform(SMComp->GetComponentTransform());
		SetRelativeTransform(BoundsTransf);

		// Save calculated data
		return SaveAreaParameters();
	}
	return false;
}

// Save values to tags
bool USLOverlapArea::SaveAreaParameters()
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
#endif // WITH_EDITOR

// Publish currently overlapping components
void USLOverlapArea::TriggerInitialOverlaps()
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
}


// Called on overlap begin events
void USLOverlapArea::OnOverlapBegin(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	//UE_LOG(LogTemp, Warning, TEXT(">> %s::%d Time=%f, OuterId=%ld, OuterName=%s, OtherId=%ld, OtherName=%s, ActId=%ld, ActName=%s"),
	//	TEXT(__FUNCTION__), __LINE__,
	//	GetWorld()->GetTimeSeconds(),
	//	OtherComp->GetOuter()->GetUniqueID(),
	//	*OtherComp->GetOuter()->GetName(),
	//	OtherComp->GetUniqueID(),
	//	*OtherComp->GetName(),
	//	OtherActor->GetUniqueID(),
	//	*OtherActor->GetName());

	// Ignore self overlaps (area with static mesh)
	if (OtherActor == GetOwner())
	{
		return;
	}

	// Check if the component or its outer is semantically annotated
	uint32 OtherId = OtherComp->GetUniqueID();
	FString OtherSemId = FSLMappings::GetInstance()->GetSemanticId(OtherId);
	FString OtherSemClass = FSLMappings::GetInstance()->GetSemanticClass(OtherId);
	if (OtherSemId.IsEmpty() || OtherSemClass.IsEmpty())
	{
		// Check if outer is semantically annotated
		OtherId = OtherComp->GetOuter()->GetUniqueID();
		OtherSemId = FSLMappings::GetInstance()->GetSemanticId(OtherId);
		OtherSemClass = FSLMappings::GetInstance()->GetSemanticClass(OtherId);
		if (OtherSemId.IsEmpty() || OtherSemClass.IsEmpty())
		{
			return;
		}
	}

	// Get the time of the event in second
	float StartTime = GetWorld()->GetTimeSeconds();

	// Check the type of the other component
	if (UMeshComponent* OtherAsMeshComp = Cast<UMeshComponent>(OtherComp))
	{
		// Broadcast begin of semantic overlap event
		FSLOverlapResult SemanticOverlapResult(OtherId, OtherSemId, OtherSemClass, 
			StartTime, false, OtherAsMeshComp);
		OnBeginSLOverlap.Broadcast(SemanticOverlapResult);
	}
	else if (USLOverlapArea* OtherContactTrigger = Cast<USLOverlapArea>(OtherComp))
	{
		// If both areas are trigger areas, they will both concurrently trigger overlap events.
		// To avoid this we consistently ignore one trigger event. This is chosen using
		// the unique ids of the overlapping actors (GetUniqueID), we compare the two values 
		// and consistently pick the event with a given (larger or smaller) value.
		// This allows us to be in sync with the overlap end event 
		// since the unique ids and the rule of ignoring the one event will not change
		// Filter out one of the trigger areas (compare unique ids)
		if (OtherId > OwnerId)
		{
			// Broadcast begin of semantic overlap event
			FSLOverlapResult SemanticOverlapResult(OtherId, OtherSemId, OtherSemClass,
				StartTime, true, OtherContactTrigger->OwnerMeshComp);
			OnBeginSLOverlap.Broadcast(SemanticOverlapResult);
		}
	}
}

// Called on overlap end events
void USLOverlapArea::OnOverlapEnd(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	// Ignore self overlaps (area with static mesh)
	if (OtherActor == GetOwner())
	{
		return;
	}

	// Check if the component or its outer is semantically annotated
	uint32 OtherId = OtherComp->GetUniqueID();
	FString OtherSemId = FSLMappings::GetInstance()->GetSemanticId(OtherId);
	FString OtherSemClass = FSLMappings::GetInstance()->GetSemanticClass(OtherId);
	if (OtherSemId.IsEmpty() || OtherSemClass.IsEmpty())
	{
		// Check if outer is semantically annotated
		OtherId = OtherComp->GetOuter()->GetUniqueID();
		FString OtherSemId = FSLMappings::GetInstance()->GetSemanticId(OtherId);
		FString OtherSemClass = FSLMappings::GetInstance()->GetSemanticClass(OtherId);
		if (OtherSemId.IsEmpty() || OtherSemClass.IsEmpty())
		{
			return;
		}
	}

	// Get the time of the event in second
	float EndTime = GetWorld()->GetTimeSeconds();

	// Check the type of the other component
	if (UMeshComponent* OtherAsMeshComp = Cast<UMeshComponent>(OtherComp))
	{
		// Broadcast end of semantic overlap event
		OnEndSLOverlap.Broadcast(OtherId, EndTime);
	}
	else if (USLOverlapArea* OtherContactTrigger = Cast<USLOverlapArea>(OtherComp))
	{
		// If both areas are trigger areas, they will both concurrently trigger overlap events.
		// To avoid this we consistently ignore one trigger event. This is chosen using
		// the unique ids of the overlapping actors (GetUniqueID), we compare the two values 
		// and consistently pick the event with a given (larger or smaller) value.
		// This allows us to be in sync with the overlap end event 
		// since the unique ids and the rule of ignoring the one event will not change
		// Filter out one of the trigger areas (compare unique ids)
		if (OtherId > OwnerId)
		{
			// Broadcast end of semantic overlap event
			OnEndSLOverlap.Broadcast(OtherId, EndTime);
		}
	}
}

