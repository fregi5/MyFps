// Copyright Epic Games, Inc. All Rights Reserved.

#include "MyFpsWeaponViewComponent.h"

#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Kismet/GameplayStatics.h"
#include "MyFpsCharacter.h"
#include "MyFpsWeaponDefinition.h"
#include "MyFpsWeaponInventoryComponent.h"

UMyFpsWeaponViewComponent::UMyFpsWeaponViewComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UMyFpsWeaponViewComponent::BeginPlay()
{
	Super::BeginPlay();

	CharacterOwner = Cast<AMyFpsCharacter>(GetOwner());
	if (!CharacterOwner)
	{
		return;
	}

	InventoryComponent = CharacterOwner->GetWeaponInventoryComponent();
	if (InventoryComponent)
	{
		InventoryComponent->OnInventoryChanged.AddDynamic(this, &UMyFpsWeaponViewComponent::HandleInventoryChanged);
	}

	EnsureVisualMeshes();
	RefreshWeaponVisuals();
}

void UMyFpsWeaponViewComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (InventoryComponent)
	{
		InventoryComponent->OnInventoryChanged.RemoveDynamic(this, &UMyFpsWeaponViewComponent::HandleInventoryChanged);
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FireMontageStopTimerHandle);
		World->GetTimerManager().ClearTimer(BoltActionDelayTimerHandle);
		World->GetTimerManager().ClearTimer(BoltActionMontageStopTimerHandle);
		World->GetTimerManager().ClearTimer(ReloadMontageStopTimerHandle);
	}

	DestroyVisualMeshes();

	Super::EndPlay(EndPlayReason);
}

void UMyFpsWeaponViewComponent::HandleInventoryChanged()
{
	RefreshWeaponVisuals();
}

void UMyFpsWeaponViewComponent::RefreshWeaponVisuals()
{
	if (!CharacterOwner)
	{
		CharacterOwner = Cast<AMyFpsCharacter>(GetOwner());
	}

	if (!CharacterOwner)
	{
		return;
	}

	if (!InventoryComponent)
	{
		InventoryComponent = CharacterOwner->GetWeaponInventoryComponent();
	}

	EnsureVisualMeshes();

	UMyFpsWeaponDefinition* WeaponDefinition = InventoryComponent
		? InventoryComponent->GetCurrentWeaponDefinition()
		: nullptr;

	if (!InventoryComponent)
	{
		ClearWeaponVisuals();
		return;
	}

	ApplyWeaponDefinition(WeaponDefinition);
}

void UMyFpsWeaponViewComponent::EnsureVisualMeshes()
{
	if (!CharacterOwner)
	{
		return;
	}

	if (!FirstPersonWeaponMesh)
	{
		FirstPersonWeaponMesh = NewObject<USkeletalMeshComponent>(CharacterOwner, TEXT("FirstPersonWeaponViewMesh"));
		if (FirstPersonWeaponMesh)
		{
			FirstPersonWeaponMesh->RegisterComponent();
			FirstPersonWeaponMesh->SetOnlyOwnerSee(true);
			FirstPersonWeaponMesh->SetOwnerNoSee(false);
			FirstPersonWeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			FirstPersonWeaponMesh->SetGenerateOverlapEvents(false);
			FirstPersonWeaponMesh->bCastDynamicShadow = false;
			FirstPersonWeaponMesh->CastShadow = false;
		}
	}

	if (!ThirdPersonWeaponMesh)
	{
		ThirdPersonWeaponMesh = NewObject<USkeletalMeshComponent>(CharacterOwner, TEXT("ThirdPersonWeaponViewMesh"));
		if (ThirdPersonWeaponMesh)
		{
			ThirdPersonWeaponMesh->RegisterComponent();
			ThirdPersonWeaponMesh->SetOnlyOwnerSee(false);
			ThirdPersonWeaponMesh->SetOwnerNoSee(true);
			ThirdPersonWeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			ThirdPersonWeaponMesh->SetGenerateOverlapEvents(false);
		}
	}
}

void UMyFpsWeaponViewComponent::ApplyWeaponDefinition(UMyFpsWeaponDefinition* WeaponDefinition)
{
	if (!CharacterOwner || !WeaponDefinition)
	{
		ClearWeaponVisuals();
		return;
	}

	const bool bHasWeapon = InventoryComponent && InventoryComponent->HasWeapon();
	const bool bShowFirstPerson = bHasWeapon && CharacterOwner->IsLocallyControlled() && !CharacterOwner->IsDead();
	const bool bShowThirdPerson = bHasWeapon && !CharacterOwner->IsDead() && !CharacterOwner->IsLocallyControlled();

	if (FirstPersonWeaponMesh)
	{
		USkeletalMesh* FirstPersonMeshAsset = WeaponDefinition->FirstPersonWeaponMesh
			? WeaponDefinition->FirstPersonWeaponMesh
			: WeaponDefinition->ThirdPersonWeaponMesh;
		FirstPersonWeaponMesh->SetSkeletalMesh(FirstPersonMeshAsset);

		if (USkeletalMeshComponent* Mesh1P = CharacterOwner->GetMesh1P())
		{
			FirstPersonWeaponMesh->AttachToComponent(
				Mesh1P,
				FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true),
				WeaponDefinition->FirstPersonAttachSocketName
			);
			Mesh1P->SetVisibility(bShowFirstPerson, true);
		}

		FirstPersonWeaponMesh->SetVisibility(bShowFirstPerson && FirstPersonMeshAsset != nullptr, true);
	}

	if (ThirdPersonWeaponMesh)
	{
		USkeletalMesh* ThirdPersonMeshAsset = WeaponDefinition->ThirdPersonWeaponMesh
			? WeaponDefinition->ThirdPersonWeaponMesh
			: WeaponDefinition->FirstPersonWeaponMesh;
		ThirdPersonWeaponMesh->SetSkeletalMesh(ThirdPersonMeshAsset);

		if (USkeletalMeshComponent* ThirdPersonMesh = CharacterOwner->GetMesh())
		{
			ThirdPersonWeaponMesh->AttachToComponent(
				ThirdPersonMesh,
				FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true),
				WeaponDefinition->ThirdPersonAttachSocketName
			);
		}

		ThirdPersonWeaponMesh->SetVisibility(bShowThirdPerson && ThirdPersonMeshAsset != nullptr, true);
	}
}

void UMyFpsWeaponViewComponent::ClearWeaponVisuals()
{
	if (FirstPersonWeaponMesh)
	{
		FirstPersonWeaponMesh->SetSkeletalMesh(nullptr);
		FirstPersonWeaponMesh->SetVisibility(false, true);
	}

	if (ThirdPersonWeaponMesh)
	{
		ThirdPersonWeaponMesh->SetSkeletalMesh(nullptr);
		ThirdPersonWeaponMesh->SetVisibility(false, true);
	}

	if (CharacterOwner && CharacterOwner->GetMesh1P())
	{
		CharacterOwner->GetMesh1P()->SetVisibility(false, true);
	}
}

FTransform UMyFpsWeaponViewComponent::GetMuzzleTransform(const FRotator& FallbackRotation) const
{
	const UMyFpsWeaponDefinition* WeaponDefinition = InventoryComponent
		? InventoryComponent->GetCurrentWeaponDefinition()
		: nullptr;
	if (!CharacterOwner || !WeaponDefinition)
	{
		return FTransform(FallbackRotation, FVector::ZeroVector);
	}

	const USkeletalMeshComponent* PreferredWeaponMesh = CharacterOwner->IsLocallyControlled()
		? FirstPersonWeaponMesh.Get()
		: ThirdPersonWeaponMesh.Get();
	const USkeletalMeshComponent* FallbackWeaponMesh = CharacterOwner->IsLocallyControlled()
		? ThirdPersonWeaponMesh.Get()
		: FirstPersonWeaponMesh.Get();

	const USkeletalMeshComponent* MeshesToSearch[] = { PreferredWeaponMesh, FallbackWeaponMesh };
	for (const USkeletalMeshComponent* WeaponMesh : MeshesToSearch)
	{
		if (!WeaponMesh || !WeaponMesh->GetSkeletalMeshAsset())
		{
			continue;
		}

		TArray<USceneComponent*> ChildComponents;
		WeaponMesh->GetChildrenComponents(true, ChildComponents);
		for (const USceneComponent* ChildComponent : ChildComponents)
		{
			if (ChildComponent && ChildComponent->ComponentHasTag(WeaponDefinition->MuzzleComponentTag))
			{
				return ChildComponent->GetComponentTransform();
			}
		}

		static const FName MuzzleSocketName(TEXT("MuzzlePoint"));
		if (WeaponMesh->DoesSocketExist(MuzzleSocketName))
		{
			return WeaponMesh->GetSocketTransform(MuzzleSocketName);
		}
	}

	return FTransform(
		FallbackRotation,
		CharacterOwner->GetActorLocation() + FallbackRotation.RotateVector(WeaponDefinition->MuzzleOffset)
	);
}

void UMyFpsWeaponViewComponent::PlayFireCosmetics()
{
	UMyFpsWeaponDefinition* WeaponDefinition = InventoryComponent
		? InventoryComponent->GetCurrentWeaponDefinition()
		: nullptr;
	if (!CharacterOwner || !WeaponDefinition)
	{
		return;
	}

	if (WeaponDefinition->FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, WeaponDefinition->FireSound, CharacterOwner->GetActorLocation());
	}

	UAnimMontage* Montage = CharacterOwner->IsLocallyControlled()
		? WeaponDefinition->FireAnimation
		: WeaponDefinition->ThirdPersonFireAnimation;
	const float PlayedDuration = PlayMontage(Montage);
	const float StopDelay = WeaponDefinition->FireMontageStopDelay > 0.0f
		? WeaponDefinition->FireMontageStopDelay
		: PlayedDuration;
	if (StopDelay > 0.0f && GetWorld())
	{
		ActiveFireMontage = Montage;
		GetWorld()->GetTimerManager().ClearTimer(FireMontageStopTimerHandle);
		GetWorld()->GetTimerManager().SetTimer(
			FireMontageStopTimerHandle,
			this,
			&UMyFpsWeaponViewComponent::StopFireMontage,
			StopDelay,
			false
		);
	}
}

void UMyFpsWeaponViewComponent::PlayBoltActionCosmetics()
{
	UMyFpsWeaponDefinition* WeaponDefinition = InventoryComponent
		? InventoryComponent->GetCurrentWeaponDefinition()
		: nullptr;
	if (!CharacterOwner || !WeaponDefinition || !GetWorld())
	{
		return;
	}

	if (WeaponDefinition->BoltActionDelay > 0.0f)
	{
		GetWorld()->GetTimerManager().ClearTimer(BoltActionDelayTimerHandle);
		GetWorld()->GetTimerManager().SetTimer(
			BoltActionDelayTimerHandle,
			this,
			&UMyFpsWeaponViewComponent::PlayDelayedBoltActionCosmetics,
			WeaponDefinition->BoltActionDelay,
			false
		);
		return;
	}

	PlayDelayedBoltActionCosmetics();
}

void UMyFpsWeaponViewComponent::PlayReloadCosmetics()
{
	UMyFpsWeaponDefinition* WeaponDefinition = InventoryComponent
		? InventoryComponent->GetCurrentWeaponDefinition()
		: nullptr;
	if (!CharacterOwner || !WeaponDefinition)
	{
		return;
	}

	if (WeaponDefinition->ReloadSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, WeaponDefinition->ReloadSound, CharacterOwner->GetActorLocation());
	}

	UAnimMontage* Montage = CharacterOwner->IsLocallyControlled()
		? WeaponDefinition->ReloadAnimation
		: WeaponDefinition->ThirdPersonReloadAnimation;
	const float PlayedDuration = PlayMontage(Montage);
	const float StopDelay = WeaponDefinition->ReloadMontageStopDelay > 0.0f
		? WeaponDefinition->ReloadMontageStopDelay
		: PlayedDuration;
	if (StopDelay > 0.0f && GetWorld())
	{
		ActiveReloadMontage = Montage;
		GetWorld()->GetTimerManager().ClearTimer(ReloadMontageStopTimerHandle);
		GetWorld()->GetTimerManager().SetTimer(
			ReloadMontageStopTimerHandle,
			this,
			&UMyFpsWeaponViewComponent::StopReloadMontage,
			StopDelay,
			false
		);
	}
}

void UMyFpsWeaponViewComponent::PlayDelayedBoltActionCosmetics()
{
	UMyFpsWeaponDefinition* WeaponDefinition = InventoryComponent
		? InventoryComponent->GetCurrentWeaponDefinition()
		: nullptr;
	if (!CharacterOwner || !WeaponDefinition)
	{
		return;
	}

	UAnimMontage* Montage = CharacterOwner->IsLocallyControlled()
		? WeaponDefinition->BoltActionAnimation
		: WeaponDefinition->ThirdPersonBoltActionAnimation;
	const float PlayedDuration = PlayMontage(Montage);
	const float StopDelay = WeaponDefinition->BoltActionMontageStopDelay > 0.0f
		? WeaponDefinition->BoltActionMontageStopDelay
		: PlayedDuration;
	if (StopDelay > 0.0f && GetWorld())
	{
		ActiveBoltActionMontage = Montage;
		GetWorld()->GetTimerManager().ClearTimer(BoltActionMontageStopTimerHandle);
		GetWorld()->GetTimerManager().SetTimer(
			BoltActionMontageStopTimerHandle,
			this,
			&UMyFpsWeaponViewComponent::StopBoltActionMontage,
			StopDelay,
			false
		);
	}
}

float UMyFpsWeaponViewComponent::PlayMontage(UAnimMontage* Montage)
{
	if (!Montage || !CharacterOwner)
	{
		return 0.0f;
	}

	USkeletalMeshComponent* AnimationMesh = CharacterOwner->IsLocallyControlled()
		? CharacterOwner->GetMesh1P()
		: CharacterOwner->GetMesh();
	if (!AnimationMesh)
	{
		return 0.0f;
	}

	UAnimInstance* AnimInstance = AnimationMesh->GetAnimInstance();
	if (!AnimInstance)
	{
		return 0.0f;
	}

	return AnimInstance->Montage_Play(Montage, 1.0f, EMontagePlayReturnType::Duration);
}

void UMyFpsWeaponViewComponent::StopMontage(UAnimMontage* Montage, float BlendOutTime) const
{
	if (!Montage || !CharacterOwner)
	{
		return;
	}

	USkeletalMeshComponent* AnimationMesh = CharacterOwner->IsLocallyControlled()
		? CharacterOwner->GetMesh1P()
		: CharacterOwner->GetMesh();
	if (!AnimationMesh)
	{
		return;
	}

	UAnimInstance* AnimInstance = AnimationMesh->GetAnimInstance();
	if (!AnimInstance || !AnimInstance->Montage_IsPlaying(Montage))
	{
		return;
	}

	AnimInstance->Montage_Stop(BlendOutTime, Montage);
}

void UMyFpsWeaponViewComponent::StopFireMontage()
{
	StopMontage(ActiveFireMontage);
	ActiveFireMontage = nullptr;
}

void UMyFpsWeaponViewComponent::StopBoltActionMontage()
{
	StopMontage(ActiveBoltActionMontage);
	ActiveBoltActionMontage = nullptr;
}

void UMyFpsWeaponViewComponent::StopReloadMontage()
{
	StopMontage(ActiveReloadMontage);
	ActiveReloadMontage = nullptr;
}

void UMyFpsWeaponViewComponent::DestroyVisualMeshes()
{
	if (FirstPersonWeaponMesh)
	{
		FirstPersonWeaponMesh->DestroyComponent();
		FirstPersonWeaponMesh = nullptr;
	}

	if (ThirdPersonWeaponMesh)
	{
		ThirdPersonWeaponMesh->DestroyComponent();
		ThirdPersonWeaponMesh = nullptr;
	}
}
