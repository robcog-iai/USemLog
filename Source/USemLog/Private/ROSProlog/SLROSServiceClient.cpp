// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Jose Rojas

#include "ROSProlog/SLROSServiceClient.h"
#include "ROSProlog/SLPrologClient.h"

// Constructor
SLROSServiceClient::SLROSServiceClient()
{
}

// Destructor
SLROSServiceClient::~SLROSServiceClient()
{
}

#if SL_WITH_ROSBRIDGE
// Init constructor
SLROSServiceClient::SLROSServiceClient(UObject *InOwner, FString InName, FString InType)
{
	Owner = InOwner;
	Name = InName;
	Type = InType;
}

// Callback to ProcessResponse in owner
void SLROSServiceClient::Callback(TSharedPtr<FROSBridgeSrv::SrvResponse> InResponse)
{
	USLPrologClient *Logger = Cast<USLPrologClient>(Owner);
	Logger->ProcessResponse(InResponse, Type);
}
#endif
