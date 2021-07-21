// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Monitors/SLContactMonitorBox.h"
#include "Individuals/SLIndividualComponent.h"
#include "Individuals/Type/SLBaseIndividual.h"
#include "Engine/StaticMeshActor.h"
#include "Animation/SkeletalMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"

// Utils
#include "Utils/SLTagIO.h"

// Default constructor
USLContactMonitorBox::USLContactMonitorBox()
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
	BoxExtentScaleFactor = 1.03f;
	BoxExtentMin = 0.25f;
	BoxExtentMax = 1.f;

	// Mimics a button
	bReCalcShapeButton = false;
#endif // WITH_EDITORONLY_DATA

	//SetCollisionParameters();
}

// Destructor
USLContactMonitorBox::~USLContactMonitorBox()
{
	if (!bIsFinished)
	{
		Finish(true);
	}
}

// Called at level startup
void USLContactMonitorBox::BeginPlay()
{
	Super::BeginPlay();
}

// Called when actor removed from game or game ended
void USLContactMonitorBox::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (!bIsFinished)
	{
		Finish();
	}
}

// Setup pointers to outer, check if semantically annotated
void USLContactMonitorBox::Init(bool bInLogSupportedByEvents)
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
		if (AStaticMeshActor* AsSMA = Cast<AStaticMeshActor>(GetOwner()))
		{			
			OwnerMeshComp = AsSMA->GetStaticMeshComponent();
		}
		else if (ASkeletalMeshActor* AsSkMA = Cast<ASkeletalMeshActor>(GetOwner()))
		{
			OwnerMeshComp = AsSkMA->GetSkeletalMeshComponent();
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
void USLContactMonitorBox::Start()
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
		OnComponentBeginOverlap.AddDynamic(this, &USLContactMonitorBox::OnOverlapBegin);
		OnComponentEndOverlap.AddDynamic(this, &USLContactMonitorBox::OnOverlapEnd);

		// Mark as started
		bIsStarted = true;
	}
}

#if WITH_EDITOR
// Update bounds visual (red/green -- parent is not/is semantically annotated)
void USLContactMonitorBox::UpdateVisualColor()
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
void USLContactMonitorBox::PostInitProperties()
{
	Super::PostInitProperties();

	//if (!USLContactMonitorBox::LoadShapeBounds())
	//{
	//	USLContactMonitorBox::CalcShapeBounds();
	//	USLContactMonitorBox::StoreShapeBounds();
	//}

	//// Set bounds visual corresponding color 
	//USLContactMonitorBox::UpdateVisualColor();
}

// Called when a property is changed in the editor
void USLContactMonitorBox::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property and member names
	FName PropertyName = PropertyChangedEvent.GetPropertyName();

	FName MemberPropertyName = (PropertyChangedEvent.MemberProperty != NULL) ? 
		PropertyChangedEvent.MemberProperty->GetFName() : NAME_None;

	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(USLContactMonitorBox, BoxExtent))
	{
		if (PropertyName == FName("X"))
		{			
			FSLTagIO::AddKVPair(GetOwner(), TagTypeName, "ExtX", FString::SanitizeFloat(BoxExtent.X));
		}
		else if (PropertyName == FName("Y"))
		{
			FSLTagIO::AddKVPair(GetOwner(), TagTypeName, "ExtY", FString::SanitizeFloat(BoxExtent.Y));
		}
		else if (PropertyName == FName("Z"))
		{
			FSLTagIO::AddKVPair(GetOwner(), TagTypeName, "ExtY", FString::SanitizeFloat(BoxExtent.Y));
		}
	}
	//else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(USLContactMonitorBox, RelativeLocation))
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
	//else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(USLContactMonitorBox, RelativeRotation))
	//{
	//	const FQuat RelQuat = GetRelativeTransform().GetRotation();
	//	FSLTagIO::AddKVPair(GetOwner(), TagTypeName, "QuatW", FString::SanitizeFloat(RelQuat.W));
	//	FSLTagIO::AddKVPair(GetOwner(), TagTypeName, "QuatX", FString::SanitizeFloat(RelQuat.X));
	//	FSLTagIO::AddKVPair(GetOwner(), TagTypeName, "QuatY", FString::SanitizeFloat(RelQuat.Y));
	//	FSLTagIO::AddKVPair(GetOwner(), TagTypeName, "QuatZ", FString::SanitizeFloat(RelQuat.Z));
	//}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(USLContactMonitorBox, bReCalcShapeButton))
	{
		CalcShapeBounds();
		bReCalcShapeButton = false;
	}
}

// Called when this component is moved in the editor
void USLContactMonitorBox::PostEditComponentMove(bool bFinished)
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
bool USLContactMonitorBox::LoadShapeBounds()
{
	TMap<FString, FString> TagKeyValMap = FSLTagIO::GetKVPairs(GetOwner(), TagTypeName);	

	if (TagKeyValMap.Num() == 0){return false;}

	FVector BoxExt;
	if (FString* ValPtr = TagKeyValMap.Find("ExtX")) { BoxExt.X = FCString::Atof(**ValPtr); }
	else { return false; }
	if (FString* ValPtr = TagKeyValMap.Find("ExtY")) { BoxExt.Y = FCString::Atof(**ValPtr); }
	else { return false; }
	if (FString* ValPtr = TagKeyValMap.Find("ExtZ")) { BoxExt.Z = FCString::Atof(**ValPtr); }
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

	SetBoxExtent(BoxExt);
	SetRelativeTransform(FTransform(RelQuat, RelLoc));
	return true;
}

// Calculate trigger area size
bool USLContactMonitorBox::CalcShapeBounds()
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
bool USLContactMonitorBox::StoreShapeBounds()
{
	const FTransform RelTransf = GetRelativeTransform();
	const FVector RelLoc = RelTransf.GetLocation();
	const FQuat RelQuat = RelTransf.GetRotation();

	FSLTagIO::AddKVPair(GetOwner(), TagTypeName, "ExtX", FString::SanitizeFloat(BoxExtent.X));
	FSLTagIO::AddKVPair(GetOwner(), TagTypeName, "ExtY", FString::SanitizeFloat(BoxExtent.Y));
	FSLTagIO::AddKVPair(GetOwner(), TagTypeName, "ExtZ", FString::SanitizeFloat(BoxExtent.Z));
	
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

void USLContactMonitorBox::SetCollisionParameters()
{
	SetCollisionProfileName("SLContact");
	//SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel4);
	//SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	//SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Overlap);
}