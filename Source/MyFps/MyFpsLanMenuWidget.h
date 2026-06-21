#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MyFpsLanMenuWidget.generated.h"

class UButton;
class UEditableTextBox;
class UTextBlock;

UCLASS()
class MYFPS_API UMyFpsLanMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintCallable, Category = "LAN")
	void SetStatusMessage(const FText& InStatusMessage);

	UFUNCTION(BlueprintCallable, Category = "LAN")
	void FocusAddressInput();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LAN")
	FText TitleText = FText::FromString(TEXT("LAN Multiplayer"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LAN")
	FText HostButtonText = FText::FromString(TEXT("Host"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LAN")
	FText JoinButtonText = FText::FromString(TEXT("Join"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LAN")
	FText StartButtonText = FText::FromString(TEXT("Start Game"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LAN")
	FText CloseButtonText = FText::FromString(TEXT("Close"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LAN")
	FText AddressHintText = FText::FromString(TEXT("127.0.0.1"));

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "LAN")
	TObjectPtr<UTextBlock> TitleTextBlock = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "LAN")
	TObjectPtr<UEditableTextBox> AddressTextBox = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "LAN")
	TObjectPtr<UTextBlock> LocalIpTextBlock = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "LAN")
	TObjectPtr<UButton> HostButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "LAN")
	TObjectPtr<UButton> JoinButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "LAN")
	TObjectPtr<UButton> CloseButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "LAN")
	TObjectPtr<UButton> StartButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "LAN")
	TObjectPtr<UTextBlock> StatusTextBlock = nullptr;

private:
	UFUNCTION()
	void HandleHostClicked();

	UFUNCTION()
	void HandleJoinClicked();

	UFUNCTION()
	void HandleStartClicked();

	UFUNCTION()
	void HandleCloseClicked();

	void BuildDefaultLayout();
	void RefreshLanInfo();
};
