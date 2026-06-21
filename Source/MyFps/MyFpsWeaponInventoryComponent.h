// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MyFpsWeaponInventoryComponent.generated.h"

class UMyFpsWeaponDefinition;

USTRUCT(BlueprintType)
struct FMyFpsWeaponRuntimeState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	FName WeaponId = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	FText DisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	int32 CurrentAmmoInClip = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	int32 CurrentReserveAmmo = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	bool bReloading = false;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	bool bEquipping = false;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	bool bFiring = false;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	int32 FireSequence = 0;

	bool HasWeapon() const
	{
		return !WeaponId.IsNone();
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMyFpsWeaponInventoryChanged);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MYFPS_API UMyFpsWeaponInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMyFpsWeaponInventoryComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FOnMyFpsWeaponInventoryChanged OnInventoryChanged;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool EquipWeaponDefinition(UMyFpsWeaponDefinition* WeaponDefinition);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void ClearCurrentWeapon();

	bool ConsumeAmmo(int32 AmmoAmount);
	bool CanReload() const;
	bool FinishReload();
	void SetReloadingState(bool bNewReloading);
	void SetFiringState(bool bNewFiring);

	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool HasWeapon() const;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	const FMyFpsWeaponRuntimeState& GetCurrentWeaponState() const { return CurrentWeaponState; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	UMyFpsWeaponDefinition* GetCurrentWeaponDefinition() const { return CurrentWeaponDefinition; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	int32 GetCurrentAmmoInClip() const { return CurrentWeaponState.CurrentAmmoInClip; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	int32 GetCurrentReserveAmmo() const { return CurrentWeaponState.CurrentReserveAmmo; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool HasAmmoInClip() const { return CurrentWeaponState.CurrentAmmoInClip > 0; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool IsReloading() const { return CurrentWeaponState.bReloading; }

private:
	UFUNCTION()
	void OnRep_CurrentWeaponDefinition();

	UFUNCTION()
	void OnRep_CurrentWeaponState();

	void BroadcastInventoryChanged();

	UPROPERTY(ReplicatedUsing = OnRep_CurrentWeaponDefinition, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMyFpsWeaponDefinition> CurrentWeaponDefinition = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentWeaponState, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	FMyFpsWeaponRuntimeState CurrentWeaponState;
};
