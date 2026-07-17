// Copyright Epic Games, Inc. All Rights Reserved.

#include "MyFpsNetworkDebugWidget.h"

#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Blueprint/WidgetTree.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Styling/CoreStyle.h"

void UMyFpsNetworkDebugWidget::NativeConstruct()
{
	Super::NativeConstruct();
	UE_LOG(LogTemp, Warning, TEXT("[NetworkDebug] NativeConstruct. Widget=%s OwningPlayer=%s RootWidget=%s"),
		*GetNameSafe(this),
		*GetNameSafe(GetOwningPlayer()),
		WidgetTree ? *GetNameSafe(WidgetTree->RootWidget) : TEXT("NoWidgetTree"));

	BuildDefaultLayout();

	if (NormalNetworkButton)
	{
		NormalNetworkButton->OnClicked.RemoveDynamic(this, &UMyFpsNetworkDebugWidget::HandleNormalNetworkClicked);
		NormalNetworkButton->OnClicked.AddDynamic(this, &UMyFpsNetworkDebugWidget::HandleNormalNetworkClicked);
	}

	if (AverageNetworkButton)
	{
		AverageNetworkButton->OnClicked.RemoveDynamic(this, &UMyFpsNetworkDebugWidget::HandleAverageNetworkClicked);
		AverageNetworkButton->OnClicked.AddDynamic(this, &UMyFpsNetworkDebugWidget::HandleAverageNetworkClicked);
	}

	if (BadNetworkButton)
	{
		BadNetworkButton->OnClicked.RemoveDynamic(this, &UMyFpsNetworkDebugWidget::HandleBadNetworkClicked);
		BadNetworkButton->OnClicked.AddDynamic(this, &UMyFpsNetworkDebugWidget::HandleBadNetworkClicked);
	}

	if (ClearNetworkButton)
	{
		ClearNetworkButton->OnClicked.RemoveDynamic(this, &UMyFpsNetworkDebugWidget::HandleClearNetworkClicked);
		ClearNetworkButton->OnClicked.AddDynamic(this, &UMyFpsNetworkDebugWidget::HandleClearNetworkClicked);
	}

	RefreshNetworkDebugText();
}

void UMyFpsNetworkDebugWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	TimeSinceLastRefresh += InDeltaTime;
	if (TimeSinceLastRefresh >= 0.25f)
	{
		TimeSinceLastRefresh = 0.0f;
		RefreshNetworkDebugText();
	}
}

void UMyFpsNetworkDebugWidget::RefreshNetworkDebugText()
{
	if (!NetworkStatusTextBlock)
	{
		return;
	}

	APlayerController* PlayerController = GetOwningPlayer();
	const APlayerState* PlayerState = PlayerController ? PlayerController->PlayerState : nullptr;
	const FString PlayerName = PlayerState && !PlayerState->GetPlayerName().IsEmpty()
		? PlayerState->GetPlayerName()
		: TEXT("Unknown");

	const FString StatusText = FString::Printf(
		TEXT("Network Debug\nMode: %s\nRole: %s\nPlayer: %s\nPing: %.0f ms\n\nPresets affect this running instance through UE Net console commands."),
		*GetNetModeText(),
		*GetLocalRoleText(),
		*PlayerName,
		GetLocalPingMilliseconds());

	NetworkStatusTextBlock->SetText(FText::FromString(StatusText));
}

void UMyFpsNetworkDebugWidget::BuildDefaultLayout()
{
	if (!WidgetTree || WidgetTree->RootWidget || NetworkStatusTextBlock)
	{
		UE_LOG(LogTemp, Warning, TEXT("[NetworkDebug] BuildDefaultLayout skipped. WidgetTree=%d RootWidget=%s StatusText=%s"),
			WidgetTree != nullptr,
			WidgetTree ? *GetNameSafe(WidgetTree->RootWidget) : TEXT("NoWidgetTree"),
			*GetNameSafe(NetworkStatusTextBlock));
		return;
	}

	UBorder* RootBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("NetworkDebugRootBorder"));
	RootBorder->SetPadding(FMargin(12.0f));
	RootBorder->SetBrushColor(FLinearColor(0.02f, 0.025f, 0.03f, 0.82f));
	RootBorder->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	WidgetTree->RootWidget = RootBorder;

	UVerticalBox* RootBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("NetworkDebugRootBox"));
	RootBorder->SetContent(RootBox);

	NetworkStatusTextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("NetworkStatusTextBlock"));
	NetworkStatusTextBlock->SetAutoWrapText(true);
	NetworkStatusTextBlock->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	NetworkStatusTextBlock->SetFont(FSlateFontInfo(FCoreStyle::GetDefaultFont(), 16));
	if (UVerticalBoxSlot* StatusSlot = RootBox->AddChildToVerticalBox(NetworkStatusTextBlock))
	{
		StatusSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
	}

	UHorizontalBox* ButtonBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("NetworkPresetButtonBox"));
	if (UVerticalBoxSlot* ButtonBoxSlot = RootBox->AddChildToVerticalBox(ButtonBox))
	{
		ButtonBoxSlot->SetPadding(FMargin(0.0f));
	}

	NormalNetworkButton = AddProfileButton(ButtonBox, FText::FromString(TEXT("Normal")));
	AverageNetworkButton = AddProfileButton(ButtonBox, FText::FromString(TEXT("100ms")));
	BadNetworkButton = AddProfileButton(ButtonBox, FText::FromString(TEXT("Bad")));
	ClearNetworkButton = AddProfileButton(ButtonBox, FText::FromString(TEXT("Clear")));
	UE_LOG(LogTemp, Warning, TEXT("[NetworkDebug] BuildDefaultLayout finished. Root=%s StatusText=%s"),
		*GetNameSafe(WidgetTree->RootWidget),
		*GetNameSafe(NetworkStatusTextBlock));
}

UButton* UMyFpsNetworkDebugWidget::AddProfileButton(UHorizontalBox* RootBox, const FText& Label)
{
	if (!WidgetTree || !RootBox)
	{
		return nullptr;
	}

	UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
	UTextBlock* ButtonText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	ButtonText->SetText(Label);
	ButtonText->SetJustification(ETextJustify::Center);
	ButtonText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	Button->SetContent(ButtonText);

	if (UHorizontalBoxSlot* ButtonSlot = RootBox->AddChildToHorizontalBox(Button))
	{
		ButtonSlot->SetPadding(FMargin(0.0f, 0.0f, 6.0f, 0.0f));
	}

	return Button;
}

FString UMyFpsNetworkDebugWidget::GetNetModeText() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return TEXT("NoWorld");
	}

	switch (World->GetNetMode())
	{
	case NM_Standalone:
		return TEXT("Standalone");
	case NM_DedicatedServer:
		return TEXT("DedicatedServer");
	case NM_ListenServer:
		return TEXT("ListenServer");
	case NM_Client:
		return TEXT("Client");
	default:
		return TEXT("Unknown");
	}
}

FString UMyFpsNetworkDebugWidget::GetLocalRoleText() const
{
	const APlayerController* PlayerController = GetOwningPlayer();
	const APawn* Pawn = PlayerController ? PlayerController->GetPawn() : nullptr;
	if (!Pawn)
	{
		return TEXT("NoPawn");
	}

	switch (Pawn->GetLocalRole())
	{
	case ROLE_Authority:
		return TEXT("Authority");
	case ROLE_AutonomousProxy:
		return TEXT("AutonomousProxy");
	case ROLE_SimulatedProxy:
		return TEXT("SimulatedProxy");
	case ROLE_None:
	default:
		return TEXT("None");
	}
}

float UMyFpsNetworkDebugWidget::GetLocalPingMilliseconds() const
{
	const APlayerController* PlayerController = GetOwningPlayer();
	const APlayerState* PlayerState = PlayerController ? PlayerController->PlayerState : nullptr;
	return PlayerState ? PlayerState->GetPingInMilliseconds() : 0.0f;
}

void UMyFpsNetworkDebugWidget::ApplyNetworkEmulation(int32 PacketLag, int32 PacketLagVariance, int32 PacketLoss)
{
	APlayerController* PlayerController = GetOwningPlayer();
	if (!PlayerController)
	{
		return;
	}

	PlayerController->ConsoleCommand(FString::Printf(TEXT("Net PktLag=%d"), FMath::Max(0, PacketLag)));
	PlayerController->ConsoleCommand(FString::Printf(TEXT("Net PktLagVariance=%d"), FMath::Max(0, PacketLagVariance)));
	PlayerController->ConsoleCommand(FString::Printf(TEXT("Net PktLoss=%d"), FMath::Clamp(PacketLoss, 0, 100)));
	RefreshNetworkDebugText();
}

void UMyFpsNetworkDebugWidget::HandleNormalNetworkClicked()
{
	ApplyNetworkEmulation(0, 0, 0);
}

void UMyFpsNetworkDebugWidget::HandleAverageNetworkClicked()
{
	ApplyNetworkEmulation(100, 20, 1);
}

void UMyFpsNetworkDebugWidget::HandleBadNetworkClicked()
{
	ApplyNetworkEmulation(180, 50, 5);
}

void UMyFpsNetworkDebugWidget::HandleClearNetworkClicked()
{
	ApplyNetworkEmulation(0, 0, 0);
}
