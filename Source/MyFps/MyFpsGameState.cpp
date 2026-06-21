#include "MyFpsGameState.h"
#include "Net/UnrealNetwork.h"

void AMyFpsGameState::SetTargetScore(int32 InTargetScore)
{
	TargetScore = FMath::Max(1, InTargetScore);
	BroadcastMatchStateChanged();
}

void AMyFpsGameState::SetMatchResult(bool bInMatchFinished, const FString& InWinnerName)
{
	bMatchFinished = bInMatchFinished;
	WinnerName = InWinnerName;
	BroadcastMatchStateChanged();
}

void AMyFpsGameState::SetMatchStarted(bool bInMatchStarted)
{
	bMatchStarted = bInMatchStarted;
	BroadcastMatchStateChanged();
}

void AMyFpsGameState::SetReadyPlayerCount(int32 InReadyPlayerCount)
{
	ReadyPlayerCount = FMath::Max(0, InReadyPlayerCount);
	BroadcastMatchStateChanged();
}

void AMyFpsGameState::AddKillFeedMessage(const FString& KillerName, const FString& VictimName)
{
	FMyFpsKillFeedMessage Message;
	Message.KillerName = KillerName.IsEmpty() ? TEXT("Player") : KillerName;
	Message.VictimName = VictimName.IsEmpty() ? TEXT("Player") : VictimName;

	KillFeedMessages.Insert(Message, 0);
	if (KillFeedMessages.Num() > MaxKillFeedMessages)
	{
		KillFeedMessages.SetNum(MaxKillFeedMessages);
	}

	BroadcastMatchStateChanged();
}

void AMyFpsGameState::ClearKillFeedMessages()
{
	KillFeedMessages.Reset();
	BroadcastMatchStateChanged();
}

void AMyFpsGameState::OnRep_TargetScore()
{
	BroadcastMatchStateChanged();
}

void AMyFpsGameState::OnRep_MatchFinished()
{
	BroadcastMatchStateChanged();
}

void AMyFpsGameState::OnRep_MatchStarted()
{
	BroadcastMatchStateChanged();
}

void AMyFpsGameState::OnRep_WinnerName()
{
	BroadcastMatchStateChanged();
}

void AMyFpsGameState::OnRep_ReadyPlayerCount()
{
	BroadcastMatchStateChanged();
}

void AMyFpsGameState::OnRep_KillFeedMessages()
{
	BroadcastMatchStateChanged();
}

void AMyFpsGameState::BroadcastMatchStateChanged()
{
	MatchStateChangedDelegate.Broadcast();
}

void AMyFpsGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMyFpsGameState, TargetScore);
	DOREPLIFETIME(AMyFpsGameState, bMatchFinished);
	DOREPLIFETIME(AMyFpsGameState, bMatchStarted);
	DOREPLIFETIME(AMyFpsGameState, WinnerName);
	DOREPLIFETIME(AMyFpsGameState, ReadyPlayerCount);
	DOREPLIFETIME(AMyFpsGameState, KillFeedMessages);
}
