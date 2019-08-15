// Copyright 2017-2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLVisMaskHandler.h"
#include "Engine/StaticMeshActor.h"
#include "Animation/SkeletalMeshActor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "EngineUtils.h"
#include "SLSkeletalDataComponent.h"
#include "Tags.h"
#include "SLVisImageWriterInterface.h"

#define SLVIS_SEM_TOL 21
#define SLVIS_BLACK_TOL 5


// Ctor
USLVisMaskHandler::USLVisMaskHandler()
{
	bIsInit = false;
	bMaskMaterialsOn = false;
};

// Dtor
USLVisMaskHandler::~USLVisMaskHandler()
{
}

// Init
void USLVisMaskHandler::Init(int32 InNumOfPixels)
{
	if (!bIsInit)
	{
		// Used for calculating the percentage in order to ignore barely visible entities
		TotalNumOfPixelsInImg = InNumOfPixels;
		
		// Load default mask material
		DefaultMaskMaterial = LoadObject<UMaterial>(this,
			TEXT("/USemLog/Vision/M_SLDefaultMask.M_SLDefaultMask"));
		if (DefaultMaskMaterial)
		{
			DefaultMaskMaterial->bUsedWithStaticLighting = true;
			DefaultMaskMaterial->bUsedWithSkeletalMesh = true;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%d Could not load default mask material! NOT initalized.."),
				*FString(__func__), __LINE__);
			return;
		}

		// Save the original color materials of the static meshes, and create a for each mesh a mask material
		SetupStaticMeshes();

		// Save the original color materials of the skeletal meshes, and create a mask material for each semantically annotated bone
		SetupSkeletalMeshes();

		bIsInit = true;
	}
};

// Save the original color materials of the static meshes, and create a for each mesh a mask material
void USLVisMaskHandler::SetupStaticMeshes()
{	
	for (TActorIterator<AStaticMeshActor> SMAItr(GetWorld()); SMAItr; ++SMAItr)
	{
		if (UStaticMeshComponent* SMC = SMAItr->GetStaticMeshComponent())
		{
			// Store the original materials
			OriginalMaterials.Emplace(SMC, SMC->GetMaterials());

			// The static mesh has one mask color, check if the hex value is stored by the actor or component
			FString ColorHex = FTags::GetValue(*SMAItr, "SemLog", "VisMask");
			if (!ColorHex.IsEmpty())
			{
				FColor SemColor(FColor::FromHex(ColorHex));
				AddSemanticData(SemColor, ColorHex, SMAItr->Tags, *SMAItr);
				UMaterialInstanceDynamic* DynamicMaskMaterial = UMaterialInstanceDynamic::Create(DefaultMaskMaterial, GetTransientPackage());
				DynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"), FLinearColor::FromSRGBColor(SemColor));
				MaskMaterials.Emplace(SMC, DynamicMaskMaterial);
			}
			else
			{
				ColorHex = FTags::GetValue(SMC, "SemLog", "VisMask");
				if (!ColorHex.IsEmpty())
				{
					FColor SemColor(FColor::FromHex(ColorHex));
					AddSemanticData(SemColor, ColorHex, SMC->ComponentTags, SMC);
					UMaterialInstanceDynamic* DynamicMaskMaterial = UMaterialInstanceDynamic::Create(DefaultMaskMaterial, GetTransientPackage());
					DynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"), FLinearColor::FromSRGBColor(SemColor));
					MaskMaterials.Emplace(SMC, DynamicMaskMaterial);
				}
				else
				{
					IgnoredMeshes.Emplace(SMC);
				}
			}
		}
	}
}

// Save the original color materials of the skeletal meshes, and create a mask material for each semantically annotated bone
void USLVisMaskHandler::SetupSkeletalMeshes()
{
	// Iterate all skeletal meshes and check if they have a skeletal data component
	for (TActorIterator<ASkeletalMeshActor> SkMAItr(GetWorld()); SkMAItr; ++SkMAItr)
	{
		if (USkeletalMeshComponent* SkMC = SkMAItr->GetSkeletalMeshComponent())
		{
			// Store the original materials
			OriginalMaterials.Emplace(SkMC, SkMC->GetMaterials());

			// Get the mask materials of the bones
			if (USLSkeletalDataComponent* SkelDataComp = Cast<USLSkeletalDataComponent>(
				SkMAItr->GetComponentByClass(USLSkeletalDataComponent::StaticClass())))
			{
				// Make sure the owner semantic data is set
				if (SkelDataComp->Init())
				{
					const FString OwnerId = SkelDataComp->OwnerSemanticData.Id;
					const FString OwnerClass = SkelDataComp->OwnerSemanticData.Class;

					// Skeletal mesh mask materials, init with mesh and the default materials
					FSLVisSkelMeshMasks SkelMeshMasks(SkMC, DefaultMaskMaterial);

					// Iterate the bones and add the mask materials
					for (const auto& BoneDataPair : SkelDataComp->SemanticBonesData)
					{
						const FString BoneColorHex = BoneDataPair.Value.MaskColorHex;
						const FString BoneClass = BoneDataPair.Value.Class;

						FColor SemColor(FColor::FromHex(BoneColorHex));
						FTransform DummyWorldTransform; // TODO get the bone world transform
						AddSkelSemanticData(OwnerId, OwnerClass, SemColor, BoneColorHex, BoneClass, DummyWorldTransform);
						UMaterialInstanceDynamic* DynamicMaskMaterial = UMaterialInstanceDynamic::Create(DefaultMaskMaterial, GetTransientPackage());
						DynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"), FLinearColor::FromSRGBColor(SemColor));

						SkelMeshMasks.MaskMaterials.Emplace(BoneDataPair.Value.MaskMaterialIndex, DynamicMaskMaterial);
					}

					SkelMaskMaterials.Emplace(SkelMeshMasks);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("%s::%d Cannot init skeletal data component.. (this should not happen)"), *FString(__func__), __LINE__);
				}
			}
			else
			{
				IgnoredMeshes.Emplace(SkMC);
			}
		}
	}
}

// Apply mask materials
bool USLVisMaskHandler::ApplyMaskMaterials()
{
	if (bIsInit)
	{
		for (const auto& MeshMatPair : MaskMaterials)
		{
			for (int32 Idx = 0; Idx < MeshMatPair.Key->GetNumMaterials(); ++Idx)
			{
				MeshMatPair.Key->SetMaterial(Idx, MeshMatPair.Value);
			}
		}
		for (auto& SkelMat : SkelMaskMaterials)
		{
			SkelMat.ApplyMaterials();
		}
		for (auto& Mesh : IgnoredMeshes)
		{
			for (int32 Idx = 0; Idx < Mesh->GetNumMaterials(); ++Idx)
			{
				Mesh->SetMaterial(Idx, DefaultMaskMaterial);
			}
		}
		bMaskMaterialsOn = true;
		return true;
	}
	return false;
}

// Apply original materials
bool USLVisMaskHandler::ApplyOriginalMaterials()
{
	if (bIsInit)
	{
		for (const auto& MeshMatPair : OriginalMaterials)
		{
			int32 Idx = 0;
			for (auto& Mat : MeshMatPair.Value)
			{
				MeshMatPair.Key->SetMaterial(Idx, Mat);
				Idx++;
			}
		}
		bMaskMaterialsOn = false;
		return true;
	}
	return false;
}

// Toggle between the mask and original materials
bool USLVisMaskHandler::ToggleMaterials()
{
	if (bMaskMaterialsOn)
	{
		return USLVisMaskHandler::ApplyOriginalMaterials();
	}
	else
	{
		return USLVisMaskHandler::ApplyMaskMaterials();
	}
}

// Process the semantic mask image, fix pixel color deviations in image, return semantic data from the image
void USLVisMaskHandler::ProcessMaskImage(TArray<FColor>& MaskImage, const FTransform& ViewWorldTransform, FSLVisViewData& OutViewData)
{
	// Temp map for easy updating of the entity data and avoiding duplicates, will be outputted as an array
	TMap<FColor, FSLVisEntitiyData> EntitiesInImage; // Cache which static meshes are in the image
	TMap<FColor, FSLVisBoneData> BonesInImage; // Cache which bones are in the image

	//TSet<FColor> ColorsInRange;
	//TSet<FColor> ColorsOutOfRange;

	// Image array index value
	int32 Idx = 0;

	// Iterate mask image
	for (auto& PixelColor : MaskImage)
	{
		// Continue if it is different than black with a tolerance
		if (!AlmostEqual(PixelColor, FColor::Black, SLVIS_BLACK_TOL))
		{
			FColor TempColor = PixelColor;
			// Check and replace if color got deviated from the semantic one due to conversions (FLinearColor to FColor)
			if (RestoreIfCloseToAMaskColor(PixelColor))
			{
				//ColorsInRange.Add(TempColor); // for debug 
			}

			// Check if pixel color belongs to a semantic mask color of a static mesh
			if (EntitiesMasks.Contains(PixelColor))
			{
				// Check if color has been cached in the temp map
				if(EntitiesInImage.Contains(PixelColor))
				{
					// Update the existing data
					EntitiesInImage[PixelColor].NumPixels++;
				}
				else
				{
					FSLVisEntitiyData EntityData = EntitiesMasks[PixelColor];
					if (EntityData.SelfAsActor)
					{
						EntityData.TransformFromView = ViewWorldTransform.GetRelativeTransform(EntityData.SelfAsActor->GetTransform());
						EntityData.LinearDistanceToView = FVector::Distance(ViewWorldTransform.GetLocation(), EntityData.SelfAsActor->GetActorLocation());
						EntityData.AngularDistanceToView = ViewWorldTransform.GetRotation().AngularDistance(EntityData.SelfAsActor->GetActorQuat());
					}
					else if (EntityData.SelfAsComponent)
					{
						EntityData.TransformFromView = ViewWorldTransform.GetRelativeTransform(EntityData.SelfAsComponent->GetComponentTransform());
						EntityData.LinearDistanceToView = FVector::Distance(ViewWorldTransform.GetLocation(), EntityData.SelfAsComponent->GetComponentLocation());
						EntityData.AngularDistanceToView = ViewWorldTransform.GetRotation().AngularDistance(EntityData.SelfAsComponent->GetComponentQuat());
					}
					// Add init entity data
					EntitiesInImage.Emplace(PixelColor, EntityData);
				}
			}
			else if (BonesMasks.Contains(PixelColor)) // Check if color belongs to a skeletal bone
			{
				// Check if color has already been cached
				if (BonesInImage.Contains(PixelColor))
				{
					// Update the data
					BonesInImage[PixelColor].NumPixels++;
				}
				else
				{
					// Add the bone to the first time
					BonesInImage.Emplace(PixelColor, BonesMasks[PixelColor]);
				}
			}
			else
			{
				// Pixel color not found int the mask semantic list, will be changed to black
				//ColorsOutOfRange.Add(Color); // for debug 
				PixelColor = FColor::Black;
			}
		}
		// Increment array position index 
		Idx++;
	}

	//UE_LOG(LogTemp, Warning, TEXT("%s::%d Semantic colors:"), *FString(__func__), __LINE__);
	//for (const auto& C : SemanticColors)
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("\t\t%s"), *C.ToString());
	//}

	//UE_LOG(LogTemp, Warning, TEXT("%s::%d Colors in range:"), *FString(__func__), __LINE__);
	//for (const auto& C : ColorsInRange)
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("\t\t%s"), *C.ToString());
	//}

	//UE_LOG(LogTemp, Error, TEXT("%s::%d Colors OUT OF range:"), *FString(__func__), __LINE__);
	//for (const auto& C : ColorsOutOfRange)
	//{
	//	UE_LOG(LogTemp, Error, TEXT("\t\t%s"), *C.ToString());
	//}


	// Iterate on all the color to bones pair and generate skeletal structure data
	for (const auto& ColorToBonePair : BonesInImage)
	{
		FSLVisBoneData BoneData = ColorToBonePair.Value;

		bool bParentFound = false;
		// Check if bone data parent is created
		for (auto& SkelParent : OutViewData.SemanticSkelEntities)
		{
			// If parent ID is the same with the Bone owner id, add the bone to the parents list
			if (SkelParent.Id.Equals(BoneData.OwnerId))
			{
				SkelParent.BonesData.AddUnique(BoneData);
				bParentFound = true;
				break;
			}
		}

		// If parent was not found, create one
		if (!bParentFound)
		{
			FSLVisSkelData Parent(BoneData.OwnerId, BoneData.OwnerClass);
			Parent.BonesData.Emplace(BoneData);
			OutViewData.SemanticSkelEntities.Emplace(Parent);
		}
	}
	
	// Remove entities that have a very small relative number of pixels in the image and set the avg distance in the scene
	SetOutputData(EntitiesInImage, BonesInImage, OutViewData);
}

// Add information about the semantic color (return true if all the fields were filled)
void USLVisMaskHandler::AddSemanticData(const FColor& Color, const FString& ColorHex, const TArray<FName>& Tags, UObject* Self)
{
	if (USLVisMaskHandler::AlmostEqual(Color, FColor::Black, SLVIS_BLACK_TOL))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Semantic color is (almost) black, this should not happen, skipping.."), *FString(__func__), __LINE__);
		return;
	}
	FSLVisEntitiyData EntityData;
	EntityData.Color = Color;
	EntityData.ColorHex = ColorHex;
	EntityData.Class = FTags::GetValue(Tags, "SemLog", "Class");
	EntityData.Id = FTags::GetValue(Tags, "SemLog", "Id");
	if (Self)
	{
		if (AStaticMeshActor* AsSMA = Cast<AStaticMeshActor>(Self))
		{
			EntityData.SelfAsActor = AsSMA;
		}
		else if (UStaticMeshComponent* AsSMC = Cast<UStaticMeshComponent>(Self))
		{
			EntityData.SelfAsComponent = AsSMC;
		}
	}

	MaskColors.Emplace(Color);
	EntitiesMasks.Emplace(Color, EntityData);

	//UE_LOG(LogTemp, Warning, TEXT("%s::%d EntityData=\n\t%s"), *FString(__func__), __LINE__, *EntityData.ToString());
}

// Store the information about the skeletal semantic color
void USLVisMaskHandler::AddSkelSemanticData(const FString& OwnerId,
	const FString& OwnerClass,
	const FColor& Color,
	const FString& ColorHex,
	const FString& BoneClass,
	const FTransform& WorldTransform)
{
	if (USLVisMaskHandler::AlmostEqual(Color, FColor::Black, SLVIS_BLACK_TOL))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%d Semantic color is (almost) black, this should not happen, skipping.."), *FString(__func__), __LINE__);
		return;
	}
	FSLVisBoneData BoneData;
	BoneData.OwnerId = OwnerId;
	BoneData.OwnerClass = OwnerClass;
	BoneData.Color = Color;
	BoneData.ColorHex = ColorHex;
	BoneData.Class = BoneClass;
	BoneData.TransformFromWorld = WorldTransform;

	MaskColors.Emplace(Color);
	BonesMasks.Emplace(Color, BoneData);

	//UE_LOG(LogTemp, Warning, TEXT("%s::%d EntityData=\n\t%s"), *FString(__func__), __LINE__, *EntityData.ToString());
}

// Compare against the semantic colors, if found switch (update color info during), returns true if the color has been switched
bool USLVisMaskHandler::RestoreIfCloseToAMaskColor(FColor& OutColor)
{
	// Lambda for the FindByPredicate function, checks if the two colors are similar 
	auto AlmostEqualPredicate = [this, &OutColor](const FColor& SemColor)
	{
		return this->AlmostEqual(OutColor, SemColor, SLVIS_SEM_TOL);
	};

	// If the two colors are similar, replace it with the semantic value
	if (FColor* FoundSemanticColor = MaskColors.FindByPredicate(AlmostEqualPredicate))
	{
		// Do the switch
		OutColor = *FoundSemanticColor;
		return true;
	}
	return false;
}

// Remove entities that have a very small relative number of pixels in the image and set the avg distance in the scene
void USLVisMaskHandler::SetOutputData(TMap<FColor, FSLVisEntitiyData>& EntitiesInImage,
	TMap<FColor, FSLVisBoneData>& BonesInImage,
	FSLVisViewData& OutViewData)
{
	const int32 RelativeMinNumOfPixels = TotalNumOfPixelsInImg * IgnorePercentage;
	//float TotalLinDistances = 0.f;
	//float TotalAngDistances = 0.f;
	
	// TODO skeletal information is not handled here
	for (auto EntityItr(EntitiesInImage.CreateIterator()); EntityItr; ++EntityItr)
	{
		// Remove entities that are `barely` in the scene
		if(EntityItr->Value.NumPixels < AbsoluteMinNumOfPixels 
			|| EntityItr->Value.NumPixels < RelativeMinNumOfPixels)
		{
			EntityItr.RemoveCurrent();
		}
		else
		{
			OutViewData.TotalLinearDistanceSize += EntityItr->Value.LinearDistanceToView;
			OutViewData.TotalAngularDistanceSize += EntityItr->Value.AngularDistanceToView;
			//TotalLinDistances += EntityItr->Value.LinearDistanceToView;
			//TotalAngDistances += EntityItr->Value.AngularDistanceToView;
		}
	}
	
	// Output the semantic data from the image
	OutViewData.NumEntities = EntitiesInImage.Num();
	//OutViewData.TotalLinearDistanceSize = TotalLinDistances / OutViewData.NumEntities;
	//OutViewData.TotalAngularDistanceSize = TotalAngDistances / OutViewData.NumEntities;
	EntitiesInImage.GenerateValueArray(OutViewData.SemanticEntities);
}
