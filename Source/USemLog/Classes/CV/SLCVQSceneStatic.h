// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "CV/SLCVQScene.h"
#include "SLCVQSceneStatic.generated.h"

// Forward declaration
class ASLIndividualManager;
class ASLMongoQueryManager;

/**
 * Uses the intial map poses for setting up scenes for scanning
 */
UCLASS()
class USLCVQSceneStatic : public USLCVQScene
{
	GENERATED_BODY()

protected:
	// Virtual implementation for the scene initialization
	virtual bool InitSceneImpl(ASLIndividualManager* IndividualManager, ASLMongoQueryManager* MQManager) override;
};
