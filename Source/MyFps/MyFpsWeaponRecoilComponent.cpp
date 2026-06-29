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

	if (WeaponDefinition->bUseRecoilPattern && WeaponDefinition->RecoilPatternMode == EMyFpsRecoilPatternMode::PerShotArray)
	{
		const int32 OneBasedShotIndex = CurrentShotIndex + 1;
		const FMyFpsRecoilPatternShot* PatternShot = WeaponDefinition->RecoilPatternShots.FindByPredicate(
			[OneBasedShotIndex](const FMyFpsRecoilPatternShot& Candidate)
			{
				return Candidate.ShotIndex == OneBasedShotIndex;
			});

		if (!PatternShot && WeaponDefinition->RecoilPatternShots.IsValidIndex(CurrentShotIndex))
		{
			PatternShot = &WeaponDefinition->RecoilPatternShots[CurrentShotIndex];
		}

		if (PatternShot)
		{
			PitchUp = PatternShot->PitchUp;
			YawOffset = PatternShot->YawOffset;
			RandomYawMin = PatternShot->RandomYawMin;
			RandomYawMax = PatternShot->RandomYawMax;
			RecoveryDelay = PatternShot->RecoveryDelay;
			RecoverySpeed = PatternShot->RecoverySpeed > 0.0f
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
		const float AllowedPitch = FMath::Max(0.0f, MaxPitch - PendingPitchRecovery);
		PitchUp = FMath::Min(PitchUp, AllowedPitch);
	}

	const float MaxYaw = FMath::Max(0.0f, WeaponDefinition->MaxRecoilYaw);
	if (MaxYaw > 0.0f)
	{
		const float AllowedYaw = FMath::Max(0.0f, MaxYaw - FMath::Abs(PendingYawRecovery));
		YawOffset = FMath::Clamp(YawOffset, -AllowedYaw, AllowedYaw);
	}

	ApplyControllerRecoil(PitchUp, YawOffset);

	PendingPitchRecovery += PitchUp;
	PendingYawRecovery += YawOffset;
	CurrentRecoverySpeed = FMath::Max(0.0f, RecoverySpeed * AmmoModifier.RecoverySpeedScale);
	RecoveryStartTime = Now + FMath::Max(0.0f, RecoveryDelay * AmmoModifier.RecoveryDelayScale);
	LastShotTime = Now;
	++CurrentShotIndex;
}

void UMyFpsWeaponRecoilComponent::ResetRecoilState()
{
	RecoverAllRecoilImmediately();
	ResetShotIndex();
	RecoveryStartTime = 0.0f;
	CurrentRecoverySpeed = 0.0f;
	PendingPitchRecovery = 0.0f;
	PendingYawRecovery = 0.0f;
}

void UMyFpsWeaponRecoilComponent::ResetShotIndex()
{
	CurrentShotIndex = 0;
	LastShotTime = -1.0f;
}

void UMyFpsWeaponRecoilComponent::RecoverRecoil(float DeltaTime)
{
	if (PendingPitchRecovery <= 0.0f && FMath::IsNearlyZero(PendingYawRecovery))
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
