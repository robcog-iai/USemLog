// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Individuals/SLIndividualInfoUtils.h"
#include "Individuals/SLIndividualInfoManager.h"
#include "Individuals/SLIndividualInfoComponent.h"
#include "Individuals/SLIndividualComponent.h"
#include "Individuals/SLIndividualUtils.h"

#include "EngineUtils.h"
#if WITH_EDITOR
#include "Kismet2/ComponentEditorUtils.h" // GenerateValidVariableName
#endif // WITH_EDITOR

// Get the semantic individual manager from the world or create a new one if none are available
ASLIndividualInfoManager* FSLIndividualInfoUtils::GetOrCreateNewIndividualInfoManager(UWorld* World, bool bCreateNew)
{
	int32 ActNum = 0;
	ASLIndividualInfoManager* Manager = nullptr;
	for (TActorIterator<ASLIndividualInfoManager> ActItr(World); ActItr; ++ActItr)
	{
		Manager = *ActItr;
		ActNum++;
	}
	if (ActNum > 1)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d There are %ld individual info managers in the world, the should only be one.."),
			*FString(__FUNCTION__), __LINE__, ActNum);
	}
	else if (ActNum == 0 && bCreateNew)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d There are no individual info managers in the world, spawning one.."),
			*FString(__FUNCTION__), __LINE__);
		FActorSpawnParameters Params;
		//Params.Name = FName(TEXT("SL_IndividualManager"));
		Manager = World->SpawnActor<ASLIndividualInfoManager>(Params);
#if WITH_EDITOR
		Manager->SetActorLabel(TEXT("SL_IndividualInfoManager"));
#endif // WITH_EDITOR
		World->MarkPackageDirty();
	}
	return Manager;
}

// Add individual components to all supported actors in the world
int32 FSLIndividualInfoUtils::CreateIndividualInfoComponents(UWorld* World)
{
	int32 Num = 0;
	for (TActorIterator<AActor> ActItr(World); ActItr; ++ActItr)
	{
		if (USLIndividualInfoComponent* IC = AddNewIndividualInfoComponent(*ActItr))
		{
			Num++;
		}
	}
	return Num;
}

// Add individual components to all supported actors from the selection
int32 FSLIndividualInfoUtils::CreateIndividualInfoComponents(const TArray<AActor*>& Actors)
{
	int32 Num = 0;
	for (const auto& Act : Actors)
	{
		if (USLIndividualInfoComponent* IC = AddNewIndividualInfoComponent(Act))
		{
			Num++;
		}
	}
	return Num;
}

// Clear individual components of all actors in the world
int32 FSLIndividualInfoUtils::ClearIndividualInfoComponents(UWorld* World)
{
	int32 Num = 0;
	for (TActorIterator<AActor> ActItr(World); ActItr; ++ActItr)
	{
		if (ClearIndividualInfoComponent(*ActItr))
		{
			Num++;
		}
	}
	return Num;
}

// Clear individual components of the selected actors
int32 FSLIndividualInfoUtils::ClearIndividualInfoComponents(const TArray<AActor*>& Actors)
{
	int32 Num = 0;
	for (const auto& Act : Actors)
	{
		if (ClearIndividualInfoComponent(Act))
		{
			Num++;
		}
	}
	return Num;
}

// Call init on all individual components in the world
int32 FSLIndividualInfoUtils::InitIndividualInfoComponents(UWorld* World, bool bReset)
{
	int32 Num = 0;
	for (TActorIterator<AActor> ActItr(World); ActItr; ++ActItr)
	{
		if (InitIndividualInfoComponent(*ActItr, bReset))
		{
			Num++;
		}
	}
	return Num;
}

// Call init on selected individual components
int32 FSLIndividualInfoUtils::InitIndividualInfoComponents(const TArray<AActor*>& Actors, bool bReset)
{
	int32 Num = 0;
	for (const auto& Act : Actors)
	{
		if (InitIndividualInfoComponent(Act, bReset))
		{
			Num++;
		}
	}
	return Num;
}

// Call load on all individual components in the world
int32 FSLIndividualInfoUtils::LoadIndividualInfoComponents(UWorld* World, bool bReset)
{
	int32 Num = 0;
	for (TActorIterator<AActor> ActItr(World); ActItr; ++ActItr)
	{
		if (LoadIndividualInfoComponent(*ActItr, bReset))
		{
			Num++;
		}
	}
	return Num;
}

// Call load on selected individual components
int32 FSLIndividualInfoUtils::LoadIndividualInfoComponents(const TArray<AActor*>& Actors, bool bReset)
{
	int32 Num = 0;
	for (const auto& Act : Actors)
	{
		if (LoadIndividualInfoComponent(Act, bReset))
		{
			Num++;
		}
	}
	return Num;
}

// Connect delegates of all individual components
int32 FSLIndividualInfoUtils::ConnectIndividualInfoComponents(UWorld* World)
{
	int32 Num = 0;
	for (TActorIterator<AActor> ActItr(World); ActItr; ++ActItr)
	{
		if (ConnectIndividualInfoComponent(*ActItr))
		{
			Num++;
		}
	}
	return Num;
}

// Connect delegates of selected individual components
int32 FSLIndividualInfoUtils::ConnectIndividualInfoComponents(const TArray<AActor*>& Actors)
{
	int32 Num = 0;
	for (const auto& Act : Actors)
	{
		if (ConnectIndividualInfoComponent(Act))
		{
			Num++;
		}
	}
	return Num;
}

// Toggle the visibility of all individual info components
int32 FSLIndividualInfoUtils::ToggleIndividualInfoComponentsVisibilty(UWorld* World)
{
	int32 Num = 0;
	for (TActorIterator<AActor> ActItr(World); ActItr; ++ActItr)
	{
		if (ToggleIndividualInfoComponentVisibilty(*ActItr))
		{
			Num++;
		}
	}
	return Num;
}

// Toggle the visibility of selected individual info components
int32 FSLIndividualInfoUtils::ToggleIndividualInfoComponentsVisibilty(const TArray<AActor*>& Actors)
{
	int32 Num = 0;
	for (const auto& Act : Actors)
	{
		if (ToggleIndividualInfoComponentVisibilty(Act))
		{
			Num++;
		}
	}
	return Num;
}


/* Private */
/* Individuals */
// Create and add new individual component
USLIndividualInfoComponent* FSLIndividualInfoUtils::AddNewIndividualInfoComponent(AActor* Actor, bool bTryInitAndLoad)
{
	// Check if the actor type is supported and there is no other existing component
	if (CanHaveIndividualInfoComponent(Actor) && !HasIndividualInfoComponent(Actor))
	{
		Actor->Modify();

		FName NewComponentName;
#if WITH_EDITOR
		// Create an appropriate name for the new component (avoid duplicates)
		NewComponentName = *FComponentEditorUtils::GenerateValidVariableName(
			USLIndividualComponent::StaticClass(), Actor);
#endif // WITH_EDITOR

		// Get the set of owned components that exists prior to instancing the new component.
		TInlineComponentArray<UActorComponent*> PreInstanceComponents;
		Actor->GetComponents(PreInstanceComponents);

		// Create a new component
		USLIndividualInfoComponent* NewComp = NewObject<USLIndividualInfoComponent>(Actor, NewComponentName, RF_Transactional);

		// Make visible in the components list in the editor
		Actor->AddInstanceComponent(NewComp);
		Actor->AddOwnedComponent(NewComp);

		//NewComp->OnComponentCreated();
		NewComp->RegisterComponent();

		// Attach componentt
		USceneComponent* RootComp = Actor->GetRootComponent();
		if (RootComp)
		{
			NewComp->AttachToComponent(RootComp, FAttachmentTransformRules::KeepRelativeTransform);
		}
		else
		{
			Actor->SetRootComponent(NewComp);
		}

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

		/* Try initializing and loading the components right after createion (this will not work for all individuals) */
		if (bTryInitAndLoad)
		{
			if (!NewComp->Init(true))
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d Individual info component %s could not be init right after creating it.. "),
					*FString(__FUNCTION__), __LINE__, *NewComp->GetFullName());
			}
			else
			{
				if (!NewComp->Load(true))
				{
					UE_LOG(LogTemp, Warning, TEXT("%s::%d Individual info component %s could not be loaded right after creating it.. "),
						*FString(__FUNCTION__), __LINE__, *NewComp->GetFullName());
				}
			}
		}
		return NewComp;
	}
	return nullptr;
}

// Check if actor supports individual info components (e.g. there is an individual component present)
bool FSLIndividualInfoUtils::CanHaveIndividualInfoComponent(AActor* Actor)
{
	return Actor->GetComponentByClass(USLIndividualComponent::StaticClass()) != nullptr;
}

// Check if actor already has an individual info component
bool FSLIndividualInfoUtils::HasIndividualInfoComponent(AActor* Actor)
{
	return Actor->GetComponentByClass(USLIndividualInfoComponent::StaticClass()) != nullptr;
}

// Clear individual info component of the actor
bool FSLIndividualInfoUtils::ClearIndividualInfoComponent(AActor* Actor)
{
	if (UActorComponent* AC = Actor->GetComponentByClass(USLIndividualInfoComponent::StaticClass()))
	{
		Actor->Modify();
		Actor->RemoveOwnedComponent(AC);
		Actor->RemoveInstanceComponent(AC);
		AC->ConditionalBeginDestroy();
		return true;
	}
	return false;
}

// Call init on the individual info component of the actor
bool FSLIndividualInfoUtils::InitIndividualInfoComponent(AActor* Actor, bool bReset)
{
	if (UActorComponent* AC = Actor->GetComponentByClass(USLIndividualInfoComponent::StaticClass()))
	{
		USLIndividualInfoComponent* IC = CastChecked<USLIndividualInfoComponent>(AC);
		return IC->Init(bReset);
	}
	return false;
}

// Call load on the individual info component of the actor
bool FSLIndividualInfoUtils::LoadIndividualInfoComponent(AActor* Actor, bool bReset)
{
	if (UActorComponent* AC = Actor->GetComponentByClass(USLIndividualInfoComponent::StaticClass()))
	{
		USLIndividualInfoComponent* IC = CastChecked<USLIndividualInfoComponent>(AC);
		return IC->Load(bReset);
	}
	return false;
}

// Connect the delegates of the individual info component
bool FSLIndividualInfoUtils::ConnectIndividualInfoComponent(AActor* Actor)
{
	if (UActorComponent* AC = Actor->GetComponentByClass(USLIndividualInfoComponent::StaticClass()))
	{
		USLIndividualInfoComponent* IC = CastChecked<USLIndividualInfoComponent>(AC);
		return IC->Connect();
	}
	return false;
}

// Toggle the visibility of all individual info components
bool FSLIndividualInfoUtils::ToggleIndividualInfoComponentVisibilty(AActor* Actor)
{
	if (UActorComponent* AC = Actor->GetComponentByClass(USLIndividualInfoComponent::StaticClass()))
	{
		USLIndividualInfoComponent* IC = CastChecked<USLIndividualInfoComponent>(AC);
		IC->SetTextVisibility(!IC->IsVisible()); // Set explicitly since the component is registered to the actor
		IC->SetVisibility(!IC->IsVisible(), true);
		IC->SetComponentTickEnabled(IC->IsVisible());
		return true;
	}
	return false;
}
