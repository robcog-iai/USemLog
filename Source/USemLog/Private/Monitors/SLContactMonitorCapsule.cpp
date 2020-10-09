// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Monitors/SLContactMonitorCapsule.h"
#include "Individuals/SLIndividualComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Animation/SkeletalMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"

// Utils
#include "Utils/SLTagIO.h"

// Default constructor
USLContactMonitorCapsule::USLContactMonitorCapsule()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// State flags
	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;
	
	bLogSupportedByEvents = true;

	IndividualComponent = nullptr;

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
USLContactMonitorCapsule::~USLContactMonitorCapsule()
{
	if (!bIsFinished)
	{
		Finish(true);
	}
}

// Called at level startup
void USLContactMonitorCapsule::BeginPlay()
{
	Super::BeginPlay();
}

// Called when actor removed from game or game ended
void USLContactMonitorCapsule::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (!bIsFinished)
	{
		Finish();
	}
}

// Setup pointers to outer, check if semantically annotated
void USLContactMonitorCapsule::Init(bool bInLogSupportedByEvents)
{
	if (!bIsInit)
	{
		bLogSupportedByEvents = bInLogSupportedByEvents;
		
		// Important, init interface with self
		if (!InitContactMonitorInterface(this, GetWorld()))
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Could not init %s"), *FString(__FUNCTION__), __LINE__, *GetFullName());
			return;
		}

		// Make sure the owner is semantically annotated
		if (UActorComponent* AC = GetOwner()->GetComponentByClass(USLIndividualComponent::StaticClass()))
		{
			IndividualComponent = CastChecked<USLIndividualComponent>(AC);
			if (!IndividualComponent->IsLoaded())
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d %s's individual component is not loaded.."), *FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
				return;
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no individual component.."), *FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
			return;
		}

		// Set the individual object
		IndividualObject = IndividualComponent->GetIndividualObject();

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
void USLContactMonitorCapsule::Start()
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
		OnComponentBeginOverlap.AddDynamic(this, &USLContactMonitorCapsule::OnOverlapBegin);
		OnComponentEndOverlap.AddDynamic(this, &USLContactMonitorCapsule::OnOverlapEnd);

		// Mark as started
		bIsStarted = true;
	}
}

#ifdef WITH_EDITOR
// Update bounds visual (red/green -- parent is not/is semantically annotated)
void USLContactMonitorCapsule::UpdateVisualColor()
{
	// Set the default color of the shape
	if (UActorComponent* AC = GetOwner()->GetComponentByClass(USLIndividualComponent::StaticClass()))
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
void USLContactMonitorCapsule::PostInitProperties()
{
	Super::PostInitProperties();

	//if (!USLContactMonitorCapsule::LoadShapeBounds())
	//{
	//	USLContactMonitorCapsule::CalcShapeBounds();
	//	USLContactMonitorCapsule::StoreShapeBounds();
	//}

	//// Set bounds visual corresponding color 
	//USLContactMonitorCapsule::UpdateVisualColor();
}

// Called when a property is changed in the editor
void USLContactMonitorCapsule::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property and member names
	FName PropertyName = PropertyChangedEvent.GetPropertyName();

	FName MemberPropertyName = (PropertyChangedEvent.MemberProperty != NULL) ?
		PropertyChangedEvent.MemberProperty->GetFName() : NAME_None;

	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(USLContactMonitorCapsule, CapsuleRadius))
	{
		FSLTagIO::AddKVPair(GetOwner(), TagTypeName, "Radius", FString::SanitizeFloat(CapsuleRadius));
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(USLContactMonitorCapsule, CapsuleHalfHeight))
	{
		FSLTagIO::AddKVPair(GetOwner(), TagTypeName, "HalfHeight", FString::SanitizeFloat(CapsuleHalfHeight));
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(USLContactMonitorCapsule, RelativeLocation))
	{
		if (PropertyName == FName("X"))
		{
			FSLTagIO::AddKVPair(GetOwner(), TagTypeName, "LocX", FString::SanitizeFloat(RelativeLocation.X));
		}
		else if (PropertyName == FName("Y"))
		{
			FSLTagIO::AddKVPair(GetOwner(), TagTypeName, "LocY", FString::SanitizeFloat(RelativeLocation.Y));
		}
		else if (PropertyName == FName("Z"))
		{
			FSLTagIO::AddKVPair(GetOwner(), TagTypeName, "LocZ", FString::SanitizeFloat(RelativeLocation.Y));
		}
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(USLContactMonitorCapsule, RelativeRotation))
	{
		const FQuat RelQuat = GetRelativeTransform().GetRotation();
		FSLTagIO::AddKVPair(GetOwner(), TagTypeName, "QuatW", FString::SanitizeFloat(RelQuat.W));
		FSLTagIO::AddKVPair(GetOwner(), TagTypeName, "QuatX", FString::SanitizeFloat(RelQuat.X));
		FSLTagIO::AddKVPair(GetOwner(), TagTypeName, "QuatY", FString::SanitizeFloat(RelQuat.Y));
		FSLTagIO::AddKVPair(GetOwner(), TagTypeName, "QuatZ", FString::SanitizeFloat(RelQuat.Z));
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(USLContactMonitorCapsule, bReCalcShapeButton))
	{
		CalcShapeBounds();
		bReCalcShapeButton = false;
	}
}

// Called when this component is moved in the editor
void USLContactMonitorCapsule::PostEditComponentMove(bool bFinished)
{
	// Update tags with the new transform
	const FTransform RelTransf = GetRelativeTransform();
	const FVector RelLoc = RelTransf.GetLocation();
	const FQuat RelQuat = RelTransf.GetRotation();

	FSLTagIO::AddKVPair(GetOwner(), TagTypeName, "LocX", FString::SanitizeFloat(RelLoc.X));
	FSLTagIO::AddKVPair(GetOwner(), TagTypeName, "LocY", FString::SanitizeFloat(RelLoc.Y));
	FSLTagIO::AddKVPair(GetOwner(), TagTypeName, "LocZ", FString::SanitizeFloat(RelLoc.Z));

	FSLTagIO::AddKVPair(GetOwner(), TagTypeName, "QuatW", FString::SanitizeFloat(RelQuat.W));
	FSLTagIO::AddKVPair(GetOwner(), TagTypeName, "QuatX", FString::SanitizeFloat(RelQuat.X));
	FSLTagIO::AddKVPair(GetOwner(), TagTypeName, "QuatY", FString::SanitizeFloat(RelQuat.Y));
	FSLTagIO::AddKVPair(GetOwner(), TagTypeName, "QuatZ", FString::SanitizeFloat(RelQuat.Z));
}

// Read values from tags
bool USLContactMonitorCapsule::LoadShapeBounds()
{
	TMap<FString, FString> TagKeyValMap = FSLTagIO::GetKVPairs(GetOwner(), TagTypeName);

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
bool USLContactMonitorCapsule::CalcShapeBounds()
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
bool USLContactMonitorCapsule::StoreShapeBounds()
{
	const FTransform RelTransf = GetRelativeTransform();
	const FVector RelLoc = RelTransf.GetLocation();
	const FQuat RelQuat = RelTransf.GetRotation();

	FSLTagIO::AddKVPair(GetOwner(), TagTypeName, "Radius", FString::SanitizeFloat(CapsuleRadius));
	FSLTagIO::AddKVPair(GetOwner(), TagTypeName, "HalfHeight", FString::SanitizeFloat(CapsuleHalfHeight));

	FSLTagIO::AddKVPair(GetOwner(), TagTypeName, "LocX", FString::SanitizeFloat(RelLoc.X));
	FSLTagIO::AddKVPair(GetOwner(), TagTypeName, "LocY", FString::SanitizeFloat(RelLoc.Y));
	FSLTagIO::AddKVPair(GetOwner(), TagTypeName, "LocZ", FString::SanitizeFloat(RelLoc.Z));

	FSLTagIO::AddKVPair(GetOwner(), TagTypeName, "QuatW", FString::SanitizeFloat(RelQuat.W));
	FSLTagIO::AddKVPair(GetOwner(), TagTypeName, "QuatX", FString::SanitizeFloat(RelQuat.X));
	FSLTagIO::AddKVPair(GetOwner(), TagTypeName, "QuatY", FString::SanitizeFloat(RelQuat.Y));
	FSLTagIO::AddKVPair(GetOwner(), TagTypeName, "QuatZ", FString::SanitizeFloat(RelQuat.Z));
	return true;
}
#endif // WITH_EDITOR
