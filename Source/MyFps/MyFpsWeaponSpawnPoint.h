// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyFpsWeaponSpawnPoint.generated.h"

class AMyFpsWeaponPickupActor;
class UArrowComponent;
class UBillboardComponent;
class UMyFpsWeaponDefinition;
class USphereComponent;

UCLASS()
class MYFPS_API AMyFpsWeaponSpawnPoint : public AActor
{
	GENERATED_BODY()

public:
	AMyFpsWeaponSpawnPoint();

	virtual void OnConstruction(const FTransform& Transform) override;

	UFUNCTION(BlueprintCallable, Category = "Spawn Point|Weapon")
	void ConfigureWeaponSpawnPoint(
		UMyFpsWeaponDefinition* NewWeaponDefinition,
		TSubclassOf<AMyFpsWeaponPickupActor> NewWeaponPickupActorClass);

	UFUNCTION(BlueprintCallable, Category = "Spawn Point|Weapon")
	void ConfigureWeaponSpawnSettings(
		bool bNewEnabled,
		bool bNewSpawnOnBeginPlay,
		bool bNewRespawnAfterPickup,
		float NewRespawnDelay);

	UFUNCTION(BlueprintCallable, Category = "Spawn Point|Weapon")
	AMyFpsWeaponPickupActor* SpawnWeapon();
	
	
	UFUNCTION(BlueprintPure, Category = "Spawn Point|Weapon")
	UMyFpsWeaponDefinition* GetWeaponDefinition() const { return WeaponDefinition; }

	UFUNCTION(BlueprintCallable, Category = "Spawn Point|Weapon")
	void ClearSpawnedWeapon();

	UFUNCTION(BlueprintPure, Category = "Spawn Point|Weapon")
	AMyFpsWeaponPickupActor* GetSpawnedWeapon() const { return SpawnedWeapon; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawn Point")
	TObjectPtr<USceneComponent> SceneRoot = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawn Point")
	TObjectPtr<UBillboardComponent> BillboardComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawn Point")
	TObjectPtr<USphereComponent> PreviewRadiusComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawn Point")
	TObjectPtr<UArrowComponent> ArrowComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn Point|Weapon")
	TSubclassOf<AMyFpsWeaponPickupActor> WeaponPickupActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn Point|Weapon")
	TObjectPtr<UMyFpsWeaponDefinition> WeaponDefinition = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn Point|Weapon")
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn Point|Weapon")
	bool bSpawnOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn Point|Weapon")
	bool bRespawnAfterPickup = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn Point|Weapon", meta = (ClampMin = "0.0", ForceUnits = "s"))
	float RespawnDelay = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn Point|Preview", meta = (ClampMin = "0.0", ForceUnits = "cm"))
	float PreviewRadius = 120.0f;

private:
	UFUNCTION()
	void HandleSpawnedWeaponDestroyed(AActor* DestroyedActor);

	void ScheduleRespawn();
	void SpawnWeaponFromTimer();
	void ApplyPreviewSettings();

	UPROPERTY(Transient)
	TObjectPtr<AMyFpsWeaponPickupActor> SpawnedWeapon = nullptr;

	FTimerHandle RespawnTimerHandle;
};
