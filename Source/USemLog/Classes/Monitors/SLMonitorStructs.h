// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "Components/MeshComponent.h"
#include "Individuals/Type/SLBaseIndividual.h"
#include "SLMonitorStructs.generated.h"



/************************************************************************/
/*                       DELEGATES                                      */
/************************************************************************/
/** Delegate to notify that a contact begins between two semantically annotated objects */
DECLARE_MULTICAST_DELEGATE_OneParam(FSLBeginContactSignature, const FSLContactResult&);

/** Delegate to notify that a contact ended between two semantically annotated objects */
DECLARE_MULTICAST_DELEGATE_ThreeParams(FSLEndContactSignature, USLBaseIndividual* /*Self*/, USLBaseIndividual* /*Other*/, float /*Time*/);

/************************************************************************/
/*                       STRUCTS                                        */
/************************************************************************/
/**
 * Structure containing information about the semantic overlap event
 */
USTRUCT()
struct FSLContactResult
{
	GENERATED_BODY()

	// Self
	USLBaseIndividual* Self;

	// Other 
	USLBaseIndividual* Other;

	// The mesh (static or skeletal) of the other overlapping component
	TWeakObjectPtr<UMeshComponent> SelfMeshComponent;

	// The mesh (static or skeletal) of the other overlapping component
	TWeakObjectPtr<UMeshComponent> OtherMeshComponent;

	// Timestamp in seconds of the event triggering
	float Time;

	// Flag showing if Other is also of type Semantic Overlap Area
	bool bIsOtherASemanticOverlapArea;

	// Default ctor
	FSLContactResult() {};

	// Init constructor
	FSLContactResult(USLBaseIndividual* InSelf, USLBaseIndividual* InOther, float InTime, bool bIsSemanticOverlapArea) :
		Self(InSelf),
		Other(InOther),
		Time(InTime),
		bIsOtherASemanticOverlapArea(bIsSemanticOverlapArea)
	{};

	// Init constructor with mesh component (static/skeletal)
	FSLContactResult(USLBaseIndividual* InSelf, USLBaseIndividual* InOther, float InTime, bool bIsSemanticOverlapArea,
		UMeshComponent* InSelfMeshComponent, UMeshComponent* InOtherMeshComponent) :
		Self(InSelf),
		Other(InOther),
		SelfMeshComponent(InSelfMeshComponent),
		OtherMeshComponent(InOtherMeshComponent),
		Time(InTime),
		bIsOtherASemanticOverlapArea(bIsSemanticOverlapArea)
	{};

	// Get result as string
	FString GetInfo() const
	{
		FString Info;
		Info.Append(FString::Printf(TEXT("Self=%s; "), Self ? *Self->GetInfo() : *FString("null")));
		Info.Append(FString::Printf(TEXT("Other=%s; "), Self ? *Self->GetInfo() : *FString("null")));
		Info.Append(FString::Printf(TEXT("Time=%f; "), Time));
		Info.Append(FString::Printf(TEXT("bIsOtherASemanticOverlapArea=%d; "), bIsOtherASemanticOverlapArea));
		Info.Append(FString::Printf(TEXT("SelfMeshComponent=%s; "), SelfMeshComponent.IsValid() ? *SelfMeshComponent->GetName() : TEXT("null")));
		Info.Append(FString::Printf(TEXT("OtherMeshComponent=%s; "), OtherMeshComponent.IsValid() ? *OtherMeshComponent->GetName() : TEXT("null")));
		return Info;
	}
};

/**
* Structure holding the semantic data of two entities
*/
struct FSLMonEntityPair
{
	// First entity
	USLBaseIndividual* Individual1;

	// Second entity
	USLBaseIndividual* Individual2;

	// Default constructor
	FSLMonEntityPair() {};

	// Init constructor
	FSLMonEntityPair(USLBaseIndividual* InIndividual1, USLBaseIndividual* InIndividual2) : Individual1(InIndividual1), Individual2(InIndividual2) {};

	// True if the unique id, the semantic id and the semantic class is not empty
	bool IsLoaded() const { return Individual1->IsLoaded() && Individual2->IsLoaded(); }

	// Get result as string
	FString GetInfo() const
	{
		FString Info;
		Info.Append(FString::Printf(TEXT("Individual1=%s; "), Individual1 ? *Individual1->GetInfo() : *FString("null")));
		Info.Append(FString::Printf(TEXT("Individual2=%s; "), Individual2 ? *Individual2->GetInfo() : *FString("null")));
		return Info;
	}
};


