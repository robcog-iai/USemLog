// Copyright 2017-present, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#pragma once

#include "CoreMinimal.h"
#include "SLUuid.generated.h"

/*
* 
*/
USTRUCT()
struct USEMLOG_API FSLUuid
{
	GENERATED_BODY();
		
	/* Encodings */
	// Creates a new GUID and encodes it to Base64
	static FString NewGuidInBase64();
	   
	// Creates a new GUID and encodes it to Base64Url
	static FString NewGuidInBase64Url();

	// Creates a new GUID and encodes it to hex
	static FString NewGuidInHex();


	/* Decodings */
	// Creates a GUID from Base64
	static FGuid Base64ToGuid(const FString& InGuidInBase64);

	// Creates a GUID from Base64Url
	static FGuid Base64UrlToGuid(const FString& InGuidInBase64);

	// Creates GUID from hex string
	static FGuid HexToGuid(const FString& InHex);


	/* Pairing functions */
	// Encode to cantor pair; !! f(a,b) != f(b,a); !! (https://en.wikipedia.org/wiki/Pairing_function)
	static uint64 PairEncodeCantor(uint32 X, uint32 Y);

	// Decode to cantor pair (if order is ignored the small number will alway be X)	
	static void PairDecodeCantor(uint64 InP, uint32& OutX, uint32& OutY);

	// Encode to 64 bit pair (https://stackoverflow.com/questions/26222273/is-there-a-better-implementation-for-keeping-a-count-for-unique-integer-pairs)
	static uint64 PairEncodeShift(uint32 X, uint32 Y);

	// Decode from 64 bit pair
	static void PairDecodeShift(uint64 InP, uint32& OutX, uint32& OutY);

	// Encode to Szudzik pair (if order is ignored the small number will alway be X) (http://szudzik.com/ElegantPairing.pdf)
	static uint64 PairEncodeSzudzik(uint32 X, uint32 Y);	

	// Decode from Szudzik pair
	static void PairDecodeSzudzik(uint64 InP, uint32& OutX, uint32& OutY);

private:
	/* Conversions */
	// Encodes GUID to Base64
	FORCEINLINE static FString GuidToBase64(FGuid InGuid);

	// Encodes GUID to Base64
	FORCEINLINE static FString GuidToBase64Url(FGuid InGuid);

	// Encodes GUID to hex
	FORCEINLINE static FString GuidToHex(FGuid InGuid);

	// Convert Base64 to Base64Url (e.g. replace '+', '/' with '_','-')
	FORCEINLINE static FString Base64ToBase64Url(const FString& InBase64);

	// Convert Base64Url to Base64 (e.g. replace '_','-' with '+', '/')
	FORCEINLINE static FString Base64UrlToBase64(const FString& InBase64Url);
};
