#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MyFpsScoreboardWidget.generated.h"

class UTextBlock;
class UBorder;
class UVerticalBox;

USTRUCT(BlueprintType)
struct FMyFpsScoreboardEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Scoreboard")
	int32 Rank = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Scoreboard")
	FString PlayerName;

	UPROPERTY(BlueprintReadOnly, Category = "Scoreboard")
	float Score = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Scoreboard")
	int32 Kills = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Scoreboard")
	int32 Deaths = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Scoreboard")
	bool bIsLocalPlayer = false;
};

UCLASS()
class MYFPS_API UMyFpsScoreboardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
	UFUNCTION(BlueprintCallable, Category = "Scoreboard")
	void RefreshScoreboard();

protected:
	float ScoreboardRefreshTimer = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scoreboard")
	float ScoreboardRefreshInterval = 0.2f;
	UPROPERTY(BlueprintReadOnly, Category = "Scoreboard")
	TArray<FMyFpsScoreboardEntry> CachedEntries;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scoreboard")
	FText TitleText = FText::FromString(TEXT("Scoreboard"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scoreboard")
	FText RankColumnText = FText::FromString(TEXT("#"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scoreboard")
	FText PlayerColumnText = FText::FromString(TEXT("Player"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scoreboard")
	FText ScoreColumnText = FText::FromString(TEXT("Score"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scoreboard")
	FText KillsColumnText = FText::FromString(TEXT("K"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scoreboard")
	FText DeathsColumnText = FText::FromString(TEXT("D"));

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UBorder> RootBorder = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TitleTextBlock = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> RankHeaderTextBlock = nullptr;


	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> PlayerHeaderTextBlock = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ScoreHeaderTextBlock = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> KillsHeaderTextBlock = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DeathsHeaderTextBlock = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> EntryListBox = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> PickupTextBlock = nullptr;

	UFUNCTION(BlueprintImplementableEvent, Category = "Scoreboard")
	void OnScoreboardUpdated();
};
