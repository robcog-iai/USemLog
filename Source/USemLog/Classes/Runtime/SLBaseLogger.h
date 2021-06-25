// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "Runtime/SLLoggerStructs.h"
#include "SLBaseLogger.generated.h"

/**
 * Abstract base class holding the common functionalities of the loggers
 */
UCLASS(ClassGroup = (SL), DisplayName = "SL Base Logger")
class USEMLOG_API ASLBaseLogger : public AInfo
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	ASLBaseLogger();

	// Force call on finish
	~ASLBaseLogger();

protected:
	// Allow actors to initialize themselves on the C++ side
	virtual void PostInitializeComponents() override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called when actor removed from game or game ended
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

#if WITH_EDITOR
	//// Called when a property is changed in the editor
	//virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;

	//// Called by the editor to query whether a property of this object is allowed to be modified.
	//virtual bool CanEditChange(const UProperty* InProperty) const override;
#endif // WITH_EDITOR

public:
	// Get init state
	bool IsInit() const { return bIsInit; };

	// Get started state
	bool IsStarted() const { return bIsStarted; };

	// Get finished state
	bool IsFinished() const { return bIsFinished; };

protected:
	// Init logger (called when the logger is used independently)
	virtual void InitImpl() {};

	// Start logger (called when the logger is used independently)
	virtual void StartImpl() {};

	// Finish logger (called when the logger is used independently) (bForced is true if called from destructor)
	virtual void FinishImpl(bool bForced = false) {};

	// Setup user input bindings
	void SetupInputBindings();

	// Start/finish logger from user input
	void UserInputToggleCallback();

protected:
	// If true the logger will start on its own (instead of being started by the manager)
	UPROPERTY(EditAnywhere, Category = "Semantic Logger")
	bool bUseIndependently;

	// Location parameters
	UPROPERTY(EditAnywhere, Category = "Semantic Logger", meta = (editcondition = "bUseIndependently"))
	FSLLoggerLocationParams LocationParameters;

	// Logger start parameters
	UPROPERTY(EditAnywhere, Category = "Semantic Logger", meta = (editcondition = "bUseIndependently"))
	FSLLoggerStartParams StartParameters;

	// True when ready to log
	bool bIsInit;

	// True when active
	bool bIsStarted;

	// True when done logging
	bool bIsFinished;
};
