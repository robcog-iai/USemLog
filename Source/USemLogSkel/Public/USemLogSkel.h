// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "ModuleManager.h"

// Declare logging types
DECLARE_LOG_CATEGORY_EXTERN(LogSLSkel, All, All);

#if defined(_MSC_VER)
#define __func__ __FUNCTION__
#endif

class FUSemLogSkel : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};