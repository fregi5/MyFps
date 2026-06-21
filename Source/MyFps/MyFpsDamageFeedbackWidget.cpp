// Copyright Epic Games, Inc. All Rights Reserved.

#include "MyFpsDamageFeedbackWidget.h"

#include "Camera/PlayerCameraManager.h"
#include "Components/Image.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Materials/MaterialInstanceDynamic.h"

namespace MyFpsDamageFeedbackParameters
{
	constexpr int32 MaxSupportedSources = 4;
	const FName DamageAngles[MaxSupportedSources] = {
		TEXT("DamageAngle0"), TEXT("DamageAngle1"), TEXT("DamageAngle2"), TEXT("DamageAngle3")
	};
	const FName DamageIntensities[MaxSupportedSources] = {
		TEXT("DamageIntensity0"), TEXT("DamageIntensity1"), TEXT("DamageIntensity2"), TEXT("DamageIntensity3")
	};
}

void UMyFpsDamageFeedbackWidget::PlayDamageFeedback(const FVector& SourceWorldLocation)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const float Now = World->GetTimeSeconds();
	const float MergeDistanceSquared = FMath::Square(SourceMergeDistance);
	for (FActiveDamageSource& ActiveSource : ActiveDamageSources)
	{
		if (FVector::DistSquared(ActiveSource.SourceWorldLocation, SourceWorldLocation) <= MergeDistanceSquared)
		{
			ActiveSource.SourceWorldLocation = SourceWorldLocation;
			ActiveSource.StartTime = Now;
			SetVisibility(ESlateVisibility::HitTestInvisible);
			UpdateRingMaterial();
			return;
		}
	}

	const int32 SourceLimit = FMath::Clamp(MaxSimultaneousSources, 1, MyFpsDamageFeedbackParameters::MaxSupportedSources);
	if (ActiveDamageSources.Num() >= SourceLimit)
	{
		int32 OldestSourceIndex = 0;
		for (int32 Index = 1; Index < ActiveDamageSources.Num(); ++Index)
		{
			if (ActiveDamageSources[Index].StartTime < ActiveDamageSources[OldestSourceIndex].StartTime)
			{
				OldestSourceIndex = Index;
			}
		}
		ActiveDamageSources.RemoveAt(OldestSourceIndex);
	}

	ActiveDamageSources.Add({ SourceWorldLocation, Now });
	SetVisibility(ESlateVisibility::HitTestInvisible);
	UpdateRingMaterial();
}

void UMyFpsDamageFeedbackWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (ActiveDamageSources.IsEmpty())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		HideDamageFeedback();
		return;
	}

	const float Now = World->GetTimeSeconds();
	ActiveDamageSources.RemoveAll([this, Now](const FActiveDamageSource& ActiveSource)
	{
		return Now - ActiveSource.StartTime >= FMath::Max(0.05f, DisplayDuration);
	});

	if (ActiveDamageSources.IsEmpty())
	{
		HideDamageFeedback();
		return;
	}

	UpdateRingMaterial();
}

void UMyFpsDamageFeedbackWidget::HideDamageFeedback()
{
	ActiveDamageSources.Reset();
	UpdateRingMaterial();
	SetVisibility(ESlateVisibility::Hidden);
}

bool UMyFpsDamageFeedbackWidget::UpdateRingMaterial()
{
	if (!DamageRing)
	{
		return false;
	}

	if (!DamageRingMaterial)
	{
		DamageRingMaterial = DamageRing->GetDynamicMaterial();
	}

	if (!DamageRingMaterial)
	{
		return false;
	}

	const UWorld* World = GetWorld();
	const float Now = World ? World->GetTimeSeconds() : 0.0f;
	for (int32 Index = 0; Index < MyFpsDamageFeedbackParameters::MaxSupportedSources; ++Index)
	{
		float DamageAngle = 0.0f;
		float DamageIntensity = 0.0f;
		if (ActiveDamageSources.IsValidIndex(Index))
		{
			const FActiveDamageSource& ActiveSource = ActiveDamageSources[Index];
			DamageAngle = CalculateDamageAngle(ActiveSource.SourceWorldLocation);
			DamageIntensity = FMath::Clamp(1.0f - ((Now - ActiveSource.StartTime) / FMath::Max(0.05f, DisplayDuration)), 0.0f, 1.0f);
		}

		DamageRingMaterial->SetScalarParameterValue(MyFpsDamageFeedbackParameters::DamageAngles[Index], DamageAngle);
		DamageRingMaterial->SetScalarParameterValue(MyFpsDamageFeedbackParameters::DamageIntensities[Index], DamageIntensity);
	}
	return true;
}

float UMyFpsDamageFeedbackWidget::CalculateDamageAngle(const FVector& SourceWorldLocation) const
{
	const APlayerController* PlayerController = GetOwningPlayer();
	const APawn* PlayerPawn = PlayerController ? PlayerController->GetPawn() : nullptr;
	if (!PlayerController || !PlayerPawn)
	{
		return 0.0f;
	}

	FVector DirectionToSource = SourceWorldLocation - PlayerPawn->GetActorLocation();
	DirectionToSource.Z = 0.0f;
	if (!DirectionToSource.Normalize())
	{
		return 0.0f;
	}

	const FRotator ViewRotation = PlayerController->PlayerCameraManager
		? PlayerController->PlayerCameraManager->GetCameraRotation()
		: PlayerController->GetControlRotation();
	const FVector Forward = ViewRotation.Vector().GetSafeNormal2D();
	const FVector Right = FRotationMatrix(ViewRotation).GetUnitAxis(EAxis::Y).GetSafeNormal2D();
	const float AngleRadians = FMath::Atan2(
		FVector::DotProduct(DirectionToSource, Right),
		FVector::DotProduct(DirectionToSource, Forward));

	return FMath::Fmod((AngleRadians / (2.0f * PI)) + 1.0f, 1.0f);
}
