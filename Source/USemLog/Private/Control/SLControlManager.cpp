// Fill out your copyright notice in the Description page of Project Settings.


#include "Control/SLControlManager.h"
#include "Individuals/SLIndividualManager.h"
#include "Individuals/Type/SLBaseIndividual.h"
#include "EngineUtils.h"
#include "TimerManager.h"

ASLControlManager::ASLControlManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ASLControlManager::Init()
{
	if (bIsInit)
	{
		UE_LOG(LogTemp, Log, TEXT("%s::%d Control manager (%s) is already init.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	bool RetValue = true;
	if (!SetIndividualManager())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Control manager (%s) could not set the individual manager.."),
			*FString(__FUNCTION__), __LINE__, *GetName());
		RetValue = false;
	}
    if (!IndividualManager->Load(false))
    {
        UE_LOG(LogTemp, Error, TEXT("%s::%d Control manager (%s) could not load the individual manager (%s).."),
            *FString(__FUNCTION__), __LINE__, *GetName(), *IndividualManager->GetName());
        RetValue = false;
    }
	bIsInit = RetValue;
}

bool ASLControlManager::SetIndividualPose(const FString& Id, FVector Location, FQuat Quat)
{
	USLBaseIndividual* Individual = IndividualManager->GetIndividual(Id);
	if (Individual == nullptr)
	{
		UE_LOG(LogTemp, Log, TEXT("%s::%d Individual (%s) does not exist.."),
			*FString(__FUNCTION__), __LINE__, *Id);
		return false;
	}

	AActor* ActorToMove = Individual->GetParentActor();

	Quat.Normalize();
	ActorToMove->SetActorLocationAndRotation(Location, Quat);
	return true;
}

bool ASLControlManager::ApplyForceTo(const FString& Id, FVector Force)
{
	USLBaseIndividual* Individual = IndividualManager->GetIndividual(Id);
	if (Individual == nullptr)
	{
		UE_LOG(LogTemp, Log, TEXT("%s::%d Individual (%s) does not exist.."),
			*FString(__FUNCTION__), __LINE__, *Id);
		return false;
	}

	AActor* ActorToApply = Individual->GetParentActor();
	UStaticMeshComponent* StaticMesh = Cast<UStaticMeshComponent>(ActorToApply->GetComponentByClass(UStaticMeshComponent::StaticClass()));
	StaticMesh->AddForce(Force * 100.f * StaticMesh->GetMass());
	return true;
}

bool ASLControlManager::StartSimulationSelectionOnly(const TArray<FString>& Ids, int32 Seconds)
{
	for (FString Id : Ids)
	{
		USLBaseIndividual* Individual = IndividualManager->GetIndividual(Id);
		if (Individual == nullptr)
		{
			UE_LOG(LogTemp, Log, TEXT("%s::%d Individual (%s) does not exist.."),
				*FString(__FUNCTION__), __LINE__, *GetName());
			continue;
		}

		AActor* Actor = Individual->GetParentActor();
		UStaticMeshComponent* StaticMesh = Cast<UStaticMeshComponent>(Actor->GetComponentByClass(UStaticMeshComponent::StaticClass()));
		StaticMesh->SetSimulatePhysics(true);
	}

	if (Seconds > 0)
	{
		FTimerHandle TimerHandle;
		FTimerDelegate TimerDelegateDelay;
		TimerDelegateDelay.BindLambda([this](TArray<FString> Ids) {
			StopSimulationSelectionOnly(Ids);
		}, Ids);
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegateDelay, Seconds, false);

	}
	
	OnSimulationStart.ExecuteIfBound();

	return true;
}

// Stop physics simulation on individuals without delay
bool ASLControlManager::StopSimulationSelectionOnly(const TArray<FString>& Ids)
{		
	for (FString Id : Ids)
	{
		USLBaseIndividual* Individual = IndividualManager->GetIndividual(Id);
		if (Individual == nullptr)
		{
			UE_LOG(LogTemp, Log, TEXT("%s::%d Individual (%s) does not exist.."),
				*FString(__FUNCTION__), __LINE__, *GetName());
			continue;
		}

		AActor* Actor = Individual->GetParentActor();
		UStaticMeshComponent* StaticMesh = Cast<UStaticMeshComponent>(Actor->GetComponentByClass(UStaticMeshComponent::StaticClass()));
		StaticMesh->SetSimulatePhysics(false);
	}
	OnSimulationFinish.ExecuteIfBound();
	
	return true;
}

/* Managers */
// Get the individual manager from the world (or spawn a new one)
bool ASLControlManager::SetIndividualManager()
{
	if (IndividualManager && IndividualManager->IsValidLowLevel() && !IndividualManager->IsPendingKillOrUnreachable())
	{
		return true;
	}

	for (TActorIterator<ASLIndividualManager>Iter(GetWorld()); Iter; ++Iter)
	{
		if ((*Iter)->IsValidLowLevel() && !(*Iter)->IsPendingKillOrUnreachable())
		{
			IndividualManager = *Iter;
			return true;
		}
	}

	// Spawning a new manager
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = TEXT("SL_IndividualManager");
	IndividualManager = GetWorld()->SpawnActor<ASLIndividualManager>(SpawnParams);
#if WITH_EDITOR
	IndividualManager->SetActorLabel(TEXT("SL_IndividualManager"));
#endif // WITH_EDITOR
	return true;
}