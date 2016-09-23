// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ModuleManager.h"

// Declare log types
DECLARE_LOG_CATEGORY_EXTERN(SemLog, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(SemLogRaw, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(SemLogEvent, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(SemLogMap, Log, All);

/**
 * The public interface to this module.  In most cases, this interface is only public to sibling modules 
 * within this plugin.
 */
class ISemLog : public IModuleInterface
{

public:

	/**
	 * Singleton-like access to this module's interface.  This is just for convenience!
	 * Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
	 *
	 * @return Returns singleton instance, loading the module on demand if needed
	 */
	static inline ISemLog& Get()
	{
		return FModuleManager::LoadModuleChecked< ISemLog >( "SemLog" );
	}

	/**
	 * Checks to see if this module is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
	 *
	 * @return True if the module is loaded and ready to use
	 */
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded( "SemLog" );
	}
};

