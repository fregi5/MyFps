#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MyFpsDeathWidget.generated.h"

class UTextBlock;

UCLASS()
class MYFPS_API UMyFpsDeathWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category = "Death")
	void StartRespawnCountdown(float DelaySeconds);

	UFUNCTION(BlueprintCallable, Category = "Death")
	void CancelRespawnCountdown();

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death")
	FText DeathMessage = FText::FromString(TEXT("You Died"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death")
	FText RespawnMessage = FText::FromString(TEXT("Press R to Respawn"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death")
	FText RespawnCountdownFormat = FText::FromString(TEXT("{0} 秒后自动重生（按 R 立即重生）"));

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DeathTextBlock = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> RespawnTextBlock = nullptr;

private:
	void RefreshRespawnCountdown();

	float RespawnEndTime = 0.0f;
	bool bRespawnCountdownActive = false;
};
