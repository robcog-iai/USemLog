// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLFurnitureStateManager.h"
#include "PhysicsEngine/PhysicsConstraintActor.h"

// Sets default values
ASLFurnitureStateManager::ASLFurnitureStateManager()
{
	PrimaryActorTick.bCanEverTick = false;

	UpdateRate = 0.25f;
}

// Called when the game starts or when spawned
void ASLFurnitureStateManager::BeginPlay()
{
	Super::BeginPlay();

	// Get the semantic log runtime manager from the world
	for (TActorIterator<ASLRuntimeManager>RMItr(GetWorld()); RMItr; ++RMItr)
	{
		SemLogRuntimeManager = *RMItr;
		break;
	}

	if (SemLogRuntimeManager)
	{
		// Init constraints and states
		ASLFurnitureStateManager::InitStates();

		// Check drawer states with the given update rate (add delay until the drawers are closed)
		GetWorldTimerManager().SetTimer(FurnitureStateTimerHandle, this, &ASLFurnitureStateManager::CheckStates, UpdateRate, true);
	}
}

// Init states
void ASLFurnitureStateManager::InitStates()
{
	// Get all relevant constraints
	for (TActorIterator<APhysicsConstraintActor> ConstrItr(GetWorld()); ConstrItr; ++ConstrItr)
	{
		// TODO we assume all relevant actors are as ConstraintActor2
		UPhysicsConstraintComponent* CurrConstrComp = ConstrItr->GetConstraintComp();
		AStaticMeshActor* CurrFurnitureActor = Cast<AStaticMeshActor>(CurrConstrComp->ConstraintActor2);

		if (CurrFurnitureActor)
		{
			// Index of the given tag type in the array
			int32 TagIndex = FTagStatics::GetTagTypeIndex(CurrFurnitureActor->Tags, "SemLog");

			// If tag type exist, read the Class and the Id of parent
			if (TagIndex != INDEX_NONE)
			{
				const FString Class = FTagStatics::GetKeyValue(CurrFurnitureActor->Tags[TagIndex], "Class");
				const FString Id = FTagStatics::GetKeyValue(CurrFurnitureActor->Tags[TagIndex], "Id");
				FOwlIndividualName FurnitureIndividual("log", Class, Id);

				// TODO hardcoded keywords
				// Check if actor is of required furniture type
				if (Class.Contains("Drawer"))
				{
					FurnitureToIndividual.Add(CurrFurnitureActor, FurnitureIndividual);
					DrawerToInitLoc.Add(CurrFurnitureActor, CurrFurnitureActor->GetActorLocation());
					DrawerToLimit.Add(CurrFurnitureActor, CurrConstrComp->ConstraintInstance.GetLinearLimit());
					FurnitureToState.Add(CurrFurnitureActor, EFurnitureState::Closed);
					StartEvent(CurrFurnitureActor, FurnitureIndividual, EFurnitureState::Closed);
				}
				else if(Class.Contains("Door"))
				{
					FurnitureToIndividual.Add(CurrFurnitureActor, FurnitureIndividual);
					DoorToConstraintComp.Add(CurrFurnitureActor, CurrConstrComp);
					FurnitureToState.Add(CurrFurnitureActor, EFurnitureState::Closed);
					StartEvent(CurrFurnitureActor, FurnitureIndividual, EFurnitureState::Closed);
				}
			}
		}
	}
}

// Check drawer states
void ASLFurnitureStateManager::CheckStates()
{
	UE_LOG(LogTemp, Warning, TEXT("Checking furniture state"));
	//for (auto& ConstrToStateItr : ConstraintsToFurnitureState)
	//{
	//	EFurnitureState CurrState = ASLFurnitureStateManager::GetState(ConstrToStateItr.Key);
	//	// Check if current state differs of the previous one
	//	
	//	if (CurrState != ConstrToStateItr.Value)
	//	{
	//		// Terminate event

	//		// Start new event
	//	}
	//}
}

// Get the current state of the furniture
EFurnitureState ASLFurnitureStateManager::GetState(AActor* FurnitureActor)
{

	//if (DrawerToInitLoc->Contains(FurnitureActor))
	//{
	//	FurnitureActor->GetActorLocation();
	//}

	//// TODO hardcoded for XLinear and Swing1 and Swing2 motions
	//if (ConstrInst.GetLinearXMotion() == ELinearConstraintMotion::LCM_Limited)
	//{
	//	ConstrInst->get
	//}
	//else if (ConstrInst.GetAngularSwing1Motion() == EAngularConstraintMotion::ACM_Limited)
	//{

	//}
	//else if (ConstrInst.GetAngularSwing2Motion() == EAngularConstraintMotion::ACM_Limited)
	//{

	//}

	return EFurnitureState::Closed;
}

// Start event
void ASLFurnitureStateManager::StartEvent(AActor* FurnitureActor, FOwlIndividualName FurnitureIndividual, EFurnitureState State)
{
	// Example of a contact event represented in OWL:
	/********************************************************************
	<!-- Event node described with a FOwlTriple (Subject-Predicate-Object) and Properties: -->
	<owl:NamedIndividual rdf:about="&log;FurnitureStateHalfOpened_OhnU">
	<!-- List of the event properties as FOwlTriple (Subject-Predicate-Object): -->
		<rdf:type rdf:resource="&knowrob_u;FurnitureStateHalfOpened"/>
		<knowrob:taskContext rdf:datatype="&xsd;string">FurnitureStateHalfOpened-IslandDrawerTopLeft_o5Ol</knowrob:taskContext>
		<knowrob:startTime rdf:resource="&log;timepoint_11.263422"/>
		<knowrob:objectActedOn rdf:resource="&log;IslandDrawerTopLeft_o5Ol"/>
		<knowrob:endTime rdf:resource="&log;timepoint_12.011743"/>
	</owl:NamedIndividual>
	*********************************************************************/

	FOwlIndividualName FurnitureEventIndividual;
	// Owl prefixed names
	const FOwlPrefixName RdfType("rdf", "type");
	const FOwlPrefixName RdfAbout("rdf", "about");
	const FOwlPrefixName RdfResource("rdf", "resource");
	const FOwlPrefixName RdfDatatype("rdf", "datatype");
	const FOwlPrefixName TaskContext("knowrob", "taskContext");
	const FOwlPrefixName PerformedBy("knowrob", "performedBy");
	const FOwlPrefixName ObjectActedOn("knowrob", "objectActedOn");
	const FOwlPrefixName OwlNamedIndividual("owl", "NamedIndividual");
	// Owl classes
	const FOwlClass XsdString("xsd", "string");
	const FOwlClass StateClosed("knowrob", "FurnitureStateClosed");
	const FOwlClass StateHalfClosed("knowrob", "FurnitureStateHalfClosed");
	const FOwlClass StateHalfOpened("knowrob", "FurnitureStateHalfOpened");
	const FOwlClass StateOpened("knowrob", "FurnitureStateOpened");

	// Add the event properties
	TArray <FOwlTriple> Properties;
	switch (State)
	{
		case EFurnitureState::Closed : 
			FurnitureEventIndividual.Set("log", "FurnitureStateClosed", FSLUtils::GenerateRandomFString(4));
			Properties.Add(FOwlTriple(RdfType, RdfResource, StateClosed));
			Properties.Add(FOwlTriple(TaskContext, RdfDatatype, XsdString,
				"FurnitureStateClosed-" + FurnitureIndividual.GetName()));
			break;
		case EFurnitureState::HalfClosed :
			FurnitureEventIndividual.Set("log", "FurnitureStateHalfClosed", FSLUtils::GenerateRandomFString(4));
			Properties.Add(FOwlTriple(RdfType, RdfResource, StateHalfClosed));
			Properties.Add(FOwlTriple(TaskContext, RdfDatatype, XsdString,
				"FurnitureStateHalfClosed-" + FurnitureIndividual.GetName()));
			break;
		case EFurnitureState::HalfOpened :
			FurnitureEventIndividual.Set("log", "FurnitureStateHalfOpened", FSLUtils::GenerateRandomFString(4));
			Properties.Add(FOwlTriple(RdfType, RdfResource, StateHalfOpened));
			Properties.Add(FOwlTriple(TaskContext, RdfDatatype, XsdString,
				"FurnitureStateHalfOpened-" + FurnitureIndividual.GetName()));
			break;
		case EFurnitureState::Opened :
			FurnitureEventIndividual.Set("log", "FurnitureStateOpened", FSLUtils::GenerateRandomFString(4));
			Properties.Add(FOwlTriple(RdfType, RdfResource, StateOpened));
			Properties.Add(FOwlTriple(TaskContext, RdfDatatype, XsdString,
				"FurnitureStateOpened-" + FurnitureIndividual.GetName()));
			break;
	}	
	Properties.Add(FOwlTriple(ObjectActedOn, RdfResource, FurnitureIndividual));

	// Create the furniture event
	TSharedPtr<FOwlNode> FurnitureEvent = MakeShareable(new FOwlNode(
		OwlNamedIndividual, RdfAbout, FurnitureEventIndividual, Properties));

	// Start the event with the given properties
	if (SemLogRuntimeManager->StartEvent(FurnitureEvent))
	{
		ActorToEvent.Add(FurnitureActor, FurnitureEvent);
	}
}

// Finish event
void ASLFurnitureStateManager::FinishEvent(AActor* FurnitureActor)
{
	// Pointer to the possible started event
	TSharedPtr<FOwlNode>CopyOfStartedEvent;

	// Check if event started, if yes remove and copy value
	if (ActorToEvent.RemoveAndCopyValue(FurnitureActor, CopyOfStartedEvent))
	{
		SemLogRuntimeManager->FinishEvent(CopyOfStartedEvent);
	}
}