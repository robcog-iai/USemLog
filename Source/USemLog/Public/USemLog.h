// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ModuleManager.h"

// Declare log types
DECLARE_LOG_CATEGORY_EXTERN(SemLog, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(SemLogRaw, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(SemLogEvent, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(SemLogMap, Log, All);

class FUSemLogModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};