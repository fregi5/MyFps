#include "MyFpsGameInstance.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "GameFramework/PlayerController.h"
#include "MyFpsGameMode.h"
#include "MyFpsLanMenuWidget.h"
#include "SocketSubsystem.h"

void UMyFpsGameInstance::ToggleLanMenu(APlayerController* PlayerController)
{
	if (LanMenuWidgetInstance && LanMenuWidgetInstance->IsInViewport() && LanMenuWidgetInstance->GetVisibility() != ESlateVisibility::Collapsed)
	{
		HideLanMenu();
		return;
	}

	ShowLanMenu(PlayerController);
}

void UMyFpsGameInstance::ShowLanMenu(APlayerController* PlayerController)
{
	if (!PlayerController || !PlayerController->IsLocalController())
	{
		return;
	}

	if (!LanMenuWidgetClass)
	{
		LanMenuWidgetClass = UMyFpsLanMenuWidget::StaticClass();
	}

	if (!LanMenuWidgetInstance)
	{
		LanMenuWidgetInstance = CreateWidget<UMyFpsLanMenuWidget>(PlayerController, LanMenuWidgetClass);
	}

	if (!LanMenuWidgetInstance)
	{
		return;
	}

	LanMenuWidgetInstance->AddToViewport(200);
	LanMenuWidgetInstance->SetVisibility(ESlateVisibility::Visible);
	PlayerController->bShowMouseCursor = true;
	UWidgetBlueprintLibrary::SetInputMode_GameAndUIEx(PlayerController, LanMenuWidgetInstance, EMouseLockMode::DoNotLock, false);
	LanMenuWidgetInstance->FocusAddressInput();
}

void UMyFpsGameInstance::HideLanMenu()
{
	if (!LanMenuWidgetInstance)
	{
		return;
	}

	APlayerController* PlayerController = LanMenuWidgetInstance->GetOwningPlayer();
	LanMenuWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);

	if (PlayerController)
	{
		PlayerController->bShowMouseCursor = false;
		UWidgetBlueprintLibrary::SetInputMode_GameOnly(PlayerController);
	}
}

void UMyFpsGameInstance::HostLanGame(UWorld* InWorld)
{
	if (!InWorld)
	{
		return;
	}

	const FString CurrentMapPath = UWorld::RemovePIEPrefix(InWorld->GetOutermost()->GetName());
	if (CurrentMapPath.IsEmpty())
	{
		return;
	}

	HideLanMenu();
	InWorld->ServerTravel(FString::Printf(TEXT("%s?listen"), *CurrentMapPath));
}

void UMyFpsGameInstance::JoinLanGame(APlayerController* PlayerController, const FString& Address)
{
	if (!PlayerController)
	{
		return;
	}

	FString JoinAddress = Address.TrimStartAndEnd();
	if (JoinAddress.IsEmpty())
	{
		if (LanMenuWidgetInstance)
		{
			LanMenuWidgetInstance->SetStatusMessage(FText::FromString(TEXT("请输入主机 IP 地址")));
		}
		return;
	}

	if (!JoinAddress.Contains(TEXT(":")))
	{
		JoinAddress.Append(TEXT(":7777"));
	}

	SetLastLanAddress(Address.TrimStartAndEnd());
	HideLanMenu();
	PlayerController->ClientTravel(JoinAddress, TRAVEL_Absolute);
}

void UMyFpsGameInstance::StartLanMatch(APlayerController* PlayerController)
{
	if (!PlayerController || !PlayerController->HasAuthority())
	{
		return;
	}

	if (AMyFpsGameMode* GameMode = PlayerController->GetWorld() ? PlayerController->GetWorld()->GetAuthGameMode<AMyFpsGameMode>() : nullptr)
	{
		HideLanMenu();
		GameMode->StartMatchGame();
	}
}

FString UMyFpsGameInstance::GetLocalLanAddress() const
{
	bool bCanBindAll = false;
	TSharedPtr<FInternetAddr> LocalHostAddress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetLocalHostAddr(*GLog, bCanBindAll);
	if (!LocalHostAddress.IsValid())
	{
		return TEXT("Unknown");
	}

	const FString Address = LocalHostAddress->ToString(false);
	return Address.IsEmpty() ? TEXT("Unknown") : Address;
}

void UMyFpsGameInstance::RequestAutoStartAfterTravel()
{
	bAutoStartAfterTravelPending = true;
}

bool UMyFpsGameInstance::ConsumeAutoStartAfterTravel()
{
	const bool bShouldAutoStart = bAutoStartAfterTravelPending;
	bAutoStartAfterTravelPending = false;
	return bShouldAutoStart;
}

void UMyFpsGameInstance::SetLastLanAddress(const FString& Address)
{
	LastLanAddress = Address.IsEmpty() ? TEXT("127.0.0.1") : Address;
	SaveConfig();
}
