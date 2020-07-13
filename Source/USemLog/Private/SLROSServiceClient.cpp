// Copyright 2019, Institute for Artificial Intelligence - University of Bremen


#include "SLROSServiceClient.h"

SLROSServiceClient::SLROSServiceClient()
{
}

SLROSServiceClient::~SLROSServiceClient()
{
}

void SLROSServiceClient::Callback(TSharedPtr<FROSBridgeSrv::SrvResponse> InResponse)
{
	UE_LOG(LogTemp, Warning, TEXT("Response Received:\n %s"), *InResponse->ToString());
}
