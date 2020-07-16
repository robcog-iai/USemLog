// Copyright 2019, Institute for Artificial Intelligence - University of Bremen


#include "SLROSServiceClient.h"
#include "SLROSPrologLogger.h"

SLROSServiceClient::SLROSServiceClient()
{
}

SLROSServiceClient::SLROSServiceClient(UObject *InOwner, FString InName, FString InType)
{
	Owner = InOwner;
	Name = InName;
	Type = InType;
}

SLROSServiceClient::~SLROSServiceClient()
{
}

void SLROSServiceClient::Callback(TSharedPtr<FROSBridgeSrv::SrvResponse> InResponse)
{
	USLROSPrologLogger *Logger = Cast<USLROSPrologLogger>(Owner);
	Logger->ProcessResponse(InResponse, Type);
}
