// Copyright Epic Games, Inc. All Rights Reserved.

#include "SlimeMoldEditorToolModeCommands.h"
#include "SlimeMoldEditorToolMode.h"
#include "EditorStyleSet.h"

#define LOCTEXT_NAMESPACE "FSlimeMoldEditorToolModeCommands"

FSlimeMoldEditorToolModeCommands::FSlimeMoldEditorToolModeCommands()
	: TCommands<FSlimeMoldEditorToolModeCommands>(
		"SlimeMoldEditorToolMode", // Context name for fast lookup
		NSLOCTEXT("SlimeMoldEditorToolMode", "SlimeMoldEditorToolModeCommands", "SlimeMoldEditor Tool Mode"), // Localized context name for displaying
		NAME_None, // Parent context name. 
		FAppStyle::GetAppStyleSetName() // Icon Style Set
	)
{
}

void FSlimeMoldEditorToolModeCommands::RegisterCommands()
{
	TArray<TSharedPtr<FUICommandInfo>>& ToolCommands = Commands.FindOrAdd(NAME_Default);

	UI_COMMAND(SkeletonEditingTool, "Skeleton", "This tool manages the skeleton of the goop", EUserInterfaceActionType::ToggleButton, FInputChord());
	ToolCommands.Add(SkeletonEditingTool);

	UI_COMMAND(MeshEditingTool, "Mesh", "This tool operates on mesh generation", EUserInterfaceActionType::ToggleButton, FInputChord());
	ToolCommands.Add(MeshEditingTool);
}

TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> FSlimeMoldEditorToolModeCommands::GetCommands()
{
	return FSlimeMoldEditorToolModeCommands::Get().Commands;
}

#undef LOCTEXT_NAMESPACE