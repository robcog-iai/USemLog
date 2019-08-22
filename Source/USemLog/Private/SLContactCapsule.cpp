// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLContactCapsule.h"
#include "SLEntitiesManager.h"
#include "Animation/SkeletalMeshActor.h"

// UUTils
#include "Tags.h"
#include "Ids.h"

// Default constructor
USLContactCapsule::USLContactCapsule()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// State flags
	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;
	
	bLogSupportedByEvents = true;

	// Is started by the event logger
	bStartAtBeginPlay = false;

#if WITH_EDITOR
	// Box extent scale
	CapsuleScaleFactor = 1.03f;
	CapsuleMinSize = 0.25f;
	CapsuleMaxSize = 1.f;

	// Mimics a button
	bReCalcShapeButton = false;
#endif // WITH_EDITOR
}

// Destructor
USLContactCapsule::~USLContactCapsule()
{
	if (!bIsFinished)
	{
		Finish(true);
	}
}

// Called at level startup
void USLContactCapsule::BeginPlay()
{
	Super::BeginPlay();

	if (bStartAtBeginPlay)
	{
		Init();
		Start();
	}
}

// Called when actor removed from game or game ended
void USLContactCapsule::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (!bIsFinished)
	{
		Finish();
	}
}

// Setup pointers to outer, check if semantically annotated
void USLContactCapsule::Init(bool bInLogSupportedByEvents)
{
	if (!bIsInit)
	{
		bLogSupportedByEvents = bInLogSupportedByEvents;
		
		// Important, set the interface pointers
		ShapeComponent = this;
		World = GetWorld();

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
void USLContactCapsule::Start()
{
	if (!bIsStarted && bIsInit)
	{
		if(bLogSupportedByEvents)
		{
			StartSupportedByUpdateCheck();
		}
		
		// Enable overlap events
		SetGenerateOverlapEvents(true);

		// Broadcast currently overlapping components
		TriggerInitialOverlaps();

		// Bind future overlapping event delegates
		OnComponentBeginOverlap.AddDynamic(this, &USLContactCapsule::OnOverlapBegin);
		OnComponentEndOverlap.AddDynamic(this, &USLContactCapsule::OnOverlapEnd);

		// Mark as started
		bIsStarted = true;
	}
}

#ifdef WITH_EDITOR
// Update bounds visual (red/green -- parent is not/is semantically annotated)
void USLContactCapsule::UpdateVisualColor()
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

// Called after the C++ constructor and after the properties have been initialized
void USLContactCapsule::PostInitProperties()
{
	Super::PostInitProperties();

	if (!USLContactCapsule::LoadShapeBounds())
	{
		USLContactCapsule::CalcShapeBounds();
		USLContactCapsule::StoreShapeBounds();
	}

	// Set bounds visual corresponding color 
	USLContactCapsule::UpdateVisualColor();
}

// Called when a property is changed in the editor
void USLContactCapsule::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property and member names
	FName PropertyName = PropertyChangedEvent.GetPropertyName();

	FName MemberPropertyName = (PropertyChangedEvent.MemberProperty != NULL) ?
		PropertyChangedEvent.MemberProperty->GetFName() : NAME_None;

	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(USLContactCapsule, CapsuleRadius))
	{
		FTags::AddKeyValuePair(GetOuter(), TagTypeName, "Radius",
			FString::SanitizeFloat(CapsuleRadius));
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(USLContactCapsule, CapsuleHalfHeight))
	{
		FTags::AddKeyValuePair(GetOuter(), TagTypeName, "HalfHeight",
			FString::SanitizeFloat(CapsuleHalfHeight));
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(USLContactCapsule, RelativeLocation))
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
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(USLContactCapsule, RelativeRotation))
	{
		const FQuat RelQuat = GetRelativeTransform().GetRotation();
		TMap<FString, FString> KeyValMap;
		KeyValMap.Add("QuatW", FString::SanitizeFloat(RelQuat.W));
		KeyValMap.Add("QuatX", FString::SanitizeFloat(RelQuat.X));
		KeyValMap.Add("QuatY", FString::SanitizeFloat(RelQuat.Y));
		KeyValMap.Add("QuatZ", FString::SanitizeFloat(RelQuat.Z));
		FTags::AddKeyValuePairs(GetOuter(), TagTypeName, KeyValMap);
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(USLContactCapsule, bReCalcShapeButton))
	{
		CalcShapeBounds();
		bReCalcShapeButton = false;
	}
}

// Called when this component is moved in the editor
void USLContactCapsule::PostEditComponentMove(bool bFinished)
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
bool USLContactCapsule::LoadShapeBounds()
{
	TMap<FString, FString> TagKeyValMap =
		FTags::GetKeyValuePairs(GetOuter(), TagTypeName);

	if (TagKeyValMap.Num() == 0) { return false; }

	float Radius;
	if (FString* ValPtr = TagKeyValMap.Find("Radius")) { Radius = FCString::Atof(**ValPtr); }
	else { return false; }

	float HalfHeight;
	if (FString* ValPtr = TagKeyValMap.Find("HalfHeight")) { HalfHeight = FCString::Atof(**ValPtr); }
	else { return false; }

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

	SetCapsuleSize(Radius, HalfHeight);
	SetRelativeTransform(FTransform(RelQuat, RelLoc));
	return true;
}

// Calculate trigger area size
bool USLContactCapsule::CalcShapeBounds()
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

			// TODO fit capsule in the box
			FBoxSphereBounds BoxSphere(FBox(BBMin*0.5f, BBMax*0.5f));
			float Radius = FMath::Clamp(BoxSphere.SphereRadius * CapsuleScaleFactor,
				BoxSphere.SphereRadius + CapsuleMinSize, BoxSphere.SphereRadius + CapsuleMaxSize);
			SetCapsuleSize(Radius, Radius * 2.f);

			return true;
		}
	}
	return false;
}

// Save values to tags
bool USLContactCapsule::StoreShapeBounds()
{
	const FTransform RelTransf = GetRelativeTransform();
	const FVector RelLoc = RelTransf.GetLocation();
	const FQuat RelQuat = RelTransf.GetRotation();

	TMap<FString, FString> KeyValMap;

	KeyValMap.Add("Radius", FString::SanitizeFloat(CapsuleRadius));
	KeyValMap.Add("HalfHeight", FString::SanitizeFloat(CapsuleHalfHeight));

	KeyValMap.Add("LocX", FString::SanitizeFloat(RelLoc.X));
	KeyValMap.Add("LocY", FString::SanitizeFloat(RelLoc.Y));
	KeyValMap.Add("LocZ", FString::SanitizeFloat(RelLoc.Z));

	KeyValMap.Add("QuatW", FString::SanitizeFloat(RelQuat.W));
	KeyValMap.Add("QuatX", FString::SanitizeFloat(RelQuat.X));
	KeyValMap.Add("QuatY", FString::SanitizeFloat(RelQuat.Y));
	KeyValMap.Add("QuatZ", FString::SanitizeFloat(RelQuat.Z));

	return FTags::AddKeyValuePairs(GetOuter(), TagTypeName, KeyValMap);
}
#endif // WITH_EDITOR
