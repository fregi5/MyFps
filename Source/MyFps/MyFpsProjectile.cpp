// Copyright Epic Games, Inc. All Rights Reserved.

#include "MyFpsProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "MyFpsWeaponDamageLibrary.h"
#include "MyFpsWeaponDefinition.h"

AMyFpsProjectile::AMyFpsProjectile() 
{
	bReplicates = true;
	SetReplicateMovement(true);

	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	CollisionComp->bReturnMaterialOnMove = true;
	CollisionComp->OnComponentHit.AddDynamic(this, &AMyFpsProjectile::OnHit);		// set up a notification for when this component hits something blocking

	// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	// Set as root component
	RootComponent = CollisionComp;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;

	// Die after 3 seconds by default
	InitialLifeSpan = 3.0f;
}

void AMyFpsProjectile::SetDamage(float NewDamage)
{
	Damage = FMath::Max(0.0f, NewDamage);
}

void AMyFpsProjectile::SetWeaponDefinition(UMyFpsWeaponDefinition* NewWeaponDefinition, const FVector& NewDamageStartLocation)
{
	WeaponDefinition = NewWeaponDefinition;
	DamageStartLocation = NewDamageStartLocation;
	if (WeaponDefinition)
	{
		Damage = FMath::Max(0.0f, WeaponDefinition->Damage);
	}
}

void AMyFpsProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if ((OtherActor != nullptr) && (OtherActor != this) && (OtherActor != GetOwner()))
    {
		const FVector EffectiveDamageStartLocation = DamageStartLocation.IsNearlyZero()
			? GetActorLocation()
			: DamageStartLocation;
		const float FinalDamage = WeaponDefinition
			? UMyFpsWeaponDamageLibrary::CalculateDamageForHit(WeaponDefinition, Hit, EffectiveDamageStartLocation)
			: Damage;

        UGameplayStatics::ApplyDamage(
            OtherActor,
            FinalDamage,
            GetInstigatorController(),
            this,
            UDamageType::StaticClass()
        );
    }
	// Only add impulse and destroy projectile if we hit a physics
	if ((OtherActor != nullptr) && (OtherActor != this) && (OtherActor != GetOwner()) && (OtherComp != nullptr) && OtherComp->IsSimulatingPhysics())
	{
		OtherComp->AddImpulseAtLocation(GetVelocity() * 100.0f, GetActorLocation());
	}

	if ((OtherActor != nullptr) && (OtherActor != this) && (OtherActor != GetOwner()))
	{
		Destroy();
	}
}
