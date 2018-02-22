// Copyright 2017, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "SLOwl.h"

/////////////////////////////////////////////////////////

//// Default constructor
//FOwlPrefixName::FOwlPrefixName()
//{
//}

//// Constructor from Prefix, Name
//FOwlPrefixName::FOwlPrefixName(const FString InPrefix, const FString InName)
//	: Prefix(InPrefix), Name(InName)
//{
//}
//
//// Constructor from FullName (e.g. rdf:type)
//FOwlPrefixName::FOwlPrefixName(const FString FullName)
//{
//	// Split into Ns and Class
//	FullName.Split(":", &Prefix, &Name);
//}
//
//// Return the object as Prefix + Name
//FString FOwlPrefixName::GetFullName() const
//{
//	return Prefix + ":" + Name;
//}
//
//// Set from full name
//void FOwlPrefixName::Set(const FString FullName)
//{
//	// Split into Prefix and Class
//	FullName.Split(":", &Prefix, &Class));
//}
//
//// Set from namespace and name
//void FOwlPrefixName::Set(const FString InPrefix, const FString InName)
//{
//	Prefix = InPrefix;
//	Name = InName;
//}