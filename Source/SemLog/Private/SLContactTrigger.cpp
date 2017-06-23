// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLContactTrigger.h"

// Constructor
USLContactTrigger::USLContactTrigger()
{
	// Default type
	AreaType = EContactAreaType::Init;

	// Set the default parent as the owning actor
	ParentActor = GetOwner();

	// Check if the parent has a static mesh component
	if (ParentActor && ParentActor->IsA(AStaticMeshActor::StaticClass()))
	{
		ParentStaticMeshComponent = Cast<AStaticMeshActor>(ParentActor)->
			GetStaticMeshComponent();
		if (!ParentStaticMeshComponent)
		{
			UE_LOG(LogTemp, Error, TEXT("Could not get te static mesh component of the parent actor!"));
		}
	}
}

// Destructor
USLContactTrigger::~USLContactTrigger()
{
}

// Called when spawned or level started
void USLContactTrigger::BeginPlay()
{
	Super::BeginPlay();

	// Bind overlap begin and end events
	OnComponentBeginOverlap.AddDynamic(this, &USLContactTrigger::OnOverlapBegin);
	OnComponentEndOverlap.AddDynamic(this, &USLContactTrigger::OnOverlapEnd);
}


// Called on overlap begin events
void USLContactTrigger::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Error, TEXT("Overlap begin!"));
}

// Called on overlap end events
void USLContactTrigger::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	UE_LOG(LogTemp, Error, TEXT("Overlap end!"));
}

#if WITH_EDITOR  
void USLContactTrigger::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	// Call the base class version  
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Check if the area type changed
	if (PropertyChangedEvent.MemberProperty->GetMetaData(TEXT("Category")).Equals(TEXT("SemanticLogger")))
	{
		if (PropertyChangedEvent.MemberProperty->GetName().Equals(TEXT("AreaType")))
		{
			// Calculate new type of box extent
			UpdateContactArea();
		}
		else if (PropertyChangedEvent.MemberProperty->GetName().Equals(TEXT("ParentActor")))
		{
			// Calculate the box extent of the new actor
			UpdateContactArea();
		}
		else if (PropertyChangedEvent.MemberProperty->GetName().Equals(TEXT("ParentStaticMeshComponent")))
		{
			// Calculate the box extent of the new static mesh
			UpdateContactArea();
		}
	}
}

// Check area type
void USLContactTrigger::UpdateContactArea()
{
	// Check if the parent is set and has a static mesh component
	if (ParentActor && ParentStaticMeshComponent)
	{
		// Check for the area type
		switch (AreaType)
		{
		case(EContactAreaType::Top):
			CaclulateAreaAsTop();
			break;
		case(EContactAreaType::Bottom):
			CalculateAreaAsBottom();
			break;
		case(EContactAreaType::Wrapper):
			CalculateAreaAsWrapper();
			break;
		case(EContactAreaType::Init):
			break;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Parent actor or static mesh not set!"));
	}
}

// Calculate surface area
void USLContactTrigger::CaclulateAreaAsTop()
{
	UE_LOG(LogTemp, Error, TEXT("USLContactTrigger %s type: SURFACE"), *GetName());

	// Get the bounding box of the mesh
	const FBox SMBox = ParentStaticMeshComponent->GetStaticMesh()->GetBoundingBox();
	const FVector SMExtent = SMBox.GetExtent();

	// Set box extent
	SetBoxExtent(FVector(SMExtent.X, SMExtent.Y, 2.f), false);

	// Set the location and the rotation
	SetWorldLocation(ParentStaticMeshComponent->GetComponentLocation() + FVector(0.f, 0.f, SMExtent.Z));
	SetWorldRotation(ParentStaticMeshComponent->GetComponentRotation());
}

// Calculate inner area
void USLContactTrigger::CalculateAreaAsBottom()
{
	UE_LOG(LogTemp, Error, TEXT("USLContactTrigger %s type: INNER"), *GetName());

	// Get the bounding box of the mesh
	const FBox SMBox = ParentStaticMeshComponent->GetStaticMesh()->GetBoundingBox();
	const FVector SMExtent = SMBox.GetExtent();

	// Set box extent
	SetBoxExtent(FVector(SMExtent.X, SMExtent.Y, 2.f), false);

	// Set the location and the rotation
	SetWorldLocation(ParentStaticMeshComponent->GetComponentLocation() - FVector(0.f, 0.f, SMExtent.Z));
	SetWorldRotation(ParentStaticMeshComponent->GetComponentRotation());
}

// Calculate wrapper area
void USLContactTrigger::CalculateAreaAsWrapper()
{
	UE_LOG(LogTemp, Error, TEXT("USLContactTrigger %s type: WRAPPER"), *GetName());

	// Get the bounding box of the mesh
	const FBox SMBox = ParentStaticMeshComponent->GetStaticMesh()->GetBoundingBox();
	const FVector SMExtent = SMBox.GetExtent();

	// Set box extent
	SetBoxExtent(SMExtent + FVector(2.f), false);

	// Set the location and the rotation
	SetWorldLocation(ParentStaticMeshComponent->GetComponentLocation());
	SetWorldRotation(ParentStaticMeshComponent->GetComponentRotation());
}
#endif 