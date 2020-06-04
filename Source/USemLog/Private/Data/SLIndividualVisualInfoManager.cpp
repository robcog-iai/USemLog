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
bool ASLIndividualVisualInfoManager::Init(bool bReset)
{
	if (bReset)
	{
		bIsInit = false;
		VisualComponents.Empty();
	}

	if (!bIsInit)
	{
		VisualComponents.Empty();
		if (GetWorld())
		{
			for (TActorIterator<AActor> ActItr(GetWorld()); ActItr; ++ActItr)
			{
				if (UActorComponent* AC = ActItr->GetComponentByClass(USLIndividualVisualInfoComponent::StaticClass()))
				{
					USLIndividualVisualInfoComponent* VIC = CastChecked<USLIndividualVisualInfoComponent>(AC);
					VIC->Init();

					if (!VIC->OnSLComponentDestroyed.IsAlreadyBound(this, &ASLIndividualVisualInfoManager::OnIndividualComponentDestroyed))
					{
						VIC->OnSLComponentDestroyed.AddDynamic(this, &ASLIndividualVisualInfoManager::OnIndividualComponentDestroyed);
					}

					if (!VIC->IsInit())
					{
						UE_LOG(LogTemp, Warning, TEXT("%s::%d Visual info component of %s is not initalized.."),
							*FString(__FUNCTION__), __LINE__, *ActItr->GetName());
					}

					VisualComponents.Emplace(VIC);
				}
			}
			bIsInit = true;
			return true;
		}
	}

	return false;
}

// Refresh all components
bool ASLIndividualVisualInfoManager::RefreshComponents()
{
	bool bMarkDirty = false;
	for (const auto& C : VisualComponents)
	{
		bMarkDirty = C->RefreshComponents() || bMarkDirty;
	}
	return bMarkDirty;
}

// Refresh only selected actors components
bool ASLIndividualVisualInfoManager::RefreshSelected(const TArray<AActor*> Owners)
{
	bool bMarkDirty = false;
	for (const auto& Act : Owners)
	{
		if (UActorComponent* AC = Act->GetComponentByClass(USLIndividualVisualInfoComponent::StaticClass()))
		{
			bMarkDirty = CastChecked<USLIndividualVisualInfoComponent>(AC)-> RefreshComponents() || bMarkDirty;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s has not visual info component.."),
				*FString(__FUNCTION__), __LINE__, *Act->GetName());
		}
	}
	return bMarkDirty;
}

// Remove destroyed individuals from array
void ASLIndividualVisualInfoManager::OnIndividualComponentDestroyed(USLIndividualVisualInfoComponent* Component)
{
	int32 Index = INDEX_NONE;
	if (VisualComponents.Find(Component, Index))
	{
		VisualComponents.RemoveAt(Index);
		UE_LOG(LogTemp, Log, TEXT("%s::%d Removing %s visual info component.., total comp num: %ld .."),
			*FString(__FUNCTION__), __LINE__, *Component->GetOwner()->GetName(), VisualComponents.Num());
	}
}
