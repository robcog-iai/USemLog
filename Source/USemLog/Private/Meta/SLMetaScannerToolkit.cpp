// Copyright 2019, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Meta/SLMetaScannerToolkit.h"

// Ctor
FSLMetaScannerToolkit::FSLMetaScannerToolkit()
{
}

// Get the bounding box and the number of pixels the item occupies in the image
void FSLMetaScannerToolkit::GetItemPixelNumAndBB(const TArray<FColor>& InBitmap, int32 Width, int32 Height,
	int32& OutPixelNum, FIntPoint& OutBBMin, FIntPoint& OutBBMax)
{
	GetColorPixelNumAndBB(InBitmap, WhiteMaskColorConst, Width, Height, OutPixelNum, OutBBMin, OutBBMax);
}

// Get the bounding box and the number of pixels the color occupies in the image
void FSLMetaScannerToolkit::GetColorPixelNumAndBB(const TArray<FColor>& InBitmap, const FColor& Color, int32 Width,
	int32 Height, int32& OutPixelNum, FIntPoint& OutBBMin, FIntPoint& OutBBMax)
{
	int32 RowIdx = 0;
	int32 ColIdx = 0;
	OutPixelNum = 0;
	OutBBMin = FIntPoint(Width, Height);
	OutBBMax = FIntPoint(0, 0);

	for (const auto& C : InBitmap)
	{
		if (C == Color)
		{
			OutPixelNum++;

			if (RowIdx < OutBBMin.Y)
			{
				OutBBMin.Y = RowIdx;
			}
			if (RowIdx > OutBBMax.Y)
			{
				OutBBMax.Y = RowIdx;
			}
			if (ColIdx < OutBBMin.X)
			{
				OutBBMin.X = ColIdx;
			}
			if (ColIdx > OutBBMax.X)
			{
				OutBBMax.X = ColIdx;
			}
		}

		//// DEBUG
		//if(C == Color)
		//{
		//	UE_LOG(LogTemp, Warning, TEXT("%s::%d \t\t[\t%ld\t|\t%ld\t];\t\t||\t\tMin=[\t%ld\t|\t%ld\t];\t\t||\t\tMax=[\t%ld\t|\t%ld\t]]"),
		//		*FString(__func__), __LINE__, RowIdx, ColIdx, OutBBMin.X, OutBBMin.Y, OutBBMax.X, OutBBMax.Y);
		//}
		//else
		//{
		//	UE_LOG(LogTemp, Error, TEXT("%s::%d \t\t[\t%ld\t|\t%ld\t];\t\t||\t\tMin=[\t%ld\t|\t%ld\t];\t\t||\t\tMax=[\t%ld\t|\t%ld\t]]"),
		//		*FString(__func__), __LINE__, RowIdx, ColIdx, OutBBMin.X, OutBBMin.Y, OutBBMax.X, OutBBMax.Y);
		//}


	}
}

// Get the number of pixels that the item occupies in the image
int32 FSLMetaScannerToolkit::GetItemPixelNum(const TArray<FColor>& Bitmap)
{
	return GetColorPixelNum(Bitmap, WhiteMaskColorConst);
}

// Get the number of pixels of the given color in the image
int32 FSLMetaScannerToolkit::GetColorPixelNum(const TArray<FColor>& Bitmap, const FColor& Color) const
{
	int32 Num = 0;
	for (const auto& C : Bitmap)
	{
		if (C == Color)
		{
			Num++;
		}
	}
	return Num++;
}

// Get the number of pixels of the given two colors in the image
void FSLMetaScannerToolkit::GetColorsPixelNum(const TArray<FColor>& Bitmap, const FColor& ColorA, int32& OutNumA, const FColor& ColorB, int32& OutNumB)
{
	OutNumA = 0;
	OutNumB = 0;
	for (const auto& C : Bitmap)
	{
		if (C == ColorA)
		{
			OutNumA++;
		}
		else if (C == ColorB)
		{
			OutNumB++;
		}
	}
}

// Count (and check) the number of pixels the item uses in the image
void FSLMetaScannerToolkit::CountItemPixelNumWithCheck(const TArray<FColor>& Bitmap, int32 ResX, int32 ResY)
{
	//// Count pixel colors
	//if (ViewModes[CurrViewModeIdx] == ESLMetaScannerViewMode::White)
	//{
	//	// Cache the previous number of item pixels
	//	int32 PrevItemPixelNum = TempItemPixelNum;

	//	int32 NumBackgroundPixels = 0;
	//	GetColorsPixelNum(Bitmap, FColor::Black, NumBackgroundPixels, MaskColorConst, TempItemPixelNum);
	//	if (TempItemPixelNum + NumBackgroundPixels != ResX * ResY)
	//	{
	//		UE_LOG(LogTemp, Error, TEXT("%s::%d Number of item [%ld] + background [%ld] pixels, differs of number image total [%ld]."),
	//			*FString(__func__), __LINE__, TempItemPixelNum, NumBackgroundPixels, ResX * ResY);
	//	}

	//	// Compare against previous (other view mode) number item pixels
	//	if (PrevItemPixelNum != INDEX_NONE)
	//	{
	//		if (TempItemPixelNum != PrevItemPixelNum)
	//		{
	//			UE_LOG(LogTemp, Error, TEXT("%s::%d Prev number [%ld] of item pixels differs of current one [%ld]."),
	//				*FString(__func__), __LINE__, PrevItemPixelNum, TempItemPixelNum);
	//		}
	//	}
	//}
	//else if (ViewModes[CurrViewModeIdx] == ESLMetaScannerViewMode::Unlit)
	//{
	//	// Cache the previous number of item pixels
	//	int32 PrevItemPixelNum = TempItemPixelNum;

	//	int32 NumBackgroundPixels = GetColorPixelNum(Bitmap, FColor::Black);
	//	TempItemPixelNum = ResX * ResY - NumBackgroundPixels;

	//	// Compare against previous (other view mode) number item pixels
	//	if (PrevItemPixelNum != INDEX_NONE)
	//	{
	//		if (TempItemPixelNum != PrevItemPixelNum)
	//		{
	//			UE_LOG(LogTemp, Error, TEXT("%s::%d Prev number [%ld] of item pixels differs of current one [%ld]."),
	//				*FString(__func__), __LINE__, PrevItemPixelNum, TempItemPixelNum);
	//		}
	//	}
	//}
}

// Generate sphere scan poses
void FSLMetaScannerToolkit::GenerateSphereScanPoses(uint32 MaxNumOfPoints, float Radius, TArray<FTransform>& OutTransforms)
{
	// (https://www.cmu.edu/biolphys/deserno/pdf/sphere_equi.pdf)
	const float Area = 4 * PI / MaxNumOfPoints;
	const float Distance = FMath::Sqrt(Area);

	// Num of latitudes
	const int32 MTheta = FMath::RoundToInt(PI / Distance);
	const float DTheta = PI / MTheta;
	const float DPhi = Area / DTheta;

	// Iterate latitude lines
	for (int32 M = 0; M < MTheta; M++)
	{
		// 0 <= Theta <= PI
		const float Theta = PI * (float(M) + 0.5) / MTheta;

		// Num of longitudes
		const int32 MPhi = FMath::RoundToInt(2 * PI * FMath::Sin(Theta) / DPhi);
		for (int32 N = 0; N < MPhi; N++)
		{
			// 0 <= Phi < 2pi 
			const float Phi = 2 * PI * N / MPhi;

			FVector Point;
			Point.X = FMath::Sin(Theta) * FMath::Cos(Phi) * Radius;
			Point.Y = FMath::Sin(Theta) * FMath::Sin(Phi) * Radius;
			Point.Z = FMath::Cos(Theta) * Radius;
			FQuat Quat = (-Point).ToOrientationQuat();

			OutTransforms.Emplace(Quat, Point);
		}
	}
}
