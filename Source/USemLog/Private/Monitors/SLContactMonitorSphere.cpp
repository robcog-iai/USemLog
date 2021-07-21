// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Monitors/SLContactMonitorSphere.h"
#include "Individuals/SLIndividualComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Animation/SkeletalMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"

// Utils
#include "Utils/SLTagIO.h"

// Default constructor
USLContactMonitorSphere::USLContactMonitorSphere()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// State flags
	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;

	bLogSupportedByEvents = true;
	
	OwnerIndividualComponent = nullptr;

#if WITH_EDITORONLY_DATA
	// Box extent scale
	SphereScaleFactor = 1.25f;
	SphereMinSize = 0.25f;
	SphereMaxSize = 1.f;

	// Mimics a button
	bReCalcShapeButton = false;
#endif // WITH_EDITORONLY_DATA

	SetCollisionParameters();
}

// Destructor
USLContactMonitorSphere::~USLContactMonitorSphere()
{
	if (!bIsFinished)
	{
		Finish(true);
	}
}

// Called at level startup
void USLContactMonitorSphere::BeginPlay()
{
	Super::BeginPlay();
}

// Called when actor removed from game or game ended
void USLContactMonitorSphere::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (!bIsFinished)
	{
		Finish();
	}
}

// Setup pointers to outer, check if semantically annotated
void USLContactMonitorSphere::Init(bool bInLogSupportedByEvents)
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
			OwnerIndividualComponent = CastChecked<USLIndividualComponent>(AC);
			if (!OwnerIndividualComponent->IsLoaded())
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d %s's individual component is not loaded.."), *FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
				return;
			}

			// Set the individual object
			OwnerIndividualObject = OwnerIndividualComponent->GetIndividualObject();
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no individual component.."), *FString(__FUNCTION__), __LINE__, *GetOwner()->GetName());
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

			//UE_LOG(LogTemp, Log, TEXT("%s::%d Succesffully init %s"), *FString(__FUNCTION__), __LINE__, *GetFullName());
		}
	}
}

// Start overlap events, trigger currently overlapping objects
void USLContactMonitorSphere::Start()
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
		OnComponentBeginOverlap.AddDynamic(this, &USLContactMonitorSphere::OnOverlapBegin);
		OnComponentEndOverlap.AddDynamic(this, &USLContactMonitorSphere::OnOverlapEnd);

		// Mark as started
		bIsStarted = true;
	}
}

#if WITH_EDITOR
// Update bounds visual (red/green -- parent is not/is semantically annotated)
void USLContactMonitorSphere::UpdateVisualColor()
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
void USLContactMonitorSphere::PostInitProperties()
{
	Super::PostInitProperties();

	//if (!USLContactMonitorSphere::LoadShapeBounds())
	//{
	//	USLContactMonitorSphere::CalcShapeBounds();
	//	USLContactMonitorSphere::StoreShapeBounds();
	//}

	//// Set bounds visual corresponding color 
	//USLContactMonitorSphere::UpdateVisualColor();
}

// Called when a property is changed in the editor
void USLContactMonitorSphere::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property and member names
	FName PropertyName = PropertyChangedEvent.GetPropertyName();

	FName MemberPropertyName = (PropertyChangedEvent.MemberProperty != NULL) ?
		PropertyChangedEvent.MemberProperty->GetFName() : NAME_None;

	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(USLContactMonitorSphere, SphereRadius))
	{
		FSLTagIO::AddKVPair(GetOwner(), TagTypeName, "Radius", FString::SanitizeFloat(SphereRadius));
	}
	//else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(USLContactMonitorSphere, RelativeLocation))
	//{
	//	if (PropertyName == FName("X"))
	//	{
	//		FSLTagIO::AddKVPair(GetOwner(), TagTypeName, "LocX", FString::SanitizeFloat(RelativeLocation.X));
	//	}
	//	else if (PropertyName == FName("Y"))
	//	{
	//		FSLTagIO::AddKVPair(GetOwner(), TagTypeName, "LocY", FString::SanitizeFloat(RelativeLocation.Y));
	//	}
	//	else if (PropertyName == FName("Z"))
	//	{
	//		FSLTagIO::AddKVPair(GetOwner(), TagTypeName, "LocZ", FString::SanitizeFloat(RelativeLocation.Y));
	//	}
	//}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(USLContactMonitorSphere, bReCalcShapeButton))
	{
		CalcShapeBounds();
		bReCalcShapeButton = false;
	}
}

// Called when this component is moved in the editor
void USLContactMonitorSphere::PostEditComponentMove(bool bFinished)
{
	// Update tags with the new transform
	const FVector RelLoc = GetRelativeTransform().GetLocation();

	FSLTagIO::AddKVPair(GetOwner(), TagTypeName, "LocX", FString::SanitizeFloat(RelLoc.X));
	FSLTagIO::AddKVPair(GetOwner(), TagTypeName, "LocY", FString::SanitizeFloat(RelLoc.Y));
	FSLTagIO::AddKVPair(GetOwner(), TagTypeName, "LocZ", FString::SanitizeFloat(RelLoc.Z));
}

// Read values from tags
bool USLContactMonitorSphere::LoadShapeBounds()
{
	TMap<FString, FString> TagKeyValMap = FSLTagIO::GetKVPairs(GetOwner(), TagTypeName);

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
bool USLContactMonitorSphere::CalcShapeBounds()
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
bool USLContactMonitorSphere::StoreShapeBounds()
{
	const FVector RelLoc = GetRelativeTransform().GetLocation();

	FSLTagIO::AddKVPair(GetOwner(), TagTypeName, "Radius", FString::SanitizeFloat(SphereRadius));
	FSLTagIO::AddKVPair(GetOwner(), TagTypeName, "LocX", FString::SanitizeFloat(RelLoc.X));
	FSLTagIO::AddKVPair(GetOwner(), TagTypeName, "LocY", FString::SanitizeFloat(RelLoc.Y));
	FSLTagIO::AddKVPair(GetOwner(), TagTypeName, "LocZ", FString::SanitizeFloat(RelLoc.Z));
	return true;
}
#endif // WITH_EDITOR

// Set collision parameters such as object name and collision responses
void USLContactMonitorSphere::SetCollisionParameters()
{
	SetCollisionProfileName("SLContact");
	//SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel4);
	//SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	//SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Overlap);
}