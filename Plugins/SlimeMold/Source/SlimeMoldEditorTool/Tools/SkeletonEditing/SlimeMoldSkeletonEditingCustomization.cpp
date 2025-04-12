// Copyright Epic Games, Inc. All Rights Reserved.


#include "SlimeMoldSkeletonEditingCustomization.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "SlimeMoldSkeletonEditingTool.h"
#include "Engine/Selection.h"

#define LOCTEXT_NAMESPACE "SkeletonEditingCustomization"

TSharedRef<IDetailCustomization> FSlimeMoldSkeletonEditingCustomization::MakeInstance()
{
	return MakeShareable(new FSlimeMoldSkeletonEditingCustomization);
}

void FSlimeMoldSkeletonEditingCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	// Create a new category named options
	IDetailCategoryBuilder& SkeletonButtonsCategory = DetailBuilder.EditCategory("Buttons");

	// Declare properties
	TSharedRef<IPropertyHandle> DeletePointsButton		= DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(USlimeMoldSkeletonEditingToolProperties, bDeletePoints));
	TSharedRef<IPropertyHandle> DisconnectPointsButton	= DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(USlimeMoldSkeletonEditingToolProperties, bDisconnectPoints));
	TSharedRef<IPropertyHandle> SplitLineButton			= DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(USlimeMoldSkeletonEditingToolProperties, bSplitLine));

	// Hide the properties themselves and use them for the workaround
	DetailBuilder.HideProperty(DisconnectPointsButton);
	DetailBuilder.HideProperty(SplitLineButton);
	DetailBuilder.HideProperty(DeletePointsButton);

#pragma region Button-delete
	// Add delete points button
	SkeletonButtonsCategory.AddCustomRow(LOCTEXT("DeletePointsButtonRow", "Delete points button"))
		.ValueContent()
		[
			SNew(SButton)
				.Text(FText::FromString("Delete"))
				.OnClicked(FOnClicked::CreateLambda([DeletePointsButton]()
				{
					// Workaround to update the property value on button click, and trigger button functionality in the tool
					bool bValue = false;
					DeletePointsButton->GetValue(bValue);
					DeletePointsButton->SetValue(!bValue);

					return FReply::Handled();
				}))
		];
#pragma endregion Button-delete

#pragma region Button-disconnect
	// Add disconnect points button
	SkeletonButtonsCategory.AddCustomRow(LOCTEXT("DisconnectPointsButtonRow", "Disconnect points button"))
		.ValueContent()
		[
			SNew(SButton)
				.Text(FText::FromString("Disconnect"))
				.OnClicked(FOnClicked::CreateLambda([DisconnectPointsButton]()
					{
						// Workaround to update the property value on button click, and trigger button functionality in the tool
						bool bValue = false;
						DisconnectPointsButton->GetValue(bValue);
						DisconnectPointsButton->SetValue(!bValue);

						return FReply::Handled();
					}))
		];
#pragma endregion Button-disconnect

#pragma region Button-split
	// Add split line button
	SkeletonButtonsCategory.AddCustomRow(LOCTEXT("SplitLineButtonRow", "Split line button"))
		.ValueContent()
		[
			SNew(SButton)
				.Text(FText::FromString("Split line"))
				.OnClicked(FOnClicked::CreateLambda([SplitLineButton]()
					{
						// Workaround to update the property value on button click, and trigger button functionality in the tool
						bool bValue = false;
						SplitLineButton->GetValue(bValue);
						SplitLineButton->SetValue(!bValue);

						return FReply::Handled();
					}))
		];
#pragma endregion Button-split
}

#undef LOCTEXT_NAMESPACE