// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Individuals/SLIndividualManager.h"
#include "Individuals/SLIndividualComponent.h"
#include "Individuals/SLIndividualUtils.h"

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

// Clear all cached references
void ASLIndividualManager::InitReset()
{
	LoadReset();
	UnbindDelegates();
	ClearCachedIndividuals();
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
	if (HasCachedIndividuals())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d The manager already has cached individual, this should not happen.."), *FString(__FUNCTION__), __LINE__);
		return false;
	}
	for (TActorIterator<AActor> ActItr(GetWorld()); ActItr; ++ActItr)
	{
		if (UActorComponent* AC = ActItr->GetComponentByClass(USLIndividualComponent::StaticClass()))
		{
			USLIndividualComponent* IC = CastChecked<USLIndividualComponent>(AC);
			if (IC->IsValidLowLevel() && !IC->IsPendingKill())
			{
				IndividualComponents.Add(IC);
			}
		}
	}
	return true;
}

//
bool ASLIndividualManager::LoadImpl()
{
	return true;
}

// Bind to the cached individual component delegates
bool ASLIndividualManager::BindDelegates()
{
	if (!HasCachedIndividuals())
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
	if (!HasCachedIndividuals())
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

// Check if there are any cached elemets
bool ASLIndividualManager::HasCachedIndividuals() const
{
	return IndividualComponents.Num() > 0;
}

// Remove any chached individuals, clear any bound delegates
void ASLIndividualManager::ClearCachedIndividuals()
{
	IndividualComponents.Empty();
}

// Remove destroyed individuals from array
void ASLIndividualManager::OnIndividualComponentDestroyed(USLIndividualComponent* DestroyedComponent)
{
	IndividualComponents.Remove(DestroyedComponent);
}
