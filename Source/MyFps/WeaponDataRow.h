#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "WeaponDataRow.generated.h"

class AMyFpsProjectile;
class UAnimMontage;
class UNiagaraSystem;
class USoundBase;

UENUM(BlueprintType)
enum class EWeaponFireMode : uint8
{
	Hitscan,
	Projectile
};

USTRUCT(BlueprintType)
struct FWeaponDataRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	FText DisplayName = FText::FromString(TEXT("Rifle"));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	EWeaponFireMode FireMode = EWeaponFireMode::Hitscan;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<AMyFpsProjectile> ProjectileClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float Damage = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	bool bAutomaticFire = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float FireInterval = 0.12f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float HitscanDistance = 100000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	int32 ClipSize = 30;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	int32 MaxReserveAmmo = 90;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float ReloadTime = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	FVector MuzzleOffset = FVector(100.0f, 0.0f, 10.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float DroppedLifeSpan = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	USoundBase* FireSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	USoundBase* ReloadSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	USoundBase* EquipSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	UAnimMontage* FireAnimation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	UAnimMontage* ReloadAnimation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	UAnimMontage* ThirdPersonReloadAnimation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	UAnimMontage* EquipAnimation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	UNiagaraSystem* TracerEffect = nullptr;
};
