// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Data/SLIndividualVisualInfoManager.h"
#include "Data/SLIndividualVisualInfoComponent.h"
#include "EngineUtils.h"

// Sets default values
ASLIndividualVisualInfoManager::ASLIndividualVisualInfoManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bIsInit = false;

}

// Called when the game starts or when spawned
void ASLIndividualVisualInfoManager::BeginPlay()
{
	Super::BeginPlay();	
}

// Called every frame
void ASLIndividualVisualInfoManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Load components from world
int32 ASLIndividualVisualInfoManager::Init(bool bReset)
{
	if (bReset)
	{
		bIsInit = false;
		int32 NumClearedComp = ClearCache();
		UE_LOG(LogTemp, Log, TEXT("%s::%d Reset: %ld components cleared.."),
			*FString(__FUNCTION__), __LINE__, NumClearedComp);
	}

	int32 NumComponentsLoaded = 0;
	if (!bIsInit)
	{
		if (GetWorld())
		{
			for (TActorIterator<AActor> ActItr(GetWorld()); ActItr; ++ActItr)
			{
				if (USLIndividualVisualInfoComponent* IC = GetInfoComponent(*ActItr))
				{
					if (IC->Init())
					{
						if (!IC->Load())
						{
							UE_LOG(LogTemp, Warning, TEXT("%s::%d Individual component %s could not be loaded.."),
								*FString(__FUNCTION__), __LINE__, *IC->GetOwner()->GetName());
						}

						if (RegisterInfoComponent(IC))
						{
							NumComponentsLoaded++;
						}
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("%s::%d Info component %s could not be init.. the manager will not register it.."),
							*FString(__FUNCTION__), __LINE__, *IC->GetOwner()->GetName());
					}
				}
			}
			bIsInit = true;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("%s::%d Init: %ld info components loaded.."),
		*FString(__FUNCTION__), __LINE__, NumComponentsLoaded);
	return NumComponentsLoaded;
}

int32 ASLIndividualVisualInfoManager::AddVisualInfoComponents()
{
	return int32();
}

int32 ASLIndividualVisualInfoManager::AddVisualInfoComponents(const TArray<AActor*>& Actors)
{
	return int32();
}

int32 ASLIndividualVisualInfoManager::DestroyVisualInfoComponents()
{
	return int32();
}

int32 ASLIndividualVisualInfoManager::DestroyVisualInfoComponents(const TArray<AActor*>& Actors)
{
	return int32();
}

// Refresh all components
int32 ASLIndividualVisualInfoManager::RefreshVisualInfoComponents()
{
	return 0;
}

// Refresh only selected actors components
int32 ASLIndividualVisualInfoManager::RefreshVisualInfoComponents(const TArray<AActor*>& Owners)
{
	return 0;
}

int32 ASLIndividualVisualInfoManager::ToggleVisualInfoComponents()
{
	return int32();
}

int32 ASLIndividualVisualInfoManager::ToggleVisualInfoComponents(const TArray<AActor*>& Owners)
{
	return int32();
}

// Remove destroyed individuals from array
void ASLIndividualVisualInfoManager::OnIndividualComponentDestroyed(USLIndividualVisualInfoComponent* Component)
{
	if (RegisteredInfoComponents.Remove(Component))
	{
		RegisteredInfoComponents.Remove(Component);
		UE_LOG(LogTemp, Log, TEXT("%s::%d Removing %s's visual info component.., total comp num: %ld .."),
			*FString(__FUNCTION__), __LINE__, *Component->GetOwner()->GetName(), RegisteredInfoComponents.Num());
	}
}

// Find the individual component of the actor, return nullptr if none found
USLIndividualVisualInfoComponent* ASLIndividualVisualInfoManager::GetInfoComponent(AActor* Actor) const
{
	if (UActorComponent* AC = Actor->GetComponentByClass(USLIndividualVisualInfoComponent::StaticClass()))
	{
		return CastChecked<USLIndividualVisualInfoComponent>(AC);
	}
	return nullptr;
}

USLIndividualVisualInfoComponent* ASLIndividualVisualInfoManager::AddNewInfoComponent(AActor* Actor)
{
	return nullptr;
}

bool ASLIndividualVisualInfoManager::CanHaveInfoComponent(AActor* Actor)
{
	return false;
}

void ASLIndividualVisualInfoManager::DestroyInfoComponent(USLIndividualVisualInfoComponent* Component)
{
}

bool ASLIndividualVisualInfoManager::RegisterInfoComponent(USLIndividualVisualInfoComponent* Component)
{
	return false;
}

bool ASLIndividualVisualInfoManager::UnregisterInfoComponent(USLIndividualVisualInfoComponent* Component)
{
	return false;
}

// Unregister and clear all cached components (return the number of cleared components)
int32 ASLIndividualVisualInfoManager::ClearCache()
{
	return 0;
}
