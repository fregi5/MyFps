#include "MyFpsLanMenuWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "GameFramework/PlayerController.h"
#include "MyFpsGameInstance.h"
#include "MyFpsGameState.h"

void UMyFpsLanMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BuildDefaultLayout();

	if (StartGameButton)
	{
		StartGameButton->OnClicked.AddDynamic(this, &UMyFpsLanMenuWidget::HandleStartGameClicked);
	}

	if (EndGameButton)
	{
		EndGameButton->OnClicked.AddDynamic(this, &UMyFpsLanMenuWidget::HandleEndGameClicked);
	}

	if (AMyFpsGameState* GameState = GetWorld() ? GetWorld()->GetGameState<AMyFpsGameState>() : nullptr)
	{
		GameState->OnMatchStateChanged().AddUObject(this, &UMyFpsLanMenuWidget::HandleMatchStateChanged);
		CachedGameState = GameState;
	}

	RefreshControlState();
}

void UMyFpsLanMenuWidget::NativeDestruct()
{
	if (StartGameButton)
	{
		StartGameButton->OnClicked.RemoveDynamic(this, &UMyFpsLanMenuWidget::HandleStartGameClicked);
	}

	if (EndGameButton)
	{
		EndGameButton->OnClicked.RemoveDynamic(this, &UMyFpsLanMenuWidget::HandleEndGameClicked);
	}

	if (AMyFpsGameState* GameState = CachedGameState.Get())
	{
		GameState->OnMatchStateChanged().RemoveAll(this);
	}

	CachedGameState.Reset();

	Super::NativeDestruct();
}

void UMyFpsLanMenuWidget::RefreshControlState()
{
	const AMyFpsGameState* GameState = GetWorld() ? GetWorld()->GetGameState<AMyFpsGameState>() : nullptr;
	const bool bMatchStarted = GameState && GameState->IsMyFpsMatchStarted();

	if (StartGameButton)
	{
		StartGameButton->SetVisibility(bMatchStarted ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}

	if (EndGameButton)
	{
		EndGameButton->SetVisibility(bMatchStarted ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UMyFpsLanMenuWidget::HandleStartGameClicked()
{
	APlayerController* PlayerController = GetOwningPlayer();
	if (!PlayerController)
	{
		return;
	}

	if (UMyFpsGameInstance* GameInstance = PlayerController->GetGameInstance<UMyFpsGameInstance>())
	{
		GameInstance->StartHostedGame(PlayerController);
		GameInstance->HideHostControlMenu(PlayerController);
	}
}

void UMyFpsLanMenuWidget::HandleEndGameClicked()
{
	APlayerController* PlayerController = GetOwningPlayer();
	if (!PlayerController)
	{
		return;
	}

	if (UMyFpsGameInstance* GameInstance = PlayerController->GetGameInstance<UMyFpsGameInstance>())
	{
		GameInstance->EndHostedGame(PlayerController);
	}
}

void UMyFpsLanMenuWidget::BuildDefaultLayout()
{
	if (!WidgetTree || WidgetTree->RootWidget)
	{
		return;
	}

	UBorder* RootBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("LanMenuRootBorder"));
	RootBorder->SetPadding(FMargin(18.0f));
	RootBorder->SetBrushColor(FLinearColor(0.02f, 0.04f, 0.08f, 0.92f));
	WidgetTree->RootWidget = RootBorder;

	UVerticalBox* RootBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("LanMenuRootBox"));
	RootBorder->SetContent(RootBox);

	StartGameButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("StartGameButton"));
	EndGameButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("EndGameButton"));

	auto AddButtonLabel = [this](UButton* Button, const TCHAR* Name, const FText& LabelText)
	{
		if (!Button || !WidgetTree)
		{
			return;
		}

		UTextBlock* Label = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
		Label->SetText(LabelText);
		Button->SetContent(Label);
	};

	AddButtonLabel(StartGameButton, TEXT("StartGameButtonLabel"), StartButtonText);
	AddButtonLabel(EndGameButton, TEXT("EndGameButtonLabel"), EndButtonText);

	auto AddToBox = [RootBox](UWidget* Child, float PaddingTop = 8.0f)
	{
		if (!RootBox || !Child)
		{
			return;
		}

		if (UVerticalBoxSlot* Slot = RootBox->AddChildToVerticalBox(Child))
		{
			Slot->SetPadding(FMargin(0.0f, PaddingTop, 0.0f, 0.0f));
		}
	};

	AddToBox(StartGameButton, 0.0f);
	AddToBox(EndGameButton);
}

void UMyFpsLanMenuWidget::HandleMatchStateChanged()
{
	RefreshControlState();
}
