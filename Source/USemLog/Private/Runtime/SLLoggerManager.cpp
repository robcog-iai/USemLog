// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLLoggerManager.h"
#include "Utils/SLUUid.h"

// Sets default values
ASLLoggerManager::ASLLoggerManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Default values
	bIsInit = false;
	bIsStarted = false;
	bIsFinished = false;

#if WITH_EDITORONLY_DATA
	// Make manager sprite smaller (used to easily find the actor in the world)
	SpriteScale = 0.65;
#endif // WITH_EDITORONLY_DATA
}

// Sets default values
ASLLoggerManager::~ASLLoggerManager()
{
	if (!bIsFinished && !IsTemplate())
	{
		Finish(true);
	}
}

// Gets called both in the editor and during gameplay. This is not called for newly spawned actors. 
void ASLLoggerManager::PostLoad()
{
	Super::PostLoad();
	UE_LOG(LogTemp, Log, TEXT("%s::%d "), *FString(__FUNCTION__), __LINE__);
	// Setup references

	//if (!ActualWorldStateLogger
	//	|| !ActualWorldStateLogger->IsValidLowLevel()
	//	|| ActualWorldStateLogger->IsPendingKillOrUnreachable()
	//	|| !ActualWorldStateLogger->CheckStillInWorld())
	//{
	//	// Search in the world
	//	for (TActorIterator<ASLWorldStateLogger> Iter(GetWorld()); Iter; ++Iter)
	//	{
	//		if ((*Iter)->IsValidLowLevel() && !(*Iter)->IsPendingKillOrUnreachable())
	//		{
	//			ActualWorldStateLogger = *Iter;
	//			UE_LOG(LogTemp, Warning, TEXT("%s::%d Reference to the Logger manager (%s) set.."),
	//				*FString(__FUNCTION__), __LINE__, *ActualWorldStateLogger->GetName());
	//			return;
	//		}
	//		else
	//		{
	//			UE_LOG(LogTemp, Error, TEXT("%s::%d Reference (%s) invalid.."),
	//				*FString(__FUNCTION__), __LINE__, *Iter->GetName());
	//		}
	//	}

	//	// Not found in the world, spawn new
	//	FActorSpawnParameters SpawnParams;
	//	SpawnParams.Name = TEXT("SL_WorldStateLogger");
	//	ActualWorldStateLogger = GetWorld()->SpawnActor<ASLWorldStateLogger>(SpawnParams);
	//	//ActualWorldStateLogger->SetActorLabel(TEXT("SL_WorldStateLogger"));

	//	UE_LOG(LogTemp, Warning, TEXT("%s::%d Logger manager not found in the world, spawned new one (%s).."),
	//		*FString(__FUNCTION__), __LINE__, *ActualWorldStateLogger->GetName());
	//}
	//else
	//{
	//	UE_LOG(LogTemp, Error, TEXT("%s::%d ActualWorldStateLogger (%s) is set.."),
	//		*FString(__FUNCTION__), __LINE__, *ActualWorldStateLogger->GetName());
	//}
}

// Allow actors to initialize themselves on the C++ side
void ASLLoggerManager::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	UE_LOG(LogTemp, Log, TEXT("%s::%d "), *FString(__FUNCTION__), __LINE__);
	Init();
}


// Called when the game starts or when spawned
void ASLLoggerManager::BeginPlay()
{
	Super::BeginPlay();
	Start();
	
}

// Called every frame
void ASLLoggerManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called when actor removed from game or game ended
void ASLLoggerManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (!bIsFinished)
	{
		Finish();
	}
}

#if WITH_EDITOR
// Called when a property is changed in the editor
void ASLLoggerManager::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property name
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ?
		PropertyChangedEvent.Property->GetFName() : NAME_None;

	/* Logger Properties */
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLLoggerManager, LocationParams.bUseCustomEpisodeId))
	{
		if (LocationParams.bUseCustomEpisodeId) { LocationParams.EpisodeId = FSLUuid::NewGuidInBase64Url(); }
		else { LocationParams.EpisodeId = TEXT(""); };
	}
}

// Called by the editor to query whether a property of this object is allowed to be modified.
bool ASLLoggerManager::CanEditChange(const UProperty* InProperty) const
{
	// Get parent edit property
	const bool ParentVal = Super::CanEditChange(InProperty);

	// Get the property name
	const FName PropertyName = InProperty->GetFName();

	//// HostIP and HostPort can only be edited if the world state writer is of type Mongo
	//if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLManager, ServerIp))
	//{
	//	return (WriterType == ESLWorldWriterType::MongoCxx) || (WriterType == ESLWorldWriterType::MongoC);
	//}
	//else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLManager, ServerPort))
	//{
	//	return (WriterType == ESLWorldWriterType::MongoCxx) || (WriterType == ESLWorldWriterType::MongoC);
	//}
	//else if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLManager, bLogMetadata))
	//{
	//	return (WriterType == ESLWorldWriterType::MongoCxx) || (WriterType == ESLWorldWriterType::MongoC);
	//}

	return ParentVal;
}
#endif // WITH_EDITOR

// Internal init
void ASLLoggerManager::Init()
{
	if (bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Logger manager (%s) is already initialized.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}


	bIsInit = true;
	UE_LOG(LogTemp, Log, TEXT("%s::%d Logger manager (%s) succesfully initialized at %f.."),
		*FString(__FUNCTION__), __LINE__, *GetName(), GetWorld()->GetTimeSeconds());
}

// Start logging
void ASLLoggerManager::Start()
{
	if (bIsStarted)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Logger manager (%s) is already started.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	if (!bIsInit)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Logger manager (%s) is not initialized, cannot start.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	if (StartParams.bResetStartTime)
	{
		GetWorld()->TimeSeconds = 0.f;
	}

	bIsStarted = true;
	UE_LOG(LogTemp, Log, TEXT("%s::%d Logger manager (%s) succesfully started at %f.."),
		*FString(__FUNCTION__), __LINE__, *GetName(), GetWorld()->GetTimeSeconds());
}

// Finish logging
void ASLLoggerManager::Finish(bool bForced)
{
	if (bIsFinished)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Logger manager (%s) is already finished.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	if (!bIsInit || !bIsStarted)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Logger manager (%s) is not initialized or started, cannot finish.."), *FString(__FUNCTION__), __LINE__, *GetName());
		return;
	}

	bIsStarted = false;
	bIsInit = false;
	bIsFinished = true;
	UE_LOG(LogTemp, Log, TEXT("%s::%d Logger manager (%s) succesfully finished at %f.."),
		*FString(__FUNCTION__), __LINE__, *GetName(), GetWorld()->GetTimeSeconds());
}

// Bind user inputs
void ASLLoggerManager::SetupInputBindings()
{
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (UInputComponent* IC = PC->InputComponent)
		{
			IC->BindAction(StartParams.UserInputActionName, IE_Pressed, this, &ASLLoggerManager::UserInputToggleCallback);
		}
	}
}

// Start input binding
void ASLLoggerManager::UserInputToggleCallback()
{
	if (bIsInit && !bIsStarted)
	{
		ASLLoggerManager::Start();
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, FString::Printf(TEXT("[%.2f] World state logger (%s) started.."), GetWorld()->GetTimeSeconds(), *GetName()));
	}
	else if (bIsStarted && !bIsFinished)
	{
		ASLLoggerManager::Finish();
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, FString::Printf(TEXT("[%.2f] World state logger (%s) finished.."), GetWorld()->GetTimeSeconds(), *GetName()));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, FString::Printf(TEXT("[%.2f] World state logger (%s) Something went wrong, try again.."), GetWorld()->GetTimeSeconds(), *GetName()));
	}
}

