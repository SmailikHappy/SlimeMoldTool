// Copyright Epic Games, Inc. All Rights Reserved.

#include "SlimeMoldMeshEditingTool.h"

#include "SlimeMoldMeshEditingCustomization.h"

// for raycast into World
#include "CollisionQueryParams.h"
#include "Engine/World.h"

// Custom static functions
#include "SlimeMoldEditorToolFunctionLibrary.h"


// localization namespace
#define LOCTEXT_NAMESPACE "USlimeMoldMeshEditingTool"

/*
 * Tool builder
 */

bool USlimeMoldMeshEditingToolBuilder::CanBuildTool(const FToolBuilderState& SceneState) const
{
	return USlimeMoldEditorFuncLib::SingleActorWithSkeletonComponentIsSelected();
}

UInteractiveTool* USlimeMoldMeshEditingToolBuilder::BuildTool(const FToolBuilderState& SceneState) const
{
	USlimeMoldMeshEditingTool* NewTool = NewObject<USlimeMoldMeshEditingTool>(SceneState.ToolManager);
	NewTool->SetWorld(SceneState.World);
	return NewTool;
}

/*
 * Tool logic
 */

void USlimeMoldMeshEditingTool::Setup()
{
	TargetActorComponent = USlimeMoldEditorFuncLib::GetSkeletonComponentFromSelectedActor();
	check(TargetActorComponent);

	UInteractiveTool::Setup();

	// Tool property set
	ToolProperties = NewObject<USlimeMoldMeshEditingToolProperties>(this);
	AddToolPropertySource(ToolProperties);

	ToolProperties->RestoreProperties(this, "MeshEditingToolProperties");

	// Register the custom detail customization
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	PropertyModule.RegisterCustomClassLayout(
		USlimeMoldMeshEditingToolProperties::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FSlimeMoldMeshEditingCustomization::MakeInstance)
	);

	// If there is a mesh property class assigned, create mesh property set of that class
	if (ToolProperties->MeshPropertyClass)
	{
		// Mesh property set
		MeshProperties = NewObject<USlimeMoldMeshPropertyBase>(this, ToolProperties->MeshPropertyClass);
		AddToolPropertySource(MeshProperties);

		MeshProperties->RestoreProperties(this, "MeshEditingMeshProperties");

		UE_LOG(LogTemp, Display, TEXT("Mesh properties have been assigned"));
	}

	UE_LOG(LogTemp, Display, TEXT("Mesh editing tool has been initialized"));
}

void USlimeMoldMeshEditingTool::SetWorld(UWorld* World)
{
	check(World);
	this->TargetWorld = World;
}

FInputRayHit USlimeMoldMeshEditingTool::CanBeginClickDragSequence(const FInputDeviceRay& PressPos)
{
	// we only start drag if press-down is on top of something we can raycast
	FVector Temp;
	FInputRayHit Result = FindRayHit(PressPos.WorldRay, Temp);
	return Result;
}

void USlimeMoldMeshEditingTool::OnClickPress(const FInputDeviceRay& PressPos)
{
	// cast ray into world to find hit position
	FVector RayStart = PressPos.WorldRay.Origin;
	FVector RayEnd = PressPos.WorldRay.PointAt(99999999.f);
	FCollisionObjectQueryParams QueryParams(FCollisionObjectQueryParams::AllObjects);
	FHitResult Result;

	// Trace the ray and select the hit actor
	if (TargetWorld->LineTraceSingleByObjectType(Result, RayStart, RayEnd, QueryParams))
	{
		GEditor->SelectActor(Result.GetActor(), true, true);
	}
}

void USlimeMoldMeshEditingTool::OnPropertyModified(UObject* PropertySet, FProperty* Property)
{
	if (!PropertySet) return;

	if (PropertySet == ToolProperties)
	{
		// Mesh property class has been assigned / changed
		if (Property->GetName() == "MeshPropertyClass")
		{
			// Reload the tool, so that MeshProperties with the chosen class would be generated in the panel 
			// For some reason we cannot add new properties to the tool in runtime
			// Huge thanks there is a workaround with a tool reloading
			ToolProperties->SaveProperties(this, "MeshEditingToolProperties");
			GetToolManager()->DeactivateTool(EToolSide::Left, EToolShutdownType::Completed);
			GetToolManager()->ActivateTool(EToolSide::Left);

			return;
		}

		if (Property->GetName() == "bGenerateMesh" || Property->GetName() == "bGenerateDebugInfo" ||
			Property->GetName() == "bAssignMaterials" || Property->GetName() == "bClearMesh")
		{
			if (!MeshProperties)
			{
				USlimeMoldEditorFuncLib::WarnUserDialog("Generation aborted", "Mesh properties are not set\nChoose property class");
				return;
			}
		}

		// Generate mesh button has been pressed
		if (Property->GetName() == "bGenerateMesh")
		{
			FEditorScriptExecutionGuard ScriptGuard;
			{
				UE_LOG(LogTemp, Display, TEXT("Generating mesh"));
				TargetActorComponent->OnGenerateMesh.Broadcast(MeshProperties);
			}
		}
		// Generate debug info button has been pressed
		else if (Property->GetName() == "bGenerateDebugInfo")
		{
			FEditorScriptExecutionGuard ScriptGuard;
			{
				UE_LOG(LogTemp, Display, TEXT("Generating debug info"));
				TargetActorComponent->OnGenerateDebugInfo.Broadcast(MeshProperties);
			}
		}
		// Assign materials button has been pressed
		else if (Property->GetName() == "bAssignMaterials")
		{
			FEditorScriptExecutionGuard ScriptGuard;
			{
				UE_LOG(LogTemp, Display, TEXT("Assigning materials"));
				TargetActorComponent->OnAssignMaterials.Broadcast(MeshProperties);
			}
		}
		// Clear mesh button has been pressed
		else if (Property->GetName() == "bClearMesh")
		{
			FEditorScriptExecutionGuard ScriptGuard;
			{
				UE_LOG(LogTemp, Display, TEXT("Clearing mesh"));
				TargetActorComponent->OnClearMesh.Broadcast(MeshProperties);
			}
		}
	}
}

void USlimeMoldMeshEditingTool::Shutdown(EToolShutdownType ShutdownType)
{
	if (MeshProperties)
	{
		MeshProperties->SaveProperties(this, "MeshEditingMeshProperties");
	}
}

void USlimeMoldMeshEditingTool::OnTick(float DeltaTime)
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

FInputRayHit USlimeMoldMeshEditingTool::FindRayHit(const FRay& WorldRay, FVector& HitPos)
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

#undef LOCTEXT_NAMESPACE
