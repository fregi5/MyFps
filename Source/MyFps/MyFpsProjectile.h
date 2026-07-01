// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "MyFpsProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UMyFpsWeaponDefinition;

UCLASS(config=Game)
class AMyFpsProjectile : public AActor
{
	GENERATED_BODY()
	/** Sphere collision component */
	UPROPERTY(VisibleDefaultsOnly, Category=Projectile)
	USphereComponent* CollisionComp;

	/** Projectile movement component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	UProjectileMovementComponent* ProjectileMovement;

public:
	AMyFpsProjectile();

	UFUNCTION(BlueprintCallable, Category = "Damage")
	void SetDamage(float NewDamage);

	UFUNCTION(BlueprintCallable, Category = "Damage")
	void SetWeaponDefinition(UMyFpsWeaponDefinition* NewWeaponDefinition, const FVector& NewDamageStartLocation);

	/** called when projectile hits something */
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	void OnOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	/** Returns CollisionComp subobject **/
	USphereComponent* GetCollisionComp() const { return CollisionComp; }
	/** Returns ProjectileMovement subobject **/
	UProjectileMovementComponent* GetProjectileMovement() const { return ProjectileMovement; }
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
    float Damage = 25.0f;

	// Number of character/pawn targets this projectile can damage before it is destroyed.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile|Penetration", meta = (ClampMin = "1"))
	int32 MaxPawnHitsBeforeDestroy = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile|Penetration", meta = (ClampMin = "0.0", ForceUnits = "s"))
	float DestroyDelayAfterWorldHit = 0.0f;

	UPROPERTY(Transient)
	TObjectPtr<UMyFpsWeaponDefinition> WeaponDefinition = nullptr;

	UPROPERTY(Transient)
	FVector DamageStartLocation = FVector::ZeroVector;

private:
	void HandleImpact(AActor* OtherActor, UPrimitiveComponent* OtherComp, const FHitResult& Hit);
	void ApplyProjectileDamage(AActor* OtherActor, const FHitResult& Hit);
	void ContinueAfterPawnHit(AActor* HitActor, UPrimitiveComponent* HitComp);
	void StopAndScheduleDestroy();
	bool IsValidImpactActor(const AActor* OtherActor) const;
	bool IsPawnTarget(const AActor* OtherActor) const;

	UPROPERTY(Transient)
	TArray<TObjectPtr<AActor>> DamagedPawnTargets;

	bool bDestroyScheduled = false;
};
