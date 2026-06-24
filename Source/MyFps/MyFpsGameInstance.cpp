#include "MyFpsGameInstance.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "MyFpsGameMode.h"
#include "MyFpsLanMenuWidget.h"
#include "SocketSubsystem.h"

void UMyFpsGameInstance::HostGameFromMainMenu(UObject* WorldContextObject, const FName MapName, const FMyFpsHostSettings& Settings)
{
	if (!WorldContextObject || MapName.IsNone())
	{
		return;
	}

	PendingHostSettings = Settings;
	PendingHostSettings.MaxPlayers = FMath::Clamp(PendingHostSettings.MaxPlayers, 1, 16);
	bIsHostingLanGame = true;
	RequestHostControlMenuAfterTravel();
	UGameplayStatics::OpenLevel(WorldContextObject, MapName, true, TEXT("listen"));
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
		return;
	}

	if (!JoinAddress.Contains(TEXT(":")))
	{
		JoinAddress.Append(TEXT(":7777"));
	}

	SetLastLanAddress(Address.TrimStartAndEnd());
	PlayerController->ClientTravel(JoinAddress, TRAVEL_Absolute);
}

void UMyFpsGameInstance::InitializeHostControlMenu(APlayerController* PlayerController)
{
	if (!bIsHostingLanGame || !PlayerController || !PlayerController->IsLocalController() || !PlayerController->HasAuthority())
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
	LanMenuWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
	LanMenuWidgetInstance->RefreshControlState();
}

void UMyFpsGameInstance::ShowHostControlMenu(APlayerController* PlayerController)
{
	InitializeHostControlMenu(PlayerController);
	if (!LanMenuWidgetInstance)
	{
		return;
	}

	LanMenuWidgetInstance->SetVisibility(ESlateVisibility::Visible);
	PlayerController->bShowMouseCursor = true;
	UWidgetBlueprintLibrary::SetInputMode_GameAndUIEx(PlayerController, LanMenuWidgetInstance, EMouseLockMode::DoNotLock, false);
	LanMenuWidgetInstance->RefreshControlState();
}

void UMyFpsGameInstance::HideHostControlMenu(APlayerController* PlayerController)
{
	if (!bIsHostingLanGame || !PlayerController || !PlayerController->IsLocalController() || !PlayerController->HasAuthority())
	{
		return;
	}

	if (LanMenuWidgetInstance)
	{
		LanMenuWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
	}

	PlayerController->bShowMouseCursor = false;
	UWidgetBlueprintLibrary::SetInputMode_GameOnly(PlayerController);
}

void UMyFpsGameInstance::ToggleHostControlMenu(APlayerController* PlayerController)
{
	if (!bIsHostingLanGame || !PlayerController || !PlayerController->IsLocalController() || !PlayerController->HasAuthority())
	{
		return;
	}

	if (LanMenuWidgetInstance && LanMenuWidgetInstance->IsInViewport() && LanMenuWidgetInstance->GetVisibility() == ESlateVisibility::Visible)
	{
		HideHostControlMenu(PlayerController);
		return;
	}

	ShowHostControlMenu(PlayerController);
}

void UMyFpsGameInstance::StartHostedGame(APlayerController* PlayerController)
{
	if (!PlayerController || !PlayerController->HasAuthority())
	{
		return;
	}

	if (AMyFpsGameMode* GameMode = PlayerController->GetWorld() ? PlayerController->GetWorld()->GetAuthGameMode<AMyFpsGameMode>() : nullptr)
	{
		GameMode->StartMatchGame();
	}
}

void UMyFpsGameInstance::EndHostedGame(APlayerController* PlayerController)
{
	if (!PlayerController || !PlayerController->HasAuthority())
	{
		return;
	}

	if (AMyFpsGameMode* GameMode = PlayerController->GetWorld() ? PlayerController->GetWorld()->GetAuthGameMode<AMyFpsGameMode>() : nullptr)
	{
		GameMode->EndHostedGame(PlayerController);
	}

	ShowHostControlMenu(PlayerController);
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

void UMyFpsGameInstance::RequestHostControlMenuAfterTravel()
{
	bHostControlMenuAfterTravelPending = true;
}

bool UMyFpsGameInstance::ConsumeHostControlMenuAfterTravel()
{
	const bool bShouldShowHostControlMenu = bHostControlMenuAfterTravelPending;
	bHostControlMenuAfterTravelPending = false;
	return bShouldShowHostControlMenu;
}

void UMyFpsGameInstance::SetLastLanAddress(const FString& Address)
{
	LastLanAddress = Address.IsEmpty() ? TEXT("127.0.0.1") : Address;
	SaveConfig();
}

void UMyFpsGameInstance::SetPlayerDisplayName(const FString& InPlayerDisplayName)
{
	const FString SanitizedName = InPlayerDisplayName.TrimStartAndEnd().Left(16);
	PlayerDisplayName = SanitizedName.IsEmpty() ? TEXT("Player") : SanitizedName;
	SaveConfig();
}
