// Copyright Epic Games, Inc. All Rights Reserved.

#include "MyFpsWeaponRecoilComponent.h"

#include "MyFpsCharacter.h"
#include "MyFpsWeaponDefinition.h"
#include "MyFpsWeaponInventoryComponent.h"

UMyFpsWeaponRecoilComponent::UMyFpsWeaponRecoilComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UMyFpsWeaponRecoilComponent::BeginPlay()
{
	Super::BeginPlay();
	CharacterOwner = Cast<AMyFpsCharacter>(GetOwner());
	InventoryComponent = CharacterOwner ? CharacterOwner->GetWeaponInventoryComponent() : nullptr;
	LastObservedWeaponId = InventoryComponent
		? InventoryComponent->GetCurrentWeaponState().WeaponId
		: NAME_None;
}

void UMyFpsWeaponRecoilComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	CheckWeaponChanged();
	ApplyQueuedRecoil(DeltaTime);
	RecoverRecoil(DeltaTime);
}

void UMyFpsWeaponRecoilComponent::ApplyWeaponRecoil(const UMyFpsWeaponDefinition* WeaponDefinition)
{
	ApplyWeaponRecoilWithAmmoModifier(WeaponDefinition, FMyFpsAmmoRecoilModifier());
}

void UMyFpsWeaponRecoilComponent::ApplyWeaponRecoilWithAmmoModifier(
	const UMyFpsWeaponDefinition* WeaponDefinition,
	const FMyFpsAmmoRecoilModifier& AmmoModifier)
{
	if (!WeaponDefinition)
	{
		return;
	}

	if (!CharacterOwner)
	{
		CharacterOwner = Cast<AMyFpsCharacter>(GetOwner());
	}

	if (!CharacterOwner || !CharacterOwner->IsLocallyControlled() || CharacterOwner->IsDead())
	{
		return;
	}

	const float Now = GetWorldTimeSeconds();
	if (LastShotTime < 0.0f || Now - LastShotTime >= WeaponDefinition->RecoilResetTime)
	{
		CurrentShotIndex = 0;
	}

	float PitchUp = WeaponDefinition->VerticalRecoilPerShot;
	float YawOffset = WeaponDefinition->HorizontalRecoilPerShot;
	float RandomYawMin = 0.0f;
	float RandomYawMax = 0.0f;
	float RecoveryDelay = WeaponDefinition->RecoilResetTime;
	float RecoverySpeed = WeaponDefinition->RecoilRecoverySpeed;
	float ViewKickScale = 1.0f;
	bool bUsedPatternShot = false;
	int32 MatchedStartShot = INDEX_NONE;
	int32 MatchedEndShot = INDEX_NONE;
	float RawPatternPitchUp = 0.0f;
	float RawPatternYawOffset = 0.0f;

	if (WeaponDefinition->bUseRecoilPattern && WeaponDefinition->RecoilPatternMode == EMyFpsRecoilPatternMode::PerShotArray)
	{
		const int32 OneBasedShotIndex = CurrentShotIndex + 1;
		const FMyFpsRecoilPatternShot* PatternShot = WeaponDefinition->RecoilPatternShots.FindByPredicate(
			[OneBasedShotIndex](const FMyFpsRecoilPatternShot& Candidate)
			{
				const int32 StartShot = FMath::Max(1, Candidate.StartShot);
				const int32 EndShot = FMath::Max(StartShot, Candidate.EndShot);
				return OneBasedShotIndex >= StartShot && OneBasedShotIndex <= EndShot;
			});

		if (PatternShot)
		{
			bUsedPatternShot = true;
			MatchedStartShot = PatternShot->StartShot;
			MatchedEndShot = PatternShot->EndShot;
			RawPatternPitchUp = PatternShot->PitchUp;
			RawPatternYawOffset = PatternShot->YawOffset;
			PitchUp = PatternShot->PitchUp;
			YawOffset = PatternShot->YawOffset;
			RandomYawMin = PatternShot->RandomYawMin;
			RandomYawMax = PatternShot->RandomYawMax;
			RecoveryDelay = PatternShot->RecoveryDelay >= 0.0f
				? PatternShot->RecoveryDelay
				: WeaponDefinition->RecoilResetTime;
			RecoverySpeed = PatternShot->RecoverySpeed >= 0.0f
				? PatternShot->RecoverySpeed
				: WeaponDefinition->RecoilRecoverySpeed;
			ViewKickScale = PatternShot->ViewKickScale;
		}
	}

	if (!FMath::IsNearlyEqual(RandomYawMin, 0.0f) || !FMath::IsNearlyEqual(RandomYawMax, 0.0f))
	{
		if (RandomYawMin > RandomYawMax)
		{
			Swap(RandomYawMin, RandomYawMax);
		}
		YawOffset += FMath::FRandRange(RandomYawMin, RandomYawMax);
	}

	PitchUp *= AmmoModifier.PitchScale * ViewKickScale * AmmoModifier.ViewKickScale;
	YawOffset *= AmmoModifier.YawScale * ViewKickScale * AmmoModifier.ViewKickScale;

	const float MaxPitch = FMath::Max(0.0f, WeaponDefinition->MaxRecoilPitch);
	if (MaxPitch > 0.0f)
	{
		const float AllowedPitch = FMath::Max(0.0f, MaxPitch - PendingPitchRecovery - QueuedPitchKick);
		PitchUp = FMath::Min(PitchUp, AllowedPitch);
	}

	const float MaxYaw = FMath::Max(0.0f, WeaponDefinition->MaxRecoilYaw);
	if (MaxYaw > 0.0f)
	{
		const float AllowedYaw = FMath::Max(0.0f, MaxYaw - FMath::Abs(PendingYawRecovery + QueuedYawKick));
		YawOffset = FMath::Clamp(YawOffset, -AllowedYaw, AllowedYaw);
	}

	UE_LOG(LogTemp, Warning, TEXT("[Recoil] Apply Weapon=%s Shot=%d Source=%s Range=%d-%d RawPitch=%.3f RawYaw=%.3f ViewKickScale=%.3f Pitch=%.3f Yaw=%.3f KickSpeed=%.3f RecoveryDelay=%.3f RecoverySpeed=%.3f PendingPitch=%.3f PendingYaw=%.3f QueuedPitch=%.3f QueuedYaw=%.3f"),
		*GetNameSafe(WeaponDefinition),
		CurrentShotIndex + 1,
		bUsedPatternShot ? TEXT("Pattern") : TEXT("WeaponDefault"),
		MatchedStartShot,
		MatchedEndShot,
		RawPatternPitchUp,
		RawPatternYawOffset,
		ViewKickScale,
		PitchUp,
		YawOffset,
		WeaponDefinition->RecoilKickSpeed,
		RecoveryDelay,
		RecoverySpeed,
		PendingPitchRecovery,
		PendingYawRecovery,
		QueuedPitchKick,
		QueuedYawKick);

	CurrentKickSpeed = FMath::Max(0.01f, WeaponDefinition->RecoilKickSpeed);
	QueuedPitchKick += PitchUp;
	QueuedYawKick += YawOffset;
	CurrentRecoverySpeed = FMath::Max(0.0f, RecoverySpeed * AmmoModifier.RecoverySpeedScale);
	RecoveryStartTime = Now + FMath::Max(0.0f, RecoveryDelay * AmmoModifier.RecoveryDelayScale);
	LastShotTime = Now;
	++CurrentShotIndex;
}

void UMyFpsWeaponRecoilComponent::ResetRecoilState()
{
	ResetShotIndex();

	// 重置连射时不再瞬间拉回视角，只跳过恢复等待时间，让准心按恢复速度自然回正。
	RecoveryStartTime = GetWorldTimeSeconds();
	if (CurrentRecoverySpeed <= 0.0f)
	{
		const UMyFpsWeaponDefinition* WeaponDefinition = InventoryComponent
			? InventoryComponent->GetCurrentWeaponDefinition()
			: nullptr;
		CurrentRecoverySpeed = WeaponDefinition
			? FMath::Max(0.0f, WeaponDefinition->RecoilRecoverySpeed)
			: 0.0f;
	}

	CurrentKickSpeed = 0.0f;
	QueuedPitchKick = 0.0f;
	QueuedYawKick = 0.0f;
}

void UMyFpsWeaponRecoilComponent::ResetShotIndex()
{
	CurrentShotIndex = 0;
	LastShotTime = -1.0f;
}

void UMyFpsWeaponRecoilComponent::ApplyQueuedRecoil(float DeltaTime)
{
	if (QueuedPitchKick <= 0.0f && FMath::IsNearlyZero(QueuedYawKick))
	{
		return;
	}

	if (!CharacterOwner)
	{
		CharacterOwner = Cast<AMyFpsCharacter>(GetOwner());
	}

	if (!CharacterOwner || !CharacterOwner->IsLocallyControlled() || CharacterOwner->IsDead())
	{
		return;
	}

	const float QueuedMagnitude = FMath::Sqrt(FMath::Square(QueuedPitchKick) + FMath::Square(QueuedYawKick));
	if (QueuedMagnitude <= UE_KINDA_SMALL_NUMBER)
	{
		QueuedPitchKick = 0.0f;
		QueuedYawKick = 0.0f;
		return;
	}

	const float StepAlpha = FMath::Min(1.0f, FMath::Max(0.01f, CurrentKickSpeed) * DeltaTime / QueuedMagnitude);
	const float PitchStep = QueuedPitchKick * StepAlpha;
	const float YawStep = QueuedYawKick * StepAlpha;

	if (PitchStep <= 0.0f && FMath::IsNearlyZero(YawStep))
	{
		return;
	}

	ApplyControllerRecoil(PitchStep, YawStep);
	PendingPitchRecovery += PitchStep;
	PendingYawRecovery += YawStep;
	QueuedPitchKick -= PitchStep;
	QueuedYawKick -= YawStep;
}

void UMyFpsWeaponRecoilComponent::RecoverRecoil(float DeltaTime)
{
	if (PendingPitchRecovery <= 0.0f && FMath::IsNearlyZero(PendingYawRecovery))
	{
		return;
	}

	if (QueuedPitchKick > 0.0f || !FMath::IsNearlyZero(QueuedYawKick))
	{
		return;
	}

	if (GetWorldTimeSeconds() < RecoveryStartTime || CurrentRecoverySpeed <= 0.0f)
	{
		return;
	}

	if (!CharacterOwner)
	{
		CharacterOwner = Cast<AMyFpsCharacter>(GetOwner());
	}

	if (!CharacterOwner || !CharacterOwner->IsLocallyControlled() || !CharacterOwner->Controller)
	{
		return;
	}

	const float RecoveryAmount = CurrentRecoverySpeed * DeltaTime;
	const float PitchRecovery = FMath::Min(PendingPitchRecovery, RecoveryAmount);
	if (PitchRecovery > 0.0f)
	{
		CharacterOwner->AddControllerPitchInput(bInvertPitchInputForViewKick ? PitchRecovery : -PitchRecovery);
		PendingPitchRecovery -= PitchRecovery;
	}

	const float YawRecovery = FMath::Min(FMath::Abs(PendingYawRecovery), RecoveryAmount);
	if (YawRecovery > 0.0f)
	{
		CharacterOwner->AddControllerYawInput(-FMath::Sign(PendingYawRecovery) * YawRecovery);
		PendingYawRecovery -= FMath::Sign(PendingYawRecovery) * YawRecovery;
	}
}

void UMyFpsWeaponRecoilComponent::CheckWeaponChanged()
{
	if (!CharacterOwner)
	{
		CharacterOwner = Cast<AMyFpsCharacter>(GetOwner());
	}

	if (!InventoryComponent && CharacterOwner)
	{
		InventoryComponent = CharacterOwner->GetWeaponInventoryComponent();
	}

	const FName CurrentWeaponId = InventoryComponent
		? InventoryComponent->GetCurrentWeaponState().WeaponId
		: NAME_None;
	if (CurrentWeaponId == LastObservedWeaponId)
	{
		return;
	}

	LastObservedWeaponId = CurrentWeaponId;
	ResetRecoilState();
}

void UMyFpsWeaponRecoilComponent::RecoverAllRecoilImmediately()
{
	if (!CharacterOwner)
	{
		CharacterOwner = Cast<AMyFpsCharacter>(GetOwner());
	}

	if (!CharacterOwner || !CharacterOwner->IsLocallyControlled() || !CharacterOwner->Controller)
	{
		return;
	}

	if (PendingPitchRecovery > 0.0f)
	{
		CharacterOwner->AddControllerPitchInput(bInvertPitchInputForViewKick ? PendingPitchRecovery : -PendingPitchRecovery);
	}

	if (!FMath::IsNearlyZero(PendingYawRecovery))
	{
		CharacterOwner->AddControllerYawInput(-PendingYawRecovery);
	}
}

void UMyFpsWeaponRecoilComponent::ApplyControllerRecoil(float PitchUp, float YawOffset)
{
	if (!CharacterOwner || !CharacterOwner->Controller)
	{
		return;
	}

	CharacterOwner->AddControllerPitchInput(bInvertPitchInputForViewKick ? -PitchUp : PitchUp);
	CharacterOwner->AddControllerYawInput(YawOffset);
}

float UMyFpsWeaponRecoilComponent::GetWorldTimeSeconds() const
{
	const UWorld* World = GetWorld();
	return World ? World->GetTimeSeconds() : 0.0f;
}
