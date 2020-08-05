// Copyright 2019, Institute for Artificial Intelligence - University of Bremen


#include "SLROSServiceClient.h"
#include "SLROSPrologLogger.h"

SLROSServiceClient::SLROSServiceClient()
{
}

SLROSServiceClient::~SLROSServiceClient()
{
}

#if SL_WITH_ROSBRIDGE
SLROSServiceClient::SLROSServiceClient(UObject *InOwner, FString InName, FString InType)
{
	Owner = InOwner;
	Name = InName;
	Type = InType;
}

void SLROSServiceClient::Callback(TSharedPtr<FROSBridgeSrv::SrvResponse> InResponse)
{
	USLROSPrologLogger *Logger = Cast<USLROSPrologLogger>(Owner);
	Logger->ProcessResponse(InResponse, Type);
}
#endif