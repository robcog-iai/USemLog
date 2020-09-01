// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SLKRWSClient.h"
//#include "SLKREventDelegator.h"
#include "SLKnowrobManager.generated.h"

/**
*
**/
UCLASS()
class USEMLOG_API ASLKnowrobManager : public AActor
{
	GENERATED_BODY()

public:	
	// Sets default values for this actor's properties
	ASLKnowrobManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

#if WITH_EDITOR
	// Called when a property is changed in the editor
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// If true, actor is ticked even if TickType == LEVELTICK_ViewportsOnly
	virtual bool ShouldTickIfViewportsOnly() const override;

	// Set any references
	void Init();

	// Clear any references
	void Reset();

	// Checks if the manager is initalized
	bool IsInit() const { return bIsInit; };

private:
	//// True if the manager is init
	UPROPERTY(VisibleAnywhere, Category = "Semantic Logger")
	uint8 bIsInit : 1;

	// Used to handle websocket connect to knowrob 
	TSharedPtr<FSLKRWSClient> Handler;
	
	// Delegate the knowrob event
	// TSharedPtr<FSLKREventDelegator> Delegator;

public:
	// Knowrob server ip addres
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	FString ServerIP = TEXT("35.246.255.195");

	// Knowrob server port
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	int32 Port = 8080;
	
	// Websocket protocal
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	FString WSProtocol = TEXT("prolog_websocket");

	/* Editor button hacks */
	// Triggers a call to init or reset
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Buttons")
	bool bConnectHack;
};
