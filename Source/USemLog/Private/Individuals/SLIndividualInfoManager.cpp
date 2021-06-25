// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Individuals/SLIndividualInfoManager.h"
#include "Individuals/SLIndividualInfoComponent.h"
#include "EngineUtils.h"
#if WITH_EDITOR
#include "LevelEditorViewport.h"
#include "Editor.h"
#endif //WITH_EDITOR

// Sets default values
ASLIndividualInfoManager::ASLIndividualInfoManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
#if WITH_EDITOR
	PrimaryActorTick.bStartWithTickEnabled = false;
#endif // WITH_EDITOR

	// Slow update rate
	SetActorTickInterval(0.25f);

	bIsInit = false;
	bIsLoaded = false;
	bIsConnected = false;

	/* Buttons hack */
	bToggleTickUpdate = false;

#if WITH_EDITORONLY_DATA
	// Make manager sprite smaller (used to easily find the actor in the world)
	SpriteScale = 0.35;
#endif // WITH_EDITORONLY_DATA
}

// Called when the game starts or when spawned
void ASLIndividualInfoManager::BeginPlay()
{
	Super::BeginPlay();
}


#if WITH_EDITOR
// Called when a property is changed in the editor
void ASLIndividualInfoManager::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property name
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ?
		PropertyChangedEvent.Property->GetFName() : NAME_None;

	/* Buttons hack */
	//if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLIndividualInfoManager, bToggleTickUpdate))
	//{
	//	bToggleTickUpdate = false;
	//	ToggleTickUpdate();
	//}
}
#endif // WITH_EDITOR

// Called every frame
void ASLIndividualInfoManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FRotator LookAtRot;

	// True if we are in the editor (this is still true when using Play In Editor). You may want to use GWorld->HasBegunPlay in that case)	
	if (GIsEditor)
	{
#if WITH_EDITOR
		for (FLevelEditorViewportClient* LevelVC : GEditor->GetLevelViewportClients())
		{
			if (LevelVC && LevelVC->IsPerspective())
			{
				LookAtRot = LevelVC->GetViewRotation() + FRotator(180.f, 0.f, 180.f);
				break;
			}
		}
#endif //WITH_EDITOR
	}
}

// If true, actor is ticked even if TickType == LEVELTICK_ViewportsOnly
bool ASLIndividualInfoManager::ShouldTickIfViewportsOnly() const
{
	return true;
}

// Load components from world
bool ASLIndividualInfoManager::Init(bool bReset)
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
bool ASLIndividualInfoManager::Load(bool bReset)
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
bool ASLIndividualInfoManager::Connect()
{
	if (IsConnected())
	{
		return true;
	}
	SetIsConnected(BindDelegates());
	return IsConnected();
}


// Enable/disable tick
void ASLIndividualInfoManager::ToggleTickUpdate()
{
	SetActorTickEnabled(!IsActorTickEnabled());
}

// Clear all cached references
void ASLIndividualInfoManager::InitReset()
{
	LoadReset();
	UnbindDelegates();
	ClearCachedIndividualInfoComponents();
	SetIsInit(false);
}

//
void ASLIndividualInfoManager::LoadReset()
{
	SetIsLoaded(false);
}

// Set the init flag, broadcast on new value
void ASLIndividualInfoManager::SetIsInit(bool bNewValue, bool bBroadcast)
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

// Set the loaded flag, broadcast on new value
void ASLIndividualInfoManager::SetIsLoaded(bool bNewValue, bool bBroadcast)
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

// Set the connected flag, broadcast on new value
void ASLIndividualInfoManager::SetIsConnected(bool bNewValue, bool bBroadcast)
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

// Cache references
bool ASLIndividualInfoManager::InitImpl()
{
	if (HasCachedIndividualInfoComponents())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d The manager already has cached individuals info, this should not happen.."), *FString(__FUNCTION__), __LINE__);
		return false;
	}
	for (TActorIterator<AActor> ActItr(GetWorld()); ActItr; ++ActItr)
	{
		if (UActorComponent* AC = ActItr->GetComponentByClass(USLIndividualInfoComponent::StaticClass()))
		{
			USLIndividualInfoComponent* IC = CastChecked<USLIndividualInfoComponent>(AC);
			if (IC->IsValidLowLevel() && !IC->IsPendingKill())
			{
				IndividualInfoComponents.Add(IC);
			}
		}
	}
	return true;
}

// Load values
bool ASLIndividualInfoManager::LoadImpl()
{
	return true;
}

// Bind to the cached individual component delegates
bool ASLIndividualInfoManager::BindDelegates()
{
	if (!HasCachedIndividualInfoComponents())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d No cached individuals, cannot bind any delegates"), *FString(__FUNCTION__), __LINE__);
		return false;
	}

	for (const auto& IC : IndividualInfoComponents)
	{
		if (!IC->OnDestroyed.IsAlreadyBound(this, &ASLIndividualInfoManager::OnIndividualInfoComponentDestroyed))
		{
			IC->OnDestroyed.AddDynamic(this, &ASLIndividualInfoManager::OnIndividualInfoComponentDestroyed);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's on destroyed delegate is already bound with the individual info manager, this should not happen.."),
				*FString(__FUNCTION__), __LINE__, *IC->GetFullName());
		}
	}
	return true;
}

// Remove bounds from the cached individuals
bool ASLIndividualInfoManager::UnbindDelegates()
{
	if (!HasCachedIndividualInfoComponents())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d No cached individuals, cannot un-bind any delegates"), *FString(__FUNCTION__), __LINE__);
		return false;
	}

	for (const auto& IC : IndividualInfoComponents)
	{
		if (IC->OnDestroyed.IsAlreadyBound(this, &ASLIndividualInfoManager::OnIndividualInfoComponentDestroyed))
		{
			IC->OnDestroyed.RemoveDynamic(this, &ASLIndividualInfoManager::OnIndividualInfoComponentDestroyed);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d %s's on destroyed delegate was not bound with the individual info manager, this should not happen.."),
				*FString(__FUNCTION__), __LINE__, *IC->GetFullName());
		}
	}
	return true;
}

// Check if there are any cached elements
bool ASLIndividualInfoManager::HasCachedIndividualInfoComponents() const
{
	return IndividualInfoComponents.Num() > 0;
}

// Remove any chached components
void ASLIndividualInfoManager::ClearCachedIndividualInfoComponents()
{
	IndividualInfoComponents.Empty();
}

/* Delegate functions */
// Remove destroyed individuals from array
void ASLIndividualInfoManager::OnIndividualInfoComponentDestroyed(USLIndividualInfoComponent* DestroyedComponent)
{
	IndividualInfoComponents.Remove(DestroyedComponent);
}