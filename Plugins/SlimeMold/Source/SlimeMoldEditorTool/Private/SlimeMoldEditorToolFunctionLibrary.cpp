// Copyright Epic Games, Inc. All Rights Reserved.

#include "SlimeMoldEditorToolFunctionLibrary.h"
#include "Selection.h"


bool USlimeMoldEditorFuncLib::SingleActorWithSkeletonComponentIsSelected()
{
	TArray<UObject*> SelectedObjects;
	GEditor->GetSelectedActors()->GetSelectedObjects(SelectedObjects);

	if (SelectedObjects.Num() != 1)
	{
		return false;
	}

	AActor* SelectedActor = Cast<AActor>(SelectedObjects[0]);

	if (!SelectedActor)
	{
		return false;
	}

	if (SelectedActor->GetComponentByClass<USlimeMoldSkeletonComponent>() == nullptr)
	{
		return false;
	}

	return true;
}

USlimeMoldSkeletonComponent* USlimeMoldEditorFuncLib::GetSkeletonComponentFromSelectedActor()
{
	if (SingleActorWithSkeletonComponentIsSelected())
	{
		TArray<UObject*> SelectedObjects;
		GEditor->GetSelectedActors()->GetSelectedObjects(SelectedObjects);
		return Cast<AActor>(SelectedObjects[0])->GetComponentByClass<USlimeMoldSkeletonComponent>();
	}

	return nullptr;
}

void USlimeMoldEditorFuncLib::WarnUserDialog(const FString& Title, const FString& Message)
{
	FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Message), FText::FromString(Title));
}
