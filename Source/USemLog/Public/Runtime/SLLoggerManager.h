// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "Runtime/SLWorldStateLogger.h"
#include "Runtime/SLLoggerStructs.h"
#include "SLLoggerManager.generated.h"

UCLASS()
class USEMLOG_API ASLLoggerManager : public AInfo
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASLLoggerManager();

	// Call finish
	~ASLLoggerManager();

protected:
	// Gets called both in the editor and during gameplay. This is not called for newly spawned actors. 
	virtual void PostLoad() override;

	// Allow actors to initialize themselves on the C++ side
	virtual void PostInitializeComponents() override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called when actor removed from game or game ended
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

#if WITH_EDITOR
	// Called when a property is changed in the editor
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;

	// Called by the editor to query whether a property of this object is allowed to be modified.
	virtual bool CanEditChange(const UProperty* InProperty) const override;
#endif // WITH_EDITOR

public:	
	// Init loggers
	void Init();

	// Start loggers
	void Start();

	// Finish loggers (forced if called from destructor)
	void Finish(bool bForced = false);

	// Get init state
	bool IsInit() const { return bIsInit; };

	// Get started state
	bool IsStarted() const { return bIsStarted; };

	// Get finished state
	bool IsFinished() const { return bIsFinished; };

	// Get TaskId
	FString GetTaskId() const { return LocationParams.TaskId; };

	// Get episode id
	FString GetEpisodeId() const { return LocationParams.EpisodeId; };

private:
	// Setup user input bindings
	void SetupInputBindings();

	// Start/finish logger from user input
	void UserInputToggleCallback();

private:
	// Set when manager is initialized
	bool bIsInit;

	// Set when manager is started
	bool bIsStarted;

	// Set when manager is finished
	bool bIsFinished;

	// Logger location parameters
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	FSLLoggerLocationParams LocationParams;

	// Logger start parameters
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	FSLLoggerStartParams StartParams;

	/* World state logger*/
	// True if the world state should be logged
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bLogWorldState;

	// World state logger (if nullptr at runtime the reference will be searched for, or a new one will be spawned)
	UPROPERTY(VisibleAnywhere, Category = "Semantic Logger|World State Logger", meta = (editcondition = "bLogWorldState"))
	ASLWorldStateLogger* WorldStateLogger;

	// World state logger parameters used for logging
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|World State Logger", meta = (editcondition = "bLogWorldState"))
	FSLWorldStateLoggerParams WorldStateLoggerParams;
};
