#include "MyFpsHUD.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "MyFpsPlayerState.h"

void AMyFpsHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Canvas)
	{
		return;
	}

	APlayerController* PlayerController = GetOwningPlayerController();
	if (!PlayerController)
	{
		return;
	}

	const AMyFpsPlayerState* PlayerState = PlayerController->GetPlayerState<AMyFpsPlayerState>();
	if (!PlayerState)
	{
		return;
	}

	const FString ScoreText = FString::Printf(TEXT("Score: %.0f"), PlayerState->GetCurrentScore());

	FCanvasTextItem TextItem(ScorePosition, FText::FromString(ScoreText), GEngine->GetSmallFont(), ScoreColor);
	TextItem.Scale = FVector2D(ScoreScale, ScoreScale);
	TextItem.EnableShadow(FLinearColor::Black);
	Canvas->DrawItem(TextItem);
}
