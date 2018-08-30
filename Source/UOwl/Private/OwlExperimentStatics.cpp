// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "OwlExperimentStatics.h"

/* Semantic map template creation */
// Create default experiment document
TSharedPtr<FOwlExperiment> FOwlExperimentStatics::CreateDefaultExperiment(
	const FString& InDocId,
	const FString& InDocPrefix,
	const FString& InDocOntologyName)
{
	// Create map document
	TSharedPtr<FOwlExperiment> Experiment = MakeShareable(
		new FOwlExperiment(InDocPrefix, InDocOntologyName, InDocId));

	// Add definitions
	Experiment->AddEntityDefintion("owl", "http://www.w3.org/2002/07/owl#");
	Experiment->AddEntityDefintion("xsd", "http://www.w3.org/2001/XMLSchema#");
	Experiment->AddEntityDefintion("knowrob", "http://knowrob.org/kb/knowrob.owl#");
	Experiment->AddEntityDefintion("rdfs", "http://www.w3.org/2000/01/rdf-schema#");
	Experiment->AddEntityDefintion("rdf", "http://www.w3.org/1999/02/22-rdf-syntax-ns#");
	Experiment->AddEntityDefintion(InDocPrefix, "http://knowrob.org/kb/" + InDocOntologyName + ".owl#");

	// Add namespaces
	Experiment->AddNamespaceDeclaration("xmlns", "", "http://knowrob.org/kb/" + InDocOntologyName + ".owl#");
	Experiment->AddNamespaceDeclaration("xml", "base", "http://knowrob.org/kb/" + InDocOntologyName + ".owl#");
	Experiment->AddNamespaceDeclaration("xmlns", "owl", "http://www.w3.org/2002/07/owl#");
	Experiment->AddNamespaceDeclaration("xmlns", "xsd", "http://www.w3.org/2001/XMLSchema#");
	Experiment->AddNamespaceDeclaration("xmlns", "knowrob", "http://knowrob.org/kb/knowrob.owl#");
	Experiment->AddNamespaceDeclaration("xmlns", "rdfs", "http://www.w3.org/2000/01/rdf-schema#");
	Experiment->AddNamespaceDeclaration("xmlns", "rdf", "http://www.w3.org/1999/02/22-rdf-syntax-ns#");
	Experiment->AddNamespaceDeclaration("xmlns", InDocPrefix, "http://knowrob.org/kb/" + InDocOntologyName + ".owl#");

	// Set and add imports
	Experiment->AddOntologyNode();
	Experiment->AddOntologyImport("package://knowrob_common/owl/knowrob.owl");

	// Add property definitions
	Experiment->AddPropertyDefinition(FOwlCommentNode("Property Definitions"));
	Experiment->AddPropertyDefinition("knowrob", "taskContext");
	Experiment->AddPropertyDefinition("knowrob", "taskSuccess");

	Experiment->AddPropertyDefinition("knowrob", "startTime");
	Experiment->AddPropertyDefinition("knowrob", "endTime");
	Experiment->AddPropertyDefinition("knowrob", "experiment");
	Experiment->AddPropertyDefinition("knowrob", "inContact");

	// Add datatype definitions
	Experiment->AddDatatypeDefinition(FOwlCommentNode("Property Definitions"));
	Experiment->AddDatatypeDefinition("knowrob", "quaternion");
	Experiment->AddDatatypeDefinition("knowrob", "translation");
	
	// Add class definitions
	Experiment->AddClassDefinition(FOwlCommentNode("Class Definitions"));
	Experiment->AddClassDefinition("knowrob", "UnrealExperiment");
	Experiment->AddClassDefinition("knowrob", "GraspingSomething");
	Experiment->AddClassDefinition("knowrob", "TouchingSituation");
	Experiment->AddClassDefinition("knowrob", "Pose");

	// Add individuals comment
	// Experiment->AddExperimentIndividual(InDocPrefix, InDocId); // Adding at end
	Experiment->AddIndividual(FOwlCommentNode("Event Individuals"));

	return Experiment;
}

// Create UE experiment document
TSharedPtr<FOwlExperiment> FOwlExperimentStatics::CreateUEExperiment(
	const FString& InDocId,
	const FString& InDocPrefix,
	const FString& InDocOntologyName)
{
	TSharedPtr<FOwlExperiment> Experiment = FOwlExperimentStatics::CreateDefaultExperiment(
		InDocId, InDocPrefix, InDocOntologyName);

	Experiment->AddOntologyImport("package://knowrob_common/owl/knowrob_iai_kitchen_ue.owl");

	return Experiment;
}


/* Owl individuals creation */
// Create an object individual
FOwlNode FOwlExperimentStatics::CreateEventIndividual(
	const FString& InDocPrefix,
	const FString& InId,
	const FString& InClass)
{
	// Prefix name constants
	const FOwlPrefixName RdfAbout("rdf", "about");
	const FOwlPrefixName OwlNI("owl", "NamedIndividual");

	FOwlNode Individual(OwlNI, FOwlAttribute(RdfAbout, FOwlAttributeValue(
		InDocPrefix, InId)));
	Individual.AddChildNode(FOwlExperimentStatics::CreateClassProperty(InClass));
	return Individual;
}

// Create a timepoint individual
FOwlNode FOwlExperimentStatics::CreateTimepointIndividual(
	const FString& InDocPrefix,
	const float Timepoint)
{
	// Prefix name constants
	const FOwlPrefixName RdfAbout("rdf", "about");
	const FOwlPrefixName OwlNI("owl", "NamedIndividual");

	const FString Id = "timepoint_" + FString::SanitizeFloat(Timepoint);
	FOwlNode Individual(OwlNI, FOwlAttribute(RdfAbout, FOwlAttributeValue(
		InDocPrefix, Id)));
	Individual.AddChildNode(FOwlExperimentStatics::CreateClassProperty("Timepoint"));
	return Individual;
}

// Create an object individual
FOwlNode FOwlExperimentStatics::CreateObjectIndividual(
	const FString& InDocPrefix,
	const FString& InId,
	const FString& InClass)
{
	// Prefix name constants
	const FOwlPrefixName RdfAbout("rdf", "about");
	const FOwlPrefixName OwlNI("owl", "NamedIndividual");

	FOwlNode Individual(OwlNI, FOwlAttribute(RdfAbout, FOwlAttributeValue(
		InDocPrefix, InId)));
	Individual.AddChildNode(FOwlExperimentStatics::CreateClassProperty(InClass));
	return Individual;
}

//// Create a constraint individual
//FOwlNode FOwlEventsStatics::CreateConstraintIndividual(
//	const FString& InDocPrefix,
//	const FString& InId,
//	const FString& ParentId,
//	const FString& ChildId)
//{
//	const FOwlPrefixName RdfAbout("rdf", "about");
//	const FOwlPrefixName OwlNI("owl", "NamedIndividual");
//
//	FOwlNode ObjectIndividual(OwlNI, FOwlAttribute(RdfAbout, FOwlAttributeValue(InDocPrefix, InId)));
//	ObjectIndividual.Comment = TEXT("Constraint " + InId);
//	ObjectIndividual.AddChildNode(FOwlEventsStatics::CreateClassProperty("Constraint"));
//	ObjectIndividual.AddChildNode(FOwlEventsStatics::CreateParentProperty(InDocPrefix, ParentId));
//	ObjectIndividual.AddChildNode(FOwlEventsStatics::CreateChildProperty(InDocPrefix, ChildId));
//
//	return ObjectIndividual;
//}
//
//// Create constraint linear limits individual
//FOwlNode FOwlEventsStatics::CreateLinearConstraintProperties(
//	const FString& InDocPrefix, 
//	const FString& InId,
//	uint8 XMotion,
//	uint8 YMotion,
//	uint8 ZMotion,
//	float Limit,
//	bool bSoftConsraint,
//	float Stiffness,
//	float Damping)
//{
//	const FOwlPrefixName RdfAbout("rdf", "about");
//	const FOwlPrefixName OwlNI("owl", "NamedIndividual");
//
//	FOwlNode ConstrPropIndividual(OwlNI, FOwlAttribute(RdfAbout, FOwlAttributeValue(InDocPrefix, InId)));
//	ConstrPropIndividual.AddChildNode(FOwlEventsStatics::CreateClassProperty("LinearConstraint"));
//	ConstrPropIndividual.AddChildNode(FOwlEventsStatics::CreateIntValueProperty(
//		FOwlPrefixName("knowrob", "xMotion"), XMotion));
//	ConstrPropIndividual.AddChildNode(FOwlEventsStatics::CreateIntValueProperty(
//		FOwlPrefixName("knowrob", "yMotion"), YMotion));
//	ConstrPropIndividual.AddChildNode(FOwlEventsStatics::CreateIntValueProperty(
//		FOwlPrefixName("knowrob", "zMotion"), ZMotion));
//	ConstrPropIndividual.AddChildNode(FOwlEventsStatics::CreateFloatValueProperty(
//		FOwlPrefixName("knowrob", "limit"), Limit));
//	ConstrPropIndividual.AddChildNode(FOwlEventsStatics::CreateBoolValueProperty(
//		FOwlPrefixName("knowrob", "softConstraint"), bSoftConsraint));
//	ConstrPropIndividual.AddChildNode(FOwlEventsStatics::CreateFloatValueProperty(
//		FOwlPrefixName("knowrob", "stiffness"), Stiffness));
//	ConstrPropIndividual.AddChildNode(FOwlEventsStatics::CreateFloatValueProperty(
//		FOwlPrefixName("knowrob", "damping"), Damping));
//
//	return ConstrPropIndividual;
//}
//
//// Create constraint angular limits individual
//FOwlNode FOwlEventsStatics::CreateAngularConstraintProperties(
//	const FString& InDocPrefix,
//	const FString& InId,
//	uint8 Swing1Motion,
//	uint8 Swing2Motion,
//	uint8 TwistMotion,
//	float Swing1Limit,
//	float Swing2Limit,
//	float TwistLimit,
//	bool bSoftSwingConstraint,
//	float SwingStiffness,
//	float SwingDamping,
//	bool bSoftTwistConstraint,
//	float TwistStiffness,
//	float TwistDamping)
//{
//	const FOwlPrefixName RdfAbout("rdf", "about");
//	const FOwlPrefixName OwlNI("owl", "NamedIndividual");
//
//	FOwlNode ConstrPropIndividual(OwlNI, FOwlAttribute(RdfAbout, FOwlAttributeValue(InDocPrefix, InId)));
//	ConstrPropIndividual.AddChildNode(FOwlEventsStatics::CreateClassProperty("AngularConstraint"));
//	ConstrPropIndividual.AddChildNode(FOwlEventsStatics::CreateIntValueProperty(
//		FOwlPrefixName("knowrob","swing1Motion"), Swing1Motion));
//	ConstrPropIndividual.AddChildNode(FOwlEventsStatics::CreateIntValueProperty(
//		FOwlPrefixName("knowrob", "swing2Motion"), Swing2Motion));
//	ConstrPropIndividual.AddChildNode(FOwlEventsStatics::CreateIntValueProperty(
//		FOwlPrefixName("knowrob", "twistMotion"), TwistMotion));
//	ConstrPropIndividual.AddChildNode(FOwlEventsStatics::CreateFloatValueProperty(
//		FOwlPrefixName("knowrob", "swing1Limit"), Swing1Limit));
//	ConstrPropIndividual.AddChildNode(FOwlEventsStatics::CreateFloatValueProperty(
//		FOwlPrefixName("knowrob", "swing2Limit"), Swing2Limit));
//	ConstrPropIndividual.AddChildNode(FOwlEventsStatics::CreateFloatValueProperty(
//		FOwlPrefixName("knowrob", "twistLimit"), TwistLimit));
//	ConstrPropIndividual.AddChildNode(FOwlEventsStatics::CreateBoolValueProperty(
//		FOwlPrefixName("knowrob", "softSwingConstraint"), bSoftSwingConstraint));
//	ConstrPropIndividual.AddChildNode(FOwlEventsStatics::CreateFloatValueProperty(
//		FOwlPrefixName("knowrob", "swingStiffness"), SwingStiffness));
//	ConstrPropIndividual.AddChildNode(FOwlEventsStatics::CreateFloatValueProperty(
//		FOwlPrefixName("knowrob", "swingDamping"), SwingDamping));
//	ConstrPropIndividual.AddChildNode(FOwlEventsStatics::CreateBoolValueProperty(
//		FOwlPrefixName("knowrob", "softTwistConstraint"), bSoftTwistConstraint));
//	ConstrPropIndividual.AddChildNode(FOwlEventsStatics::CreateFloatValueProperty(
//		FOwlPrefixName("knowrob", "twistStiffness"), TwistStiffness));
//	ConstrPropIndividual.AddChildNode(FOwlEventsStatics::CreateFloatValueProperty(
//		FOwlPrefixName("knowrob", "twistDamping"), TwistDamping));
//
//	return ConstrPropIndividual;
//}
//
//// Create a class individual
//FOwlNode FOwlEventsStatics::CreateClassDefinition(const FString& InClass)
//{
//	// Prefix name constants
//	const FOwlPrefixName RdfAbout("rdf", "about");
//	const FOwlPrefixName OwlClass("owl", "Class");
//
//	return FOwlNode(OwlClass, FOwlAttribute(RdfAbout, FOwlAttributeValue("knowrob", InClass)));
//}

/* Owl properties creation */
// Create class property
FOwlNode FOwlExperimentStatics::CreateClassProperty(const FString& InClass)
{
	const FOwlPrefixName RdfResource("rdf", "resource");
	const FOwlPrefixName RdfType("rdf", "type");

	return FOwlNode(RdfType, FOwlAttribute(
		RdfResource, FOwlAttributeValue("knowrob", InClass)));
}

// Create startTime property
FOwlNode FOwlExperimentStatics::CreateStartTimeProperty(const FString& InDocPrefix, const float Timepoint)
{
	const FOwlPrefixName RdfResource("rdf", "resource");
	const FOwlPrefixName KbStartTime("knowrob", "startTime");

	const FString Id = "timepoint_" + FString::SanitizeFloat(Timepoint);
	return FOwlNode(KbStartTime, FOwlAttribute(
		RdfResource, FOwlAttributeValue(InDocPrefix, Id)));
}

// Create endTime property
FOwlNode FOwlExperimentStatics::CreateEndTimeProperty(const FString& InDocPrefix, const float Timepoint)
{
	const FOwlPrefixName RdfResource("rdf", "resource");
	const FOwlPrefixName KbEndTime("knowrob", "endTime");

	const FString Id = "timepoint_" + FString::SanitizeFloat(Timepoint);
	return FOwlNode(KbEndTime, FOwlAttribute(
		RdfResource, FOwlAttributeValue(InDocPrefix, Id)));
}

// Create inContact property
FOwlNode FOwlExperimentStatics::CreateInContactProperty(const FString& InDocPrefix, const FString& InObjId)
{
	const FOwlPrefixName RdfResource("rdf", "resource");
	const FOwlPrefixName KbInContact("knowrob", "inContact");

	return FOwlNode(KbInContact, FOwlAttribute(
		RdfResource, FOwlAttributeValue(InDocPrefix, InObjId)));
}

// Create isSupported property
FOwlNode FOwlExperimentStatics::CreateIsSupportedProperty(const FString& InDocPrefix, const FString& InObjId)
{
	const FOwlPrefixName RdfResource("rdf", "resource");
	const FOwlPrefixName KbIsSupported("knowrob", "isSupported");

	return FOwlNode(KbIsSupported, FOwlAttribute(
		RdfResource, FOwlAttributeValue(InDocPrefix, InObjId)));
}

// Create supports property
FOwlNode FOwlExperimentStatics::CreateIsSupportingProperty(const FString& InDocPrefix, const FString& InObjId)
{
	const FOwlPrefixName RdfResource("rdf", "resource");
	const FOwlPrefixName KbIsSupporting("knowrob", "isSupporting");

	return FOwlNode(KbIsSupporting, FOwlAttribute(
		RdfResource, FOwlAttributeValue(InDocPrefix, InObjId)));
}

//// Create describedInMap property
//FOwlNode FOwlEventsStatics::CreateDescribedInMapProperty(
//	const FString& InDocPrefix, const FString& InDocId)
//{
//	const FOwlPrefixName RdfResource("rdf", "resource");
//	const FOwlPrefixName KbDescribedInMap("knowrob", "describedInMap");
//
//	return FOwlNode(KbDescribedInMap, FOwlAttribute(RdfResource, FOwlAttributeValue(InDocPrefix, InDocId)));
//}
//
//// Create pathToCadModel property
//FOwlNode FOwlEventsStatics::CreatePathToCadModelProperty(const FString& InClass)
//{
//	const FOwlPrefixName RdfDatatype("rdf", "datatype");
//	const FOwlPrefixName KbPathToCadModel("knowrob", "pathToCadModel");
//	const FOwlAttributeValue AttrValString("xsd", "string");
//	const FString Path = "package://robcog/" + InClass + "/" + InClass + ".dae";
//
//	return FOwlNode(KbPathToCadModel, FOwlAttribute(RdfDatatype,
//		AttrValString), Path);
//}
//
//// Create subClassOf property
//FOwlNode FOwlEventsStatics::CreateSubClassOfProperty(const FString& InSubClass)
//{
//	const FOwlPrefixName RdfResource("rdf", "resource");
//	const FOwlPrefixName RdfsSubClassOf("rdfs", "subClassOf");
//
//	return FOwlNode(RdfsSubClassOf, FOwlAttribute(RdfResource, FOwlAttributeValue("knowrob", InSubClass)));
//}
//
//// Create skeletal bone property
//FOwlNode FOwlEventsStatics::CreateSkeletalBoneProperty(const FString& InBone)
//{
//	const FOwlPrefixName KbSkelBone("knowrob", "skeletalBone");
//	const FOwlPrefixName RdfDatatype("rdf", "datatype");
//	const FOwlAttributeValue AttrValString("xsd", "string");
//
//	return FOwlNode(KbSkelBone, FOwlAttribute(RdfDatatype, AttrValString), InBone);
//}
//
//// Create subclass - depth property
//FOwlNode FOwlEventsStatics::CreateDepthProperty(float Value) 
//{
//	const FOwlPrefixName RdfsSubClass("rdfs", "subClassOf");
//	const FOwlPrefixName OwlRestriction("owl", "Restriction");
//	const FOwlPrefixName OwlHasVal("owl", "hasValue");
//
//	FOwlNode SubClass(RdfsSubClass);
//	FOwlNode Restriction(OwlRestriction);
//	Restriction.AddChildNode(CreateOnProperty("depthOfObject"));
//	Restriction.AddChildNode(CreateFloatValueProperty(OwlHasVal, Value));
//	SubClass.AddChildNode(Restriction);
//	return SubClass;
//}
//
//// Create subclass - height property
//FOwlNode FOwlEventsStatics::CreateHeightProperty(float Value) 
//{
//	const FOwlPrefixName RdfsSubClass("rdfs", "subClassOf");
//	const FOwlPrefixName OwlRestriction("owl", "Restriction");
//	const FOwlPrefixName OwlHasVal("owl", "hasValue");
//
//	FOwlNode SubClass(RdfsSubClass);
//	FOwlNode Restriction(OwlRestriction);
//	Restriction.AddChildNode(CreateOnProperty("heightOfObject"));
//	Restriction.AddChildNode(CreateFloatValueProperty(OwlHasVal, Value));
//	SubClass.AddChildNode(Restriction);
//	return SubClass;
//}
//
//// Create subclass - width property
//FOwlNode FOwlEventsStatics::CreateWidthProperty(float Value) 
//{
//	const FOwlPrefixName RdfsSubClass("rdfs", "subClassOf");
//	const FOwlPrefixName OwlRestriction("owl", "Restriction");
//	const FOwlPrefixName OwlHasVal("owl", "hasValue");
//
//	FOwlNode SubClass(RdfsSubClass);
//	FOwlNode Restriction(OwlRestriction);
//	Restriction.AddChildNode(CreateOnProperty("widthOfObject"));
//	Restriction.AddChildNode(CreateFloatValueProperty(OwlHasVal, Value));
//	SubClass.AddChildNode(Restriction);
//	return SubClass;
//}
//
//// Create owl:onProperty meta property
//FOwlNode FOwlEventsStatics::CreateOnProperty(const FString& InProperty)
//{
//	const FOwlPrefixName OwlOnProp("owl", "onProperty");
//	const FOwlPrefixName RdfResource("rdf", "resource");
//
//	return FOwlNode(OwlOnProp, FOwlAttribute(RdfResource,FOwlAttributeValue("knowrob", InProperty)));
//}
//
//// Create a property with a bool value
//FOwlNode FOwlEventsStatics::CreateBoolValueProperty(const FOwlPrefixName& InPrefixName, bool bValue)
//{
//	const FOwlPrefixName RdfDatatype("rdf", "datatype");
//	const FOwlAttributeValue AttrValBool("xsd", "boolean");
//
//	return FOwlNode(InPrefixName, FOwlAttribute(RdfDatatype, AttrValBool), bValue ? "true" : "false");
//}
//
//// Create a property with an integer value
//FOwlNode FOwlEventsStatics::CreateIntValueProperty(const FOwlPrefixName& InPrefixName, int32 Value)
//{
//	const FOwlPrefixName RdfDatatype("rdf", "datatype");
//	const FOwlAttributeValue AttrValInt("xsd", "integer");
//
//	return FOwlNode(InPrefixName, FOwlAttribute(RdfDatatype, AttrValInt), FString::FromInt(Value));
//}
//
//// Create a property with a float value
//FOwlNode FOwlEventsStatics::CreateFloatValueProperty(const FOwlPrefixName& InPrefixName, float Value)
//{
//	const FOwlPrefixName RdfDatatype("rdf", "datatype");
//	const FOwlAttributeValue AttrValFloat("xsd", "float");
//
//	return FOwlNode(InPrefixName, FOwlAttribute(RdfDatatype, AttrValFloat), FString::SanitizeFloat(Value));
//}
//
//// Create a property with a string value
//FOwlNode FOwlEventsStatics::CreateStringValueProperty(const FOwlPrefixName& InPrefixName, const FString& InValue)
//{
//	const FOwlPrefixName RdfDatatype("rdf", "datatype");
//	const FOwlAttributeValue AttrValString("xsd", "string");
//
//	return FOwlNode(InPrefixName, FOwlAttribute(RdfDatatype, AttrValString), InValue);
//}
//
//// Create pose property
//FOwlNode FOwlEventsStatics::CreatePoseProperty(const FString& InDocPrefix, const FString& InId)
//{
//	const FOwlPrefixName KbPose("knowrob", "pose");
//	const FOwlPrefixName RdfResource("rdf", "resource");
//
//	return FOwlNode(KbPose, FOwlAttribute(RdfResource, FOwlAttributeValue(InDocPrefix, InId)));
//}
//
//// Create linear constraint property
//FOwlNode FOwlEventsStatics::CreateLinearConstraintProperty(const FString& InDocPrefix, const FString& InId)
//{
//	const FOwlPrefixName KbLinearConstr("knowrob", "linearConstraint");
//	const FOwlPrefixName RdfResource("rdf", "resource");
//
//	return FOwlNode(KbLinearConstr, FOwlAttribute(RdfResource, FOwlAttributeValue(InDocPrefix, InId)));
//}
//
//// Create angular constraint property
//FOwlNode FOwlEventsStatics::CreateAngularConstraintProperty(const FString& InDocPrefix, const FString& InId)
//{
//	const FOwlPrefixName KbAngularConstr("knowrob", "angularConstraint");
//	const FOwlPrefixName RdfResource("rdf", "resource");
//
//	return FOwlNode(KbAngularConstr, FOwlAttribute(RdfResource, FOwlAttributeValue(InDocPrefix, InId)));
//}
//
//// Create child property
//FOwlNode FOwlEventsStatics::CreateChildProperty(const FString& InDocPrefix, const FString& InId)
//{
//	const FOwlPrefixName KbChild("knowrob", "child");
//	const FOwlPrefixName RdfResource("rdf", "resource");
//
//	return FOwlNode(KbChild, FOwlAttribute(RdfResource, FOwlAttributeValue(InDocPrefix, InId)));
//}
//
//// Create parent property
//FOwlNode FOwlEventsStatics::CreateParentProperty(const FString& InDocPrefix, const FString& InId)
//{
//	const FOwlPrefixName KbParent("knowrob", "parent");
//	const FOwlPrefixName RdfResource("rdf", "resource");
//
//	return FOwlNode(KbParent, FOwlAttribute(RdfResource, FOwlAttributeValue(InDocPrefix, InId)));
//}
//
//// Create a location node
//FOwlNode FOwlEventsStatics::CreateLocationProperty(const FVector& InLoc)
//{
//	const FOwlPrefixName KbTransl("knowrob", "translation");
//	const FOwlPrefixName RdfDatatype("rdf", "datatype");
//	const FOwlAttributeValue AttrValString("xsd", "string");
//
//	const FString LocStr = FString::Printf(TEXT("%f %f %f"),
//		InLoc.X, InLoc.Y, InLoc.Z);
//	return FOwlNode(KbTransl, FOwlAttribute(RdfDatatype, AttrValString), LocStr);
//}
//
//// Create a quaternion node
//FOwlNode FOwlEventsStatics::CreateQuaternionProperty(const FQuat& InQuat)
//{
//	const FOwlPrefixName RdfDatatype("rdf", "datatype");
//	const FOwlPrefixName KbQuat("knowrob", "quaternion");
//	const FOwlAttributeValue AttrValString("xsd", "string");
//	
//	const FString QuatStr = FString::Printf(TEXT("%f %f %f %f"),
//		InQuat.W, InQuat.X, InQuat.Y, InQuat.Z);
//	return FOwlNode(KbQuat, FOwlAttribute(RdfDatatype, AttrValString), QuatStr);
//}