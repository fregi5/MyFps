// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyFpsWeaponPickupActor.generated.h"

class AMyFpsCharacter;
class UMyFpsWeaponDefinition;
class UPrimitiveComponent;
class USkeletalMesh;
class USkeletalMeshComponent;
class UTP_PickUpComponent;
class UWidgetComponent;

UCLASS()
class MYFPS_API AMyFpsWeaponPickupActor : public AActor
{
	GENERATED_BODY()

public:
	AMyFpsWeaponPickupActor();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Weapon|Pickup")
	bool TryPickup(AMyFpsCharacter* PickupCharacter);

	UFUNCTION(BlueprintCallable, Category = "Weapon|Pickup")
	void SetPickupEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "Weapon|Pickup")
	bool IsPickupEnabled() const { return bPickupEnabled; }

	UFUNCTION(BlueprintPure, Category = "Weapon|Pickup")
	UMyFpsWeaponDefinition* GetWeaponDefinition() const { return WeaponDefinition; }

	UFUNCTION(BlueprintCallable, Category = "Weapon|Pickup")
	void SetWeaponDefinition(UMyFpsWeaponDefinition* NewWeaponDefinition);

	UFUNCTION(BlueprintCallable, Category = "Weapon|Drop")
	void StartDroppedPhysics(const FVector& DropImpulse);

	UFUNCTION(BlueprintCallable, Category = "Weapon|Visual")
	void SetMeshHighlight(bool bEnabled);

	bool IsPickupMeshComponent(const UPrimitiveComponent* Component) const;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandlePickupRequested(AMyFpsCharacter* PickupCharacter);

	UFUNCTION()
	void OnRep_PickupEnabled();

	UFUNCTION()
	void OnRep_WeaponDefinition();

	void ApplyPickupEnabledState();
	void ApplyPickupMesh();
	void ApplyPickupDisplayData();
	void HidePickupPrompt();
	void UpdateDroppedPhysicsState();
	void FreezeDroppedPhysics();
	bool FindGroundBelow(FHitResult& OutGroundHit, float TraceDistance) const;
	void SnapToGroundIfPossible();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Pickup")
	TObjectPtr<USceneComponent> SceneRoot = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Pickup")
	TObjectPtr<USkeletalMeshComponent> PickupMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Pickup")
	TObjectPtr<USkeletalMesh> PickupSkeletalMesh = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Pickup")
	TObjectPtr<UTP_PickUpComponent> InteractionSphere = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Pickup")
	TObjectPtr<UWidgetComponent> PickupPromptWidget = nullptr;

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_WeaponDefinition, BlueprintReadOnly, Category = "Weapon|Pickup")
	TObjectPtr<UMyFpsWeaponDefinition> WeaponDefinition = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_PickupEnabled, BlueprintReadOnly, Category = "Weapon|Pickup")
	bool bPickupEnabled = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Visual")
	int32 HighlightCustomDepthStencil = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Drop")
	float DroppedPhysicsCheckInterval = 0.05f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Drop")
	float DroppedMaxPhysicsDuration = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Drop")
	float DroppedStableRequiredTime = 0.2f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Drop")
	float DroppedGroundTraceDistance = 35.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Drop")
	float DroppedFallbackGroundTraceDistance = 3000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Drop")
	float DroppedLinearSleepSpeed = 15.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Drop")
	float DroppedAngularSleepSpeed = 30.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Drop")
	float DroppedSnapToGroundOffset = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Drop")
	float DroppedPickupEnableDelay = 0.2f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Drop")
	float DroppedAngularImpulseStrength = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Drop")
	float DroppedLinearDamping = 2.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Drop")
	float DroppedAngularDamping = 8.0f;

	UPROPERTY()
	bool bDroppedPhysicsActive = false;

	float DroppedPhysicsStartTime = 0.0f;
	float DroppedStableTime = 0.0f;

	FTimerHandle DroppedPhysicsTimerHandle;
};
