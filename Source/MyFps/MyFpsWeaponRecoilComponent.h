// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MyFpsWeaponRecoilComponent.generated.h"

class AMyFpsCharacter;
class UMyFpsWeaponDefinition;
class UMyFpsWeaponInventoryComponent;

USTRUCT(BlueprintType)
struct FMyFpsAmmoRecoilModifier
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil", meta = (ClampMin = "0.0"))
	float PitchScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil", meta = (ClampMin = "0.0"))
	float YawScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil", meta = (ClampMin = "0.0"))
	float RecoveryDelayScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil", meta = (ClampMin = "0.0"))
	float RecoverySpeedScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil", meta = (ClampMin = "0.0"))
	float ViewKickScale = 1.0f;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MYFPS_API UMyFpsWeaponRecoilComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMyFpsWeaponRecoilComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Weapon|Recoil")
	void ApplyWeaponRecoil(const UMyFpsWeaponDefinition* WeaponDefinition);

	UFUNCTION(BlueprintCallable, Category = "Weapon|Recoil")
	void ApplyWeaponRecoilWithAmmoModifier(const UMyFpsWeaponDefinition* WeaponDefinition, const FMyFpsAmmoRecoilModifier& AmmoModifier);

	UFUNCTION(BlueprintCallable, Category = "Weapon|Recoil")
	void ResetRecoilState();

	UFUNCTION(BlueprintCallable, Category = "Weapon|Recoil")
	void ResetShotIndex();

	UFUNCTION(BlueprintPure, Category = "Weapon|Recoil")
	int32 GetCurrentShotIndex() const { return CurrentShotIndex; }

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Recoil")
	bool bInvertPitchInputForViewKick = true;

private:
	void RecoverRecoil(float DeltaTime);
	void CheckWeaponChanged();
	void RecoverAllRecoilImmediately();
	void ApplyControllerRecoil(float PitchUp, float YawOffset);
	float GetWorldTimeSeconds() const;

	UPROPERTY(Transient)
	TObjectPtr<AMyFpsCharacter> CharacterOwner = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMyFpsWeaponInventoryComponent> InventoryComponent = nullptr;

	FName LastObservedWeaponId = NAME_None;
	int32 CurrentShotIndex = 0;
	float LastShotTime = -1.0f;
	float RecoveryStartTime = 0.0f;
	float CurrentRecoverySpeed = 0.0f;
	float PendingPitchRecovery = 0.0f;
	float PendingYawRecovery = 0.0f;
};
