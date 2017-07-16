// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

//#include "CoreMinimal.h"

/**
* FSLDelegates
* Delegates used by the semantic logger.
**/
struct SEMLOG_API FSLDelegates
{
	/** Delegate type for new raw data */
	DECLARE_MULTICAST_DELEGATE_OneParam(FSLOnNewRawDataSignature, const FString&);

	///** Delegate type for new raw data */
	//DECLARE_MULTICAST_DELEGATE_OneParam(FSLOnNewRawDataSignature, const FString&);

	///** Called when new raw log data is available */
	//static FSLOnNewRawDataSignature NewRawData;

};