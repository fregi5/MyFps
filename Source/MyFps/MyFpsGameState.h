#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "MyFpsGameState.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnMatchStateChanged);

USTRUCT(BlueprintType)
struct FMyFpsKillFeedMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Kill Feed")
	FString KillerName;

	UPROPERTY(BlueprintReadOnly, Category = "Kill Feed")
	FString VictimName;
};

UCLASS()
class MYFPS_API AMyFpsGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Match")
	void SetTargetScore(int32 InTargetScore);

	UFUNCTION(BlueprintCallable, Category = "Match")
	void SetMatchResult(bool bInMatchFinished, const FString& InWinnerName);

	UFUNCTION(BlueprintPure, Category = "Match")
	int32 GetTargetScore() const { return TargetScore; }

	UFUNCTION(BlueprintPure, Category = "Match")
	bool IsMatchFinished() const { return bMatchFinished; }

	UFUNCTION(BlueprintCallable, Category = "Match")
	void SetMatchStarted(bool bInMatchStarted);

	UFUNCTION(BlueprintPure, Category = "Match")
	bool IsMyFpsMatchStarted() const { return bMatchStarted; }

	UFUNCTION(BlueprintPure, Category = "Match")
	FString GetWinnerName() const { return WinnerName; }

	UFUNCTION(BlueprintCallable, Category = "Match")
	void SetReadyPlayerCount(int32 InReadyPlayerCount);

	UFUNCTION(BlueprintPure, Category = "Match")
	int32 GetReadyPlayerCount() const { return ReadyPlayerCount; }

	void AddKillFeedMessage(const FString& KillerName, const FString& VictimName);
	void ClearKillFeedMessages();

	UFUNCTION(BlueprintPure, Category = "Kill Feed")
	const TArray<FMyFpsKillFeedMessage>& GetKillFeedMessages() const { return KillFeedMessages; }

	FOnMatchStateChanged& OnMatchStateChanged() { return MatchStateChangedDelegate; }

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UFUNCTION()
	void OnRep_TargetScore();

	UFUNCTION()
	void OnRep_MatchFinished();

	UFUNCTION()
	void OnRep_MatchStarted();

	UFUNCTION()
	void OnRep_WinnerName();

	UFUNCTION()
	void OnRep_ReadyPlayerCount();

	UFUNCTION()
	void OnRep_KillFeedMessages();

	UPROPERTY(ReplicatedUsing = OnRep_TargetScore, BlueprintReadOnly, Category = "Match")
	int32 TargetScore = 500;

	UPROPERTY(ReplicatedUsing = OnRep_MatchFinished, BlueprintReadOnly, Category = "Match")
	bool bMatchFinished = false;

	UPROPERTY(ReplicatedUsing = OnRep_MatchStarted, BlueprintReadOnly, Category = "Match")
	bool bMatchStarted = false;

	UPROPERTY(ReplicatedUsing = OnRep_WinnerName, BlueprintReadOnly, Category = "Match")
	FString WinnerName;

	UPROPERTY(ReplicatedUsing = OnRep_ReadyPlayerCount, BlueprintReadOnly, Category = "Match")
	int32 ReadyPlayerCount = 0;

	UPROPERTY(ReplicatedUsing = OnRep_KillFeedMessages, BlueprintReadOnly, Category = "Kill Feed")
	TArray<FMyFpsKillFeedMessage> KillFeedMessages;

private:
	static constexpr int32 MaxKillFeedMessages = 3;

	void BroadcastMatchStateChanged();

	FOnMatchStateChanged MatchStateChangedDelegate;
};
