// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLVisGameMode.h"
#include "SLVisSpectatorPC.h"

// Ctor
ASLVisGameMode::ASLVisGameMode()
{
	ReplaySpectatorPlayerControllerClass = ASLVisSpectatorPC::StaticClass();
}


