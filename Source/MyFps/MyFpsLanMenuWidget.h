#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MyFpsLanMenuWidget.generated.h"

class UButton;
class AMyFpsGameState;

UCLASS()
class MYFPS_API UMyFpsLanMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintCallable, Category = "LAN")
	void RefreshControlState();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LAN")
	FText StartButtonText = FText::FromString(TEXT("Start Game"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LAN")
	FText EndButtonText = FText::FromString(TEXT("End Game"));

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "LAN")
	TObjectPtr<UButton> StartGameButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "LAN")
	TObjectPtr<UButton> EndGameButton = nullptr;

private:
	UFUNCTION()
	void HandleStartGameClicked();

	UFUNCTION()
	void HandleEndGameClicked();

	void BuildDefaultLayout();
	void HandleMatchStateChanged();

	TWeakObjectPtr<AMyFpsGameState> CachedGameState;
};
