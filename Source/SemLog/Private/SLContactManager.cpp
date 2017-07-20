// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLContactManager.h"
#include "SLUtils.h"
#include "TagStatics.h"
#include "EngineUtils.h"

// Constructor
USLContactManager::USLContactManager()
{
	// Default type
	AreaType = EContactAreaType::Default;

	// Set the default parent as the owning actor
	ParentActor = GetOwner();

	// Check if the parent has a static mesh component
	if (ParentActor && ParentActor->IsA(AStaticMeshActor::StaticClass()))
	{
		ParentStaticMeshComponent = Cast<AStaticMeshActor>(ParentActor)->GetStaticMeshComponent();
		if (!ParentStaticMeshComponent)
		{
			UE_LOG(LogTemp, Error, TEXT("Could not get te static mesh component of the parent actor!"));
		}
	}
}

// Destructor
USLContactManager::~USLContactManager()
{
}

// Called when spawned or level started
void USLContactManager::BeginPlay()
{
	Super::BeginPlay();

	// Index of the given tag type in the array
	int32 TagIndex = FTagStatics::GetTagTypeIndex(GetOwner()->Tags, "SemLog");

	// If tag type exist, read the Class and the Id of parent
	if (TagIndex != INDEX_NONE)
	{
		const FString Class = FTagStatics::GetKeyValue(GetOwner()->Tags[TagIndex], "Class");
		const FString Id = FTagStatics::GetKeyValue(GetOwner()->Tags[TagIndex], "Id");
		ParentIndividual.Set("log", Class, Id);
	}
	
	// Get the semantic log runtime manager from the world
	SemLogRuntimeManager = *TActorIterator<ASLRuntimeManager>(GetWorld());
	if (SemLogRuntimeManager)
	{
		// Bind overlap begin and end events
		OnComponentBeginOverlap.AddDynamic(this, &USLContactManager::OnOverlapBegin);
		OnComponentEndOverlap.AddDynamic(this, &USLContactManager::OnOverlapEnd);
	}
}


// Called on overlap begin events
void USLContactManager::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Check if the colliding actor has a semantic description
	int32 TagIndex = FTagStatics::GetTagTypeIndex(OtherActor->Tags, "SemLog");

	// If tag type exist, read the Class and the Id
	if (TagIndex != INDEX_NONE)
	{
		// Create the other actor individual
		const FString OtherClass = FTagStatics::GetKeyValue(OtherActor->Tags[TagIndex], "Class");
		const FString OtherId = FTagStatics::GetKeyValue(OtherActor->Tags[TagIndex], "Id");
		const FOwlIndividualName OtherIndividual("log", OtherClass, OtherId);

		// Owl prefixed names
		const FOwlPrefixName RdfType("rdf", "type");
		const FOwlPrefixName RdfResource("rdf", "resource");
		const FOwlPrefixName RdfDatatype("rdf", "datatype");		
		const FOwlPrefixName TaskContext("knowrob", "taskContext");
		const FOwlPrefixName InContact("knowrob_u", "inContact");

		// Owl classes
		const FOwlClass XsdString("xsd", "string");
		const FOwlClass TouchingSituation("knowrob_u", "TouchingSituation");

		// Add the event properties
		TArray <FOwlTriple> Properties;
		Properties.Add(FOwlTriple(RdfType, RdfResource, TouchingSituation));
		Properties.Add(FOwlTriple(TaskContext, RdfDatatype, XsdString,
			"Contact-" + ParentIndividual.GetName() + "-" + OtherIndividual.GetName()));
		Properties.Add(FOwlTriple(InContact, RdfResource, ParentIndividual));
		Properties.Add(FOwlTriple(InContact, RdfResource, OtherIndividual));

		// Create contact event individual name and add it to the map
		TSharedPtr<FOwlIndividualName> ContactEvent = 
			MakeShareable(new FOwlIndividualName("log", "TouchingSituation", FSLUtils::GenerateRandomFString(4)));
		OtherActorToContactEventIndividualName.Emplace(OtherActor, ContactEvent);

		// Start the event with the given properties
		SemLogRuntimeManager->StartEvent(ContactEvent, GetWorld()->GetTimeSeconds(), Properties);
	}
	UE_LOG(LogTemp, Error, TEXT("Overlap begin!"));
}

// Called on overlap end events
void USLContactManager::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// Check if other actor is in the map (contact event was started)
	if (OtherActorToContactEventIndividualName.Contains(OtherActor))
	{
		// End the event
		SemLogRuntimeManager->FinishEvent(
			OtherActorToContactEventIndividualName[OtherActor], GetWorld()->GetTimeSeconds());
	}

	UE_LOG(LogTemp, Error, TEXT("Overlap end!"));
}

#if WITH_EDITOR  
void USLContactManager::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	// Call the base class version  
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Check if the area type changed
	if (PropertyChangedEvent.MemberProperty->GetMetaData(TEXT("Category")).Equals(TEXT("SL")))
	{
		if (PropertyChangedEvent.MemberProperty->GetName().Equals(TEXT("AreaType")))
		{
			// Calculate new type of box extent
			UpdateContactArea();
		}
		else if (PropertyChangedEvent.MemberProperty->GetName().Equals(TEXT("ParentActor")))
		{
			// Calculate the box extent of the new actor
			UpdateContactArea();
		}
		else if (PropertyChangedEvent.MemberProperty->GetName().Equals(TEXT("ParentStaticMeshComponent")))
		{
			// Calculate the box extent of the new static mesh
			UpdateContactArea();
		}
	}
}

// Check area type
void USLContactManager::UpdateContactArea()
{
	// Check if the parent is set and has a static mesh component
	if (ParentActor && ParentStaticMeshComponent)
	{
		// Check for the area type
		switch (AreaType)
		{
		case(EContactAreaType::Top):
			CaclulateAreaAsTop();
			break;
		case(EContactAreaType::Bottom):
			CalculateAreaAsBottom();
			break;
		case(EContactAreaType::Wrapper):
			CalculateAreaAsWrapper();
			break;
		case(EContactAreaType::Default):
			break;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Parent actor or static mesh not set!"));
	}
}

// Calculate surface area
void USLContactManager::CaclulateAreaAsTop()
{
	UE_LOG(LogTemp, Error, TEXT("USLContactManager %s type: SURFACE"), *GetName());

	// Get the bounding box of the mesh
	const FBox SMBox = ParentStaticMeshComponent->GetStaticMesh()->GetBoundingBox();
	const FVector SMExtent = SMBox.GetExtent();

	// Set box extent
	SetBoxExtent(FVector(SMExtent.X, SMExtent.Y, 2.f), false);

	// Set the location and the rotation
	SetWorldLocation(ParentStaticMeshComponent->GetComponentLocation() + FVector(0.f, 0.f, SMExtent.Z));
	SetWorldRotation(ParentStaticMeshComponent->GetComponentRotation());
}

// Calculate inner area
void USLContactManager::CalculateAreaAsBottom()
{
	UE_LOG(LogTemp, Error, TEXT("USLContactManager %s type: INNER"), *GetName());

	// Get the bounding box of the mesh
	const FBox SMBox = ParentStaticMeshComponent->GetStaticMesh()->GetBoundingBox();
	const FVector SMExtent = SMBox.GetExtent();

	// Set box extent
	SetBoxExtent(FVector(SMExtent.X, SMExtent.Y, 2.f), false);

	// Set the location and the rotation
	SetWorldLocation(ParentStaticMeshComponent->GetComponentLocation() - FVector(0.f, 0.f, SMExtent.Z));
	SetWorldRotation(ParentStaticMeshComponent->GetComponentRotation());
}

// Calculate wrapper area
void USLContactManager::CalculateAreaAsWrapper()
{
	UE_LOG(LogTemp, Error, TEXT("USLContactManager %s type: WRAPPER"), *GetName());

	// Get the bounding box of the mesh
	const FBox SMBox = ParentStaticMeshComponent->GetStaticMesh()->GetBoundingBox();
	const FVector SMExtent = SMBox.GetExtent();

	// Set box extent
	SetBoxExtent(SMExtent + FVector(2.f), false);

	// Set the location and the rotation
	SetWorldLocation(ParentStaticMeshComponent->GetComponentLocation());
	SetWorldRotation(ParentStaticMeshComponent->GetComponentRotation());
}
#endif 