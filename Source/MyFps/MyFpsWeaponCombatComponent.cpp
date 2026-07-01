// Copyright Epic Games, Inc. All Rights Reserved.

#include "MyFpsWeaponCombatComponent.h"

#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/SphereComponent.h"
#include "EnemyCharacter.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "MyFpsCharacter.h"
#include "MyFpsProjectile.h"
#include "MyFpsPlayerController.h"
#include "MyFpsWeaponDamageLibrary.h"
#include "MyFpsWeaponDefinition.h"
#include "MyFpsWeaponInventoryComponent.h"
#include "MyFpsWeaponRecoilComponent.h"
#include "MyFpsWeaponViewComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "TimerManager.h"

UMyFpsWeaponCombatComponent::UMyFpsWeaponCombatComponent()
{
	SetIsReplicatedByDefault(true);
	PrimaryComponentTick.bCanEverTick = false;
}

void UMyFpsWeaponCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	CharacterOwner = Cast<AMyFpsCharacter>(GetOwner());
	if (CharacterOwner)
	{
		InventoryComponent = CharacterOwner->GetWeaponInventoryComponent();
		WeaponViewComponent = CharacterOwner->FindComponentByClass<UMyFpsWeaponViewComponent>();
		WeaponRecoilComponent = CharacterOwner->FindComponentByClass<UMyFpsWeaponRecoilComponent>();
	}
}

void UMyFpsWeaponCombatComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoFireTimerHandle);
		World->GetTimerManager().ClearTimer(ReloadTimerHandle);
		World->GetTimerManager().ClearTimer(LocalRecoilTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void UMyFpsWeaponCombatComponent::StartFire()
{
	if (!CharacterOwner)
	{
		CharacterOwner = Cast<AMyFpsCharacter>(GetOwner());
	}

	if (!CharacterOwner)
	{
		return;
	}

	if (!CharacterOwner->HasAuthority())
	{
		StartLocalRecoilPrediction();
		ServerStartFire();
		return;
	}

	if (InventoryComponent
		&& InventoryComponent->HasWeapon()
		&& !InventoryComponent->HasAmmoInClip())
	{
		Reload();
		return;
	}

	if (!CanFire())
	{
		return;
	}

	if (InventoryComponent)
	{
		InventoryComponent->SetFiringState(true);
	}

	FireOnce();

	const UMyFpsWeaponDefinition* WeaponDefinition = InventoryComponent
		? InventoryComponent->GetCurrentWeaponDefinition()
		: nullptr;
	if (WeaponDefinition && WeaponDefinition->bAutomaticFire && GetWorld())
	{
		const float FireInterval = FMath::Max(0.01f, WeaponDefinition->GetFireInterval());
		GetWorld()->GetTimerManager().SetTimer(
			AutoFireTimerHandle,
			this,
			&UMyFpsWeaponCombatComponent::FireOnce,
			FireInterval,
			true,
			FireInterval
		);
	}
}

void UMyFpsWeaponCombatComponent::StopFire()
{
	if (!CharacterOwner)
	{
		CharacterOwner = Cast<AMyFpsCharacter>(GetOwner());
	}

	if (!CharacterOwner)
	{
		return;
	}

	if (!CharacterOwner->HasAuthority())
	{
		StopLocalRecoilPrediction();
		ServerStopFire();
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoFireTimerHandle);
	}

	if (InventoryComponent)
	{
		InventoryComponent->SetFiringState(false);
	}
}

void UMyFpsWeaponCombatComponent::ServerStartFire_Implementation()
{
	StartFire();
}

void UMyFpsWeaponCombatComponent::ServerStopFire_Implementation()
{
	StopFire();
}

void UMyFpsWeaponCombatComponent::Reload()
{
	if (!CharacterOwner)
	{
		CharacterOwner = Cast<AMyFpsCharacter>(GetOwner());
	}

	if (!CharacterOwner)
	{
		return;
	}

	if (!CharacterOwner->HasAuthority())
	{
		StopLocalRecoilPrediction();
		if (WeaponRecoilComponent)
		{
			WeaponRecoilComponent->ResetRecoilState();
		}
		ServerReload();
		return;
	}

	if (!CanReload())
	{
		return;
	}

	StopFire();
	if (WeaponRecoilComponent)
	{
		WeaponRecoilComponent->ResetRecoilState();
	}
	InventoryComponent->SetReloadingState(true);
	MulticastReloadCosmetics();

	UMyFpsWeaponDefinition* WeaponDefinition = InventoryComponent->GetCurrentWeaponDefinition();
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ReloadTimerHandle);
		World->GetTimerManager().SetTimer(
			ReloadTimerHandle,
			this,
			&UMyFpsWeaponCombatComponent::FinishReload,
			WeaponDefinition ? FMath::Max(0.01f, WeaponDefinition->ReloadTime) : 0.01f,
			false
		);
	}
}

void UMyFpsWeaponCombatComponent::ServerReload_Implementation()
{
	Reload();
}

bool UMyFpsWeaponCombatComponent::CanFire() const
{
	const AMyFpsCharacter* Character = CharacterOwner ? CharacterOwner.Get() : Cast<AMyFpsCharacter>(GetOwner());
	const UMyFpsWeaponInventoryComponent* Inventory = InventoryComponent
		? InventoryComponent.Get()
		: (Character ? Character->GetWeaponInventoryComponent() : nullptr);

	if (!Character || !Inventory || !Inventory->HasWeapon())
	{
		return false;
	}

	const UMyFpsWeaponDefinition* WeaponDefinition = Inventory->GetCurrentWeaponDefinition();
	const bool bUsesExplicitFireCooldown = WeaponDefinition
		&& (WeaponDefinition->FireCooldown >= 0.0f
			|| WeaponDefinition->BoltActionTime > 0.0f
			|| WeaponDefinition->BoltActionAnimation != nullptr
			|| WeaponDefinition->ThirdPersonBoltActionAnimation != nullptr);
	const bool bFireCooldownReady = WeaponDefinition
		&& (!bUsesExplicitFireCooldown
			|| !Character->GetWorld()
			|| Character->GetWorld()->GetTimeSeconds() >= NextAllowedFireTime);

	return Character->HasAuthority()
		&& Character->IsMatchInProgress()
		&& !Character->IsDead()
		&& WeaponDefinition != nullptr
		&& Inventory->HasAmmoInClip()
		&& !Inventory->IsReloading()
		&& bFireCooldownReady;
}

bool UMyFpsWeaponCombatComponent::CanReload() const
{
	const AMyFpsCharacter* Character = CharacterOwner ? CharacterOwner.Get() : Cast<AMyFpsCharacter>(GetOwner());
	const UMyFpsWeaponInventoryComponent* Inventory = InventoryComponent
		? InventoryComponent.Get()
		: (Character ? Character->GetWeaponInventoryComponent() : nullptr);

	return Character != nullptr
		&& Inventory != nullptr
		&& Character->HasAuthority()
		&& Character->IsMatchInProgress()
		&& !Character->IsDead()
		&& Inventory->HasWeapon()
		&& Inventory->CanReload();
}

bool UMyFpsWeaponCombatComponent::CanPredictLocalRecoil() const
{
	const AMyFpsCharacter* Character = CharacterOwner ? CharacterOwner.Get() : Cast<AMyFpsCharacter>(GetOwner());
	const UMyFpsWeaponInventoryComponent* Inventory = InventoryComponent
		? InventoryComponent.Get()
		: (Character ? Character->GetWeaponInventoryComponent() : nullptr);

	const UMyFpsWeaponDefinition* WeaponDefinition = Inventory ? Inventory->GetCurrentWeaponDefinition() : nullptr;
	const bool bUsesExplicitFireCooldown = WeaponDefinition
		&& (WeaponDefinition->FireCooldown >= 0.0f
			|| WeaponDefinition->BoltActionTime > 0.0f
			|| WeaponDefinition->BoltActionAnimation != nullptr
			|| WeaponDefinition->ThirdPersonBoltActionAnimation != nullptr);
	const bool bFireCooldownReady = WeaponDefinition
		&& (!bUsesExplicitFireCooldown
			|| !Character->GetWorld()
			|| Character->GetWorld()->GetTimeSeconds() >= NextLocalPredictedFireTime);

	return Character != nullptr
		&& Inventory != nullptr
		&& Character->IsLocallyControlled()
		&& Character->IsMatchInProgress()
		&& !Character->IsDead()
		&& Inventory->HasWeapon()
		&& WeaponDefinition != nullptr
		&& Inventory->HasAmmoInClip()
		&& !Inventory->IsReloading()
		&& bFireCooldownReady;
}

void UMyFpsWeaponCombatComponent::StartLocalRecoilPrediction()
{
	if (!CanPredictLocalRecoil())
	{
		return;
	}

	ApplyPredictedLocalRecoil();

	const UMyFpsWeaponDefinition* WeaponDefinition = InventoryComponent
		? InventoryComponent->GetCurrentWeaponDefinition()
		: nullptr;
	if (WeaponDefinition && WeaponDefinition->bAutomaticFire && GetWorld())
	{
		const float FireInterval = FMath::Max(0.01f, WeaponDefinition->GetFireInterval());
		GetWorld()->GetTimerManager().SetTimer(
			LocalRecoilTimerHandle,
			this,
			&UMyFpsWeaponCombatComponent::ApplyPredictedLocalRecoil,
			FireInterval,
			true,
			FireInterval
		);
	}
}

void UMyFpsWeaponCombatComponent::StopLocalRecoilPrediction()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(LocalRecoilTimerHandle);
	}
}

void UMyFpsWeaponCombatComponent::ApplyPredictedLocalRecoil()
{
	if (!CanPredictLocalRecoil())
	{
		StopLocalRecoilPrediction();
		return;
	}

	if (!CharacterOwner)
	{
		CharacterOwner = Cast<AMyFpsCharacter>(GetOwner());
	}

	if (CharacterOwner && !WeaponRecoilComponent)
	{
		WeaponRecoilComponent = CharacterOwner->FindComponentByClass<UMyFpsWeaponRecoilComponent>();
	}

	if (WeaponRecoilComponent && InventoryComponent)
	{
		UMyFpsWeaponDefinition* WeaponDefinition = InventoryComponent->GetCurrentWeaponDefinition();
		WeaponRecoilComponent->ApplyWeaponRecoil(WeaponDefinition);

		if (WeaponDefinition && GetWorld())
		{
			const bool bWillHaveAmmoAfterShot = InventoryComponent->GetCurrentAmmoInClip() > 1;
			NextLocalPredictedFireTime = GetWorld()->GetTimeSeconds() + WeaponDefinition->GetFireCooldown(ShouldPlayBoltAction(WeaponDefinition, bWillHaveAmmoAfterShot));
		}
	}
}

void UMyFpsWeaponCombatComponent::FinishReload()
{
	if (!CharacterOwner)
	{
		CharacterOwner = Cast<AMyFpsCharacter>(GetOwner());
	}

	if (!CharacterOwner || !CharacterOwner->HasAuthority() || !InventoryComponent)
	{
		return;
	}

	InventoryComponent->FinishReload();
	if (WeaponRecoilComponent)
	{
		WeaponRecoilComponent->ResetRecoilState();
	}
}

void UMyFpsWeaponCombatComponent::FireOnce()
{
	if (!CanFire())
	{
		StopFire();
		return;
	}

	UMyFpsWeaponDefinition* WeaponDefinition = InventoryComponent->GetCurrentWeaponDefinition();
	if (!WeaponDefinition || !InventoryComponent->ConsumeAmmo(1))
	{
		StopFire();
		return;
	}

	const bool bHasAmmoAfterShot = InventoryComponent->HasAmmoInClip();
	const bool bShouldPlayBoltAction = ShouldPlayBoltAction(WeaponDefinition, bHasAmmoAfterShot);
	if (GetWorld())
	{
		NextAllowedFireTime = GetWorld()->GetTimeSeconds() + WeaponDefinition->GetFireCooldown(bShouldPlayBoltAction);
	}

	if (CharacterOwner && CharacterOwner->IsLocallyControlled() && WeaponRecoilComponent)
	{
		WeaponRecoilComponent->ApplyWeaponRecoil(WeaponDefinition);
	}
	else if (CharacterOwner && CharacterOwner->IsLocallyControlled())
	{
		WeaponRecoilComponent = CharacterOwner->FindComponentByClass<UMyFpsWeaponRecoilComponent>();
		if (WeaponRecoilComponent)
		{
			WeaponRecoilComponent->ApplyWeaponRecoil(WeaponDefinition);
		}
	}

	FVector ViewLocation = FVector::ZeroVector;
	FRotator ViewRotation = FRotator::ZeroRotator;
	if (!GetViewTraceData(ViewLocation, ViewRotation))
	{
		StopFire();
		return;
	}

	FHitResult HitResult;
	FVector AimPoint = FVector::ZeroVector;
	const bool bHit = TraceAim(ViewLocation, ViewRotation, HitResult, AimPoint);
	const FVector MuzzleLocation = GetMuzzleLocation(ViewLocation, ViewRotation, AimPoint);

	if (WeaponDefinition->TraceType == EMyFpsTraceType::Projectile)
	{
		SpawnProjectile(WeaponDefinition, MuzzleLocation, AimPoint);
	}
	else if (bHit)
	{
		if (AActor* HitActor = HitResult.GetActor())
		{
			const float FinalDamage = UMyFpsWeaponDamageLibrary::CalculateDamageForHit(WeaponDefinition, HitResult, ViewLocation);
			const float ActualDamage = UGameplayStatics::ApplyDamage(
				HitActor,
				FinalDamage,
				CharacterOwner ? CharacterOwner->GetController() : nullptr,
				CharacterOwner,
				UDamageType::StaticClass()
			);

			if (ActualDamage > 0.0f)
			{
				bool bKill = false;
				if (const AMyFpsCharacter* HitCharacter = Cast<AMyFpsCharacter>(HitActor))
				{
					bKill = HitCharacter->IsDead();
				}
				else if (const AEnemyCharacter* HitEnemy = Cast<AEnemyCharacter>(HitActor))
				{
					bKill = HitEnemy->IsDead();
				}

				if (AMyFpsPlayerController* InstigatorController = CharacterOwner
					? Cast<AMyFpsPlayerController>(CharacterOwner->GetController())
					: nullptr)
				{
					UE_LOG(LogTemp, Warning, TEXT("[HitMarker] Server damage confirmed. Damage=%.1f Target=%s Kill=%d Controller=%s"), ActualDamage, *GetNameSafe(HitActor), bKill, *GetNameSafe(InstigatorController));
					InstigatorController->ClientShowHitMarker(bKill);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("[HitMarker] Server damage confirmed, but shooter controller is not MyFpsPlayerController. Controller=%s"), CharacterOwner ? *GetNameSafe(CharacterOwner->GetController()) : TEXT("None"));
				}
			}
		}
	}

	MulticastFireCosmetics(MuzzleLocation, AimPoint);
	if (bShouldPlayBoltAction)
	{
		MulticastBoltActionCosmetics();
	}
}

bool UMyFpsWeaponCombatComponent::GetViewTraceData(FVector& OutViewLocation, FRotator& OutViewRotation) const
{
	if (!CharacterOwner)
	{
		return false;
	}

	if (APlayerController* PlayerController = Cast<APlayerController>(CharacterOwner->GetController()))
	{
		if (PlayerController->PlayerCameraManager)
		{
			OutViewLocation = PlayerController->PlayerCameraManager->GetCameraLocation();
			OutViewRotation = PlayerController->PlayerCameraManager->GetCameraRotation();
			return true;
		}
	}

	if (UCameraComponent* FirstPersonCamera = CharacterOwner->GetFirstPersonCameraComponent())
	{
		OutViewLocation = FirstPersonCamera->GetComponentLocation();
		OutViewRotation = FirstPersonCamera->GetComponentRotation();
		return true;
	}

	OutViewLocation = CharacterOwner->GetPawnViewLocation();
	OutViewRotation = CharacterOwner->GetBaseAimRotation();
	return true;
}

bool UMyFpsWeaponCombatComponent::TraceAim(
	const FVector& ViewLocation,
	const FRotator& ViewRotation,
	FHitResult& OutHitResult,
	FVector& OutAimPoint) const
{
	if (!GetWorld() || !InventoryComponent)
	{
		return false;
	}

	const UMyFpsWeaponDefinition* WeaponDefinition = InventoryComponent->GetCurrentWeaponDefinition();
	const float TraceDistance = WeaponDefinition ? WeaponDefinition->GetTraceDistance() : 100000.0f;
	const FVector TraceEnd = ViewLocation + ViewRotation.Vector() * TraceDistance;

	static constexpr ECollisionChannel BulletTraceChannel = ECC_GameTraceChannel2;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(NewWeaponAimTrace), true, CharacterOwner);
	QueryParams.AddIgnoredActor(CharacterOwner);
	QueryParams.bTraceComplex = true;
	QueryParams.bReturnPhysicalMaterial = true;

	TArray<FHitResult> HitResults;
	if (GetWorld()->LineTraceMultiByChannel(HitResults, ViewLocation, TraceEnd, BulletTraceChannel, QueryParams))
	{
		for (const FHitResult& HitResult : HitResults)
		{
			AActor* HitActor = HitResult.GetActor();
			if (!HitActor || HitActor == CharacterOwner || HitActor->IsA<AMyFpsProjectile>())
			{
				continue;
			}

			OutHitResult = HitResult;
			OutAimPoint = HitResult.ImpactPoint;
			return true;
		}
	}

	OutHitResult = FHitResult();
	OutAimPoint = TraceEnd;
	return false;
}

FVector UMyFpsWeaponCombatComponent::GetMuzzleLocation(
	const FVector& ViewLocation,
	const FRotator& ViewRotation,
	const FVector& AimPoint) const
{
	const UMyFpsWeaponDefinition* WeaponDefinition = InventoryComponent
		? InventoryComponent->GetCurrentWeaponDefinition()
		: nullptr;

	if (!WeaponDefinition || !CharacterOwner)
	{
		return ViewLocation;
	}

	const UMyFpsWeaponViewComponent* CurrentWeaponViewComponent = WeaponViewComponent
		? WeaponViewComponent.Get()
		: CharacterOwner->FindComponentByClass<UMyFpsWeaponViewComponent>();
	if (CurrentWeaponViewComponent)
	{
		return CurrentWeaponViewComponent->GetMuzzleTransform(ViewRotation).GetLocation();
	}

	return CharacterOwner->GetActorLocation() + ViewRotation.RotateVector(WeaponDefinition->MuzzleOffset);
}

bool UMyFpsWeaponCombatComponent::ShouldPlayBoltAction(const UMyFpsWeaponDefinition* WeaponDefinition, bool bHasAmmoAfterShot) const
{
	return bHasAmmoAfterShot
		&& WeaponDefinition != nullptr
		&& (WeaponDefinition->BoltActionAnimation != nullptr
			|| WeaponDefinition->ThirdPersonBoltActionAnimation != nullptr
			|| WeaponDefinition->BoltActionTime > 0.0f);
}

void UMyFpsWeaponCombatComponent::SpawnProjectile(
	const UMyFpsWeaponDefinition* WeaponDefinition,
	const FVector& MuzzleLocation,
	const FVector& AimPoint) const
{
	if (!GetWorld() || !WeaponDefinition || !WeaponDefinition->ProjectileClass || !CharacterOwner)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ProjectileWeapon] Spawn failed. Weapon=%s ProjectileClass=%s Character=%s"),
			*GetNameSafe(WeaponDefinition),
			WeaponDefinition && WeaponDefinition->ProjectileClass ? *GetNameSafe(WeaponDefinition->ProjectileClass) : TEXT("None"),
			*GetNameSafe(CharacterOwner));
		return;
	}

	const FVector Direction = (AimPoint - MuzzleLocation).GetSafeNormal();
	const FRotator SpawnRotation = Direction.IsNearlyZero()
		? CharacterOwner->GetBaseAimRotation()
		: Direction.Rotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = CharacterOwner;
	SpawnParams.Instigator = CharacterOwner;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AMyFpsProjectile* Projectile = GetWorld()->SpawnActor<AMyFpsProjectile>(
		WeaponDefinition->ProjectileClass,
		MuzzleLocation,
		SpawnRotation,
		SpawnParams);
	if (!Projectile)
	{
		return;
	}

	Projectile->SetWeaponDefinition(const_cast<UMyFpsWeaponDefinition*>(WeaponDefinition), MuzzleLocation);
	if (USphereComponent* CollisionComp = Projectile->GetCollisionComp())
	{
		CollisionComp->IgnoreActorWhenMoving(CharacterOwner, true);
	}

	if (UProjectileMovementComponent* ProjectileMovement = Projectile->GetProjectileMovement())
	{
		const float ProjectileSpeed = FMath::Max(1.0f, ProjectileMovement->InitialSpeed);
		ProjectileMovement->Velocity = SpawnRotation.Vector() * ProjectileSpeed;
		ProjectileMovement->UpdateComponentVelocity();
	}
}

void UMyFpsWeaponCombatComponent::MulticastFireCosmetics_Implementation(
	const FVector_NetQuantize& MuzzleLocation,
	const FVector_NetQuantize& AimPoint)
{
	FVector LocalMuzzleLocation = MuzzleLocation;
	const FVector Direction = (FVector(AimPoint) - FVector(MuzzleLocation)).GetSafeNormal();
	const FRotator FallbackRotation = Direction.IsNearlyZero() ? FRotator::ZeroRotator : Direction.Rotation();

	if (!CharacterOwner)
	{
		CharacterOwner = Cast<AMyFpsCharacter>(GetOwner());
	}

	if (CharacterOwner && !WeaponViewComponent)
	{
		WeaponViewComponent = CharacterOwner->FindComponentByClass<UMyFpsWeaponViewComponent>();
	}

	if (WeaponViewComponent)
	{
		WeaponViewComponent->PlayFireCosmetics();
		LocalMuzzleLocation = WeaponViewComponent->GetMuzzleTransform(FallbackRotation).GetLocation();
	}

	SpawnTracerEffect(LocalMuzzleLocation, AimPoint);
}

void UMyFpsWeaponCombatComponent::MulticastBoltActionCosmetics_Implementation()
{
	if (!CharacterOwner)
	{
		CharacterOwner = Cast<AMyFpsCharacter>(GetOwner());
	}

	if (CharacterOwner && !WeaponViewComponent)
	{
		WeaponViewComponent = CharacterOwner->FindComponentByClass<UMyFpsWeaponViewComponent>();
	}

	if (WeaponViewComponent)
	{
		WeaponViewComponent->PlayBoltActionCosmetics();
	}
}

void UMyFpsWeaponCombatComponent::MulticastReloadCosmetics_Implementation()
{
	if (!CharacterOwner)
	{
		CharacterOwner = Cast<AMyFpsCharacter>(GetOwner());
	}

	if (CharacterOwner && !WeaponViewComponent)
	{
		WeaponViewComponent = CharacterOwner->FindComponentByClass<UMyFpsWeaponViewComponent>();
	}

	if (CharacterOwner && !WeaponRecoilComponent)
	{
		WeaponRecoilComponent = CharacterOwner->FindComponentByClass<UMyFpsWeaponRecoilComponent>();
	}

	if (WeaponRecoilComponent)
	{
		WeaponRecoilComponent->ResetRecoilState();
	}

	if (WeaponViewComponent)
	{
		WeaponViewComponent->PlayReloadCosmetics();
	}
}

void UMyFpsWeaponCombatComponent::SpawnTracerEffect(const FVector& MuzzleLocation, const FVector& AimPoint) const
{
	const UMyFpsWeaponDefinition* WeaponDefinition = InventoryComponent
		? InventoryComponent->GetCurrentWeaponDefinition()
		: nullptr;
	if (!WeaponDefinition || !WeaponDefinition->TracerEffect || !GetWorld())
	{
		return;
	}

	UNiagaraComponent* NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		WeaponDefinition->TracerEffect,
		MuzzleLocation,
		(AimPoint - MuzzleLocation).Rotation()
	);
	if (!NiagaraComponent)
	{
		return;
	}

	static const FName MuzzleLocationVarName(TEXT("User.MuzzleLocation"));
	static const FName ImpactLocationVarName(TEXT("User.ImpactLocation"));
	NiagaraComponent->SetVariableVec3(MuzzleLocationVarName, MuzzleLocation);
	NiagaraComponent->SetVariableVec3(ImpactLocationVarName, AimPoint);
}
