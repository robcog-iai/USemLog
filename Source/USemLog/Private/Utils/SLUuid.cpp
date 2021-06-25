// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
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


/* Pairing functions */
// Encode to cantor pair; !! f(a,b) != f(b,a); (https://en.wikipedia.org/wiki/Pairing_function)
uint64 FSLUuid::PairEncodeCantor(uint32 X, uint32 Y)
{
	 return (uint64)(0.5 * (X + Y) * (X + Y + 1) + Y);
}

// Decode to cantor pair (if order is ignored the small number will alway be X)	
void FSLUuid::PairDecodeCantor(uint64 InP, uint32& OutX, uint32& OutY)
{
	uint32 W = floor(((sqrt((InP * 8) + 1)) - 1) / 2);
	uint32 T = (W * (W + 1)) / 2;
	OutY = InP - T;
	OutX = W - OutY;
}

// Encode to 64 bit pair (https://stackoverflow.com/questions/26222273/is-there-a-better-implementation-for-keeping-a-count-for-unique-integer-pairs)
uint64 FSLUuid::PairEncodeShift(uint32 X, uint32 Y)
{
	UE_LOG(LogTemp, Error, TEXT("%s::%d TODO"), *FString(__FUNCTION__), __LINE__);
	return 0;
	//uint64 A = X + Y;
	//uint64 B = abs((int32)(X - Y));
	//return (uint64)(A << 32) | (B);
}

// Decode from 64 bit pair
void FSLUuid::PairDecodeShift(uint64 InP, uint32& OutX, uint32& OutY)
{
	UE_LOG(LogTemp, Error, TEXT("%s::%d TODO"), *FString(__FUNCTION__), __LINE__);
	//OutX = InP >> 32;
	//OutY = InP & 0xFFFFFFFF;
}

// Encode to Szudzik pair (if order is ignored the small number will alway be X) (http://szudzik.com/ElegantPairing.pdf)
uint64 FSLUuid::PairEncodeSzudzik(uint32 X, uint32 Y)
{
	UE_LOG(LogTemp, Error, TEXT("%s::%d TODO"), *FString(__FUNCTION__), __LINE__);
	return 0;
	//return X < Y ? (uint64)(Y*Y+X) : (uint64)(X*X+X+Y);
}

// Decode from Szudzik pair
void FSLUuid::PairDecodeSzudzik(uint64 InP, uint32& OutX, uint32& OutY)
{
	UE_LOG(LogTemp, Error, TEXT("%s::%d TODO"), *FString(__FUNCTION__), __LINE__);
	//uint32 Q = floor(sqrt(InP));
	//uint32 L = InP - Q ^ 2;
	//if (Q < L)
	//{
	//	OutX = Q;
	//	OutY = L;
	//}
	//else
	//{
	//	OutX = Q;
	//	OutY = L - Q;
	//}
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

