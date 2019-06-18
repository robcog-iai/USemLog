// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLContactSphere.h"
#include "SLEntitiesManager.h"
#include "Animation/SkeletalMeshActor.h"

// UUTils
#include "Tags.h"
#include "Ids.h"

// Default constructor
USLContactSphere::USLContactSphere()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// State flags
	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;

	// Is started by the event logger
	bStartAtBeginPlay = false;

#if WITH_EDITOR
	// Box extent scale
	SphereScaleFactor = 1.25f;
	SphereMinSize = 0.25f;
	SphereMaxSize = 1.f;

	// Mimics a button
	bReCalcShapeButton = false;
#endif // WITH_EDITOR
}

// Destructor
USLContactSphere::~USLContactSphere()
{
	if (!bIsFinished)
	{
		Finish(true);
	}
}

// Called at level startup
void USLContactSphere::BeginPlay()
{
	Super::BeginPlay();

	if (bStartAtBeginPlay)
	{
		Init();
		Start();
	}
}

// Called when actor removed from game or game ended
void USLContactSphere::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (!bIsFinished)
	{
		Finish();
	}
}

// Setup pointers to outer, check if semantically annotated
void USLContactSphere::Init()
{
	if (!bIsInit)
	{
		// Make sure the semantic entities are set
		if (!FSLEntitiesManager::GetInstance()->IsInit())
		{
			FSLEntitiesManager::GetInstance()->Init(GetWorld());
		}

		// TODO add case where owner is a component (e.g. instead of using get owner, use outer)
		// Make sure owner is a valid semantic item
		SemanticOwner = FSLEntitiesManager::GetInstance()->GetEntity(GetOwner());
		if (!SemanticOwner.IsSet())
		{
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
void USLContactSphere::Start()
{
	if (!bIsStarted && bIsInit)
	{
		// Enable overlap events
		SetGenerateOverlapEvents(true);

		// Broadcast currently overlapping components
		USLContactSphere::TriggerInitialOverlaps();

		// Bind future overlapping event delegates
		OnComponentBeginOverlap.AddDynamic(this, &USLContactSphere::OnOverlapBegin);
		OnComponentEndOverlap.AddDynamic(this, &USLContactSphere::OnOverlapEnd);

		// Mark as started
		bIsStarted = true;
	}
}

// Stop publishing overlap events
void USLContactSphere::Finish(bool bForced)
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{
		// Disable overlap events
		SetGenerateOverlapEvents(false);

		// Mark as finished
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
	}
}

#if WITH_EDITOR
// Called after the C++ constructor and after the properties have been initialized
void USLContactSphere::PostInitProperties()
{
	Super::PostInitProperties();

	if (!USLContactSphere::LoadShapeBounds())
	{
		USLContactSphere::CalcShapeBounds();
		USLContactSphere::StoreShapeBounds();
	}

	// Set bounds visual corresponding color 
	USLContactSphere::UpdateVisualColor();
}

// Called when a property is changed in the editor
void USLContactSphere::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property and member names
	FName PropertyName = PropertyChangedEvent.GetPropertyName();

	FName MemberPropertyName = (PropertyChangedEvent.MemberProperty != NULL) ?
		PropertyChangedEvent.MemberProperty->GetFName() : NAME_None;

	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(USLContactSphere, SphereRadius))
	{
			FTags::AddKeyValuePair(GetOuter(), TagTypeName, "Radius",
				FString::SanitizeFloat(SphereRadius));
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(USLContactSphere, RelativeLocation))
	{
		if (PropertyName == FName("X"))
		{
			FTags::AddKeyValuePair(GetOuter(), TagTypeName, "LocX",
				FString::SanitizeFloat(RelativeLocation.X));
		}
		else if (PropertyName == FName("Y"))
		{
			FTags::AddKeyValuePair(GetOuter(), TagTypeName, "LocY",
				FString::SanitizeFloat(RelativeLocation.Y));
		}
		else if (PropertyName == FName("Z"))
		{
			FTags::AddKeyValuePair(GetOuter(), TagTypeName, "LocZ",
				FString::SanitizeFloat(RelativeLocation.Y));
		}
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(USLContactSphere, bReCalcShapeButton))
	{
		CalcShapeBounds();
		bReCalcShapeButton = false;
	}
}

// Called when this component is moved in the editor
void USLContactSphere::PostEditComponentMove(bool bFinished)
{
	// Update tags with the new transform
	const FVector RelLoc = GetRelativeTransform().GetLocation();

	TMap<FString, FString> KeyValMap;

	KeyValMap.Add("LocX", FString::SanitizeFloat(RelLoc.X));
	KeyValMap.Add("LocY", FString::SanitizeFloat(RelLoc.Y));
	KeyValMap.Add("LocZ", FString::SanitizeFloat(RelLoc.Z));

	FTags::AddKeyValuePairs(GetOuter(), TagTypeName, KeyValMap);
}

// Read values from tags
bool USLContactSphere::LoadShapeBounds()
{
	TMap<FString, FString> TagKeyValMap =
		FTags::GetKeyValuePairs(GetOuter(), TagTypeName);

	if (TagKeyValMap.Num() == 0) { return false; }

	float Radius;
	if (FString* ValPtr = TagKeyValMap.Find("Radius")) { Radius = FCString::Atof(**ValPtr); }
	else { return false; }

	FVector RelLoc;
	if (FString* ValPtr = TagKeyValMap.Find("LocX")) { RelLoc.X = FCString::Atof(**ValPtr); }
	else { return false; }
	if (FString* ValPtr = TagKeyValMap.Find("LocY")) { RelLoc.Y = FCString::Atof(**ValPtr); }
	else { return false; }
	if (FString* ValPtr = TagKeyValMap.Find("LocZ")) { RelLoc.Z = FCString::Atof(**ValPtr); }
	else { return false; }

	SetSphereRadius(Radius);
	SetRelativeLocation(RelLoc);
	return true;
}

// Calculate trigger area size
bool USLContactSphere::CalcShapeBounds()
{
	// Get the static mesh component
	if (AStaticMeshActor* OuterAsSMAct = Cast<AStaticMeshActor>(GetOuter()))
	{
		if (UStaticMeshComponent* SMComp = OuterAsSMAct->GetStaticMeshComponent())
		{
			// Apply smallest sphere size
			FVector BBMin;
			FVector BBMax;
			SMComp->GetLocalBounds(BBMin, BBMax);
			FBoxSphereBounds BoxSphere(FBox(BBMin*0.5f, BBMax*0.5f));
			float Radius = FMath::Clamp(BoxSphere.SphereRadius * SphereScaleFactor,
				BoxSphere.SphereRadius + SphereMinSize, BoxSphere.SphereRadius + SphereMaxSize);

			SetSphereRadius(Radius);
			return true;
		}
	}
	return false;
}

// Save values to tags
bool USLContactSphere::StoreShapeBounds()
{
	const FVector RelLoc = GetRelativeTransform().GetLocation();

	TMap<FString, FString> KeyValMap;

	KeyValMap.Add("Radius", FString::SanitizeFloat(SphereRadius));

	KeyValMap.Add("LocX", FString::SanitizeFloat(RelLoc.X));
	KeyValMap.Add("LocY", FString::SanitizeFloat(RelLoc.Y));
	KeyValMap.Add("LocZ", FString::SanitizeFloat(RelLoc.Z));

	return FTags::AddKeyValuePairs(GetOuter(), TagTypeName, KeyValMap);
}

// Update bounds visual (red/green -- parent is not/is semantically annotated)
void USLContactSphere::UpdateVisualColor()
{
	// Set the default color of the shape
	if (FTags::HasKey(GetOuter(), "SemLog", "Class"))
	{
		if (ShapeColor != FColor::Green)
		{
			ShapeColor = FColor::Green;
			MarkRenderStateDirty();
		}
	}
	else
	{
		if (ShapeColor != FColor::Red)
		{
			ShapeColor = FColor::Red;
			MarkRenderStateDirty();
		}
	}
}

#endif // WITH_EDITOR

// Publish currently overlapping components
void USLContactSphere::TriggerInitialOverlaps()
{
	// If objects are already overlapping at begin play, they will not be triggered
	// Here we do a manual overlap check and forward them to OnOverlapBegin
	TSet<UPrimitiveComponent*> CurrOverlappingComponents;
	GetOverlappingComponents(CurrOverlappingComponents);
	FHitResult Dummy;
	for (const auto& CompItr : CurrOverlappingComponents)
	{
		USLContactSphere::OnOverlapBegin(
			this, CompItr->GetOwner(), CompItr, 0, false, Dummy);
	}
}

// Called on overlap begin events
void USLContactSphere::OnOverlapBegin(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	// Ignore self overlaps (area with static mesh)
	if (OtherActor == GetOwner())
	{
		return;
	}

	// Check if the component or its outer is semantically annotated
	FSLEntity OtherItem = FSLEntitiesManager::GetInstance()->GetEntity(OtherComp);
	if (!OtherItem.IsSet())
	{
		// Other not valid, check if its outer is semantically annotated
		OtherItem = FSLEntitiesManager::GetInstance()->GetEntity(OtherComp->GetOuter());
		if (!OtherItem.IsSet())
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
		FSLContactResult SemanticOverlapResult(SemanticOwner, OtherItem,
			StartTime, false, OwnerMeshComp, OtherAsMeshComp);
		OnBeginSLContact.Broadcast(SemanticOverlapResult);
	}
	else if (USLContactSphere* OtherContactTrigger = Cast<USLContactSphere>(OtherComp))
	{
		// If both areas are trigger areas, they will both concurrently trigger overlap events.
		// To avoid this we consistently ignore one trigger event. This is chosen using
		// the unique ids of the overlapping actors (GetUniqueID), we compare the two values 
		// and consistently pick the event with a given (larger or smaller) value.
		// This allows us to be in sync with the overlap end event 
		// since the unique ids and the rule of ignoring the one event will not change
		// Filter out one of the trigger areas (compare unique ids)
		if (OtherItem.Obj->GetUniqueID() > SemanticOwner.Obj->GetUniqueID())
		{
			// Broadcast begin of semantic overlap event
			FSLContactResult SemanticOverlapResult(SemanticOwner, OtherItem,
				StartTime, true, OwnerMeshComp, OtherContactTrigger->OwnerMeshComp);
			OnBeginSLContact.Broadcast(SemanticOverlapResult);
		}
	}
}

// Called on overlap end events
void USLContactSphere::OnOverlapEnd(UPrimitiveComponent* OverlappedComp,
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
	FSLEntity OtherItem = FSLEntitiesManager::GetInstance()->GetEntity(OtherComp);
	if (!OtherItem.IsSet())
	{
		// Other not valid, check if its outer is semantically annotated
		OtherItem = FSLEntitiesManager::GetInstance()->GetEntity(OtherComp->GetOuter());
		if (!OtherItem.IsSet())
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
		OnEndSLContact.Broadcast(SemanticOwner.Obj, OtherItem.Obj, EndTime);
	}
	else if (USLContactSphere* OtherContactTrigger = Cast<USLContactSphere>(OtherComp))
	{
		// If both areas are trigger areas, they will both concurrently trigger overlap events.
		// To avoid this we consistently ignore one trigger event. This is chosen using
		// the unique ids of the overlapping actors (GetUniqueID), we compare the two values 
		// and consistently pick the event with a given (larger or smaller) value.
		// This allows us to be in sync with the overlap end event 
		// since the unique ids and the rule of ignoring the one event will not change
		// Filter out one of the trigger areas (compare unique ids)
		if (OtherItem.Obj->GetUniqueID() > SemanticOwner.Obj->GetUniqueID())
		{
			// Broadcast end of semantic overlap event
			OnEndSLContact.Broadcast(SemanticOwner.Obj, OtherItem.Obj, EndTime);
		}
	}
}

