#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MyFpsGameInstance.generated.h"

class UMyFpsLanMenuWidget;

UCLASS(Config = Game)
class MYFPS_API UMyFpsGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "LAN")
	void ToggleLanMenu(APlayerController* PlayerController);

	UFUNCTION(BlueprintCallable, Category = "LAN")
	void ShowLanMenu(APlayerController* PlayerController);

	UFUNCTION(BlueprintCallable, Category = "LAN")
	void HideLanMenu();

	UFUNCTION(BlueprintCallable, Category = "LAN")
	void HostLanGame(UWorld* InWorld);

	UFUNCTION(BlueprintCallable, Category = "LAN")
	void JoinLanGame(APlayerController* PlayerController, const FString& Address);

	UFUNCTION(BlueprintCallable, Category = "LAN")
	void StartLanMatch(APlayerController* PlayerController);

	UFUNCTION(BlueprintPure, Category = "LAN")
	FString GetLastLanAddress() const { return LastLanAddress; }

	UFUNCTION(BlueprintPure, Category = "LAN")
	FString GetLocalLanAddress() const;

	void RequestAutoStartAfterTravel();
	bool ConsumeAutoStartAfterTravel();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LAN")
	TSubclassOf<UMyFpsLanMenuWidget> LanMenuWidgetClass;

private:
	void SetLastLanAddress(const FString& Address);

	UPROPERTY(Transient)
	TObjectPtr<UMyFpsLanMenuWidget> LanMenuWidgetInstance = nullptr;

	UPROPERTY(Config)
	FString LastLanAddress = TEXT("127.0.0.1");

	bool bAutoStartAfterTravelPending = false;
};
