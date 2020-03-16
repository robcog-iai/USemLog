// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Utils/SLUuid.h"
#include "Misc/Guid.h"
#include "Misc/Base64.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/BufferArchive.h"

/* Encodings */
// Creates a new GUID and encodes it to Base64
FString FSLUuid::NewGuidInBase64()
{
	return GuidToBase64(FGuid::NewGuid());
}

// Creates a new GUID and encodes it to Base64Url
FString FSLUuid::NewGuidInBase64Url()
{
	return GuidToBase64Url(FGuid::NewGuid());
}

// Creates a new GUID and encodes it to hex
FString FSLUuid::NewGuidInHex()
{
	return GuidToHex(FGuid::NewGuid());
}

/* Decodings */
// Creates a GUID from Base64
FGuid FSLUuid::Base64ToGuid(const FString& InGuidInBase64)
{
	TArray<uint8> GuidBinaryArray;
	FBase64::Decode(InGuidInBase64, GuidBinaryArray);
	FMemoryReader Ar = FMemoryReader(GuidBinaryArray, true);
	Ar.Seek(0);
	FGuid LocalGuid;
	Ar << LocalGuid;
	if (LocalGuid.IsValid())
	{
		return LocalGuid;
	}
	else
	{
		return FGuid(); //Invalid guid
	}
}

// Creates a GUID from Base64Url
FGuid FSLUuid::Base64UrlToGuid(const FString& InGuidInBase64)
{
	return Base64ToGuid(Base64UrlToBase64(InGuidInBase64));
}

// Creates GUID from hex string
FGuid FSLUuid::HexToGuid(const FString& InHex)
{
	// TODO check size first
	return FGuid(
		FParse::HexNumber(*InHex.Mid(0, 8)),
		FParse::HexNumber(*InHex.Mid(8, 8)),
		FParse::HexNumber(*InHex.Mid(16, 8)),
		FParse::HexNumber(*InHex.Mid(24, 8))
	);
}


/* Conversions */
// Encodes GUID to Base64
FString FSLUuid::GuidToBase64(FGuid InGuid)
{
	FBufferArchive GuidBufferArchive;							// FBufferArchive inherits from TArray<uint8>
	GuidBufferArchive << InGuid;
	FString GuidInBase64 = FBase64::Encode(GuidBufferArchive);	// Needs binary as TArray<uint8>
	GuidInBase64.RemoveFromEnd(TEXT("=="));						// Remove last unnecessary two equal characters from end
	return GuidInBase64;
}

// Encodes GUID to Base64
FString FSLUuid::GuidToBase64Url(FGuid InGuid)
{
	FBufferArchive GuidBufferArchive;							// FBufferArchive inherits from TArray<uint8>
	GuidBufferArchive << InGuid;
	FString GuidInBase64 = FBase64::Encode(GuidBufferArchive);	// Needs binary as TArray<uint8>
	GuidInBase64.RemoveFromEnd(TEXT("=="));						// Remove last unnecessary two equal characters from end
	return Base64ToBase64Url(GuidInBase64);
}

// Encodes GUID to hex
FString FSLUuid::GuidToHex(FGuid InGuid)
{
	// TODO not tested
	return FString::Printf(TEXT("%08X%04X%04X%02X%02X%02X%02X%02X%02X%02X%02X"),
		InGuid.A, InGuid.B >> 16, InGuid.B & 0xFFFF,
		InGuid.C >> 24, (InGuid.C >> 16) & 0xFF, (InGuid.C >> 8) & 0xFF, InGuid.C & 0XFF,
		InGuid.D >> 24, (InGuid.D >> 16) & 0XFF, (InGuid.D >> 8) & 0XFF, InGuid.D & 0XFF);
}

// Convert Base64 to Base64Url (e.g. replace '+', '/' with '_','-')
FString FSLUuid::Base64ToBase64Url(const FString& InBase64)
{
	FString Base64Url = InBase64;
	Base64Url.ReplaceInline(TEXT("+"), TEXT("-"), ESearchCase::CaseSensitive);
	Base64Url.ReplaceInline(TEXT("/"), TEXT("_"), ESearchCase::CaseSensitive);
	return Base64Url;
}

// Convert Base64Url to Base64 (e.g. replace '_','-' with '+', '/')
FString FSLUuid::Base64UrlToBase64(const FString& InBase64Url)
{
	FString Base64 = InBase64Url;
	Base64.ReplaceInline(TEXT("-"), TEXT("+"), ESearchCase::CaseSensitive);
	Base64.ReplaceInline(TEXT("_"), TEXT("/"), ESearchCase::CaseSensitive);
	return Base64;
}

