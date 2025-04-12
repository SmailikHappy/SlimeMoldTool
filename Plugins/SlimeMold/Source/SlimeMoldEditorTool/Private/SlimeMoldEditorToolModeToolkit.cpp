// Copyright Epic Games, Inc. All Rights Reserved.

#include "SlimeMoldEditorToolModeToolkit.h"
#include "SlimeMoldEditorToolMode.h"
#include "Engine/Selection.h"

#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "EditorModeManager.h"

#define LOCTEXT_NAMESPACE "SlimeMoldEditorToolEditorModeToolkit"

FSlimeMoldEditorToolEditorModeToolkit::FSlimeMoldEditorToolEditorModeToolkit()
{
}

void FSlimeMoldEditorToolEditorModeToolkit::Init(const TSharedPtr<IToolkitHost>& InitToolkitHost, TWeakObjectPtr<UEdMode> InOwningMode)
{
	FModeToolkit::Init(InitToolkitHost, InOwningMode);
}

void FSlimeMoldEditorToolEditorModeToolkit::GetToolPaletteNames(TArray<FName>& PaletteNames) const
{
	PaletteNames.Add(NAME_Default);
}


FName FSlimeMoldEditorToolEditorModeToolkit::GetToolkitFName() const
{
	return FName("SlimeMoldEditorToolEditorMode");
}

FText FSlimeMoldEditorToolEditorModeToolkit::GetBaseToolkitName() const
{
	return LOCTEXT("DisplayName", "SlimeMoldEditorToolEditorMode Toolkit");
}

#undef LOCTEXT_NAMESPACE
