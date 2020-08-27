// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Viz/SLVizManager.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Tags.h" //UUtils
#include "Materials/MaterialInstanceDynamic.h"

// Sets default values
ASLVizManager::ASLVizManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	VizKey = INDEX_NONE;
	bIsInit = false;
}

// Called when the game starts or when spawned
void ASLVizManager::BeginPlay()
{
	Super::BeginPlay();

	// Self initialization
	Init();
}

// Init the mappings between the unique ids and the entities
void ASLVizManager::Init()
{
	if(!bIsInit)
	{
		LoadEntityMappings();
		if (LoadMarkerAssets())
		{
			bIsInit = true;
		}
	}
}

// Clears all markers, reset key counter
void ASLVizManager::Clear()
{
	ClearMarkers();
	ClearClones();
	VizKey = INDEX_NONE;
}

// Create a single pose marker with default orientation
int32 ASLVizManager::CreateMarker(const FVector& Location, ESLVizMeshType Type, FVector Scale, FLinearColor Color, bool bUnlit)
{
	return CreateMarker(FTransform(Location), Type, Scale, Color, bUnlit);
}

// Create a single pose marker
int32 ASLVizManager::CreateMarker(const FTransform& Pose, ESLVizMeshType Type, FVector Scale, FLinearColor Color, bool bUnlit)
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Manager is not initialized.."), *FString(__func__), __LINE__);
		return INDEX_NONE;
	}
	
	UInstancedStaticMeshComponent* ISMC = NewObject<UInstancedStaticMeshComponent>(this);
	UMaterialInstanceDynamic* DynMat = bUnlit ? UMaterialInstanceDynamic::Create(MaterialUnlit, NULL) : UMaterialInstanceDynamic::Create(MaterialLit, NULL);
	DynMat->SetVectorParameterValue(FName("Color"), Color);
	
	ISMC->SetMaterial(0, DynMat);
	ISMC->SetStaticMesh(GetMeshFromType(Type));
	ISMC->RegisterComponentWithWorld(GetWorld());
	ISMC->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);

	AddOwnedComponent(ISMC);

	// Add instance
	FTransform Transform = Pose;
	Transform.SetScale3D(Scale);
	ISMC->AddInstance(Transform);

	VizKey++;
	Markers.Emplace(VizKey, ISMC);
	return VizKey;
}

// Create a trajectory marker with default orientation
int32 ASLVizManager::CreateMarker(const TArray<FVector>& Locations, ESLVizMeshType Type, FVector Scale, FLinearColor Color,
	bool bUnlit)
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Manager is not initialized.."), *FString(__func__), __LINE__);
		return INDEX_NONE;
	}
	
	UInstancedStaticMeshComponent* ISMC = NewObject<UInstancedStaticMeshComponent>(this);
	UMaterialInstanceDynamic* DynMat = bUnlit ? UMaterialInstanceDynamic::Create(MaterialUnlit, NULL) : UMaterialInstanceDynamic::Create(MaterialLit, NULL);
	DynMat->SetVectorParameterValue(FName("Color"), Color);
	
	ISMC->SetMaterial(0, DynMat);
	ISMC->SetStaticMesh(GetMeshFromType(Type));
	ISMC->RegisterComponentWithWorld(GetWorld());
	ISMC->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);

	AddOwnedComponent(ISMC);

	// Add instances
	for(const auto& P : Locations)
	{
		FTransform T(P);
		T.SetScale3D(Scale);
		ISMC->AddInstance(T);
	}
	
	VizKey++;
	Markers.Emplace(VizKey, ISMC);
	return VizKey;
}

// Create a trajectory marker
int32 ASLVizManager::CreateMarker(const TArray<FTransform>& Poses, ESLVizMeshType Type, FVector Scale, FLinearColor Color, bool bUnlit )
{
	if (!bIsInit)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Manager is not initialized.."), *FString(__func__), __LINE__);
		return INDEX_NONE;
	}
	
	UInstancedStaticMeshComponent* ISMC = NewObject<UInstancedStaticMeshComponent>(this);
	UMaterialInstanceDynamic* DynMat = bUnlit ? UMaterialInstanceDynamic::Create(MaterialUnlit, NULL) : UMaterialInstanceDynamic::Create(MaterialLit, NULL);
	DynMat->SetVectorParameterValue(FName("Color"), Color);
	
	ISMC->SetMaterial(0, DynMat);
	ISMC->SetStaticMesh(GetMeshFromType(Type));
	ISMC->RegisterComponentWithWorld(GetWorld());
	ISMC->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);

	AddOwnedComponent(ISMC);

	// Add instances
	for(auto P : Poses)
	{
		P.SetScale3D(Scale);
		ISMC->AddInstance(P);
	}
	
	VizKey++;
	Markers.Emplace(VizKey, ISMC);
	return VizKey;
}

// Append a pose to the marker
bool ASLVizManager::AppendMarker(int32 Key, const FTransform& Pose, FVector Scale)
{
	if (UInstancedStaticMeshComponent** ISMC = Markers.Find(Key))
	{
		FTransform Transform = Pose;
		Transform.SetScale3D(Scale);
		(*ISMC)->AddInstance(Transform);
		return true;
	}
	UE_LOG(LogTemp, Error, TEXT("%s::%d Could not find marker with Key=%d"), *FString(__func__), __LINE__, Key);
	return false;
}

// Append an array to the marker
bool ASLVizManager::AppendMarker(int32 Key, const TArray<FTransform>& Poses, FVector Scale)
{
	if (UInstancedStaticMeshComponent** ISMC = Markers.Find(Key))
	{
		for(auto P : Poses)
		{
			P.SetScale3D(Scale);
			(*ISMC)->AddInstance(P);
		}
		return true;
	}
	UE_LOG(LogTemp, Error, TEXT("%s::%d Could not find marker with Key=%d"), *FString(__func__), __LINE__, Key);
	return false;
}

// Remove marker
bool ASLVizManager::RemoveMarker(int32 Key)
{
	if(UInstancedStaticMeshComponent* StaticMesh =  *Markers.Find(Key))
	{
		StaticMesh->DestroyComponent();
		Markers.Remove(Key);
		return true;
	}
	UE_LOG(LogTemp, Error, TEXT("%s::%d Could not find marker with Key=%d"), *FString(__func__), __LINE__, Key);
	return false;
}

// Remove all markers
void ASLVizManager::ClearMarkers()
{
	for (auto& Maker : Markers)
	{
		Maker.Value->DestroyComponent();
	}
	Markers.Empty();
}

// Create clone copy of the given entity id
int32 ASLVizManager::CreateEntityClone(const FString& Id, const FTransform& Transform)
{
	if(AStaticMeshActor** SMA = IdToSMA.Find(Id))
	{
		FActorSpawnParameters Parameters;
		Parameters.Template = *SMA;
		Parameters.Instigator = (*SMA)->GetInstigator();
		Parameters.Name = FName(*((*SMA)->GetName() + TEXT("_SMClone_") + FString::FromInt(VizKey)));
		AStaticMeshActor* SMClone =  GetWorld()->SpawnActor<AStaticMeshActor>((*SMA)->GetClass(), Transform, Parameters);

		// Store the marker
		VizKey++;
		EntityClones.Emplace(VizKey, SMClone);
		return VizKey;
	}
	UE_LOG(LogTemp, Error, TEXT("%s::%d Could not find entity with Id=%s, did you run init before?"), *FString(__func__), __LINE__, *Id);
	return INDEX_NONE;
}

// Get the entity clone
AStaticMeshActor* ASLVizManager::GetEntityClone(int32 Key)
{
	if(AStaticMeshActor** SMA = EntityClones.Find(Key))
	{
		return *SMA;
	}
	UE_LOG(LogTemp, Error, TEXT("%s::%d Could not find entity with Key=%d"), *FString(__func__), __LINE__, Key);
	return nullptr;
}

// Remove the entity clone marker
bool ASLVizManager::RemoveEntityClone(int32 Key)
{
	AStaticMeshActor* SMA = nullptr;
	if(EntityClones.RemoveAndCopyValue(Key,SMA))
	{
		//SMA->SetLifeSpan(5);
		SMA->Destroy();
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Entity with Key=%d was succesfully removed.."), *FString(__func__), __LINE__, Key);
		return true;
	}
	UE_LOG(LogTemp, Error, TEXT("%s::%d Could not find entity with Key=%d"), *FString(__func__), __LINE__, Key);
	return false;
}

// Remove all entity clones
void ASLVizManager::ClearEntityClones()
{
	for (auto& Pair : EntityClones)
	{
		//Entity.Value->SetLifeSpan(5);
		Pair.Value->Destroy();
	}
	EntityClones.Empty();
}

// Create skeletal clone
int32 ASLVizManager::CreateSkeletalClone(const FString& Id, const FTransform& Transform)
{
	if(ASkeletalMeshActor** SkMA = IdToSkMA.Find(Id))
	{
		FActorSpawnParameters Parameters;
		Parameters.Template = *SkMA;
		Parameters.Instigator = (*SkMA)->GetInstigator();
		//FString NewName = Skeletal->GetName()+ TEXT("_SKClone_") + FString::FromInt(VizKey);
		Parameters.Name = FName(*((*SkMA)->GetName() + TEXT("_SkMClone_") + FString::FromInt(VizKey)));
		ASkeletalMeshActor* SkMClone = GetWorld()->SpawnActor<ASkeletalMeshActor>((*SkMA)->GetClass(), Transform, Parameters);
		
		// Store the marker
		VizKey++;
		SkeletalClones.Emplace(VizKey, SkMClone);
		return VizKey;
	}
	UE_LOG(LogTemp, Error, TEXT("%s::%d Could not find skeletal entity with Id=%s, did you run init before?"), *FString(__func__), __LINE__, *Id);
	return INDEX_NONE;
}

// Get access to the skeletal clone
ASkeletalMeshActor* ASLVizManager::GetSkeletalClone(int32 Key)
{
	if(ASkeletalMeshActor** SkMA = SkeletalClones.Find(Key))
	{
		return *SkMA;
	}
	UE_LOG(LogTemp, Error, TEXT("%s::%d Could not find skeletal entity with Key=%d"), *FString(__func__), __LINE__, Key);
	return nullptr;
}

// Remove skeletal clone
bool ASLVizManager::RemoveSkeletalClone(int32 Key)
{
	ASkeletalMeshActor* SkMA = nullptr;
	if(SkeletalClones.RemoveAndCopyValue(Key,SkMA))
	{
		//SkMA->SetLifeSpan(5);
		SkMA->Destroy();
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Entity with Key=%d was succesfully removed.."), *FString(__func__), __LINE__, Key);
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not find entity with Key=%d"), *FString(__func__), __LINE__, Key);
		return false;
	}
}

// Remove all skeletal clones from the world
void ASLVizManager::ClearSkeletalClones()
{
	for (auto& Pair : SkeletalClones)
	{
		//Entity.Value->SetLifeSpan(5);
		Pair.Value->Destroy();
	}
	SkeletalClones.Empty();
}

// Remove all clones from the world
void ASLVizManager::ClearClones()
{
	ClearEntityClones();
	ClearSkeletalClones();
}

// Apply highlight material to entity
void ASLVizManager::Highlight(const FString& Id, FLinearColor Color, ESLVizHighlightType HighlightType)
{
	if (AStaticMeshActor* SMA = *IdToSMA.Find(Id))
	{
		TArray<UMaterialInterface*> Materials = SMA->GetStaticMeshComponent()->GetMaterials();
		FSLVizMaterialList MatList = FSLVizMaterialList();
		MatList.Materials = Materials;
		IdToOriginalMaterials.Add(Id, MatList);

		
		if (HighlightType == ESLVizHighlightType::Additive)
		{
			UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(MaterialHighlightAdditive, NULL);
			DynMat->SetVectorParameterValue(FName("Color"), Color);

			for (int32 Idx = 0; Idx < SMA->GetStaticMeshComponent()->GetNumMaterials(); Idx++)
			{
				SMA->GetStaticMeshComponent()->SetMaterial(Idx, DynMat);
			}
		}
		else if (HighlightType == ESLVizHighlightType::Translucent)
		{
			UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(MaterialHighlightTranslucent, NULL);
			DynMat->SetVectorParameterValue(FName("Color"), Color);

			for (int32 Idx = 0; Idx < SMA->GetStaticMeshComponent()->GetNumMaterials(); Idx++)
			{
				SMA->GetStaticMeshComponent()->SetMaterial(Idx, DynMat);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not find entity with Id=%s"), *FString(__func__), __LINE__, *Id);
	}
	
}

// Remove highlight from entity
void ASLVizManager::RemoveHighlight(const FString& Id)
{
	if (AStaticMeshActor* SMA = *IdToSMA.Find(Id))
	{
		if(FSLVizMaterialList* MatStruct = IdToOriginalMaterials.Find(Id))
		{
			for (int32 Idx = 0; Idx < MatStruct->Materials.Num(); ++Idx)
			{
				SMA->GetStaticMeshComponent()->SetMaterial(Idx, MatStruct->Materials[Idx]);
			}
			IdToOriginalMaterials.Remove(Id);
		}

	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Could not find entity with Id=%s or entity is not highlighted"), *FString(__func__), __LINE__, *Id);
	}
	
}

// Remove all highlights
void ASLVizManager::ClearHighlights()
{
	for (auto& Pair : IdToOriginalMaterials)
	{
		if (AStaticMeshActor* SMA = *IdToSMA.Find(Pair.Key))
		{
			for (int32 Idx = 0; Idx < Pair.Value.Materials.Num(); ++Idx)
			{
				SMA->GetStaticMeshComponent()->SetMaterial(Idx, Pair.Value.Materials[Idx]);
			}
		}
	}
	IdToOriginalMaterials.Empty();
}

// Get Skeletal Mesh Actor
ASkeletalMeshActor* ASLVizManager::GetSkeletalMeshActor(const FString& Id)
{
	return *IdToSkMA.Find(Id);
}

// Get Static Mesh Actor
AStaticMeshActor* ASLVizManager::GetStaticMeshActor(const FString& Id)
{
	return *IdToSMA.Find(Id);
}

// Load marker meshes and materials
bool ASLVizManager::LoadMarkerAssets()
{
	MeshBox = LoadObject<UStaticMesh>(this, TEXT("StaticMesh'/UViz/SM_Box1m.SM_Box1m'"));
	MeshSphere = LoadObject<UStaticMesh>(this, TEXT("StaticMesh'/UViz/SM_Sphere1m.SM_Sphere1m'"));
	MeshCylinder = LoadObject<UStaticMesh>(this, TEXT("StaticMesh'/UViz/SM_Cylinder1m.SM_Cylinder1m'"));
	MeshArrow = LoadObject<UStaticMesh>(this, TEXT("StaticMesh'/UViz/SM_Arrow1m.SM_Arrow1m'"));
	MeshAxis = LoadObject<UStaticMesh>(this, TEXT("StaticMesh'/UViz/SM_Axis1m.SM_Axis1m'"));

	MaterialLit = LoadObject<UMaterial>(this, TEXT("Material'/UViz/M_MarkerDynamicColorLit.M_MarkerDynamicColorLit'"));
	MaterialUnlit = LoadObject<UMaterial>(this, TEXT("Material'/UViz/M_MarkerDynamicColorUnlit.M_MarkerDynamicColorUnlit'"));
	MaterialHighlightAdditive = LoadObject<UMaterial>(this, TEXT("Material'/UViz/M_HighlightDynamicColorAdditive.M_HighlightDynamicColorAdditive'"));
	MaterialHighlightTranslucent = LoadObject<UMaterial>(this, TEXT("Material'/UViz/M_HighlightDynamicColorTranslucent.M_HighlightDynamicColorTranslucent'"));

	if (!MeshBox)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Failed to load MeshBox assets.."), *FString(__func__), __LINE__);
		return false;
	}
	if (!MeshSphere)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Failed to load MeshSphere assets.."), *FString(__func__), __LINE__);
		return false;
	}
	if (!MeshCylinder)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Failed to load MeshCylinder assets.."), *FString(__func__), __LINE__);
		return false;
	}
	if (!MeshArrow)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Failed to load MeshArrow assets.."), *FString(__func__), __LINE__);
		return false;
	}
	if (!MeshAxis)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Failed to load MeshAxis assets.."), *FString(__func__), __LINE__);
		return false;
	}

	
	if (!MaterialLit)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Failed to load MaterialUnlit assets.."), *FString(__func__), __LINE__);
		return false;
	}
	if (!MaterialUnlit)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Failed to load MaterialHighlightAdditive assets.."), *FString(__func__), __LINE__);
		return false;
	}
	if (!MaterialHighlightAdditive)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Failed to load MaterialHighlightAdditive assets.."), *FString(__func__), __LINE__);
		return false;
	}
	if (!MaterialHighlightTranslucent)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Failed to load MaterialHighlightTranslucent assets.."), *FString(__func__), __LINE__);
		return false;
	}

	return true;
}

// Load the mappings of the unique ids to the entities
void ASLVizManager::LoadEntityMappings()
{
	// Iterate all actors
	for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		FString ActId = FTags::GetValue(*ActorItr, "SemLog", "Id");
		if (!ActId.IsEmpty())
		{
			// Store quick map of id to actor pointer
			if(AStaticMeshActor* AsSMA = Cast<AStaticMeshActor>(*ActorItr))
			{
				IdToSMA.Emplace(ActId, AsSMA);
			}
			else if(ASkeletalMeshActor* AsSkMA = Cast<ASkeletalMeshActor>(*ActorItr))
			{
				IdToSkMA.Emplace(ActId, AsSkMA);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("%s::%d %s is not a static nor a skeletal mesh actor"),
					*FString(__func__), __LINE__, *ActorItr->GetName());
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d %s has no SemLog Id"),
				*FString(__func__), __LINE__, *ActorItr->GetName());
		}
	}
}

// Get mesh instance from enum type
UStaticMesh* ASLVizManager::GetMeshFromType(ESLVizMeshType Type) const
{
	switch (Type)
	{
	case ESLVizMeshType::Box:
		return MeshBox;
	case ESLVizMeshType::Sphere:
		return MeshSphere;
	case ESLVizMeshType::Cylinder:
		return MeshCylinder;
	case ESLVizMeshType::Arrow:
		return MeshArrow;
	case ESLVizMeshType::Axis:
		return MeshAxis;
	default:
		return MeshBox;
	}
}
