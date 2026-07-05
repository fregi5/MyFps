// Copyright Epic Games, Inc. All Rights Reserved.

#include "MyFpsEnemySpawnPoint.h"

#include "Components/ArrowComponent.h"
#include "Components/BillboardComponent.h"
#include "Components/SphereComponent.h"
#include "EnemyCharacter.h"

AMyFpsEnemySpawnPoint::AMyFpsEnemySpawnPoint()
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

	EnemyActorClass = AEnemyCharacter::StaticClass();
	ApplyPreviewSettings();
}

void AMyFpsEnemySpawnPoint::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ApplyPreviewSettings();
}

void AMyFpsEnemySpawnPoint::ConfigureEnemySpawnPoint(TSubclassOf<AEnemyCharacter> NewEnemyActorClass)
{
	// EUW 生成刷新点后，会把敌人类型写进关卡里的 SpawnPoint。
	if (NewEnemyActorClass)
	{
		EnemyActorClass = NewEnemyActorClass;
	}
}

void AMyFpsEnemySpawnPoint::ConfigureEnemySpawnSettings(
	bool bNewEnabled,
	bool bNewSpawnOnBeginPlay,
	bool bNewRespawnAfterDestroyed,
	float NewRespawnDelay)
{
	// 这些是运行时刷新规则，保存到关卡 Actor 后由服务器在 BeginPlay 使用。
	bEnabled = bNewEnabled;
	bSpawnOnBeginPlay = bNewSpawnOnBeginPlay;
	bRespawnAfterDestroyed = bNewRespawnAfterDestroyed;
	RespawnDelay = FMath::Max(0.0f, NewRespawnDelay);
}

void AMyFpsEnemySpawnPoint::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() && bEnabled && bSpawnOnBeginPlay)
	{
		// 多人模式下只允许服务器生成实际敌人，客户端通过复制看到结果。
		SpawnEnemy();
	}
}

AEnemyCharacter* AMyFpsEnemySpawnPoint::SpawnEnemy()
{
	if (!HasAuthority() || !bEnabled || SpawnedEnemy || !EnemyActorClass)
	{
		return SpawnedEnemy;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// 刷新点运行时生成真正的敌人 Actor。
	AEnemyCharacter* Enemy = World->SpawnActor<AEnemyCharacter>(
		EnemyActorClass,
		GetActorLocation(),
		GetActorRotation(),
		SpawnParameters);
	if (!Enemy)
	{
		return nullptr;
	}

	// 监听敌人死亡/销毁，用于延迟重刷。
	Enemy->OnDestroyed.AddDynamic(this, &AMyFpsEnemySpawnPoint::HandleSpawnedEnemyDestroyed);
	SpawnedEnemy = Enemy;
	return SpawnedEnemy;
}

void AMyFpsEnemySpawnPoint::ClearSpawnedEnemy()
{
	GetWorldTimerManager().ClearTimer(RespawnTimerHandle);

	if (SpawnedEnemy)
	{
		SpawnedEnemy->OnDestroyed.RemoveDynamic(this, &AMyFpsEnemySpawnPoint::HandleSpawnedEnemyDestroyed);
		SpawnedEnemy->Destroy();
		SpawnedEnemy = nullptr;
	}
}

void AMyFpsEnemySpawnPoint::HandleSpawnedEnemyDestroyed(AActor* DestroyedActor)
{
	if (DestroyedActor == SpawnedEnemy)
	{
		SpawnedEnemy = nullptr;
	}

	if (HasAuthority() && bEnabled && bRespawnAfterDestroyed)
	{
		// 当前敌人消失后，由刷新点负责安排下一次生成。
		ScheduleRespawn();
	}
}

void AMyFpsEnemySpawnPoint::ScheduleRespawn()
{
	if (RespawnDelay <= 0.0f)
	{
		SpawnEnemy();
		return;
	}

	GetWorldTimerManager().SetTimer(
		RespawnTimerHandle,
		this,
		&AMyFpsEnemySpawnPoint::SpawnEnemyFromTimer,
		RespawnDelay,
		false);
}

void AMyFpsEnemySpawnPoint::SpawnEnemyFromTimer()
{
	// Timer 要求 void() 签名，所以用这个包装函数调用真正的 SpawnEnemy。
	SpawnEnemy();
}

void AMyFpsEnemySpawnPoint::ApplyPreviewSettings()
{
	if (PreviewRadiusComponent)
	{
		PreviewRadiusComponent->SetSphereRadius(FMath::Max(1.0f, PreviewRadius));
	}
}
