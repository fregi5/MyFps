#include "MyFpsScoreWidget.h"
#include "Components/ProgressBar.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "GameFramework/PlayerController.h"
#include "MyFpsCharacter.h"
#include "MyFpsGameState.h"
#include "MyFpsPlayerState.h"
#include "Blueprint/WidgetTree.h"

void UMyFpsScoreWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BuildDefaultLayout();
	RefreshHud();
}

void UMyFpsScoreWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	RefreshHud();
}

float UMyFpsScoreWidget::GetCurrentScore() const
{
	APlayerController* PlayerController = GetOwningPlayer();
	if (!PlayerController)
	{
		return 0.0f;
	}

	const AMyFpsPlayerState* PlayerState = PlayerController->GetPlayerState<AMyFpsPlayerState>();
	return PlayerState ? PlayerState->GetCurrentScore() : 0.0f;
}

float UMyFpsScoreWidget::GetCurrentHealth() const
{
	APlayerController* PlayerController = GetOwningPlayer();
	if (!PlayerController)
	{
		return 0.0f;
	}

	const AMyFpsCharacter* PlayerCharacter = Cast<AMyFpsCharacter>(PlayerController->GetPawn());
	return PlayerCharacter ? PlayerCharacter->GetHealth() : 0.0f;
}

float UMyFpsScoreWidget::GetMaxHealth() const
{
	APlayerController* PlayerController = GetOwningPlayer();
	if (!PlayerController)
	{
		return 0.0f;
	}

	const AMyFpsCharacter* PlayerCharacter = Cast<AMyFpsCharacter>(PlayerController->GetPawn());
	return PlayerCharacter ? PlayerCharacter->GetMaxHealth() : 0.0f;
}

bool UMyFpsScoreWidget::HasAvailablePickup() const
{
	APlayerController* PlayerController = GetOwningPlayer();
	if (!PlayerController)
	{
		return false;
	}

	const AMyFpsCharacter* PlayerCharacter = Cast<AMyFpsCharacter>(PlayerController->GetPawn());
	return PlayerCharacter ? PlayerCharacter->HasAvailablePickup() : false;
}

bool UMyFpsScoreWidget::HasWeaponEquipped() const
{
	APlayerController* PlayerController = GetOwningPlayer();
	if (!PlayerController)
	{
		return false;
	}

	const AMyFpsCharacter* PlayerCharacter = Cast<AMyFpsCharacter>(PlayerController->GetPawn());
	return PlayerCharacter ? PlayerCharacter->HasWeaponEquipped() : false;
}

int32 UMyFpsScoreWidget::GetCurrentAmmoInClip() const
{
	APlayerController* PlayerController = GetOwningPlayer();
	if (!PlayerController)
	{
		return 0;
	}

	const AMyFpsCharacter* PlayerCharacter = Cast<AMyFpsCharacter>(PlayerController->GetPawn());
	return PlayerCharacter ? PlayerCharacter->GetCurrentAmmoInClip() : 0;
}

int32 UMyFpsScoreWidget::GetCurrentReserveAmmo() const
{
	APlayerController* PlayerController = GetOwningPlayer();
	if (!PlayerController)
	{
		return 0;
	}

	const AMyFpsCharacter* PlayerCharacter = Cast<AMyFpsCharacter>(PlayerController->GetPawn());
	return PlayerCharacter ? PlayerCharacter->GetCurrentReserveAmmo() : 0;
}

bool UMyFpsScoreWidget::IsCurrentWeaponReloading() const
{
	APlayerController* PlayerController = GetOwningPlayer();
	if (!PlayerController)
	{
		return false;
	}

	const AMyFpsCharacter* PlayerCharacter = Cast<AMyFpsCharacter>(PlayerController->GetPawn());
	return PlayerCharacter ? PlayerCharacter->IsCurrentWeaponReloading() : false;
}

int32 UMyFpsScoreWidget::GetTargetScore() const
{
	APlayerController* PlayerController = GetOwningPlayer();
	if (!PlayerController || !PlayerController->GetWorld())
	{
		return 0;
	}

	const AMyFpsGameState* GameState = PlayerController->GetWorld()->GetGameState<AMyFpsGameState>();
	return GameState ? GameState->GetTargetScore() : 0;
}

bool UMyFpsScoreWidget::IsMatchFinished() const
{
	APlayerController* PlayerController = GetOwningPlayer();
	if (!PlayerController || !PlayerController->GetWorld())
	{
		return false;
	}

	const AMyFpsGameState* GameState = PlayerController->GetWorld()->GetGameState<AMyFpsGameState>();
	return GameState ? GameState->IsMatchFinished() : false;
}

FString UMyFpsScoreWidget::GetWinnerName() const
{
	APlayerController* PlayerController = GetOwningPlayer();
	if (!PlayerController || !PlayerController->GetWorld())
	{
		return FString();
	}

	const AMyFpsGameState* GameState = PlayerController->GetWorld()->GetGameState<AMyFpsGameState>();
	return GameState ? GameState->GetWinnerName() : FString();
}

void UMyFpsScoreWidget::BuildDefaultLayout()
{
	if (!WidgetTree || WidgetTree->RootWidget)
	{
		return;
	}

	UVerticalBox* RootPanel = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("HudRootPanel"));
	if (!RootPanel)
	{
		return;
	}

	WidgetTree->RootWidget = RootPanel;

	ScoreTextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ScoreTextBlock"));
	HealthTextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("HealthTextBlock"));
	HealthProgressBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("HealthProgressBar"));
	AmmoTextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("AmmoTextBlock"));
	TargetScoreTextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TargetScoreTextBlock"));
	MatchStatusTextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("MatchStatusTextBlock"));
	PickupPromptTextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("PickupPromptTextBlock"));
	KillFeedScrollBox = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("KillFeedScrollBox"));

	if (ScoreTextBlock)
	{
		ScoreTextBlock->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		ScoreTextBlock->SetShadowOffset(FVector2D(1.0f, 1.0f));
		ScoreTextBlock->SetText(FText::GetEmpty());
		if (UVerticalBoxSlot* ScoreSlot = RootPanel->AddChildToVerticalBox(ScoreTextBlock))
		{
			ScoreSlot->SetPadding(FMargin(20.0f, 20.0f, 20.0f, 8.0f));
		}
	}

	if (HealthTextBlock)
	{
		HealthTextBlock->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		HealthTextBlock->SetShadowOffset(FVector2D(1.0f, 1.0f));
		HealthTextBlock->SetText(FText::GetEmpty());
		if (UVerticalBoxSlot* HealthTextSlot = RootPanel->AddChildToVerticalBox(HealthTextBlock))
		{
			HealthTextSlot->SetPadding(FMargin(20.0f, 0.0f, 20.0f, 6.0f));
		}
	}

	if (HealthProgressBar)
	{
		HealthProgressBar->SetFillColorAndOpacity(FLinearColor(0.85f, 0.15f, 0.15f, 1.0f));
		HealthProgressBar->SetPercent(1.0f);
		if (UVerticalBoxSlot* HealthBarSlot = RootPanel->AddChildToVerticalBox(HealthProgressBar))
		{
			HealthBarSlot->SetPadding(FMargin(20.0f, 0.0f, 20.0f, 0.0f));
		}
	}

	if (AmmoTextBlock)
	{
		AmmoTextBlock->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		AmmoTextBlock->SetShadowOffset(FVector2D(1.0f, 1.0f));
		AmmoTextBlock->SetText(FText::GetEmpty());
		AmmoTextBlock->SetVisibility(ESlateVisibility::Hidden);
		if (UVerticalBoxSlot* AmmoSlot = RootPanel->AddChildToVerticalBox(AmmoTextBlock))
		{
			AmmoSlot->SetPadding(FMargin(20.0f, 8.0f, 20.0f, 0.0f));
		}
	}

	if (TargetScoreTextBlock)
	{
		TargetScoreTextBlock->SetColorAndOpacity(FSlateColor(FLinearColor(0.75f, 0.92f, 1.0f, 1.0f)));
		TargetScoreTextBlock->SetShadowOffset(FVector2D(1.0f, 1.0f));
		TargetScoreTextBlock->SetText(FText::GetEmpty());
		if (UVerticalBoxSlot* TargetScoreSlot = RootPanel->AddChildToVerticalBox(TargetScoreTextBlock))
		{
			TargetScoreSlot->SetPadding(FMargin(20.0f, 8.0f, 20.0f, 0.0f));
		}
	}

	if (MatchStatusTextBlock)
	{
		MatchStatusTextBlock->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.9f, 0.35f, 1.0f)));
		MatchStatusTextBlock->SetShadowOffset(FVector2D(1.0f, 1.0f));
		MatchStatusTextBlock->SetText(FText::GetEmpty());
		MatchStatusTextBlock->SetVisibility(ESlateVisibility::Hidden);
		if (UVerticalBoxSlot* MatchStatusSlot = RootPanel->AddChildToVerticalBox(MatchStatusTextBlock))
		{
			MatchStatusSlot->SetPadding(FMargin(20.0f, 8.0f, 20.0f, 0.0f));
		}
	}

	if (PickupPromptTextBlock)
	{
		PickupPromptTextBlock->SetColorAndOpacity(FSlateColor(FLinearColor(0.98f, 0.86f, 0.35f, 1.0f)));
		PickupPromptTextBlock->SetShadowOffset(FVector2D(1.0f, 1.0f));
		PickupPromptTextBlock->SetText(PickupPromptText);
		PickupPromptTextBlock->SetVisibility(ESlateVisibility::Hidden);
		if (UVerticalBoxSlot* PickupPromptSlot = RootPanel->AddChildToVerticalBox(PickupPromptTextBlock))
		{
			PickupPromptSlot->SetPadding(FMargin(20.0f, 18.0f, 20.0f, 0.0f));
		}
	}

	if (KillFeedScrollBox)
	{
		if (UVerticalBoxSlot* KillFeedSlot = RootPanel->AddChildToVerticalBox(KillFeedScrollBox))
		{
			KillFeedSlot->SetPadding(FMargin(20.0f, 18.0f, 20.0f, 0.0f));
		}
	}
}

void UMyFpsScoreWidget::RefreshHud()
{
	CachedScore = GetCurrentScore();
	CachedHealth = GetCurrentHealth();
	CachedMaxHealth = GetMaxHealth();
	CachedHealthPercent = CachedMaxHealth > 0.0f ? CachedHealth / CachedMaxHealth : 0.0f;
	CachedAmmoInClip = GetCurrentAmmoInClip();
	CachedReserveAmmo = GetCurrentReserveAmmo();
	bCachedIsReloading = IsCurrentWeaponReloading();
	bCachedShowAmmo = HasWeaponEquipped();
	CachedTargetScore = GetTargetScore();
	bCachedMatchFinished = IsMatchFinished();
	CachedAmmoText = bCachedIsReloading
		? FText::Format(
			FText::FromString(TEXT("{0}{1} / {2}  {3}")),
			AmmoPrefix,
			FText::AsNumber(CachedAmmoInClip),
			FText::AsNumber(CachedReserveAmmo),
			ReloadingText
		)
		: FText::Format(
			FText::FromString(TEXT("{0}{1} / {2}")),
			AmmoPrefix,
			FText::AsNumber(CachedAmmoInClip),
			FText::AsNumber(CachedReserveAmmo)
		);
	CachedTargetScoreText = FText::Format(
		FText::FromString(TEXT("{0}{1}")),
		TargetScorePrefix,
		FText::AsNumber(CachedTargetScore)
	);
	CachedMatchStatusText = bCachedMatchFinished
		? FText::Format(
			FText::FromString(TEXT("{0} {1}")),
			FText::FromString(GetWinnerName()),
			VictorySuffixText
		)
		: FText::GetEmpty();
	bCachedShowPickupPrompt = false;
	CachedPickupPromptText = FText::GetEmpty();
	if (HasAvailablePickup())
	{
		bCachedShowPickupPrompt = true;
		CachedPickupPromptText = HasWeaponEquipped() ? SwapPromptText : PickupPromptText;
	}

	if (ScoreTextBlock)
	{
		const FText ScoreValueText = FText::AsNumber(FMath::RoundToInt(CachedScore));
		ScoreTextBlock->SetText(FText::Format(FText::FromString(TEXT("{0}{1}")), ScorePrefix, ScoreValueText));
	}

	if (HealthTextBlock)
	{
		const int32 CurrentHealth = FMath::RoundToInt(CachedHealth);
		const int32 MaxHealth = FMath::RoundToInt(CachedMaxHealth);
		HealthTextBlock->SetText(FText::Format(
			FText::FromString(TEXT("{0}{1} / {2}")),
			HealthPrefix,
			FText::AsNumber(CurrentHealth),
			FText::AsNumber(MaxHealth)
		));
	}

	if (HealthProgressBar)
	{
		HealthProgressBar->SetPercent(FMath::Clamp(CachedHealthPercent, 0.0f, 1.0f));
	}

	if (AmmoTextBlock)
	{
		AmmoTextBlock->SetText(CachedAmmoText);
		AmmoTextBlock->SetVisibility(bCachedShowAmmo ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}

	if (TargetScoreTextBlock)
	{
		TargetScoreTextBlock->SetText(CachedTargetScoreText);
	}

	if (MatchStatusTextBlock)
	{
		MatchStatusTextBlock->SetText(CachedMatchStatusText);
		MatchStatusTextBlock->SetVisibility(bCachedMatchFinished ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}

	if (PickupPromptTextBlock)
	{
		PickupPromptTextBlock->SetText(CachedPickupPromptText);
		PickupPromptTextBlock->SetVisibility(bCachedShowPickupPrompt ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}

	RefreshKillFeed();

	OnHudDataUpdated();
}

void UMyFpsScoreWidget::RefreshKillFeed()
{
	CachedKillFeedTexts.Reset();

	APlayerController* PlayerController = GetOwningPlayer();
	const AMyFpsGameState* GameState = PlayerController && PlayerController->GetWorld()
		? PlayerController->GetWorld()->GetGameState<AMyFpsGameState>()
		: nullptr;
	if (GameState)
	{
		for (const FMyFpsKillFeedMessage& Message : GameState->GetKillFeedMessages())
		{
			CachedKillFeedTexts.Add(FText::Format(
				FText::FromString(TEXT("{0} 击杀了 {1}")),
				FText::FromString(Message.KillerName),
				FText::FromString(Message.VictimName)
			));
		}
	}

	if (!KillFeedScrollBox)
	{
		return;
	}

	KillFeedScrollBox->ClearChildren();
	KillFeedScrollBox->SetVisibility(CachedKillFeedTexts.Num() > 0 ? ESlateVisibility::Visible : ESlateVisibility::Hidden);

	for (const FText& KillFeedText : CachedKillFeedTexts)
	{
		UTextBlock* KillTextBlock = WidgetTree
			? WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass())
			: nullptr;
		if (!KillTextBlock)
		{
			continue;
		}

		KillTextBlock->SetText(KillFeedText);
		KillTextBlock->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.88f, 0.35f, 1.0f)));
		KillTextBlock->SetShadowOffset(FVector2D(1.0f, 1.0f));
		KillFeedScrollBox->AddChild(KillTextBlock);
	}
}
