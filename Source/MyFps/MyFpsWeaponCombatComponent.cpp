// Copyright Epic Games, Inc. All Rights Reserved.

#include "MyFpsWeaponCombatComponent.h"

#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "EnemyCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "MyFpsCharacter.h"
#include "MyFpsProjectile.h"
#include "MyFpsPlayerController.h"
#include "MyFpsWeaponDefinition.h"
#include "MyFpsWeaponInventoryComponent.h"
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
	}
}

void UMyFpsWeaponCombatComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoFireTimerHandle);
		World->GetTimerManager().ClearTimer(ReloadTimerHandle);
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
		GetWorld()->GetTimerManager().SetTimer(
			AutoFireTimerHandle,
			this,
			&UMyFpsWeaponCombatComponent::FireOnce,
			FMath::Max(0.01f, WeaponDefinition->FireInterval),
			true,
			FMath::Max(0.01f, WeaponDefinition->FireInterval)
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
		ServerReload();
		return;
	}

	if (!CanReload())
	{
		return;
	}

	StopFire();
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

	return Character->HasAuthority()
		&& Character->IsMatchInProgress()
		&& !Character->IsDead()
		&& Inventory->GetCurrentWeaponDefinition() != nullptr
		&& Inventory->HasAmmoInClip()
		&& !Inventory->IsReloading();
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
	if (bHit)
	{
		if (AActor* HitActor = HitResult.GetActor())
		{
			const float ActualDamage = UGameplayStatics::ApplyDamage(
				HitActor,
				WeaponDefinition->Damage,
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

	const FVector MuzzleLocation = GetMuzzleLocation(ViewLocation, ViewRotation, AimPoint);
	MulticastFireCosmetics(MuzzleLocation, AimPoint);
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
	const float TraceDistance = WeaponDefinition ? WeaponDefinition->HitscanDistance : 100000.0f;
	const FVector TraceEnd = ViewLocation + ViewRotation.Vector() * TraceDistance;

	static constexpr ECollisionChannel BulletTraceChannel = ECC_GameTraceChannel2;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(NewWeaponAimTrace), true, CharacterOwner);
	QueryParams.AddIgnoredActor(CharacterOwner);
	QueryParams.bTraceComplex = true;

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
