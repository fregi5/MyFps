#include "MyFpsDeathWidget.h"
#include "Components/TextBlock.h"

void UMyFpsDeathWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (DeathTextBlock)
	{
		DeathTextBlock->SetText(DeathMessage);
	}

	if (RespawnTextBlock)
	{
		RespawnTextBlock->SetText(RespawnMessage);
	}
}

void UMyFpsDeathWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bRespawnCountdownActive)
	{
		RefreshRespawnCountdown();
	}
}

void UMyFpsDeathWidget::StartRespawnCountdown(float DelaySeconds)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	RespawnEndTime = World->GetTimeSeconds() + FMath::Max(0.0f, DelaySeconds);
	bRespawnCountdownActive = true;
	RefreshRespawnCountdown();
}

void UMyFpsDeathWidget::CancelRespawnCountdown()
{
	bRespawnCountdownActive = false;
	if (RespawnTextBlock)
	{
		RespawnTextBlock->SetText(RespawnMessage);
	}
}

void UMyFpsDeathWidget::RefreshRespawnCountdown()
{
	UWorld* World = GetWorld();
	if (!World || !RespawnTextBlock)
	{
		return;
	}

	const int32 RemainingSeconds = FMath::Max(0, FMath::CeilToInt(RespawnEndTime - World->GetTimeSeconds()));
	RespawnTextBlock->SetText(FText::Format(RespawnCountdownFormat, FText::AsNumber(RemainingSeconds)));
}
