// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLVisionLogger.h"
#include "SLEntitiesManager.h"
#if SL_WITH_SLVIS
#include "SLVisRecordGameMode.h"
#include "Engine/DemoNetDriver.h"
#include "Kismet/GameplayStatics.h"
#endif //SL_WITH_SLVIS

// Constructor
USLVisionLogger::USLVisionLogger() : bIsInit(false), bIsStarted(false), bIsFinished(false)
{
}

// Destructor
USLVisionLogger::~USLVisionLogger()
{
	if (!bIsFinished && !IsTemplate())
	{
		Finish(true);
	}
}

// Init Logger
void USLVisionLogger::Init(float InMinRecHz, float InMaxRecHz)
{
	if (!bIsInit)
	{
#if SL_WITH_SLVIS
		// Check is recording game mode is set, otherwise cancel vision logging
		if (ASLVisRecordGameMode* SLGameMode = Cast<ASLVisRecordGameMode>(GetWorld()->GetAuthGameMode()))
		{ 
			// Set movement replications to objects
			FSLEntitiesManager::GetInstance()->SetReplicates(true, 10, InMinRecHz, InMaxRecHz);

			// Set update rates
			IConsoleManager::Get().FindConsoleVariable(TEXT("demo.MinRecordHZ"))->Set(InMinRecHz);
			IConsoleManager::Get().FindConsoleVariable(TEXT("demo.RecordHZ"))->Set(InMaxRecHz);

			// Mark as initialized
			bIsInit = true;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Game Mode not set, vision replay will not be recorded.."), *FString(__func__), __LINE__);
		}
#endif //SL_WITH_SLVIS
	}
}

// Start logger
void USLVisionLogger::Start(const FString& EpisodeId)
{
	if (!bIsStarted && bIsInit)
	{
		if (UGameInstance* GI = GetWorld()->GetGameInstance())
		{
			const FString RecName = EpisodeId + "_RP";
			//TArray<FString> AdditionalOptions;
			GI->StartRecordingReplay(RecName, RecName);

			// Mark as started
			bIsStarted = true;
		}
	}
}

// Finish logger
void USLVisionLogger::Finish(bool bForced)
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{
		if (UGameInstance* GI = GetWorld()->GetGameInstance())
		{
			GI->StopRecordingReplay();

			// Mark logger as finished
			bIsStarted = false;
			bIsInit = false;
			bIsFinished = true;
		}
	}
}
