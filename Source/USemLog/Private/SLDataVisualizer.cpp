// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLDataVisualizer.h"
#include "GameFramework/PlayerController.h"
#include "Components/InputComponent.h"

// Constructor
USLDataVisualizer::USLDataVisualizer() : bIsInit(false), bIsStarted(false), bIsFinished(false)
{
	QueryIdx = INDEX_NONE;
}

// Destructor
USLDataVisualizer::~USLDataVisualizer()
{
	if (!bIsFinished && !IsTemplate())
	{
		Finish(true);
	}
}

// Init Logger
void USLDataVisualizer::Init(USLDataVisQueries* InQueries)
{
	if (!bIsInit)
	{
		if (!InQueries)
		{
			return;
		}
		VisQueries = InQueries;

#if SL_WITH_DATA_VIS
		if (QAHandler.ConnectToServer(VisQueries->ServerIP, VisQueries->ServerPort))
		{
			bIsInit = true;
		}		
#endif //SL_WITH_DATA_VIS
	}
}

// Start logger
void USLDataVisualizer::Start(const FName& UserInputActionName)
{
	if (!bIsStarted && bIsInit)
	{
		if (SetupUserInput(UserInputActionName))
		{
			bIsStarted = true;
		}		
	}
}

// Finish logger
void USLDataVisualizer::Finish(bool bForced)
{
	if (!bIsFinished && (bIsInit || bIsStarted))
	{
#if SL_WITH_DATA_VIS
		QAHandler.Disconnect();
#endif //SL_WITH_DATA_VIS

		// Mark logger as finished
		bIsStarted = false;
		bIsInit = false;
		bIsFinished = true;
	}
}

// Setup the user input bindings
bool USLDataVisualizer::SetupUserInput(const FName& UserInputActionName)
{
	// Set user input bindings
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (UInputComponent* IC = PC->InputComponent)
		{
			IC->BindAction(UserInputActionName, IE_Released, this, &USLDataVisualizer::Visualize);
			return true;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d No Input Component found.."), *FString(__func__), __LINE__);
			return false;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d No Player Controller found.."), *FString(__func__), __LINE__);
		return false;
	}
}

// Visualize current query
void USLDataVisualizer::Visualize()
{
	QueryIdx++;
	UE_LOG(LogTemp, Warning, TEXT("%s::%d QueryIdx=%ld;"), *FString(__func__), __LINE__, QueryIdx);

#if SL_WITH_DATA_VIS
	if (VisQueries->Queries.IsValidIndex(QueryIdx))
	{
		const FString DBName = VisQueries->Queries[QueryIdx].TaskId;
		if (!PrevDBName.Equals(DBName))
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Setting database to %s.."), *FString(__func__), __LINE__, *DBName);
			QAHandler.SetDatabase(DBName);
			PrevDBName = DBName;
		}

		const FString CollName = VisQueries->Queries[QueryIdx].EpisodeId;
		if (!PrevCollName.Equals(CollName))
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d Setting collection to %s.."), *FString(__func__), __LINE__, *CollName);
			QAHandler.SetCollection(CollName);
			PrevCollName = CollName;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d No more queries.."), *FString(__func__), __LINE__);
	}
#endif //SL_WITH_DATA_VIS
}
