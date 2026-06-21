#include "MyFpsLanMenuWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "GameFramework/PlayerController.h"
#include "MyFpsGameInstance.h"

void UMyFpsLanMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BuildDefaultLayout();

	if (HostButton)
	{
		HostButton->OnClicked.AddDynamic(this, &UMyFpsLanMenuWidget::HandleHostClicked);
	}

	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &UMyFpsLanMenuWidget::HandleJoinClicked);
	}

	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &UMyFpsLanMenuWidget::HandleCloseClicked);
	}

	if (StartButton)
	{
		StartButton->OnClicked.AddDynamic(this, &UMyFpsLanMenuWidget::HandleStartClicked);
	}

	if (TitleTextBlock)
	{
		TitleTextBlock->SetText(TitleText);
	}

	if (AddressTextBox)
	{
		AddressTextBox->SetHintText(AddressHintText);
	}

	RefreshLanInfo();
	SetStatusMessage(FText::FromString(TEXT("输入主机 IP，然后点击 Join")));
}

void UMyFpsLanMenuWidget::NativeDestruct()
{
	if (HostButton)
	{
		HostButton->OnClicked.RemoveDynamic(this, &UMyFpsLanMenuWidget::HandleHostClicked);
	}

	if (JoinButton)
	{
		JoinButton->OnClicked.RemoveDynamic(this, &UMyFpsLanMenuWidget::HandleJoinClicked);
	}

	if (CloseButton)
	{
		CloseButton->OnClicked.RemoveDynamic(this, &UMyFpsLanMenuWidget::HandleCloseClicked);
	}

	if (StartButton)
	{
		StartButton->OnClicked.RemoveDynamic(this, &UMyFpsLanMenuWidget::HandleStartClicked);
	}

	Super::NativeDestruct();
}

void UMyFpsLanMenuWidget::SetStatusMessage(const FText& InStatusMessage)
{
	if (StatusTextBlock)
	{
		StatusTextBlock->SetText(InStatusMessage);
	}
}

void UMyFpsLanMenuWidget::FocusAddressInput()
{
	if (!AddressTextBox)
	{
		return;
	}

	AddressTextBox->SetKeyboardFocus();
}

void UMyFpsLanMenuWidget::HandleHostClicked()
{
	APlayerController* PlayerController = GetOwningPlayer();
	if (!PlayerController)
	{
		return;
	}

	if (UMyFpsGameInstance* GameInstance = PlayerController->GetGameInstance<UMyFpsGameInstance>())
	{
		SetStatusMessage(FText::FromString(TEXT("正在创建 LAN 主机...")));
		GameInstance->HostLanGame(GetWorld());
	}
}

void UMyFpsLanMenuWidget::HandleJoinClicked()
{
	APlayerController* PlayerController = GetOwningPlayer();
	if (!PlayerController)
	{
		return;
	}

	const FString Address = AddressTextBox ? AddressTextBox->GetText().ToString() : FString();
	if (UMyFpsGameInstance* GameInstance = PlayerController->GetGameInstance<UMyFpsGameInstance>())
	{
		SetStatusMessage(FText::Format(FText::FromString(TEXT("正在连接 {0} ...")), FText::FromString(Address)));
		GameInstance->JoinLanGame(PlayerController, Address);
	}
}

void UMyFpsLanMenuWidget::HandleCloseClicked()
{
	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		if (UMyFpsGameInstance* GameInstance = PlayerController->GetGameInstance<UMyFpsGameInstance>())
		{
			GameInstance->HideLanMenu();
		}
	}
}

void UMyFpsLanMenuWidget::HandleStartClicked()
{
	APlayerController* PlayerController = GetOwningPlayer();
	if (!PlayerController)
	{
		return;
	}

	if (UMyFpsGameInstance* GameInstance = PlayerController->GetGameInstance<UMyFpsGameInstance>())
	{
		GameInstance->StartLanMatch(PlayerController);
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

	TitleTextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TitleTextBlock"));
	AddressTextBox = WidgetTree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(), TEXT("AddressTextBox"));
	HostButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("HostButton"));
	JoinButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("JoinButton"));
	StartButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("StartButton"));
	CloseButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("CloseButton"));
	StatusTextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StatusTextBlock"));
	LocalIpTextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LocalIpTextBlock"));

	TitleTextBlock->SetText(TitleText);
	LocalIpTextBlock->SetAutoWrapText(true);
	LocalIpTextBlock->SetColorAndOpacity(FSlateColor(FLinearColor(0.45f, 0.72f, 1.0f, 1.0f)));
	StatusTextBlock->SetAutoWrapText(true);
	StatusTextBlock->SetColorAndOpacity(FSlateColor(FLinearColor(0.8f, 0.9f, 1.0f, 1.0f)));

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

	AddButtonLabel(HostButton, TEXT("HostButtonLabel"), HostButtonText);
	AddButtonLabel(JoinButton, TEXT("JoinButtonLabel"), JoinButtonText);
	AddButtonLabel(StartButton, TEXT("StartButtonLabel"), StartButtonText);
	AddButtonLabel(CloseButton, TEXT("CloseButtonLabel"), CloseButtonText);

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

	AddToBox(TitleTextBlock, 0.0f);
	AddToBox(LocalIpTextBlock);
	AddToBox(AddressTextBox);
	AddToBox(HostButton);
	AddToBox(JoinButton);
	AddToBox(StartButton);
	AddToBox(CloseButton);
	AddToBox(StatusTextBlock, 12.0f);
}

void UMyFpsLanMenuWidget::RefreshLanInfo()
{
	APlayerController* PlayerController = GetOwningPlayer();
	UMyFpsGameInstance* GameInstance = PlayerController ? PlayerController->GetGameInstance<UMyFpsGameInstance>() : nullptr;
	if (!GameInstance)
	{
		return;
	}

	if (AddressTextBox)
	{
		AddressTextBox->SetText(FText::FromString(GameInstance->GetLastLanAddress()));
	}

	if (LocalIpTextBlock)
	{
		LocalIpTextBlock->SetText(FText::Format(
			FText::FromString(TEXT("本机 LAN IP: {0}")),
			FText::FromString(GameInstance->GetLocalLanAddress())
		));
	}
}
