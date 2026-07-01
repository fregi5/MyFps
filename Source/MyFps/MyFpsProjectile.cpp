// Copyright Epic Games, Inc. All Rights Reserved.

#include "MyFpsProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "EnemyCharacter.h"
#include "GameFramework/Pawn.h"
#include "MyFpsWeaponDamageLibrary.h"
#include "MyFpsWeaponDefinition.h"
#include "MyFpsCharacter.h"
#include "MyFpsPlayerController.h"

AMyFpsProjectile::AMyFpsProjectile() 
{
	bReplicates = true;
	SetReplicateMovement(true);

	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	CollisionComp->bReturnMaterialOnMove = true;
	CollisionComp->SetGenerateOverlapEvents(true);
	CollisionComp->OnComponentHit.AddDynamic(this, &AMyFpsProjectile::OnHit);		// set up a notification for when this component hits something blocking
	CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &AMyFpsProjectile::OnOverlap);

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
	HandleImpact(OtherActor, OtherComp, Hit);
}

void AMyFpsProjectile::OnOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	HandleImpact(OtherActor, OtherComp, SweepResult);
}

void AMyFpsProjectile::HandleImpact(AActor* OtherActor, UPrimitiveComponent* OtherComp, const FHitResult& Hit)
{
	if (bDestroyScheduled || !IsValidImpactActor(OtherActor))
	{
		return;
	}

	if (IsPawnTarget(OtherActor))
	{
		if (!DamagedPawnTargets.Contains(OtherActor))
		{
			ApplyProjectileDamage(OtherActor, Hit);
			DamagedPawnTargets.Add(OtherActor);
		}

		if (DamagedPawnTargets.Num() >= FMath::Max(1, MaxPawnHitsBeforeDestroy))
		{
			Destroy();
			return;
		}

		ContinueAfterPawnHit(OtherActor, OtherComp);
		return;
	}

	StopAndScheduleDestroy();
}

void AMyFpsProjectile::ApplyProjectileDamage(AActor* OtherActor, const FHitResult& Hit)
{
	if (!HasAuthority() || !OtherActor)
	{
		return;
	}

	const FVector EffectiveDamageStartLocation = DamageStartLocation.IsNearlyZero()
		? GetActorLocation()
		: DamageStartLocation;
	const float FinalDamage = WeaponDefinition
		? UMyFpsWeaponDamageLibrary::CalculateDamageForHit(WeaponDefinition, Hit, EffectiveDamageStartLocation)
		: Damage;

	const float ActualDamage = UGameplayStatics::ApplyDamage(
		OtherActor,
		FinalDamage,
		GetInstigatorController(),
		this,
		UDamageType::StaticClass()
	);

	if (ActualDamage <= 0.0f)
	{
		return;
	}

	bool bKill = false;
	if (const AMyFpsCharacter* HitCharacter = Cast<AMyFpsCharacter>(OtherActor))
	{
		bKill = HitCharacter->IsDead();
	}
	else if (const AEnemyCharacter* HitEnemy = Cast<AEnemyCharacter>(OtherActor))
	{
		bKill = HitEnemy->IsDead();
	}

	if (AMyFpsPlayerController* InstigatorController = Cast<AMyFpsPlayerController>(GetInstigatorController()))
	{
		InstigatorController->ClientShowHitMarker(bKill);
	}
}

void AMyFpsProjectile::ContinueAfterPawnHit(AActor* HitActor, UPrimitiveComponent* HitComp)
{
	if (!CollisionComp || !ProjectileMovement)
	{
		return;
	}

	CollisionComp->IgnoreActorWhenMoving(HitActor, true);
	if (HitComp)
	{
		HitComp->IgnoreActorWhenMoving(this, true);
	}

	const FVector Direction = GetActorForwardVector().GetSafeNormal();
	const float Speed = FMath::Max(ProjectileMovement->InitialSpeed, ProjectileMovement->Velocity.Size());
	const float Radius = CollisionComp->GetScaledSphereRadius();
	SetActorLocation(GetActorLocation() + Direction * FMath::Max(1.0f, Radius * 2.0f), false);

	ProjectileMovement->StopMovementImmediately();
	ProjectileMovement->SetUpdatedComponent(CollisionComp);
	ProjectileMovement->Velocity = Direction * Speed;
	ProjectileMovement->Activate(true);
	ProjectileMovement->UpdateComponentVelocity();
}

void AMyFpsProjectile::StopAndScheduleDestroy()
{
	bDestroyScheduled = true;

	if (ProjectileMovement)
	{
		ProjectileMovement->StopMovementImmediately();
		ProjectileMovement->bShouldBounce = false;
	}

	if (CollisionComp)
	{
		CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (DestroyDelayAfterWorldHit > 0.0f)
	{
		SetLifeSpan(DestroyDelayAfterWorldHit);
	}
	else
	{
		Destroy();
	}
}

bool AMyFpsProjectile::IsValidImpactActor(const AActor* OtherActor) const
{
	return OtherActor != nullptr
		&& OtherActor != this
		&& OtherActor != GetOwner();
}

bool AMyFpsProjectile::IsPawnTarget(const AActor* OtherActor) const
{
	return OtherActor && OtherActor->IsA<APawn>();
}
