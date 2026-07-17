// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MyFpsNetworkDebugWidget.generated.h"

class UButton;
class UHorizontalBox;
class UTextBlock;

UCLASS()
class MYFPS_API UMyFpsNetworkDebugWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Network Debug")
	void RefreshNetworkDebugText();

protected:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> NetworkStatusTextBlock = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> NormalNetworkButton = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> AverageNetworkButton = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> BadNetworkButton = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> ClearNetworkButton = nullptr;

private:
	void BuildDefaultLayout();
	UButton* AddProfileButton(UHorizontalBox* RootBox, const FText& Label);
	FString GetNetModeText() const;
	FString GetLocalRoleText() const;
	float GetLocalPingMilliseconds() const;
	void ApplyNetworkEmulation(int32 PacketLag, int32 PacketLagVariance, int32 PacketLoss);

	UFUNCTION()
	void HandleNormalNetworkClicked();

	UFUNCTION()
	void HandleAverageNetworkClicked();

	UFUNCTION()
	void HandleBadNetworkClicked();

	UFUNCTION()
	void HandleClearNetworkClicked();

	float TimeSinceLastRefresh = 0.0f;
};
