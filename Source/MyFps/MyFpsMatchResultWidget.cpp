#include "MyFpsMatchResultWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "MyFpsCharacter.h"
#include "MyFpsGameState.h"
#include "MyFpsPlayerState.h"

void UMyFpsMatchResultWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ReadyButton)
	{
		ReadyButton->OnClicked.AddDynamic(this, &UMyFpsMatchResultWidget::HandleReadyClicked);
		ReadyButton->OnHovered.AddDynamic(this, &UMyFpsMatchResultWidget::HandleReadyHovered);
		ReadyButton->OnPressed.AddDynamic(this, &UMyFpsMatchResultWidget::HandleReadyPressed);
	}

	BindToGameState();
	RefreshWidget();
}

void UMyFpsMatchResultWidget::NativeDestruct()
{
	UnbindFromGameState();

	if (ReadyButton)
	{
		ReadyButton->OnClicked.RemoveDynamic(this, &UMyFpsMatchResultWidget::HandleReadyClicked);
		ReadyButton->OnHovered.RemoveDynamic(this, &UMyFpsMatchResultWidget::HandleReadyHovered);
		ReadyButton->OnPressed.RemoveDynamic(this, &UMyFpsMatchResultWidget::HandleReadyPressed);
	}

	Super::NativeDestruct();
}

void UMyFpsMatchResultWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!CachedGameState.IsValid())
	{
		BindToGameState();
	}

	if (GetVisibility() == ESlateVisibility::Visible)
	{
		RefreshReadyStateOnly();
	}
}

void UMyFpsMatchResultWidget::RefreshFromGameState()
{
	RefreshWidget();
}

void UMyFpsMatchResultWidget::RefreshReadyDisplay()
{
	RefreshReadyStateOnly();
}

void UMyFpsMatchResultWidget::ShowExplicitResult(const FString& WinnerName)
{
	APlayerController* PlayerController = GetOwningPlayer();
	if (!PlayerController)
	{
		return;
	}

	SetVisibility(ESlateVisibility::Visible);
	bLocalReadyRequested = false;
	PlayerController->bShowMouseCursor = true;
	UWidgetBlueprintLibrary::SetInputMode_GameAndUIEx(PlayerController, this, EMouseLockMode::DoNotLock, false);

	if (WinnerTextBlock)
	{
		WinnerTextBlock->SetText(FText::Format(
			FText::FromString(TEXT("{0} {1}")),
			FText::FromString(WinnerName),
			VictorySuffixText
		));
	}

	RefreshReadyStateOnly();

	OnMatchResultUpdated();
}

void UMyFpsMatchResultWidget::HandleReadyClicked()
{
	APlayerController* PlayerController = GetOwningPlayer();
	if (!PlayerController)
	{
		return;
	}

	if (AMyFpsCharacter* PlayerCharacter = Cast<AMyFpsCharacter>(PlayerController->GetPawn()))
	{
		bLocalReadyRequested = true;
		PlayerCharacter->RequestReadyForNextRound();
		RefreshReadyStateOnly();
	}
}

void UMyFpsMatchResultWidget::HandleReadyHovered()
{
}

void UMyFpsMatchResultWidget::HandleReadyPressed()
{
}

void UMyFpsMatchResultWidget::BindToGameState()
{
	APlayerController* PlayerController = GetOwningPlayer();
	if (!PlayerController || !PlayerController->GetWorld())
	{
		return;
	}

	AMyFpsGameState* GameState = PlayerController->GetWorld()->GetGameState<AMyFpsGameState>();
	if (!GameState || CachedGameState.Get() == GameState)
	{
		return;
	}

	UnbindFromGameState();
	GameState->OnMatchStateChanged().AddUObject(this, &UMyFpsMatchResultWidget::HandleMatchStateChanged);
	CachedGameState = GameState;
}

void UMyFpsMatchResultWidget::UnbindFromGameState()
{
	if (AMyFpsGameState* GameState = CachedGameState.Get())
	{
		GameState->OnMatchStateChanged().RemoveAll(this);
	}

	CachedGameState.Reset();
}

void UMyFpsMatchResultWidget::HandleMatchStateChanged()
{
	RefreshWidget();
}

void UMyFpsMatchResultWidget::RefreshWidget()
{
	BindToGameState();

	APlayerController* PlayerController = GetOwningPlayer();
	if (!PlayerController || !PlayerController->GetWorld())
	{
		return;
	}

	AMyFpsGameState* GameState = PlayerController->GetWorld()->GetGameState<AMyFpsGameState>();
	AMyFpsPlayerState* PlayerState = PlayerController->GetPlayerState<AMyFpsPlayerState>();
	if (!GameState)
	{
		return;
	}

	const bool bMatchFinished = GameState->IsMatchFinished();
	SetVisibility(bMatchFinished ? ESlateVisibility::Visible : ESlateVisibility::Hidden);

	PlayerController->bShowMouseCursor = bMatchFinished;
	if (bMatchFinished)
	{
		UWidgetBlueprintLibrary::SetInputMode_GameAndUIEx(PlayerController, this, EMouseLockMode::DoNotLock, false);
	}
	else
	{
		UWidgetBlueprintLibrary::SetInputMode_GameOnly(PlayerController);
	}

	if (!bMatchFinished)
	{
		bLocalReadyRequested = false;
		return;
	}

	if (WinnerTextBlock)
	{
		WinnerTextBlock->SetText(FText::Format(
			FText::FromString(TEXT("{0} {1}")),
			FText::FromString(GameState->GetWinnerName()),
			VictorySuffixText
		));
	}

	RefreshReadyStateOnly();

	OnMatchResultUpdated();
}

void UMyFpsMatchResultWidget::RefreshReadyStateOnly()
{
	APlayerController* PlayerController = GetOwningPlayer();
	if (!PlayerController)
	{
		return;
	}

	AMyFpsPlayerState* PlayerState = PlayerController->GetPlayerState<AMyFpsPlayerState>();
	const bool bReplicatedReady = PlayerState && PlayerState->IsReadyForNextRound();
	if (bReplicatedReady)
	{
		bLocalReadyRequested = false;
	}

	const bool bIsReady = bLocalReadyRequested || bReplicatedReady;

	if (ReadyButton)
	{
		ReadyButton->SetIsEnabled(!bIsReady);
		ReadyButton->SetVisibility(bIsReady ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}

	if (ReadyStatusTextBlock)
	{
		ReadyStatusTextBlock->SetText(bIsReady ? ReadyConfirmedText : ReadyPromptText);
	}
}
