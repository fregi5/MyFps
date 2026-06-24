#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MyFpsPlayerController.generated.h"

UCLASS()
class MYFPS_API AMyFpsPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void OnRep_PlayerState() override;

	UFUNCTION(Client, Reliable)
	void ClientShowMatchResult(const FString& WinnerName);

	UFUNCTION(Client, Reliable)
	void ClientPrepareForMatch();

	UFUNCTION(Client, Reliable)
	void ClientShowHitMarker(bool bKill);

	UFUNCTION(Client, Reliable)
	void ClientShowDamageFeedback(const FVector_NetQuantize& SourceWorldLocation);

	UFUNCTION(Client, Reliable)
	void ClientStartRespawnCountdown(float DelaySeconds);

	UFUNCTION(Client, Reliable)
	void ClientReturnToMainMenu(FName MenuMapName);

	UFUNCTION(Server, Reliable)
	void ServerSetPlayerDisplayName(const FString& InPlayerDisplayName);

	UFUNCTION(Server, Reliable)
	void ServerSetReadyForNextRound();

private:
	void ApplySavedPlayerDisplayName();
};
