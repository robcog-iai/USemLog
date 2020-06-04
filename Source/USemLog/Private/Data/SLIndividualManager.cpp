// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Data/SLIndividualManager.h"
#include "Data/SLIndividualComponent.h"
#include "Data/SLIndividualUtils.h"

#include "EngineUtils.h"
#include "Kismet2/ComponentEditorUtils.h"

#include "Engine/StaticMeshActor.h"
#include "Animation/SkeletalMeshActor.h"

// Sets default values
ASLIndividualManager::ASLIndividualManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bIsInit = false;
}

// Called when the game starts or when spawned
void ASLIndividualManager::BeginPlay()
{
	Super::BeginPlay();	
}

// Called every frame
void ASLIndividualManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Load components from world
int32 ASLIndividualManager::Init(bool bReset)
{
	int32 NumComponentsLoaded = 0;
	if (bReset)
	{
		bIsInit = false;
		ClearIndividualComponents();
	}

	if (!bIsInit)
	{
		if (GetWorld())
		{
			for (TActorIterator<AActor> ActItr(GetWorld()); ActItr; ++ActItr)
			{
				if (USLIndividualComponent* IC = GetIndividualComponent(*ActItr))
				{
					bool bRegisterComponent = true;
					if (IC->Init())
					{
						if (!IC->Load())
						{
							//UE_LOG(LogTemp, Warning, TEXT("%s::%d Individual component %s could not be loaded.. the manager will not register it.."),
							//	*FString(__FUNCTION__), __LINE__);
							//bRegisterComponent = false;
							UE_LOG(LogTemp, Warning, TEXT("%s::%d Individual component %s could not be loaded.."),
								*FString(__FUNCTION__), __LINE__, *IC->GetFullName());
						}

						RegisterIndividualComponent(IC);
						NumComponentsLoaded++;
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("%s::%d Individual component %s could not be init.. the manager will not register it.."),
							*FString(__FUNCTION__), __LINE__);
						bRegisterComponent = false;
					}

					if (bRegisterComponent)
					{

					}
				}
			}
			bIsInit = true;
		}
	}

	return NumComponentsLoaded;
}

// Add new semantic data components to the actors in the world
int32 ASLIndividualManager::AddIndividualComponents()
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Init manager first.."), *FString(__FUNCTION__), __LINE__);
		return INDEX_NONE;
	}

	int32 NumNewComponents = 0;
	if (GetWorld())
	{
		for (TActorIterator<AActor> ActItr(GetWorld()); ActItr; ++ActItr)
		{
			if (USLIndividualComponent* IC = AddNewIndividualComponent(*ActItr))
			{
				if (RegisterIndividualComponent(IC))
				{
					NumNewComponents++;
				}
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not access the world.."), *FString(__FUNCTION__), __LINE__);
	}
	return NumNewComponents;
}

// Add new semantic data components to the selected actors
int32 ASLIndividualManager::AddIndividualComponents(const TArray<AActor*>& Actors)
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Init manager first.."), *FString(__FUNCTION__), __LINE__);
		return INDEX_NONE;
	}

	int32 NumNewComponents = 0;
	for (const auto& Act : Actors)
	{
		if (USLIndividualComponent* IC = AddNewIndividualComponent(Act))
		{
			if (RegisterIndividualComponent(IC))
			{
				NumNewComponents++;
			}
		}
	}
	return NumNewComponents;
}

// Remove all semantic data components from the world
int32 ASLIndividualManager::DestroyIndividualComponents()
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Init manager first.."), *FString(__FUNCTION__), __LINE__);
		return INDEX_NONE;
	}

	int32 NumRemovedComponents = RegisteredIndividualComponents.Num();
	for (const auto& IC : RegisteredIndividualComponents)
	{
		DestroyIndividualComponent(IC);
	}

	// Clear cached individuals
	ClearIndividualComponents();

	return NumRemovedComponents;
}

// Remove selected semantic data components
int32 ASLIndividualManager::DestroyIndividualComponents(const TArray<AActor*>& Actors)
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Init manager first.."), *FString(__FUNCTION__), __LINE__);
		return INDEX_NONE;
	}

	int32 NumRemovedComponents = 0;
	for (const auto& Act : Actors)
	{
		if (USLIndividualComponent** FoundIC = IndividualComponentOwners.Find(Act))
		{
			UnregisterIndividualComponent(*FoundIC);
			DestroyIndividualComponent(*FoundIC);
			NumRemovedComponents++;
		}
	}

	return NumRemovedComponents;
}

// Reload components data
int32 ASLIndividualManager::ReloadIndividualComponents()
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Init manager first.."), *FString(__FUNCTION__), __LINE__);
		return INDEX_NONE;
	}

	int32 NumReloadedComponents = 0;
	for (const auto& IC : RegisteredIndividualComponents)
	{
		bool bReset = true;
		if (IC->Load(bReset))
		{
			NumReloadedComponents++;
		}
		else
		{
			//UnregisterIndividualComponent(IC);
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Could not reload individual component %s .."),
				*FString(__FUNCTION__), __LINE__, *IC->GetFullName());
		}
	}

	return NumReloadedComponents;
}

// Reload selected actor components data
int32 ASLIndividualManager::ReloadIndividualComponents(const TArray<AActor*>& Actors)
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Init manager first.."), *FString(__FUNCTION__), __LINE__);
		return INDEX_NONE;
	}

	int32 NumReloadedComponents = 0;
	for (const auto& Act : Actors)
	{
		if (USLIndividualComponent** FoundIC = IndividualComponentOwners.Find(Act))
		{
			bool bReset = true;
			if ((*FoundIC)->Load(bReset))
			{
				NumReloadedComponents++;
			}
			else
			{
				//UnregisterIndividualComponent(*FoundIC);
				UE_LOG(LogTemp, Warning, TEXT("%s::%d Could not reload individual component %s .."),
					*FString(__FUNCTION__), __LINE__, *(*FoundIC)->GetFullName());
			}
		}
	}

	return NumReloadedComponents;
}

// Remove destroyed individuals from array
void ASLIndividualManager::OnIndividualComponentDestroyed(USLIndividualComponent* Component)
{
	if (UnregisterIndividualComponent(Component))
	{
		UE_LOG(LogTemp, Log, TEXT("%s::%d Unregsitered externally destroyed component %s .."),
			*FString(__FUNCTION__), __LINE__, *Component->GetFullName());
	}
}

// Find the individual component of the actor, return nullptr if none found
USLIndividualComponent* ASLIndividualManager::GetIndividualComponent(AActor* Actor)
{
	if (UActorComponent* AC = Actor->GetComponentByClass(USLIndividualComponent::StaticClass()))
	{
		return CastChecked<USLIndividualComponent>(AC);
	}
	return nullptr;
}

// Create and add new individual component
USLIndividualComponent* ASLIndividualManager::AddNewIndividualComponent(AActor* Actor)
{
	if (CanHaveIndividualComponents(Actor))
	{
		if (!GetIndividualComponent(Actor))
		{
			Actor->Modify();

			// Create an appropriate name for the new component (avoid duplicates)
			FName NewComponentName = *FComponentEditorUtils::GenerateValidVariableName(
				USLIndividualComponent::StaticClass(), Actor);

			// Get the set of owned components that exists prior to instancing the new component.
			TInlineComponentArray<UActorComponent*> PreInstanceComponents;
			Actor->GetComponents(PreInstanceComponents);

			// Create a new component
			USLIndividualComponent* NewComp = NewObject<USLIndividualComponent>(Actor, NewComponentName, RF_Transactional);

			// Make visible in the components list in the editor
			Actor->AddInstanceComponent(NewComp);
			//Actor->AddOwnedComponent(NewComp);

			NewComp->OnComponentCreated();
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
					//UE_LOG(LogTemp, Warning, TEXT("%s::%d Individual component %s could not be loaded.. the manager will not register it.."),
					//	*FString(__FUNCTION__), __LINE__, *NewComp->GetFullName());
					//return nullptr;
					UE_LOG(LogTemp, Warning, TEXT("%s::%d Individual component %s could not be loaded.."),
						*FString(__FUNCTION__), __LINE__, *NewComp->GetFullName());
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d Individual component %s could not be init.. the manager will not register it.."),
					*FString(__FUNCTION__), __LINE__, *NewComp->GetFullName());
				return nullptr;
			}

			return NewComp;
		}
	}
	return nullptr;
}

bool ASLIndividualManager::CanHaveIndividualComponents(AActor* Actor)
{
	if (Actor->IsA(AStaticMeshActor::StaticClass()))
	{
		return true;
	}
	else if (Actor->IsA(ASkeletalMeshActor::StaticClass()))
	{
		return true;
	}
	return false;
}

// Remove individual component from owner
void ASLIndividualManager::DestroyIndividualComponent(USLIndividualComponent* Component)
{
	AActor* CompOwner = Component->GetOwner();
	CompOwner->Modify();
	CompOwner->RemoveInstanceComponent(Component);
	Component->ConditionalBeginDestroy();
}

// Cache component, bind delegates
bool ASLIndividualManager::RegisterIndividualComponent(USLIndividualComponent* Component)
{
	bool bSuccess = true;
	if (!RegisteredIndividualComponents.Contains(Component))
	{
		RegisteredIndividualComponents.Emplace(Component);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Component %s is already registered, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *Component->GetFullName());
		bSuccess = false;
	}

	AActor* CompOwner = Component->GetOwner();
	if (!IndividualComponentOwners.Contains(CompOwner))
	{
		IndividualComponentOwners.Emplace(CompOwner, Component);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Owner %s is already registered, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *CompOwner->GetName());
		bSuccess = false;
	}

	if (!Component->OnSLComponentDestroyed.IsAlreadyBound(this, &ASLIndividualManager::OnIndividualComponentDestroyed))
	{
		Component->OnSLComponentDestroyed.AddDynamic(this, &ASLIndividualManager::OnIndividualComponentDestroyed);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Component %s delegate is already bound, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *Component->GetFullName());
		bSuccess = false;
	}

	return bSuccess;
}

// Remove component from cache, unbind delegates
bool ASLIndividualManager::UnregisterIndividualComponent(USLIndividualComponent* Component)
{
	bool bSuccess = true;

	if (!RegisteredIndividualComponents.Remove(Component))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Component %s was not registered, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *Component->GetFullName());
		bSuccess = false;
	}

	AActor* CompOwner = Component->GetOwner();
	if (!IndividualComponentOwners.Remove(CompOwner))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Owner %s was not registered, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *CompOwner->GetName());
		bSuccess = false;
	}

	if (Component->OnSLComponentDestroyed.IsAlreadyBound(this, &ASLIndividualManager::OnIndividualComponentDestroyed))
	{
		Component->OnSLComponentDestroyed.RemoveDynamic(this, &ASLIndividualManager::OnIndividualComponentDestroyed);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Component %s delegate is not bound, this should not happen.."),
			*FString(__FUNCTION__), __LINE__, *Component->GetFullName());
		bSuccess = false;
	}

	return bSuccess;
}

// Unregister all cached components
void ASLIndividualManager::ClearIndividualComponents()
{
	for (const auto& C : RegisteredIndividualComponents)
	{
		//UnregisterIndividualComponent(C);

		if (C->OnSLComponentDestroyed.IsAlreadyBound(this, &ASLIndividualManager::OnIndividualComponentDestroyed))
		{
			C->OnSLComponentDestroyed.RemoveDynamic(this, &ASLIndividualManager::OnIndividualComponentDestroyed);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Component %s delegate is not bound, this should not happen.."),
				*FString(__FUNCTION__), __LINE__, *C->GetFullName());
		}
	}

	//for (auto CItr(RegisteredIndividualComponents.CreateIterator()); CItr; ++CItr)
	//{
	//	UnregisterIndividualComponent(*CItr);
	//}

	RegisteredIndividualComponents.Empty();
	IndividualComponentOwners.Empty();
}

bool ASLIndividualManager::IsIndividualComponentRegisteredFull(USLIndividualComponent* Component) const
{
	 return RegisteredIndividualComponents.Contains(Component)
		&& IndividualComponentOwners.Contains(Component->GetOwner())
		&& Component->OnSLComponentDestroyed.IsAlreadyBound(this, &ASLIndividualManager::OnIndividualComponentDestroyed);
}

