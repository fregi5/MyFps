#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MyFpsMainMenu.generated.h"

class UButton;
class UEditableTextBox;
class UWidgetSwitcher;

UCLASS()
class MYFPS_API UMyFpsMainMenu: public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Menu|Maps")
    FName SinglePlayerMapName = TEXT("FirstPersonMap");

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Menu|Maps")
    FName MultiplayerMapName = TEXT("FirstPersonMap");

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UWidgetSwitcher> PageSwitcher = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> SinglePlayerButton = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> MultiplayerButton = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> SettingsButton = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> QuitButton = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> HostGameButton = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> JoinGameButton = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> MultiplayerBackButton = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> StartHostButton = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> HostSettingsBackButton = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> JoinButton = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> JoinBackButton = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> SaveSettingsButton = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> SettingsBackButton = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UEditableTextBox> HostRoomNameInput = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UEditableTextBox> HostPasswordInput = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UEditableTextBox> HostMaxPlayersInput = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UEditableTextBox> JoinIpInput = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UEditableTextBox> JoinPasswordInput = nullptr;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UEditableTextBox> PlayerNameInput = nullptr;

    UFUNCTION()
	void HandleSinglePlayerClicked();

	UFUNCTION()
	void HandleMultiplayerClicked();

	UFUNCTION()
	void HandleSettingsClicked();

	UFUNCTION()
	void HandleQuitClicked();

	UFUNCTION()
	void HandleHostGameClicked();

	UFUNCTION()
	void HandleJoinGameClicked();

	UFUNCTION()
	void HandleMultiplayerBackClicked();

	UFUNCTION()
	void HandleStartHostClicked();

	UFUNCTION()
	void HandleHostSettingsBackClicked();

	UFUNCTION()
	void HandleJoinClicked();

	UFUNCTION()
	void HandleJoinBackClicked();

	UFUNCTION()
	void HandleSaveSettingsClicked();

	UFUNCTION()
	void HandleSettingsBackClicked();

private:
	enum class EMenuPage : int32
	{
		Main = 0,
		Multiplayer = 1,
		HostSettings = 2,
		Join = 3,
		Settings = 4
	};

	void ShowPage(EMenuPage Page);
	FString GetInputText(const UEditableTextBox* Input) const;
};
