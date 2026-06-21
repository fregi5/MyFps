#include "MyFpsPlayerState.h"
#include "MyFpsCharacter.h"
#include "Net/UnrealNetwork.h"

void AMyFpsPlayerState::AddScore(float ScoreAmount)
{
	if (ScoreAmount <= 0.0f)
	{
		return;
	}

	SetScore(GetScore() + ScoreAmount);
}

void AMyFpsPlayerState::AddKill()
{
	++Kills;
}

void AMyFpsPlayerState::AddDeath()
{
	++Deaths;
}

float AMyFpsPlayerState::GetCurrentScore() const
{
	return GetScore();
}

int32 AMyFpsPlayerState::GetKills() const
{
	return Kills;
}

int32 AMyFpsPlayerState::GetDeaths() const
{
	return Deaths;
}

void AMyFpsPlayerState::SetReadyForNextRound(bool bReady)
{
	bReadyForNextRound = bReady;
}

bool AMyFpsPlayerState::IsReadyForNextRound() const
{
	return bReadyForNextRound;
}

void AMyFpsPlayerState::ResetRoundState()
{
	SetScore(0.0f);
	Kills = 0;
	Deaths = 0;
	bReadyForNextRound = false;
}

void AMyFpsPlayerState::OnRep_ReadyForNextRound()
{
	if (AController* OwnerController = GetOwningController())
	{
		if (AMyFpsCharacter* PlayerCharacter = Cast<AMyFpsCharacter>(OwnerController->GetPawn()))
		{
			PlayerCharacter->RefreshMatchResultWidget();
		}
	}
}

void AMyFpsPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMyFpsPlayerState, Kills);
	DOREPLIFETIME(AMyFpsPlayerState, Deaths);
	DOREPLIFETIME(AMyFpsPlayerState, bReadyForNextRound);
}
