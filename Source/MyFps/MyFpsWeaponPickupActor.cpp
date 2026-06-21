// Copyright Epic Games, Inc. All Rights Reserved.

#include "MyFpsWeaponPickupActor.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "MyFpsCharacter.h"
#include "MyFpsWeaponDefinition.h"
#include "MyFpsWeaponInventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "TP_PickUpComponent.h"

AMyFpsWeaponPickupActor::AMyFpsWeaponPickupActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));

	PickupMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PickupMesh"));
	SetRootComponent(PickupMesh);
	SceneRoot->SetupAttachment(PickupMesh);
	PickupMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PickupMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	PickupMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	PickupMesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	PickupMesh->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	PickupMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	PickupMesh->CanCharacterStepUpOn = ECB_No;
	PickupMesh->SetSimulatePhysics(false);
	PickupMesh->SetEnableGravity(false);
	PickupMesh->SetIsReplicated(true);

	InteractionSphere = CreateDefaultSubobject<UTP_PickUpComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(PickupMesh);
	InteractionSphere->InitSphereRadius(120.0f);

	PickupPromptWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupPromptWidget"));
	PickupPromptWidget->SetupAttachment(PickupMesh);
	PickupPromptWidget->SetRelativeLocation(FVector(0.0f, 0.0f, 80.0f));
	PickupPromptWidget->SetWidgetSpace(EWidgetSpace::Screen);
	PickupPromptWidget->SetDrawSize(FVector2D(260.0f, 48.0f));
	PickupPromptWidget->SetVisibility(false);
}

void AMyFpsWeaponPickupActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ApplyPickupMesh();
}

void AMyFpsWeaponPickupActor::BeginPlay()
{
	Super::BeginPlay();

	ApplyPickupDisplayData();

	ApplyPickupMesh();
	ApplyPickupEnabledState();
}

void AMyFpsWeaponPickupActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMyFpsWeaponPickupActor, WeaponDefinition);
	DOREPLIFETIME(AMyFpsWeaponPickupActor, bPickupEnabled);
}

bool AMyFpsWeaponPickupActor::TryPickup(AMyFpsCharacter* PickupCharacter)
{
	if (!HasAuthority() || bDroppedPhysicsActive || !bPickupEnabled || !PickupCharacter || !WeaponDefinition)
	{
		return false;
	}

	UMyFpsWeaponInventoryComponent* InventoryComponent = PickupCharacter->GetWeaponInventoryComponent();
	if (!InventoryComponent || !InventoryComponent->EquipWeaponDefinition(WeaponDefinition))
	{
		return false;
	}

	SetPickupEnabled(false);
	HidePickupPrompt();
	SetLifeSpan(0.1f);
	return true;
}

void AMyFpsWeaponPickupActor::SetWeaponDefinition(UMyFpsWeaponDefinition* NewWeaponDefinition)
{
	if (HasAuthority())
	{
		WeaponDefinition = NewWeaponDefinition;
		ForceNetUpdate();
	}

	ApplyPickupMesh();
	ApplyPickupDisplayData();
}

void AMyFpsWeaponPickupActor::StartDroppedPhysics(const FVector& DropImpulse)
{
	if (!HasAuthority() || !PickupMesh)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	bDroppedPhysicsActive = true;
	DroppedPhysicsStartTime = World ? World->GetTimeSeconds() : 0.0f;
	DroppedStableTime = 0.0f;
	HidePickupPrompt();

	if (InteractionSphere)
	{
		InteractionSphere->SetPickupEnabled(false);
	}

	PickupMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	PickupMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	PickupMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	PickupMesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	PickupMesh->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	PickupMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	PickupMesh->CanCharacterStepUpOn = ECB_No;
	PickupMesh->SetEnableGravity(true);
	PickupMesh->SetLinearDamping(DroppedLinearDamping);
	PickupMesh->SetAngularDamping(DroppedAngularDamping);
	PickupMesh->SetSimulatePhysics(true);
	PickupMesh->WakeAllRigidBodies();

	if (!DropImpulse.IsNearlyZero())
	{
		PickupMesh->AddImpulse(DropImpulse, NAME_None, true);
	}

	if (!FMath::IsNearlyZero(DroppedAngularImpulseStrength))
	{
		PickupMesh->AddAngularImpulseInDegrees(
			FVector(DroppedAngularImpulseStrength, DroppedAngularImpulseStrength * 0.35f, DroppedAngularImpulseStrength * 0.15f),
			NAME_None,
			true);
	}

	GetWorldTimerManager().SetTimer(
		DroppedPhysicsTimerHandle,
		this,
		&AMyFpsWeaponPickupActor::UpdateDroppedPhysicsState,
		FMath::Max(0.01f, DroppedPhysicsCheckInterval),
		true);
}

void AMyFpsWeaponPickupActor::HandlePickupRequested(AMyFpsCharacter* PickupCharacter)
{
	TryPickup(PickupCharacter);
}

void AMyFpsWeaponPickupActor::SetPickupEnabled(bool bEnabled)
{
	if (HasAuthority())
	{
		bPickupEnabled = bEnabled;
	}

	ApplyPickupEnabledState();
}

void AMyFpsWeaponPickupActor::SetMeshHighlight(bool bEnabled)
{
	if (!PickupMesh)
	{
		return;
	}

	PickupMesh->SetRenderCustomDepth(bEnabled);
	if (bEnabled)
	{
		PickupMesh->SetCustomDepthStencilValue(HighlightCustomDepthStencil);
	}
}

bool AMyFpsWeaponPickupActor::IsPickupMeshComponent(const UPrimitiveComponent* Component) const
{
	return Component && Component == PickupMesh;
}

void AMyFpsWeaponPickupActor::OnRep_PickupEnabled()
{
	ApplyPickupEnabledState();
}

void AMyFpsWeaponPickupActor::OnRep_WeaponDefinition()
{
	ApplyPickupMesh();
	ApplyPickupDisplayData();
}

void AMyFpsWeaponPickupActor::ApplyPickupEnabledState()
{
	SetActorEnableCollision(bPickupEnabled);

	if (PickupMesh)
	{
		PickupMesh->SetVisibility(bPickupEnabled, true);
		if (!bPickupEnabled)
		{
			SetMeshHighlight(false);
		}

		if (!bDroppedPhysicsActive)
		{
			PickupMesh->SetCollisionEnabled(bPickupEnabled ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
			PickupMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
			PickupMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
			PickupMesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
			PickupMesh->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
			PickupMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
			PickupMesh->CanCharacterStepUpOn = ECB_No;
		}
	}

	if (InteractionSphere)
	{
		InteractionSphere->SetPickupEnabled(bPickupEnabled);
	}

	if (PickupPromptWidget)
	{
		PickupPromptWidget->SetVisibility(false);
	}

	if (!bPickupEnabled)
	{
		HidePickupPrompt();
	}
}

void AMyFpsWeaponPickupActor::ApplyPickupMesh()
{
	if (PickupMesh)
	{
		USkeletalMesh* MeshToUse = PickupSkeletalMesh;
		if (!MeshToUse && WeaponDefinition)
		{
			MeshToUse = WeaponDefinition->ThirdPersonWeaponMesh;
		}

		PickupMesh->SetSkeletalMesh(MeshToUse);
	}
}

void AMyFpsWeaponPickupActor::ApplyPickupDisplayData()
{
	if (!InteractionSphere)
	{
		return;
	}

	InteractionSphere->WeaponDisplayName = WeaponDefinition
		? WeaponDefinition->DisplayName
		: FText::FromString(TEXT("Weapon"));

	if (!InteractionSphere->OnPickUp.IsAlreadyBound(this, &AMyFpsWeaponPickupActor::HandlePickupRequested))
	{
		InteractionSphere->OnPickUp.AddDynamic(this, &AMyFpsWeaponPickupActor::HandlePickupRequested);
	}
}

void AMyFpsWeaponPickupActor::HidePickupPrompt()
{
	if (PickupPromptWidget)
	{
		PickupPromptWidget->SetVisibility(false);
	}
}

void AMyFpsWeaponPickupActor::UpdateDroppedPhysicsState()
{
	if (!HasAuthority() || !bDroppedPhysicsActive || !PickupMesh)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const float Now = World->GetTimeSeconds();
	const float ElapsedTime = Now - DroppedPhysicsStartTime;

	if (ElapsedTime >= DroppedMaxPhysicsDuration)
	{
		SnapToGroundIfPossible();
		FreezeDroppedPhysics();
		return;
	}

	FHitResult GroundHit;
	const bool bNearGround = FindGroundBelow(GroundHit, DroppedGroundTraceDistance);
	const bool bLinearStable = PickupMesh->GetPhysicsLinearVelocity().Size() <= DroppedLinearSleepSpeed;
	const bool bAngularStable = PickupMesh->GetPhysicsAngularVelocityInDegrees().Size() <= DroppedAngularSleepSpeed;
	const bool bCanSettle = ElapsedTime >= DroppedPickupEnableDelay;

	if (bNearGround && bLinearStable && bAngularStable && bCanSettle)
	{
		DroppedStableTime += FMath::Max(0.01f, DroppedPhysicsCheckInterval);
		if (DroppedStableTime >= DroppedStableRequiredTime)
		{
			SnapToGroundIfPossible();
			FreezeDroppedPhysics();
		}
	}
	else
	{
		DroppedStableTime = 0.0f;
	}
}

bool AMyFpsWeaponPickupActor::FindGroundBelow(FHitResult& OutGroundHit, float TraceDistance) const
{
	UWorld* World = GetWorld();
	if (!World || !PickupMesh)
	{
		return false;
	}

	const FVector TraceStart = PickupMesh->GetComponentLocation();
	const FVector TraceEnd = TraceStart - FVector::UpVector * FMath::Max(1.0f, TraceDistance);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(DroppedWeaponGroundTrace), false, this);
	QueryParams.AddIgnoredActor(this);

	return World->LineTraceSingleByChannel(
		OutGroundHit,
		TraceStart,
		TraceEnd,
		ECC_Visibility,
		QueryParams)
		&& OutGroundHit.bBlockingHit;
}

void AMyFpsWeaponPickupActor::SnapToGroundIfPossible()
{
	if (!HasAuthority() || !PickupMesh)
	{
		return;
	}

	FHitResult GroundHit;
	if (!FindGroundBelow(GroundHit, DroppedFallbackGroundTraceDistance))
	{
		return;
	}

	SetActorLocation(GroundHit.ImpactPoint + FVector::UpVector * DroppedSnapToGroundOffset, false, nullptr, ETeleportType::TeleportPhysics);
}

void AMyFpsWeaponPickupActor::FreezeDroppedPhysics()
{
	if (!HasAuthority() || !PickupMesh)
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(DroppedPhysicsTimerHandle);
	PickupMesh->SetSimulatePhysics(false);
	PickupMesh->SetEnableGravity(false);
	PickupMesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
	PickupMesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	PickupMesh->SetLinearDamping(0.0f);
	PickupMesh->SetAngularDamping(0.0f);
	bDroppedPhysicsActive = false;

	ApplyPickupEnabledState();
	ForceNetUpdate();
}
