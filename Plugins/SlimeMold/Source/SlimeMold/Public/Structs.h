// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Structs.generated.h"

USTRUCT(Blueprintable, BlueprintType)
struct SLIMEMOLD_API FSkeletonPoint
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector WorldPos = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Thickness = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Clusterization = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Veinness = 0.0f;
};



USTRUCT(BlueprintType)
struct SLIMEMOLD_API FSkeletonLine
{
	GENERATED_BODY()

	FSkeletonLine() {}
	FSkeletonLine(int32 i, int32 j) : Point1ID(i), Point2ID(j) {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Point1ID = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Point2ID = -1;

	bool operator==(const FSkeletonLine& Other) const
	{
		return (this->Point1ID == Other.Point1ID && this->Point2ID == Other.Point2ID)
			|| (this->Point1ID == Other.Point2ID && this->Point2ID == Other.Point1ID);
	}
};