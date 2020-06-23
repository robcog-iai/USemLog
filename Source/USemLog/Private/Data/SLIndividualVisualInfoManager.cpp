// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Data/SLIndividualVisualInfoManager.h"
#include "Data/SLIndividualVisualInfoComponent.h"
#include "Data/SLIndividualComponent.h"
#include "EngineUtils.h"
#include "Kismet2/ComponentEditorUtils.h"
#if WITH_EDITOR
#include "LevelEditorViewport.h"
#include "Editor.h"
#endif //WITH_EDITOR

// Sets default values
ASLIndividualVisualInfoManager::ASLIndividualVisualInfoManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
#if WITH_EDITOR
	PrimaryActorTick.bStartWithTickEnabled = false;
#endif // WITH_EDITOR

	// Slow update rate
	SetActorTickInterval(0.25f);

	bIsInit = false;

	/* Buttons hack */
	bToggleTickUpdate = false;
}


#if WITH_EDITOR
// Called when a property is changed in the editor
void ASLIndividualVisualInfoManager::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property name
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ?
		PropertyChangedEvent.Property->GetFName() : NAME_None;

	/* Buttons hack */
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLIndividualVisualInfoManager, bToggleTickUpdate))
	{
		bToggleTickUpdate = false;
		ToggleTickUpdate();
	}
}
#endif // WITH_EDITOR

// Called every frame
void ASLIndividualVisualInfoManager::Tick(float DeltaTime)
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

	for (const auto C : RegisteredInfoComponents)
	{
		C->SetWorldRotation(LookAtRot);
	}
}

// If true, actor is ticked even if TickType == LEVELTICK_ViewportsOnly
bool ASLIndividualVisualInfoManager::ShouldTickIfViewportsOnly() const
{
	return true;
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

	int32 NumComp = 0;
	if (!bIsInit)
	{
		if (GetWorld())
		{
			for (TActorIterator<AActor> ActItr(GetWorld()); ActItr; ++ActItr)
			{
				if (USLIndividualVisualInfoComponent* IC = GetInfoComponent(*ActItr))
				{
					if (IC->IsInit() || IC->Init())
					{
						//if (!IC->IsLoaded() && !IC->Load())
						//{
						//	UE_LOG(LogTemp, Warning, TEXT("%s::%d Individual component %s could not be loaded.."),
						//		*FString(__FUNCTION__), __LINE__, *IC->GetOwner()->GetName());
						//}

						if (RegisterInfoComponent(IC))
						{
							NumComp++;
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

	UE_LOG(LogTemp, Log, TEXT("%s::%d Init: registered %ld info components.."),
		*FString(__FUNCTION__), __LINE__, NumComp);
	return NumComp;
}

// Add new visual info components to all actors
int32 ASLIndividualVisualInfoManager::AddVisualInfoComponents()
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Init manager first.."), *FString(__FUNCTION__), __LINE__);
		return INDEX_NONE;
	}

	int32 Num = 0;
	if (GetWorld())
	{
		for (TActorIterator<AActor> ActItr(GetWorld()); ActItr; ++ActItr)
		{
			if (USLIndividualVisualInfoComponent* IC = AddNewInfoComponent(*ActItr))
			{
				if (RegisterInfoComponent(IC))
				{
					Num++;
				}
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not access the world.."), *FString(__FUNCTION__), __LINE__);
	}
	return Num;
}

// Add new visual info components to selection actors
int32 ASLIndividualVisualInfoManager::AddVisualInfoComponents(const TArray<AActor*>& Actors)
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Init manager first.."), *FString(__FUNCTION__), __LINE__);
		return INDEX_NONE;
	}

	int32 Num = 0;
	for (const auto& Act : Actors)
	{
		if (USLIndividualVisualInfoComponent* IC = AddNewInfoComponent(Act))
		{
			if (RegisterInfoComponent(IC))
			{
				Num++;
			}
		}
	}
	return Num;
}

// Remove all info components from world and clear manager's cache
int32 ASLIndividualVisualInfoManager::DestroyVisualInfoComponents()
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Init manager first.."), *FString(__FUNCTION__), __LINE__);
		return INDEX_NONE;
	}

	int32 Num = RegisteredInfoComponents.Num();

	// Removes component from the actors instanced components, triggers destroy
	for (const auto& IC : RegisteredInfoComponents)
	{
		DestroyInfoComponent(IC);
	}

	// Clear components from the managers cache
	ClearCache();

	return Num;
}

// Remove selected info components from world and clear manager's cache
int32 ASLIndividualVisualInfoManager::DestroyVisualInfoComponents(const TArray<AActor*>& Actors)
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Init manager first.."), *FString(__FUNCTION__), __LINE__);
		return INDEX_NONE;
	}

	int32 Num = 0;
	for (const auto& Act : Actors)
	{
		if (auto** FoundIC = InfoComponentOwners.Find(Act))
		{
			// Remove component from cache, then destroy it
			UnregisterInfoComponent(*FoundIC);
			DestroyInfoComponent(*FoundIC);
			Num++;
		}
	}
	return Num;
}

// Refresh all components
int32 ASLIndividualVisualInfoManager::ResetVisualInfoComponents()
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Init manager first.."), *FString(__FUNCTION__), __LINE__);
		return INDEX_NONE;
	}

	int32 Num = 0;
	for (const auto& IC : RegisteredInfoComponents)
	{
		bool bReset = true;
		if (IC->Init(bReset) && IC->Load(bReset))
		{
			Num++;
		}
		else
		{
			//UnregisterInfoComponent(IC);
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Could not fully reset info component %s .."),
				*FString(__FUNCTION__), __LINE__, *IC->GetOwner()->GetName());
		}
	}
	return Num;
}

// Refresh only selected actors components
int32 ASLIndividualVisualInfoManager::ResetVisualInfoComponents(const TArray<AActor*>& Actors)
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Init manager first.."), *FString(__FUNCTION__), __LINE__);
		return INDEX_NONE;
	}

	int32 Num = 0;
	for (const auto& Act : Actors)
	{
		if (auto** FoundIC = InfoComponentOwners.Find(Act))
		{
			bool bReset = true;			
			if ((*FoundIC)->Init(bReset) && (*FoundIC)->Load(bReset))
			{
				Num++;
			}
			else
			{
				//UnregisterIndividualComponent(*FoundIC);
				UE_LOG(LogTemp, Warning, TEXT("%s::%d Could not ully reset info component %s .."),
					*FString(__FUNCTION__), __LINE__, *(*FoundIC)->GetOwner()->GetName());
			}
		}
	}
	return Num;
}

/* Functionalities */
// Toggle visiblity for all components
int32 ASLIndividualVisualInfoManager::ToggleVisualInfoComponentsVisibility()
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Init manager first.."), *FString(__FUNCTION__), __LINE__);
		return INDEX_NONE;
	}

	for (const auto& IC : RegisteredInfoComponents)
	{
		IC->ToggleVisibility();
	}
	return RegisteredInfoComponents.Num();
}

// Toggle visiblity for selected components
int32 ASLIndividualVisualInfoManager::ToggleVisualInfoComponentsVisibility(const TArray<AActor*>& Actors)
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Init manager first.."), *FString(__FUNCTION__), __LINE__);
		return INDEX_NONE;
	}

	int32 Num = 0;
	for (const auto& Act : Actors)
	{
		if (auto** FoundIC = InfoComponentOwners.Find(Act))
		{
			(*FoundIC)->ToggleVisibility();
			Num++;
		}
	}
	return Num;
}

// Point text towards camera
int32 ASLIndividualVisualInfoManager::PointToCamera()
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Init manager first.."), *FString(__FUNCTION__), __LINE__);
		return INDEX_NONE;
	}

	int32 Num = 0;
	for (const auto& IC : RegisteredInfoComponents)
	{
		if (IC->PointToCamera())
		{
			Num++;
		}
	}
	return Num;
}

// Point selection text towards camera
int32 ASLIndividualVisualInfoManager::PointToCamera(const TArray<AActor*>& Actors)
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Init manager first.."), *FString(__FUNCTION__), __LINE__);
		return INDEX_NONE;
	}

	int32 Num = 0;
	for (const auto& Act : Actors)
	{
		if (auto** FoundIC = InfoComponentOwners.Find(Act))
		{
			if ((*FoundIC)->PointToCamera())
			{
				Num++;
			}
		}
	}
	return Num;
}

void ASLIndividualVisualInfoManager::ToggleTickUpdate()
{
	SetActorTickEnabled(!IsActorTickEnabled());
	//SetTickableWhenPaused(bEnable);
}

// Find the individual component of the actor, return nullptr if none found
USLIndividualVisualInfoComponent* ASLIndividualVisualInfoManager::GetInfoComponent(AActor* Actor) const
{
	if (UActorComponent* AC = Actor->GetComponentByClass(USLIndividualVisualInfoComponent::StaticClass()))
	{
		if (!AC->IsPendingKillOrUnreachable())
		{
			return CastChecked<USLIndividualVisualInfoComponent>(AC);
		}
	}
	return nullptr;
}

// Create and add new individual component to actor
USLIndividualVisualInfoComponent* ASLIndividualVisualInfoManager::AddNewInfoComponent(AActor* Actor)
{
	// Make sure actor does not have an info component already, and that it has an individual component
	if (!HasInfoComponent(Actor) && HasIndividualComponentSibling(Actor))
	{
		Actor->Modify();

		// Create an appropriate name for the new component (avoid duplicates)
		FName NewComponentName = *FComponentEditorUtils::GenerateValidVariableName(
			USLIndividualVisualInfoComponent::StaticClass(), Actor);

		// Get the set of owned components that exists prior to instancing the new component.
		TInlineComponentArray<UActorComponent*> PreInstanceComponents;
		Actor->GetComponents(PreInstanceComponents);

		// Create a new component
		USLIndividualVisualInfoComponent* NewComp = NewObject<USLIndividualVisualInfoComponent>(Actor, NewComponentName, RF_Transactional);

		USceneComponent* ActRoot = Actor->GetRootComponent();
		if (ActRoot)
		{
			NewComp->AttachToComponent(ActRoot, FAttachmentTransformRules::KeepRelativeTransform);
		}
		else
		{
			Actor->SetRootComponent(NewComp);
		}

		// Make visible in the components list in the editor
		Actor->AddInstanceComponent(NewComp);
		//Actor->AddOwnedComponent(NewComp);

		//NewComp->OnComponentCreated();
		NewComp->RegisterComponent();

		// Register any new components that may have been created during construction of the instanced component, but were not explicitly registered.
		TInlineComponentArray<UActorComponent*> PostInstanceComponents;
		Actor->GetComponents(PostInstanceComponents);
		for (UActorComponent* ActorComponent : PostInstanceComponents)
		{
			if (!ActorComponent->IsRegistered() && ActorComponent->bAutoRegister && !ActorComponent->IsPendingKill() && !PreInstanceComponents.Contains(ActorComponent))
			{
				ActorComponent->RegisterComponent();
			}
		}

		Actor->RerunConstructionScripts();

		if (NewComp->Init())
		{
			if (!NewComp->Load())
			{
				//UE_LOG(LogTemp, Warning, TEXT("%s::%d Info component %s could not be loaded.."),
				//	*FString(__FUNCTION__), __LINE__, *NewComp->GetOwner()->GetName());
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Info component %s could not be init.. the manager will not register it.."),
				*FString(__FUNCTION__), __LINE__, *NewComp->GetOwner()->GetName());
			return nullptr;
		}
		return NewComp;
	}
	return nullptr;
}

// Check if actor already has an info component
bool ASLIndividualVisualInfoManager::HasInfoComponent(AActor* Actor) const
{
	return Actor->GetComponentByClass(USLIndividualVisualInfoComponent::StaticClass()) != nullptr;
}

// Info component can be created if there is an individual component present
bool ASLIndividualVisualInfoManager::HasIndividualComponentSibling(AActor* Actor) const
{
	return Actor->GetComponentByClass(USLIndividualComponent::StaticClass()) != nullptr;
}

// Removes component from the actors instanced components, triggers destroy
void ASLIndividualVisualInfoManager::DestroyInfoComponent(USLIndividualVisualInfoComponent* Component)
{
	AActor* CompOwner = Component->GetOwner();
	CompOwner->Modify();
	CompOwner->RemoveInstanceComponent(Component);
	Component->ConditionalBeginDestroy();
}

// Cache component, bind delegates
bool ASLIndividualVisualInfoManager::RegisterInfoComponent(USLIndividualVisualInfoComponent* Component)
{
	bool bRetVal = true;

	// Cache component
	if (!RegisteredInfoComponents.Contains(Component))
	{
		RegisteredInfoComponents.Emplace(Component);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Component %s is already registered, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *Component->GetOwner()->GetName());
		bRetVal = false;
	}

	// Cache components owner
	AActor* CompOwner = Component->GetOwner();
	if (!InfoComponentOwners.Contains(CompOwner))
	{
		InfoComponentOwners.Emplace(CompOwner, Component);
		if (!CompOwner->OnDestroyed.IsAlreadyBound(this, &ASLIndividualVisualInfoManager::OnSemanticOwnerDestroyed))
		{
			CompOwner->OnDestroyed.AddDynamic(this, &ASLIndividualVisualInfoManager::OnSemanticOwnerDestroyed);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Owner %s is already registered, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *CompOwner->GetName());
		bRetVal = false;
	}

	// Bind component events
	if (!Component->OnDestroyed.IsAlreadyBound(this, &ASLIndividualVisualInfoManager::OnInfoComponentDestroyed))
	{
		Component->OnDestroyed.AddDynamic(this, &ASLIndividualVisualInfoManager::OnInfoComponentDestroyed);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Component %s delegate is already bound, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *Component->GetOwner()->GetName());
		bRetVal = false;
	}

	return bRetVal;
}

// Remove component from cache, unbind delegates
bool ASLIndividualVisualInfoManager::UnregisterInfoComponent(USLIndividualVisualInfoComponent* Component)
{
	bool bSuccess = true;

	if (!RegisteredInfoComponents.Remove(Component))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Component %s was not registered, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *Component->GetOwner()->GetName());
		bSuccess = false;
	}

	AActor* CompOwner = Component->GetOwner();
	if (!InfoComponentOwners.Remove(CompOwner))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Owner %s was not registered, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *CompOwner->GetName());
		bSuccess = false;
	}

	if (Component->OnDestroyed.IsAlreadyBound(this, &ASLIndividualVisualInfoManager::OnInfoComponentDestroyed))
	{
		Component->OnDestroyed.RemoveDynamic(this, &ASLIndividualVisualInfoManager::OnInfoComponentDestroyed);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Component %s delegate is not bound, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *Component->GetOwner()->GetName());
		bSuccess = false;
	}

	return bSuccess;
}

// Unregister and clear all cached components (return the number of cleared components)
int32 ASLIndividualVisualInfoManager::ClearCache()
{
	int32 Num = 0;
	for (const auto& C : RegisteredInfoComponents)
	{
		//UnregisterIndividualComponent(C);

		if (C->OnDestroyed.IsAlreadyBound(this, &ASLIndividualVisualInfoManager::OnInfoComponentDestroyed))
		{
			C->OnDestroyed.RemoveDynamic(this, &ASLIndividualVisualInfoManager::OnInfoComponentDestroyed);
			Num++;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Component %s delegate is not bound, this should not happen.."),
				*FString(__FUNCTION__), __LINE__, *C->GetOwner()->GetName());
		}
	}

	//for (auto CItr(RegisteredIndividualComponents.CreateIterator()); CItr; ++CItr)
	//{
	//	UnregisterIndividualComponent(*CItr);
	//}

	if (Num != RegisteredInfoComponents.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Num of bound delegates (%ld) is out of sync with the num of registered components (%ld).."),
			*FString(__FUNCTION__), __LINE__, Num, RegisteredInfoComponents.Num());
		Num = INDEX_NONE;
	}

	RegisteredInfoComponents.Empty();
	InfoComponentOwners.Empty();

	return Num;
}

/* Delegate functions */
// Remove destroyed individuals from array
void ASLIndividualVisualInfoManager::OnInfoComponentDestroyed(USLIndividualVisualInfoComponent* DestroyedComponent)
{
	if (UnregisterInfoComponent(DestroyedComponent))
	{
		UE_LOG(LogTemp, Log, TEXT("%s::%d Unregistered externally destroyed component %s.."),
			*FString(__FUNCTION__), __LINE__, *DestroyedComponent->GetOwner()->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Externally destroyed component %s is not registered, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *DestroyedComponent->GetOwner()->GetName());
	}
}

// Triggered by external destruction of semantic owner
void ASLIndividualVisualInfoManager::OnSemanticOwnerDestroyed(AActor* DestroyedActor)
{
	UE_LOG(LogTemp, Error, TEXT("%s::%d Log message"), *FString(__FUNCTION__), __LINE__);
}
