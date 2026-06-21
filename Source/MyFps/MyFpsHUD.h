#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MyFpsHUD.generated.h"

UCLASS()
class MYFPS_API AMyFpsHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Score")
	FVector2D ScorePosition = FVector2D(50.0f, 50.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Score")
	FLinearColor ScoreColor = FLinearColor::White;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Score")
	float ScoreScale = 1.5f;
};
