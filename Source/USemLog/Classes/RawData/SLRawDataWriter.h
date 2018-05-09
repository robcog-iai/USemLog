// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"

/**
 * Base class for raw data writer
 */
class FSLRawDataWriter
{
public:
	// Constr
	FSLRawDataWriter();

	// Destr
	virtual ~FSLRawDataWriter();

	// Called to write the data
	virtual void WriteData();
};
