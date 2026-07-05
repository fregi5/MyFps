// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyFpsEnemySpawnPoint.generated.h"

class AEnemyCharacter;
class UArrowComponent;
class UBillboardComponent;
class USphereComponent;

UCLASS()
class MYFPS_API AMyFpsEnemySpawnPoint : public AActor
{
	GENERATED_BODY()

public:
	AMyFpsEnemySpawnPoint();

	virtual void OnConstruction(const FTransform& Transform) override;

	UFUNCTION(BlueprintCallable, Category = "Spawn Point|Enemy")
	void ConfigureEnemySpawnPoint(TSubclassOf<AEnemyCharacter> NewEnemyActorClass);

	UFUNCTION(BlueprintCallable, Category = "Spawn Point|Enemy")
	void ConfigureEnemySpawnSettings(
		bool bNewEnabled,
		bool bNewSpawnOnBeginPlay,
		bool bNewRespawnAfterDestroyed,
		float NewRespawnDelay);

	UFUNCTION(BlueprintCallable, Category = "Spawn Point|Enemy")
	AEnemyCharacter* SpawnEnemy();

	UFUNCTION(BlueprintCallable, Category = "Spawn Point|Enemy")
	void ClearSpawnedEnemy();

	UFUNCTION(BlueprintPure, Category = "Spawn Point|Enemy")
	AEnemyCharacter* GetSpawnedEnemy() const { return SpawnedEnemy; }

	
	UFUNCTION(BlueprintPure, Category = "Spawn Point|Enemy")
	TSubclassOf<AEnemyCharacter> GetEnemyActorClass() const { return EnemyActorClass; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawn Point")
	TObjectPtr<USceneComponent> SceneRoot = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawn Point")
	TObjectPtr<UBillboardComponent> BillboardComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawn Point")
	TObjectPtr<USphereComponent> PreviewRadiusComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawn Point")
	TObjectPtr<UArrowComponent> ArrowComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn Point|Enemy")
	TSubclassOf<AEnemyCharacter> EnemyActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn Point|Enemy")
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn Point|Enemy")
	bool bSpawnOnBeginPlay = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn Point|Enemy")
	bool bRespawnAfterDestroyed = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn Point|Enemy", meta = (ClampMin = "0.0", ForceUnits = "s"))
	float RespawnDelay = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn Point|Preview", meta = (ClampMin = "0.0", ForceUnits = "cm"))
	float PreviewRadius = 180.0f;

private:
	UFUNCTION()
	void HandleSpawnedEnemyDestroyed(AActor* DestroyedActor);

	void ScheduleRespawn();
	void SpawnEnemyFromTimer();
	void ApplyPreviewSettings();

	UPROPERTY(Transient)
	TObjectPtr<AEnemyCharacter> SpawnedEnemy = nullptr;

	FTimerHandle RespawnTimerHandle;
};
