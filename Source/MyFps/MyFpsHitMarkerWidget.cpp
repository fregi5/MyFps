// Copyright Epic Games, Inc. All Rights Reserved.

#include "MyFpsHitMarkerWidget.h"

#include "Animation/WidgetAnimation.h"
#include "Kismet/GameplayStatics.h"

void UMyFpsHitMarkerWidget::PlayHitMarker(bool bKill)
{
	SetVisibility(ESlateVisibility::HitTestInvisible);

	if (HitBodySound)
	{
		UGameplayStatics::PlaySound2D(this, HitBodySound);
	}

	UWidgetAnimation* AnimationToPlay = bKill ? HitMarker_ShowKill : HitMarker_Show;
	if (!AnimationToPlay)
	{
		UE_LOG(LogTemp, Warning, TEXT("[HitMarker] Animation binding failed. Widget=%s Expected=%s"), *GetNameSafe(this), bKill ? TEXT("HitMarker_ShowKill") : TEXT("HitMarker_Show"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[HitMarker] Playing animation %s on widget %s."), *GetNameSafe(AnimationToPlay), *GetNameSafe(this));
	StopAnimation(AnimationToPlay);
	PlayAnimation(AnimationToPlay, 0.0f, 1, EUMGSequencePlayMode::Forward, 1.0f, false);
}
