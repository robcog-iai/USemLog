// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

/**
 * Abstract class for raw data writer
 */
class ISLRawDataWriter
{
public:
	// Called to write the data
	virtual void WriteData() = 0;
};
