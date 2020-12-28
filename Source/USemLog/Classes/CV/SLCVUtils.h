// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class USEMLOG_API FSLCVUtils
{
public:
	//
	static void ReplacePixels(TArray<FColor>& InOutBitmap, FColor From, FColor To, float ManhattanTolerance);

};
