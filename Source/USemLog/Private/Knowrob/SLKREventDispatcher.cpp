// Fill out your copyright notice in the Description page of Project Settings.


#include "Knowrob/SLKREventDispatcher.h"


// Ctor
FSLKREventDispatcher::FSLKREventDispatcher(ASLMongoQueryManager* InMongoManger, ASLVizManager* InVizManager) : MongoManager(InMongoManger), VizManager(InVizManager)
{
}

// Dtor
FSLKREventDispatcher::~FSLKREventDispatcher()
{
}


// Parse the proto sequence and trigger function
void FSLKREventDispatcher::ProcessProtobuf(std::string ProtoStr)
{
	sl_pb::KRAmevaEvent AmevaEvent;
	AmevaEvent.ParseFromString(ProtoStr);
	if (AmevaEvent.functocall() == AmevaEvent.SetTask)
	{
		LogSetTask(AmevaEvent.settaskparam());
	}
	else if (AmevaEvent.functocall() == AmevaEvent.DrawMarkerAt)
	{
		LogDrawMarker(AmevaEvent.drawmarkeratparams());
	}
}

void FSLKREventDispatcher::LogSetTask(sl_pb::SetTaskParams params)
{
	UE_LOG(LogTemp, Warning, TEXT("[ProtoMessage]Set Task Event"));
	UE_LOG(LogTemp, Warning, TEXT("[ProtoMessage]Task: %s"), UTF8_TO_TCHAR(params.task().c_str()));
}

void FSLKREventDispatcher::LogDrawMarker(sl_pb::DrawMarkerAtParams params)
{
	UE_LOG(LogTemp, Warning, TEXT("[ProtoMessage]Draw Marker At Event"));
	UE_LOG(LogTemp, Warning, TEXT("[ProtoMessage]Id: %s"), UTF8_TO_TCHAR(params.id().c_str()));
	UE_LOG(LogTemp, Warning, TEXT("[ProtoMessage]Ts: %ld"), params.timestamp());
	UE_LOG(LogTemp, Warning, TEXT("[ProtoMessage]Marker: %s"), *GetVizMeshType(params.marker()));
	UE_LOG(LogTemp, Warning, TEXT("[ProtoMessage]Scale: %ld"), params.scale());
	UE_LOG(LogTemp, Warning, TEXT("[ProtoMessage]Color: %s"), UTF8_TO_TCHAR(params.color().c_str()));
	UE_LOG(LogTemp, Warning, TEXT("[ProtoMessage]Unlit: %s"), UTF8_TO_TCHAR(params.unlit().c_str()));
}

FString FSLKREventDispatcher::GetVizMeshType(sl_pb::MarkerType Marker)
{
	if (Marker == sl_pb::Sphere)
	{
		return TEXT("Sphere");
	}
	else if (Marker == sl_pb::Cylinder)
	{
		return TEXT("Cylinder");
	}
	else if (Marker == sl_pb::Arrow)
	{
		return TEXT("Arrow");
	}
	else if (Marker == sl_pb::Axis)
	{
		return TEXT("Axis");
	}
	return TEXT("Box");
}

FLinearColor FSLKREventDispatcher::GetMarkerColor(FString Color)
{
	if (Color.Equals("Red") || Color.Equals("red") || Color.Equals("RED"))
	{
		return FLinearColor::Red;
	}
	else if (Color.Equals("Black") || Color.Equals("black") || Color.Equals("BLACK"))
	{
		return FLinearColor::Black;
	}
	else if (Color.Equals("Blue") || Color.Equals("black") || Color.Equals("BLUE"))
	{
		return FLinearColor::Blue;
	}
	else if (Color.Equals("Gray") || Color.Equals("gray") || Color.Equals("GRAY"))
	{
		return FLinearColor::Gray;
	}
	else if (Color.Equals("Yellow") || Color.Equals("yellow") || Color.Equals("YELLOW"))
	{
		return FLinearColor::Yellow;
	}
	else if (Color.Equals("Green") || Color.Equals("green") || Color.Equals("GREEN"))
	{
		return FLinearColor::Green;
	}
	return FLinearColor::White;
}