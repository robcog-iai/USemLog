// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "OwlSemanticMapStatics.h"

/* Semantic map template creation */
// Create Default semantic map
TSharedPtr<FOwlSemanticMap> FOwlSemanticMapStatics::CreateDefaultSemanticMap(
	const FString& InMapId,
	const FString& InMapPrefix,
	const FString& InMapName)
{
	// Create map document
	TSharedPtr<FOwlSemanticMap> SemMap = MakeShareable(new FOwlSemanticMap(InMapPrefix, InMapName, InMapId));

	// Add definitions
	SemMap->AddEntityDefintion("owl", "http://www.w3.org/2002/07/owl#");
	SemMap->AddEntityDefintion("xsd", "http://www.w3.org/2001/XMLSchema#");
	SemMap->AddEntityDefintion("knowrob", "http://knowrob.org/kb/knowrob.owl#");
	SemMap->AddEntityDefintion("rdfs", "http://www.w3.org/2000/01/rdf-schema#");
	SemMap->AddEntityDefintion("rdf", "http://www.w3.org/1999/02/22-rdf-syntax-ns#");
	SemMap->AddEntityDefintion(InMapPrefix, "http://knowrob.org/kb/" + InMapName + ".owl#");

	// Add namespaces
	SemMap->AddNamespaceDeclaration("xmlns", "", "http://knowrob.org/kb/" + InMapName + ".owl#");
	SemMap->AddNamespaceDeclaration("xml", "base", "http://knowrob.org/kb/" + InMapName + ".owl#");
	SemMap->AddNamespaceDeclaration("xmlns", "owl", "http://www.w3.org/2002/07/owl#");
	SemMap->AddNamespaceDeclaration("xmlns", "xsd", "http://www.w3.org/2001/XMLSchema#");
	SemMap->AddNamespaceDeclaration("xmlns", "knowrob", "http://knowrob.org/kb/knowrob.owl#");
	SemMap->AddNamespaceDeclaration("xmlns", "rdfs", "http://www.w3.org/2000/01/rdf-schema#");
	SemMap->AddNamespaceDeclaration("xmlns", "rdf", "http://www.w3.org/1999/02/22-rdf-syntax-ns#");
	SemMap->AddNamespaceDeclaration("xmlns", InMapPrefix, "http://knowrob.org/kb/" + InMapName + ".owl#");

	// Set and add imports
	SemMap->SetOntologyNode(InMapName);
	SemMap->AddOntologyImport("package://knowrob_common/owl/knowrob.owl");

	// Add property definitions
	SemMap->AddPropertyDefinition(FOwlCommentNode("Property Definitions"));
	SemMap->AddPropertyDefinition("knowrob", "describedInMap");
	SemMap->AddPropertyDefinition("knowrob", "pathToCadModel");

	// Add datatype definitions
	SemMap->AddDatatypeDefinition(FOwlCommentNode("Property Definitions"));
	SemMap->AddDatatypeDefinition("knowrob", "quaternion");
	SemMap->AddDatatypeDefinition("knowrob", "translation");
	
	// Add class definitions
	SemMap->AddClassDefinition(FOwlCommentNode("Class Definitions"));
	SemMap->AddClassDefinition("knowrob", "SemanticEnvironmentMap");
	SemMap->AddClassDefinition("knowrob", "Pose");

	// Add individuals comment
	SemMap->AddIndividual(FOwlCommentNode("Individuals"));
	SemMap->AddSemanticMapIndividual(InMapPrefix, InMapId);

	return SemMap;
}

// Create IAI Kitchen semantic map
TSharedPtr<FOwlSemanticMap> FOwlSemanticMapStatics::CreateIAIKitchenSemanticMap(
	const FString& InMapId,
	const FString& InMapPrefix,
	const FString& InMapName)
{
	TSharedPtr<FOwlSemanticMap> SemMap = FOwlSemanticMapStatics::CreateDefaultSemanticMap(
		InMapId, InMapPrefix, InMapName);

	SemMap->AddOntologyImport("package://knowrob_common/owl/knowrob_iai_kitchen_ue.owl");

	return SemMap;
}

// Create IAI Supermarket semantic map
TSharedPtr<FOwlSemanticMap> FOwlSemanticMapStatics::CreateIAISupermarketSemanticMap(
	const FString& InMapId,
	const FString& InMapPrefix,
	const FString& InMapName)
{
	TSharedPtr<FOwlSemanticMap> SemMap = FOwlSemanticMapStatics::CreateDefaultSemanticMap(
		InMapId, InMapPrefix, InMapName);

	SemMap->AddOntologyImport("package://knowrob_common/owl/knowrob_iai_supermarket_ue.owl");

	return SemMap;
}

/* Owl individuals creation */
// Create an object individual
FOwlNode FOwlSemanticMapStatics::CreateObjectIndividual(
	const FString& InMapPrefix, 
	const FString& Id,
	const FString& Class)
{
	// Prefix name constants
	const FOwlPrefixName RdfAbout("rdf", "about");
	const FOwlPrefixName OwlNI("owl", "NamedIndividual");

	FOwlNode ObjectIndividual(OwlNI, FOwlAttribute(RdfAbout, FOwlAttributeValue(InMapPrefix, Id)));
	ObjectIndividual.Comment = TEXT("Individual " + Class + " " + Id);
	ObjectIndividual.AddChildNode(FOwlSemanticMapStatics::CreateClassProperty(Class));
	return ObjectIndividual;
}

// Create a pose individual
FOwlNode FOwlSemanticMapStatics::CreatePoseIndividual(
	const FString& InMapPrefix,
	const FString& InId,
	const FVector& InLoc,
	const FQuat& InQuat)
{
	// Prefix name constants
	const FOwlPrefixName RdfAbout("rdf", "about");
	const FOwlPrefixName RdfType("rdf", "type");
	const FOwlPrefixName RdfResource("rdf", "resource");
	const FOwlPrefixName OwlNI("owl", "NamedIndividual");

	// Attribute values constants
	const FOwlAttributeValue AttrValPose("knowrob", "Pose");
	const FOwlAttributeValue AttrValString("xsd", "string");

	// Pose individual
	FOwlNode PoseIndividual(OwlNI, FOwlAttribute(RdfAbout, FOwlAttributeValue(InMapPrefix, InId)));
	FOwlNode PoseProperty(RdfType, FOwlAttribute(RdfResource, AttrValPose));
	PoseIndividual.AddChildNode(PoseProperty);
	PoseIndividual.AddChildNode(FOwlSemanticMapStatics::CreateQuaternionProperty(InQuat));
	PoseIndividual.AddChildNode(FOwlSemanticMapStatics::CreateLocationProperty(InLoc));
	return PoseIndividual;
}

// Create a constraint individual
FOwlNode FOwlSemanticMapStatics::CreateConstraintIndividual(
	const FString& InMapPrefix,
	const FString& InId,
	const FString& ParentId,
	const FString& ChildId)
{
	const FOwlPrefixName RdfAbout("rdf", "about");
	const FOwlPrefixName OwlNI("owl", "NamedIndividual");

	FOwlNode ObjectIndividual(OwlNI, FOwlAttribute(RdfAbout, FOwlAttributeValue(InMapPrefix, InId)));
	ObjectIndividual.Comment = TEXT("Constraint " + InId);
	ObjectIndividual.AddChildNode(FOwlSemanticMapStatics::CreateClassProperty("Constraint"));
	ObjectIndividual.AddChildNode(FOwlSemanticMapStatics::CreateParentProperty(InMapPrefix, ParentId));
	ObjectIndividual.AddChildNode(FOwlSemanticMapStatics::CreateChildProperty(InMapPrefix, ChildId));

	return ObjectIndividual;
}

// Create constraint linear limits individual
FOwlNode FOwlSemanticMapStatics::CreateLinearConstraintProperties(
	const FString& InMapPrefix, 
	const FString& InId,
	uint8 XMotion,
	uint8 YMotion,
	uint8 ZMotion,
	float Limit,
	bool bSoftConsraint,
	float Stiffness,
	float Damping)
{
	const FOwlPrefixName RdfAbout("rdf", "about");
	const FOwlPrefixName OwlNI("owl", "NamedIndividual");

	FOwlNode ConstrPropIndividual(OwlNI, FOwlAttribute(RdfAbout, FOwlAttributeValue(InMapPrefix, InId)));
	ConstrPropIndividual.AddChildNode(FOwlSemanticMapStatics::CreateClassProperty("LinearConstraint"));
	ConstrPropIndividual.AddChildNode(FOwlSemanticMapStatics::CreateIntValueProperty(
		FOwlPrefixName("knowrob", "xMotion"), XMotion));
	ConstrPropIndividual.AddChildNode(FOwlSemanticMapStatics::CreateIntValueProperty(
		FOwlPrefixName("knowrob", "yMotion"), YMotion));
	ConstrPropIndividual.AddChildNode(FOwlSemanticMapStatics::CreateIntValueProperty(
		FOwlPrefixName("knowrob", "zMotion"), ZMotion));
	ConstrPropIndividual.AddChildNode(FOwlSemanticMapStatics::CreateFloatValueProperty(
		FOwlPrefixName("knowrob", "limit"), Limit));
	ConstrPropIndividual.AddChildNode(FOwlSemanticMapStatics::CreateBoolValueProperty(
		FOwlPrefixName("knowrob", "softConstraint"), bSoftConsraint));
	ConstrPropIndividual.AddChildNode(FOwlSemanticMapStatics::CreateFloatValueProperty(
		FOwlPrefixName("knowrob", "stiffness"), Stiffness));
	ConstrPropIndividual.AddChildNode(FOwlSemanticMapStatics::CreateFloatValueProperty(
		FOwlPrefixName("knowrob", "damping"), Damping));

	return ConstrPropIndividual;
}

// Create constraint angular limits individual
FOwlNode FOwlSemanticMapStatics::CreateAngularConstraintProperties(
	const FString& InMapPrefix,
	const FString& InId,
	uint8 Swing1Motion,
	uint8 Swing2Motion,
	uint8 TwistMotion,
	float Swing1Limit,
	float Swing2Limit,
	float TwistLimit,
	bool bSoftSwingConstraint,
	float SwingStiffness,
	float SwingDamping,
	bool bSoftTwistConstraint,
	float TwistStiffness,
	float TwistDamping)
{
	const FOwlPrefixName RdfAbout("rdf", "about");
	const FOwlPrefixName OwlNI("owl", "NamedIndividual");

	FOwlNode ConstrPropIndividual(OwlNI, FOwlAttribute(RdfAbout, FOwlAttributeValue(InMapPrefix, InId)));
	ConstrPropIndividual.AddChildNode(FOwlSemanticMapStatics::CreateClassProperty("AngularConstraint"));
	ConstrPropIndividual.AddChildNode(FOwlSemanticMapStatics::CreateIntValueProperty(
		FOwlPrefixName("knowrob","swing1Motion"), Swing1Motion));
	ConstrPropIndividual.AddChildNode(FOwlSemanticMapStatics::CreateIntValueProperty(
		FOwlPrefixName("knowrob", "swing2Motion"), Swing2Motion));
	ConstrPropIndividual.AddChildNode(FOwlSemanticMapStatics::CreateIntValueProperty(
		FOwlPrefixName("knowrob", "twistMotion"), TwistMotion));
	ConstrPropIndividual.AddChildNode(FOwlSemanticMapStatics::CreateFloatValueProperty(
		FOwlPrefixName("knowrob", "swing1Limit"), Swing1Limit));
	ConstrPropIndividual.AddChildNode(FOwlSemanticMapStatics::CreateFloatValueProperty(
		FOwlPrefixName("knowrob", "swing2Limit"), Swing2Limit));
	ConstrPropIndividual.AddChildNode(FOwlSemanticMapStatics::CreateFloatValueProperty(
		FOwlPrefixName("knowrob", "twistLimit"), TwistLimit));
	ConstrPropIndividual.AddChildNode(FOwlSemanticMapStatics::CreateBoolValueProperty(
		FOwlPrefixName("knowrob", "softSwingConstraint"), bSoftSwingConstraint));
	ConstrPropIndividual.AddChildNode(FOwlSemanticMapStatics::CreateFloatValueProperty(
		FOwlPrefixName("knowrob", "swingStiffness"), SwingStiffness));
	ConstrPropIndividual.AddChildNode(FOwlSemanticMapStatics::CreateFloatValueProperty(
		FOwlPrefixName("knowrob", "swingDamping"), SwingDamping));
	ConstrPropIndividual.AddChildNode(FOwlSemanticMapStatics::CreateBoolValueProperty(
		FOwlPrefixName("knowrob", "softTwistConstraint"), bSoftTwistConstraint));
	ConstrPropIndividual.AddChildNode(FOwlSemanticMapStatics::CreateFloatValueProperty(
		FOwlPrefixName("knowrob", "twistStiffness"), TwistStiffness));
	ConstrPropIndividual.AddChildNode(FOwlSemanticMapStatics::CreateFloatValueProperty(
		FOwlPrefixName("knowrob", "twistDamping"), TwistDamping));

	return ConstrPropIndividual;
}

// Create a class individual
FOwlNode FOwlSemanticMapStatics::CreateClassDefinition(const FString& InClass)
{
	// Prefix name constants
	const FOwlPrefixName RdfAbout("rdf", "about");
	const FOwlPrefixName OwlClass("owl", "Class");

	return FOwlNode(OwlClass, FOwlAttribute(RdfAbout, FOwlAttributeValue("knowrob", InClass)));
}

/* Owl properties creation */
// Create property
FOwlNode FOwlSemanticMapStatics::CreateGenericProperty(const FOwlPrefixName& InPrefixName,
	const FOwlAttributeValue& InAttributeValue)
{
	return FOwlNode();
}

// Create class property
FOwlNode FOwlSemanticMapStatics::CreateClassProperty(const FString& InClass)
{
	const FOwlPrefixName RdfResource("rdf", "resource");
	const FOwlPrefixName RdfType("rdf", "type");

	return FOwlNode(RdfType, FOwlAttribute(RdfResource, FOwlAttributeValue("knowrob", InClass)));
}

// Create describedInMap property
FOwlNode FOwlSemanticMapStatics::CreateDescribedInMapProperty(
	const FString& InMapPrefix, const FString& InMapId)
{
	const FOwlPrefixName RdfResource("rdf", "resource");
	const FOwlPrefixName KbDescribedInMap("knowrob", "describedInMap");

	return FOwlNode(KbDescribedInMap, FOwlAttribute(RdfResource, FOwlAttributeValue(InMapPrefix, InMapId)));
}

// Create pathToCadModel property
FOwlNode FOwlSemanticMapStatics::CreatePathToCadModelProperty(const FString& InClass)
{
	const FOwlPrefixName RdfDatatype("rdf", "datatype");
	const FOwlPrefixName KbPathToCadModel("knowrob", "pathToCadModel");
	const FOwlAttributeValue AttrValString("xsd", "string");
	const FString Path = "package://robcog/" + InClass + "/" + InClass + ".dae";

	return FOwlNode(KbPathToCadModel, FOwlAttribute(RdfDatatype,
		AttrValString), Path);
}

// Create tagsData property
FOwlNode FOwlSemanticMapStatics::CreateTagsDataProperty(const TArray<FName>& InTags)
{
	const FOwlPrefixName RdfDatatype("rdf", "datatype");
	const FOwlPrefixName KbTagsData("knowrob", "tagsData");
	const FOwlAttributeValue AttrValString("xsd", "string");
	FString Data;
	for (const auto Tag : InTags)
	{
		Data += Tag.ToString() + " ";
	}
	Data.RemoveFromEnd(" ");

	return FOwlNode(KbTagsData, FOwlAttribute(RdfDatatype,
		AttrValString), Data);
}

// Create subClassOf property
FOwlNode FOwlSemanticMapStatics::CreateSubClassOfProperty(const FString& InSubClass)
{
	const FOwlPrefixName RdfResource("rdf", "resource");
	const FOwlPrefixName RdfsSubClassOf("rdfs", "subClassOf");

	return FOwlNode(RdfsSubClassOf, FOwlAttribute(RdfResource, FOwlAttributeValue("knowrob", InSubClass)));
}

// Create skeletal bone property
FOwlNode FOwlSemanticMapStatics::CreateSkeletalBoneProperty(const FString& InBone)
{
	const FOwlPrefixName KbSkelBone("knowrob", "skeletalBone");
	const FOwlPrefixName RdfDatatype("rdf", "datatype");
	const FOwlAttributeValue AttrValString("xsd", "string");

	return FOwlNode(KbSkelBone, FOwlAttribute(RdfDatatype, AttrValString), InBone);
}

// Create subclass - depth property
FOwlNode FOwlSemanticMapStatics::CreateDepthProperty(float Value) 
{
	const FOwlPrefixName RdfsSubClass("rdfs", "subClassOf");
	const FOwlPrefixName OwlRestriction("owl", "Restriction");
	const FOwlPrefixName OwlHasVal("owl", "hasValue");

	FOwlNode SubClass(RdfsSubClass);
	FOwlNode Restriction(OwlRestriction);
	Restriction.AddChildNode(CreateOnProperty("depthOfObject"));
	Restriction.AddChildNode(CreateFloatValueProperty(OwlHasVal, Value));
	SubClass.AddChildNode(Restriction);
	return SubClass;
}

// Create subclass - height property
FOwlNode FOwlSemanticMapStatics::CreateHeightProperty(float Value) 
{
	const FOwlPrefixName RdfsSubClass("rdfs", "subClassOf");
	const FOwlPrefixName OwlRestriction("owl", "Restriction");
	const FOwlPrefixName OwlHasVal("owl", "hasValue");

	FOwlNode SubClass(RdfsSubClass);
	FOwlNode Restriction(OwlRestriction);
	Restriction.AddChildNode(CreateOnProperty("heightOfObject"));
	Restriction.AddChildNode(CreateFloatValueProperty(OwlHasVal, Value));
	SubClass.AddChildNode(Restriction);
	return SubClass;
}

// Create subclass - width property
FOwlNode FOwlSemanticMapStatics::CreateWidthProperty(float Value) 
{
	const FOwlPrefixName RdfsSubClass("rdfs", "subClassOf");
	const FOwlPrefixName OwlRestriction("owl", "Restriction");
	const FOwlPrefixName OwlHasVal("owl", "hasValue");

	FOwlNode SubClass(RdfsSubClass);
	FOwlNode Restriction(OwlRestriction);
	Restriction.AddChildNode(CreateOnProperty("widthOfObject"));
	Restriction.AddChildNode(CreateFloatValueProperty(OwlHasVal, Value));
	SubClass.AddChildNode(Restriction);
	return SubClass;
}

// Create owl:onProperty meta property
FOwlNode FOwlSemanticMapStatics::CreateOnProperty(const FString& InProperty)
{
	const FOwlPrefixName OwlOnProp("owl", "onProperty");
	const FOwlPrefixName RdfResource("rdf", "resource");

	return FOwlNode(OwlOnProp, FOwlAttribute(RdfResource,FOwlAttributeValue("knowrob", InProperty)));
}

// Create a property with a bool value
FOwlNode FOwlSemanticMapStatics::CreateBoolValueProperty(const FOwlPrefixName& InPrefixName, bool bValue)
{
	const FOwlPrefixName RdfDatatype("rdf", "datatype");
	const FOwlAttributeValue AttrValBool("xsd", "boolean");

	return FOwlNode(InPrefixName, FOwlAttribute(RdfDatatype, AttrValBool), bValue ? "true" : "false");
}

// Create a property with an integer value
FOwlNode FOwlSemanticMapStatics::CreateIntValueProperty(const FOwlPrefixName& InPrefixName, int32 Value)
{
	const FOwlPrefixName RdfDatatype("rdf", "datatype");
	const FOwlAttributeValue AttrValInt("xsd", "integer");

	return FOwlNode(InPrefixName, FOwlAttribute(RdfDatatype, AttrValInt), FString::FromInt(Value));
}

// Create a property with a float value
FOwlNode FOwlSemanticMapStatics::CreateFloatValueProperty(const FOwlPrefixName& InPrefixName, float Value)
{
	const FOwlPrefixName RdfDatatype("rdf", "datatype");
	const FOwlAttributeValue AttrValFloat("xsd", "float");

	return FOwlNode(InPrefixName, FOwlAttribute(RdfDatatype, AttrValFloat), FString::SanitizeFloat(Value));
}

// Create a property with a string value
FOwlNode FOwlSemanticMapStatics::CreateStringValueProperty(const FOwlPrefixName& InPrefixName, const FString& InValue)
{
	const FOwlPrefixName RdfDatatype("rdf", "datatype");
	const FOwlAttributeValue AttrValString("xsd", "string");

	return FOwlNode(InPrefixName, FOwlAttribute(RdfDatatype, AttrValString), InValue);
}

// Create pose property
FOwlNode FOwlSemanticMapStatics::CreatePoseProperty(const FString& InMapPrefix, const FString& InId)
{
	const FOwlPrefixName KbPose("knowrob", "pose");
	const FOwlPrefixName RdfResource("rdf", "resource");

	return FOwlNode(KbPose, FOwlAttribute(RdfResource, FOwlAttributeValue(InMapPrefix, InId)));
}

// Create linear constraint property
FOwlNode FOwlSemanticMapStatics::CreateLinearConstraintProperty(const FString& InMapPrefix, const FString& InId)
{
	const FOwlPrefixName KbLinearConstr("knowrob", "linearConstraint");
	const FOwlPrefixName RdfResource("rdf", "resource");

	return FOwlNode(KbLinearConstr, FOwlAttribute(RdfResource, FOwlAttributeValue(InMapPrefix, InId)));
}

// Create angular constraint property
FOwlNode FOwlSemanticMapStatics::CreateAngularConstraintProperty(const FString& InMapPrefix, const FString& InId)
{
	const FOwlPrefixName KbAngularConstr("knowrob", "angularConstraint");
	const FOwlPrefixName RdfResource("rdf", "resource");

	return FOwlNode(KbAngularConstr, FOwlAttribute(RdfResource, FOwlAttributeValue(InMapPrefix, InId)));
}

// Create child property
FOwlNode FOwlSemanticMapStatics::CreateChildProperty(const FString& InMapPrefix, const FString& InId)
{
	const FOwlPrefixName KbChild("knowrob", "child");
	const FOwlPrefixName RdfResource("rdf", "resource");

	return FOwlNode(KbChild, FOwlAttribute(RdfResource, FOwlAttributeValue(InMapPrefix, InId)));
}

// Create parent property
FOwlNode FOwlSemanticMapStatics::CreateParentProperty(const FString& InMapPrefix, const FString& InId)
{
	const FOwlPrefixName KbParent("knowrob", "parent");
	const FOwlPrefixName RdfResource("rdf", "resource");

	return FOwlNode(KbParent, FOwlAttribute(RdfResource, FOwlAttributeValue(InMapPrefix, InId)));
}

// Create a location node
FOwlNode FOwlSemanticMapStatics::CreateLocationProperty(const FVector& InLoc)
{
	const FOwlPrefixName KbTransl("knowrob", "translation");
	const FOwlPrefixName RdfDatatype("rdf", "datatype");
	const FOwlAttributeValue AttrValString("xsd", "string");

	const FString LocStr = FString::Printf(TEXT("%f %f %f"),
		InLoc.X, InLoc.Y, InLoc.Z);
	return FOwlNode(KbTransl, FOwlAttribute(RdfDatatype, AttrValString), LocStr);
}

// Create a quaternion node
FOwlNode FOwlSemanticMapStatics::CreateQuaternionProperty(const FQuat& InQuat)
{
	const FOwlPrefixName RdfDatatype("rdf", "datatype");
	const FOwlPrefixName KbQuat("knowrob", "quaternion");
	const FOwlAttributeValue AttrValString("xsd", "string");
	
	const FString QuatStr = FString::Printf(TEXT("%f %f %f %f"),
		InQuat.W, InQuat.X, InQuat.Y, InQuat.Z);
	return FOwlNode(KbQuat, FOwlAttribute(RdfDatatype, AttrValString), QuatStr);
}