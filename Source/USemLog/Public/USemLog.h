// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once
#if SL_WITH_EYE_TRACKING
#include "SRanipal_Enums.h" // Added here since in other places weird namsepace error occur
#endif // SL_WITH_EYE_TRACKING
#include "CoreMinimal.h"
#include "ModuleManager.h"

// Declare logging types
DECLARE_LOG_CATEGORY_EXTERN(LogSL, All, All);

#if defined(_MSC_VER)
#define __func__ __FUNCTION__
#endif


class FUSemLog : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};