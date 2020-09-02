// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Knowrob/SLKnowrobManager.h"


// Sets default values
ASLKnowrobManager::ASLKnowrobManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
#if WITH_EDITOR
	PrimaryActorTick.bStartWithTickEnabled = true;
#endif // WITH_EDITOR

	bIsInit = false;
}

// Called when the game starts or when spawned
void ASLKnowrobManager::BeginPlay()
{
	Super::BeginPlay();	
	Init();
}

#if WITH_EDITOR
// Called when a property is changed in the editor
void ASLKnowrobManager::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Get the changed property name
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ?
		PropertyChangedEvent.Property->GetFName() : NAME_None;

	/* Button hacks */
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ASLKnowrobManager, bConnectHack))
	{
		if (bConnectHack)
		{
			Init();
		}
		else
		{
			Reset();
		}
	}
}
#endif // WITH_EDITOR

// Called every frame
void ASLKnowrobManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	while (Handler.IsValid() && !Handler->QueueTask.IsEmpty()) 
	{
		std::string ProtoStr;
		Handler->QueueTask.Dequeue(ProtoStr);
		//Delegator->ProcessProtobuf(ProtoStr);
	}
}

// If true, actor is ticked even if TickType == LEVELTICK_ViewportsOnly
bool ASLKnowrobManager::ShouldTickIfViewportsOnly() const
{
	return true;
}

void ASLKnowrobManager::Init()
{
	if (bIsInit)
	{
		return;
	}
	//Delegator = MakeShareable<FSLKREventDelegator>(new FSLKREventDelegator());
	Handler = MakeShareable<FSLKRWSClient>(new FSLKRWSClient(ServerIP, Port, WSProtocol));
	Handler->Connect();
	bIsInit = true;
}

// Reset references
void ASLKnowrobManager::Reset()
{
	bIsInit = false;
	
	Handler->Disconnect();
}

