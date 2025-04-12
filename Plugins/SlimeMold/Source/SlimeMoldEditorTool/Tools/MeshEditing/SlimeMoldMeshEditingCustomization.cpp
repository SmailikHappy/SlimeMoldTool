// Copyright Epic Games, Inc. All Rights Reserved.


#include "SlimeMoldMeshEditingCustomization.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "SlimeMoldMeshEditingTool.h"
#include "Engine/Selection.h"

#define LOCTEXT_NAMESPACE "MeshEditingCustomization"

TSharedRef<IDetailCustomization> FSlimeMoldMeshEditingCustomization::MakeInstance()
{
	return MakeShareable(new FSlimeMoldMeshEditingCustomization);
}

void FSlimeMoldMeshEditingCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	// Create a new category named options
	IDetailCategoryBuilder& MeshButtonsCategory = DetailBuilder.EditCategory("Buttons");

	TSharedRef<IPropertyHandle> GenerateMeshButton = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(USlimeMoldMeshEditingToolProperties, bGenerateMesh));
	TSharedRef<IPropertyHandle> GenerateDebugInfoButton = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(USlimeMoldMeshEditingToolProperties, bGenerateDebugInfo));
	TSharedRef<IPropertyHandle> AssignMaterialsButton = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(USlimeMoldMeshEditingToolProperties, bAssignMaterials));
	TSharedRef<IPropertyHandle> ClearMeshButton = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(USlimeMoldMeshEditingToolProperties, bClearMesh));

	// Hide the properties themselves and use them for the workaround
	DetailBuilder.HideProperty(GenerateMeshButton);
	DetailBuilder.HideProperty(GenerateDebugInfoButton);
	DetailBuilder.HideProperty(AssignMaterialsButton);
	DetailBuilder.HideProperty(ClearMeshButton);

	// Add generate mesh button
	MeshButtonsCategory.AddCustomRow(LOCTEXT("GenerateMeshButtonRow", "Generate mesh button"))
		.ValueContent()
		[
			SNew(SButton)
				.Text(FText::FromString("Generate mesh"))
				.OnClicked(FOnClicked::CreateLambda([GenerateMeshButton]()
					{
						// Workaround to update the property value on button click, and trigger button functionality in the tool
						bool bValue = false;
						GenerateMeshButton->GetValue(bValue);
						GenerateMeshButton->SetValue(!bValue);
						return FReply::Handled();
					}))
		];

	// Add generate debug info button
	MeshButtonsCategory.AddCustomRow(LOCTEXT("GenerateDebugInfoButtonRow", "Generate debug info button"))
		.ValueContent()
		[
			SNew(SButton)
				.Text(FText::FromString("Generate debug info"))
				.OnClicked(FOnClicked::CreateLambda([GenerateDebugInfoButton]()
					{
						// Workaround to update the property value on button click, and trigger button functionality in the tool
						bool bValue = false;
						GenerateDebugInfoButton->GetValue(bValue);
						GenerateDebugInfoButton->SetValue(!bValue);
						return FReply::Handled();
					}))
		];

	// Add assign materials button
	MeshButtonsCategory.AddCustomRow(LOCTEXT("AssignMaterialsButtonRow", "Assign materials button"))
		.ValueContent()
		[
			SNew(SButton)
				.Text(FText::FromString("Assign materials"))
				.OnClicked(FOnClicked::CreateLambda([AssignMaterialsButton]()
					{
						// Workaround to update the property value on button click, and trigger button functionality in the tool
						bool bValue = false;
						AssignMaterialsButton->GetValue(bValue);
						AssignMaterialsButton->SetValue(!bValue);
						return FReply::Handled();
					}))
		];

	// Add clear mesh button
	MeshButtonsCategory.AddCustomRow(LOCTEXT("ClearMeshButtonRow", "Clear mesh button"))
		.ValueContent()
		[
			SNew(SButton)
				.Text(FText::FromString("Clear mesh"))
				.OnClicked(FOnClicked::CreateLambda([ClearMeshButton]()
					{
						// Workaround to update the property value on button click, and trigger button functionality in the tool
						bool bValue = false;
						ClearMeshButton->GetValue(bValue);
						ClearMeshButton->SetValue(!bValue);
						return FReply::Handled();
					}))
		];
}

#undef LOCTEXT_NAMESPACE