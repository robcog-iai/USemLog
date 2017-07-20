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
	for (TActorIterator<ASLRuntimeManager>RMItr(GetWorld()); RMItr; ++RMItr)
	{
		SemLogRuntimeManager = *RMItr;
		break;
	}
	if (SemLogRuntimeManager)
	{
		// Bind overlap begin and end events
		OnComponentBeginOverlap.AddDynamic(this, &USLContactManager::OnOverlapBegin);
		OnComponentEndOverlap.AddDynamic(this, &USLContactManager::OnOverlapEnd);
	}
}

// Called on overlap begin events
void USLContactManager::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Check if the colliding actor has a semantic description
	int32 TagIndex = FTagStatics::GetTagTypeIndex(OtherActor->Tags, "SemLog");

	// If tag type exist, read the Class and the Id
	if (TagIndex != INDEX_NONE)
	{
		// Get the Class and Id from the tags
		const FString OtherActorClass = FTagStatics::GetKeyValue(OtherActor->Tags[TagIndex], "Class");
		const FString OtherActorId = FTagStatics::GetKeyValue(OtherActor->Tags[TagIndex], "Id");

		/********************************************************************
		Example of a contact event represented in OWL:
		<!-- Event node described with a FOwlTriple (Subject-Predicate-Object) and Properties: -->
		<owl:NamedIndividual rdf:about="&log;TouchingSituation_icaO">
		<!-- List of the event properties as FOwlTriple (Subject-Predicate-Object): -->
		<rdf:type rdf:resource = "&knowrob_u;TouchingSituation"/>
		<knowrob:taskContext rdf:datatype = "&xsd;string">Contact-IslandDrawerTopLeft_o5Ol-Bowl3_9w2Y</knowrob:taskContext>
		<knowrob:startTime rdf:resource="&log;timepoint_0.302039"/>
		<knowrob_u:inContact rdf:resource="&log;IslandDrawerTopLeft_o5Ol"/>
		<knowrob_u:inContact rdf:resource="&log;Bowl3_9w2Y"/>
		<knowrob:endTime rdf:resource="&log;timepoint_0.331089"/>
		</owl:NamedIndividual>
		*********************************************************************/

		// Create contact event and other actor individual
		const FOwlIndividualName OtherIndividual("log", OtherActorClass, OtherActorId);
		const FOwlIndividualName ContactIndividual("log", "TouchingSituation", FSLUtils::GenerateRandomFString(4));
		// Owl prefixed names
		const FOwlPrefixName RdfType("rdf", "type");
		const FOwlPrefixName RdfAbout("rdf", "about");
		const FOwlPrefixName RdfResource("rdf", "resource");
		const FOwlPrefixName RdfDatatype("rdf", "datatype");
		const FOwlPrefixName TaskContext("knowrob", "taskContext");
		const FOwlPrefixName InContact("knowrob_u", "inContact");
		const FOwlPrefixName OwlNamedIndividual("owl", "NamedIndividual");
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

		// Create contact event add it to the map
		TSharedPtr<FOwlNode> ContactEvent = MakeShareable(new FOwlNode(
			OwlNamedIndividual, RdfAbout, ContactIndividual));
		OtherActorToEvent.Emplace(OtherActor, ContactEvent);

		// Start the event with the given properties
		SemLogRuntimeManager->StartEvent(ContactEvent);
	}
}

// Called on overlap end events
void USLContactManager::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// Check if other actor is in the map (contact event was started)
	if (OtherActorToEvent.Contains(OtherActor))
	{
		// End the event
		SemLogRuntimeManager->FinishEvent(OtherActorToEvent[OtherActor]);
	}
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