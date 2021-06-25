// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "Runtime/SLLoggerStructs.h"
#include "Runtime/SLWorldStateDBHandler.h"
#include "SLWorldStateLogger.generated.h"

// Forward declarations
class ASLIndividualManager;

/**
 * Subsymbolic data logger
 */
UCLASS(ClassGroup = (SL), DisplayName = "SL World State Logger")
class USEMLOG_API ASLWorldStateLogger : public AInfo
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASLWorldStateLogger();

	// Force call on finish
	~ASLWorldStateLogger();

protected:
	// Allow actors to initialize themselves on the C++ side
	virtual void PostInitializeComponents() override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called when actor removed from game or game ended
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Init logger (called when the logger is synced externally)
	void Init(const FSLWorldStateLoggerParams& InLoggerParameters,
		const FSLLoggerLocationParams& InLocationParameters,
		const FSLLoggerDBServerParams& InDBServerParameters);

	// Start logger (called when the logger is synced externally)
	void Start();

	// Finish logger (called when the logger is synced externally) (bForced is true if called from destructor)
	void Finish(bool bForced = false);

	// Get init state
	bool IsInit() const { return bIsInit; };

	// Get started state
	bool IsStarted() const { return bIsStarted; };

	// Get finished state
	bool IsFinished() const { return bIsFinished; };

	// Check if the manager is running independently
	bool IsRunningIndependently() const { return bUseIndependently; };

protected:
	// Init logger (called when the logger is used independently)
	void InitImpl();

	// Start logger (called when the logger is used independently)
	void StartImpl();

	// Finish logger (called when the logger is used independently) (bForced is true if called from destructor)
	void FinishImpl(bool bForced = false);

	// Setup user input bindings
	void SetupInputBindings();

	// Start/finish logger from user input
	void UserInputToggleCallback();

private:
	// Get the reference or spawn a new initialized individual manager
	bool SetIndividualManager();

	// First update call (log all individuals)
	void FirstUpdate();

	// Log individuals which changed state
	void Update();

protected:
	// True when ready to log
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	uint8 bIsInit : 1;

	// True when active
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	uint8 bIsStarted : 1;

	// True when done logging
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	uint8 bIsFinished : 1;

private:
	// If true the logger will start on its own (instead of being started by the manager)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	uint8 bUseIndependently : 1;

	// Logger parameters
	UPROPERTY(EditAnywhere, Category = "Semantic Logger", meta = (editcondition = "bUseIndependently"))
	FSLWorldStateLoggerParams LoggerParameters;

	// Location parameters
	UPROPERTY(EditAnywhere, Category = "Semantic Logger", meta = (editcondition = "bUseIndependently"))
	FSLLoggerLocationParams LocationParameters;

	// Database parameters
	UPROPERTY(EditAnywhere, Category = "Semantic Logger", meta = (editcondition = "bUseIndependently"))
	FSLLoggerDBServerParams DBServerParameters;

	// Logger start parameters
	UPROPERTY(EditAnywhere, Category = "Semantic Logger", meta = (editcondition = "bUseIndependently"))
	FSLLoggerStartParams StartParameters;

	// Access to all individuals in the world
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	ASLIndividualManager* IndividualManager;

	// Database handler
	TSharedPtr<FSLWorldStateDBHandler> DBHandler;
};
