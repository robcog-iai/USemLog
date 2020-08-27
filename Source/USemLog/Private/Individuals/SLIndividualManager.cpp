// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Individuals/SLIndividualManager.h"
#include "Individuals/SLIndividualComponent.h"
#include "Individuals/SLIndividualUtils.h"
#include "Individuals/Type/SLBaseIndividual.h"

#include "EngineUtils.h"
#include "Kismet2/ComponentEditorUtils.h"

#include "Engine/StaticMeshActor.h"
#include "Animation/SkeletalMeshActor.h"

#include "PhysicsEngine/PhysicsConstraintActor.h"
#include "Vision/SLVirtualCameraView.h"

// Sets default values
ASLIndividualManager::ASLIndividualManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bIsInit = false;
	bIsLoaded = false;
	bIsConnected = false;
}

// Called when the game starts or when spawned
void ASLIndividualManager::BeginPlay()
{
	Super::BeginPlay();	
}

// Set references
bool ASLIndividualManager::Init(bool bReset)
{
	if (bReset)
	{
		InitReset();
	}

	if (IsInit())
	{
		return true;
	}

	SetIsInit(InitImpl());
	return IsInit();
}

// 
bool ASLIndividualManager::Load(bool bReset)
{
	if (bReset)
	{
		LoadReset();
	}

	if (IsLoaded())
	{
		return true;
	}

	if (!IsInit())
	{
		if (!Init(bReset))
		{
			return false;
		}
	}

	SetIsLoaded(LoadImpl());
	return IsLoaded();
}

// Listen to individual component delegates
bool ASLIndividualManager::Connect()
{
	if (IsConnected())
	{
		return true;
	}
	SetIsConnected(BindDelegates());
	return IsConnected();
}

// Get the individual from the unique id
USLBaseIndividual* ASLIndividualManager::GetIndividual(const FString& Id)
{
	if (auto Ind = IdToIndividuals.Find(Id))
	{
		return *Ind;
	}
	return nullptr;
}

// Get the individual component from the unique id
USLIndividualComponent* ASLIndividualManager::GetIndividualComponent(const FString& Id)
{
	if (auto Ind = IdToIndividualComponents.Find(Id))
	{
		return *Ind;
	}
	return nullptr;
}

// Get the individual component owner from the unique id
AActor* ASLIndividualManager::GetIndividualActor(const FString& Id)
{
	if (auto Ind = IdToIndividualComponents.Find(Id))
	{
		return (*Ind)->GetOwner();
	}
	return nullptr;
}

// Clear all cached references
void ASLIndividualManager::InitReset()
{
	LoadReset();
	UnbindDelegates();
	ClearCache();
	SetIsInit(false);
}

//
void ASLIndividualManager::LoadReset()
{
	SetIsLoaded(false);
}

// Set state to init
void ASLIndividualManager::SetIsInit(bool bNewValue, bool bBroadcast)
{
	if (bIsInit != bNewValue)
	{
		if (!bNewValue)
		{
			SetIsLoaded(false);
		}

		bIsInit = bNewValue;
		if (bBroadcast)
		{
			//OnInitChanged.Broadcast(this, bNewValue);
		}
	}
}

// Set state to loaded
void ASLIndividualManager::SetIsLoaded(bool bNewValue, bool bBroadcast)
{
	if (bIsLoaded != bNewValue)
	{
		bIsLoaded = bNewValue;
		if (bBroadcast)
		{
			//OnLoadedChanged.Broadcast(this, bNewValue);
		}
	}
}

// Set state to connected
void ASLIndividualManager::SetIsConnected(bool bNewValue, bool bBroadcast)
{
	if (bIsConnected != bNewValue)
	{
		bIsConnected = bNewValue;
		if (bBroadcast)
		{
			//OnConnectedChanged.Broadcast(this, bNewValue);
		}
	}
}

/* Private */
// Cache individual component references
bool ASLIndividualManager::InitImpl()
{	
	if (HasCache())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d The manager already has cached individuals, this should not happen.."), *FString(__FUNCTION__), __LINE__);
		return false;
	}
	for (TActorIterator<AActor> ActItr(GetWorld()); ActItr; ++ActItr)
	{
		if (UActorComponent* AC = ActItr->GetComponentByClass(USLIndividualComponent::StaticClass()))
		{			
			USLIndividualComponent* IC = CastChecked<USLIndividualComponent>(AC);
			if (IC->IsValidLowLevel() && !IC->IsPendingKill())
			{
				AddToCache(IC);
			}
		}
	}
	return HasCache();
}

//
bool ASLIndividualManager::LoadImpl()
{
	// Make sure all individuals and their components are loaded
	bool bAllLoaded = true;

	for (const auto& IC : IndividualComponents)
	{
		if (!IC->IsLoaded())
		{
			bAllLoaded = false;
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s is not loaded.."), *FString(__FUNCTION__), __LINE__, *IC->GetFullName());
		}
	}

	for (const auto& IO : Individuals)
	{
		if (!IO->IsLoaded())
		{
			bAllLoaded = false;
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s is not loaded.."), *FString(__FUNCTION__), __LINE__, *IO->GetFullName());
		}
	}

	return bAllLoaded;
}

// Bind to the cached individual component delegates
bool ASLIndividualManager::BindDelegates()
{
	if (!HasCache())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d No cached individuals, cannot bind any delegates"), *FString(__FUNCTION__), __LINE__);
		return false;
	}

	for (const auto& IC : IndividualComponents)
	{
		if (!IC->OnDestroyed.IsAlreadyBound(this, &ASLIndividualManager::OnIndividualComponentDestroyed))
		{
			IC->OnDestroyed.AddDynamic(this, &ASLIndividualManager::OnIndividualComponentDestroyed);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's on destroyed delegate is already bound with the individual manager, this should not happen.."),
				*FString(__FUNCTION__), __LINE__, *IC->GetFullName());
		}
	}
	return true;
}

// Remove bounds from the cached individuals
bool ASLIndividualManager::UnbindDelegates()
{
	if (!HasCache())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d No cached individuals, cannot un-bind any delegates"), *FString(__FUNCTION__), __LINE__);
		return false;
	}

	for (const auto& IC : IndividualComponents)
	{
		if (IC->OnDestroyed.IsAlreadyBound(this, &ASLIndividualManager::OnIndividualComponentDestroyed))
		{
			IC->OnDestroyed.RemoveDynamic(this, &ASLIndividualManager::OnIndividualComponentDestroyed);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's on destroyed delegate was not bound with the individual manager, this should not happen.."),
				*FString(__FUNCTION__), __LINE__, *IC->GetFullName());
		}
	}
	return true;
}

// Check if there are any cached individuals
bool ASLIndividualManager::HasCache() const
{
	return IndividualComponents.Num() > 0
		|| Individuals.Num() > 0
		|| IdToIndividualComponents.Num() > 0
		|| IdToIndividuals.Num() > 0;
}

// Remove any chached individuals, clear any bound delegates
void ASLIndividualManager::ClearCache()
{
	IndividualComponents.Empty();
	Individuals.Empty();
	IdToIndividuals.Empty();
	IdToIndividualComponents.Empty();
	if (HasCache())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Somethig went wrong on clearing the cache.."), *FString(__FUNCTION__), __LINE__);
	}
}

// Add individual info to cache
void ASLIndividualManager::AddToCache(USLIndividualComponent* IC)
{
	IndividualComponents.Add(IC);

	// Add to quick access maps
	if (auto Individual = IC->GetIndividualObject())
	{
		Individuals.Add(Individual);
		const FString Id = Individual->GetIdValue();
		IdToIndividuals.Add(Id, Individual);
		IdToIndividualComponents.Add(Id, IC);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no valid individual object.."), *FString(__FUNCTION__), __LINE__, *IC->GetFullName());
	}

	// Add children to quick access maps
	for (const auto& Child : IC->GetIndividualChildren())
	{
		Individuals.Add(Child);
		const FString Id = Child->GetIdValue();
		IdToIndividuals.Add(Id, Child);
		IdToIndividualComponents.Add(Id, IC);
	}
}

// Remove destroyed individuals from array
void ASLIndividualManager::OnIndividualComponentDestroyed(USLIndividualComponent* DestroyedComponent)
{
	IndividualComponents.Remove(DestroyedComponent);
	
	// Remove from quick access maps
	if (auto Individual = DestroyedComponent->GetIndividualObject())
	{
		const FString Id = Individual->GetIdValue();
		IdToIndividuals.Remove(Id);
		IdToIndividualComponents.Remove(Id);
	}

	// Remove children from quick access maps
	for (const auto& Child : DestroyedComponent->GetIndividualChildren())
	{
		const FString ChildId = Child->GetIdValue();
		IdToIndividuals.Remove(ChildId);
		IdToIndividualComponents.Remove(ChildId);
	}
}
