// Fill out your copyright notice in the Description page of Project Settings.


#include "Knowrob/SLKREventDispatcher.h"
#include "Mongo/SLMongoQueryManager.h"
#include "Knowrob/SLSemanticMapManager.h"
#include "Control/SLControlManager.h"
#include "Viz/SLVizManager.h"
#include "Viz/SLVizStructs.h"
#include "Runtime/SLSymbolicLogger.h"
#include "Runtime/SLLoggerStructs.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"

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
void  FSLKREventDispatcher::ProcessProtobuf(FSLKRResponse& Out, std::string ProtoStr)
{
	sl_pb::KRAmevaEvent AmevaEvent;
	AmevaEvent.ParseFromString(ProtoStr);
	if (AmevaEvent.functocall() == AmevaEvent.SetTask)
	{
		SetTask(Out, AmevaEvent.settaskparam());
	}
	else if (AmevaEvent.functocall() == AmevaEvent.SetEpisode)
	{
		SetEpisode(Out, AmevaEvent.setepisodeparams());
	}
	else if (AmevaEvent.functocall() == AmevaEvent.DrawMarkerTraj)
	{
		DrawMarkerTraj(Out, AmevaEvent.drawmarkertrajparams());
	}
	else if (AmevaEvent.functocall() == AmevaEvent.LoadMap)
	{
		LoadMap(Out, AmevaEvent.loadmapparams());
	}
	else if (AmevaEvent.functocall() == AmevaEvent.StartSimulation)
	{
		StartSimulation(Out, AmevaEvent.startsimulationparams());
	}
	else if (AmevaEvent.functocall() == AmevaEvent.StopSimulation)
	{
		StopSimulation(Out, AmevaEvent.stopsimulationparams());
	}
	else if (AmevaEvent.functocall() == AmevaEvent.StartSymbolicLog)
	{
		StartSymbolicLogger(Out, AmevaEvent.startsymboliclogparams());
	}
	else if (AmevaEvent.functocall() == AmevaEvent.StopSymbolicLog)
	{
		StoptSymbolicLogger(Out);
	}
}

// Set the task of MongoManager
void FSLKREventDispatcher::SetTask(FSLKRResponse& Out, sl_pb::SetTaskParams params)
{
	MongoManager->SetTask(UTF8_TO_TCHAR(params.task().c_str()));
	Out.Type = ResponseType::TEXT;
	Out.Text = TEXT("Set task successfully");
}

// Set the episode of MongoManager
void FSLKREventDispatcher::SetEpisode(FSLKRResponse& Out, sl_pb::SetEpisodeParams params)
{
	MongoManager->SetEpisode(UTF8_TO_TCHAR(params.episode().c_str()));
	Out.Type = ResponseType::TEXT;
	Out.Text = TEXT("Set episode successfully");
}

// Draw the trajectory
void FSLKREventDispatcher::DrawMarkerTraj(FSLKRResponse& Out, sl_pb::DrawMarkerTrajParams params)
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
	Out.Type = ResponseType::TEXT;
	Out.Text = TEXT("draw trajectory");
}

// Load the Semantic Map
void FSLKREventDispatcher::LoadMap(FSLKRResponse& Out, sl_pb::LoadMapParams params)
{
	FString Map = UTF8_TO_TCHAR(params.map().c_str());
	SemanticMapManager->LoadMap(FName(*Map));
	Out.Type = ResponseType::TEXT;
	Out.Text = TEXT("Switch successfully");
}

// Start Symbolic Logger
void FSLKREventDispatcher::StartSymbolicLogger(FSLKRResponse& Out, sl_pb::StartSymbolicLogParams params)
{
	FString TaskId = UTF8_TO_TCHAR(params.taskid().c_str());
	FString EpisodeId = UTF8_TO_TCHAR(params.episodeid().c_str());
	LocationParameters.bUseCustomTaskId = true;
	LocationParameters.TaskId = TaskId;
	LocationParameters.bUseCustomEpisodeId = true;
	LocationParameters.EpisodeId = EpisodeId;
	LocationParameters.bOverwrite = true;
	SymbolicLogger->Init(LoggerParameters, LocationParameters);
	SymbolicLogger->Start();
	Out.Type = ResponseType::TEXT;
	Out.Text = TEXT("Start logging");
}

// Stop Symbolic Logger
void FSLKREventDispatcher::StoptSymbolicLogger(FSLKRResponse& Out)
{
	SymbolicLogger->Finish();
	const FString DirPath = FPaths::ProjectDir() + "/SL/" + LocationParameters.TaskId /*+ TEXT("/Episodes/")*/ + "/";
	// Write experiment to file
	FString FullFilePath = DirPath + LocationParameters.EpisodeId + TEXT("_ED.owl");
	FPaths::RemoveDuplicateSlashes(FullFilePath);
	if (FPaths::FileExists(FullFilePath))
	{
		TArray<uint8> FileBinary;
		Out.Type = ResponseType::FILE;
		Out.FileName = LocationParameters.EpisodeId + TEXT("_ED.owl");
		FFileHelper::LoadFileToArray(Out.FileData, *FullFilePath);
	}
}

// Start Simulation
void FSLKREventDispatcher::StartSimulation(FSLKRResponse& Out, sl_pb::StartSimulationParams params)
{
	TArray<FString> Ids;
	for (int i = 0; i < params.id_size(); i++) 
	{
		FString Id = UTF8_TO_TCHAR(params.id(i).c_str());
		Ids.Add(Id);
	}
	ControlManager->StartSimulationSelectionOnly(Ids);
	Out.Type = ResponseType::TEXT;
	Out.Text = TEXT("Start Sim");
}

// Stop Simulation
void FSLKREventDispatcher::StopSimulation(FSLKRResponse& Out, sl_pb::StopSimulationParams params)
{
	TArray<FString> Ids;
	for (int i = 0; i < params.id_size(); i++)
	{
		FString Id = UTF8_TO_TCHAR(params.id(i).c_str());
		Ids.Add(Id);
	}
	ControlManager->StopSimulationSelectionOnly(Ids);
	Out.Type = ResponseType::TEXT;
	Out.Text = TEXT("Stop Sim");
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