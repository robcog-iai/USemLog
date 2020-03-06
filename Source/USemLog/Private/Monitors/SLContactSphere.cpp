// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Monitors/SLContactSphere.h"
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

	bLogSupportedByEvents = true;
	
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
void USLContactSphere::Init(bool bInLogSupportedByEvents)
{
	if (!bIsInit)
	{
		bLogSupportedByEvents = bInLogSupportedByEvents;
		
		// Important, set the interface pointers
		if(!InitInterface(this, GetWorld()))
		{
			return;
		}

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
		if(bLogSupportedByEvents)
		{
			StartSupportedByUpdateCheck();
		}
		
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

#if WITH_EDITOR
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
#endif // WITH_EDITOR
