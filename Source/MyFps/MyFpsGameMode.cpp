// Copyright Epic Games, Inc. All Rights Reserved.

#include "MyFpsGameMode.h"
#include "EnemyCharacter.h"
#include "MyFpsCharacter.h"
#include "MyFpsGameInstance.h"
#include "MyFpsGameState.h"
#include "MyFpsPlayerController.h"
#include "MyFpsPlayerState.h"
#include "MyFpsWeaponInventoryComponent.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/HUD.h"
#include "UObject/ConstructorHelpers.h"

AMyFpsGameMode::AMyFpsGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;
	PlayerStateClass = AMyFpsPlayerState::StaticClass();
	PlayerControllerClass = AMyFpsPlayerController::StaticClass();
	GameStateClass = AMyFpsGameState::StaticClass();
	HUDClass = AHUD::StaticClass();
}

void AMyFpsGameMode::InitGameState()
{
	Super::InitGameState();

	if (AMyFpsGameState* MyGameState = GetGameState<AMyFpsGameState>())
	{
		MyGameState->SetTargetScore(TargetScoreToWin);
		MyGameState->SetMatchResult(false, FString());
		MyGameState->SetMatchStarted(false);
		MyGameState->SetReadyPlayerCount(0);
		MyGameState->ClearKillFeedMessages();
	}

	SetEnemiesActive(false);
}

void AMyFpsGameMode::StartPlay()
{
	Super::StartPlay();

	UMyFpsGameInstance* MyGameInstance = GetGameInstance<UMyFpsGameInstance>();
	if (MyGameInstance && MyGameInstance->ConsumeAutoStartAfterTravel())
	{
		GetWorldTimerManager().SetTimerForNextTick(this, &AMyFpsGameMode::StartMatchGame);
	}
	else if (MyGameInstance && MyGameInstance->ConsumeHostControlMenuAfterTravel())
	{
		GetWorldTimerManager().SetTimerForNextTick([this, MyGameInstance]()
		{
			if (APlayerController* HostPlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr)
			{
				MyGameInstance->InitializeHostControlMenu(HostPlayerController);
			}
		});
	}
}

void AMyFpsGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	AMyFpsGameState* MyGameState = GetGameState<AMyFpsGameState>();
	if (MyGameState && MyGameState->IsMyFpsMatchStarted() && NewPlayer)
	{
		if (AMyFpsCharacter* Character = Cast<AMyFpsCharacter>(NewPlayer->GetPawn()))
		{
			GiveStartingWeaponToCharacter(Character);
			Character->HandleMatchStarted();
		}

		if (AMyFpsPlayerController* MyPlayerController = Cast<AMyFpsPlayerController>(NewPlayer))
		{
			MyPlayerController->ClientPrepareForMatch();
		}
	}
}

void AMyFpsGameMode::AddScore(AController* ScoringController, float ScoreAmount)
{
	if (!ScoringController || ScoreAmount <= 0.0f)
	{
		return;
	}

	if (const AMyFpsGameState* MyGameState = GetGameState<AMyFpsGameState>())
	{
		if (MyGameState->IsMatchFinished() || !MyGameState->IsMyFpsMatchStarted())
		{
			return;
		}
	}

	AMyFpsPlayerState* PlayerState = ScoringController->GetPlayerState<AMyFpsPlayerState>();
	if (!PlayerState)
	{
		return;
	}

	PlayerState->AddScore(ScoreAmount);

	UE_LOG(LogTemp, Warning, TEXT("%s score increased by %.1f, total score = %.0f"), *ScoringController->GetName(), ScoreAmount, PlayerState->GetCurrentScore());
	TryHandleVictory(ScoringController);
}

void AMyFpsGameMode::RegisterKill(AController* KillerController)
{
	if (!KillerController)
	{
		return;
	}

	AMyFpsPlayerState* PlayerState = KillerController->GetPlayerState<AMyFpsPlayerState>();
	if (!PlayerState)
	{
		return;
	}

	PlayerState->AddKill();
}

void AMyFpsGameMode::RegisterDeath(AController* VictimController)
{
	if (!VictimController)
	{
		return;
	}

	AMyFpsPlayerState* PlayerState = VictimController->GetPlayerState<AMyFpsPlayerState>();
	if (!PlayerState)
	{
		return;
	}

	PlayerState->AddDeath();
}

void AMyFpsGameMode::RegisterKillFeed(AController* KillerController, AController* VictimController)
{
	if (!KillerController || !VictimController)
	{
		return;
	}

	AMyFpsGameState* MyGameState = GetGameState<AMyFpsGameState>();
	if (!MyGameState)
	{
		return;
	}

	const APlayerState* KillerPlayerState = KillerController->PlayerState;
	const APlayerState* VictimPlayerState = VictimController->PlayerState;
	const FString KillerName = KillerPlayerState && !KillerPlayerState->GetPlayerName().IsEmpty()
		? KillerPlayerState->GetPlayerName()
		: KillerController->GetName();
	const FString VictimName = VictimPlayerState && !VictimPlayerState->GetPlayerName().IsEmpty()
		? VictimPlayerState->GetPlayerName()
		: VictimController->GetName();

	MyGameState->AddKillFeedMessage(KillerName, VictimName);
}

void AMyFpsGameMode::RespawnPlayer(AController* PlayerController)
{
	if (!PlayerController)
	{
		return;
	}

	if (const AMyFpsGameState* MyGameState = GetGameState<AMyFpsGameState>())
	{
		if (MyGameState->IsMatchFinished())
		{
			return;
		}
	}

	APawn* ExistingPawn = PlayerController->GetPawn();
	if (ExistingPawn)
	{
		PlayerController->UnPossess();
		ExistingPawn->Destroy();
	}

	RestartPlayer(PlayerController);

	if (AMyFpsCharacter* RespawnedCharacter = Cast<AMyFpsCharacter>(PlayerController->GetPawn()))
	{
		RespawnedCharacter->ResetHealthToMax();
		if (const AMyFpsGameState* MyGameState = GetGameState<AMyFpsGameState>(); MyGameState && MyGameState->IsMyFpsMatchStarted())
		{
			GiveStartingWeaponToCharacter(RespawnedCharacter);
		}
	}
}

void AMyFpsGameMode::SchedulePlayerRespawn(AMyFpsCharacter* Character)
{
	if (!Character)
	{
		return;
	}

	if (const AMyFpsGameState* MyGameState = GetGameState<AMyFpsGameState>())
	{
		if (MyGameState->IsMatchFinished())
		{
			return;
		}
	}

	Character->ScheduleAutoRespawn(PlayerRespawnDelay);
}

float AMyFpsGameMode::GetPlayerScore(AController* PlayerController) const
{
	if (!PlayerController)
	{
		return 0.0f;
	}

	const AMyFpsPlayerState* PlayerState = PlayerController->GetPlayerState<AMyFpsPlayerState>();
	return PlayerState ? PlayerState->GetCurrentScore() : 0.0f;
}

void AMyFpsGameMode::TryHandleVictory(AController* ScoringController)
{
	if (!ScoringController)
	{
		return;
	}

	AMyFpsGameState* MyGameState = GetGameState<AMyFpsGameState>();
	if (!MyGameState || MyGameState->IsMatchFinished())
	{
		return;
	}

	const AMyFpsPlayerState* PlayerState = ScoringController->GetPlayerState<AMyFpsPlayerState>();
	if (!PlayerState || PlayerState->GetCurrentScore() < MyGameState->GetTargetScore())
	{
		return;
	}

	const FString WinnerName = PlayerState->GetPlayerName().IsEmpty()
		? TEXT("Player")
		: PlayerState->GetPlayerName();

	MyGameState->SetMatchResult(true, WinnerName);
	MyGameState->SetReadyPlayerCount(0);

	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		if (AMyFpsPlayerController* PlayerController = Cast<AMyFpsPlayerController>(Iterator->Get()))
		{
			PlayerController->ClientShowMatchResult(WinnerName);
		}
	}

	FreezeMatch();
}

void AMyFpsGameMode::MarkPlayerReadyForNextRound(AController* PlayerController)
{
	if (!PlayerController)
	{
		return;
	}

	AMyFpsGameState* MyGameState = GetGameState<AMyFpsGameState>();
	if (!MyGameState || !MyGameState->IsMatchFinished())
	{
		return;
	}

	AMyFpsPlayerState* PlayerState = PlayerController->GetPlayerState<AMyFpsPlayerState>();
	if (!PlayerState || PlayerState->IsReadyForNextRound())
	{
		return;
	}

	PlayerState->SetReadyForNextRound(true);
	TryStartNextRound();
}

void AMyFpsGameMode::StartMatchGame()
{
	AMyFpsGameState* MyGameState = GetGameState<AMyFpsGameState>();
	if (!MyGameState || MyGameState->IsMyFpsMatchStarted())
	{
		return;
	}

	MyGameState->SetMatchStarted(true);
	SetEnemiesActive(true);

	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PlayerController = Iterator->Get();
		if (!PlayerController)
		{
			continue;
		}

		if (AMyFpsPlayerState* PlayerState = PlayerController->GetPlayerState<AMyFpsPlayerState>())
		{
			PlayerState->ResetRoundState();
		}

		if (AMyFpsCharacter* Character = Cast<AMyFpsCharacter>(PlayerController->GetPawn()))
		{
			GiveStartingWeaponToCharacter(Character);
			Character->HandleMatchStarted();
		}

		if (AMyFpsPlayerController* MyPlayerController = Cast<AMyFpsPlayerController>(PlayerController))
		{
			MyPlayerController->ClientPrepareForMatch();
		}
	}
}

void AMyFpsGameMode::EndHostedGame(APlayerController* HostPlayerController)
{
	if (!HostPlayerController || !HostPlayerController->HasAuthority())
	{
		return;
	}

	AMyFpsGameState* MyGameState = GetGameState<AMyFpsGameState>();
	if (!MyGameState)
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(NextRoundStartTimerHandle);
	MyGameState->SetMatchStarted(false);
	MyGameState->SetMatchResult(false, FString());
	MyGameState->SetReadyPlayerCount(0);
	MyGameState->ClearKillFeedMessages();
	SetEnemiesActive(false);

	TArray<AMyFpsPlayerController*> RemotePlayerControllers;
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		AMyFpsPlayerController* PlayerController = Cast<AMyFpsPlayerController>(Iterator->Get());
		if (!PlayerController)
		{
			continue;
		}

		if (PlayerController == HostPlayerController)
		{
			ResetPlayerToPreMatchState(PlayerController);
		}
		else
		{
			RemotePlayerControllers.Add(PlayerController);
		}
	}

	for (AMyFpsPlayerController* RemotePlayerController : RemotePlayerControllers)
	{
		RemotePlayerController->ClientReturnToMainMenu(MainMenuMapName);
	}
}

void AMyFpsGameMode::ResetPlayerToPreMatchState(APlayerController* PlayerController)
{
	if (!PlayerController)
	{
		return;
	}

	if (AMyFpsPlayerState* PlayerState = PlayerController->GetPlayerState<AMyFpsPlayerState>())
	{
		PlayerState->ResetRoundState();
	}

	if (AMyFpsCharacter* Character = Cast<AMyFpsCharacter>(PlayerController->GetPawn()))
	{
		Character->CancelAutoRespawn();
		if (UMyFpsWeaponInventoryComponent* InventoryComponent = Character->GetWeaponInventoryComponent())
		{
			InventoryComponent->ClearCurrentWeapon();
		}
	}

	RespawnPlayer(PlayerController);
}

void AMyFpsGameMode::GiveStartingWeaponToCharacter(AMyFpsCharacter* Character)
{
	if (!Character || Character->HasWeaponEquipped())
	{
		return;
	}

	if (StartingWeaponDefinition)
	{
		if (UMyFpsWeaponInventoryComponent* InventoryComponent = Character->GetWeaponInventoryComponent())
		{
			InventoryComponent->EquipWeaponDefinition(StartingWeaponDefinition);
		}
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[Weapon] StartingWeaponDefinition is not set on %s. No starting weapon was equipped."), *GetNameSafe(this));
}

void AMyFpsGameMode::FreezeMatch()
{
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PlayerController = Iterator->Get();
		if (!PlayerController)
		{
			continue;
		}

		if (AMyFpsCharacter* Character = Cast<AMyFpsCharacter>(PlayerController->GetPawn()))
		{
			Character->HandleMatchFinished();
		}
	}

	for (TActorIterator<AEnemyCharacter> EnemyIterator(GetWorld()); EnemyIterator; ++EnemyIterator)
	{
		EnemyIterator->HandleMatchFinished();
	}
}

void AMyFpsGameMode::SetEnemiesActive(bool bActive)
{
	for (TActorIterator<AEnemyCharacter> EnemyIterator(GetWorld()); EnemyIterator; ++EnemyIterator)
	{
		if (bActive)
		{
			EnemyIterator->ForceRespawnNow();
		}
		else
		{
			EnemyIterator->HandleMatchFinished();
		}
	}
}

void AMyFpsGameMode::TryStartNextRound()
{
	AMyFpsGameState* MyGameState = GetGameState<AMyFpsGameState>();
	if (!MyGameState || !GameState)
	{
		return;
	}

	int32 PlayerCount = 0;
	int32 ReadyCount = 0;

	for (APlayerState* PlayerStateBase : GameState->PlayerArray)
	{
		AMyFpsPlayerState* PlayerState = Cast<AMyFpsPlayerState>(PlayerStateBase);
		if (!PlayerState)
		{
			continue;
		}

		++PlayerCount;
		if (PlayerState->IsReadyForNextRound())
		{
			++ReadyCount;
		}
	}

	MyGameState->SetReadyPlayerCount(ReadyCount);

	if (PlayerCount > 0 && ReadyCount >= PlayerCount)
	{
		if (UWorld* World = GetWorld())
		{
			if (!World->GetTimerManager().IsTimerActive(NextRoundStartTimerHandle))
			{
				World->GetTimerManager().SetTimer(
					NextRoundStartTimerHandle,
					this,
					&AMyFpsGameMode::StartNextRoundAfterDelay,
					FMath::Max(0.0f, NextRoundStartDelay),
					false
				);
			}
		}
	}
}

void AMyFpsGameMode::StartNextRoundAfterDelay()
{
	AMyFpsGameState* MyGameState = GetGameState<AMyFpsGameState>();
	if (!MyGameState || !GameState)
	{
		return;
	}

	for (APlayerState* PlayerStateBase : GameState->PlayerArray)
	{
		if (AMyFpsPlayerState* PlayerState = Cast<AMyFpsPlayerState>(PlayerStateBase))
		{
			PlayerState->ResetRoundState();
		}
	}

	MyGameState->SetReadyPlayerCount(0);
	MyGameState->SetMatchResult(false, FString());
	MyGameState->ClearKillFeedMessages();
	ResetRound();
}

void AMyFpsGameMode::ResetRound()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Fully restart the current map so dropped weapons, AI state, physics, and widgets
	// all return to their initial world state for the next round.
	if (UMyFpsGameInstance* MyGameInstance = GetGameInstance<UMyFpsGameInstance>())
	{
		MyGameInstance->RequestAutoStartAfterTravel();
	}

	World->ServerTravel(TEXT("?Restart"));
}
