// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "Runtime/SLLoggerStructs.h"
#include "SLLoggerManager.generated.h"

// Forward declarations
class ASLWorldStateLogger;
class ASLSymbolicLogger;

UCLASS(ClassGroup = (SL), DisplayName = "SL Logger Manager")
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

	// Called when actor removed from game or game ended
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

#if WITH_EDITOR
	// Called when a property is changed in the editor
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
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

	// Set the location parameters (useful when controlled externally)
	void SetLocationParams(const FSLLoggerLocationParams& InParams) { LocationParams = InParams; };

	// Set the start parameters (useful when controlled externally)
	void SetStartParams(const FSLLoggerStartParams& InParams) { StartParams = InParams; };

	// Set the world state logger parameters (useful when controlled externally)
	void SetWorldStateLoggerParams(const FSLWorldStateLoggerParams& InParams, const FSLLoggerDBServerParams InDBServerParams) 
	{
		bLogWorldState = true; 
		DBServerParams = InDBServerParams;
		WorldStateLoggerParams = InParams; 
	};

	// Set the symbolic logger parameters (useful when controlled externally)
	void SetSymbolicLoggerParams(const FSLSymbolicLoggerParams& InParams) { bLogActionsAndEvents = true; SymbolicLoggerParams = InParams; };

	// Set log world state flag
	void SetLogWorldState(bool Value) { bLogWorldState = Value; };

	// Set log symbolic data
	void SetLogActionsAndEvens(bool Value) { bLogActionsAndEvents = Value; };

	// Get semantic map id
	FString GetSemanticMapId() const { return LocationParams.SemanticMapId; };

	// Get TaskId
	FString GetTaskId() const { return LocationParams.TaskId; };

	// Get episode id
	FString GetEpisodeId() const { return LocationParams.EpisodeId; };

	// Check if the manager is running independently
	bool IsRunningIndependently() const { return bUseIndependently; };

protected:
	// Setup user input bindings
	void SetupInputBindings();

	// Start/finish logger from user input
	void UserInputToggleCallback();

private:
	// Get the reference or spawn a new initialized world state logger
	bool SetWorldStateLogger();

	// Get the reference or spawn a new initialized symbolic logger
	bool SetSymbolicLogger();

	// Write semantic map owl file using the semantic map id
	void WriteSemanticMap(bool bOverwrite = true);

	// Write task owl file using the task id
	void WriteTask(bool bOverwrite = true);

protected:
	// Set when manager is initialized
	uint8 bIsInit : 1;

	// Set when manager is started
	uint8 bIsStarted : 1;

	// Set when manager is finished
	uint8 bIsFinished : 1;

private:
	// Call init and start once the world is started, or execute externally
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	uint8 bUseIndependently : 1;

	// Logger location parameters
	UPROPERTY(EditAnywhere, Category = "Semantic Logger", meta = (editcondition = "bUseIndependently"))
	FSLLoggerLocationParams LocationParams;

	// Logger start parameters
	UPROPERTY(EditAnywhere, Category = "Semantic Logger", meta = (editcondition = "bUseIndependently"))
	FSLLoggerStartParams StartParams;


	/* World state logger */
	// True if the world state should be logged
	UPROPERTY(EditAnywhere, Category = "Semantic Logger", meta = (editcondition = "bUseIndependently"))
	bool bLogWorldState = false;

	// World state logger (if nullptr at runtime the reference will be searched for, or a new one will be spawned)
	UPROPERTY(VisibleAnywhere, Category = "Semantic Logger|World State Logger", meta = (editcondition = "bLogWorldState"))
	ASLWorldStateLogger* WorldStateLogger;

	// World state logger parameters used for logging
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|World State Logger", meta = (editcondition = "bLogWorldState"))
	FSLWorldStateLoggerParams WorldStateLoggerParams;
	
	// DB server parameters
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|World State Logger", meta = (editcondition = "bLogWorldState"))
	FSLLoggerDBServerParams DBServerParams;


	/* Symbolic logger */
	UPROPERTY(EditAnywhere, Category = "Semantic Logger", meta = (editcondition = "bUseIndependently"))
	bool bLogActionsAndEvents = false;

	// World state logger (if nullptr at runtime the reference will be searched for, or a new one will be spawned)
	UPROPERTY(VisibleAnywhere, Category = "Semantic Logger|Symbolic Logger", meta = (editcondition = "bLogActionsAndEvents"))
	ASLSymbolicLogger* SymbolicLogger;

	// World state logger parameters used for logging
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Symbolic Logger", meta = (editcondition = "bLogActionsAndEvents"))
	FSLSymbolicLoggerParams SymbolicLoggerParams;

	// Editor button hack to write the semantic map
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Edit")
	bool bWriteSemanticMapButton = false;

	// Editor button hack to write the task owl file
	UPROPERTY(EditAnywhere, Category = "Semantic Logger|Edit")
	bool bWriteTaskButton = false;
};
