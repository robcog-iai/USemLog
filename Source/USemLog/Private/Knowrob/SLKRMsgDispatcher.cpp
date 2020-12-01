// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen

#include "Knowrob/SLKRMsgDispatcher.h"
#include "Mongo/SLMongoQueryManager.h"
#include "Knowrob/SLLevelManager.h"
#include "Control/SLControlManager.h"
#include "Viz/SLVizManager.h"
#include "Viz/SLVizStructs.h"
#include "Runtime/SLSymbolicLogger.h"
#include "Runtime/SLLoggerStructs.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "TimerManager.h"

// Ctor
SLKRMsgDispatcher::SLKRMsgDispatcher()
{
}

// Dtor
SLKRMsgDispatcher::~SLKRMsgDispatcher()
{
}

// Set up required manager
void SLKRMsgDispatcher::Init(TSharedPtr<FSLKRWSClient> InKRWSClient, ASLMongoQueryManager* InMongoManger,
	ASLVizManager* InVizManager, ASLLevelManager* InLevelManager,
	ASLControlManager* InControlManager, ASLSymbolicLogger* InSymbolicLogger)
{
	KRWSClient = InKRWSClient;
	MongoManager = InMongoManger;
	VizManager = InVizManager;
	LevelManager = InLevelManager;
	ControlManager = InControlManager;
	SymbolicLogger = InSymbolicLogger;

	ControlManager->OnSimulationFinish.BindRaw(this, &SLKRMsgDispatcher::SimulationStopResponse);
	bIsInit = true;
}

void SLKRMsgDispatcher::Reset()
{
	KRWSClient = nullptr;
	MongoManager = nullptr;
	VizManager = nullptr;
	LevelManager = nullptr;
	ControlManager = nullptr;
	SymbolicLogger = nullptr;
	ControlManager->OnSimulationFinish.Unbind();
	bIsInit = false;
}

// Parse the proto sequence and trigger function
void  SLKRMsgDispatcher::ProcessProtobuf(std::string ProtoStr)
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
	else if (AmevaEvent.functocall() == AmevaEvent.DrawMarkerAt)
	{
		DrawMarker(AmevaEvent.drawmarkeratparams());
	}
	else if (AmevaEvent.functocall() == AmevaEvent.DrawMarkerTraj)
	{
		DrawMarkerTraj(AmevaEvent.drawmarkertrajparams());
	}
	else if (AmevaEvent.functocall() == AmevaEvent.LoadLevel)
	{
		LoadLevel(AmevaEvent.loadlevelparams());
	}
	else if (AmevaEvent.functocall() == AmevaEvent.StartSimulation)
	{
		StartSimulation(AmevaEvent.startsimulationparams());
	}
	else if (AmevaEvent.functocall() == AmevaEvent.StopSimulation)
	{
		StopSimulation(AmevaEvent.stopsimulationparams());
	}
	else if (AmevaEvent.functocall() == AmevaEvent.StartSymbolicLog)
	{
		StartSymbolicLogger(AmevaEvent.startsymboliclogparams());
	}
	else if (AmevaEvent.functocall() == AmevaEvent.StopSymbolicLog)
	{
		StopSymbolicLogger();
	}
	else if (AmevaEvent.functocall() == AmevaEvent.RecvSymbolicLog)
	{
		SendSymbolicLog(AmevaEvent.recvsymboliclogparams());
	}
	else if (AmevaEvent.functocall() == AmevaEvent.SetIndividualPose)
	{
		SetIndividualPose(AmevaEvent.setindividualposeparams());
	}

#endif // SL_WITH_PROTO_MSGS
}

#if SL_WITH_PROTO_MSGS
// Set the task of MongoManager
void SLKRMsgDispatcher::SetTask(sl_pb::SetTaskParams params)
{
	MongoManager->SetTask(UTF8_TO_TCHAR(params.task().c_str()));
	FSLKRResponse Response;
	Response.Type = ResponseType::TEXT;
	Response.Text = TEXT("Completed: Set task");
	KRWSClient->SendResponse(Response);
}

// Set the episode of MongoManager
void SLKRMsgDispatcher::SetEpisode(sl_pb::SetEpisodeParams params)
{
	MongoManager->SetEpisode(UTF8_TO_TCHAR(params.episode().c_str()));
	FSLKRResponse Response;
	Response.Type = ResponseType::TEXT;
	Response.Text = TEXT("Completed: Set episode");
	KRWSClient->SendResponse(Response);
}

// Draw the individual marker
void SLKRMsgDispatcher::DrawMarker(sl_pb::DrawMarkerAtParams params)
{
	FString Id = UTF8_TO_TCHAR(params.id().c_str());
	float TimeStamp = params.timestamp();
	FVector Scale(params.scale());
	ESLVizPrimitiveMarkerType Type = GetMarkerType(params.marker());
	ESLVizMaterialType MaterialType = GetMarkerMaterialType(UTF8_TO_TCHAR(params.material().c_str()));
	FLinearColor Color = GetMarkerColor(UTF8_TO_TCHAR(params.color().c_str()));
	FTransform Pose = MongoManager->GetIndividualPoseAt(Id, TimeStamp);
	TArray<FTransform> Poses;
	Poses.Add(Pose);
	VizManager->CreatePrimitiveMarker(Id, Poses, Type, params.scale(), Color, MaterialType);
	FSLKRResponse Response;
	Response.Type = ResponseType::TEXT;
	Response.Text = TEXT("Completed: Draw marker");
	KRWSClient->SendResponse(Response);
}

// Draw the individual trajectory
void SLKRMsgDispatcher::DrawMarkerTraj(sl_pb::DrawMarkerTrajParams params)
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
	FSLKRResponse Response;
	Response.Type = ResponseType::TEXT;
	Response.Text = TEXT("Completed: Draw trajectory");
	KRWSClient->SendResponse(Response);
}

// Load the Semantic Map
void SLKRMsgDispatcher::LoadLevel(sl_pb::LoadLevelParams params)
{
	FString Level = UTF8_TO_TCHAR(params.level().c_str());
	LevelManager->SwitchLevelTo(FName(*Level));
	FSLKRResponse Response;
	Response.Type = ResponseType::TEXT;
	Response.Text = TEXT("Completed: Switch level");
	KRWSClient->SendResponse(Response);
}

// Start Symbolic Logger
void SLKRMsgDispatcher::StartSymbolicLogger(sl_pb::StartSymbolicLogParams params)
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
	FSLKRResponse Response;
	Response.Type = ResponseType::TEXT;
	Response.Text = TEXT("Completed: Start logging");
	KRWSClient->SendResponse(Response);
}

// Stop Symbolic Logger
void SLKRMsgDispatcher::StopSymbolicLogger()
{
	SymbolicLogger->Finish();
	FSLKRResponse Response;
	Response.Type = ResponseType::TEXT;
	Response.Text = TEXT("Completed: Stop logging");
}

// Send the Symbolic log owl file
void SLKRMsgDispatcher::SendSymbolicLog(sl_pb::RecvSymbolicLogParams params)
{
	FString TaskId = UTF8_TO_TCHAR(params.taskid().c_str());
	FString EpisodeId = UTF8_TO_TCHAR(params.episodeid().c_str());
	const FString DirPath = FPaths::ProjectDir() + "/SL/" + TaskId + "/";
	// Write experiment to file
	FString FullFilePath = DirPath + EpisodeId + TEXT("_ED.owl");
	FPaths::RemoveDuplicateSlashes(FullFilePath);
	if (FPaths::FileExists(FullFilePath))
	{
		TArray<uint8> FileBinary;
		FSLKRResponse Response;
		Response.Type = ResponseType::FILE;
		Response.FileName = EpisodeId + TEXT("_ED.owl");
		FFileHelper::LoadFileToArray(Response.FileData, *FullFilePath);
		KRWSClient->SendResponse(Response);
	}
	else 
	{
		FSLKRResponse Response;
		Response.Type = ResponseType::TEXT;
		Response.Text = TEXT("Error: File not exists");
		KRWSClient->SendResponse(Response);
	}
}

// Start Simulation
void SLKRMsgDispatcher::StartSimulation(sl_pb::StartSimulationParams params)
{
	TArray<FString> Ids;
	for (int i = 0; i < params.id_size(); i++) 
	{
		FString Id = UTF8_TO_TCHAR(params.id(i).c_str());
		Ids.Add(Id);
	}
	if (params.seconds() < 0) 
	{
		ControlManager->StartSimulationSelectionOnly(Ids);
	}
	else
	{
		ControlManager->StartSimulationSelectionOnlyForSeconds(Ids, params.seconds());
	}
	
	FSLKRResponse Response;
	Response.Type = ResponseType::TEXT;
	Response.Text = TEXT("Completed: Start simulation");
	KRWSClient->SendResponse(Response);
}

// Stop Simulation
void SLKRMsgDispatcher::StopSimulation(sl_pb::StopSimulationParams params)
{
	TArray<FString> Ids;
	for (int i = 0; i < params.id_size(); i++)
	{
		FString Id = UTF8_TO_TCHAR(params.id(i).c_str());
		Ids.Add(Id);
	}
	// No need to send response for stopping simulation, will be triggered by delegate
}

// Move Individual
void SLKRMsgDispatcher::SetIndividualPose(sl_pb::SetIndividualPoseParams params)
{
	FString Id = UTF8_TO_TCHAR(params.id().c_str());
	FVector Loc = FVector(params.vecx(), params.vecy(), params.vecz());
	FQuat Quat = FQuat(params.quatw(), params.quatx(), params.quaty(), params.quatz());
	ControlManager->SetIndividualPose(Id, Loc, Quat);
	FSLKRResponse Response;
	Response.Type = ResponseType::TEXT;
	Response.Text = TEXT("Completed: Set individual pose");
	KRWSClient->SendResponse(Response);
}

// Transform the maker type
ESLVizPrimitiveMarkerType SLKRMsgDispatcher::GetMarkerType(sl_pb::MarkerType Marker)
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
FLinearColor SLKRMsgDispatcher::GetMarkerColor(const FString &Color)
{
	if (Color.Equals("Red", ESearchCase::IgnoreCase))
	{
		return FLinearColor::Red;
	}
	else if (Color.Equals("Black", ESearchCase::IgnoreCase))
	{
		return FLinearColor::Black;
	}
	else if (Color.Equals("Blue", ESearchCase::IgnoreCase))
	{
		return FLinearColor::Blue;
	}
	else if (Color.Equals("Gray", ESearchCase::IgnoreCase))
	{
		return FLinearColor::Gray;
	}
	else if (Color.Equals("Yellow", ESearchCase::IgnoreCase))
	{
		return FLinearColor::Yellow;
	}
	else if (Color.Equals("Green", ESearchCase::IgnoreCase))
	{
		return FLinearColor::Green;
	}
	return FLinearColor::White;
}

// Transform the material type
ESLVizMaterialType SLKRMsgDispatcher::GetMarkerMaterialType(const FString& MaterialType)
{
	if (MaterialType.Equals("Lit", ESearchCase::IgnoreCase))
	{
		return ESLVizMaterialType::Lit;
	}
	else if (MaterialType.Equals("Unlit", ESearchCase::IgnoreCase))
	{
		return ESLVizMaterialType::Unlit;
	}
	else if (MaterialType.Equals("Additive", ESearchCase::IgnoreCase))
	{
		return ESLVizMaterialType::Additive;
	}
	else if (MaterialType.Equals("Translucent", ESearchCase::IgnoreCase))
	{
		return ESLVizMaterialType::Translucent;
	}
	return ESLVizMaterialType::NONE;
}

// Send response when simulation stop
void SLKRMsgDispatcher::SimulationStopResponse()
{
	FSLKRResponse Response;
	Response.Type = ResponseType::TEXT;
	Response.Text = TEXT("Stop Simulation");
	KRWSClient->SendResponse(Response);
}