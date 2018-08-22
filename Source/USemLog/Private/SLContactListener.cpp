// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLContactListener.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "SLContentSingleton.h"
#include "SLContactPoolSingleton.h"
#include "DrawDebugHelpers.h"

// UUTils
#include "Tags.h"

#define SL_COLL_SCALE_FACTOR 1.04f
#define SL_COLL_TAGTYPE "SemLogColl"

// Default constructor
USLContactListener::USLContactListener()
{
}

// Called at level startup
void USLContactListener::BeginPlay()
{
	Super::BeginPlay();

	// Set the outer (owner) mesh component
	OuterMeshComp = GetOuterMesh();

	// Set the unique Id of the outer
	OuterId = GetOuterId();

	// Make sure there are no overlap events on the mesh as well
	// (these will be calculated on the contact listener)
	// TODO this will cause problems with grasping objects
	OuterMeshComp->SetGenerateOverlapEvents(false);

	if (OuterMeshComp && !OuterId.IsEmpty())
	{
		// Bind event delegates
		OnComponentBeginOverlap.AddDynamic(this, &USLContactListener::OnOverlapBegin);
		OnComponentEndOverlap.AddDynamic(this, &USLContactListener::OnOverlapEnd);
		//OnComponentHit.AddDynamic(this, &USLContactListener::OnHit);
	}

	// Register listener to the pool
	USLContactPoolSingleton::GetInstance()->Register(this);
}

// Called after the C++ constructor and after the properties have been initialized
void  USLContactListener::PostInitProperties()
{
	Super::PostInitProperties();

	if (!LoadStoredParameters())
	{
		ComputeAndStoreParameters(GetOuterMesh());
	}
	ShapeColor = FColor::Blue;
}

#if WITH_EDITOR
// Called when a property is changed in the editor
void USLContactListener::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property and member names
	FName PropertyName = PropertyChangedEvent.GetPropertyName();

	FName MemberPropertyName = (PropertyChangedEvent.MemberProperty != NULL) ? 
		PropertyChangedEvent.MemberProperty->GetFName() : NAME_None;

	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(USLContactListener, BoxExtent))
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
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(USLContactListener, RelativeLocation))
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
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(USLContactListener, RelativeRotation))
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
void USLContactListener::PostEditComponentMove(bool bFinished)
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
bool USLContactListener::LoadStoredParameters()
{
	TMap<FString, FString> TagKeyValMap = 
		FTags::GetKeyValuePairs(GetOuter(), SL_COLL_TAGTYPE);

	if (TagKeyValMap.Num() == 0){return false;}

	if (FString* ValPtr = TagKeyValMap.Find("ExtX")) { BoxExtent.X = FCString::Atof(**ValPtr); }
	else { return false; }
	if (FString* ValPtr = TagKeyValMap.Find("ExtY")) { BoxExtent.Y = FCString::Atof(**ValPtr); }
	else { return false; }
	if (FString* ValPtr = TagKeyValMap.Find("ExtZ")) { BoxExtent.Z = FCString::Atof(**ValPtr); }
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

	SetRelativeTransform(FTransform(RelQuat, RelLoc));
	return true;
}

// Save values to tags
bool USLContactListener::ComputeAndStoreParameters(UStaticMeshComponent* SMComp)
{
	if (SMComp == nullptr)
	{
		return false;
	}

	// Apply parameters to the contact listener area
	SetBoxExtent(SMComp->Bounds.BoxExtent * SL_COLL_SCALE_FACTOR);
	// Apply its location
	FTransform BoundsTransf(FQuat::Identity, SMComp->Bounds.Origin);
	BoundsTransf.SetToRelativeTransform(SMComp->GetComponentTransform());
	SetRelativeTransform(BoundsTransf);

	// Store calculated parameters (size / rotation / location)
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

// Get the outer (owner) mesh component
UStaticMeshComponent* USLContactListener::GetOuterMesh()
{
	if (AStaticMeshActor* OuterAsSMAct = Cast<AStaticMeshActor>(GetOuter()))
	{
		return OuterAsSMAct->GetStaticMeshComponent();
	}
	else
	{
		return Cast<UStaticMeshComponent>(GetOuter());
	}
}

// Get Id of outer (owner)
FString USLContactListener::GetOuterId() const
{
	return FTags::GetValue(GetOuter(), "SemLog", "Id");
}

// Called on overlap begin events
void USLContactListener::OnOverlapBegin(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	// TODO use USLContentSingleton
	// Get the id of the other component
	FString OtherId = FTags::GetValue(OtherActor, "SemLog", "Id");
	if (OtherId.IsEmpty())
	{
		OtherId = FTags::GetValue(OtherComp, "SemLog", "Id");
		// If no unique id is found, return
		if (OtherId.IsEmpty()) return;
	}

	OtherActor->GetUniqueID();
	OtherComp->GetUniqueID();

	// Check if other is a semantically annotated object
	//USLContentSingleton::

	//// If other 
	//if (Cast<USLContactListener>(OtherComp))
	//{

	//}
	UE_LOG(LogTemp, Error, TEXT("\t OVERLAP Outer=%s, this=%s <--> Other=%s || %f || OtherId=%s -- MyId=%s"),
		*GetOuter()->GetName(),
		*GetName(),
		*OtherActor->GetName(),
		GetWorld()->GetTimeSeconds(),
		*OtherId,
		*OuterId);

	//// TODO 
	//// if OtherComp is SLContactListener --> avoid two contact events triggering

	////UE_LOG(LogTemp, Error, TEXT("[%s][%d] Other=%s, Normal=%s, ImpactNormal=%s, bFromSweep=%d, Time=%f"),
	////	TEXT(__FUNCTION__), __LINE__,
	////	*OtherActor->GetName(),
	////	*SweepResult.Normal.ToString(),
	////	*SweepResult.ImpactNormal.ToString(),
	////	bFromSweep,
	////	GetOuter()->GetWorld()->GetTimeSeconds());

	////UE_LOG(LogTemp, Error, TEXT("\t\t SweepResult: %s"), *SweepResult.ToString());

	////UE_LOG(LogTemp, Error, TEXT("\t\t this->GetComponentVelocity: %s"), *GetComponentVelocity().ToString());
	////UE_LOG(LogTemp, Error, TEXT("\t\t OverlappedComp->GetComponentVelocity(): %s"), *OverlappedComp->GetComponentVelocity().ToString());
	////UE_LOG(LogTemp, Error, TEXT("\t\t OuterMeshComp->GetComponentVelocity: %s"), *OuterMeshComp->GetComponentVelocity().ToString());
	////
	////GetOuter()->GetVel
	//
	//FHitResult OutHit;
	//FCollisionShape B;
	//if (SweepComponent(OutHit,
	//	GetComponentLocation(),
	//	GetComponentLocation() + OuterMeshComp->GetComponentVelocity(),
	//	GetComponentQuat(), B, true))
	//{
	//	UE_LOG(LogTemp, Error, TEXT("\t OutHit: %s"), *OutHit.ToString());

	//	DrawDebugPoint(GetWorld(), OutHit.ImpactPoint, 10.f, FColor::Red, true);
	//	//DrawDebugPoint(GetWorld(), OutHit.Location, 10.f, FColor::Cyan, true);

	//	DrawDebugDirectionalArrow(GetWorld(), OutHit.ImpactPoint, OutHit.ImpactPoint + OutHit.ImpactNormal, 1.f, FColor::Red, true);
	//	//DrawDebugDirectionalArrow(GetWorld(), OutHit.Location, OutHit.Location + OutHit.Normal, 10.f, FColor::Cyan, true);

	//	//DrawDebugCoordinateSystem(GetWorld(),
	//	//	OutHit.Location,
	//	//	OverlappedComp->GetComponentRotation(),
	//	//	10.f,
	//	//	true);

	//	//DrawDebugLine(
	//	//	GetWorld(),
	//	//	OutHit.Location,
	//	//	OutHit.ImpactPoint,
	//	//	10.f,
	//	//	FColor::Blue,
	//	//	true,
	//	//	5);
	//}
	//
	//UE_LOG(LogTemp, Error, TEXT("\t ****"));


	
	OnBeginSemanticContact.ExecuteIfBound(OtherActor, OtherId);
}

// Called on overlap end events
void USLContactListener::OnOverlapEnd(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
}

//// Called on hit event
//void USLContactListener::OnHit(UPrimitiveComponent* HitComponent,
//	AActor* OtherActor,
//	UPrimitiveComponent* OtherComp,
//	FVector NormalImpulse,
//	const FHitResult& Hit)
//{
//	UE_LOG(LogTemp, Error, TEXT("[%s][%d] Other=%s, NormalImpulse=%s, HitNormal=%s, HitImpactNormal=%s, Hit.Distance=%f, Time=%f"),
//		TEXT(__FUNCTION__), __LINE__,
//		*OtherActor->GetName(),
//		*NormalImpulse.ToString(),
//		*Hit.Normal.ToString(),
//		*Hit.ImpactNormal.ToString(),
//		Hit.Distance,
//		GetOuter()->GetWorld()->GetTimeSeconds());
//}
