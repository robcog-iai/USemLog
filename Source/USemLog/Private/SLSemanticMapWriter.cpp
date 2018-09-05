// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLSemanticMapWriter.h"
#include "PhysicsEngine/PhysicsConstraintActor.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Animation/SkeletalMeshActor.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"

// UOwl
#include "OwlSemanticMapStatics.h"

// UUtils
#include "Tags.h"
#include "Ids.h"
#include "Conversions.h"

// Default constructor
FSLSemanticMapWriter::FSLSemanticMapWriter()
{
}

// Write semantic map to file
bool FSLSemanticMapWriter::WriteToFile(UWorld* World,
	EOwlSemanticMapTemplate TemplateType,
	const FString& InDirectory,
	const FString& InFilename)
{
	// Create the semantic map template
	TSharedPtr<FOwlSemanticMap> SemMap = CreateSemanticMapDocTemplate(TemplateType);

	// Add individuals to map
	AddAllIndividuals(SemMap, World);

	// Write map to file
	FString FullFilePath = FPaths::ProjectDir() +
		InDirectory + TEXT("/") + InFilename + TEXT(".owl");
	FPaths::RemoveDuplicateSlashes(FullFilePath);
	return FFileHelper::SaveStringToFile(SemMap->ToString(), *FullFilePath);
}

// Create semantic map template
TSharedPtr<FOwlSemanticMap> FSLSemanticMapWriter::CreateSemanticMapDocTemplate(EOwlSemanticMapTemplate TemplateType, const FString& InDocId)
{
	const FString DocId = InDocId.IsEmpty() ? FIds::NewGuidInBase64Url() : InDocId;

	if (TemplateType == EOwlSemanticMapTemplate::Default)
	{
		return FOwlSemanticMapStatics::CreateDefaultSemanticMap(DocId);
	}
	else if (TemplateType == EOwlSemanticMapTemplate::IAIKitchen)
	{
		return FOwlSemanticMapStatics::CreateIAIKitchenSemanticMap(DocId);
	}
	else if (TemplateType == EOwlSemanticMapTemplate::IAISupermarket)
	{
		return FOwlSemanticMapStatics::CreateIAISupermarketSemanticMap(DocId);
	}
	return MakeShareable(new FOwlSemanticMap());
}

// Add individuals to the semantic map
void FSLSemanticMapWriter::AddAllIndividuals(TSharedPtr<FOwlSemanticMap> InSemMap, UWorld* World)
{
	// Iterate objects with SemLog tag key
	for (const auto& ObjToTagsItr : FTags::GetObjectKeyValuePairsMap(World, "SemLog"))
	{
		// Get Id and Class of items
		const FString* IdPtr = ObjToTagsItr.Value.Find("Id");
		const FString* ClassPtr = ObjToTagsItr.Value.Find("Class");

		// Take into account only objects with an id
		if (IdPtr)
		{
			if (ClassPtr)
			{
				AddObjectIndividual(InSemMap, ObjToTagsItr.Key, *IdPtr, *ClassPtr);
			}
			// Check for other types, e.g. constraints can be actors or components
			else if (APhysicsConstraintActor* ConstrAct = Cast<APhysicsConstraintActor>(ObjToTagsItr.Key))
			{
				AddConstraintIndividual(InSemMap, ConstrAct->GetConstraintComp(), *IdPtr, ConstrAct->Tags);
			}
			else if (UPhysicsConstraintComponent* ConstrComp = Cast<UPhysicsConstraintComponent>(ObjToTagsItr.Key))
			{
				AddConstraintIndividual(InSemMap, ConstrComp, *IdPtr, ConstrComp->ComponentTags);
			}
		}

		// Add class individuals (Id not mandatory)
		if (ClassPtr)
		{
			const FString* SubClassOfPtr = ObjToTagsItr.Value.Find("SubClassOf");
			const FString SubClassOf = SubClassOfPtr ? *SubClassOfPtr : "";
			AddClassDefinition(InSemMap, ObjToTagsItr.Key, *ClassPtr, SubClassOf);
		}
	}
}

// Add object individual to the semantic map
void FSLSemanticMapWriter::AddObjectIndividual(TSharedPtr<FOwlSemanticMap> InSemMap,
	UObject* Object, const FString& InId, const FString& InClass)
{
	// Get map data
	const FString MapPrefix = InSemMap->Prefix;
	const FString DocId = InSemMap->Id;

	// Create the object individual
	FOwlNode ObjIndividual = FOwlSemanticMapStatics::CreateObjectIndividual(
		MapPrefix, InId, InClass);

	// Add describedInMap property
	ObjIndividual.AddChildNode(FOwlSemanticMapStatics::CreateDescribedInMapProperty(
		MapPrefix, DocId));
	
	// Add parent property
	const FString ParentId = GetParentId(Object);
	if (!ParentId.IsEmpty())
	{
		ObjIndividual.AddChildNode(FOwlSemanticMapStatics::CreateParentProperty(
			MapPrefix, ParentId));
	}

	// Add children properties
	for (const auto& ChildId : GetAllChildIds(Object))
	{
		ObjIndividual.AddChildNode(FOwlSemanticMapStatics::CreateChildProperty(
			MapPrefix, ChildId));
	}

	// Add pose individual to map
	if (AActor* ObjAsAct = Cast<AActor>(Object))
	{
		// Generate unique id for the properties
		const FString PoseId = FIds::NewGuidInBase64Url();

		/* Add properties */
		// Pose property
		ObjIndividual.AddChildNode(FOwlSemanticMapStatics::CreatePoseProperty(
			MapPrefix, PoseId));

		// If static mesh, add pathToCadModel property
		if (AStaticMeshActor* ActAsSMAct = Cast<AStaticMeshActor>(ObjAsAct))
		{
			ObjIndividual.AddChildNode(
				FOwlSemanticMapStatics::CreatePathToCadModelProperty(InClass));
		}

		// Add tags data property
		ObjIndividual.AddChildNode(FOwlSemanticMapStatics::CreateTagsDataProperty(
			ObjAsAct->Tags));

		// Add individuals 
		InSemMap->AddIndividual(ObjIndividual);

		// Create pose individual
		const FVector ROSLoc = FConversions::UToROS(ObjAsAct->GetActorLocation());
		const FQuat ROSQuat = FConversions::UToROS(ObjAsAct->GetActorQuat());
		InSemMap->AddIndividual(FOwlSemanticMapStatics::CreatePoseIndividual(
			MapPrefix, PoseId, ROSLoc, ROSQuat));
	}
	else if (USceneComponent* ObjAsSceneComp = Cast<USceneComponent>(Object))
	{
		// Generate unique id for the properties
		const FString PoseId = FIds::NewGuidInBase64Url();
		const FString TagsId = FIds::NewGuidInBase64Url();

		// Add properties
		ObjIndividual.AddChildNode(FOwlSemanticMapStatics::CreatePoseProperty(
			MapPrefix, PoseId));

		// If static mesh, add pathToCadModel property
		if (UStaticMeshComponent* CompAsSMComp = Cast<UStaticMeshComponent>(ObjAsSceneComp))
		{
			ObjIndividual.AddChildNode(FOwlSemanticMapStatics::CreatePathToCadModelProperty(
				InClass));
		}

		// Add tags data property
		ObjIndividual.AddChildNode(FOwlSemanticMapStatics::CreateTagsDataProperty(
			ObjAsSceneComp->ComponentTags));
		
		// Add individuals 
		InSemMap->AddIndividual(ObjIndividual);

		// Create pose individual
		const FVector ROSLoc = FConversions::UToROS(ObjAsSceneComp->GetComponentLocation());
		const FQuat ROSQuat = FConversions::UToROS(ObjAsSceneComp->GetComponentQuat());
		InSemMap->AddIndividual(FOwlSemanticMapStatics::CreatePoseIndividual(
			MapPrefix, PoseId, ROSLoc, ROSQuat));
	}
	else
	{
		// Obj has no pose info
		InSemMap->AddIndividual(ObjIndividual);
	}
}

// Add class individual
void FSLSemanticMapWriter::AddClassDefinition(TSharedPtr<FOwlSemanticMap> InSemMap,
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
	FOwlNode ClassDefinition = FOwlSemanticMapStatics::CreateClassDefinition(InClass);
	ClassDefinition.Comment = TEXT("Class ") + InClass;

	// Check if subclass is known
	if (!InSubClassOf.IsEmpty())
	{
		ClassDefinition.AddChildNode(FOwlSemanticMapStatics::CreateSubClassOfProperty(InSubClassOf));
	}

	// Add bounds if available
	if (AStaticMeshActor* ObjAsSMAct = Cast<AStaticMeshActor>(Object))
	{
		if (UStaticMeshComponent* SMComp = ObjAsSMAct->GetStaticMeshComponent())
		{
			const FVector BBSize = FConversions::CmToM(
				SMComp->Bounds.GetBox().GetSize());
			if (!BBSize.IsZero())
			{
				ClassDefinition.AddChildNode(FOwlSemanticMapStatics::CreateDepthProperty(BBSize.X));
				ClassDefinition.AddChildNode(FOwlSemanticMapStatics::CreateWidthProperty(BBSize.Y));
				ClassDefinition.AddChildNode(FOwlSemanticMapStatics::CreateHeightProperty(BBSize.Z));
			}
		}
	}
	else if (ASkeletalMeshActor* ObjAsSkelAct = Cast<ASkeletalMeshActor>(Object))
	{
		if (USkeletalMeshComponent* SkelComp = ObjAsSkelAct->GetSkeletalMeshComponent())
		{
			const FVector BBSize = FConversions::CmToM(
				SkelComp->Bounds.GetBox().GetSize());
			if (!BBSize.IsZero())
			{
				ClassDefinition.AddChildNode(FOwlSemanticMapStatics::CreateDepthProperty(BBSize.X));
				ClassDefinition.AddChildNode(FOwlSemanticMapStatics::CreateWidthProperty(BBSize.Y));
				ClassDefinition.AddChildNode(FOwlSemanticMapStatics::CreateHeightProperty(BBSize.Z));
			}

			TArray<FName> BoneNames;
			SkelComp->GetBoneNames(BoneNames);
			for (const auto& BoneName : BoneNames)
			{
				ClassDefinition.AddChildNode(
					FOwlSemanticMapStatics::CreateSkeletalBoneProperty(BoneName.ToString()));
			}
		}
	}
	else if (USkeletalMeshComponent* ObjAsSkelComp = Cast<USkeletalMeshComponent>(Object))
	{
		const FVector BBSize = FConversions::CmToM(
			ObjAsSkelComp->Bounds.GetBox().GetSize());
		if (!BBSize.IsZero())
		{
			ClassDefinition.AddChildNode(FOwlSemanticMapStatics::CreateDepthProperty(BBSize.X));
			ClassDefinition.AddChildNode(FOwlSemanticMapStatics::CreateWidthProperty(BBSize.Y));
			ClassDefinition.AddChildNode(FOwlSemanticMapStatics::CreateHeightProperty(BBSize.Z));
		}

		TArray<FName> BoneNames;
		ObjAsSkelComp->GetBoneNames(BoneNames);
		for (const auto& BoneName : BoneNames)
		{
			ClassDefinition.AddChildNode(
				FOwlSemanticMapStatics::CreateSkeletalBoneProperty(BoneName.ToString()));
		}
	}
	else if(UPrimitiveComponent* ObjAsPrimComp = Cast<UPrimitiveComponent>(Object))
	{
		const FVector BBSize = FConversions::CmToM(
			ObjAsPrimComp->Bounds.GetBox().GetSize());
		if (!BBSize.IsZero())
		{
			ClassDefinition.AddChildNode(FOwlSemanticMapStatics::CreateDepthProperty(BBSize.X));
			ClassDefinition.AddChildNode(FOwlSemanticMapStatics::CreateWidthProperty(BBSize.Y));
			ClassDefinition.AddChildNode(FOwlSemanticMapStatics::CreateHeightProperty(BBSize.Z));
		}
	}
	InSemMap->ClassDefinitions.Add(ClassDefinition);
}

// Add constraint individual
void FSLSemanticMapWriter::AddConstraintIndividual(TSharedPtr<FOwlSemanticMap> InSemMap,
	UPhysicsConstraintComponent* ConstraintComp,
	const FString& InId,
	const TArray<FName>& InTags)
{
	const FString MapPrefix = InSemMap->Prefix;
	AActor* ParentAct = ConstraintComp->ConstraintActor1;
	AActor* ChildAct = ConstraintComp->ConstraintActor2;
	if (ParentAct && ChildAct)
	{
		const FString ParentId = FTags::GetValue(ParentAct, "SemLog", "Id");
		const FString ChildId = FTags::GetValue(ChildAct, "SemLog", "Id");
		if (!ParentId.IsEmpty() && !ChildId.IsEmpty())
		{
			// Create the object individual
			FOwlNode ConstrIndividual = FOwlSemanticMapStatics::CreateConstraintIndividual(
				MapPrefix, InId, ParentId, ChildId);

			// Add tags data property
			ConstrIndividual.AddChildNode(FOwlSemanticMapStatics::CreateTagsDataProperty(
				InTags));

			// Generate unique ids 
			const FString PoseId = FIds::NewGuidInBase64Url();
			const FString LinId = FIds::NewGuidInBase64Url();
			const FString AngId = FIds::NewGuidInBase64Url();

			// Add properties
			ConstrIndividual.AddChildNode(FOwlSemanticMapStatics::CreatePoseProperty(
				MapPrefix, PoseId));
			ConstrIndividual.AddChildNode(FOwlSemanticMapStatics::CreateLinearConstraintProperty(
				MapPrefix, LinId));
			ConstrIndividual.AddChildNode(FOwlSemanticMapStatics::CreateAngularConstraintProperty(
				MapPrefix, AngId));
			
			// Add individuals to the map
			InSemMap->AddIndividual(ConstrIndividual);

			// Create pose individual
			const FVector ROSLoc = FConversions::UToROS(ConstraintComp->GetComponentLocation());
			const FQuat ROSQuat = FConversions::UToROS(ConstraintComp->GetComponentQuat());
			InSemMap->AddIndividual(FOwlSemanticMapStatics::CreatePoseIndividual(
				MapPrefix, PoseId, ROSLoc, ROSQuat));

			// Create linear constraint individual
			const uint8 LinXMotion = ConstraintComp->ConstraintInstance.GetLinearXMotion();
			const uint8 LinYMotion = ConstraintComp->ConstraintInstance.GetLinearYMotion();
			const uint8 LinZMotion = ConstraintComp->ConstraintInstance.GetLinearZMotion();
			const float LinLimit = FConversions::CmToM(ConstraintComp->ConstraintInstance.GetLinearLimit());
			const bool bLinSoftConstraint = false; // ConstraintComp->ConstraintInstance.ProfileInstance.
			const float LinStiffness = 0.f; // ConstraintComp->ConstraintInstance.ProfileInstance.
			const float LinDamping = 0.f; // ConstraintComp->ConstraintInstance.ProfileInstance.

			InSemMap->AddIndividual(FOwlSemanticMapStatics::CreateLinearConstraintProperties(
				MapPrefix, LinId, LinXMotion, LinYMotion, LinZMotion, LinLimit, 
				bLinSoftConstraint, LinStiffness, LinDamping));

			// Create angular constraint individual
			const uint8 AngSwing1Motion = ConstraintComp->ConstraintInstance.GetAngularSwing1Motion();
			const uint8 AngSwing2Motion = ConstraintComp->ConstraintInstance.GetAngularSwing2Motion();
			const uint8 AngTwistMotion = ConstraintComp->ConstraintInstance.GetAngularTwistMotion();
			const float AngSwing1Limit = FMath::DegreesToRadians(ConstraintComp->ConstraintInstance.GetAngularSwing1Limit());
			const float AngSwing2Limit = FMath::DegreesToRadians(ConstraintComp->ConstraintInstance.GetAngularSwing2Limit());
			const float AngTwistLimit = FMath::DegreesToRadians(ConstraintComp->ConstraintInstance.GetAngularTwistLimit());
			const bool bAngSoftSwingConstraint = false;
			const float AngSwingStiffness = 0.f;
			const float AngSwingDamping = 0.f;
			const bool bAngSoftTwistConstraint = false;
			const float AngTwistStiffness = 0.f;
			const float AngTwistDamping = 0.f;

			InSemMap->AddIndividual(FOwlSemanticMapStatics::CreateAngularConstraintProperties(
				MapPrefix, AngId, AngSwing1Motion, AngSwing2Motion, AngTwistMotion,
				AngSwing1Limit, AngSwing2Limit, AngTwistLimit, bAngSoftSwingConstraint,
				AngSwingStiffness, AngSwingDamping, bAngSoftTwistConstraint,
				AngTwistStiffness, AngTwistDamping));
		}
	}
}

// Get children ids (only direct children, no grandchildren etc.)
TArray<FString> FSLSemanticMapWriter::GetAllChildIds(UObject* Object)
{
	// Array of children ids
	TArray<FString> Ids;

	// Check object type
	if (AActor* ObjAsActor = Cast<AActor>(Object))
	{
		// Iterate child actors (only direct children, no grandchildren etc.)
		TArray<AActor*> ChildActors;
		ObjAsActor->GetAllChildActors(ChildActors, false);
		for (const auto& ChildAct : ChildActors)
		{
			const FString ChildId = FTags::GetValue(ChildAct, "SemLog", "Id");
			if (!ChildId.IsEmpty() && FTags::HasKey(ChildAct, "SemLog", "Class"))
			{
				Ids.Add(ChildId);
			}
		}

		// Iterate child components (only direct children, no grandchildren etc.)
		TInlineComponentArray<UActorComponent*> ChildComponents;
		ObjAsActor->GetComponents(ChildComponents, false);
		for (const auto& ChildComp : ChildComponents)
		{
			const FString ChildId = FTags::GetValue(ChildComp, "SemLog", "Id");
			if (!ChildId.IsEmpty() && FTags::HasKey(ChildComp, "SemLog", "Class"))
			{
				Ids.Add(ChildId);
			}
		}
	}
	// Only scene components can have other components as children
	else if (USceneComponent* ObjAsSceneComp = Cast<USceneComponent>(Object))
	{
		// Iterate child components (only direct children, no grandchildren etc.)
		TArray<USceneComponent*> ChildComponents;
		ObjAsSceneComp->GetChildrenComponents(false, ChildComponents);
		for (const auto& ChildComp : ChildComponents)
		{
			const FString ChildId = FTags::GetValue(ChildComp, "SemLog", "Id");
			if (!ChildId.IsEmpty() && FTags::HasKey(ChildComp, "SemLog", "Class"))
			{
				Ids.Add(ChildId);
			}
		}
	}
	return Ids;
}

// Get parent id (empty string if none)
FString FSLSemanticMapWriter::GetParentId(UObject* Object)
{
	// Check object type
	if (AActor* ObjAsAct = Cast<AActor>(Object))
	{
		AActor* ParentAct = ObjAsAct->GetParentActor();
		const FString ParentActId = FTags::GetValue(ParentAct, "SemLog", "Id");
		if (!ParentActId.IsEmpty() && FTags::HasKey(ParentAct, "SemLog", "Class"))
		{
			return ParentActId;
		}

		// If this actor was created by a child actor component
		UChildActorComponent* ParentComp = ObjAsAct->GetParentComponent();
		const FString ParentCompId = FTags::GetValue(ParentComp, "SemLog", "Id");
		if (!ParentCompId.IsEmpty() && FTags::HasKey(ParentComp, "SemLog", "Class"))
		{
			return ParentCompId;
		}
	}
	else if (USceneComponent* ObjAsSceneComp = Cast<USceneComponent>(Object))
	{
		USceneComponent* AttachComp = ObjAsSceneComp->GetAttachParent();
		const FString AttachCompId = FTags::GetValue(AttachComp, "SemLog", "Id");
		if (!AttachCompId.IsEmpty() && FTags::HasKey(AttachComp, "SemLog", "Class"))
		{
			return AttachCompId;
		}

		AActor* ParentAct = ObjAsSceneComp->GetAttachmentRootActor();
		const FString ParentActId = FTags::GetValue(ParentAct, "SemLog", "Id");
		if (!ParentActId.IsEmpty() && FTags::HasKey(ParentAct, "SemLog", "Class"))
		{
			return ParentActId;
		}
	}
	else if (UActorComponent* ObjAsActComp = Cast<USceneComponent>(Object))
	{
		AActor* Owner = ObjAsActComp->GetOwner();
		const FString OwnerId = FTags::GetValue(Owner, "SemLog", "Id");
		if (!OwnerId.IsEmpty() && FTags::HasKey(Owner, "SemLog", "Class"))
		{
			return OwnerId;
		}
	}
	// No parent
	return FString();
}
