// Copyright Epic Games, Inc. All Rights Reserved.

#include "MyFpsWeaponSpawnPoint.h"

#include "Components/ArrowComponent.h"
#include "Components/BillboardComponent.h"
#include "Components/SphereComponent.h"
#include "MyFpsWeaponPickupActor.h"

AMyFpsWeaponSpawnPoint::AMyFpsWeaponSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	BillboardComponent = CreateDefaultSubobject<UBillboardComponent>(TEXT("BillboardComponent"));
	BillboardComponent->SetupAttachment(SceneRoot);

	// 只作为编辑器预览半径，不参与运行时碰撞。
	PreviewRadiusComponent = CreateDefaultSubobject<USphereComponent>(TEXT("PreviewRadius"));
	PreviewRadiusComponent->SetupAttachment(SceneRoot);
	PreviewRadiusComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PreviewRadiusComponent->SetGenerateOverlapEvents(false);

	ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComponent"));
	ArrowComponent->SetupAttachment(SceneRoot);

	WeaponPickupActorClass = AMyFpsWeaponPickupActor::StaticClass();
	ApplyPreviewSettings();
}

void AMyFpsWeaponSpawnPoint::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ApplyPreviewSettings();
}

void AMyFpsWeaponSpawnPoint::ConfigureWeaponSpawnPoint(
	UMyFpsWeaponDefinition* NewWeaponDefinition,
	TSubclassOf<AMyFpsWeaponPickupActor> NewWeaponPickupActorClass)
{
	// EUW 生成刷新点后，会把 DA 和拾取 Actor 类型写进关卡里的 SpawnPoint。
	WeaponDefinition = NewWeaponDefinition;
	if (NewWeaponPickupActorClass)
	{
		WeaponPickupActorClass = NewWeaponPickupActorClass;
	}
}

void AMyFpsWeaponSpawnPoint::ConfigureWeaponSpawnSettings(
	bool bNewEnabled,
	bool bNewSpawnOnBeginPlay,
	bool bNewRespawnAfterPickup,
	float NewRespawnDelay)
{
	// 这些是运行时刷新规则，保存到关卡 Actor 后由服务器在 BeginPlay 使用。
	bEnabled = bNewEnabled;
	bSpawnOnBeginPlay = bNewSpawnOnBeginPlay;
	bRespawnAfterPickup = bNewRespawnAfterPickup;
	RespawnDelay = FMath::Max(0.0f, NewRespawnDelay);
}

void AMyFpsWeaponSpawnPoint::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() && bEnabled && bSpawnOnBeginPlay)
	{
		// 多人模式下只允许服务器生成实际武器，客户端通过复制看到结果。
		SpawnWeapon();
	}
}

AMyFpsWeaponPickupActor* AMyFpsWeaponSpawnPoint::SpawnWeapon()
{
	if (!HasAuthority() || !bEnabled || SpawnedWeapon || !WeaponPickupActorClass || !WeaponDefinition)
	{
		return SpawnedWeapon;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// 刷新点运行时生成的是可拾取武器 Actor，并把 WeaponDefinition 注入进去。
	AMyFpsWeaponPickupActor* WeaponPickup = World->SpawnActor<AMyFpsWeaponPickupActor>(
		WeaponPickupActorClass,
		GetActorLocation(),
		GetActorRotation(),
		SpawnParameters);
	if (!WeaponPickup)
	{
		return nullptr;
	}

	WeaponPickup->SetWeaponDefinition(WeaponDefinition);
	WeaponPickup->SetPickupEnabled(true);
	// 监听武器被销毁，用于拾取后延迟重刷。
	WeaponPickup->OnDestroyed.AddDynamic(this, &AMyFpsWeaponSpawnPoint::HandleSpawnedWeaponDestroyed);
	SpawnedWeapon = WeaponPickup;
	return SpawnedWeapon;
}

void AMyFpsWeaponSpawnPoint::ClearSpawnedWeapon()
{
	GetWorldTimerManager().ClearTimer(RespawnTimerHandle);

	if (SpawnedWeapon)
	{
		SpawnedWeapon->OnDestroyed.RemoveDynamic(this, &AMyFpsWeaponSpawnPoint::HandleSpawnedWeaponDestroyed);
		SpawnedWeapon->Destroy();
		SpawnedWeapon = nullptr;
	}
}

void AMyFpsWeaponSpawnPoint::HandleSpawnedWeaponDestroyed(AActor* DestroyedActor)
{
	if (DestroyedActor == SpawnedWeapon)
	{
		SpawnedWeapon = nullptr;
	}

	if (HasAuthority() && bEnabled && bRespawnAfterPickup)
	{
		// 当前武器消失后，由刷新点负责安排下一次生成。
		ScheduleRespawn();
	}
}

void AMyFpsWeaponSpawnPoint::ScheduleRespawn()
{
	if (RespawnDelay <= 0.0f)
	{
		SpawnWeapon();
		return;
	}

	GetWorldTimerManager().SetTimer(
		RespawnTimerHandle,
		this,
		&AMyFpsWeaponSpawnPoint::SpawnWeaponFromTimer,
		RespawnDelay,
		false);
}

void AMyFpsWeaponSpawnPoint::SpawnWeaponFromTimer()
{
	// Timer 要求 void() 签名，所以用这个包装函数调用真正的 SpawnWeapon。
	SpawnWeapon();
}

void AMyFpsWeaponSpawnPoint::ApplyPreviewSettings()
{
	if (PreviewRadiusComponent)
	{
		PreviewRadiusComponent->SetSphereRadius(FMath::Max(1.0f, PreviewRadius));
	}
}
