#include "MyFpsMenuPlayerController.h"
#include "Blueprint/UserWidget.h"

void AMyFpsMenuPlayerController::BeginPlay()
{
    Super::BeginPlay();
	if (!IsLocalController())
{
	return;
}
	bShowMouseCursor = true;

	FInputModeUIOnly InputMode;
	SetInputMode(InputMode);

	if (MainMenuClass)
	{
		MainMenuWidget = CreateWidget<UUserWidget>(this, MainMenuClass);

		if (MainMenuWidget)
		{
			MainMenuWidget->AddToViewport();
		}
	}
}
