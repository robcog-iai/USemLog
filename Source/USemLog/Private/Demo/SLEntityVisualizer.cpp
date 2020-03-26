// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)


#include "Demo/SLEntityVisualizer.h"
#include "EngineUtils.h"

// Sets default values
ASLEntityVisualizer::ASLEntityVisualizer()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASLEntityVisualizer::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d \t\t\t\t\t Act=%s;"),
			*FString(__FUNCTION__), __LINE__, *It->GetName());
	}

	UE_LOG(LogTemp, Error, TEXT("%s::%d **********************"),
		*FString(__FUNCTION__), __LINE__);

	for (ULevelStreaming* LevelStreaming : World->GetStreamingLevels())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d \t Streaming level=%s;"),
			*FString(__FUNCTION__), __LINE__, *LevelStreaming->GetName());
		if (LevelStreaming && LevelStreaming->IsLevelVisible())
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d \t\t Is visible=%s;"),
				*FString(__FUNCTION__), __LINE__, *LevelStreaming->GetName());
			if (ULevel* Level = LevelStreaming->GetLoadedLevel())
			{
				UE_LOG(LogTemp, Error, TEXT("%s::%d \t\t\t Loaded level=%s;"),
					*FString(__FUNCTION__), __LINE__, *Level->GetName());
				for (AActor* Actor : Level->Actors)
				{
					if (Actor)
					{
						UE_LOG(LogTemp, Warning, TEXT("%s::%d \t\t\t\t Act=%s;"),
							*FString(__FUNCTION__), __LINE__, *Actor->GetName());
					}
					else
					{
						UE_LOG(LogTemp, Error, TEXT("%s::%d \t\t\t\t Act=nullptr;"),
							*FString(__FUNCTION__), __LINE__);
					}



					//// Make sure the actor does not have a component already
					//if (Actor->GetComponentByClass(USLIndividualComponent::StaticClass()))
					//{
					//	//USLIndividualComponent* SemanticDataComponent = NewObject<USLIndividualComponent>(USLIndividualComponent::StaticClass(), Actor);
					//	//UE_LOG(LogTemp, Error, TEXT("%s::%d %s received a new semantic data component (%s).."), *FString(__FUNCTION__), __LINE__, *Actor->GetName(), *SemanticDataComponent->GetName());
					//}
					//else
					//{
					//	//UE_LOG(LogTemp, Warning, TEXT("%s::%d %s already has a semantic data component.."), *FString(__FUNCTION__), __LINE__, *Actor->GetName());
					//}
				}


				UE_LOG(LogTemp, Error, TEXT("%s::%d ----"), *FString(__FUNCTION__), __LINE__);

				// Iterate method 1
				for (TActorIterator<AActor> ActorItr(Level->GetWorld()); ActorItr; ++ActorItr)
				{
					UE_LOG(LogTemp, Warning, TEXT("%s::%d \t\t\t\t Act=%s;"),
						*FString(__FUNCTION__), __LINE__, *ActorItr->GetName());
				}
			}
		}
	}
	
}

// Called every frame
void ASLEntityVisualizer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// True if it should tick in the editor
bool ASLEntityVisualizer::ShouldTickIfViewportsOnly() const
{
	return false;
}

