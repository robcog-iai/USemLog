// Fill out your copyright notice in the Description page of Project Settings.

#include "USemLogPrivatePCH.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "SLDrawerStates.h"


// Sets default values
ASLDrawerStates::ASLDrawerStates()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Set default update rate (sec)
	UpdateRate = 0.25f;
}

// Called when the game starts or when spawned
void ASLDrawerStates::BeginPlay()
{
	Super::BeginPlay();

	// Set the semantic events exporter
	for (TActorIterator<ASLManager> SLManagerItr(GetWorld()); SLManagerItr; ++SLManagerItr)
	{
		SemEventsExporter = SLManagerItr->GetEventsExporter();
		break;
	}

	if (SemEventsExporter)
	{
		// Get the drawer/door constraints
		for (TActorIterator<APhysicsConstraintActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
		{
			Constraints.Add(ActorItr->GetConstraintComp());
		}

		// Apply force to close the drawers (after a delay until the objects fall on the surfaces)
		GetWorldTimerManager().SetTimer(
			CloseFurnitureTimerHandle, this, &ASLDrawerStates::CloseDrawers, UpdateRate, true, 2);
	}
	else
	{
		UE_LOG(SemLog, Error, TEXT(" ** DrawerStates: events exporter is not set!"));
	}
}

// Close drawers
void ASLDrawerStates::CloseDrawers()
{
	// TODO remove close drawers
	for (const auto ConstrItr : Constraints)
	{
		// Cast to static mesh actor
		AStaticMeshActor* SMAct = Cast<AStaticMeshActor>(ConstrItr->ConstraintActor2);
		
		if (SMAct)
		{
			// Copy of the constraint instance
			const FConstraintInstance& CurrConstr = ConstrItr->ConstraintInstance;
			

			if (CurrConstr.GetLinearXMotion() == ELinearConstraintMotion::LCM_Limited)
			{
				// Add the drawer and its initial position to the map
				DrawerToInitLocMap.Add(SMAct, SMAct->GetActorLocation());
			}
			else if (CurrConstr.GetAngularSwing1Motion() == EAngularConstraintMotion::ACM_Limited)
			{
				// Compute the min and max value of the joint
				TPair<float, float> MinMax;
				MinMax.Key = CurrConstr.GetCurrentSwing1();
				MinMax.Value = MinMax.Key + FMath::DegreesToRadians(CurrConstr.GetAngularSwing1Limit() + CurrConstr.AngularRotationOffset.Yaw);
				// Add the doors minmax pos
				DoorToMinMaxMap.Add(SMAct, MinMax);
			}
			else if (CurrConstr.GetAngularSwing2Motion() == EAngularConstraintMotion::ACM_Limited)
			{
				// Compute the min and max value of the joint
				TPair<float, float> MinMax;
				MinMax.Key = CurrConstr.GetCurrentSwing2();
				MinMax.Value = MinMax.Key - FMath::DegreesToRadians(CurrConstr.GetAngularSwing2Limit() + CurrConstr.AngularRotationOffset.Pitch);
				// Add the doors minmax pos
				DoorToMinMaxMap.Add(SMAct, MinMax);
			}

			// Get the static mesh component of the actor
			UStaticMeshComponent* SMComp = SMAct->GetStaticMeshComponent();
			
			// Add impule to static mesh in order to close the drawer/door
			SMAct->GetStaticMeshComponent()->AddImpulse(FVector(-900) * SMAct->GetActorForwardVector());
		}
	}

	// Clear timer
	GetWorldTimerManager().ClearTimer(CloseFurnitureTimerHandle);

	// Check drawer states with the given update rate (add delay until the drawers are closed)
	GetWorldTimerManager().SetTimer(
		FurnitureStateTimerHandle, this, &ASLDrawerStates::CheckDrawerStates, UpdateRate, true, 3);
}

// Check drawer states
void ASLDrawerStates::CheckDrawerStates()
{
	// Iterate all constraints
	for (const auto ConstrItr : Constraints)
	{
		// Get actor2
		AActor* FurnitureAct = ConstrItr->ConstraintActor2;

		// Check if it's type drawer or door (all drawers have linear X motion)
		if (ConstrItr->ConstraintInstance.GetLinearXMotion() == ELinearConstraintMotion::LCM_Limited)
		{
			const FVector CurrPos = FurnitureAct->GetActorLocation();
			const FVector InitPos = *DrawerToInitLocMap.Find(FurnitureAct);
			const float Dist = (CurrPos.X - InitPos.X) * FurnitureAct->GetActorForwardVector().X;

			if (Dist < -20.0f)
			{
				ASLDrawerStates::LogState(FurnitureAct, "Closed");
			}
			else if (Dist < 0.0f && Dist > -20.0f)
			{
				ASLDrawerStates::LogState(FurnitureAct, "HalfClosed");
			}
			else if (Dist > 20.0f)
			{
				ASLDrawerStates::LogState(FurnitureAct, "Opened");
			}
			else if (Dist > 0.0f && Dist < 20.0f)
			{
				ASLDrawerStates::LogState(FurnitureAct, "HalfOpened");
			}
		}
		else if(ConstrItr->ConstraintInstance.GetAngularSwing1Motion() == EAngularConstraintMotion::ACM_Limited)
		{
			const float CurrPos = ConstrItr->ConstraintInstance.GetCurrentSwing1();
			TPair<float, float> MinMax = *DoorToMinMaxMap.Find(FurnitureAct);

			const float ClosedVal = MinMax.Key + 0.15f;
			const float HalfVal = (MinMax.Value + MinMax.Key) - 0.5f;
			const float OpenedVal = MinMax.Value * 0.5;

			if (CurrPos < ClosedVal)
			{
				ASLDrawerStates::LogState(FurnitureAct, "Closed");
			}
			else if (CurrPos < HalfVal && CurrPos > ClosedVal)
			{
				ASLDrawerStates::LogState(FurnitureAct, "HalfClosed");
			}
			else if (CurrPos > OpenedVal)
			{
				ASLDrawerStates::LogState(FurnitureAct, "Opened");
			}
			else if (CurrPos < OpenedVal && CurrPos > HalfVal)
			{
				ASLDrawerStates::LogState(FurnitureAct, "HalfOpened");
			}
		}
		else if (ConstrItr->ConstraintInstance.GetAngularSwing2Motion() == EAngularConstraintMotion::ACM_Limited)
		{
			const float CurrPos = ConstrItr->ConstraintInstance.GetCurrentSwing2();
			TPair<float, float> MinMax = *DoorToMinMaxMap.Find(FurnitureAct);

			const float ClosedVal = MinMax.Key - 0.1f;
			const float HalfVal = ClosedVal * 0.5f;
			const float OpenedVal = MinMax.Value + 0.1f;

			if (CurrPos > ClosedVal)
			{
				ASLDrawerStates::LogState(FurnitureAct, "Closed");
			}
			else if (CurrPos > HalfVal && CurrPos < ClosedVal)
			{
				ASLDrawerStates::LogState(FurnitureAct, "HalfClosed");
			}
			else if (CurrPos < OpenedVal)
			{
				ASLDrawerStates::LogState(FurnitureAct, "Opened");
			}
			else if (CurrPos > OpenedVal && CurrPos < HalfVal)
			{
				ASLDrawerStates::LogState(FurnitureAct, "HalfOpened");
			}
		}
	}
}

// Log state
void ASLDrawerStates::LogState(AActor* Furniture, const FString State)
{
	// Get the previous state of the furniture
	FString PrevState = FurnitureToStateMap.FindRef(Furniture);

	// Create state if it the first
	if(PrevState.IsEmpty())
	{
		// Add to map
		FurnitureToStateMap.Add(Furniture, State);
		// Log first state, init the semantic event
		SemEventsExporter->FurnitureStateEvent(
			Furniture, State, GetWorld()->GetTimeSeconds());
	}
	else
	{
		if (PrevState.Equals(State))
		{
			// Skip if the current state is the same with the previous one
			return;
		}
		else
		{
			// Update map state
			FurnitureToStateMap.Add(Furniture, State);
			// Log state
			SemEventsExporter->FurnitureStateEvent(
				Furniture, State, GetWorld()->GetTimeSeconds());
		}
	}
}