#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MyFpsPickupPromptWidget.generated.h"

class UTextBlock;

UCLASS()
class MYFPS_API UMyFpsPickupPromptWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category = "Pickup")
	void SetWeaponName(const FText& InWeaponName);

	UFUNCTION(BlueprintCallable, Category = "Pickup")
	void SetPromptText(const FText& InPromptText);

	void SetPickupText(const FText& WeaponName);
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup")
	FText PickupPrefix = FText::FromString(TEXT("Press E to pick up"));

	UPROPERTY(BlueprintReadOnly, Category = "Pickup")
	FText CachedWeaponName;

	UPROPERTY(BlueprintReadOnly, Category = "Pickup")
	FText CachedPromptText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> PickupTextBlock = nullptr;

	UFUNCTION(BlueprintImplementableEvent, Category = "Pickup")
	void OnPickupPromptUpdated();

private:
	void RefreshPromptText();
};
