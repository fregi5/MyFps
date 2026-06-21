// Copyright Epic Games, Inc. All Rights Reserved.

#include "MyFpsWeaponInventoryComponent.h"

#include "MyFpsWeaponDefinition.h"
#include "Net/UnrealNetwork.h"

UMyFpsWeaponInventoryComponent::UMyFpsWeaponInventoryComponent()
{
	SetIsReplicatedByDefault(true);
	PrimaryComponentTick.bCanEverTick = false;
}

void UMyFpsWeaponInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UMyFpsWeaponInventoryComponent, CurrentWeaponDefinition);
	DOREPLIFETIME(UMyFpsWeaponInventoryComponent, CurrentWeaponState);
}

bool UMyFpsWeaponInventoryComponent::EquipWeaponDefinition(UMyFpsWeaponDefinition* WeaponDefinition)
{
	if (!WeaponDefinition)
	{
		return false;
	}

	CurrentWeaponDefinition = WeaponDefinition;
	CurrentWeaponState.WeaponId = WeaponDefinition->WeaponId;
	CurrentWeaponState.DisplayName = WeaponDefinition->DisplayName;
	CurrentWeaponState.CurrentAmmoInClip = FMath::Max(0, WeaponDefinition->ClipSize);
	CurrentWeaponState.CurrentReserveAmmo = FMath::Max(0, WeaponDefinition->MaxReserveAmmo);
	CurrentWeaponState.bReloading = false;
	CurrentWeaponState.bEquipping = false;
	CurrentWeaponState.bFiring = false;
	CurrentWeaponState.FireSequence = 0;

	BroadcastInventoryChanged();
	return true;
}

void UMyFpsWeaponInventoryComponent::ClearCurrentWeapon()
{
	CurrentWeaponDefinition = nullptr;
	CurrentWeaponState = FMyFpsWeaponRuntimeState();
	BroadcastInventoryChanged();
}

bool UMyFpsWeaponInventoryComponent::ConsumeAmmo(int32 AmmoAmount)
{
	if (AmmoAmount <= 0 || CurrentWeaponState.bReloading || CurrentWeaponState.CurrentAmmoInClip < AmmoAmount)
	{
		return false;
	}

	CurrentWeaponState.CurrentAmmoInClip -= AmmoAmount;
	BroadcastInventoryChanged();
	return true;
}

bool UMyFpsWeaponInventoryComponent::CanReload() const
{
	const UMyFpsWeaponDefinition* WeaponDefinition = CurrentWeaponDefinition;
	return WeaponDefinition != nullptr
		&& !CurrentWeaponState.bReloading
		&& CurrentWeaponState.CurrentAmmoInClip < WeaponDefinition->ClipSize
		&& CurrentWeaponState.CurrentReserveAmmo > 0;
}

bool UMyFpsWeaponInventoryComponent::FinishReload()
{
	UMyFpsWeaponDefinition* WeaponDefinition = CurrentWeaponDefinition;
	if (!WeaponDefinition)
	{
		CurrentWeaponState.bReloading = false;
		BroadcastInventoryChanged();
		return false;
	}

	const int32 AmmoNeeded = FMath::Max(0, WeaponDefinition->ClipSize - CurrentWeaponState.CurrentAmmoInClip);
	const int32 AmmoToLoad = FMath::Min(AmmoNeeded, CurrentWeaponState.CurrentReserveAmmo);
	CurrentWeaponState.CurrentAmmoInClip += AmmoToLoad;
	CurrentWeaponState.CurrentReserveAmmo -= AmmoToLoad;
	CurrentWeaponState.bReloading = false;
	BroadcastInventoryChanged();
	return AmmoToLoad > 0;
}

void UMyFpsWeaponInventoryComponent::SetReloadingState(bool bNewReloading)
{
	if (CurrentWeaponState.bReloading == bNewReloading)
	{
		return;
	}

	CurrentWeaponState.bReloading = bNewReloading;
	BroadcastInventoryChanged();
}

void UMyFpsWeaponInventoryComponent::SetFiringState(bool bNewFiring)
{
	if (CurrentWeaponState.bFiring == bNewFiring)
	{
		return;
	}

	CurrentWeaponState.bFiring = bNewFiring;
	BroadcastInventoryChanged();
}

bool UMyFpsWeaponInventoryComponent::HasWeapon() const
{
	return CurrentWeaponState.HasWeapon();
}

void UMyFpsWeaponInventoryComponent::OnRep_CurrentWeaponDefinition()
{
	BroadcastInventoryChanged();
}

void UMyFpsWeaponInventoryComponent::OnRep_CurrentWeaponState()
{
	BroadcastInventoryChanged();
}

void UMyFpsWeaponInventoryComponent::BroadcastInventoryChanged()
{
	OnInventoryChanged.Broadcast();
}
