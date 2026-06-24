#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MyFpsGameInstance.generated.h"

class UMyFpsLanMenuWidget;

USTRUCT(BlueprintType)
struct FMyFpsHostSettings
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "LAN")
	FString RoomName = TEXT("MyFps LAN Room");

	UPROPERTY(BlueprintReadOnly, Category = "LAN")
	FString Password;

	UPROPERTY(BlueprintReadOnly, Category = "LAN")
	int32 MaxPlayers = 2;
};

UCLASS(Config = Game)
class MYFPS_API UMyFpsGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "LAN")
	void HostGameFromMainMenu(UObject* WorldContextObject, FName MapName, const FMyFpsHostSettings& Settings);

	UFUNCTION(BlueprintCallable, Category = "LAN")
	void JoinLanGame(APlayerController* PlayerController, const FString& Address);

	void ShowHostControlMenu(APlayerController* PlayerController);
	void InitializeHostControlMenu(APlayerController* PlayerController);
	void HideHostControlMenu(APlayerController* PlayerController);
	void ToggleHostControlMenu(APlayerController* PlayerController);
	void StartHostedGame(APlayerController* PlayerController);
	void EndHostedGame(APlayerController* PlayerController);

	UFUNCTION(BlueprintPure, Category = "LAN")
	FString GetLastLanAddress() const { return LastLanAddress; }

	UFUNCTION(BlueprintPure, Category = "Player")
	FString GetPlayerDisplayName() const { return PlayerDisplayName; }

	UFUNCTION(BlueprintCallable, Category = "Player")
	void SetPlayerDisplayName(const FString& InPlayerDisplayName);

	UFUNCTION(BlueprintPure, Category = "LAN")
	FString GetLocalLanAddress() const;

	UFUNCTION(BlueprintPure, Category = "LAN")
	FMyFpsHostSettings GetPendingHostSettings() const { return PendingHostSettings; }

	void RequestAutoStartAfterTravel();
	bool ConsumeAutoStartAfterTravel();
	void RequestHostControlMenuAfterTravel();
	bool ConsumeHostControlMenuAfterTravel();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LAN")
	TSubclassOf<UMyFpsLanMenuWidget> LanMenuWidgetClass;

private:
	void SetLastLanAddress(const FString& Address);

	UPROPERTY(Transient)
	TObjectPtr<UMyFpsLanMenuWidget> LanMenuWidgetInstance = nullptr;

	UPROPERTY(Config)
	FString LastLanAddress = TEXT("127.0.0.1");

	UPROPERTY(Config)
	FString PlayerDisplayName = TEXT("Player");

	FMyFpsHostSettings PendingHostSettings;

	bool bAutoStartAfterTravelPending = false;
	bool bHostControlMenuAfterTravelPending = false;
	bool bIsHostingLanGame = false;
};
