// Copyright Epic Games, Inc. All Rights Reserved.

#include "SlimeMoldEditorToolMode.h"
#include "SlimeMoldEditorToolModeToolkit.h"
#include "EdModeInteractiveToolsContext.h"
#include "InteractiveToolManager.h"
#include "SlimeMoldEditorToolModeCommands.h"


//////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////// 
// AddYourTool Step 1 - include the header file for your Tools here
//////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////// 
#include "../Tools/MeshEditing/SlimeMoldMeshEditingTool.h"
#include "../Tools/SkeletonEditing/SlimeMoldSkeletonEditingTool.h"

// step 2: register a ToolBuilder in FSlimeMoldEditorToolEditorMode::Enter() below


#define LOCTEXT_NAMESPACE "SlimeMoldEditorToolEditorMode"

const FEditorModeID USlimeMoldEditorToolEditorMode::EM_SlimeMoldEditorToolEditorModeId = TEXT("EM_SlimeMoldEditorToolEditorMode");

FString USlimeMoldEditorToolEditorMode::SkeletonEditingToolName = TEXT("SlimeMoldEditorTool_SkeletonEditingTool");
FString USlimeMoldEditorToolEditorMode::MeshEditingToolName = TEXT("SlimeMoldEditorTool_MeshEditingTool");


USlimeMoldEditorToolEditorMode::USlimeMoldEditorToolEditorMode()
{
	FModuleManager::Get().LoadModule("EditorStyle");

	// appearance and icon in the editing mode ribbon can be customized here
	Info = FEditorModeInfo(USlimeMoldEditorToolEditorMode::EM_SlimeMoldEditorToolEditorModeId,
		LOCTEXT("ModeName", "SlimeMoldEditorTool"),
		FSlateIcon(),
		true);
}


USlimeMoldEditorToolEditorMode::~USlimeMoldEditorToolEditorMode()
{
}


void USlimeMoldEditorToolEditorMode::ActorSelectionChangeNotify()
{
}

void USlimeMoldEditorToolEditorMode::Enter()
{
	UEdMode::Enter();

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	// AddYourTool Step 2 - register the ToolBuilders for your Tools here.
	// The string name you pass to the ToolManager is used to select/activate your ToolBuilder later.
	//////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////// 
	const FSlimeMoldEditorToolModeCommands& SampleToolCommands = FSlimeMoldEditorToolModeCommands::Get();

	RegisterTool(SampleToolCommands.SkeletonEditingTool, SkeletonEditingToolName, NewObject<USlimeMoldSkeletonEditingToolBuilder>(this));
	RegisterTool(SampleToolCommands.MeshEditingTool, MeshEditingToolName, NewObject<USlimeMoldMeshEditingToolBuilder>(this));
}

void USlimeMoldEditorToolEditorMode::CreateToolkit()
{
	Toolkit = MakeShareable(new FSlimeMoldEditorToolEditorModeToolkit);
}

TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> USlimeMoldEditorToolEditorMode::GetModeCommands() const
{
	return FSlimeMoldEditorToolModeCommands::Get().GetCommands();
}

#undef LOCTEXT_NAMESPACE
