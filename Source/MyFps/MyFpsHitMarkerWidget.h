// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MyFpsHitMarkerWidget.generated.h"

UCLASS()
class MYFPS_API UMyFpsHitMarkerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "HitMarker")
	void PlayHitMarker(bool bKill);

protected:
	UPROPERTY(Transient, BlueprintReadOnly, Category = "HitMarker", meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> HitMarker_Show = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "HitMarker", meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> HitMarker_ShowKill = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HitMarker|Sound")
	TObjectPtr<USoundBase> HitBodySound = nullptr;
};
