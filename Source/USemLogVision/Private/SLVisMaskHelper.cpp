// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLVisMaskHelper.h"
#include "Tags.h"
#include "EngineUtils.h"
#include "Engine/StaticMeshActor.h"
#include "Animation/SkeletalMeshActor.h"
#include "Materials/MaterialInstanceDynamic.h"

// Default constructor
FSLVisMaskHelper::FSLVisMaskHelper()
{
	bIsInit = false;
}

// Init material constructor
FSLVisMaskHelper::FSLVisMaskHelper(UMaterial* InDefaultMaskMaterial)
{
	bIsInit = false;
	InDefaultMaskMaterial->bUsedWithStaticLighting = true;
	DefaultMaskMaterial = InDefaultMaskMaterial;
}

// Init
void FSLVisMaskHelper::Init(UObject* InParent)
{
	UE_LOG(LogTemp, Warning, TEXT("%s::%d INIT"), TEXT(__FUNCTION__), __LINE__);
	if (!bIsInit && InParent->GetWorld() && DefaultMaskMaterial)
	{
		UE_LOG(LogTemp, Warning, TEXT("\t%s::%d IN INIT"), TEXT(__FUNCTION__), __LINE__);
		auto AddDynamicMaskMaterialsLambda = [&](UMeshComponent* MC, const FString& HexColorVal, UObject* Parent)
		{
			TArray<UMaterialInterface*> Materials;
			for (int32 Idx = 0; Idx < MC->GetNumMaterials(); ++Idx)
			{
				UMaterialInstanceDynamic* DynamicMaskMaterial = UMaterialInstanceDynamic::Create(DefaultMaskMaterial, GetTransientPackage());
				DynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"),
					FLinearColor(FColor::FromHex(HexColorVal)));
				Materials.Emplace(DynamicMaskMaterial); 
				UE_LOG(LogTemp, Warning, TEXT("\t\t%s::%d Add MASK. Entity=%s; DynMatName=%s; Hex=%s;"),
					TEXT(__FUNCTION__), __LINE__, *MC->GetOuter()->GetName(), *DynamicMaskMaterial->GetName(), *HexColorVal);
			}
			MeshesMaskMaterials.Emplace(MC, Materials);
		};

		// Iterate static meshes from the world
		for (TActorIterator<AStaticMeshActor> SMAItr(InParent->GetWorld()); SMAItr; ++SMAItr)
		{
			if (UStaticMeshComponent* SMC = SMAItr->GetStaticMeshComponent())
			{
				// Cache original materials
				MeshesOrigMaterials.Add(SMC, SMC->GetMaterials());

				// Check if the entity is annotated with a semantic color mask
				FString ColorHex = FTags::GetValue(*SMAItr,"SemLog", "VisMask");
				if (ColorHex.IsEmpty())
				{
					ColorHex = FTags::GetValue(SMC, "SemLog", "VisMask");
					if (ColorHex.IsEmpty())
					{
						UnknownMeshes.Emplace(SMC);
					}
					else
					{
						//AddDynamicMaskMaterialsLambda(SMC, ColorHex, InParent);

						TArray<UMaterialInterface*> Materials;
						for (int32 Idx = 0; Idx < SMC->GetNumMaterials(); ++Idx)
						{
							UMaterialInstanceDynamic* DynamicMaskMaterial = UMaterialInstanceDynamic::Create(DefaultMaskMaterial, GetTransientPackage());
							DynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"),
								FLinearColor(FColor::FromHex(ColorHex)));
							Materials.Emplace(DynamicMaskMaterial);
							UE_LOG(LogTemp, Warning, TEXT("\t\t%s::%d Add MASK. Entity=%s; DynMatName=%s; Hex=%s;"),
								TEXT(__FUNCTION__), __LINE__, *SMC->GetOuter()->GetName(), *DynamicMaskMaterial->GetName(), *ColorHex);
						}
						MeshesMaskMaterials.Emplace(SMC, Materials);
					}
				}
				else
				{
					//AddDynamicMaskMaterialsLambda(SMC, ColorHex, InParent);

					TArray<UMaterialInterface*> Materials;
					for (int32 Idx = 0; Idx < SMC->GetNumMaterials(); ++Idx)
					{
						UMaterialInstanceDynamic* DynamicMaskMaterial = UMaterialInstanceDynamic::Create(DefaultMaskMaterial, GetTransientPackage());
						DynamicMaskMaterial->SetVectorParameterValue(FName("MaskColorParam"),
							FLinearColor(FColor::FromHex(ColorHex)));
						Materials.Emplace(DynamicMaskMaterial);
						UE_LOG(LogTemp, Warning, TEXT("\t\t%s::%d Add MASK. Entity=%s; DynMatName=%s; Hex=%s;"),
							TEXT(__FUNCTION__), __LINE__, *SMC->GetOuter()->GetName(), *DynamicMaskMaterial->GetName(), *ColorHex);
					}
					MeshesMaskMaterials.Emplace(SMC, Materials);
				}
			}
		}

		// TODO include semantic data from skeletal meshes as well
		// Iterate skeletal meshes
		for (TActorIterator<ASkeletalMeshActor> SkMAItr(InParent->GetWorld()); SkMAItr; ++SkMAItr)
		{
			if (USkeletalMeshComponent* SkMC = SkMAItr->GetSkeletalMeshComponent())
			{
				// Add original materials
				MeshesOrigMaterials.Add(SkMC, SkMC->GetMaterials());

				// Check if the entity is annotated with a semantic color mask
				// TODO how to read bone colors
				FString ColorHex = FTags::GetValue(*SkMAItr, "SemLog", "VisMask");
				if (ColorHex.IsEmpty())
				{
					ColorHex = FTags::GetValue(SkMC, "SemLog", "VisMask");
					if (ColorHex.IsEmpty())
					{
						UnknownMeshes.Emplace(SkMC);
					}
					else
					{
						//AddDynamicMaskMaterialsLambda(SkMC, ColorHex, InParent);
					}
				}
				else
				{
					//AddDynamicMaskMaterialsLambda(SkMC, ColorHex, InParent);
				}
			}
		}
		
		for (auto& MeshMatPair : MeshesMaskMaterials)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s::%d VIEW mask of %s"),
				TEXT(__FUNCTION__), __LINE__, *MeshMatPair.Key->GetOuter()->GetName());
			int32 MatIdx = 0;
			for (auto& Mat : MeshMatPair.Value)
			{
				UE_LOG(LogTemp, Warning, TEXT("\t%s::%d Mat name=%s"),
					TEXT(__FUNCTION__), __LINE__, *Mat->GetName());
				//MeshMatPair.Key->SetMaterial(MatIdx, Mat);
				MatIdx++;
			}
		}


		bIsInit = true;
	}
}

// Apply mask materials
bool FSLVisMaskHelper::ApplyMaskMaterials()
{
	if (!bIsInit)
	{
		return false;
	}

	UE_LOG(LogTemp, Warning, TEXT("\t%s::%d Setting mask materials.."), TEXT(__FUNCTION__), __LINE__);
	for (auto& MeshMatPair : MeshesMaskMaterials)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Apply mask to %s"),
			TEXT(__FUNCTION__), __LINE__, *MeshMatPair.Key->GetOuter()->GetName());
		int32 MatIdx = 0;
		for (auto& Mat : MeshMatPair.Value)
		{
			//UE_LOG(LogTemp, Warning, TEXT("\t%s::%d Mat name=%s"),
			//	TEXT(__FUNCTION__), __LINE__, *Mat->GetName());
			//MeshMatPair.Key->SetMaterial(MatIdx, Mat);
			MatIdx++;
		}
	}

	for (auto& Mesh : UnknownMeshes)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Apply mask to %s"),
			TEXT(__FUNCTION__), __LINE__, *Mesh->GetOuter()->GetName());
		for (int32 Idx = 0; Idx < Mesh->GetNumMaterials(); ++Idx)
		{
			UE_LOG(LogTemp, Warning, TEXT("\t%s::%d Mat name=%s"),
				TEXT(__FUNCTION__), __LINE__, *DefaultMaskMaterial->GetName());
			//Mesh->SetMaterial(Idx, DefaultMaskMaterial);
		}
	}
	return true;
}

// Apply original materials
bool FSLVisMaskHelper::ApplyOriginalMaterials()
{
	if (!bIsInit)
	{
		return false;
	}

	UE_LOG(LogTemp, Warning, TEXT("\t%s::%d Setting orig materials.."), TEXT(__FUNCTION__), __LINE__);
	for (auto& MeshMatPair : MeshesMaskMaterials)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%d Apply mask to %s"),
			TEXT(__FUNCTION__), __LINE__, *MeshMatPair.Key->GetOuter()->GetName());
		int32 MatIdx = 0;
		for (auto& Mat : MeshMatPair.Value)
		{
			UE_LOG(LogTemp, Warning, TEXT("\t%s::%d Mat name=%s"),
				TEXT(__FUNCTION__), __LINE__, *Mat->GetName());
			//MeshMatPair.Key->SetMaterial(MatIdx, Mat);
			MatIdx++;
		}
	}
	return true;
}