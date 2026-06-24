#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MyFpsMenuPlayerController.generated.h"

class UUserWidget;

UCLASS()
class MYFPS_API AMyFpsMenuPlayerController: public APlayerController
{
    GENERATED_BODY()

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditDefaultsOnly, Category= "UI")
    TSubclassOf<UUserWidget> MainMenuClass;

    UPROPERTY()
	UUserWidget* MainMenuWidget;
};