// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Owl/SLOwlSemanticMapStatics.h"

/* Semantic map template creation */
// Create Default semantic map
TSharedPtr<FSLOwlSemanticMap> FSLOwlSemanticMapStatics::CreateDefaultSemanticMap(
	const FString& InSemMapId,
	const FString& InDocPrefix,
	const FString& InDocOntologyName)
{
	// Create map document (with init)
	TSharedPtr<FSLOwlSemanticMap> SemMap = MakeShareable(new FSLOwlSemanticMap(InDocPrefix, InDocOntologyName, InSemMapId));

	// Add definitions
	SemMap->AddEntityDefintion("owl", "http://www.w3.org/2002/07/owl#");
	SemMap->AddEntityDefintion("xsd", "http://www.w3.org/2001/XMLSchema#");
	SemMap->AddEntityDefintion("knowrob", "http://knowrob.org/kb/knowrob.owl#");
	SemMap->AddEntityDefintion("rdfs", "http://www.w3.org/2000/01/rdf-schema#");
	SemMap->AddEntityDefintion("rdf", "http://www.w3.org/1999/02/22-rdf-syntax-ns#");
	SemMap->AddEntityDefintion("log", "http://knowrob.org/kb/ameva_log.owl#");
	//SemMap->AddEntityDefintion(InDocPrefix, "http://knowrob.org/kb/" + InDocOntologyName + ".owl#");

	// Add namespaces
	//SemMap->AddNamespaceDeclaration("xmlns", "", "http://knowrob.org/kb/" + InDocOntologyName + ".owl#");
	//SemMap->AddNamespaceDeclaration("xml", "base", "http://knowrob.org/kb/" + InDocOntologyName + ".owl#");
	SemMap->AddNamespaceDeclaration("xmlns", "", "http://knowrob.org/kb/ameva_log.owl#");
	SemMap->AddNamespaceDeclaration("xml", "base", "http://knowrob.org/kb/ameva_log.owl#");
	SemMap->AddNamespaceDeclaration("xmlns", "owl", "http://www.w3.org/2002/07/owl#");
	SemMap->AddNamespaceDeclaration("xmlns", "xsd", "http://www.w3.org/2001/XMLSchema#");
	SemMap->AddNamespaceDeclaration("xmlns", "knowrob", "http://knowrob.org/kb/knowrob.owl#");
	SemMap->AddNamespaceDeclaration("xmlns", "rdfs", "http://www.w3.org/2000/01/rdf-schema#");
	SemMap->AddNamespaceDeclaration("xmlns", "rdf", "http://www.w3.org/1999/02/22-rdf-syntax-ns#");
	SemMap->AddNamespaceDeclaration("xmlns", "log", "http://knowrob.org/kb/ameva_log.owl#");
	//SemMap->AddNamespaceDeclaration("xmlns", InDocPrefix, "http://knowrob.org/kb/" + InDocOntologyName + ".owl#");

	// Add imports
	SemMap->CreateOntologyNode();
	SemMap->AddOntologyImport("package://knowrob/owl/knowrob.owl");

	// Add property definitions
	SemMap->AddPropertyDefinition(FOwlCommentNode("Property Definitions"));
	SemMap->AddPropertyDefinition("knowrob", "describedInMap");
	SemMap->AddPropertyDefinition("knowrob", "pathToCadModel");
	SemMap->AddPropertyDefinition("knowrob", "overlapEvents");
	SemMap->AddPropertyDefinition("knowrob", "mobility");
	SemMap->AddPropertyDefinition("knowrob", "levelName");
	SemMap->AddPropertyDefinition("knowrob", "tagsData");
	SemMap->AddPropertyDefinition("knowrob", "gravity");
	SemMap->AddPropertyDefinition("knowrob", "mapDescription");

	// Add datatype definitions
	SemMap->AddDatatypeDefinition(FOwlCommentNode("Datatype Definitions"));
	SemMap->AddDatatypeDefinition("knowrob", "quaternion");
	SemMap->AddDatatypeDefinition("knowrob", "translation");
	
	// Add class definitions
	SemMap->AddClassDefinition(FOwlCommentNode("Class Definitions"));
	SemMap->AddClassDefinition("knowrob", "SemanticEnvironmentMap");
	SemMap->AddClassDefinition("knowrob", "Pose");

	return SemMap;
}

//// Create IAI Kitchen semantic map
//TSharedPtr<FSLOwlSemanticMap> FSLOwlSemanticMapStatics::CreateIAIKitchenSemanticMap(
//	const FString& InSemMapId,
//	const FString& InDocPrefix,
//	const FString& InDocOntologyName)
//{
//	TSharedPtr<FSLOwlSemanticMap> SemMap = FSLOwlSemanticMapStatics::CreateDefaultSemanticMap(
//		InSemMapId, InDocPrefix, InDocOntologyName);
//
//	//SemMap->AddOntologyImport("package://knowrob/owl/knowrob_iai_kitchen_ue.owl");
//
//	return SemMap;
//}
//
//// Create IAI Supermarket semantic map
//TSharedPtr<FSLOwlSemanticMap> FSLOwlSemanticMapStatics::CreateIAISupermarketSemanticMap(
//	const FString& InSemMapId,
//	const FString& InDocPrefix,
//	const FString& InDocOntologyName)
//{
//	TSharedPtr<FSLOwlSemanticMap> SemMap = FSLOwlSemanticMapStatics::CreateDefaultSemanticMap(
//		InSemMapId, InDocPrefix, InDocOntologyName);
//
//	//SemMap->AddOntologyImport("package://knowrob/owl/knowrob_iai_supermarket_ue.owl");
//
//	return SemMap;
//}

/* Owl individuals creation */
// Create an object individual
FSLOwlNode FSLOwlSemanticMapStatics::CreateObjectIndividual(
	const FString& InDocPrefix, 
	const FString& Id,
	const FString& Class)
{
	// Prefix name constants
	const FSLOwlPrefixName RdfAbout("rdf", "about");
	const FSLOwlPrefixName OwlNI("owl", "NamedIndividual");

	FSLOwlNode ObjectIndividual(OwlNI, FSLOwlAttribute(RdfAbout, FSLOwlAttributeValue(InDocPrefix, Id)));
	ObjectIndividual.Comment = TEXT("Individual " + Class/* + " " + Id*/);
	ObjectIndividual.AddChildNode(FSLOwlSemanticMapStatics::CreateClassProperty(Class));
	return ObjectIndividual;
}

// Create a pose individual
FSLOwlNode FSLOwlSemanticMapStatics::CreatePoseIndividual(
	const FString& InDocPrefix,
	const FString& InId,
	const FVector& InLoc,
	const FQuat& InQuat)
{
	// Prefix name constants
	const FSLOwlPrefixName RdfAbout("rdf", "about");
	const FSLOwlPrefixName RdfType("rdf", "type");
	const FSLOwlPrefixName RdfResource("rdf", "resource");
	const FSLOwlPrefixName OwlNI("owl", "NamedIndividual");

	// Attribute values constants
	const FSLOwlAttributeValue AttrValPose("knowrob", "Pose");

	// Pose individual
	FSLOwlNode PoseIndividual(OwlNI, FSLOwlAttribute(RdfAbout, FSLOwlAttributeValue(InDocPrefix, InId)));
	FSLOwlNode PoseProperty(RdfType, FSLOwlAttribute(RdfResource, AttrValPose));
	PoseIndividual.AddChildNode(PoseProperty);
	PoseIndividual.AddChildNode(FSLOwlSemanticMapStatics::CreateQuaternionProperty(InQuat));
	PoseIndividual.AddChildNode(FSLOwlSemanticMapStatics::CreateLocationProperty(InLoc));
	return PoseIndividual;
}

// Create a constraint individual
FSLOwlNode FSLOwlSemanticMapStatics::CreateConstraintIndividual(
	const FString& InDocPrefix,
	const FString& InId,
	const FString& ParentId,
	const FString& ChildId)
{
	const FSLOwlPrefixName RdfAbout("rdf", "about");
	const FSLOwlPrefixName OwlNI("owl", "NamedIndividual");

	FSLOwlNode ObjectIndividual(OwlNI, FSLOwlAttribute(RdfAbout, FSLOwlAttributeValue(InDocPrefix, InId)));
	ObjectIndividual.Comment = TEXT("Constraint"/* + InId*/);
	ObjectIndividual.AddChildNode(FSLOwlSemanticMapStatics::CreateClassProperty("Constraint"));
	ObjectIndividual.AddChildNode(FSLOwlSemanticMapStatics::CreateParentProperty(InDocPrefix, ParentId));
	ObjectIndividual.AddChildNode(FSLOwlSemanticMapStatics::CreateChildProperty(InDocPrefix, ChildId));

	return ObjectIndividual;
}

// Create constraint linear limits individual
FSLOwlNode FSLOwlSemanticMapStatics::CreateLinearConstraintProperties(
	const FString& InDocPrefix, 
	const FString& InId,
	uint8 XMotion,
	uint8 YMotion,
	uint8 ZMotion,
	float Limit,
	bool bSoftConsraint,
	float Stiffness,
	float Damping)
{
	const FSLOwlPrefixName RdfAbout("rdf", "about");
	const FSLOwlPrefixName OwlNI("owl", "NamedIndividual");

	FSLOwlNode ConstrPropIndividual(OwlNI, FSLOwlAttribute(RdfAbout, FSLOwlAttributeValue(InDocPrefix, InId)));
	ConstrPropIndividual.AddChildNode(FSLOwlSemanticMapStatics::CreateClassProperty("LinearConstraint"));
	ConstrPropIndividual.AddChildNode(FSLOwlSemanticMapStatics::CreateIntValueProperty(
		FSLOwlPrefixName("knowrob", "xMotion"), XMotion));
	ConstrPropIndividual.AddChildNode(FSLOwlSemanticMapStatics::CreateIntValueProperty(
		FSLOwlPrefixName("knowrob", "yMotion"), YMotion));
	ConstrPropIndividual.AddChildNode(FSLOwlSemanticMapStatics::CreateIntValueProperty(
		FSLOwlPrefixName("knowrob", "zMotion"), ZMotion));
	ConstrPropIndividual.AddChildNode(FSLOwlSemanticMapStatics::CreateFloatValueProperty(
		FSLOwlPrefixName("knowrob", "limit"), Limit));
	ConstrPropIndividual.AddChildNode(FSLOwlSemanticMapStatics::CreateBoolValueProperty(
		FSLOwlPrefixName("knowrob", "softConstraint"), bSoftConsraint));
	ConstrPropIndividual.AddChildNode(FSLOwlSemanticMapStatics::CreateFloatValueProperty(
		FSLOwlPrefixName("knowrob", "stiffness"), Stiffness));
	ConstrPropIndividual.AddChildNode(FSLOwlSemanticMapStatics::CreateFloatValueProperty(
		FSLOwlPrefixName("knowrob", "damping"), Damping));

	return ConstrPropIndividual;
}

// Create constraint angular limits individual
FSLOwlNode FSLOwlSemanticMapStatics::CreateAngularConstraintProperties(
	const FString& InDocPrefix,
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
	const FSLOwlPrefixName RdfAbout("rdf", "about");
	const FSLOwlPrefixName OwlNI("owl", "NamedIndividual");

	FSLOwlNode ConstrPropIndividual(OwlNI, FSLOwlAttribute(RdfAbout, FSLOwlAttributeValue(InDocPrefix, InId)));
	ConstrPropIndividual.AddChildNode(FSLOwlSemanticMapStatics::CreateClassProperty("AngularConstraint"));
	ConstrPropIndividual.AddChildNode(FSLOwlSemanticMapStatics::CreateIntValueProperty(
		FSLOwlPrefixName("knowrob","swing1Motion"), Swing1Motion));
	ConstrPropIndividual.AddChildNode(FSLOwlSemanticMapStatics::CreateIntValueProperty(
		FSLOwlPrefixName("knowrob", "swing2Motion"), Swing2Motion));
	ConstrPropIndividual.AddChildNode(FSLOwlSemanticMapStatics::CreateIntValueProperty(
		FSLOwlPrefixName("knowrob", "twistMotion"), TwistMotion));
	ConstrPropIndividual.AddChildNode(FSLOwlSemanticMapStatics::CreateFloatValueProperty(
		FSLOwlPrefixName("knowrob", "swing1Limit"), Swing1Limit));
	ConstrPropIndividual.AddChildNode(FSLOwlSemanticMapStatics::CreateFloatValueProperty(
		FSLOwlPrefixName("knowrob", "swing2Limit"), Swing2Limit));
	ConstrPropIndividual.AddChildNode(FSLOwlSemanticMapStatics::CreateFloatValueProperty(
		FSLOwlPrefixName("knowrob", "twistLimit"), TwistLimit));
	ConstrPropIndividual.AddChildNode(FSLOwlSemanticMapStatics::CreateBoolValueProperty(
		FSLOwlPrefixName("knowrob", "softSwingConstraint"), bSoftSwingConstraint));
	ConstrPropIndividual.AddChildNode(FSLOwlSemanticMapStatics::CreateFloatValueProperty(
		FSLOwlPrefixName("knowrob", "swingStiffness"), SwingStiffness));
	ConstrPropIndividual.AddChildNode(FSLOwlSemanticMapStatics::CreateFloatValueProperty(
		FSLOwlPrefixName("knowrob", "swingDamping"), SwingDamping));
	ConstrPropIndividual.AddChildNode(FSLOwlSemanticMapStatics::CreateBoolValueProperty(
		FSLOwlPrefixName("knowrob", "softTwistConstraint"), bSoftTwistConstraint));
	ConstrPropIndividual.AddChildNode(FSLOwlSemanticMapStatics::CreateFloatValueProperty(
		FSLOwlPrefixName("knowrob", "twistStiffness"), TwistStiffness));
	ConstrPropIndividual.AddChildNode(FSLOwlSemanticMapStatics::CreateFloatValueProperty(
		FSLOwlPrefixName("knowrob", "twistDamping"), TwistDamping));

	return ConstrPropIndividual;
}

// Create a class individual
FSLOwlNode FSLOwlSemanticMapStatics::CreateClassDefinition(const FString& InClass)
{
	// Prefix name constants
	const FSLOwlPrefixName RdfAbout("rdf", "about");
	const FSLOwlPrefixName OwlClass("owl", "Class");

	return FSLOwlNode(OwlClass, FSLOwlAttribute(RdfAbout, FSLOwlAttributeValue("knowrob", InClass)));
}

/* Owl properties creation */
// Create property
FSLOwlNode FSLOwlSemanticMapStatics::CreateGenericResourceProperty(const FSLOwlPrefixName& InPrefixName,
	const FSLOwlAttributeValue& InAttributeValue)
{
	const FSLOwlPrefixName RdfResource("rdf", "resource");
	return FSLOwlNode(InPrefixName, FSLOwlAttribute(RdfResource, InAttributeValue));
}

// Create class property
FSLOwlNode FSLOwlSemanticMapStatics::CreateClassProperty(const FString& InClass)
{
	const FSLOwlPrefixName RdfResource("rdf", "resource");
	const FSLOwlPrefixName RdfType("rdf", "type");

	return FSLOwlNode(RdfType, FSLOwlAttribute(RdfResource, FSLOwlAttributeValue("knowrob", InClass)));
}

// Create describedInMap property
FSLOwlNode FSLOwlSemanticMapStatics::CreateDescribedInMapProperty(
	const FString& InDocPrefix, const FString& InSemMapId)
{
	const FSLOwlPrefixName RdfResource("rdf", "resource");
	const FSLOwlPrefixName KbDescribedInMap("knowrob", "describedInMap");

	return FSLOwlNode(KbDescribedInMap, FSLOwlAttribute(RdfResource, FSLOwlAttributeValue(InDocPrefix, InSemMapId)));
}

// Create pathToCadModel property
FSLOwlNode FSLOwlSemanticMapStatics::CreatePathToCadModelProperty(const FString& InPath)
{
	const FSLOwlPrefixName RdfDatatype("rdf", "datatype");
	const FSLOwlPrefixName KbPathToCadModel("knowrob", "pathToCadModel");
	const FSLOwlAttributeValue AttrValString("xsd", "string");
	const FString Path = "package://robcog/" + InPath + ".dae";

	return FSLOwlNode(KbPathToCadModel, FSLOwlAttribute(RdfDatatype,
		AttrValString), Path);
}

// Create tagsData property
FSLOwlNode FSLOwlSemanticMapStatics::CreateTagsDataProperty(const TArray<FName>& InTags)
{
	const FSLOwlPrefixName RdfDatatype("rdf", "datatype");
	const FSLOwlPrefixName KbTagsData("knowrob", "tagsData");
	const FSLOwlAttributeValue AttrValString("xsd", "string");
	FString Data;
	for (const auto Tag : InTags)
	{
		Data += Tag.ToString() + " ";
	}
	Data.RemoveFromEnd(" ");

	return FSLOwlNode(KbTagsData, FSLOwlAttribute(RdfDatatype,
		AttrValString), Data);
}

// Create subClassOf property
FSLOwlNode FSLOwlSemanticMapStatics::CreateSubClassOfProperty(const FString& InSubClass)
{
	const FSLOwlPrefixName RdfResource("rdf", "resource");
	const FSLOwlPrefixName RdfsSubClassOf("rdfs", "subClassOf");

	return FSLOwlNode(RdfsSubClassOf, FSLOwlAttribute(RdfResource, FSLOwlAttributeValue("knowrob", InSubClass)));
}

// Create skeletal bone property
FSLOwlNode FSLOwlSemanticMapStatics::CreateSkeletalBoneProperty(const FString& InBone)
{
	const FSLOwlPrefixName KbSkelBone("knowrob", "skeletalBone");
	const FSLOwlPrefixName RdfDatatype("rdf", "datatype");
	const FSLOwlAttributeValue AttrValString("xsd", "string");

	return FSLOwlNode(KbSkelBone, FSLOwlAttribute(RdfDatatype, AttrValString), InBone);
}

// Create subclass - depth property
FSLOwlNode FSLOwlSemanticMapStatics::CreateDepthProperty(float Value) 
{
	const FSLOwlPrefixName RdfsSubClass("rdfs", "subClassOf");
	const FSLOwlPrefixName OwlRestriction("owl", "Restriction");
	const FSLOwlPrefixName OwlHasVal("owl", "hasValue");

	FSLOwlNode SubClass(RdfsSubClass);
	FSLOwlNode Restriction(OwlRestriction);
	Restriction.AddChildNode(CreateOnProperty("depthOfObject"));
	Restriction.AddChildNode(CreateFloatValueProperty(OwlHasVal, Value));
	SubClass.AddChildNode(Restriction);
	return SubClass;
}

// Create subclass - height property
FSLOwlNode FSLOwlSemanticMapStatics::CreateHeightProperty(float Value) 
{
	const FSLOwlPrefixName RdfsSubClass("rdfs", "subClassOf");
	const FSLOwlPrefixName OwlRestriction("owl", "Restriction");
	const FSLOwlPrefixName OwlHasVal("owl", "hasValue");

	FSLOwlNode SubClass(RdfsSubClass);
	FSLOwlNode Restriction(OwlRestriction);
	Restriction.AddChildNode(CreateOnProperty("heightOfObject"));
	Restriction.AddChildNode(CreateFloatValueProperty(OwlHasVal, Value));
	SubClass.AddChildNode(Restriction);
	return SubClass;
}

// Create subclass - width property
FSLOwlNode FSLOwlSemanticMapStatics::CreateWidthProperty(float Value) 
{
	const FSLOwlPrefixName RdfsSubClass("rdfs", "subClassOf");
	const FSLOwlPrefixName OwlRestriction("owl", "Restriction");
	const FSLOwlPrefixName OwlHasVal("owl", "hasValue");

	FSLOwlNode SubClass(RdfsSubClass);
	FSLOwlNode Restriction(OwlRestriction);
	Restriction.AddChildNode(CreateOnProperty("widthOfObject"));
	Restriction.AddChildNode(CreateFloatValueProperty(OwlHasVal, Value));
	SubClass.AddChildNode(Restriction);
	return SubClass;
}

// Create owl:onProperty meta property
FSLOwlNode FSLOwlSemanticMapStatics::CreateOnProperty(const FString& InProperty, const FString& Ns)
{
	const FSLOwlPrefixName OwlOnProp("owl", "onProperty");
	const FSLOwlPrefixName RdfResource("rdf", "resource");

	return FSLOwlNode(OwlOnProp, FSLOwlAttribute(RdfResource, FSLOwlAttributeValue(Ns, InProperty)));
}

// Create a property with a bool value
FSLOwlNode FSLOwlSemanticMapStatics::CreateBoolValueProperty(const FSLOwlPrefixName& InPrefixName, bool bValue)
{
	const FSLOwlPrefixName RdfDatatype("rdf", "datatype");
	const FSLOwlAttributeValue AttrValBool("xsd", "boolean");

	return FSLOwlNode(InPrefixName, FSLOwlAttribute(RdfDatatype, AttrValBool), bValue ? "true" : "false");
}

// Create a property with an integer value
FSLOwlNode FSLOwlSemanticMapStatics::CreateIntValueProperty(const FSLOwlPrefixName& InPrefixName, int32 Value)
{
	const FSLOwlPrefixName RdfDatatype("rdf", "datatype");
	const FSLOwlAttributeValue AttrValInt("xsd", "integer");

	return FSLOwlNode(InPrefixName, FSLOwlAttribute(RdfDatatype, AttrValInt), FString::FromInt(Value));
}

// Create a property with a float value
FSLOwlNode FSLOwlSemanticMapStatics::CreateFloatValueProperty(const FSLOwlPrefixName& InPrefixName, float Value)
{
	const FSLOwlPrefixName RdfDatatype("rdf", "datatype");
	const FSLOwlAttributeValue AttrValFloat("xsd", "float");

	return FSLOwlNode(InPrefixName, FSLOwlAttribute(RdfDatatype, AttrValFloat), FString::SanitizeFloat(Value));
}

// Create a property with a string value
FSLOwlNode FSLOwlSemanticMapStatics::CreateStringValueProperty(const FSLOwlPrefixName& InPrefixName, const FString& InValue)
{
	const FSLOwlPrefixName RdfDatatype("rdf", "datatype");
	const FSLOwlAttributeValue AttrValString("xsd", "string");

	return FSLOwlNode(InPrefixName, FSLOwlAttribute(RdfDatatype, AttrValString), InValue);
}

// Create pose property
FSLOwlNode FSLOwlSemanticMapStatics::CreatePoseProperty(const FString& InDocPrefix, const FString& InId)
{
	const FSLOwlPrefixName KbPose("knowrob", "pose");
	const FSLOwlPrefixName RdfResource("rdf", "resource");

	return FSLOwlNode(KbPose, FSLOwlAttribute(RdfResource, FSLOwlAttributeValue(InDocPrefix, InId)));
}

// Create linear constraint property
FSLOwlNode FSLOwlSemanticMapStatics::CreateLinearConstraintProperty(const FString& InDocPrefix, const FString& InId)
{
	const FSLOwlPrefixName KbLinearConstr("knowrob", "linearConstraint");
	const FSLOwlPrefixName RdfResource("rdf", "resource");

	return FSLOwlNode(KbLinearConstr, FSLOwlAttribute(RdfResource, FSLOwlAttributeValue(InDocPrefix, InId)));
}

// Create angular constraint property
FSLOwlNode FSLOwlSemanticMapStatics::CreateAngularConstraintProperty(const FString& InDocPrefix, const FString& InId)
{
	const FSLOwlPrefixName KbAngularConstr("knowrob", "angularConstraint");
	const FSLOwlPrefixName RdfResource("rdf", "resource");

	return FSLOwlNode(KbAngularConstr, FSLOwlAttribute(RdfResource, FSLOwlAttributeValue(InDocPrefix, InId)));
}

// Create parent property
FSLOwlNode FSLOwlSemanticMapStatics::CreateParentProperty(const FString& InDocPrefix, const FString& InId)
{
	const FSLOwlPrefixName KbChild("knowrob", "parent");
	const FSLOwlPrefixName RdfResource("rdf", "resource");

	return FSLOwlNode(KbChild, FSLOwlAttribute(RdfResource, FSLOwlAttributeValue(InDocPrefix, InId)));
}

// Create child property
FSLOwlNode FSLOwlSemanticMapStatics::CreateChildProperty(const FString& InDocPrefix, const FString& InId)
{
	const FSLOwlPrefixName KbChild("knowrob", "child");
	const FSLOwlPrefixName RdfResource("rdf", "resource");

	return FSLOwlNode(KbChild, FSLOwlAttribute(RdfResource, FSLOwlAttributeValue(InDocPrefix, InId)));
}

// Create mobility property
FSLOwlNode FSLOwlSemanticMapStatics::CreateMobilityProperty(const FString& Mobility)
{
	const FSLOwlPrefixName KbMobility("knowrob", "mobility");
	const FSLOwlPrefixName RdfDatatype("rdf", "datatype");
	const FSLOwlAttributeValue AttrValString("xsd", "string");
	return FSLOwlNode(KbMobility, FSLOwlAttribute(RdfDatatype, AttrValString), Mobility);
}

// Create mass property
FSLOwlNode FSLOwlSemanticMapStatics::CreateMassProperty(float Mass)
{
	const FSLOwlPrefixName KbMass("knowrob", "mass");
	const FSLOwlPrefixName RdfDatatype("rdf", "datatype");
	const FSLOwlAttributeValue AttrValFloat("xsd", "float");
	return FSLOwlNode(KbMass, FSLOwlAttribute(RdfDatatype, AttrValFloat), FString::SanitizeFloat(Mass));
}

// Create physics properties
TArray<FSLOwlNode> FSLOwlSemanticMapStatics::CreatePhysicsProperties(float Mass, bool bGenerateOverlapEvents, bool bGravity)
{
	const FSLOwlPrefixName KbMass("knowrob", "mass");
	const FSLOwlPrefixName KbOverlap("knowrob", "overlapEvents");
	const FSLOwlPrefixName KbGravity("knowrob", "gravity");
	const FSLOwlPrefixName RdfDatatype("rdf", "datatype");
	const FSLOwlAttributeValue AttrValFloat("xsd", "float");
	const FSLOwlAttributeValue AttrValBool("xsd", "boolean");
	
	TArray<FSLOwlNode> PhysicsProperties;
	//PhysicsProperties.Emplace(FSLOwlNode(KbMass, FSLOwlAttribute(RdfDatatype, AttrValFloat), FString::SanitizeFloat(Mass)));
	PhysicsProperties.Emplace(FSLOwlNode(KbOverlap, FSLOwlAttribute(RdfDatatype, AttrValBool), bGenerateOverlapEvents ? "true" : "false"));
	PhysicsProperties.Emplace(FSLOwlNode(KbGravity, FSLOwlAttribute(RdfDatatype, AttrValBool), bGravity ? "true" : "false"));

	return PhysicsProperties;
}

// Create mask color property
FSLOwlNode FSLOwlSemanticMapStatics::CreateMaskColorProperty(const FString& HexColor)
{
	const FSLOwlPrefixName KbMaskColor("knowrob", "maskColor");
	const FSLOwlPrefixName RdfDatatype("rdf", "datatype");
	const FSLOwlAttributeValue AttrValString("xsd", "string");
	return FSLOwlNode(KbMaskColor, FSLOwlAttribute(RdfDatatype, AttrValString), HexColor);
}

// Create a location node
FSLOwlNode FSLOwlSemanticMapStatics::CreateLocationProperty(const FVector& InLoc)
{
	const FSLOwlPrefixName KbTransl("knowrob", "translation");
	const FSLOwlPrefixName RdfDatatype("rdf", "datatype");
	const FSLOwlAttributeValue AttrValString("xsd", "string");

	const FString LocStr = FString::Printf(TEXT("%f %f %f"),
		InLoc.X, InLoc.Y, InLoc.Z);
	return FSLOwlNode(KbTransl, FSLOwlAttribute(RdfDatatype, AttrValString), LocStr);
}

// Create a quaternion node
FSLOwlNode FSLOwlSemanticMapStatics::CreateQuaternionProperty(const FQuat& InQuat)
{
	const FSLOwlPrefixName RdfDatatype("rdf", "datatype");
	const FSLOwlPrefixName KbQuat("knowrob", "quaternion");
	const FSLOwlAttributeValue AttrValString("xsd", "string");
	
	const FString QuatStr = FString::Printf(TEXT("%f %f %f %f"),
		InQuat.X, InQuat.Y, InQuat.Z, InQuat.W);
	return FSLOwlNode(KbQuat, FSLOwlAttribute(RdfDatatype, AttrValString), QuatStr);
}

/* SRDL */
// Create srdl has capability properties
FSLOwlNode FSLOwlSemanticMapStatics::CreateHasCapabilityProperties(const TArray<FString>& Capabilities)
{
	const FSLOwlPrefixName RdfsSubClassOf("rdfs", "subClassOf");
	const FSLOwlPrefixName OwlClass("owl", "Class");
	const FSLOwlPrefixName OwlIntersectionOf("owl", "intersectionOf");
	const FSLOwlPrefixName OwlRestriction("owl", "Restriction");

	const FSLOwlPrefixName OwlOnProp("owl", "onProperty");
	const FSLOwlPrefixName OwlSomeValuesFrom("owl", "someValuesFrom");

	const FSLOwlAttributeValue HasCapabilityAttr("srdl2-cap", "hasCapability");

	FSLOwlNode SubClassOf(RdfsSubClassOf);
	FSLOwlNode Class(OwlClass);
	FSLOwlNode IntersectionOf(OwlIntersectionOf);
	for (const auto& Capability : Capabilities)
	{
		FSLOwlNode Restriction(OwlRestriction);
		Restriction.AddChildNode(CreateGenericResourceProperty(OwlOnProp, HasCapabilityAttr));
		const FSLOwlAttributeValue CapabilityAttr("srdl2-cap", Capability);
		Restriction.AddChildNode(CreateGenericResourceProperty(OwlSomeValuesFrom, CapabilityAttr));
		IntersectionOf.AddChildNode(Restriction);
	}
	Class.AddChildNode(IntersectionOf);
	SubClassOf.AddChildNode(Class);
	return SubClassOf;
}


// Create srdl skeletal bone property
FSLOwlNode FSLOwlSemanticMapStatics::CreateSrdlSkeletalBoneProperty(const FString& InDocPrefix, const FString& InId)
{
	const FSLOwlPrefixName KbSkelBone("srdl2-comp", "subComponent");
	const FSLOwlPrefixName RdfResource("rdf", "resource");

	return FSLOwlNode(KbSkelBone, FSLOwlAttribute(RdfResource, FSLOwlAttributeValue(InDocPrefix, InId)));
}

// Create a bone individual
FSLOwlNode FSLOwlSemanticMapStatics::CreateBoneIndividual(
	const FString& InDocPrefix,
	const FString& InId,
	const FString& Class,
	const FString& BaseLinkId,
	const FString& EndLinkId,
	const FString& BoneName)
{
	// Prefix name constants
	const FSLOwlPrefixName OwlNI("owl", "NamedIndividual");
	const FSLOwlPrefixName RdfAbout("rdf", "about");
	const FSLOwlPrefixName RdfType("rdf", "type");
	const FSLOwlPrefixName RdfResource("rdf", "resource");
	const FSLOwlPrefixName SrdlBaseLink("srdl2-comp", "baseLinkOfComposition");
	const FSLOwlPrefixName SrdlEndLink("srdl2-comp", "endLinkOfComposition");
	const FSLOwlPrefixName KbSkelBone("knowrob", "skeletalBoneName");
	const FSLOwlPrefixName RdfDatatype("rdf", "datatype");
		
	// Attribute values constants
	const FSLOwlAttributeValue ComponentCompAttr("srdl2-comp", "ComponentComposition");
	const FSLOwlAttributeValue AttrValString("xsd", "string");

	// Bone individual
	FSLOwlNode BoneIndividual(OwlNI, FSLOwlAttribute(RdfAbout, FSLOwlAttributeValue(InDocPrefix, InId)));
	
	BoneIndividual.Comment = TEXT("Individual " + Class/* + " " + Id*/);
	BoneIndividual.AddChildNode(FSLOwlSemanticMapStatics::CreateClassProperty(Class));

	if (!BaseLinkId.IsEmpty())
	{
		FSLOwlNode BaseLinkProperty(SrdlBaseLink, FSLOwlAttribute(RdfResource, FSLOwlAttributeValue(InDocPrefix, BaseLinkId)));
		BoneIndividual.AddChildNode(BaseLinkProperty);
	}

	if (!EndLinkId.IsEmpty())
	{
		FSLOwlNode EndLinkProperty(SrdlEndLink, FSLOwlAttribute(RdfResource, FSLOwlAttributeValue(InDocPrefix, EndLinkId)));
		BoneIndividual.AddChildNode(EndLinkProperty);
	}

	FSLOwlNode BoneNameProperty(KbSkelBone, FSLOwlAttribute(RdfDatatype, AttrValString), BoneName);
	BoneIndividual.AddChildNode(BoneNameProperty);

	return BoneIndividual;
}
