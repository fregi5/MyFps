#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MyFpsScoreWidget.generated.h"

class UProgressBar;
class UScrollBox;
class UTextBlock;

UCLASS()
class MYFPS_API UMyFpsScoreWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION(BlueprintPure, Category = "Score")
	float GetCurrentScore() const;

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetCurrentHealth() const;

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetMaxHealth() const;

	UFUNCTION(BlueprintPure, Category = "Interaction")
	bool HasAvailablePickup() const;

	UFUNCTION(BlueprintPure, Category = "Interaction")
	bool HasWeaponEquipped() const;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	int32 GetCurrentAmmoInClip() const;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	int32 GetCurrentReserveAmmo() const;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool IsCurrentWeaponReloading() const;

	UFUNCTION(BlueprintPure, Category = "Match")
	int32 GetTargetScore() const;

	UFUNCTION(BlueprintPure, Category = "Match")
	bool IsMatchFinished() const;

	UFUNCTION(BlueprintPure, Category = "Match")
	FString GetWinnerName() const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Score")
	FText ScorePrefix = FText::FromString(TEXT("得分: "));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Score")
	FText TargetScorePrefix = FText::FromString(TEXT("目标分数: "));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health")
	FText HealthPrefix = FText::FromString(TEXT("生命值: "));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FText AmmoPrefix = FText::FromString(TEXT("子弹/备弹: "));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FText ReloadingText = FText::FromString(TEXT("Reloading"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Match")
	FText VictorySuffixText = FText::FromString(TEXT("获胜!"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction")
	FText PickupPromptText = FText::FromString(TEXT("Press E to pick up"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction")
	FText SwapPromptText = FText::FromString(TEXT("Press E to swap weapon"));

	UPROPERTY(BlueprintReadOnly, Category = "HUD")
	float CachedScore = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "HUD")
	float CachedHealth = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "HUD")
	float CachedMaxHealth = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "HUD")
	float CachedHealthPercent = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "HUD")
	int32 CachedAmmoInClip = 0;

	UPROPERTY(BlueprintReadOnly, Category = "HUD")
	int32 CachedReserveAmmo = 0;

	UPROPERTY(BlueprintReadOnly, Category = "HUD")
	bool bCachedIsReloading = false;

	UPROPERTY(BlueprintReadOnly, Category = "HUD")
	bool bCachedShowAmmo = false;

	UPROPERTY(BlueprintReadOnly, Category = "HUD")
	FText CachedAmmoText;

	UPROPERTY(BlueprintReadOnly, Category = "HUD")
	int32 CachedTargetScore = 0;

	UPROPERTY(BlueprintReadOnly, Category = "HUD")
	bool bCachedMatchFinished = false;

	UPROPERTY(BlueprintReadOnly, Category = "HUD")
	FText CachedTargetScoreText;

	UPROPERTY(BlueprintReadOnly, Category = "HUD")
	FText CachedMatchStatusText;

	UPROPERTY(BlueprintReadOnly, Category = "HUD")
	bool bCachedShowPickupPrompt = false;

	UPROPERTY(BlueprintReadOnly, Category = "HUD")
	FText CachedPickupPromptText;

	UPROPERTY(BlueprintReadOnly, Category = "HUD")
	TArray<FText> CachedKillFeedTexts;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ScoreTextBlock = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> HealthTextBlock = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> HealthProgressBar = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> AmmoTextBlock = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TargetScoreTextBlock = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> MatchStatusTextBlock = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> PickupPromptTextBlock = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UScrollBox> KillFeedScrollBox = nullptr;

	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void OnHudDataUpdated();

	void BuildDefaultLayout();
	void RefreshHud();
	void RefreshKillFeed();
};
