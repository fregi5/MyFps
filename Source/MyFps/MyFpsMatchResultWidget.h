#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MyFpsMatchResultWidget.generated.h"

class UButton;
class UTextBlock;
class AMyFpsGameState;

UCLASS()
class MYFPS_API UMyFpsMatchResultWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Match")
	void RefreshFromGameState();

	UFUNCTION(BlueprintCallable, Category = "Match")
	void ShowExplicitResult(const FString& WinnerName);

	UFUNCTION(BlueprintCallable, Category = "Match")
	void RefreshReadyDisplay();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Match")
	FText VictorySuffixText = FText::FromString(TEXT("获胜!"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Match")
	FText ReadyButtonText = FText::FromString(TEXT("准备下一局"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Match")
	FText ReadyPromptText = FText::FromString(TEXT("点击按钮准备下一局"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Match")
	FText ReadyConfirmedText = FText::FromString(TEXT("已准备"));

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> WinnerTextBlock = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ReadyStatusTextBlock = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> ReadyButton = nullptr;

	UFUNCTION(BlueprintImplementableEvent, Category = "Match")
	void OnMatchResultUpdated();

private:
	UFUNCTION()
	void HandleReadyClicked();

	UFUNCTION()
	void HandleReadyHovered();

	UFUNCTION()
	void HandleReadyPressed();

	void BindToGameState();
	void UnbindFromGameState();
	void HandleMatchStateChanged();
	void RefreshWidget();
	void RefreshReadyStateOnly();

	TWeakObjectPtr<AMyFpsGameState> CachedGameState;
	bool bLocalReadyRequested = false;
};
