// Copyright 2019, Institute for Artificial Intelligence - University of Bremen

#pragma once

#include "CoreMinimal.h"
#if SL_WITH_ROSBRIDGE
#include "ROSBridgeSrvClient.h"

class USEMLOG_API SLROSServiceClient : public FROSBridgeSrvClient
#else
//#include "Json.h"
class SLROSServiceClient
#endif
{
public:

	SLROSServiceClient();
	~SLROSServiceClient();

#if SL_WITH_ROSBRIDGE
	SLROSServiceClient(UObject *InOwner, FString InName, FString InType);

	SLROSServiceClient(FString InName, FString InType)
	{
		Name = InName;
		Type = InType;
	}

	void Callback(TSharedPtr<FROSBridgeSrv::SrvResponse> InResponse) override;
#endif

	UObject *Owner;

};


