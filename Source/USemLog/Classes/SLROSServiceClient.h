// Copyright 2019, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#include "ROSBridgeSrvClient.h"

/**
 * 
 */
class USEMLOG_API SLROSServiceClient : public FROSBridgeSrvClient
{
public:
	SLROSServiceClient();
	~SLROSServiceClient();

	SLROSServiceClient(FString InName, FString InType)
	{
		Name = InName;
		Type = InType;
	}

	void Callback(TSharedPtr<FROSBridgeSrv::SrvResponse> InResponse) override;

};

