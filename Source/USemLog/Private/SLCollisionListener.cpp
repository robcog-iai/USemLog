// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLCollisionListener.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"

// UUTils
#include "Tags.h"

#define SL_COLL_SCALE_FACTOR 1.1f

// Default constructor
USLCollisionListener::USLCollisionListener()
{
}

// Called after the C++ constructor and after the properties have been initialized
void  USLCollisionListener::PostInitProperties()
{
	Super::PostInitProperties();
	
	UObject* Outer = GetOuter();
	if (AStaticMeshActor* OuterAsSMAct = Cast<AStaticMeshActor>(Outer))
	{
		if (UStaticMeshComponent* SMComp = OuterAsSMAct->GetStaticMeshComponent())
		{
			FVector Ext;
			FTransform RelTransf;
			if (LoadParameters(OuterAsSMAct->Tags, Ext, RelTransf))
			{
				SetBoxExtent(Ext * SL_COLL_SCALE_FACTOR);
				SetRelativeTransform(RelTransf);
			}
			else
			{
				Ext = SMComp->Bounds.BoxExtent;
				SetBoxExtent(Ext * SL_COLL_SCALE_FACTOR);
				SaveParameters(OuterAsSMAct->Tags, Ext, GetRelativeTransform());
			}
		}
	}
	else if (UStaticMeshComponent* OuterAsSMComp = Cast<UStaticMeshComponent>(Outer))
	{
		FVector Ext;
		FTransform RelTransf;
		if (LoadParameters(OuterAsSMComp->ComponentTags, Ext, RelTransf))
		{
			SetBoxExtent(Ext * SL_COLL_SCALE_FACTOR);
			SetRelativeTransform(RelTransf);
		}
		else
		{
			Ext = OuterAsSMComp->Bounds.BoxExtent;
			SetBoxExtent(Ext * SL_COLL_SCALE_FACTOR);
			SaveParameters(OuterAsSMComp->ComponentTags, Ext, GetRelativeTransform());
		}
	}
	ShapeColor = FColor::White;
}

#if WITH_EDITOR
// Called when a property is changed in the editor
void USLCollisionListener::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	UE_LOG(LogTemp, Error, TEXT("[%s][%d]"), TEXT(__FUNCTION__), __LINE__);

	// Get the changed property name
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ?
		PropertyChangedEvent.Property->GetFName() : NAME_None;

	//// Radio button style between bLogToJson, bLogToBson, bLogToMongo
	//if (PropertyName == GET_MEMBER_NAME_CHECKED(ASemanticLogger, bLogToJson))
	//{
	//}
	//else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASemanticLogger, bLogToBson))
	//{
	//}
	//else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASemanticLogger, bLogToMongo))
	//{
	//}
}
#endif // WITH_EDITOR


// Read values from tags
bool USLCollisionListener::LoadParameters(const TArray<FName>& InTags, FVector& OutExtent, FTransform& OutRelTransf)
{
	TMap<FString, FString> TagKeyValMap = 
		FTags::GetKeyValuePairs(InTags, "SemLogColl");

	if (TagKeyValMap.Num() == 0){return false;}

	if (TagKeyValMap.Contains("ExtX")){OutExtent.X = FCString::Atof(*TagKeyValMap["ExtX"]);}
	else{return false;}
	if (TagKeyValMap.Contains("ExtY")){OutExtent.Y = FCString::Atof(*TagKeyValMap["ExtY"]);}
	else{return false;}
	if (TagKeyValMap.Contains("ExtZ")){OutExtent.Z = FCString::Atof(*TagKeyValMap["ExtZ"]);}
	else{return false;}

	FVector RelLoc;
	if (TagKeyValMap.Contains("LocX")){RelLoc.X = FCString::Atof(*TagKeyValMap["LocX"]);}
	else{return false;}
	if (TagKeyValMap.Contains("LocY")){RelLoc.Y = FCString::Atof(*TagKeyValMap["LocY"]);}
	else{return false;}
	if (TagKeyValMap.Contains("LocZ")){	RelLoc.Z = FCString::Atof(*TagKeyValMap["LocZ"]);}
	else{return false;}
	OutRelTransf.SetLocation(RelLoc);
	
	FQuat RelQuat;
	if (TagKeyValMap.Contains("QuatW")) { RelQuat.W = FCString::Atof(*TagKeyValMap["QuatW"]); }
	else { return false; }
	if (TagKeyValMap.Contains("QuatX")) { RelQuat.X = FCString::Atof(*TagKeyValMap["QuatX"]); }
	else { return false; }
	if (TagKeyValMap.Contains("QuatY")) { RelQuat.Y = FCString::Atof(*TagKeyValMap["QuatY"]); }
	else { return false; }
	if (TagKeyValMap.Contains("QuatZ")) { RelQuat.Z = FCString::Atof(*TagKeyValMap["QuatZ"]); }
	else { return false; }
	OutRelTransf.SetRotation(RelQuat);
	
	return true;
}

// Save values to tags
void USLCollisionListener::SaveParameters(TArray<FName>& OutTags, const FVector& InExtent, const FTransform& InRelTransf)
{
	const FVector RelLoc = InRelTransf.GetLocation();
	const FQuat RelQuat = InRelTransf.GetRotation();

	TMap<FString, FString> KeyValMap;
	
	KeyValMap.Add("ExtX", FString::SanitizeFloat(InExtent.X));
	KeyValMap.Add("ExtY", FString::SanitizeFloat(InExtent.Y));
	KeyValMap.Add("ExtZ", FString::SanitizeFloat(InExtent.Z));
	
	KeyValMap.Add("LocX", FString::SanitizeFloat(RelLoc.X));
	KeyValMap.Add("LocY", FString::SanitizeFloat(RelLoc.Y));
	KeyValMap.Add("LocZ", FString::SanitizeFloat(RelLoc.Z));

	KeyValMap.Add("QuatW", FString::SanitizeFloat(RelQuat.W));
	KeyValMap.Add("QuatX", FString::SanitizeFloat(RelQuat.X));
	KeyValMap.Add("QuatY", FString::SanitizeFloat(RelQuat.Y));
	KeyValMap.Add("QuatZ", FString::SanitizeFloat(RelQuat.Z));
	
	FTags::AddKeyValuePairs(OutTags, "SemLogColl", KeyValMap);
}