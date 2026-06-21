#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "MyFpsPlayerState.generated.h"

UCLASS()
class MYFPS_API AMyFpsPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Score")
	void AddScore(float ScoreAmount);

	UFUNCTION(BlueprintCallable, Category = "Score")
	void AddKill();

	UFUNCTION(BlueprintCallable, Category = "Score")
	void AddDeath();

	UFUNCTION(BlueprintPure, Category = "Score")
	float GetCurrentScore() const;

	UFUNCTION(BlueprintPure, Category = "Score")
	int32 GetKills() const;

	UFUNCTION(BlueprintPure, Category = "Score")
	int32 GetDeaths() const;

	UFUNCTION(BlueprintCallable, Category = "Match")
	void SetReadyForNextRound(bool bReady);

	UFUNCTION(BlueprintPure, Category = "Match")
	bool IsReadyForNextRound() const;

	UFUNCTION(BlueprintCallable, Category = "Match")
	void ResetRoundState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UFUNCTION()
	void OnRep_ReadyForNextRound();

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Score")
	int32 Kills = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Score")
	int32 Deaths = 0;

	UPROPERTY(ReplicatedUsing = OnRep_ReadyForNextRound, BlueprintReadOnly, Category = "Match")
	bool bReadyForNextRound = false;
};
