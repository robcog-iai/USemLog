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
		ASLFurnitureStateManager::InitStates();
		// TODO run with a delay to make sure the runtime manager is init
		// Init constraints and states
		//FTimerHandle DelayTimer;
		//GetWorldTimerManager().SetTimer(DelayTimer, this, &ASLFurnitureStateManager::InitStates, 0.05f, false);
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
					FurnitureToIndividual.Emplace(CurrFurnitureActor, FurnitureIndividual);
					DrawerToInitLoc.Emplace(CurrFurnitureActor, CurrFurnitureActor->GetActorLocation());
					DrawerToLimit.Emplace(CurrFurnitureActor, CurrConstrComp->ConstraintInstance.GetLinearLimit());
					FurnitureToState.Emplace(CurrFurnitureActor, EFurnitureState::Closed);
					StartEvent(CurrFurnitureActor, FurnitureIndividual, EFurnitureState::Closed);
				}
				else if(Class.Contains("Door"))
				{
					FurnitureToIndividual.Emplace(CurrFurnitureActor, FurnitureIndividual);
					DoorToConstraintComp.Emplace(CurrFurnitureActor, CurrConstrComp);
					FurnitureToState.Emplace(CurrFurnitureActor, EFurnitureState::Closed);
					StartEvent(CurrFurnitureActor, FurnitureIndividual, EFurnitureState::Closed);
				}
			}
		}
	}

	// Check drawer states with the given update rate (add delay until the drawers are closed)
	GetWorldTimerManager().SetTimer(FurnitureStateTimerHandle, this, &ASLFurnitureStateManager::CheckStates, UpdateRate, true);
}

// Check drawer states
void ASLFurnitureStateManager::CheckStates()
{
	for (auto& FurnitureItr : FurnitureToState)
	{
		AActor* const CurrFurniture = FurnitureItr.Key;

		EFurnitureState CurrState = ASLFurnitureStateManager::GetState(CurrFurniture);

		// Check if current state differs of the previous one		
		if (CurrState != FurnitureItr.Value)
		{
			// Terminate event
			ASLFurnitureStateManager::FinishEvent(CurrFurniture);

			// Start new event			
			FOwlIndividualName CurrFurnitureIndividual = FurnitureToIndividual[CurrFurniture];
			ASLFurnitureStateManager::StartEvent(CurrFurniture, CurrFurnitureIndividual, CurrState);
			FurnitureItr.Value =  CurrState;
		}
	}
}

// Get the current state of the furniture
EFurnitureState ASLFurnitureStateManager::GetState(AActor* FurnitureActor)
{
	// Check for furniture type
	if (DrawerToInitLoc.Contains(FurnitureActor))
	{
		const FVector InitLoc = DrawerToInitLoc[FurnitureActor];
		const float ConstraintLimit = DrawerToLimit[FurnitureActor];
		const float Distance = InitLoc.Distance(InitLoc, FurnitureActor->GetActorLocation());

		// Closed limit
		if (Distance < ConstraintLimit * 0.1)
		{
			return EFurnitureState::Closed;
		}
		// Half closed limit
		else if (Distance > ConstraintLimit * 0.1 
			&& Distance < ConstraintLimit * 0.5)
		{
			return EFurnitureState::HalfClosed;
		}
		// Half opened limit
		else if (Distance > ConstraintLimit * 0.5 
			&& Distance < ConstraintLimit - (ConstraintLimit * 0.1))
		{
			return EFurnitureState::HalfOpened;
		}
		// Opened limit
		else if (Distance > ConstraintLimit - (ConstraintLimit * 0.1))
		{
			return EFurnitureState::Opened;
		}
	}
	else if (DoorToConstraintComp.Contains(FurnitureActor))
	{
		UPhysicsConstraintComponent* CurrConstr = DoorToConstraintComp[FurnitureActor];		
		FConstraintInstance CurrConstrInst = CurrConstr->ConstraintInstance;

		if (CurrConstrInst.GetAngularSwing1Motion() == EAngularConstraintMotion::ACM_Limited)
		{
			const float LimitRad = FMath::DegreesToRadians(CurrConstrInst.GetAngularSwing1Limit()) * 2.f;
			const float OffsetRad = FMath::DegreesToRadians(CurrConstrInst.AngularRotationOffset.Yaw);
			const float MinRad = 0.f - OffsetRad;
			const float MaxRad = LimitRad - OffsetRad;
			const float CurrSwing1 = CurrConstrInst.GetCurrentSwing1();

			if (CurrSwing1 < MinRad + (LimitRad * 0.1))
			{
				return EFurnitureState::Closed;
			}
			// Half closed limit
			else if (CurrSwing1 > MinRad + (LimitRad * 0.1)
				&& CurrSwing1 < MinRad + (LimitRad * 0.5))
			{
				return EFurnitureState::HalfClosed;
			}
			// Half opened limit
			else if (CurrSwing1 > MinRad + (LimitRad * 0.5)
				&& CurrSwing1 < MaxRad - (LimitRad * 0.1))
			{
				return EFurnitureState::HalfOpened;
			}
			// Opened limit
			else if (CurrSwing1 > MaxRad - (LimitRad * 0.1))
			{
				return EFurnitureState::Opened;
			}
		}
		else if (CurrConstrInst.GetAngularSwing2Motion() == EAngularConstraintMotion::ACM_Limited)
		{
			const float LimitRad = FMath::DegreesToRadians(CurrConstrInst.GetAngularSwing2Limit()) * 2.f;
			const float OffsetRad = FMath::DegreesToRadians(CurrConstrInst.AngularRotationOffset.Pitch);
			const float MinRad = 0.f - OffsetRad;
			const float MaxRad = LimitRad - OffsetRad;
			const float CurrSwing2 = CurrConstrInst.GetCurrentSwing2();

			// TODO no general rule found, for swing2 with the given offset and initial position the state ordering is switched
			if (CurrSwing2 < MinRad + (LimitRad * 0.1))
			{
				return EFurnitureState::Opened;
			}
			else if (CurrSwing2 > MinRad + (LimitRad * 0.1)
				&& CurrSwing2 < MinRad + (LimitRad * 0.5))
			{
				return EFurnitureState::HalfOpened;
			}
			// Half opened limit
			else if (CurrSwing2 > MinRad + (LimitRad * 0.5)
				&& CurrSwing2 < MaxRad - (LimitRad * 0.1))
			{
				return EFurnitureState::HalfClosed;
			}
			// Opened limit
			else if (CurrSwing2 > MaxRad - (LimitRad * 0.1))
			{
				return EFurnitureState::Closed;
			}
		}
	}
	else
	{
		if (FurnitureToState.Contains(FurnitureActor))
		{
			return FurnitureToState[FurnitureActor];
		}
	}
	// Default state
	return EFurnitureState::Closed;
}

// Start event
void ASLFurnitureStateManager::StartEvent(AActor* FurnitureActor, FOwlIndividualName FurnitureIndividual, EFurnitureState State)
{
	// Example event
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
	const FOwlPrefixName ObjectActedOn("knowrob", "objectActedOn");
	const FOwlPrefixName OwlNamedIndividual("owl", "NamedIndividual");
	// Owl classes
	const FOwlClass XsdString("xsd", "string");
	const FOwlClass StateClosed("knowrob_u", "FurnitureStateClosed");
	const FOwlClass StateHalfClosed("knowrob_u", "FurnitureStateHalfClosed");
	const FOwlClass StateHalfOpened("knowrob_u", "FurnitureStateHalfOpened");
	const FOwlClass StateOpened("knowrob_u", "FurnitureStateOpened");

	// Add the event properties
	TArray <FOwlTriple> Properties;
	switch (State)
	{
		case EFurnitureState::Closed : 
			FurnitureEventIndividual.Set("log", "FurnitureStateClosed", FSLUtils::GenerateRandomFString(4));
			Properties.Emplace(FOwlTriple(RdfType, RdfResource, StateClosed));
			Properties.Emplace(FOwlTriple(TaskContext, RdfDatatype, XsdString,
				"FurnitureStateClosed-" + FurnitureIndividual.GetName()));
			break;
		case EFurnitureState::HalfClosed :
			FurnitureEventIndividual.Set("log", "FurnitureStateHalfClosed", FSLUtils::GenerateRandomFString(4));
			Properties.Emplace(FOwlTriple(RdfType, RdfResource, StateHalfClosed));
			Properties.Emplace(FOwlTriple(TaskContext, RdfDatatype, XsdString,
				"FurnitureStateHalfClosed-" + FurnitureIndividual.GetName()));
			break;
		case EFurnitureState::HalfOpened :
			FurnitureEventIndividual.Set("log", "FurnitureStateHalfOpened", FSLUtils::GenerateRandomFString(4));
			Properties.Emplace(FOwlTriple(RdfType, RdfResource, StateHalfOpened));
			Properties.Emplace(FOwlTriple(TaskContext, RdfDatatype, XsdString,
				"FurnitureStateHalfOpened-" + FurnitureIndividual.GetName()));
			break;
		case EFurnitureState::Opened :
			FurnitureEventIndividual.Set("log", "FurnitureStateOpened", FSLUtils::GenerateRandomFString(4));
			Properties.Emplace(FOwlTriple(RdfType, RdfResource, StateOpened));
			Properties.Emplace(FOwlTriple(TaskContext, RdfDatatype, XsdString,
				"FurnitureStateOpened-" + FurnitureIndividual.GetName()));
			break;
	}	
	Properties.Emplace(FOwlTriple(ObjectActedOn, RdfResource, FurnitureIndividual));

	// Create the furniture event
	TSharedPtr<FOwlNode> FurnitureEvent = MakeShareable(new FOwlNode(
		OwlNamedIndividual, RdfAbout, FurnitureEventIndividual, Properties));

	// Start the event with the given properties
	if (SemLogRuntimeManager->StartEvent(FurnitureEvent))
	{
		ActorToEvent.Emplace(FurnitureActor, FurnitureEvent);
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