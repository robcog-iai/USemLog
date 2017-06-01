// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "ModuleManager.h"
//#include "ModuleManager.h"
//#include "Modules/ModuleInterface.h"

// Declare log types
DECLARE_LOG_CATEGORY_EXTERN(SemLogEd, All, All);

class FSemLogEdModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};