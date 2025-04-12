// Copyright Epic Games, Inc. All Rights Reserved.

#include "SlimeMoldSkeletonEditingTool.h"
#include "InteractiveToolManager.h"
#include "ToolBuilderUtil.h"
#include "BaseBehaviors/SingleClickBehavior.h"
#include "BaseBehaviors/ClickDragBehavior.h"
#include "BaseBehaviors/MouseHoverBehavior.h"

#include "SlimeMoldSkeletonEditingCustomization.h"

// for raycast into World
#include "CollisionQueryParams.h"
#include "Engine/World.h"

#include "SceneManagement.h"
#include <Kismet/GameplayStatics.h>


// Custom static functions
#include "SlimeMoldEditorToolFunctionLibrary.h"



// localization namespace
#define LOCTEXT_NAMESPACE "USlimeMoldSkeletonEditingTool"


/*
 * Tool builder
 */

bool USlimeMoldSkeletonEditingToolBuilder::CanBuildTool(const FToolBuilderState& SceneState) const
{
	return USlimeMoldEditorFuncLib::SingleActorWithSkeletonComponentIsSelected();
}

UInteractiveTool* USlimeMoldSkeletonEditingToolBuilder::BuildTool(const FToolBuilderState& SceneState) const
{
	USlimeMoldSkeletonEditingTool* NewTool = NewObject<USlimeMoldSkeletonEditingTool>(SceneState.ToolManager);
	NewTool->SetWorld(SceneState.World);
	return NewTool;
}



/*
 * Tool logic
 */

void USlimeMoldSkeletonEditingTool::Setup()
{
	TargetActorComponent = USlimeMoldEditorFuncLib::GetSkeletonComponentFromSelectedActor();
	check(TargetActorComponent);

	UInteractiveTool::Setup();

	UMouseHoverBehavior* HoverBehavior = NewObject<UMouseHoverBehavior>();
	HoverBehavior->Initialize(this); 
	AddInputBehavior(HoverBehavior);

	UClickDragInputBehavior* ClickDragBehavior = NewObject<UClickDragInputBehavior>();
	ClickDragBehavior->Initialize(this);
	AddInputBehavior(ClickDragBehavior);

	HoverBehavior->Modifiers.RegisterModifier(1, FInputDeviceState::IsShiftKeyDown);
	HoverBehavior->Modifiers.RegisterModifier(2, FInputDeviceState::IsCtrlKeyDown);

	CreateGizmo();



	Properties = NewObject<USlimeMoldSkeletonEditingToolProperties>(this);
	AddToolPropertySource(Properties);

	Properties->RestoreProperties(this, "Skeleton properties");

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	PropertyModule.RegisterCustomClassLayout(
		USlimeMoldSkeletonEditingToolProperties::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FSlimeMoldSkeletonEditingCustomization::MakeInstance)
	);

	UE_LOG(LogTemp, Display, TEXT("Skeleton editing tool has been initialized"));
}

void USlimeMoldSkeletonEditingTool::SetWorld(UWorld* World)
{
	check(World);
	this->TargetWorld = World;
}

void USlimeMoldSkeletonEditingTool::OnPropertyModified(UObject* PropertySet, FProperty* Property)
{
	if (!PropertySet) return;

	if (PropertySet != Properties) return;

	// Buttons
	{
		// "Delete points" button pressed
		if (Property->GetName() == "bDeletePoints")
		{
			DeleteSelectedPoints();
		}
		// "Disconnect points" button pressed
		else if (Property->GetName() == "bDisconnectPoints")
		{
			DisconnectSelectedPoints();
		}
		// "Split the line" button pressed
		else if (Property->GetName() == "bSplitLine")
		{
			TArray<FSkeletonLine> SelectedLines = GetSelectedLines();

			for (FSkeletonLine LineToSplit : SelectedLines)
			{
				SplitLine(LineToSplit);
			}
		}
	}

	// Point data update
	if (Property->GetName() == "PointThickness")
	{
		for (int32 PointID : SelectedPointIDs)
		{
			TargetActorComponent->SkeletonPoints[PointID].Thickness = Properties->PointThickness;
		}
	}
	else if (Property->GetName() == "PointClusterization")
	{
		for (int32 PointID : SelectedPointIDs)
		{
			TargetActorComponent->SkeletonPoints[PointID].Clusterization = Properties->PointClusterization;
		}
	}
	else if (Property->GetName() == "PointVeinness")
	{
		for (int32 PointID : SelectedPointIDs)
		{
			TargetActorComponent->SkeletonPoints[PointID].Veinness = Properties->PointVeinness;
		}
	}
}

void USlimeMoldSkeletonEditingTool::Shutdown(EToolShutdownType ShutdownType)
{
	Properties->SaveProperties(this, "Skeleton properties");
	DestroyGizmo();
}

/*
 * Full mouse controls
 */
void USlimeMoldSkeletonEditingTool::MousePressed()
{
	DrawDebugMouseInfo(MouseRayWhenPressed, FColor::Green);

	// Select single point
	if (!ShiftIsPressed && !CtrlIsPressed)
	{
		DeselectAllPoints();

		TSet<int32> PointsInRegion = GetPointIDsInMouseRegion(MouseRayWhenPressed);

		if (!PointsInRegion.IsEmpty())
		{
			int32 ClosestPointID;
			GetClosestPointToMouse(MouseRayWhenPressed, PointsInRegion, ClosestPointID);
			SelectPoint(ClosestPointID);
		}
	}

	// Select another point
	if (!ShiftIsPressed && CtrlIsPressed)
	{
		TSet<int32> PointIDsInRegion = GetPointIDsInMouseRegion(MouseRayWhenPressed);

		// Select the closest point
		if (!PointIDsInRegion.IsEmpty())
		{
			int32 ClosestPointID;
			GetClosestPointToMouse(MouseRayWhenPressed, PointIDsInRegion, ClosestPointID);
			SelectPoint(ClosestPointID);
		}
	}

	// Connect points / Create and connect
	if (ShiftIsPressed && !CtrlIsPressed)
	{
		TSet<int32> PointsInRegion = GetPointIDsInMouseRegion(MouseRayWhenPressed);

		int32 JustConnectedPointID = -1;

		if (!PointsInRegion.IsEmpty())
		{
			int32 ClosestPointID;
			GetClosestPointToMouse(MouseRayWhenPressed, PointsInRegion, ClosestPointID);

			if (ClosestPointID != -1)
			{
				for (int32 PointID : SelectedPointIDs)
				{
					ConnectPoints(PointID, ClosestPointID);
				}
			}

			JustConnectedPointID = ClosestPointID;
		}
		else
		{
			int32 NewPointID;
			CreatePoint(MouseRayWhenPressed, NewPointID);

			for (int32 PointID : SelectedPointIDs)
			{
				ConnectPoints(PointID, NewPointID);
			}

			JustConnectedPointID = NewPointID;
		}

		// Change selection from selected points to just connected one if property stays
		if (Properties->bChangeSelectionOnPointCreate)
		{
			DeselectAllPoints();

			if (JustConnectedPointID != -1) {
				SelectPoint(JustConnectedPointID);
			}
		}
	}
}

void USlimeMoldSkeletonEditingTool::MouseUpdate(const FInputDeviceRay& DevicePos)
{
	DrawDebugMouseInfo(DevicePos, MouseIsPressed ? FColor::Blue : FColor::Cyan);

	bDrawGhostPoint = false;
	bDrawGhostLines = false;

	// Multiple point selection
	if (MouseIsPressed && !ShiftIsPressed && CtrlIsPressed)
	{
		TSet<int32> PointIDsInRegion = GetPointIDsInMouseRegion(DevicePos);
		
		for (int32 PointID : PointIDsInRegion)
		{
			SelectPoint(PointID);
		}
	}

	// Point visual potential connection / Line visual potential drawing 
	if (!MouseIsPressed && ShiftIsPressed && !CtrlIsPressed)
	{
		bDrawGhostLines = true;
		TSet<int32> PointsInRegion = GetPointIDsInMouseRegion(DevicePos);

		if (!PointsInRegion.IsEmpty())
		{
			int32 ClosestPointID;
			FSkeletonPoint& ClosestPoint = GetClosestPointToMouse(DevicePos, PointsInRegion, ClosestPointID);
			if (ClosestPointID != -1)
			{
				GhostPoint = ClosestPoint;
			}
		}
		else
		{
			bDrawGhostPoint = true;
			FindRayHit(DevicePos.WorldRay, GhostPoint.WorldPos);
		}
	}
}

void USlimeMoldSkeletonEditingTool::MouseReleased()
{
	DrawDebugMouseInfo(MouseRayWhenReleased, FColor::Magenta);
}


/*
 * Helper functions
 */
void USlimeMoldSkeletonEditingTool::AssignProperties()
{
	
}

FInputRayHit USlimeMoldSkeletonEditingTool::FindRayHit(const FRay& WorldRay, FVector& HitPos)
{
	// trace a ray into the World
	FCollisionObjectQueryParams QueryParams(FCollisionObjectQueryParams::AllObjects);
	FHitResult Result;
	bool bHitWorld = TargetWorld->LineTraceSingleByObjectType(Result, WorldRay.Origin, WorldRay.PointAt(999999), QueryParams);
	if (bHitWorld)
	{
		HitPos = Result.ImpactPoint;
		return FInputRayHit(Result.Distance);
	}
	return FInputRayHit();
}

FInputRayHit USlimeMoldSkeletonEditingTool::MouseHittingWorld(const FInputDeviceRay& ClickPos)
{
	FVector RayStart = ClickPos.WorldRay.Origin;
	FVector RayEnd = ClickPos.WorldRay.PointAt(99999999.f);
	FCollisionObjectQueryParams QueryParams(FCollisionObjectQueryParams::AllObjects);
	FHitResult Result;
	FInputRayHit Hit;


	if (TargetWorld->LineTraceSingleByObjectType(Result, RayStart, RayEnd, QueryParams))
	{

		Hit.bHit = true;
		Hit.HitDepth = Result.Distance;
	}
	else
	{
		Hit.bHit = false;
	}

	return Hit;
}

void USlimeMoldSkeletonEditingTool::OnUpdateModifierState(int ModifierID, bool bIsOn)
{
	if (ModifierID == 1)
	{
		ShiftIsPressed = bIsOn;
	}
	if (ModifierID == 2)
	{
		CtrlIsPressed = bIsOn;
	}
}


/*
 * Skeleton related functions
 */
void USlimeMoldSkeletonEditingTool::SelectPoint(int32 PointID)
{
	FSkeletonPoint& Point = TargetActorComponent->SkeletonPoints[PointID];
	if (SelectedPointIDs.IsEmpty())
	{
		ShowGizmo(FTransform(Point.WorldPos));
	}

	Properties->PointThickness = Point.Thickness;
	Properties->PointClusterization = Point.Clusterization;
	Properties->PointVeinness = Point.Veinness;

	SelectedPointIDs.Add(PointID);
}

void USlimeMoldSkeletonEditingTool::DeselectPoint(int32 PointID)
{
	if (SelectedPointIDs.Contains(PointID))
	{
		SelectedPointIDs.Remove(PointID);
	}

	if (SelectedPointIDs.IsEmpty())
	{
		HideGizmo();
	}
}

void USlimeMoldSkeletonEditingTool::DeselectAllPoints()
{
	SelectedPointIDs.Empty();
	HideGizmo();
}

void USlimeMoldSkeletonEditingTool::DeleteSelectedPoints()
{
	// Remove connected lines to these points first
	for (int32 PointID : SelectedPointIDs)
	{
		for (int i = 0; i < TargetActorComponent->SkeletonLines.Num(); i++)
		{
			FSkeletonLine& Line = TargetActorComponent->SkeletonLines[i];
			if (Line.Point1ID == PointID || Line.Point2ID == PointID)
			{
				TargetActorComponent->SkeletonLines.RemoveAt(i--);
			}
		}
	}

	// Reconnect lines to right IDs
	TArray<int32> SelectedPointIDsArray = SelectedPointIDs.Array();
	SelectedPointIDsArray.Sort();

	DeselectAllPoints();

	while (!SelectedPointIDsArray.IsEmpty())
	{
		int32 PointIDToRemove = SelectedPointIDsArray[0];

		TargetActorComponent->SkeletonPoints.RemoveAt(PointIDToRemove);

		// Lower the indices of the points in the lines
		for (FSkeletonLine& Line : TargetActorComponent->SkeletonLines)
		{
			if (Line.Point1ID > PointIDToRemove)
			{
				Line.Point1ID--;
			}
			if (Line.Point2ID > PointIDToRemove)
			{
				Line.Point2ID--;
			}
		}

		// Lower the indices of the selected points
		for (int32& PointID : SelectedPointIDsArray)
		{
			PointID--;
		}

		SelectedPointIDsArray.RemoveAt(0);
	}
}

void USlimeMoldSkeletonEditingTool::ConnectPoints(int32 Point1ID, int32 Point2ID)
{
	// Add a new line
	FSkeletonLine NewLine;
	NewLine.Point1ID = Point1ID;
	NewLine.Point2ID = Point2ID;
	// Check if the points are already connected
	for (const FSkeletonLine& Line : TargetActorComponent->SkeletonLines)
	{
		if (NewLine == Line)
		{
			return;
		}
	}
	TargetActorComponent->SkeletonLines.Add(NewLine);
}

void USlimeMoldSkeletonEditingTool::DisconnectPoints(int32 Point1ID, int32 Point2ID)
{
	// Find the line to remove
	FSkeletonLine LineToRemove(Point1ID, Point2ID);
	for (int i = 0; i < TargetActorComponent->SkeletonLines.Num(); i++)
	{
		FSkeletonLine& Line = TargetActorComponent->SkeletonLines[i];
		if (LineToRemove == Line)
		{
			TargetActorComponent->SkeletonLines.RemoveAt(i);
			return;
		}
	}
}

void USlimeMoldSkeletonEditingTool::DisconnectSelectedPoints()
{
	// UNOPTIMIZED
	for (int32 Point1ID : SelectedPointIDs)
	{
		for (int32 Point2ID : SelectedPointIDs)
		{
			if (Point1ID != Point2ID)
			{
				DisconnectPoints(Point1ID, Point2ID);
			}
		}
	}
}

TArray<FSkeletonLine> USlimeMoldSkeletonEditingTool::GetSelectedLines()
{
	TArray<FSkeletonLine> LineArray;

	for (FSkeletonLine& Line : TargetActorComponent->SkeletonLines)
	{
		if (SelectedPointIDs.Contains(Line.Point1ID) && SelectedPointIDs.Contains(Line.Point2ID))
		{
			LineArray.Add(Line);
		}
	}

	return LineArray;
}

TSet<int32> USlimeMoldSkeletonEditingTool::GetPointIDsInMouseRegion(const FInputDeviceRay& DevicePos)
{
	TSet<int32> PointsInRegion;

	FEditorViewportClient* client = (FEditorViewportClient*)GEditor->GetActiveViewport()->GetClient();
	FVector CameraRotation = client->GetViewRotation().Vector();
	FVector CameraLocation = client->GetViewLocation();

	float dotProductK = FVector::DotProduct(CameraRotation, DevicePos.WorldRay.Direction);

	for (int32 PointID = 0; PointID < TargetActorComponent->SkeletonPoints.Num(); PointID++)
	{
		FSkeletonPoint& Point = TargetActorComponent->SkeletonPoints[PointID];

		FVector DirectionToPoint = Point.WorldPos - CameraLocation;
		DirectionToPoint.Normalize();

		float OneMinusDotProduct = 1.0f - FVector::DotProduct(DirectionToPoint, DevicePos.WorldRay.Direction);

		if (OneMinusDotProduct / (dotProductK * dotProductK * dotProductK) < Properties->SelectionRadiusThreshold * 0.001f)
		{
			PointsInRegion.Add(PointID);
		}
	}

	return PointsInRegion;
}

FSkeletonPoint& USlimeMoldSkeletonEditingTool::GetClosestPointToMouse(const FInputDeviceRay& DevicePos, const TSet<int32>& SetOfPoints, int32& ClosestPointID)
{
	// The closest point is the one with biggest dot product
	ClosestPointID = -1;
	float MaxDotProduct = 0.0f;

	for (int32 PointID : SetOfPoints)
	{
		FSkeletonPoint& Point = TargetActorComponent->SkeletonPoints[PointID];
		FVector DirectionToPoint = Point.WorldPos - DevicePos.WorldRay.Origin;
		DirectionToPoint.Normalize();
		float DotProduct = FVector::DotProduct(DirectionToPoint, DevicePos.WorldRay.Direction);
		if (DotProduct > MaxDotProduct)
		{
			MaxDotProduct = DotProduct;
			ClosestPointID = PointID;
		}
	}

	return TargetActorComponent->SkeletonPoints[ClosestPointID];
}

void USlimeMoldSkeletonEditingTool::SplitLine(const FSkeletonLine& Line)
{
	// Create a new point in the middle of the line
	FSkeletonPoint NewPoint;
	FSkeletonPoint& LinePoint1 = TargetActorComponent->SkeletonPoints[Line.Point1ID];
	FSkeletonPoint& LinePoint2 = TargetActorComponent->SkeletonPoints[Line.Point2ID];

	NewPoint.WorldPos = (LinePoint1.WorldPos + LinePoint2.WorldPos) / 2.0f;
	NewPoint.Thickness = (LinePoint1.Thickness + LinePoint2.Thickness) / 2.0f;
	NewPoint.Clusterization = (LinePoint1.Clusterization + LinePoint2.Clusterization) / 2.0f;
	NewPoint.Veinness = (LinePoint1.Veinness + LinePoint2.Veinness) / 2.0f;	

	int32 NewPointID = TargetActorComponent->SkeletonPoints.Add(NewPoint);

	// Create two new lines
	FSkeletonLine NewLine1;
	NewLine1.Point1ID = Line.Point1ID;
	NewLine1.Point2ID = NewPointID;
	TargetActorComponent->SkeletonLines.Add(NewLine1);

	FSkeletonLine NewLine2;
	NewLine2.Point1ID = NewPointID;
	NewLine2.Point2ID = Line.Point2ID;
	TargetActorComponent->SkeletonLines.Add(NewLine2);

	// Remove the old line
	TargetActorComponent->SkeletonLines.Remove(Line);
}

FSkeletonPoint& USlimeMoldSkeletonEditingTool::CreatePoint(const FInputDeviceRay& ClickPos, int32& NewPointID)
{
	FCollisionObjectQueryParams QueryParams(FCollisionObjectQueryParams::AllObjects);
	FHitResult Result;
	bool bHitWorld = TargetWorld->LineTraceSingleByObjectType(Result, ClickPos.WorldRay.Origin, ClickPos.WorldRay.PointAt(999999), QueryParams);
	FSkeletonPoint NewPoint;
	NewPointID = -1;

	if (bHitWorld)
	{
		// Add a new point
		NewPoint.WorldPos = Result.Location;

		NewPointID = TargetActorComponent->SkeletonPoints.Add(NewPoint);
	}

	return TargetActorComponent->SkeletonPoints[NewPointID];
}

/*
 * Debug drawing
 */
void USlimeMoldSkeletonEditingTool::DrawDebugMouseInfo(const FInputDeviceRay& DevicePos, FColor Color)
{
	if (!bDrawDebugMouseInfo) return;

	FVector WorldPosToDraw;
	FindRayHit(DevicePos.WorldRay, WorldPosToDraw);
	DrawDebugPoint(TargetWorld, WorldPosToDraw, 10.0f, Color, false, 0.1f);
}

/*
 * Gizmo functionality
 */
void USlimeMoldSkeletonEditingTool::DestroyGizmo()
{
	UInteractiveGizmoManager* const GizmoManager = GetToolManager()->GetPairedGizmoManager();
	ensure(GizmoManager);
	GizmoManager->DestroyGizmo(TransformGizmo);

	TransformGizmo = nullptr;
}

void USlimeMoldSkeletonEditingTool::ShowGizmo(const FTransform& IntialTransform)
{
	PreviousGizmoTransform = IntialTransform;
	TransformGizmo->SetNewGizmoTransform(IntialTransform);
	TransformGizmo->SetVisibility(true);
}

void USlimeMoldSkeletonEditingTool::HideGizmo()
{
	TransformGizmo->SetVisibility(false);
}

void USlimeMoldSkeletonEditingTool::CreateGizmo()
{
	UInteractiveGizmoManager* const GizmoManager = GetToolManager()->GetPairedGizmoManager();
	ensure(GizmoManager);

	UTransformProxy* GizmoProxy = NewObject<UTransformProxy>(this);
	ensure(GizmoProxy);

	TransformGizmo = GizmoManager->Create3AxisTransformGizmo(this);
	ensure(TransformGizmo);

	TransformGizmo->SetActiveTarget(GizmoProxy, GetToolManager());

	GizmoProxy->OnTransformChanged.AddWeakLambda(this, [this](UTransformProxy*, FTransform NewTransform)
		{
			GizmoPositionDelta = NewTransform.GetLocation() - PreviousGizmoTransform.GetLocation();

			for (int32 PointID : SelectedPointIDs)
			{
				FSkeletonPoint& Point = TargetActorComponent->SkeletonPoints[PointID];
				Point.WorldPos += GizmoPositionDelta;
			}

			PreviousGizmoTransform = NewTransform;
		}
	);

	TransformGizmo->CurrentCoordinateSystem = EToolContextCoordinateSystem::World;
	TransformGizmo->SetVisibility(false);
}

/*
 * Rendering
 */
void USlimeMoldSkeletonEditingTool::Render(IToolsContextRenderAPI* RenderAPI)
{
	if (TargetActorComponent)
	{
		FPrimitiveDrawInterface* PDI = RenderAPI->GetPrimitiveDrawInterface();
		
		// Draw the ghost point
		if (bDrawGhostPoint) 
		{
			PDI->DrawPoint(GhostPoint.WorldPos, Properties->GhostPointColor, 10.0f, SDPG_Foreground);
		}

		// Draw the ghost lines
		if (bDrawGhostLines || (bConnectGhostAndSelectedPoints && bDrawGhostPoint))
		{
			for (int32 PointID : SelectedPointIDs)
			{
				FSkeletonPoint& Point = TargetActorComponent->SkeletonPoints[PointID];
				PDI->DrawLine(Point.WorldPos, GhostPoint.WorldPos,
					Properties->GhostLineColor, SDPG_Foreground, 1.0f);
			}
		}

		// Render the skeleton with debug view
		for (const FSkeletonLine& Line : TargetActorComponent->SkeletonLines)
		{
			TArray<FSkeletonLine> SelectedLines = GetSelectedLines();

			FVector Point1Pos = TargetActorComponent->SkeletonPoints[Line.Point1ID].WorldPos;
			FVector Point2Pos = TargetActorComponent->SkeletonPoints[Line.Point2ID].WorldPos;

			float AvgVeinness = (TargetActorComponent->SkeletonPoints[Line.Point1ID].Veinness + TargetActorComponent->SkeletonPoints[Line.Point2ID].Veinness) / 2.0f;

			FLinearColor LineColor = SelectedLines.Contains(Line) ? Properties->LineColorSelected :
				FMath::Lerp(Properties->LineColorMinVeinness, Properties->LineColorMaxVeinness, (1.0f - 1.0f / (FMath::Max<float>(AvgVeinness, 0.001f) + 1.0f)));

			PDI->DrawLine(Point1Pos, Point2Pos,	LineColor, SDPG_Foreground, 1.0f);
		}

		for (int32 i = 0; i < TargetActorComponent->SkeletonPoints.Num(); i++)
		{
			FSkeletonPoint& Point = TargetActorComponent->SkeletonPoints[i];
			FLinearColor PointColor = SelectedPointIDs.Contains(i) ? Properties->PointColorSelected :
				FMath::Lerp( Properties->PointColorMinClusterization, Properties->PointColorMaxClasterization, (1.0f - 1.0f / (FMath::Max<float>(Point.Clusterization, 0.001f) + 1.0f)));

			PDI->DrawPoint(Point.WorldPos, PointColor, Point.Thickness + 10.0f, SDPG_Foreground);
		}
	}
}

void USlimeMoldSkeletonEditingTool::OnTick(float DeltaTime)
{
	// Alway check if we are working with the right actor and the actor is valid
	if (USlimeMoldEditorFuncLib::SingleActorWithSkeletonComponentIsSelected())
	{
		if (TargetActorComponent != USlimeMoldEditorFuncLib::GetSkeletonComponentFromSelectedActor())
		{
			// Reload the tool
			GetToolManager()->DeactivateTool(EToolSide::Left, EToolShutdownType::Completed);
			GetToolManager()->ActivateTool(EToolSide::Left);
		}
	}
	else
	{
		GetToolManager()->DeactivateTool(EToolSide::Left, EToolShutdownType::Completed);
	}
}


#undef LOCTEXT_NAMESPACE
