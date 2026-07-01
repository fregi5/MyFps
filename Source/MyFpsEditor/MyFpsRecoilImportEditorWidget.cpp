// Copyright Epic Games, Inc. All Rights Reserved.

#include "MyFpsRecoilImportEditorWidget.h"

#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "MyFpsWeaponDefinition.h"

void UMyFpsRecoilImportEditorWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ImportButton)
	{
		ImportButton->OnClicked.RemoveDynamic(this, &UMyFpsRecoilImportEditorWidget::HandleImportClicked);
		ImportButton->OnClicked.AddDynamic(this, &UMyFpsRecoilImportEditorWidget::HandleImportClicked);
	}
}

bool UMyFpsRecoilImportEditorWidget::ImportRecoilPattern()
{
	if (!TargetWeaponDefinition)
	{
		SetStatusText(FText::FromString(TEXT("Import failed: TargetWeaponDefinition is not set.")));
		return false;
	}

	FString CsvFilePath;
	if (CsvPathTextBox)
	{
		CsvFilePath = CsvPathTextBox->GetText().ToString();
	}

	if (CsvFilePath.IsEmpty())
	{
		CsvFilePath = TargetWeaponDefinition->RecoilPatternCsvFile.FilePath;
	}

	if (CsvFilePath.IsEmpty())
	{
		SetStatusText(FText::FromString(TEXT("Import failed: CSV path is empty.")));
		return false;
	}

	const bool bImported = TargetWeaponDefinition->ImportRecoilPatternFromCsvPath(CsvFilePath);
	SetStatusText(bImported
		? FText::Format(
			FText::FromString(TEXT("Imported {0} recoil rows into {1}.")),
			FText::AsNumber(TargetWeaponDefinition->RecoilPatternShots.Num()),
			FText::FromString(TargetWeaponDefinition->GetName()))
		: FText::FromString(TEXT("Import failed. Check Output Log.")));
	return bImported;
}

void UMyFpsRecoilImportEditorWidget::SetTargetWeaponDefinition(UMyFpsWeaponDefinition* NewTargetWeaponDefinition)
{
	TargetWeaponDefinition = NewTargetWeaponDefinition;
}

void UMyFpsRecoilImportEditorWidget::SetCsvFilePath(const FString& NewCsvFilePath)
{
	if (CsvPathTextBox)
	{
		CsvPathTextBox->SetText(FText::FromString(NewCsvFilePath));
	}
}

void UMyFpsRecoilImportEditorWidget::HandleImportClicked()
{
	ImportRecoilPattern();
}

void UMyFpsRecoilImportEditorWidget::SetStatusText(const FText& NewStatusText)
{
	if (StatusTextBlock)
	{
		StatusTextBlock->SetText(NewStatusText);
	}
}
