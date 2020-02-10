// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLDataVisualizer.h"
#include "GameFramework/PlayerController.h"
#include "Components/InputComponent.h"

#if SL_WITH_DATA_VIS
#include "MongoQA.h"
#endif //SL_WITH_DATA_VIS

// Constructor
USLDataVisualizer::USLDataVisualizer() : bIsInit(false), bIsStarted(false), bIsFinished(false)
{
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
		if (InQueries)
		{
			VisQueries = InQueries;
		}
		bIsInit = true;
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
	UE_LOG(LogTemp, Log, TEXT("%s::%d "), *FString(__func__), __LINE__);
}
