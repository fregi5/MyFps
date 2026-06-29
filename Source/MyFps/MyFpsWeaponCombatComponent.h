// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MyFpsWeaponCombatComponent.generated.h"

class AMyFpsCharacter;
class UMyFpsWeaponDefinition;
class UMyFpsWeaponInventoryComponent;
class UMyFpsWeaponRecoilComponent;
class UMyFpsWeaponViewComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MYFPS_API UMyFpsWeaponCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMyFpsWeaponCombatComponent();

	UFUNCTION(BlueprintCallable, Category = "Weapon|Combat")
	void StartFire();

	UFUNCTION(BlueprintCallable, Category = "Weapon|Combat")
	void StopFire();

	UFUNCTION(BlueprintCallable, Category = "Weapon|Combat")
	void Reload();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	bool CanFire() const;
	bool CanReload() const;
	void FireOnce();
	void FinishReload();
	bool GetViewTraceData(FVector& OutViewLocation, FRotator& OutViewRotation) const;
	bool TraceAim(const FVector& ViewLocation, const FRotator& ViewRotation, FHitResult& OutHitResult, FVector& OutAimPoint) const;
	FVector GetMuzzleLocation(const FVector& ViewLocation, const FRotator& ViewRotation, const FVector& AimPoint) const;
	void SpawnTracerEffect(const FVector& MuzzleLocation, const FVector& AimPoint) const;

	UFUNCTION(Server, Reliable)
	void ServerStartFire();

	UFUNCTION(Server, Reliable)
	void ServerStopFire();

	UFUNCTION(Server, Reliable)
	void ServerReload();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastFireCosmetics(const FVector_NetQuantize& MuzzleLocation, const FVector_NetQuantize& AimPoint);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastReloadCosmetics();

	UPROPERTY(Transient)
	TObjectPtr<AMyFpsCharacter> CharacterOwner = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMyFpsWeaponInventoryComponent> InventoryComponent = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMyFpsWeaponViewComponent> WeaponViewComponent = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMyFpsWeaponRecoilComponent> WeaponRecoilComponent = nullptr;

	FTimerHandle AutoFireTimerHandle;
	FTimerHandle ReloadTimerHandle;
};
