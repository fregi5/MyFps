// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TimerManager.h"
#include "MyFpsGameMode.generated.h"

class AController;
class AMyFpsCharacter;
class UMyFpsWeaponDefinition;

UCLASS(minimalapi)
class AMyFpsGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMyFpsGameMode();
	virtual void InitGameState() override;
	virtual void StartPlay() override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

	UFUNCTION(BlueprintCallable, Category = "Score")
	void AddScore(AController* ScoringController, float ScoreAmount);

	UFUNCTION(BlueprintCallable, Category = "Score")
	void RegisterKill(AController* KillerController);

	UFUNCTION(BlueprintCallable, Category = "Score")
	void RegisterDeath(AController* VictimController);

	UFUNCTION(BlueprintCallable, Category = "Kill Feed")
	void RegisterKillFeed(AController* KillerController, AController* VictimController);

	UFUNCTION(BlueprintCallable, Category = "Respawn")
	void RespawnPlayer(AController* PlayerController);

	UFUNCTION(BlueprintCallable, Category = "Respawn")
	void SchedulePlayerRespawn(AMyFpsCharacter* Character);

	UFUNCTION(BlueprintPure, Category = "Score")
	float GetPlayerScore(AController* PlayerController) const;

	UFUNCTION(BlueprintPure, Category = "Score")
	float GetPlayerKillScore() const { return PlayerKillScore; }

	UFUNCTION(BlueprintCallable, Category = "Match")
	void MarkPlayerReadyForNextRound(AController* PlayerController);

	UFUNCTION(BlueprintCallable, Category = "Match")
	void StartMatchGame();

	UFUNCTION(BlueprintCallable, Category = "Match")
	void EndHostedGame(APlayerController* HostPlayerController);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Match")
	int32 TargetScoreToWin = 500;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Score")
	float PlayerKillScore = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Match")
	float NextRoundStartDelay = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Respawn", meta = (ClampMin = "0.1"))
	float PlayerRespawnDelay = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Match")
	FName MainMenuMapName = TEXT("L_MainMenu");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UMyFpsWeaponDefinition> StartingWeaponDefinition = nullptr;

private:
	void GiveStartingWeaponToCharacter(AMyFpsCharacter* Character);
	void TryHandleVictory(AController* ScoringController);
	void FreezeMatch();
	void SetEnemiesActive(bool bActive);
	void ResetPlayerToPreMatchState(APlayerController* PlayerController);
	void TryStartNextRound();
	void StartNextRoundAfterDelay();
	void ResetRound();

	FTimerHandle NextRoundStartTimerHandle;
};



