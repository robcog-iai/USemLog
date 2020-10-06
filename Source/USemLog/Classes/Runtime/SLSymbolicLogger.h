// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "Runtime/SLLoggerStructs.h"
#include "SLSymbolicLogger.generated.h"

// Forward declarations
class ASLIndividualManager;

/**
 * Subsymbolic data logger
 */
UCLASS(ClassGroup = (SL), DisplayName = "SL Symbolic Logger")
class USEMLOG_API ASLSymbolicLogger : public AInfo
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASLSymbolicLogger();

	// Force call on finish
	~ASLSymbolicLogger();

protected:
	// Allow actors to initialize themselves on the C++ side
	virtual void PostInitializeComponents() override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called when actor removed from game or game ended
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Init logger (called when the logger is synced externally)
	void Init(const FSLSymbolicLoggerParams& InLoggerParameters, const FSLLoggerLocationParams& InLocationParameters);

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
	bool bUseIndependently;

	// Logger parameters
	UPROPERTY(EditAnywhere, Category = "Semantic Logger", meta = (editcondition = "bUseIndependently"))
	FSLSymbolicLoggerParams LoggerParameters;

	// Location parameters
	UPROPERTY(EditAnywhere, Category = "Semantic Logger", meta = (editcondition = "bUseIndependently"))
	FSLLoggerLocationParams LocationParameters;

	// Logger start parameters
	UPROPERTY(EditAnywhere, Category = "Semantic Logger", meta = (editcondition = "bUseIndependently"))
	FSLLoggerStartParams StartParameters;

	// Access to all individuals in the world
	UPROPERTY(VisibleAnywhere, Transient, Category = "Semantic Logger")
	ASLIndividualManager* IndividualManager;
};
