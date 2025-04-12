 // Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include <CoreMinimal.h>
#include "Structs.h"


#include "SlimeMoldSkeletonComponent.generated.h"



DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGenerateMeshEvent, UObject*, Properties);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGenerateDebugInfoEvent, UObject*, Properties);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAssignMaterials, UObject*, Properties);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FClearMesh, UObject*, Properties);

UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class SLIMEMOLD_API USlimeMoldSkeletonComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	TArray<FSkeletonPoint> SkeletonPoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	TArray<FSkeletonLine> SkeletonLines;

	// GenerateMesh button triggers this event
	UPROPERTY(BlueprintAssignable, EditDefaultsOnly)
	FGenerateMeshEvent OnGenerateMesh;

	// GenerateDebugInfo button triggers this event
	UPROPERTY(BlueprintAssignable, EditDefaultsOnly)
	FGenerateDebugInfoEvent OnGenerateDebugInfo;

	// AssignMaterials button triggers this event
	UPROPERTY(BlueprintAssignable, EditDefaultsOnly)
	FAssignMaterials OnAssignMaterials;

	// ClearMesh button triggers this event
	UPROPERTY(BlueprintAssignable, EditDefaultsOnly)
	FClearMesh OnClearMesh;
};