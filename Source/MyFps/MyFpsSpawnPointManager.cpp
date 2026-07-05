// Copyright Epic Games, Inc. All Rights Reserved.

#include "MyFpsSpawnPointManager.h"

#include "EngineUtils.h"
#include "MyFpsEnemySpawnPoint.h"
#include "MyFpsSpawnGenerationDefinition.h"
#include "MyFpsWeaponSpawnPoint.h"

AMyFpsSpawnPointManager::AMyFpsSpawnPointManager()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
}

void AMyFpsSpawnPointManager::RebuildFromWorld()
{
	ClearManagedLists();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Manager 不生成点，只扫描关卡中带指定 Tag 的武器刷新点并缓存引用。
	for (TActorIterator<AMyFpsWeaponSpawnPoint> ActorIterator(World); ActorIterator; ++ActorIterator)
	{
		AMyFpsWeaponSpawnPoint* SpawnPoint = *ActorIterator;
		if (SpawnPoint
			&& SpawnPoint->Tags.Contains(ManagedActorTag)
			&& SpawnPoint->Tags.Contains(WeaponSpawnPointTag))
		{
			WeaponSpawnPoints.Add(SpawnPoint);
		}
	}

	// 同样扫描敌人刷新点，便于策划在一个 Actor 上查看完整清单。
	for (TActorIterator<AMyFpsEnemySpawnPoint> ActorIterator(World); ActorIterator; ++ActorIterator)
	{
		AMyFpsEnemySpawnPoint* SpawnPoint = *ActorIterator;
		if (SpawnPoint
			&& SpawnPoint->Tags.Contains(ManagedActorTag)
			&& SpawnPoint->Tags.Contains(EnemySpawnPointTag))
		{
			EnemySpawnPoints.Add(SpawnPoint);
		}
	}

	RefreshSummary();
}

void AMyFpsSpawnPointManager::ClearManagedLists()
{
	WeaponSpawnPoints.Reset();
	EnemySpawnPoints.Reset();
	RefreshSummary();
}

void AMyFpsSpawnPointManager::RefreshSummary()
{
	// Details 面板中的简短摘要，方便不展开数组也能看到当前配置状态。
	Summary = FString::Printf(
		TEXT("Weapon Spawn Points: %d\nEnemy Spawn Points: %d\nGeneration Definition: %s"),
		WeaponSpawnPoints.Num(),
		EnemySpawnPoints.Num(),
		*GetNameSafe(SpawnGenerationDefinition.Get()));
}
