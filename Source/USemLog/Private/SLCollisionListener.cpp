// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLCollisionListener.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"

// UUTils
#include "Tags.h"

#define SL_COLL_SCALE_FACTOR 1.05f

// Default constructor
USLCollisionListener::USLCollisionListener()
{
}

// Called after the C++ constructor and after the properties have been initialized
void  USLCollisionListener::PostInitProperties()
{
	Super::PostInitProperties();

	if (!LoadAndApplyParameters())
	{
		if (AStaticMeshActor* OuterAsSMAct = Cast<AStaticMeshActor>(GetOuter()))
		{
			if (UStaticMeshComponent* SMComp = OuterAsSMAct->GetStaticMeshComponent())
			{
				ApplyAndSaveParameters(SMComp);
			}
		}
		else if (UStaticMeshComponent* OuterAsSMComp = Cast<UStaticMeshComponent>(GetOuter()))
		{
			ApplyAndSaveParameters(OuterAsSMComp);
		}
	}
	ShapeColor = FColor::White;
}

#if WITH_EDITOR
// Called when a property is changed in the editor
void USLCollisionListener::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property and member names
	FName PropertyName = PropertyChangedEvent.GetPropertyName();

	FName MemberPropertyName = (PropertyChangedEvent.MemberProperty != NULL) ? 
		PropertyChangedEvent.MemberProperty->GetFName() : NAME_None;

	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(USLCollisionListener, BoxExtent))
	{
		if (PropertyName == FName("X"))
		{
			FTags::AddKeyValuePair(GetOuter(), "SemLogColl", "ExtX", FString::SanitizeFloat(BoxExtent.X));
		}
		else if (PropertyName == FName("Y"))
		{
			FTags::AddKeyValuePair(GetOuter(), "SemLogColl", "ExtY", FString::SanitizeFloat(BoxExtent.Y));
		}
		else if (PropertyName == FName("Z"))
		{
			FTags::AddKeyValuePair(GetOuter(), "SemLogColl", "ExtY", FString::SanitizeFloat(BoxExtent.Y));
		}
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(USLCollisionListener, RelativeLocation))
	{
		if (PropertyName == FName("X"))
		{
			FTags::AddKeyValuePair(GetOuter(), "SemLogColl", "LocX", FString::SanitizeFloat(RelativeLocation.X));
		}
		else if (PropertyName == FName("Y"))
		{
			FTags::AddKeyValuePair(GetOuter(), "SemLogColl", "LocY", FString::SanitizeFloat(RelativeLocation.Y));
		}
		else if (PropertyName == FName("Z"))
		{
			FTags::AddKeyValuePair(GetOuter(), "SemLogColl", "LocZ", FString::SanitizeFloat(RelativeLocation.Y));
		}
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(USLCollisionListener, RelativeRotation))
	{
		const FQuat RelQuat = GetRelativeTransform().GetRotation();
		TMap<FString, FString> KeyValMap;
		KeyValMap.Add("QuatW", FString::SanitizeFloat(RelQuat.W));
		KeyValMap.Add("QuatX", FString::SanitizeFloat(RelQuat.X));
		KeyValMap.Add("QuatY", FString::SanitizeFloat(RelQuat.Y));
		KeyValMap.Add("QuatZ", FString::SanitizeFloat(RelQuat.Z));
		FTags::AddKeyValuePairs(GetOuter(), "SemLogColl", KeyValMap);
	}
}
#endif // WITH_EDITOR

// Called when this component is moved in the editor
void USLCollisionListener::PostEditComponentMove(bool bFinished)
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

	FTags::AddKeyValuePairs(GetOuter(), "SemLogColl", KeyValMap);
}

// Read values from tags
bool USLCollisionListener::LoadAndApplyParameters()
{
	TMap<FString, FString> TagKeyValMap = 
		FTags::GetKeyValuePairs(GetOuter(), "SemLogColl");

	if (TagKeyValMap.Num() == 0){return false;}

	if (TagKeyValMap.Contains("ExtX")){BoxExtent.X = FCString::Atof(*TagKeyValMap["ExtX"]);}
	else{return false;}
	if (TagKeyValMap.Contains("ExtY")){BoxExtent.Y = FCString::Atof(*TagKeyValMap["ExtY"]);}
	else{return false;}
	if (TagKeyValMap.Contains("ExtZ")){BoxExtent.Z = FCString::Atof(*TagKeyValMap["ExtZ"]);}
	else{return false;}

	FVector RelLoc;
	if (TagKeyValMap.Contains("LocX")){RelLoc.X = FCString::Atof(*TagKeyValMap["LocX"]);}
	else{return false;}
	if (TagKeyValMap.Contains("LocY")){RelLoc.Y = FCString::Atof(*TagKeyValMap["LocY"]);}
	else{return false;}
	if (TagKeyValMap.Contains("LocZ")){	RelLoc.Z = FCString::Atof(*TagKeyValMap["LocZ"]);}
	else{return false;}
		
	FQuat RelQuat;
	if (TagKeyValMap.Contains("QuatW")) { RelQuat.W = FCString::Atof(*TagKeyValMap["QuatW"]); }
	else { return false; }
	if (TagKeyValMap.Contains("QuatX")) { RelQuat.X = FCString::Atof(*TagKeyValMap["QuatX"]); }
	else { return false; }
	if (TagKeyValMap.Contains("QuatY")) { RelQuat.Y = FCString::Atof(*TagKeyValMap["QuatY"]); }
	else { return false; }
	if (TagKeyValMap.Contains("QuatZ")) { RelQuat.Z = FCString::Atof(*TagKeyValMap["QuatZ"]); }
	else { return false; }
	
	SetRelativeTransform(FTransform(RelQuat, RelLoc));
	return true;
}

// Save values to tags
bool USLCollisionListener::ApplyAndSaveParameters(UStaticMeshComponent* SMComp)
{
	// Apply parameters
	SetBoxExtent(SMComp->Bounds.BoxExtent * SL_COLL_SCALE_FACTOR);

	FTransform BoundsTransf(FQuat::Identity, SMComp->Bounds.Origin);
	BoundsTransf.SetToRelativeTransform(SMComp->GetComponentTransform());
	SetRelativeTransform(BoundsTransf);

	// Save parameters
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
	
	return FTags::AddKeyValuePairs(GetOuter(), "SemLogColl", KeyValMap);
}