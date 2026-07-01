// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "MyFpsRecoilImportEditorWidget.generated.h"

class UButton;
class UEditableTextBox;
class UMyFpsWeaponDefinition;
class UTextBlock;

UCLASS(BlueprintType, Blueprintable)
class MYFPSEDITOR_API UMyFpsRecoilImportEditorWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil Import")
	TObjectPtr<UMyFpsWeaponDefinition> TargetWeaponDefinition = nullptr;

	UFUNCTION(BlueprintCallable, Category = "Recoil Import")
	bool ImportRecoilPattern();

	UFUNCTION(BlueprintCallable, Category = "Recoil Import")
	void SetTargetWeaponDefinition(UMyFpsWeaponDefinition* NewTargetWeaponDefinition);

	UFUNCTION(BlueprintCallable, Category = "Recoil Import")
	void SetCsvFilePath(const FString& NewCsvFilePath);

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UEditableTextBox> CsvPathTextBox = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> ImportButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> StatusTextBlock = nullptr;

private:
	UFUNCTION()
	void HandleImportClicked();

	void SetStatusText(const FText& NewStatusText);
};
