// Copyright 2020, Institute for Artificial Intelligence - University of Bremen
// Author: Jose Rojas

#pragma once

#include "CoreMinimal.h"
#if SL_WITH_ROSBRIDGE
#include "ROSBridgeSrvClient.h"

/**
 * ROS Service Client for USemLog
 */
class USEMLOG_API SLROSServiceClient : public FROSBridgeSrvClient
#else

/**
 * ROS Empty Service Client
 */
class SLROSServiceClient
#endif
{
public:

	// Constructor
	SLROSServiceClient();

	// Destructor
	~SLROSServiceClient();

#if SL_WITH_ROSBRIDGE
	// Init constructor
	SLROSServiceClient(UObject *InOwner, FString InName, FString InType);

	// Init constructor
	SLROSServiceClient(FString InName, FString InType)
	{
		Name = InName;
		Type = InType;
	}

	// ROS Client callback
	void Callback(TSharedPtr<FROSBridgeSrv::SrvResponse> InResponse) override;
#endif

	// Owner
	UObject *Owner;

};


