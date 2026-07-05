// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyFpsSpawnPointManager.generated.h"

class AMyFpsEnemySpawnPoint;
class AMyFpsWeaponSpawnPoint;
class UMyFpsSpawnGenerationDefinition;

UCLASS()
class MYFPS_API AMyFpsSpawnPointManager : public AActor
{
	GENERATED_BODY()

public:
	AMyFpsSpawnPointManager();

	// 重新扫描当前关卡，收集所有带工具 Tag 的武器/敌人刷新点。
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Spawn Point Manager")
	void RebuildFromWorld();

	// 清空当前缓存列表；不会删除关卡里的 SpawnPoint Actor。
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Spawn Point Manager")
	void ClearManagedLists();

	// 刷新 Details 面板里显示的统计文本。
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Spawn Point Manager")
	void RefreshSummary();

	UFUNCTION(BlueprintPure, Category = "Spawn Point Manager")
	int32 GetWeaponSpawnPointCount() const { return WeaponSpawnPoints.Num(); }

	UFUNCTION(BlueprintPure, Category = "Spawn Point Manager")
	int32 GetEnemySpawnPointCount() const { return EnemySpawnPoints.Num(); }

	// EUW 生成的所有刷新点都会带这个公共 Tag，Manager 靠它筛选工具产物。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn Point Manager")
	FName ManagedActorTag = TEXT("MyFpsGeneratedSpawnPoint");

	// 武器刷新点的细分 Tag。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn Point Manager")
	FName WeaponSpawnPointTag = TEXT("MyFpsGeneratedWeaponSpawnPoint");

	// 敌人刷新点的细分 Tag。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn Point Manager")
	FName EnemySpawnPointTag = TEXT("MyFpsGeneratedEnemySpawnPoint");

	// 当前关卡使用的生成规则资产，方便策划回溯这批点来自哪份配置。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn Point Manager")
	TObjectPtr<UMyFpsSpawnGenerationDefinition> SpawnGenerationDefinition = nullptr;

	// 当前关卡内由工具生成的武器刷新点缓存列表。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawn Point Manager")
	TArray<TObjectPtr<AMyFpsWeaponSpawnPoint>> WeaponSpawnPoints;

	// 当前关卡内由工具生成的敌人刷新点缓存列表。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawn Point Manager")
	TArray<TObjectPtr<AMyFpsEnemySpawnPoint>> EnemySpawnPoints;

	// 给策划看的简短统计信息。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawn Point Manager")
	FString Summary;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawn Point Manager")
	TObjectPtr<USceneComponent> SceneRoot = nullptr;
};
