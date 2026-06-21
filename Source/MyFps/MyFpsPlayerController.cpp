#include "MyFpsPlayerController.h"

#include "MyFpsCharacter.h"
#include "MyFpsGameMode.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

void AMyFpsPlayerController::ClientShowMatchResult_Implementation(const FString& WinnerName)
{
	if (AMyFpsCharacter* PlayerCharacter = Cast<AMyFpsCharacter>(GetPawn()))
	{
		PlayerCharacter->ShowMatchResultFromServer(WinnerName);
	}
}

void AMyFpsPlayerController::ClientPrepareForMatch_Implementation()
{
	bShowMouseCursor = false;
	ResetIgnoreMoveInput();
	ResetIgnoreLookInput();
	FlushPressedKeys();
	UWidgetBlueprintLibrary::SetInputMode_GameOnly(this);

	if (AMyFpsCharacter* PlayerCharacter = Cast<AMyFpsCharacter>(GetPawn()))
	{
		PlayerCharacter->HandleMatchStarted();
	}
}

void AMyFpsPlayerController::ClientShowHitMarker_Implementation(bool bKill)
{
	UE_LOG(LogTemp, Warning, TEXT("[HitMarker] Client RPC received. Controller=%s Pawn=%s Kill=%d"), *GetNameSafe(this), *GetNameSafe(GetPawn()), bKill);

	if (AMyFpsCharacter* PlayerCharacter = Cast<AMyFpsCharacter>(GetPawn()))
	{
		PlayerCharacter->ShowHitMarkerFromServer(bKill);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[HitMarker] Client RPC has no MyFpsCharacter pawn."));
	}
}

void AMyFpsPlayerController::ClientShowDamageFeedback_Implementation(const FVector_NetQuantize& SourceWorldLocation)
{
	if (AMyFpsCharacter* PlayerCharacter = Cast<AMyFpsCharacter>(GetPawn()))
	{
		PlayerCharacter->ShowDamageFeedbackFromServer(SourceWorldLocation);
	}
}

void AMyFpsPlayerController::ClientStartRespawnCountdown_Implementation(float DelaySeconds)
{
	if (AMyFpsCharacter* PlayerCharacter = Cast<AMyFpsCharacter>(GetPawn()))
	{
		PlayerCharacter->ShowRespawnCountdownFromServer(DelaySeconds);
	}
}

void AMyFpsPlayerController::ServerSetReadyForNextRound_Implementation()
{
	if (AMyFpsGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AMyFpsGameMode>() : nullptr)
	{
		GameMode->MarkPlayerReadyForNextRound(this);
	}
}
