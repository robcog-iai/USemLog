// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Editor/SLSemanticMapWriter.h"

#include "PhysicsEngine/PhysicsConstraintActor.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Animation/SkeletalMeshActor.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "UObject/UObjectGlobals.h" // DuplicateObject

// UOwl
#include "Owl/SLOwlSemanticMap.h"
#include "Owl/SLOwlSemanticMapStatics.h"

// UUtils
#include "Utils/SLTagIO.h"
#include "Utils/SLUuid.h"

#if SL_WITH_ROS_CONVERSIONS
#include "Conversions.h"
#endif // SL_WITH_ROS_CONVERSIONS

// Default constructor
FSLSemanticMapWriter::FSLSemanticMapWriter()
{
}

// Write semantic map to file
bool FSLSemanticMapWriter::WriteToFile(UWorld* World, const FSLSemanticMapWriterParams& InParams)
{
	FString FullFilePath = FPaths::ProjectDir() + TEXT("/") + InParams.DirectoryPath + TEXT("/") + InParams.Id + TEXT("_SM.owl");
	FPaths::RemoveDuplicateSlashes(FullFilePath);

	// Check if map already exists
	if (!InParams.bOverwrite && FPaths::FileExists(FullFilePath))
	{
		return false;
	}

	// Create the semantic map template
	TSharedPtr<FSLOwlSemanticMap> SemMap = CreateSemanticMapDocTemplate(InParams.TemplateType, InParams.Id);

	// Add map individual
	SemMap->AddSemanticMapIndividual(InParams.Description, InParams.Level);

	// Add individuals to map
	AddWorldIndividuals(SemMap, World);

	// Write map to file	
	return FFileHelper::SaveStringToFile(SemMap->ToString(), *FullFilePath);
}

// Create semantic map template
TSharedPtr<FSLOwlSemanticMap> FSLSemanticMapWriter::CreateSemanticMapDocTemplate(
	ESLOwlSemanticMapTemplate TemplateType, const FString& InSemMapId)
{
	return FSLOwlSemanticMapStatics::CreateDefaultSemanticMap(InSemMapId, TEXT("log"), TEXT("ameva_log"));
	//const FString DocId = InSemMapId.IsEmpty() ? FSLUuid::NewGuidInBase64Url() : InSemMapId;

	//if (TemplateType == ESLOwlSemanticMapTemplate::Default)
	//{
	//	return FSLOwlSemanticMapStatics::CreateDefaultSemanticMap(DocId, InLevelName, TEXT("log"), TEXT("ameva"));
	//}
	//else if (TemplateType == ESLOwlSemanticMapTemplate::IAIKitchen)
	//{
	//	return FSLOwlSemanticMapStatics::CreateDefaultSemanticMap(DocId, InLevelName, TEXT("log"), TEXT("ameva"));
	//}
	//else if (TemplateType == ESLOwlSemanticMapTemplate::IAISupermarket)
	//{
	//	return FSLOwlSemanticMapStatics::CreateDefaultSemanticMap(DocId, InLevelName, TEXT("log"), TEXT("ameva"));
	//}
	//return MakeShareable(new FSLOwlSemanticMap());
}

// Add individuals to the semantic map
void FSLSemanticMapWriter::AddWorldIndividuals(TSharedPtr<FSLOwlSemanticMap> InSemMap, UWorld* World)
{
	// Iterate objects with SemLog tag key
	for (const auto& ActorPairs : FSLTagIO::GetWorldKVPairs(World, "SemLog"))
	{
		// Get Id and Class of items
		const FString* IdPtr = ActorPairs.Value.Find("Id");
		const FString* ClassPtr = ActorPairs.Value.Find("Class");

		// Take into account only objects with an id
		if (IdPtr)
		{
			// Check if class is also available
			if (ClassPtr)
			{
				AddObjectIndividual(InSemMap, ActorPairs.Key, *IdPtr, *ClassPtr);
			}
			// No class is available, check for other types, e.g. constraints can be actors or components
			else if (APhysicsConstraintActor* ConstrAct = Cast<APhysicsConstraintActor>(ActorPairs.Key))
			{
				AddConstraintIndividual(InSemMap, ConstrAct->GetConstraintComp(), *IdPtr, ConstrAct->Tags);
			}
		}

		// Add class individuals (Id not mandatory)
		if (ClassPtr)
		{
			const FString* SubClassOfPtr = ActorPairs.Value.Find("SubClassOf");
			const FString SubClassOf = SubClassOfPtr ? *SubClassOfPtr : "";
			AddClassDefinition(InSemMap, ActorPairs.Key, *ClassPtr, SubClassOf);
		}
	}
}

// Add object individual to the semantic map
void FSLSemanticMapWriter::AddObjectIndividual(TSharedPtr<FSLOwlSemanticMap> InSemMap,
	UObject* Object, const FString& InId, const FString& InClass)
{
	// Get map data
	const FString MapPrefix = InSemMap->Prefix;
	const FString DocId = InSemMap->Id;

	// Create the object individual
	FSLOwlNode ObjIndividual = FSLOwlSemanticMapStatics::CreateObjectIndividual(MapPrefix, InId, InClass);

	// Add describedInMap property
	ObjIndividual.AddChildNode(FSLOwlSemanticMapStatics::CreateDescribedInMapProperty(MapPrefix, DocId));
	
	// Add parent property
	const FString ParentId = GetParentId(Object);
	if (!ParentId.IsEmpty())
	{
		ObjIndividual.AddChildNode(FSLOwlSemanticMapStatics::CreateParentProperty(MapPrefix, ParentId));
	}

	// Add child properties
	TArray<FString> ChildIds;
	GetChildIds(Object, ChildIds);
	for (const auto& ChildId : ChildIds)
	{
		ObjIndividual.AddChildNode(FSLOwlSemanticMapStatics::CreateChildProperty(MapPrefix, ChildId));
	}

	// Add mobility property
	const FString Mobility = GetMobility(Object);
	if (!Mobility.IsEmpty())
	{
		ObjIndividual.AddChildNode(FSLOwlSemanticMapStatics::CreateMobilityProperty(Mobility));
	}

	// Add physics properties (gravity, overlap events, mass)
	TArray<FSLOwlNode> PhysicsProperties = GetIndividualPhysicsProperties(Object);
	if (PhysicsProperties.Num() > 0)
	{
		ObjIndividual.AddChildNodes(PhysicsProperties);
	}
	
	// Add color property
	if (AActor* ObjAsAct = Cast<AActor>(Object))
	{
		FString ColorHex = FSLTagIO::GetValue(ObjAsAct, "SemLog", "VisMask");
		if (!ColorHex.IsEmpty())
		{
			ObjIndividual.AddChildNode(FSLOwlSemanticMapStatics::CreateMaskColorProperty(ColorHex));
		}
	}

	// Lambda to remove path before and including "Models/", after and including ".", and the "SM_" prefixes 
	auto GetPathToCadModelLambda = [](UObject* Obj)->FString
	{
		FString Path = Obj->GetFullName();
		const FString SubStr("Models/");
		int32 FindIdx = Path.Find(SubStr, ESearchCase::CaseSensitive);
		Path.RemoveAt(0, FindIdx + SubStr.Len());
		Path.FindLastChar('.', FindIdx);
		Path.RemoveAt(FindIdx, Path.Len() - FindIdx);
		Path.ReplaceInline(*FString("SM_"), *FString(""));
		return Path;
	};

	// Add various properties
	if (AActor* ObjAsAct = Cast<AActor>(Object))
	{
		// Generate unique id for the properties
		const FString PoseId = FSLUuid::NewGuidInBase64Url();

		/* Add properties */
		// Pose property
		ObjIndividual.AddChildNode(FSLOwlSemanticMapStatics::CreatePoseProperty(MapPrefix, PoseId));


		// Commented out since value it is stored as a class property
		//// If static mesh, add pathToCadModel property
		//if (AStaticMeshActor* ActAsSMA = Cast<AStaticMeshActor>(ObjAsAct))
		//{
		//	if (UStaticMeshComponent* SMC = ActAsSMA->GetStaticMeshComponent())
		//	{
		//		if (UStaticMesh* SM = SMC->GetStaticMesh())
		//		{
		//			//ObjIndividual.AddChildNode(FSLOwlSemanticMapStatics::CreatePathToCadModelProperty(GetPathToCadModelLambda(SM)));
		//			ObjIndividual.AddChildNode(FSLOwlSemanticMapStatics::CreatePathToCadModelProperty(InClass));
		//		}
		//	}
		//}

		// If skeletalmesh, add bones as properties
		// Cache created bone individuals (if any)
		TArray<FSLOwlNode> BoneIndividuals;
		if (ASkeletalMeshActor* ActAsSkMA = Cast<ASkeletalMeshActor>(ObjAsAct))
		{
			if (USkeletalMeshComponent* SkelComp = ActAsSkMA->GetSkeletalMeshComponent())
			{
				// Create and cache unique ids for every bone name
				TMap<FName, FString> CachedBoneIds;

				// Add bones to the skeletal individual, cache the ids to access them later on
				TArray<FName> BoneNames;
				SkelComp->GetBoneNames(BoneNames);
				for (const auto& BoneName : BoneNames)
				{
					// TODO read bone ids from data structure
					const FString BoneId = FSLUuid::NewGuidInBase64Url();
					ObjIndividual.AddChildNode(FSLOwlSemanticMapStatics::CreateSrdlSkeletalBoneProperty(MapPrefix, BoneId));
					CachedBoneIds.Add(BoneName, BoneId);
				}


				// Create bone individuals
				for (auto& BoneName : BoneNames)
				{
					const FString BoneNameStr = BoneName.ToString();

					FString BoneId = "IdNotFound";
					if (FString* FoundId = CachedBoneIds.Find(BoneName))
					{
						BoneId = *FoundId;
					}
				
					// Check for parent bone
					FString BaseLinkId = "";
					FName ParentBone = SkelComp->GetParentBone(BoneName);
					if (!ParentBone.IsNone())
					{
						if (FString* FoundBaseId = CachedBoneIds.Find(ParentBone))
						{
							BaseLinkId = *FoundBaseId;
						}
					}
					
					// Check for child bone
					FString EndLinkId = "";
					// Check child link by iterating all bones and checking if their parent equals this
					for (auto& ChildName : BoneNames)
					{
						FName CurrParent = SkelComp->GetParentBone(ChildName);
						if (CurrParent.IsEqual(BoneName))
						{
							if (FString* FoundEndId = CachedBoneIds.Find(ChildName))
							{
								EndLinkId = *FoundEndId;
							}
							break;
						}
					}

					// TODO read class from datastructure, otherwise use the bone name
					const FString BoneClass = BoneNameStr;
					BoneIndividuals.Add(FSLOwlSemanticMapStatics::CreateBoneIndividual(MapPrefix, BoneId, BoneClass, BaseLinkId, EndLinkId, BoneNameStr));

					// Direct access to bone name
					//ObjIndividual.AddChildNode(FSLOwlSemanticMapStatics::CreateSkeletalBoneProperty(BoneNameStr));
				}
			}
		}

		// Add tags data property
		ObjIndividual.AddChildNode(FSLOwlSemanticMapStatics::CreateTagsDataProperty(
			ObjAsAct->Tags));

		// Add skeletal individual
		InSemMap->AddIndividual(ObjIndividual);

		// Create pose individual
#if SL_WITH_ROS_CONVERSIONS
		const FVector ROSLoc = FConversions::UToROS(ObjAsAct->GetActorLocation());
		const FQuat ROSQuat = FConversions::UToROS(ObjAsAct->GetActorQuat());
		InSemMap->AddIndividual(FSLOwlSemanticMapStatics::CreatePoseIndividual(
			MapPrefix, PoseId, ROSLoc, ROSQuat));
#else
		InSemMap->AddIndividual(FSLOwlSemanticMapStatics::CreatePoseIndividual(
			MapPrefix, PoseId, ObjAsAct->GetActorLocation(), ObjAsAct->GetActorQuat()));
#endif // SL_WITH_ROS_CONVERSIONS

		// Create and add skeletal bones individuals (if any)
		if (BoneIndividuals.Num())
		{
			InSemMap->AddIndividuals(BoneIndividuals);
		}
	}
	else if (USceneComponent* ObjAsSceneComp = Cast<USceneComponent>(Object))
	{
		// Generate unique id for the properties
		const FString PoseId = FSLUuid::NewGuidInBase64Url();
		const FString TagsId = FSLUuid::NewGuidInBase64Url();

		// Add properties
		ObjIndividual.AddChildNode(FSLOwlSemanticMapStatics::CreatePoseProperty(
			MapPrefix, PoseId));

		// If static mesh, add pathToCadModel property
		if (UStaticMeshComponent* CompAsSMC = Cast<UStaticMeshComponent>(ObjAsSceneComp))
		{
			if (UStaticMesh* SM = CompAsSMC->GetStaticMesh())
			{
				//ObjIndividual.AddChildNode(FSLOwlSemanticMapStatics::CreatePathToCadModelProperty(GetPathToCadModelLambda(SM)));
				ObjIndividual.AddChildNode(FSLOwlSemanticMapStatics::CreatePathToCadModelProperty(InClass));
			}
		}

		// Add tags data property
		ObjIndividual.AddChildNode(FSLOwlSemanticMapStatics::CreateTagsDataProperty(
			ObjAsSceneComp->ComponentTags));
		
		// Add individuals 
		InSemMap->AddIndividual(ObjIndividual);

		// Create pose individual
#if SL_WITH_ROS_CONVERSIONS
		const FVector ROSLoc = FConversions::UToROS(ObjAsSceneComp->GetComponentLocation());
		const FQuat ROSQuat = FConversions::UToROS(ObjAsSceneComp->GetComponentQuat());
		InSemMap->AddIndividual(FSLOwlSemanticMapStatics::CreatePoseIndividual(
			MapPrefix, PoseId, ROSLoc, ROSQuat));
#else
		InSemMap->AddIndividual(FSLOwlSemanticMapStatics::CreatePoseIndividual(
			MapPrefix, PoseId, ObjAsSceneComp->GetComponentLocation(), ObjAsSceneComp->GetComponentQuat()));
#endif // SL_WITH_ROS_CONVERSIONS
	}
	else
	{
		// Obj has no pose info
		InSemMap->AddIndividual(ObjIndividual);
	}
}

// Add class individual
void FSLSemanticMapWriter::AddClassDefinition(TSharedPtr<FSLOwlSemanticMap> InSemMap,
	UObject* Object,
	const FString& InClass,
	const FString& InSubClassOf)
{
	// Return if class was already defined
	for (const auto& ClassDef : InSemMap->ClassDefinitions)
	{
		for (const auto& ClassAttr : ClassDef.Attributes)
		{
			if (ClassAttr.Key.Prefix.Equals("rdf") &&
				ClassAttr.Key.LocalName.Equals("about") &&
				ClassAttr.Value.LocalValue.Equals(InClass))
			{
				return;
			}
		}
	}

	// Create class definition individual
	FSLOwlNode ClassDefinition = FSLOwlSemanticMapStatics::CreateClassDefinition(InClass);
	ClassDefinition.Comment = TEXT("Class ") + InClass;

	// If object is skeletal, create class definitions for each bone
	TArray<FSLOwlNode> BonesClassDefintions;

	// Check if upper class is known
	if (!InSubClassOf.IsEmpty())
	{
		ClassDefinition.AddChildNode(FSLOwlSemanticMapStatics::CreateSubClassOfProperty(InSubClassOf));
	}

	// Add bounds if available
	if (AStaticMeshActor* ObjAsSMAct = Cast<AStaticMeshActor>(Object))
	{
		if (UStaticMeshComponent* SMComp = ObjAsSMAct->GetStaticMeshComponent())
		{
			// Duplicate static mesh component to ensure the bounding box is in its initial pose
			UStaticMeshComponent* SMCompDupl = DuplicateObject<UStaticMeshComponent>(SMComp, GetTransientPackage());
			SMCompDupl->SetWorldRotation(FQuat::Identity);
			SMCompDupl->UpdateBounds();
			// Bounding box size
			FVector BBSize;
#if SL_WITH_ROS_CONVERSIONS
			//BBSize = FConversions::CmToM(SMComp->Bounds.GetBox().GetSize());
			BBSize = FConversions::CmToM(SMCompDupl->Bounds.GetBox().GetSize());
#else
			//BBSize = SMComp->Bounds.GetBox().GetSize();	
			BBSize = SMCompDupl->Bounds.GetBox().GetSize();			
#endif // SL_WITH_ROS_CONVERSIONS

			SMCompDupl->DestroyComponent();

			if (!BBSize.IsZero())
			{
				ClassDefinition.AddChildNode(FSLOwlSemanticMapStatics::CreateDepthProperty(BBSize.X));
				ClassDefinition.AddChildNode(FSLOwlSemanticMapStatics::CreateWidthProperty(BBSize.Y));
				ClassDefinition.AddChildNode(FSLOwlSemanticMapStatics::CreateHeightProperty(BBSize.Z));
			}

			// Path to cad model
			if (UStaticMesh* SM = SMComp->GetStaticMesh())
			{
				ClassDefinition.AddChildNode(FSLOwlSemanticMapStatics::CreatePathToCadModelProperty(InClass));
			}

			// Mass property
			float Mass = SMComp->IsSimulatingPhysics() ? SMComp->GetMass() : SMComp->CalculateMass();
			ClassDefinition.AddChildNode(FSLOwlSemanticMapStatics::CreateMassProperty(Mass));
		}
	}
	else if (ASkeletalMeshActor* ObjAsSkelAct = Cast<ASkeletalMeshActor>(Object))
	{
		if (USkeletalMeshComponent* SkelComp = ObjAsSkelAct->GetSkeletalMeshComponent())
		{
			// Set a generic upper class if none is given
			if (InSubClassOf.IsEmpty())
			{
				if (ObjAsSkelAct->GetName().Contains("hand"))
				{
					ClassDefinition.AddChildNode(FSLOwlSemanticMapStatics::CreateSubClassOfProperty("Hand"));
				}
				else
				{
					ClassDefinition.AddChildNode(FSLOwlSemanticMapStatics::CreateSubClassOfProperty("Person"));
				}

			}

			FVector BBSize;
#if SL_WITH_ROS_CONVERSIONS
			BBSize = FConversions::CmToM(SkelComp->Bounds.GetBox().GetSize());
#else
			BBSize = SkelComp->Bounds.GetBox().GetSize();
#endif // SL_WITH_ROS_CONVERSIONS
			if (!BBSize.IsZero())
			{
				ClassDefinition.AddChildNode(FSLOwlSemanticMapStatics::CreateDepthProperty(BBSize.X));
				ClassDefinition.AddChildNode(FSLOwlSemanticMapStatics::CreateWidthProperty(BBSize.Y));
				ClassDefinition.AddChildNode(FSLOwlSemanticMapStatics::CreateHeightProperty(BBSize.Z));
			}

			// Add srdl capabilities
			TArray<FString> Capabilities = { "GraspingCapability", "move_arm", "move_base"};
			ClassDefinition.AddChildNode(FSLOwlSemanticMapStatics::CreateHasCapabilityProperties(Capabilities));

			TArray<FName> BoneNames;
			SkelComp->GetBoneNames(BoneNames);
			for (const auto& BoneName : BoneNames)
			{
				const FString BoneNameStr = BoneName.ToString();
				ClassDefinition.AddChildNode(FSLOwlSemanticMapStatics::CreateSkeletalBoneProperty(BoneNameStr));

				// Create separate bone class definition
				FSLOwlNode BoneClassDefinition = FSLOwlSemanticMapStatics::CreateClassDefinition(BoneNameStr);
				BoneClassDefinition.Comment = TEXT("Bone Class ") + BoneNameStr;

				// TODO read from actor skeletal component
				const FString BoneSubClassOf = "SkeletalBone";
				// Check if upper class is known
				if (!BoneSubClassOf.IsEmpty())
				{
					BoneClassDefinition.AddChildNode(FSLOwlSemanticMapStatics::CreateSubClassOfProperty(BoneSubClassOf));
				}

				BonesClassDefintions.Add(BoneClassDefinition);
			}
		}
	}
	else if (USkeletalMeshComponent* ObjAsSkelComp = Cast<USkeletalMeshComponent>(Object))
	{		
		FVector BBSize;
#if SL_WITH_ROS_CONVERSIONS
		BBSize = FConversions::CmToM(ObjAsSkelComp->Bounds.GetBox().GetSize());
#else
		BBSize = ObjAsSkelComp->Bounds.GetBox().GetSize();
#endif // SL_WITH_ROS_CONVERSIONS
		if (!BBSize.IsZero())
		{
			ClassDefinition.AddChildNode(FSLOwlSemanticMapStatics::CreateDepthProperty(BBSize.X));
			ClassDefinition.AddChildNode(FSLOwlSemanticMapStatics::CreateWidthProperty(BBSize.Y));
			ClassDefinition.AddChildNode(FSLOwlSemanticMapStatics::CreateHeightProperty(BBSize.Z));
		}

		TArray<FName> BoneNames;
		ObjAsSkelComp->GetBoneNames(BoneNames);
		for (const auto& BoneName : BoneNames)
		{
			ClassDefinition.AddChildNode(
				FSLOwlSemanticMapStatics::CreateSkeletalBoneProperty(BoneName.ToString()));
		}
	}
	else if(UPrimitiveComponent* ObjAsPrimComp = Cast<UPrimitiveComponent>(Object))
	{
		FVector BBSize;
#if SL_WITH_ROS_CONVERSIONS
		BBSize = FConversions::CmToM(ObjAsPrimComp->Bounds.GetBox().GetSize());
#else
		BBSize = ObjAsPrimComp->Bounds.GetBox().GetSize();
#endif // SL_WITH_ROS_CONVERSIONS
		if (!BBSize.IsZero())
		{
			ClassDefinition.AddChildNode(FSLOwlSemanticMapStatics::CreateDepthProperty(BBSize.X));
			ClassDefinition.AddChildNode(FSLOwlSemanticMapStatics::CreateWidthProperty(BBSize.Y));
			ClassDefinition.AddChildNode(FSLOwlSemanticMapStatics::CreateHeightProperty(BBSize.Z));
		}
	}
	InSemMap->ClassDefinitions.Add(ClassDefinition);

	if (BonesClassDefintions.Num())
	{
		InSemMap->ClassDefinitions.Append(BonesClassDefintions);
	}
}

// Add constraint individual
void FSLSemanticMapWriter::AddConstraintIndividual(TSharedPtr<FSLOwlSemanticMap> InSemMap,
	UPhysicsConstraintComponent* ConstraintComp,
	const FString& InId,
	const TArray<FName>& InTags)
{
	const FString MapPrefix = InSemMap->Prefix;
	const FString DocId = InSemMap->Id;

	AActor* ParentAct = ConstraintComp->ConstraintActor1;
	AActor* ChildAct = ConstraintComp->ConstraintActor2;

	if (ParentAct && ChildAct)
	{
		const FString ParentId = FSLTagIO::GetValue(ParentAct, "SemLog", "Id");
		const FString ChildId = FSLTagIO::GetValue(ChildAct, "SemLog", "Id");
		if (!ParentId.IsEmpty() && !ChildId.IsEmpty())
		{
			// Create the object individual
			FSLOwlNode ConstrIndividual = FSLOwlSemanticMapStatics::CreateConstraintIndividual(
				MapPrefix, InId, ParentId, ChildId);

			// Add describedInMap property
			ConstrIndividual.AddChildNode(FSLOwlSemanticMapStatics::CreateDescribedInMapProperty(
				MapPrefix, DocId));

			// Add tags data property
			ConstrIndividual.AddChildNode(FSLOwlSemanticMapStatics::CreateTagsDataProperty(
				InTags));

			// Generate unique ids 
			const FString PoseId = FSLUuid::NewGuidInBase64Url();
			const FString LinId = FSLUuid::NewGuidInBase64Url();
			const FString AngId = FSLUuid::NewGuidInBase64Url();

			// Add properties
			ConstrIndividual.AddChildNode(FSLOwlSemanticMapStatics::CreatePoseProperty(
				MapPrefix, PoseId));
			ConstrIndividual.AddChildNode(FSLOwlSemanticMapStatics::CreateLinearConstraintProperty(
				MapPrefix, LinId));
			ConstrIndividual.AddChildNode(FSLOwlSemanticMapStatics::CreateAngularConstraintProperty(
				MapPrefix, AngId));
			
			// Add individuals to the map
			InSemMap->AddIndividual(ConstrIndividual);

			// Create pose individual
#if SL_WITH_ROS_CONVERSIONS
			const FVector ROSLoc = FConversions::UToROS(ConstraintComp->GetComponentLocation());
			const FQuat ROSQuat = FConversions::UToROS(ConstraintComp->GetComponentQuat());
			InSemMap->AddIndividual(FSLOwlSemanticMapStatics::CreatePoseIndividual(
				MapPrefix, PoseId, ROSLoc, ROSQuat));
#else
			InSemMap->AddIndividual(FSLOwlSemanticMapStatics::CreatePoseIndividual(
				MapPrefix, PoseId, ConstraintComp->GetComponentLocation(), ConstraintComp->GetComponentQuat()));
#endif // SL_WITH_ROS_CONVERSIONS

			// Create linear constraint individual
			const uint8 LinXMotion = ConstraintComp->ConstraintInstance.GetLinearXMotion();
			const uint8 LinYMotion = ConstraintComp->ConstraintInstance.GetLinearYMotion();
			const uint8 LinZMotion = ConstraintComp->ConstraintInstance.GetLinearZMotion();
			float LinLimit;
#if SL_WITH_ROS_CONVERSIONS
			LinLimit = FConversions::CmToM(ConstraintComp->ConstraintInstance.GetLinearLimit());
#else
			LinLimit =ConstraintComp->ConstraintInstance.GetLinearLimit();
#endif // SL_WITH_ROS_CONVERSIONS
			const bool bLinSoftConstraint = ConstraintComp->ConstraintInstance.ProfileInstance.LinearLimit.bSoftConstraint;
			const float LinStiffness = ConstraintComp->ConstraintInstance.ProfileInstance.LinearLimit.Stiffness;
			const float LinDamping = ConstraintComp->ConstraintInstance.ProfileInstance.LinearLimit.Damping;
			
			InSemMap->AddIndividual(FSLOwlSemanticMapStatics::CreateLinearConstraintProperties(
				MapPrefix, LinId, LinXMotion, LinYMotion, LinZMotion, LinLimit, 
				bLinSoftConstraint, LinStiffness, LinDamping));

			// Create angular constraint individual
			const uint8 AngSwing1Motion = ConstraintComp->ConstraintInstance.GetAngularSwing1Motion();
			const uint8 AngSwing2Motion = ConstraintComp->ConstraintInstance.GetAngularSwing2Motion();
			const uint8 AngTwistMotion = ConstraintComp->ConstraintInstance.GetAngularTwistMotion();
			const float AngSwing1Limit = FMath::DegreesToRadians(ConstraintComp->ConstraintInstance.GetAngularSwing1Limit());
			const float AngSwing2Limit = FMath::DegreesToRadians(ConstraintComp->ConstraintInstance.GetAngularSwing2Limit());
			const float AngTwistLimit = FMath::DegreesToRadians(ConstraintComp->ConstraintInstance.GetAngularTwistLimit());
			const bool bAngSoftSwingConstraint = ConstraintComp->ConstraintInstance.ProfileInstance.ConeLimit.bSoftConstraint;
			const float AngSwingStiffness = ConstraintComp->ConstraintInstance.ProfileInstance.ConeLimit.Stiffness;
			const float AngSwingDamping = ConstraintComp->ConstraintInstance.ProfileInstance.ConeLimit.Damping;
			const bool bAngSoftTwistConstraint = ConstraintComp->ConstraintInstance.ProfileInstance.TwistLimit.bSoftConstraint;
			const float AngTwistStiffness = ConstraintComp->ConstraintInstance.ProfileInstance.TwistLimit.Stiffness;
			const float AngTwistDamping = ConstraintComp->ConstraintInstance.ProfileInstance.TwistLimit.Damping;

			InSemMap->AddIndividual(FSLOwlSemanticMapStatics::CreateAngularConstraintProperties(
				MapPrefix, AngId, AngSwing1Motion, AngSwing2Motion, AngTwistMotion,
				AngSwing1Limit, AngSwing2Limit, AngTwistLimit, bAngSoftSwingConstraint,
				AngSwingStiffness, AngSwingDamping, bAngSoftTwistConstraint,
				AngTwistStiffness, AngTwistDamping));
		}
	}
}

// Add individual physics properties
TArray<FSLOwlNode> FSLSemanticMapWriter::GetIndividualPhysicsProperties(UObject* Object)
{
	// Check object type
	if (AStaticMeshActor* ObjAsSMA = Cast<AStaticMeshActor>(Object))
	{
		if (UStaticMeshComponent* SMC = ObjAsSMA->GetStaticMeshComponent())
		{
			float Mass = SMC->IsSimulatingPhysics() ? SMC->GetMass() : SMC->CalculateMass();
			return FSLOwlSemanticMapStatics::CreatePhysicsProperties(
				Mass, SMC->GetGenerateOverlapEvents(), SMC->IsGravityEnabled());
		}
	}
	else if (UStaticMeshComponent* ObjAsSMC = Cast<UStaticMeshComponent>(Object))
	{
		float Mass = ObjAsSMC->IsSimulatingPhysics() ? ObjAsSMC->GetMass() : ObjAsSMC->CalculateMass();
		return FSLOwlSemanticMapStatics::CreatePhysicsProperties(
			Mass, ObjAsSMC->GetGenerateOverlapEvents(), ObjAsSMC->IsGravityEnabled());
	}
	return TArray<FSLOwlNode>();
}

// Get parent id (empty string if none)
FString FSLSemanticMapWriter::GetParentId(UObject* Object)
{
	// Check object type
	if (AActor* ObjAsAct = Cast<AActor>(Object))
	{
		if (AActor* ParentAct = ObjAsAct->GetParentActor())
		{
			const FString ParentId = FSLTagIO::GetValue(ParentAct, "SemLog", "Id");
			if (!ParentId.IsEmpty() && FSLTagIO::HasKey(ParentAct, "SemLog", "Class"))
			{
				return ParentId;
			}
		}

		if (AActor* ParentAttAct = ObjAsAct->GetAttachParentActor())
		{
			const FString Id = FSLTagIO::GetValue(ParentAttAct, "SemLog", "Id");
			if (!Id.IsEmpty() && FSLTagIO::HasKey(ParentAttAct, "SemLog", "Class"))
			{
				return Id;
			}
		}
		
		//if (UChildActorComponent* ParentComp = ObjAsAct->GetParentComponent())
		//{
		//	const FString Id = FSLTagIO::GetValue(ParentComp, "SemLog", "Id");
		//	if (!Id.IsEmpty() && FSLTagIO::HasKey(ParentComp, "SemLog", "Class"))
		//	{
		//		return Id;
		//	}
		//}

		//if (USceneComponent* ParentSceneComp = ObjAsAct->GetDefaultAttachComponent())
		//{
		//	const FString Id = FTags::GetValue(ParentSceneComp, "SemLog", "Id");
		//	if (!Id.IsEmpty() && FTags::HasKey(ParentSceneComp, "SemLog", "Class"))
		//	{
		//		return Id;
		//	}
		//}
	}
	//else if (USceneComponent* ObjAsSceneComp = Cast<USceneComponent>(Object))
	//{
	//	if (USceneComponent* ParentAttComp = ObjAsSceneComp->GetAttachParent())
	//	{
	//		const FString Id = FTags::GetValue(ParentAttComp, "SemLog", "Id");
	//		if (!Id.IsEmpty() && FTags::HasKey(ParentAttComp, "SemLog", "Class"))
	//		{
	//			return Id;
	//		}
	//	}

	//	if (AActor* ParentAttRootAct = ObjAsSceneComp->GetAttachmentRootActor())
	//	{
	//		const FString Id = FTags::GetValue(ParentAttRootAct, "SemLog", "Id");
	//		if (!Id.IsEmpty() && FTags::HasKey(ParentAttRootAct, "SemLog", "Class"))
	//		{
	//			return Id;
	//		}
	//	}
	//}
	//else if (UActorComponent* ObjAsActComp = Cast<UActorComponent>(Object))
	//{
	//	if (AActor* Owner = ObjAsActComp->GetOwner())
	//	{
	//		const FString Id = FTags::GetValue(Owner, "SemLog", "Id");
	//		if (!Id.IsEmpty() && FTags::HasKey(Owner, "SemLog", "Class"))
	//		{
	//			return Id;
	//		}
	//	}
	//}
	// No parent
	return FString();
}

// Get attachment ids parent/children (direct children, no grandchildren etc.)
void FSLSemanticMapWriter::GetChildIds(UObject* Object, TArray<FString>& OutChildIds)
{
	// Check object type
	if (AActor* ObjAsActor = Cast<AActor>(Object))
	{
		// Iterate child actors (only direct children, no grandchildren etc.)
		TArray<AActor*> AttachedActors;
		ObjAsActor->GetAttachedActors(AttachedActors);
		for (const auto& ChildAct : AttachedActors)
		{
			const FString ChildId = FSLTagIO::GetValue(ChildAct, "SemLog", "Id");
			if (!ChildId.IsEmpty() && FSLTagIO::HasKey(ChildAct, "SemLog", "Class"))
			{
				OutChildIds.AddUnique(ChildId);
			}
		}

		//// Iterate child components (only direct children, no grandchildren etc.)
		//TInlineComponentArray<UActorComponent*> ChildComponents;
		//ObjAsActor->GetComponents(ChildComponents, false);
		//for (const auto& ChildComp : ChildComponents)
		//{
		//	const FString ChildId = FTags::GetValue(ChildComp, "SemLog", "Id");
		//	if (!ChildId.IsEmpty() && FTags::HasKey(ChildComp, "SemLog", "Class"))
		//	{
		//		OutChildIds.AddUnique(ChildId);
		//	}
		//}
	}
	// Only scene components can have other components as children
	else if (USceneComponent* ObjAsSceneComp = Cast<USceneComponent>(Object))
	{
		//// Iterate child components (only direct children, no grandchildren etc.)
		//TArray<USceneComponent*> ChildComponents;
		//ObjAsSceneComp->GetChildrenComponents(false, ChildComponents);
		//for (const auto& ChildComp : ChildComponents)
		//{
		//	const FString ChildId = FSLTagIO::GetValue(ChildComp, "SemLog", "Id");
		//	if (!ChildId.IsEmpty() && FSLTagIO::HasKey(ChildComp, "SemLog", "Class"))
		//	{
		//		OutChildIds.AddUnique(ChildId);
		//	}
		//}
	}
}

// Get mobility property
FString FSLSemanticMapWriter::GetMobility(UObject* Object)
{
	// Check object type
	if (AActor* ObjAsAct = Cast<AActor>(Object))
	{
		if (ObjAsAct->IsRootComponentStatic())
		{
			return FString("static");
		}
		else if (ObjAsAct->IsRootComponentStationary())
		{
			return FString("stationary");
		}
		else if (ObjAsAct->IsRootComponentMovable())
		{
			// Check if kinematic (no physics) or dynamic (with physics)
			if (AStaticMeshActor* AsSMA = Cast<AStaticMeshActor>(ObjAsAct))
			{
				if (UStaticMeshComponent* SMC = AsSMA->GetStaticMeshComponent())
				{
					if (SMC->IsSimulatingPhysics())
					{
						return FString("dynamic");
					}
					else
					{
						return FString("kinematic");
					}
				}
				else
				{
					return FString("kinematic");
				}
			}
			else
			{
				return FString("kinematic");
			}
		}
	}
	else if (USceneComponent* ObjAsSceneComp = Cast<USceneComponent>(Object))
	{
		if(ObjAsSceneComp->Mobility == EComponentMobility::Static)
		{
			return FString("static");
		}
		else if (ObjAsSceneComp->Mobility == EComponentMobility::Stationary)
		{
			return FString("stationary");
		}
		else if (ObjAsSceneComp->Mobility == EComponentMobility::Movable)
		{
			if (ObjAsSceneComp->IsSimulatingPhysics())
			{
				return FString("dynamic");
			}
			else
			{
				return FString("kinematic");
			}
		}
	}
	// No mobility
	return FString();
}
