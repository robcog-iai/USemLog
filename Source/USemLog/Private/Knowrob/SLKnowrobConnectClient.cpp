// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Knowrob/SLKnowrobConnectClient.h"
#include "Individuals/SLIndividualManager.h"
#include "Mongo/SLMongoManager.h"
#include "Viz/SLVizManager.h"
#include "EngineUtils.h"


// Sets default values
ASLKnowrobConnectClient::ASLKnowrobConnectClient()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bIsInit = false;

	bIndividualManagerSet = false;
	bMongoManagerSet = false;
	bVizManagerSet = false;

	IndividualManager = nullptr;
	MongoManager = nullptr;
	VizManager = nullptr;

	Handler = MakeShareable<FSLKnowrobConnectHandler>(new FSLKnowrobConnectHandler(TEXT("35.246.255.195"), 8080, TEXT("prolog_websocket")));
}

// Called when the game starts or when spawned
void ASLKnowrobConnectClient::BeginPlay()
{
	Super::BeginPlay();	
	Init();
}

// Called every frame
void ASLKnowrobConnectClient::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	while (!Handler->QueueTask.IsEmpty()) {
		std::string ProtoStr;
		Handler->QueueTask.Dequeue(ProtoStr);
		ProcessProtobuf(ProtoStr);
	}
}

bool ASLKnowrobConnectClient::Init()
{
	if (bIsInit)
	{
		return true;
	}

	if (SetIndividualManager() && SetMongoManager() && SetVizManager())
	{
		Handler->Connect();
		bIsInit = true;
		return true;
	}
	
	return false;
}

// Reset references
void ASLKnowrobConnectClient::Reset()
{
	bIsInit = false;
	
	bIndividualManagerSet = false;
	bMongoManagerSet = false;
	bVizManagerSet = false;

	IndividualManager = nullptr;
	MongoManager = nullptr;
	VizManager = nullptr;
}

// Parse the proto sequence and trigger function
void ASLKnowrobConnectClient::ProcessProtobuf(std::string ProtoStr)
{
	knowrob_ameva::KRAmevaEvent AmevaEvent;
	AmevaEvent.ParseFromString(ProtoStr);
	if (AmevaEvent.func() == AmevaEvent.ViewEntityPoseAt)
	{
		FString Id = UTF8_TO_TCHAR(AmevaEvent.viewentityposeparam().id().c_str());
		float Timestamp = AmevaEvent.viewentityposeparam().timestamp();
		FVector Scale(AmevaEvent.viewentityposeparam().scale());
		ESLVizMeshType Type = GetVizMeshType(AmevaEvent.viewentityposeparam().marker());
		bool Unlit = AmevaEvent.viewentityposeparam().unlit();
		FLinearColor Color  = GetMarkerColor(UTF8_TO_TCHAR(AmevaEvent.viewentityposeparam().color().c_str()));
		ViewEntityPoseAt(Id, Timestamp, Type, Scale, Color, Unlit);
	}
	else if (AmevaEvent.func() == AmevaEvent.ViewEntityTraj)
	{
		FString Id = UTF8_TO_TCHAR(AmevaEvent.viewentitytrajparam().id().c_str());
		float start = AmevaEvent.viewentitytrajparam().start();
		float end = AmevaEvent.viewentitytrajparam().end();
		FVector Scale(AmevaEvent.viewentitytrajparam().scale());
		ESLVizMeshType Type = GetVizMeshType(AmevaEvent.viewentitytrajparam().marker());
		bool Unlit = AmevaEvent.viewentitytrajparam().unlit();
		FLinearColor Color = GetMarkerColor(UTF8_TO_TCHAR(AmevaEvent.viewentitytrajparam().color().c_str()));
		ViewEntityTraj(Id, start, end, Type, Scale, Color, Unlit);
	}
}


void ASLKnowrobConnectClient::ViewEntityPoseAt(const FString& Id, float Timestamp, ESLVizMeshType Type, FVector Scale, FLinearColor Color, bool bUnlit)
{
	FTransform Pose;
	if (MongoManager->GetEntityPoseAt(Id, Timestamp, Pose))
	{
		VizManager->CreateMarker(Pose, Type, Scale, Color);
	}
}

void ASLKnowrobConnectClient::ViewEntityTraj(const FString& Id, float start, float end, ESLVizMeshType Type, FVector Scale, FLinearColor Color, bool bUnlit)
{
	TArray<FTransform> Poses;
	if (MongoManager->GetEntityTrajectory(Id, start, end, Poses))
	{
		VizManager->CreateMarker(Poses, Type, Scale, Color);
	}

}

ESLVizMeshType ASLKnowrobConnectClient::GetVizMeshType(knowrob_ameva::MarkerType Marker)
{

	if (Marker == knowrob_ameva::Sphere) {
		return ESLVizMeshType::Sphere;
	}
	else if (Marker == knowrob_ameva::Cylinder) {
		return ESLVizMeshType::Cylinder;
	}
	else if (Marker == knowrob_ameva::Arrow) {
		return ESLVizMeshType::Arrow;
	}
	else if (Marker == knowrob_ameva::Axis) {
		return ESLVizMeshType::Axis;
	}
	return ESLVizMeshType::Box;
}

FLinearColor ASLKnowrobConnectClient::GetMarkerColor(FString Color)
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

// Set the individual manager
bool ASLKnowrobConnectClient::SetIndividualManager()
{
	if (IndividualManager && IndividualManager->IsValidLowLevel() && !IndividualManager->IsPendingKill())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Individual manager is already set.."), *FString(__FUNCTION__), __LINE__);
		IndividualManager->Init();
		bIndividualManagerSet = true;
		return true;
	}
	for (TActorIterator<ASLIndividualManager> Iter(GetWorld()); Iter; ++Iter)
	{
		IndividualManager = *Iter;
		IndividualManager->Init();
		bIndividualManagerSet = true;
		return true;
	}
	UE_LOG(LogTemp, Error, TEXT("%s::%d Could not find the individual manager in the world.."), *FString(__FUNCTION__), __LINE__);
	return false;
}

// Set the mongo db manager
bool ASLKnowrobConnectClient::SetMongoManager()
{
	if (MongoManager && MongoManager->IsValidLowLevel() && !MongoManager->IsPendingKill())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Mongo manager is already set.."), *FString(__FUNCTION__), __LINE__);
		//MongoManager->Init();
		if (InitMongo())
		{
			bMongoManagerSet = true;
			return true;
		}
	}
	for (TActorIterator<ASLMongoManager> Iter(GetWorld()); Iter; ++Iter)
	{
		MongoManager = *Iter;
		//MongoManager->Init();
		if (InitMongo()) 
		{
			bMongoManagerSet = true;
			return true;
		}
	}
	UE_LOG(LogTemp, Error, TEXT("%s::%d Could not find the mongo manager in the world.."), *FString(__FUNCTION__), __LINE__);
	return false;
}

// Set the visualization manager
bool ASLKnowrobConnectClient::SetVizManager()
{
	if (VizManager && VizManager->IsValidLowLevel() && !VizManager->IsPendingKill())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Viz manager is already set.."), *FString(__FUNCTION__), __LINE__);
		//VizManager->Init();
		bVizManagerSet = true;
		return true;
	}
	for (TActorIterator<ASLVizManager> Iter(GetWorld()); Iter; ++Iter)
	{
		VizManager = *Iter;
		//VizManager->Init();
		bVizManagerSet = true;
		return true;
	}
	UE_LOG(LogTemp, Error, TEXT("%s::%d Could not find the viz manager in the world.."), *FString(__FUNCTION__), __LINE__);
	return false;
}

bool ASLKnowrobConnectClient::InitMongo()
{
	return MongoManager->ConnectToServer(TEXT("localhost"), 27017, true) && MongoManager->SetDatabase(TEXT("SemLogVis421")) && MongoManager->SetCollection(TEXT("t1"));
}