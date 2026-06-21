// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MyFpsWeaponViewComponent.generated.h"

class AMyFpsCharacter;
class UAnimMontage;
class UMyFpsWeaponDefinition;
class UMyFpsWeaponInventoryComponent;
class USkeletalMeshComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MYFPS_API UMyFpsWeaponViewComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMyFpsWeaponViewComponent();

	UFUNCTION(BlueprintCallable, Category = "Weapon|View")
	void RefreshWeaponVisuals();

	UFUNCTION(BlueprintCallable, Category = "Weapon|View")
	void ClearWeaponVisuals();

	UFUNCTION(BlueprintPure, Category = "Weapon|View")
	FTransform GetMuzzleTransform(const FRotator& FallbackRotation) const;

	UFUNCTION(BlueprintCallable, Category = "Weapon|View")
	void PlayFireCosmetics();

	UFUNCTION(BlueprintCallable, Category = "Weapon|View")
	void PlayReloadCosmetics();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UFUNCTION()
	void HandleInventoryChanged();

	void EnsureVisualMeshes();
	void ApplyWeaponDefinition(UMyFpsWeaponDefinition* WeaponDefinition);
	void DestroyVisualMeshes();
	float PlayMontage(UAnimMontage* Montage);
	void StopMontage(UAnimMontage* Montage, float BlendOutTime = 0.1f) const;
	void StopFireMontage();
	void StopReloadMontage();

	UPROPERTY(Transient)
	TObjectPtr<AMyFpsCharacter> CharacterOwner = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMyFpsWeaponInventoryComponent> InventoryComponent = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<USkeletalMeshComponent> FirstPersonWeaponMesh = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<USkeletalMeshComponent> ThirdPersonWeaponMesh = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> ActiveFireMontage = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> ActiveReloadMontage = nullptr;

	FTimerHandle FireMontageStopTimerHandle;
	FTimerHandle ReloadMontageStopTimerHandle;
};
