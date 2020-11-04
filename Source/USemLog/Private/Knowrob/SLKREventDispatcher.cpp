// Fill out your copyright notice in the Description page of Project Settings.


#include "Knowrob/SLKREventDispatcher.h"
#include "Mongo/SLMongoQueryManager.h"
#include "Knowrob/SLSemanticMapManager.h"
#include "Control/SLControlManager.h"
#include "Viz/SLVizManager.h"
#include "Viz/SLVizStructs.h"
#include "Runtime/SLSymbolicLogger.h"
#include "Runtime/SLLoggerStructs.h"

// Ctor
FSLKREventDispatcher::FSLKREventDispatcher(ASLMongoQueryManager* InMongoManger, 
	ASLVizManager* InVizManager, ASLSemanticMapManager* InSemanticMapManager, 
	ASLControlManager* InControlManager, ASLSymbolicLogger* InSymbolicLogger) :
	MongoManager(InMongoManger), VizManager(InVizManager), SemanticMapManager(InSemanticMapManager),
	ControlManager(InControlManager), SymbolicLogger(InSymbolicLogger)
	
{
}

// Dtor
FSLKREventDispatcher::~FSLKREventDispatcher()
{
}


// Parse the proto sequence and trigger function
FString FSLKREventDispatcher::ProcessProtobuf(std::string ProtoStr)
{
	sl_pb::KRAmevaEvent AmevaEvent;
	AmevaEvent.ParseFromString(ProtoStr);
	if (AmevaEvent.functocall() == AmevaEvent.SetTask)
	{
		return SetTask(AmevaEvent.settaskparam());
	}
	else if (AmevaEvent.functocall() == AmevaEvent.SetEpisode)
	{
		return SetEpisode(AmevaEvent.setepisodeparams());
	}
	else if (AmevaEvent.functocall() == AmevaEvent.DrawMarkerTraj)
	{
		return DrawMarkerTraj(AmevaEvent.drawmarkertrajparams());
	}
	else if (AmevaEvent.functocall() == AmevaEvent.LoadMap)
	{
		return LoadMap(AmevaEvent.loadmapparams());
	}
	else if (AmevaEvent.functocall() == AmevaEvent.StartSimulation)
	{
		return StartSimulation(AmevaEvent.startsimulationparams());
	}
	else if (AmevaEvent.functocall() == AmevaEvent.StopSimulation)
	{
		return StopSimulation(AmevaEvent.stopsimulationparams());
	}
	else if (AmevaEvent.functocall() == AmevaEvent.StartSymbolicLog)
	{
		return StartSymbolicLogger(AmevaEvent.startsymboliclogparams());
	}
	else if (AmevaEvent.functocall() == AmevaEvent.StopSymbolicLog)
	{
		return StoptSymbolicLogger();
	}
	return "";
}

// Set the task of MongoManager
FString FSLKREventDispatcher::SetTask(sl_pb::SetTaskParams params)
{
	MongoManager->SetTask(UTF8_TO_TCHAR(params.task().c_str()));
	return TEXT("Set task successfully");
}

// Set the episode of MongoManager
FString FSLKREventDispatcher::SetEpisode(sl_pb::SetEpisodeParams params)
{
	MongoManager->SetEpisode(UTF8_TO_TCHAR(params.episode().c_str()));
	return TEXT("Set episode successfully");
}

// Draw the trajectory
FString FSLKREventDispatcher::DrawMarkerTraj(sl_pb::DrawMarkerTrajParams params)
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
	return TEXT("draw trajectory");
}

// Load the Semantic Map
FString FSLKREventDispatcher::LoadMap(sl_pb::LoadMapParams params)
{
	FString Map = UTF8_TO_TCHAR(params.map().c_str());
	SemanticMapManager->LoadMap(FName(*Map));
	return TEXT("Switch successfully");
}

// Start Symbolic Logger
FString FSLKREventDispatcher::StartSymbolicLogger(sl_pb::StartSymbolicLogParams params)
{
	FString TaskId = UTF8_TO_TCHAR(params.taskid().c_str());
	FString EpisodeId = UTF8_TO_TCHAR(params.episodeid().c_str());
	FSLSymbolicLoggerParams LoggerParameters;
	FSLLoggerLocationParams LocationParameters;
	LocationParameters.bUseCustomTaskId = true;
	LocationParameters.TaskId = TaskId;
	LocationParameters.bUseCustomEpisodeId = true;
	LocationParameters.EpisodeId = EpisodeId;
	LocationParameters.bOverwrite = true;
	SymbolicLogger->Init(LoggerParameters, LocationParameters);
	SymbolicLogger->Start();
	return TEXT("Start logging");
}

// Stop Symbolic Logger
FString FSLKREventDispatcher::StoptSymbolicLogger()
{
	SymbolicLogger->Finish();
	return TEXT("Stop logging");
}

// Start Simulation
FString FSLKREventDispatcher::StartSimulation(sl_pb::StartSimulationParams params)
{
	TArray<FString> Ids;
	for (int i = 0; i < params.id_size(); i++) 
	{
		FString Id = UTF8_TO_TCHAR(params.id(i).c_str());
		Ids.Add(Id);
	}
	ControlManager->StartSimulationSelectionOnly(Ids);
	return TEXT("Start Sim");
}

// Stop Simulation
FString FSLKREventDispatcher::StopSimulation(sl_pb::StopSimulationParams params)
{
	TArray<FString> Ids;
	for (int i = 0; i < params.id_size(); i++)
	{
		FString Id = UTF8_TO_TCHAR(params.id(i).c_str());
		Ids.Add(Id);
	}
	ControlManager->StopSimulationSelectionOnly(Ids);
	return TEXT("Stop Sim");
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
	else if (Marker == sl_pb::Box) {
		return ESLVizPrimitiveMarkerType::Box;
	}
	return ESLVizPrimitiveMarkerType::NONE;
}

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