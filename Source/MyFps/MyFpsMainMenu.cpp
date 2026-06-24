#include "MyFpsMainMenu.h"

#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/WidgetSwitcher.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "MyFpsGameInstance.h"

void UMyFpsMainMenu::NativeConstruct()
{
    Super::NativeConstruct();

    if (SinglePlayerButton)
    {
        SinglePlayerButton->OnClicked.AddDynamic(this, &UMyFpsMainMenu::HandleSinglePlayerClicked);
    }
    if (MultiplayerButton)
    {
        MultiplayerButton->OnClicked.AddDynamic(this, &UMyFpsMainMenu::HandleMultiplayerClicked);
    }
    if (SettingsButton)
    {
        SettingsButton->OnClicked.AddDynamic(this, &UMyFpsMainMenu::HandleSettingsClicked);
    }
    if (QuitButton)
    {
        QuitButton->OnClicked.AddDynamic(this, &UMyFpsMainMenu::HandleQuitClicked);
    }
    if (HostGameButton)
    {
        HostGameButton->OnClicked.AddDynamic(this, &UMyFpsMainMenu::HandleHostGameClicked);
    }
    if (JoinGameButton)
    {
        JoinGameButton->OnClicked.AddDynamic(this, &UMyFpsMainMenu::HandleJoinGameClicked);
    }
    if (MultiplayerBackButton)
    {
        MultiplayerBackButton->OnClicked.AddDynamic(this, &UMyFpsMainMenu::HandleMultiplayerBackClicked);
    }
    if (StartHostButton)
    {
        StartHostButton->OnClicked.AddDynamic(this, &UMyFpsMainMenu::HandleStartHostClicked);
    }
    if (HostSettingsBackButton)
    {
        HostSettingsBackButton->OnClicked.AddDynamic(this, &UMyFpsMainMenu::HandleHostSettingsBackClicked);
    }
    if (JoinButton)
    {
        JoinButton->OnClicked.AddDynamic(this, &UMyFpsMainMenu::HandleJoinClicked);
    }
    if (JoinBackButton)
    {
        JoinBackButton->OnClicked.AddDynamic(this, &UMyFpsMainMenu::HandleJoinBackClicked);
    }
    if (SaveSettingsButton)
    {
        SaveSettingsButton->OnClicked.AddDynamic(this, &UMyFpsMainMenu::HandleSaveSettingsClicked);
    }
    if (SettingsBackButton)
    {
        SettingsBackButton->OnClicked.AddDynamic(this, &UMyFpsMainMenu::HandleSettingsBackClicked);
    }

    ShowPage(EMenuPage::Main);
}

void UMyFpsMainMenu::HandleSinglePlayerClicked()
{
    if (UMyFpsGameInstance* GameInstance = GetGameInstance<UMyFpsGameInstance>())
    {
        GameInstance->RequestAutoStartAfterTravel();
    }

    UGameplayStatics::OpenLevel(this, SinglePlayerMapName);
}

void UMyFpsMainMenu::HandleMultiplayerClicked()
{
    ShowPage(EMenuPage::Multiplayer);
}

void UMyFpsMainMenu::HandleSettingsClicked()
{
    if (PlayerNameInput)
    {
        if (UMyFpsGameInstance* GameInstance = GetGameInstance<UMyFpsGameInstance>())
        {
            PlayerNameInput->SetText(FText::FromString(GameInstance->GetPlayerDisplayName()));
        }
    }

    ShowPage(EMenuPage::Settings);
}

void UMyFpsMainMenu::HandleQuitClicked()
{
    APlayerController* PC = GetOwningPlayer();

    UKismetSystemLibrary::QuitGame(
        this,
        PC,
        EQuitPreference::Quit,
        false
    );
}

void UMyFpsMainMenu::HandleHostGameClicked()
{
    ShowPage(EMenuPage::HostSettings);
}

void UMyFpsMainMenu::HandleJoinGameClicked()
{
    ShowPage(EMenuPage::Join);
}

void UMyFpsMainMenu::HandleMultiplayerBackClicked()
{
    ShowPage(EMenuPage::Main);
}

void UMyFpsMainMenu::HandleStartHostClicked()
{
    UMyFpsGameInstance* GameInstance = GetGameInstance<UMyFpsGameInstance>();
    if (!GameInstance)
    {
        return;
    }

    FMyFpsHostSettings Settings;
    Settings.RoomName = GetInputText(HostRoomNameInput);
    Settings.Password = GetInputText(HostPasswordInput);
    const FString MaxPlayersText = GetInputText(HostMaxPlayersInput);
    Settings.MaxPlayers = MaxPlayersText.IsEmpty()
        ? 2
        : FMath::Clamp(FCString::Atoi(*MaxPlayersText), 1, 16);

    if (Settings.RoomName.IsEmpty())
    {
        Settings.RoomName = TEXT("MyFps LAN Room");
    }

    GameInstance->HostGameFromMainMenu(this, MultiplayerMapName, Settings);
}

void UMyFpsMainMenu::HandleHostSettingsBackClicked()
{
    ShowPage(EMenuPage::Multiplayer);
}

void UMyFpsMainMenu::HandleJoinClicked()
{
    APlayerController* PlayerController = GetOwningPlayer();
    UMyFpsGameInstance* GameInstance = GetGameInstance<UMyFpsGameInstance>();
    if (!PlayerController || !GameInstance)
    {
        return;
    }

    // Password validation is intentionally added with the future lobby PreLogin flow.
    GameInstance->JoinLanGame(PlayerController, GetInputText(JoinIpInput));
}

void UMyFpsMainMenu::HandleJoinBackClicked()
{
    ShowPage(EMenuPage::Multiplayer);
}

void UMyFpsMainMenu::HandleSaveSettingsClicked()
{
    if (UMyFpsGameInstance* GameInstance = GetGameInstance<UMyFpsGameInstance>())
    {
        GameInstance->SetPlayerDisplayName(GetInputText(PlayerNameInput));
    }

    ShowPage(EMenuPage::Main);
}

void UMyFpsMainMenu::HandleSettingsBackClicked()
{
    ShowPage(EMenuPage::Main);
}

void UMyFpsMainMenu::ShowPage(const EMenuPage Page)
{
    if (PageSwitcher)
    {
        PageSwitcher->SetActiveWidgetIndex(static_cast<int32>(Page));
    }
}

FString UMyFpsMainMenu::GetInputText(const UEditableTextBox* Input) const
{
    return Input ? Input->GetText().ToString().TrimStartAndEnd() : FString();
}
