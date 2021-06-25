// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Individuals/SLIndividualManager.h"
#include "Individuals/SLIndividualComponent.h"
#include "Individuals/SLIndividualUtils.h"
#include "Individuals/Type/SLBaseIndividual.h"
#include "Individuals/Type/SLSkeletalIndividual.h"
#include "Individuals/Type/SLRobotIndividual.h"

#include "EngineUtils.h"

#include "Engine/StaticMeshActor.h"
#include "Animation/SkeletalMeshActor.h"

#include "PhysicsEngine/PhysicsConstraintActor.h"
#include "Vision/SLVirtualCameraView.h"

#if WITH_EDITOR
#include "Components/BillboardComponent.h"
#endif // WITH_EDITOR

// Sets default values
ASLIndividualManager::ASLIndividualManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bIsInit = false;
	bIsLoaded = false;
	bIsConnected = false;

	bThreadSafeToRead = true;

#if WITH_EDITORONLY_DATA
	// Make manager sprite smaller (used to easily find the actor in the world)
	SpriteScale = 0.35;
	ConstructorHelpers::FObjectFinderOptional<UTexture2D> SpriteTexture(TEXT("/USemLog/Sprites/S_SLIndividuals"));
	GetSpriteComponent()->Sprite = SpriteTexture.Get();
#endif // WITH_EDITORONLY_DATA
}

// Called when the game starts or when spawned
void ASLIndividualManager::BeginPlay()
{
	Super::BeginPlay();	
}

#if WITH_EDITOR
// Called when a property is changed in the editor
void ASLIndividualManager::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property name
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ?
		PropertyChangedEvent.Property->GetFName() : NAME_None;

	/* Button hacks */
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLIndividualManager, bInitButton))
	{
		bInitButton = false;
		Init(bResetButton);
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLIndividualManager, bLoadButton))
	{
		bLoadButton = false;
		Load(bResetButton);
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLIndividualManager, bResetButton))
	{
		bResetButton = false;
		InitReset();
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLIndividualManager, bFindIdButton))
	{
		bFindIdButton = false;
		if (FindIdValue.IsEmpty())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Id value is empty, set value first.. "), *FString(__FUNCTION__), __LINE__);
			return;
		}
		
		if (auto Individual = GetIndividual(FindIdValue))
		{
			if (AActor* ParentActor = Individual->GetParentActor())
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d Individual %s ParentActor=%s.. "),
					*FString(__FUNCTION__), __LINE__, *FindIdValue, *ParentActor->GetName());
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d Individual %s has no valid parent actor set, OuterOuter=%s.. "),
					*FString(__FUNCTION__), __LINE__, *FindIdValue, *Individual->GetOuter()->GetOuter()->GetName());
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Individual %s could not be found.. "), *FString(__FUNCTION__), __LINE__, *FindIdValue);
		}

	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLIndividualManager, bToggleVisualMaskVisiblityButton))
	{
		bToggleVisualMaskVisiblityButton = false;
		for (const auto& IC : IndividualComponents)
		{
			bool bIncludeChildren = true;
			IC->ToggleVisualMaskVisibility(bIncludeChildren);
		}
	}

}
#endif // WITH_EDITOR

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

// Spawn or get manager from the world
ASLIndividualManager* ASLIndividualManager::GetExistingOrSpawnNew(UWorld* World)
{
	// Check in world
	for (TActorIterator<ASLIndividualManager>Iter(World); Iter; ++Iter)
	{
		if ((*Iter)->IsValidLowLevel() && !(*Iter)->IsPendingKillOrUnreachable())
		{
			return *Iter;
		}
	}

	// Spawning a new manager
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = TEXT("SL_IndividualManager");
	auto Manager = World->SpawnActor<ASLIndividualManager>(SpawnParams);
#if WITH_EDITOR
	Manager->SetActorLabel(TEXT("SL_IndividualManager"));
#endif // WITH_EDITOR
	return Manager;
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
	bool bAllInit = true;
	//if (HasCache())
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("%s::%d The manager already has cached individuals, this should not happen.."), *FString(__FUNCTION__), __LINE__);
	//	return false;
	//}
	for (TActorIterator<AActor> ActItr(GetWorld()); ActItr; ++ActItr)
	{
		if (UActorComponent* AC = ActItr->GetComponentByClass(USLIndividualComponent::StaticClass()))
		{			
			USLIndividualComponent* IC = CastChecked<USLIndividualComponent>(AC);
			if (IC->IsValidLowLevel() && !IC->IsPendingKill())
			{
				AddToCache(IC);
				if (!IC->IsInit())
				{
					bAllInit = false;
					UE_LOG(LogTemp, Error, TEXT("%s::%d %s is not init.."), *FString(__FUNCTION__), __LINE__, *IC->GetFullName());
				}
			}
		}
	}
	//return HasCache();
	return bAllInit;
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
	return IndividualComponents.Num() > 0 || Individuals.Num() > 0;
}

// Remove any chached individuals, clear any bound delegates
void ASLIndividualManager::ClearCache()
{
	bThreadSafeToRead = false;

	IndividualComponents.Empty();
	Individuals.Empty();

	/* World state logger convenience containers */
	MovableIndividuals.Empty();
	ChildlessRootIndividuals.Empty();
	SkeletalIndividuals.Empty();
	RobotIndividuals.Empty();

	/* Quick acess id based mapping*/
	IdToIndividuals.Empty();
	IdToIndividualComponents.Empty();
	if (HasCache())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Somethig went wrong on clearing the cache.."), *FString(__FUNCTION__), __LINE__);
	}

	bThreadSafeToRead = true;
}

// Add individual info to cache
void ASLIndividualManager::AddToCache(USLIndividualComponent* IC)
{
	bThreadSafeToRead = false;

	IndividualComponents.Add(IC);

	// Add individuals
	if (auto Individual = IC->GetIndividualObject())
	{
		if (!Individual->Init(false))
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is not init.."), *FString(__FUNCTION__), __LINE__, *Individual->GetFullName());
		}
		Individuals.Add(Individual);

		/* Id based quick acess */
		const FString Id = Individual->GetIdValue();
		IdToIndividuals.Add(Id, Individual);
		IdToIndividualComponents.Add(Id, IC);

		/* World state logger */
		if (Individual->IsMovable())
		{
			MovableIndividuals.Add(Individual);
		}

		if (auto AsSkelIndividual = Cast<USLSkeletalIndividual>(Individual))
		{
			SkeletalIndividuals.Add(AsSkelIndividual);
		}
		else if (auto AsRobotIndividual = Cast<USLRobotIndividual>(Individual))
		{
			RobotIndividuals.Add(AsRobotIndividual);
		}
		else
		{
			ChildlessRootIndividuals.Add(Individual);
		}

	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no valid individual object.."), *FString(__FUNCTION__), __LINE__, *IC->GetFullName());
	}

	// Add children individuals
	for (const auto& Child : IC->GetIndividualChildren())
	{
		if (!Child->Init(false))
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is not init.."), *FString(__FUNCTION__), __LINE__, *Child->GetFullName());
		}
		Individuals.Add(Child);

		/* World state logger */
		if (Child->IsMovable())
		{
			MovableIndividuals.Add(Child);
		}

		/* Id based quick access */
		const FString Id = Child->GetIdValue();
		IdToIndividuals.Add(Id, Child);
		IdToIndividualComponents.Add(Id, IC);
	}

	bThreadSafeToRead = true;
}

// Remove from cache
bool ASLIndividualManager::RemoveFromCache(USLIndividualComponent* IC)
{
	bThreadSafeToRead = false;

	bool bAnyRemoved = false;

	if (IndividualComponents.Remove(IC) != INDEX_NONE)
	{
		bAnyRemoved = true;
	}

	// Remove from quick access maps
	if (auto Individual = IC->GetIndividualObject())
	{
		Individuals.Remove(Individual);

		/* Id based quick acess */
		const FString Id = Individual->GetIdValue();
		IdToIndividuals.Remove(Id);
		IdToIndividualComponents.Remove(Id);

		/* World state logger */
		MovableIndividuals.Remove(Individual);
		if (auto AsSkelIndividual = Cast<USLSkeletalIndividual>(Individual))
		{
			SkeletalIndividuals.Remove(AsSkelIndividual);
		}
		else if (auto AsRobotIndividual = Cast<USLRobotIndividual>(Individual))
		{
			RobotIndividuals.Remove(AsRobotIndividual);
		}
		else
		{
			ChildlessRootIndividuals.Remove(Individual);
		}
	}

	// Remove children from quick access maps
	for (const auto& Child : IC->GetIndividualChildren())
	{
		Individuals.Remove(Child);

		/* World state logger */
		MovableIndividuals.Remove(Child);

		/* Id based quick acess */
		const FString ChildId = Child->GetIdValue();
		IdToIndividuals.Remove(ChildId);
		IdToIndividualComponents.Remove(ChildId);
	}

	bThreadSafeToRead = true;

	return bAnyRemoved;
}

// Remove destroyed individuals from array
void ASLIndividualManager::OnIndividualComponentDestroyed(USLIndividualComponent* DestroyedComponent)
{
	RemoveFromCache(DestroyedComponent);
}
