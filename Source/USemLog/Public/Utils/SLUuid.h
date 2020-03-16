// Copyright 2017-2020, Institute for Artificial Intelligence - University of Bremen
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
