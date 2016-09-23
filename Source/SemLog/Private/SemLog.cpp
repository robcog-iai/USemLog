// Fill out your copyright notice in the Description page of Project Settings.

#include "SemLogPrivatePCH.h"

// Define logging types
DEFINE_LOG_CATEGORY(SemLog);
DEFINE_LOG_CATEGORY(SemLogRaw);
DEFINE_LOG_CATEGORY(SemLogEvent);
DEFINE_LOG_CATEGORY(SemLogMap);

class FISemLog : public ISemLog
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

IMPLEMENT_MODULE( FISemLog, ISemLog )

void FISemLog::StartupModule()
{
	UE_LOG(SemLog, Log, TEXT(" ** SemLog module startup!"));
	// This code will execute after your module is loaded into memory (but after global variables are initialized, of course.)
}


void FISemLog::ShutdownModule()
{
	UE_LOG(SemLog, Log, TEXT(" ** SemLog module shutdown!"));
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}
