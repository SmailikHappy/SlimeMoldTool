// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InteractiveToolBuilder.h"
#include "BaseTools/ClickDragTool.h"
#include "BaseTools/SingleClickTool.h"

// SlimeMold data structures
#include "SlimeMoldSkeletonComponent.h"
#include "Structs.h"

// Adding gizmo to the world
#include "BaseGizmos/TransformGizmoUtil.h"
#include "InteractiveToolObjects.h"
#include "InteractiveToolBuilder.h"



#include "SlimeMoldSkeletonEditingTool.generated.h"

/**
 * This is a Tool Builder for our skeleton editor
 */
UCLASS()
class SLIMEMOLDEDITORTOOL_API USlimeMoldSkeletonEditingToolBuilder : public UInteractiveToolBuilder
{
	GENERATED_BODY()

public:
	virtual bool CanBuildTool(const FToolBuilderState& SceneState) const override;
	virtual UInteractiveTool* BuildTool(const FToolBuilderState& SceneState) const override;
};



UCLASS(Config = EditorPerProjectUserSettings)
class SLIMEMOLDEDITORTOOL_API USlimeMoldSkeletonEditingToolProperties : public UInteractiveToolPropertySet
{
	GENERATED_BODY()

public:
	USlimeMoldSkeletonEditingToolProperties() {}

	/** Point thickness of the of the main vein */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Point data")
	float PointThickness = 1.0f;

	/** The amount of clusters (not a number) to generate from the line with this point */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Point data")
	float PointClusterization = 0.0f;

	/** Value that corresponds to vein producing */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Point data")
	float PointVeinness = 0.0f;


	UPROPERTY(EditAnywhere, Category = "Buttons")
	bool bDeletePoints = false;

	UPROPERTY(EditAnywhere, Category = "Buttons")
	bool bDisconnectPoints = false;

	UPROPERTY(EditAnywhere, Category = "Buttons")
	bool bSplitLine = false;
	
#pragma region EditorSettings
	/** Mouse 'radius' point detection threshlod */
	UPROPERTY(EditAnywhere, Category = "Editor settings")
	float SelectionRadiusThreshold = 1.0f;

	/** Decides whether we shall change the selection to just created point (deselect all the points that were selected THEN select only the point that was just created) */
	UPROPERTY(EditAnywhere, Category = "Editor settings")
	bool bChangeSelectionOnPointCreate = true;

	/** Point color with minimal clusterization */
	UPROPERTY(EditAnywhere, Category = "Editor settings")
	FLinearColor PointColorMinClusterization = FLinearColor::Black;

	/** Point color with maximal veinness */
	UPROPERTY(EditAnywhere, Category = "Editor settings")
	FLinearColor PointColorMaxClasterization = FLinearColor::White;

	/** Point color when selected */
	UPROPERTY(EditAnywhere, Category = "Editor settings")
	FLinearColor PointColorSelected = FLinearColor::Red;

	/** Ghost point color */
	UPROPERTY(EditAnywhere, Category = "Editor settings")
	FLinearColor GhostPointColor = FLinearColor(1.0f, 0.2f, 1.0f, 1.0f);

	/** Line color with minimal veinness */
	UPROPERTY(EditAnywhere, Category = "Editor settings")
	FLinearColor LineColorMinVeinness = FLinearColor(0.25f, 0.25f, 0.0f, 1.0f);

	/** Line color with maximal veinness */
	UPROPERTY(EditAnywhere, Category = "Editor settings")
	FLinearColor LineColorMaxVeinness = FLinearColor(0.0f, 1.0f, 1.0f, 1.0f);

	/** Line color when selected */
	UPROPERTY(EditAnywhere, Category = "Editor settings")
	FLinearColor LineColorSelected = FLinearColor::Red;

	/** Ghost line color */
	UPROPERTY(EditAnywhere, Category = "Editor settings")
	FLinearColor GhostLineColor = FLinearColor(1.0f, 0.5f, 0.5f, 1.0f);
#pragma endregion
};



/**
 * Tool logic
 */
UCLASS()
class SLIMEMOLDEDITORTOOL_API USlimeMoldSkeletonEditingTool : public UInteractiveTool, public IHoverBehaviorTarget, public IClickDragBehaviorTarget
{
	GENERATED_BODY()

public:
	virtual void SetWorld(UWorld* World);

	/** UInteractiveTool overrides */
	void Setup() override;
	void OnPropertyModified(UObject* PropertySet, FProperty* Property) override;
	void Shutdown(EToolShutdownType ShutdownType) override;
	void Render(IToolsContextRenderAPI* RenderAPI) override;
	void OnTick(float DeltaTime) override;


	/** IModifierToggleBehaviorTarget implementation */
	void OnUpdateModifierState(int ModifierID, bool bIsOn) override; 

	/** IHoverBehaviorTarget implementation */
	FInputRayHit BeginHoverSequenceHitTest(const FInputDeviceRay& PressPos) override { return MouseHittingWorld(PressPos); };
	void OnBeginHover(const FInputDeviceRay& ClickPos) override {};
	void OnEndHover() override {};
	bool OnUpdateHover(const FInputDeviceRay& DevicePos) override { MouseUpdate(DevicePos); return true; };

	/** IClickDragBehaviorTarget implementation */
	FInputRayHit CanBeginClickDragSequence(const FInputDeviceRay& PressPos) override { return MouseHittingWorld(PressPos); };
	void OnClickPress(const FInputDeviceRay& PressPos) override { MouseRayWhenPressed = PressPos; MouseIsPressed = true; MousePressed(); };
	void OnClickDrag(const FInputDeviceRay& DragPos) override { MouseUpdate(DragPos); };
	void OnClickRelease(const FInputDeviceRay& ReleasePos) override { MouseRayWhenReleased = ReleasePos; MouseIsPressed = false; MouseReleased(); };
	void OnTerminateDragSequence() override {};

protected:
	/** Properties of the tool are stored here */
	UPROPERTY()
	TObjectPtr<USlimeMoldSkeletonEditingToolProperties> Properties;

	/** Mouse controls */
	void MousePressed();
	void MouseUpdate(const FInputDeviceRay& DevicePos);
	void MouseReleased();

	/** Debug draw mouse */
	void DrawDebugMouseInfo(const FInputDeviceRay& DevicePos, FColor Color);
	
	/** Skeleton managing functions */
	void AssignProperties();
	void SelectPoint(int32 PointID);
	void DeselectPoint(int32 PointID);
	void DeselectAllPoints();
	void DeleteSelectedPoints();
	void ConnectPoints(int32 Point1ID, int32 Point2ID);
	void DisconnectPoints(int32 Point1ID, int32 Point2ID);
	void DisconnectSelectedPoints();
	void SplitLine(const FSkeletonLine& line);

	bool bDrawDebugMouseInfo = false;


	/** Helper functions */
	TArray<FSkeletonLine> GetSelectedLines();
	TSet<int32> GetPointIDsInMouseRegion(const FInputDeviceRay& DevicePos);
	FSkeletonPoint& GetClosestPointToMouse(const FInputDeviceRay& DevicePos, const TSet<int32>& SetOfPointIDs, int32& ClosestPointID);
	FInputRayHit FindRayHit(const FRay& WorldRay, FVector& HitPos);

	FSkeletonPoint& CreatePoint(const FInputDeviceRay& ClickPos, int32& NewPointID);

private:
	/** Utility functions */
	FInputRayHit MouseHittingWorld(const FInputDeviceRay& ClickPos);
	
	/** Gizmo functionality */
	void ShowGizmo(const FTransform& IntialTransform);
	void HideGizmo();
	void CreateGizmo();
	void DestroyGizmo();

	UPROPERTY()
	UCombinedTransformGizmo* TransformGizmo = nullptr;

	FTransform PreviousGizmoTransform = FTransform::Identity;
	FVector GizmoPositionDelta = FVector::ZeroVector;

protected:

	// Variable updates only when the mouse is not pressed
	bool ShiftIsPressed = false;
	// Variable updates only when the mouse is not pressed
	bool CtrlIsPressed = false;

	bool MouseIsPressed = false;
	

	FInputDeviceRay MouseRayWhenPressed;
	FInputDeviceRay MouseRayWhenReleased;

	UPROPERTY()
	UWorld* TargetWorld = nullptr;
	
	UPROPERTY()
	USlimeMoldSkeletonComponent* TargetActorComponent = nullptr;
	
	TSet<int32> SelectedPointIDs;		// set ensures no duplicates
	 
	bool bDrawGhostPoint = false;
	bool bDrawGhostLines = false;
	FSkeletonPoint GhostPoint;		// A ghost point that is not yet placed but is being shown where it could be placed OR
									// A ghost connection with selected and ghost point, shown where new lines might appear


	bool bConnectGhostAndSelectedPoints = true;
};
