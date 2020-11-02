// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen

#include "Knowrob/SLKREventDispatcher.h"
#include "Mongo/SLMongoQueryManager.h"
#include "Viz/SLVizManager.h"
#include "Viz/SLVizStructs.h"

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
#if SL_WITH_PROTO_MSGS
	sl_pb::KRAmevaEvent AmevaEvent;
	AmevaEvent.ParseFromString(ProtoStr);
	if (AmevaEvent.functocall() == AmevaEvent.SetTask)
	{
		SetTask(AmevaEvent.settaskparam());
	}
	else if (AmevaEvent.functocall() == AmevaEvent.SetEpisode)
	{
		SetEpisode(AmevaEvent.setepisodeparams());
	}
	else if (AmevaEvent.functocall() == AmevaEvent.DrawMarkerTraj)
	{
		DrawMarkerTraj(AmevaEvent.drawmarkertrajparams());
	}
#endif // SL_WITH_PROTO_MSGS
}

#if SL_WITH_PROTO_MSGS
// Set the task of MongoManager
void FSLKREventDispatcher::SetTask(sl_pb::SetTaskParams params)
{
	MongoManager->SetTask(UTF8_TO_TCHAR(params.task().c_str()));
}

// Set the episode of MongoManager
void FSLKREventDispatcher::SetEpisode(sl_pb::SetEpisodeParams params)
{
	MongoManager->SetEpisode(UTF8_TO_TCHAR(params.episode().c_str()));
}

// Draw the trajectory
void FSLKREventDispatcher::DrawMarkerTraj(sl_pb::DrawMarkerTrajParams params)
{
	FString Id = UTF8_TO_TCHAR(params.id().c_str());
	float Start = params.start();
	float End = params.end();
	FVector Scale(params.scale());
	ESLVizPrimitiveMarkerType Type = GetMarkerType(params.marker());
	ESLVizMaterialType MaterialType = GetMarkerMaterialType(UTF8_TO_TCHAR(params.material().c_str()));
	FLinearColor Color = GetMarkerColor(UTF8_TO_TCHAR(params.color().c_str()));
	TArray<FTransform> Poses = MongoManager->GetIndividualTrajectory(Id, Start, End);
	VizManager->CreatePrimitiveMarker(Id, Poses, Type, params.scale(), Color, MaterialType);
}

// Transform the maker type
ESLVizPrimitiveMarkerType FSLKREventDispatcher::GetMarkerType(sl_pb::MarkerType Marker)
{
	if (Marker == sl_pb::Sphere)
	{
		return ESLVizPrimitiveMarkerType::Sphere;
	}
	else if (Marker == sl_pb::Cylinder)
	{
		return ESLVizPrimitiveMarkerType::Cylinder;
	}
	else if (Marker == sl_pb::Arrow)
	{
		return ESLVizPrimitiveMarkerType::Arrow;
	}
	else if (Marker == sl_pb::Axis)
	{
		return ESLVizPrimitiveMarkerType::Axis;
	}
	else if (Marker == sl_pb::Box) 
	{
		return ESLVizPrimitiveMarkerType::Box;
	}
	return ESLVizPrimitiveMarkerType::NONE;
}
#endif // SL_WITH_PROTO_MSGS

// Transform the string to color
FLinearColor FSLKREventDispatcher::GetMarkerColor(const FString &Color)
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

// Transform the material type
ESLVizMaterialType FSLKREventDispatcher::GetMarkerMaterialType(const FString& MaterialType)
{
	if (MaterialType.Equals("Lit") || MaterialType.Equals("lit")) 
	{
		return ESLVizMaterialType::Lit;
	}
	else if (MaterialType.Equals("Unlit") || MaterialType.Equals("unlit")) 
	{
		return ESLVizMaterialType::Unlit;
	}
	else if (MaterialType.Equals("Additive") || MaterialType.Equals("additive"))
	{
		return ESLVizMaterialType::Additive;
	}
	else if (MaterialType.Equals("Translucent") || MaterialType.Equals("translucent"))
	{
		return ESLVizMaterialType::Translucent;
	}
	return ESLVizMaterialType::NONE;
}