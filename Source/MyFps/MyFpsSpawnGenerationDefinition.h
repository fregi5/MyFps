// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MyFpsSpawnGenerationDefinition.generated.h"

class AEnemyCharacter;
class AMyFpsWeaponPickupActor;
class UMyFpsWeaponDefinition;

USTRUCT(BlueprintType)
struct FMyFpsWeaponSpawnGenerationRule
{
	GENERATED_BODY()

	// 这条规则要生成的武器配置。真正武器参数仍然来自 WeaponDefinition。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Spawn Rule")
	TObjectPtr<UMyFpsWeaponDefinition> WeaponDefinition = nullptr;

	// 地上可拾取武器 Actor 类型；为空时 EUW 会回退到工具里的默认拾取类。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Spawn Rule")
	TSubclassOf<AMyFpsWeaponPickupActor> WeaponPickupActorClass;

	// 按这条规则生成多少个武器刷新点。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Spawn Rule", meta = (ClampMin = "0"))
	int32 Count = 0;

	// 游戏开始时，刷新点是否立刻生成一把武器。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Spawn Rule")
	bool bSpawnOnBeginPlay = true;

	// 武器被拾取或销毁后，刷新点是否自动重新生成武器。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Spawn Rule")
	bool bRespawnAfterPickup = true;

	// 武器被拾取或销毁后，等待多少秒重新生成。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Spawn Rule", meta = (ClampMin = "0.0", ForceUnits = "s"))
	float RespawnDelay = 20.0f;
};

USTRUCT(BlueprintType)
struct FMyFpsEnemySpawnGenerationRule
{
	GENERATED_BODY()

	// 这条规则要生成的敌人类型。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Spawn Rule")
	TSubclassOf<AEnemyCharacter> EnemyActorClass;

	// 按这条规则生成多少个敌人刷新点。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Spawn Rule", meta = (ClampMin = "0"))
	int32 Count = 0;

	// 游戏开始时，刷新点是否立刻生成敌人。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Spawn Rule")
	bool bSpawnOnBeginPlay = true;

	// 敌人死亡或销毁后，刷新点是否自动重新生成敌人。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Spawn Rule")
	bool bRespawnAfterDestroyed = true;

	// 敌人死亡或销毁后，等待多少秒重新生成。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Spawn Rule", meta = (ClampMin = "0.0", ForceUnits = "s"))
	float RespawnDelay = 10.0f;
};

// 刷新点生成规则 DA：保存“生成多少、生成什么、按什么规则分布”。
// EUW 读取这个资产并把结果落成关卡里的 SpawnPoint Actor。
UCLASS(BlueprintType)
class MYFPS_API UMyFpsSpawnGenerationDefinition : public UDataAsset
{
	GENERATED_BODY()

public:
	// 以选中 Actor 或 ManualCenterLocation 为圆心的生成半径。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn Generation|Common", meta = (ClampMin = "0.0", ForceUnits = "cm"))
	float SpawnRadius = 2000.0f;

	// 生成点之间的最小 2D 间距，用于避免点位堆叠。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn Generation|Common", meta = (ClampMin = "0.0", ForceUnits = "cm"))
	float MinDistanceBetweenPoints = 300.0f;

	// 最大尝试次数，避免半径太小或间距太大时无限循环。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn Generation|Common", meta = (ClampMin = "1"))
	int32 MaxPlacementAttempts = 500;

	// 固定随机种子用于复现生成结果。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn Generation|Common")
	int32 RandomSeed = 1337;

	// 开启后每次按相同规则生成同一批点；关闭后每次点击都会变化。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn Generation|Common")
	bool bUseFixedRandomSeed = false;

	// 生成新点时是否避开关卡中已经由工具生成过的旧点。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn Generation|Common")
	bool bAvoidExistingGeneratedSpawnPoints = true;

	// 生成候选位置后是否向下 Trace，把点贴到地面。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn Generation|Common")
	bool bSnapToGround = true;

	// 贴地 Trace 的起点向上偏移距离。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn Generation|Common", meta = (ClampMin = "0.0", ForceUnits = "cm"))
	float GroundTraceUpDistance = 1000.0f;

	// 贴地 Trace 的终点向下偏移距离。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn Generation|Common", meta = (ClampMin = "0.0", ForceUnits = "cm"))
	float GroundTraceDownDistance = 5000.0f;

	// 是否给生成的刷新点随机 Yaw 朝向。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn Generation|Common")
	bool bRandomYaw = true;

	// 武器刷新点生成规则列表，一条规则可以生成同一种武器的多个点。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn Generation|Weapon")
	TArray<FMyFpsWeaponSpawnGenerationRule> WeaponRules;

	// 敌人刷新点生成规则列表，一条规则可以生成同一种敌人的多个点。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn Generation|Enemy")
	TArray<FMyFpsEnemySpawnGenerationRule> EnemyRules;
};
