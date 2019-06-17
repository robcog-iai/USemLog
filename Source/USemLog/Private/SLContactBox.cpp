// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLContactBox.h"
#include "SLEntitiesManager.h"
#include "Animation/SkeletalMeshActor.h"

// UUTils
#include "Tags.h"
#include "Ids.h"

// Default constructor
USLContactBox::USLContactBox()
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
	BoxExtentScaleFactor = 1.03f;
	BoxExtentMin = 0.25f;
	BoxExtentMax = 1.f;

	// Mimics a button
	bReCalcShapeButton = false;
#endif // WITH_EDITOR
}

// Destructor
USLContactBox::~USLContactBox()
{
	if (!bIsFinished)
	{
		Finish(true);
	}
}

// Called at level startup
void USLContactBox::BeginPlay()
{
	Super::BeginPlay();

	if (bStartAtBeginPlay)
	{
		Init();
		Start();
	}
}

// Called when actor removed from game or game ended
void USLContactBox::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (!bIsFinished)
	{
		Finish();
	}
}

// Setup pointers to outer, check if semantically annotated
void USLContactBox::Init()
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
void USLContactBox::Start()
{
	if (!bIsStarted && bIsInit)
	{
		// Enable overlap events
		SetGenerateOverlapEvents(true);

		// Broadcast currently overlapping components
		USLContactBox::TriggerInitialOverlaps();

		// Bind future overlapping event delegates
		OnComponentBeginOverlap.AddDynamic(this, &USLContactBox::OnOverlapBegin);
		OnComponentEndOverlap.AddDynamic(this, &USLContactBox::OnOverlapEnd);

		// Mark as started
		bIsStarted = true;
	}
}

// Stop publishing overlap events
void USLContactBox::Finish(bool bForced)
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
void USLContactBox::PostInitProperties()
{
	Super::PostInitProperties();

	if (!USLContactBox::LoadShapeBounds())
	{
		USLContactBox::CalcShapeBounds();
		USLContactBox::StoreShapeBounds();
	}

	// Set bounds visual corresponding color 
	USLContactBox::UpdateVisualColor();
}

// Called when a property is changed in the editor
void USLContactBox::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property and member names
	FName PropertyName = PropertyChangedEvent.GetPropertyName();

	FName MemberPropertyName = (PropertyChangedEvent.MemberProperty != NULL) ? 
		PropertyChangedEvent.MemberProperty->GetFName() : NAME_None;

	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(USLContactBox, BoxExtent))
	{
		if (PropertyName == FName("X"))
		{
			FTags::AddKeyValuePair(GetOuter(), TagTypeName, "ExtX",
				FString::SanitizeFloat(BoxExtent.X));
		}
		else if (PropertyName == FName("Y"))
		{
			FTags::AddKeyValuePair(GetOuter(), TagTypeName, "ExtY",
				FString::SanitizeFloat(BoxExtent.Y));
		}
		else if (PropertyName == FName("Z"))
		{
			FTags::AddKeyValuePair(GetOuter(), TagTypeName, "ExtY",
				FString::SanitizeFloat(BoxExtent.Y));
		}
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(USLContactBox, RelativeLocation))
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
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(USLContactBox, RelativeRotation))
	{
		const FQuat RelQuat = GetRelativeTransform().GetRotation();
		TMap<FString, FString> KeyValMap;
		KeyValMap.Add("QuatW", FString::SanitizeFloat(RelQuat.W));
		KeyValMap.Add("QuatX", FString::SanitizeFloat(RelQuat.X));
		KeyValMap.Add("QuatY", FString::SanitizeFloat(RelQuat.Y));
		KeyValMap.Add("QuatZ", FString::SanitizeFloat(RelQuat.Z));
		FTags::AddKeyValuePairs(GetOuter(), TagTypeName, KeyValMap);
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(USLContactBox, bReCalcShapeButton))
	{
		CalcShapeBounds();
		bReCalcShapeButton = false;
	}
}

// Called when this component is moved in the editor
void USLContactBox::PostEditComponentMove(bool bFinished)
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

	FTags::AddKeyValuePairs(GetOuter(), TagTypeName, KeyValMap);
}

// Read values from tags
bool USLContactBox::LoadShapeBounds()
{
	TMap<FString, FString> TagKeyValMap = 
		FTags::GetKeyValuePairs(GetOuter(), TagTypeName);

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
bool USLContactBox::CalcShapeBounds()
{
	// Get the static mesh component
	if (AStaticMeshActor* OuterAsSMAct = Cast<AStaticMeshActor>(GetOuter()))
	{
		if (UStaticMeshComponent* SMComp = OuterAsSMAct->GetStaticMeshComponent())
		{
			// Apply smallest box extent
			FVector BBMin;
			FVector BBMax;
			SMComp->GetLocalBounds(BBMin, BBMax);
			const FVector Ext = (BBMax - BBMin) * 0.5f;
			const FVector ScaledExt = Ext * BoxExtentScaleFactor;
			SetBoxExtent(ScaledExt.BoundToBox(Ext + BoxExtentMin, Ext + BoxExtentMax));

			// Apply its location
			//FTransform BoundsTransf(FQuat::Identity, SMComp->Bounds.Origin);
			//BoundsTransf.SetToRelativeTransform(SMComp->GetComponentTransform());
			//SetRelativeTransform(BoundsTransf);
			return true;
		}
	}
	return false;
}

// Save values to tags
bool USLContactBox::StoreShapeBounds()
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
	
	return FTags::AddKeyValuePairs(GetOuter(), TagTypeName, KeyValMap);
}

// Update bounds visual (red/green -- parent is not/is semantically annotated)
void USLContactBox::UpdateVisualColor()
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
void USLContactBox::TriggerInitialOverlaps()
{
	// If objects are already overlapping at begin play, they will not be triggered
	// Here we do a manual overlap check and forward them to OnOverlapBegin
	TSet<UPrimitiveComponent*> CurrOverlappingComponents;
	GetOverlappingComponents(CurrOverlappingComponents);
	FHitResult Dummy;
	for (const auto& CompItr : CurrOverlappingComponents)
	{
		USLContactBox::OnOverlapBegin(
			this, CompItr->GetOwner(), CompItr, 0, false, Dummy);
	}
}

// Called on overlap begin events
void USLContactBox::OnOverlapBegin(UPrimitiveComponent* OverlappedComp,
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
	else if (USLContactBox* OtherContactTrigger = Cast<USLContactBox>(OtherComp))
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
void USLContactBox::OnOverlapEnd(UPrimitiveComponent* OverlappedComp,
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
	else if (USLContactBox* OtherContactTrigger = Cast<USLContactBox>(OtherComp))
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

