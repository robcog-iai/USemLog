// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLVisViewActor.h"
#include "Tags.h"
#if WITH_EDITOR
#include "Camera/CameraComponent.h"
#endif // WITH_EDITOR

// Sets default values
ASLVisViewActor::ASLVisViewActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

#if WITH_EDITORONLY_DATA
	VisCamera = CreateEditorOnlyDefaultSubobject<UCameraComponent>(TEXT("SLVisViewCamera"));
	if (VisCamera)
	{
		VisCamera->SetupAttachment(GetRootComponent());
		VisCamera->SetWorldScale3D(FVector(0.25));
	}
#endif // WITH_EDITORONLY_DATA

	bIsInit = false;

	// Replication workarounds, use a physics enabled dummy mesh
	static ConstructorHelpers::FObjectFinder<UStaticMesh> DummyMesh(
		TEXT("/USemLog/Vision/SM_ViewReplicateDummy.SM_ViewReplicateDummy"));
	if (DummyMesh.Succeeded())
	{
		GetStaticMeshComponent()->SetStaticMesh(DummyMesh.Object);
		SetMobility(EComponentMobility::Movable);
		GetStaticMeshComponent()->SetSimulatePhysics(true);
		GetStaticMeshComponent()->SetEnableGravity(false);
		this->bStaticMeshReplicateMovement = true;
		GetStaticMeshComponent()->SetCollisionProfileName("ViewReplicateDummy");
		GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::Type::PhysicsOnly);
		GetStaticMeshComponent()->SetCollisionObjectType(ECC_WorldDynamic);
		GetStaticMeshComponent()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	}

	// This has to be set in the constructor in order to be replicated
	SetReplicates(true);
	// The following are not necessarily needed
	SetReplicateMovement(true);
	bAlwaysRelevant = true;
	bReplicates = true;
	bReplicateMovement = true;
	NetPriority = 10.0f;
	NetUpdateFrequency = 100.0f;
	MinNetUpdateFrequency = 60.0f;
}

// Called when the game starts or when spawned
void ASLVisViewActor::Init()
{
	if (!bIsInit)
	{
		Class = FTags::GetValue(Tags, "SemLog", "Class");
		Id = FTags::GetValue(Tags, "SemLog", "Id");

		if (!Class.IsEmpty() && !Id.IsEmpty())
		{
			// Init even if component attachment might fail
			bIsInit = true;
		}
	}
}

// Only called during gameplay, used for attaching to components of parent (if requested)
void ASLVisViewActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Skip attachment if it is a replay
	if (GetWorld()->DemoNetDriver && GetWorld()->DemoNetDriver->IsPlaying())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d ATT Skipped"), *FString(__func__), __LINE__);
		return;
	}

	// Check if the actor should be attached to a component of its attached parent actor
	if (ComponentTagStamp != NAME_None)
	{
		if (!GetAttachParentActor())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Attached parent is not set, cannot search for the component with the tag stamp=%s"),
				*FString(__func__), __LINE__, *ComponentTagStamp.ToString());
			return;
		}

		if (!GetStaticMeshComponent())
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Static mesh is not set, replication cannot happen without, aborting component attachment.."),
				*FString(__func__), __LINE__, *ComponentTagStamp.ToString());
			return;
		}

		// Get components with stamped with the tag
		TArray<UActorComponent*> ComponentsWithTagStamp =
			GetAttachParentActor()->GetComponentsByTag(USceneComponent::StaticClass(), ComponentTagStamp);
		if (ComponentsWithTagStamp.Num() == 1)
		{
			/*USceneComponent* SC = CastChecked<USceneComponent>(ComponentsWithTagStamp[0]);
			AttachToComponent(SC, FAttachmentTransformRules::SnapToTargetIncludingScale);*/
			CompToFollow = CastChecked<USceneComponent>(ComponentsWithTagStamp[0]);
			SetActorTickEnabled(true);
		}
		else if (ComponentsWithTagStamp.Num() > 1)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Multiple components found to attach to with tag stamp=%s, attaching to first one"),
				*FString(__func__), __LINE__, *ComponentTagStamp.ToString());
			/*USceneComponent* SC = CastChecked<USceneComponent>(ComponentsWithTagStamp[0]);
			AttachToComponent(SC, FAttachmentTransformRules::SnapToTargetIncludingScale);*/
			CompToFollow = CastChecked<USceneComponent>(ComponentsWithTagStamp[0]);
			SetActorTickEnabled(true);
		}
		else if (ComponentsWithTagStamp.Num() == 0)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d No components found to attach to with tag stamp=%s"),
				*FString(__func__), __LINE__, *ComponentTagStamp.ToString());
		}
	}
}

// Called when the games starts
void ASLVisViewActor::BeginPlay()
{
	Super::BeginPlay();
}

// Called every update frame
void ASLVisViewActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (CompToFollow)
	{
		SetActorLocationAndRotation(CompToFollow->GetComponentLocation(), CompToFollow->GetComponentQuat());
		//TeleportTo(CompToFollow->GetComponentLocation(), CompToFollow->GetComponentRotation(), false, true);
	}
}