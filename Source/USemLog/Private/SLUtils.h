// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include <string>
#include <algorithm>
#include "rapidxml/rapidxml_print.hpp"
#include "USemLogPrivatePCH.h"

/**
 * Utils for OWL generation
 */
struct FSLUtils
{
public:
	// Generate random FString
	static FORCEINLINE FString GenerateRandomFString(const int32 Length)
	{
		auto RandChar = []() -> char
		{
			const char CharSet[] =
				"0123456789"
				"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
				"abcdefghijklmnopqrstuvwxyz";
			const size_t MaxIndex = (sizeof(CharSet) - 1);
			return CharSet[rand() % MaxIndex];
		};
		std::string RandString(Length, 0);
		std::generate_n(RandString.begin(), Length, RandChar);
		// Return as Fstring
		return FString(RandString.c_str());
	}

	// Convert FString to const char*
	static FORCEINLINE const char* FStringToChar(const FString FStr)
	{
		const std::string str = TCHAR_TO_UTF8(*FStr);
		const size_t str_length = str.length() + 1;
		char *cstr = new char[str_length];
		#if PLATFORM_LINUX
		  strcpy(cstr, str.c_str());
		#elif PLATFORM_WINDOWS
		  strcpy_s(cstr, str_length + 1, str.c_str());
		#endif
		return cstr;
	}

	// Convert FString to const char*
	static FORCEINLINE FString CharToFString(const char* CharArr)
	{
		const std::string str(CharArr);
		return FString(str.c_str());
		//return FString(ANSI_TO_TCHAR(CharArr));
		//return FString((UTF8_TO_TCHAR(CharArr));
	}

	// Get the enum value to string
	template<typename TEnum>
	static FORCEINLINE FString GetEnumValueToString(const FString& Name, TEnum Value)
	{
		const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, *Name, true);
		if (!EnumPtr)
		{
			return FString("Invalid");
		}
		return EnumPtr->GetEnumName((int32)Value);
	};
	
	// Get the value of pair, from an array of pairs
	static FORCEINLINE FString GetPairArrayValue(
		const TArray<TPair<FString, FString>>& PairArray,
		const FString PairKey)
	{
		const TPair<FString, FString>* KeyValPair = PairArray.FindByPredicate(
			[&PairKey](const TPair<FString, FString>& SemLogKeyVal)
			{return SemLogKeyVal.Key.Equals(PairKey);}
		);
		// Avoid returning nullptr (return empty string if the case)
		if (KeyValPair)
		{
			return KeyValPair->Value;
		}
		else
		{
			FString Def = PairKey + "_DEFAULT";
			UE_LOG(SemLog, Error, TEXT(" !! %s is not available in the SemLogInfo. Using default name: %s"), *PairKey, *Def);
			return Def;
		}
	}

	// Get all pair array values
	static FORCEINLINE TMap<AActor*, FString> GetMapOfSemLogInfoToActorToClass(
		const TMap<AActor*, TArray<TPair<FString, FString>>>& ActorToSemLogInfo,
		const FString ClassName)
	{
		TMap<AActor*, FString> ActToClass;
		// Iterate through the semlog info
		for (const auto ActToSemLogInfoItr : ActorToSemLogInfo)
		{
			// Add actor pointer and found class to the map
			ActToClass.Add(ActToSemLogInfoItr.Key, FSLUtils::GetPairArrayValue(ActToSemLogInfoItr.Value, ClassName));
		}
		return ActToClass;
	}

	/* OWL Utils */

	// Node attribute as struct
	struct RNodeAttribute
	{
		RNodeAttribute(const char* NodeName, const char* NodeValue)
			: Name(NodeName), Value(NodeValue)
		{}
		const char* Name;
		const char* Value;
	};

	// Owl triple as struct
	struct SLOwlTriple
	{
		SLOwlTriple(const char* Subj, const char* Pred, const char* Obj, const char* Val = "")
			: Subject(Subj), Predicate(Pred), Object(Obj), Value(Val)
		{}
		const char* Subject;
		const char* Predicate;
		const char* Object;
		const char* Value;
	};

	// Add attribute to node
	static FORCEINLINE void AddNodeAttribute(
		rapidxml::xml_document<>* Doc,
		rapidxml::xml_node<>* ParentNode,
		const char* Name,
		const char* Value)
	{
		ParentNode->append_attribute(Doc->allocate_attribute(
			Name, Value));
	}

	// Add attributes to node
	static FORCEINLINE void AddNodeAttributes(
		rapidxml::xml_document<>* Doc,
		rapidxml::xml_node<>* ParentNode,
		TArray<RNodeAttribute> Attributes)
	{
		for(const auto AttributeItr : Attributes)
		{
			ParentNode->append_attribute(Doc->allocate_attribute(
				AttributeItr.Name, AttributeItr.Value));
		}
	}

	// Add comment to node
	static FORCEINLINE void AddNodeComment(
		rapidxml::xml_document<>* Doc,
		rapidxml::xml_node<>* ParentNode,
		const char* Comment)
	{
		// Create comment node
		rapidxml::xml_node<> *CommentNode = Doc->allocate_node(rapidxml::node_comment, "", Comment);
		// Append comment node to parent
		ParentNode->append_node(CommentNode);
	}

	// Add OWL triple to node
	static FORCEINLINE void AddNodeTriple(
		rapidxml::xml_document<>* Doc,
		rapidxml::xml_node<>* ParentNode,
		const SLOwlTriple Triple)
	{
		// Create Triple node
		rapidxml::xml_node<> *TripleNode = Doc->allocate_node(rapidxml::node_element, Triple.Subject, Triple.Value);
		// Add predicate and object to Triple
		TripleNode->append_attribute(Doc->allocate_attribute(Triple.Predicate, Triple.Object));
		// Append triple to parent
		ParentNode->append_node(TripleNode);
	}

	// Add OWL triples to node
	static FORCEINLINE void AddNodeTriples(
		rapidxml::xml_document<>* Doc,
		rapidxml::xml_node<>* ParentNode,
		const TArray<SLOwlTriple>& Triples)
	{
		for (const auto TripleItr : Triples)
		{
			FSLUtils::AddNodeTriple(Doc, ParentNode, TripleItr);
		}
	}

	// Add OWL entity with one property to node
	static FORCEINLINE void AddNodeEntityWithProperty(
		rapidxml::xml_document<>* Doc,
		rapidxml::xml_node<>* ParentNode,
		const SLOwlTriple EntityTriple,
		const SLOwlTriple PropertyTriple)
	{
		// Create the entity node
		rapidxml::xml_node<> *EntityNode = Doc->allocate_node(rapidxml::node_element, EntityTriple.Subject, EntityTriple.Value);
		// Add predicate and object to entity node
		EntityNode->append_attribute(Doc->allocate_attribute(EntityTriple.Predicate, EntityTriple.Object));
		// Add property triple to entity node
		FSLUtils::AddNodeTriple(Doc, EntityNode, PropertyTriple);
		// Append entity to parent
		ParentNode->append_node(EntityNode);
	}
	
	// Add OWL entity with multiple properties to node
	static FORCEINLINE void AddNodeEntityWithProperties(
		rapidxml::xml_document<>* Doc,
		rapidxml::xml_node<>* ParentNode,
		const SLOwlTriple EntityTriple,
		const TArray<SLOwlTriple>& PropertyTriples)
	{
		// Create the entity node
		rapidxml::xml_node<> *EntityNode = Doc->allocate_node(rapidxml::node_element, EntityTriple.Subject, EntityTriple.Value);
		// Add predicate and object to entity node
		EntityNode->append_attribute(Doc->allocate_attribute(EntityTriple.Predicate, EntityTriple.Object));
		// Add property triples to entity node
		FSLUtils::AddNodeTriples(Doc, EntityNode, PropertyTriples);
		// Append entity to parent
		ParentNode->append_node(EntityNode);
	}
};
