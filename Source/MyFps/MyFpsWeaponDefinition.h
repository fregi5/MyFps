// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WeaponDataRow.h"
#include "MyFpsWeaponDefinition.generated.h"

class AActor;
class AMyFpsProjectile;
class UAnimInstance;
class UAnimMontage;
class UBlendSpace;
class UNiagaraSystem;
class UPhysicalMaterial;
class USkeletalMesh;
class USoundBase;

UENUM(BlueprintType)
enum class EMyFpsWeaponType : uint8
{
	None UMETA(DisplayName = "None"),
	AssaultRifle UMETA(DisplayName = "Assault Rifle"),
	Pistol UMETA(DisplayName = "Pistol"),
	Shotgun UMETA(DisplayName = "Shotgun"),
	SniperRifle UMETA(DisplayName = "Sniper Rifle"),
	Launcher UMETA(DisplayName = "Launcher")
};

UENUM(BlueprintType)
enum class EMyFpsTraceType : uint8
{
	SingleLine UMETA(DisplayName = "Single Line"),
	MultiLine UMETA(DisplayName = "Multi Line"),
	Projectile UMETA(DisplayName = "Projectile")
};

UENUM(BlueprintType)
enum class EMyFpsHitZone : uint8
{
	Head UMETA(DisplayName = "Head"),
	Body UMETA(DisplayName = "Body"),
	Arm UMETA(DisplayName = "Arm"),
	Leg UMETA(DisplayName = "Leg")
};

UENUM(BlueprintType)
enum class EMyFpsRecoilPatternMode : uint8
{
	PerShotArray UMETA(DisplayName = "Per Shot Array")
};

USTRUCT(BlueprintType)
struct FMyFpsHitZoneMaterialRule
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit Zone")
	EMyFpsHitZone HitZone = EMyFpsHitZone::Body;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit Zone")
	TArray<TObjectPtr<UPhysicalMaterial>> PhysicalMaterials;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit Zone", meta = (ClampMin = "0.0"))
	float DamageMultiplier = 1.0f;
};

USTRUCT(BlueprintType)
struct FMyFpsRecoilPatternShot
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recoil", meta = (ClampMin = "1"))
	int32 StartShot = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recoil", meta = (ClampMin = "1"))
	int32 EndShot = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recoil")
	float PitchUp = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recoil")
	float YawOffset = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recoil")
	float RandomYawMin = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recoil")
	float RandomYawMax = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recoil", meta = (ClampMin = "0.0"))
	float SpreadAdd = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recoil", meta = (ClampMin = "-1.0", ForceUnits = "s"))
	float RecoveryDelay = -1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recoil", meta = (ClampMin = "-1.0"))
	float RecoverySpeed = -1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recoil", meta = (ClampMin = "0.0"))
	float CameraShakeScale = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recoil", meta = (ClampMin = "0.0"))
	float ViewKickScale = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recoil", meta = (ClampMin = "0.0"))
	float WeaponKickScale = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recoil", meta = (MultiLine = "true"))
	FString Remark;
};

UCLASS(BlueprintType)
class MYFPS_API UMyFpsWeaponDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	static const FPrimaryAssetType WeaponDefinitionAssetType;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	float GetFireInterval() const;
	float GetFireCooldown(bool bIncludeBoltAction) const;
	float GetTraceDistance() const;

	UFUNCTION(CallInEditor, BlueprintCallable, Category = "Recoil|Import")
	bool ImportRecoilPatternFromCsv();

	UFUNCTION(BlueprintCallable, Category = "Recoil|Import")
	bool ImportRecoilPatternFromCsvPath(const FString& CsvFilePath);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FName WeaponId = TEXT("Rifle");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FText DisplayName = FText::FromString(TEXT("Rifle"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	EMyFpsWeaponType WeaponType = EMyFpsWeaponType::AssaultRifle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire")
	EWeaponFireMode FireMode = EWeaponFireMode::Hitscan;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire")
	EMyFpsTraceType TraceType = EMyFpsTraceType::SingleLine;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire")
	TSubclassOf<AMyFpsProjectile> ProjectileClass = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	float Damage = 25.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|HitZone")
	TArray<FMyFpsHitZoneMaterialRule> HitZoneRules;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage|Range Falloff", meta = (ClampMin = "0.0", ForceUnits = "cm"))
	float CloseRangeEnd = 5000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage|Range Falloff", meta = (ClampMin = "0.0"))
	float CloseRangeDamageMultiplier = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage|Range Falloff", meta = (ClampMin = "0.0", ForceUnits = "cm"))
	float MidRangeEnd = 20000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage|Range Falloff", meta = (ClampMin = "0.0"))
	float MidRangeDamageMultiplier = 0.75f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage|Range Falloff", meta = (ClampMin = "0.0", ForceUnits = "cm"))
	float LongRangeEnd = 100000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage|Range Falloff", meta = (ClampMin = "0.0"))
	float LongRangeDamageMultiplier = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire")
	bool bAutomaticFire = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire", meta = (ClampMin = "0.01"))
	float FireInterval = 0.12f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire", meta = (ClampMin = "1.0", ForceUnits = "rpm"))
	float FireRateRPM = 500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire", meta = (ClampMin = "-1.0", ForceUnits = "s"))
	float FireCooldown = -1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Range")
	float HitscanDistance = 100000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo", meta = (ClampMin = "1"))
	int32 ClipSize = 30;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo", meta = (ClampMin = "0"))
	int32 InitialReserveAmmo = 90;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo", meta = (ClampMin = "0"))
	int32 MaxReserveAmmo = 90;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reload", meta = (ClampMin = "0.0", ForceUnits = "s"))
	float ReloadTime = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reload", meta = (ClampMin = "0.0", ForceUnits = "s"))
	float EmptyReloadTime = 2.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reload", meta = (ClampMin = "0.0", ForceUnits = "s"))
	float AmmoApplyTime = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equip", meta = (ClampMin = "0.0", ForceUnits = "s"))
	float EquipTime = 0.4f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ADS", meta = (ClampMin = "0.0", ForceUnits = "s"))
	float AdsInTime = 0.15f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ADS", meta = (ClampMin = "0.0", ForceUnits = "s"))
	float AdsOutTime = 0.12f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Range", meta = (ClampMin = "0.0", ForceUnits = "cm"))
	float EffectiveRange = 5000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Range", meta = (ClampMin = "0.0", ForceUnits = "cm"))
	float MaxRange = 100000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recoil", meta = (ClampMin = "0.0"))
	float VerticalRecoilPerShot = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recoil", meta = (ClampMin = "0.0"))
	float HorizontalRecoilPerShot = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recoil", meta = (ClampMin = "0.0", ForceUnits = "s"))
	float RecoilResetTime = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recoil", meta = (ClampMin = "0.0"))
	float MaxRecoilPitch = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recoil", meta = (ClampMin = "0.0"))
	float MaxRecoilYaw = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recoil", meta = (ClampMin = "0.0"))
	float RecoilRecoverySpeed = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recoil", meta = (ClampMin = "0.01"))
	float RecoilKickSpeed = 30.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recoil|Pattern")
	bool bUseRecoilPattern = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recoil|Pattern")
	EMyFpsRecoilPatternMode RecoilPatternMode = EMyFpsRecoilPatternMode::PerShotArray;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recoil|Pattern")
	TArray<FMyFpsRecoilPatternShot> RecoilPatternShots;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recoil|Import", meta = (FilePathFilter = "csv"))
	FFilePath RecoilPatternCsvFile;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spread", meta = (ClampMin = "0.0"))
	float HipSpreadStanding = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spread", meta = (ClampMin = "0.0"))
	float HipSpreadMoving = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spread", meta = (ClampMin = "0.0"))
	float HipSpreadRunning = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spread", meta = (ClampMin = "0.0"))
	float HipSpreadJumping = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spread", meta = (ClampMin = "0.0"))
	float AdsSpreadStanding = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spread", meta = (ClampMin = "0.0"))
	float AdsSpreadMoving = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation", meta = (ClampMin = "0.0"))
	float FireMontageStopDelay = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation", meta = (ClampMin = "0.0"))
	float ReloadMontageStopDelay = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation", meta = (ClampMin = "0.0", ForceUnits = "s"))
	float BoltActionDelay = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation", meta = (ClampMin = "0.0", ForceUnits = "s"))
	float BoltActionTime = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation", meta = (ClampMin = "0.0"))
	float BoltActionMontageStopDelay = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Visual")
	TObjectPtr<USkeletalMesh> FirstPersonWeaponMesh = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Visual")
	TObjectPtr<USkeletalMesh> ThirdPersonWeaponMesh = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Visual")
	TSubclassOf<AActor> FirstPersonVisualActorClass = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Visual")
	TSubclassOf<AActor> ThirdPersonVisualActorClass = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Attachment")
	FName FirstPersonAttachSocketName = TEXT("WeaponPoint");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Attachment")
	FName FirstPersonGripSocketName = TEXT("GripSocket");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Attachment")
	FName FirstPersonLeftHandIKSocketName = TEXT("LeftHandIK");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Attachment")
	FVector FirstPersonAttachLocationOffset = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Attachment")
	FRotator FirstPersonAttachRotationOffset = FRotator::ZeroRotator;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Attachment")
	FName ThirdPersonAttachSocketName = TEXT("Weapon_R");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Attachment")
	FName ThirdPersonGripSocketName = TEXT("GripSocket");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Attachment")
	FName ThirdPersonLeftHandIKSocketName = TEXT("LeftHandIK");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Attachment")
	FVector ThirdPersonAttachLocationOffset = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Attachment")
	FRotator ThirdPersonAttachRotationOffset = FRotator::ZeroRotator;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Projectile")
	FVector MuzzleOffset = FVector(100.0f, 0.0f, 10.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Projectile")
	FName MuzzleComponentTag = TEXT("muzzle");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SFX")
	TObjectPtr<USoundBase> FireSound = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SFX")
	TObjectPtr<USoundBase> ReloadSound = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SFX")
	TObjectPtr<USoundBase> EquipSound = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> FireAnimation = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> ThirdPersonFireAnimation = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> BoltActionAnimation = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> ThirdPersonBoltActionAnimation = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> ReloadAnimation = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> ThirdPersonReloadAnimation = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> EquipAnimation = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> ThirdPersonEquipAnimation = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Pose")
	TSubclassOf<UAnimInstance> FirstPersonAnimClass = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Pose")
	TSubclassOf<UAnimInstance> ThirdPersonAnimClass = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Pose")
	TObjectPtr<UBlendSpace> FirstPersonLocomotionBlendSpace = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Pose")
	TObjectPtr<UBlendSpace> ThirdPersonLocomotionBlendSpace = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Pose")
	TObjectPtr<UBlendSpace> FirstPersonAimOffset = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Pose")
	TObjectPtr<UBlendSpace> ThirdPersonAimOffset = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	TObjectPtr<UNiagaraSystem> TracerEffect = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	TObjectPtr<UNiagaraSystem> MuzzleFlash = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	TObjectPtr<UNiagaraSystem> ImpactEffect = nullptr;
};
