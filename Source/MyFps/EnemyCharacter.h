#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "EnemyCharacter.generated.h"

class AMyFpsProjectile;

UENUM(BlueprintType)
enum class EEnemyAttackMode : uint8
{
	MeleeOnly UMETA(DisplayName = "Melee Only"),
	RangedOnly UMETA(DisplayName = "Ranged Only")
};

UCLASS()
class MYFPS_API AEnemyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AEnemyCharacter();

	bool IsDead() const { return bIsDead; }

	virtual float TakeDamage(
		float DamageAmount,
		struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator,
		AActor* DamageCauser
	) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health")
	float MaxHealth = 50.0f;

	UPROPERTY(ReplicatedUsing = OnRep_HealthState, BlueprintReadOnly, Category = "Health")
	float Health = 50.0f;

	UPROPERTY(ReplicatedUsing = OnRep_HealthState, BlueprintReadOnly, Category = "Health")
	bool bIsDead = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	float AcceptanceRadius = 150.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	float KillScore = 100.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	float ChaseUpdateInterval = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	bool bRespawnOnDeath = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI", meta = (EditCondition = "bRespawnOnDeath"))
	float RespawnDelay = 5.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI", meta = (EditCondition = "bRespawnOnDeath"))
	float RespawnLocationRadius = 300.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI", meta = (EditCondition = "bRespawnOnDeath"))
	float MinRespawnDistanceFromPlayer = 800.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI", meta = (EditCondition = "bRespawnOnDeath"))
	float MaxRespawnDistanceFromPlayer = 1800.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI", meta = (EditCondition = "bRespawnOnDeath"))
	int32 RespawnSearchAttempts = 12;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	EEnemyAttackMode AttackMode = EEnemyAttackMode::MeleeOnly;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float MeleeAttackRange = 150.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float MeleeAttackInterval = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float MeleeDamage = 20.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float RangedAttackRange = 800.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float RangedAttackInterval = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float ProjectileDamage = 15.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	FVector ProjectileSpawnOffset = FVector(80.0f, 0.0f, 50.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TSubclassOf<AMyFpsProjectile> ProjectileClass;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bIsAttacking = false;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bUseMeleeAttack = true;

	UPROPERTY()
	TObjectPtr<APawn> CurrentTarget = nullptr;

	FTimerHandle ChaseTimerHandle;
	FTimerHandle AttackTimerHandle;
	FTimerHandle RespawnTimerHandle;
	FTransform InitialSpawnTransform;

	UFUNCTION()
	void OnRep_HealthState();

	void Die(AController* KillerController);
	void RespawnEnemy();
	void StartChasing();
	void ApplyLifeState();
	FVector GetNextRespawnLocation() const;
	bool TryFindRespawnLocationNearPlayer(FVector& OutRespawnLocation) const;
	bool IsRespawnLocationClear(const FVector& CandidateLocation) const;
	APawn* GetBestTargetPlayer() const;
	APawn* GetRespawnReferencePlayer() const;

	void ChasePlayer();
	void StartAttacking(APawn* PlayerPawn);
	void StopAttacking();
	void PerformAttack();
	bool CanAttackTarget(float DistanceToTarget) const;
	float GetCurrentAttackInterval() const;
	void PerformMeleeAttack(APawn* TargetPawn);
	void PerformRangedAttack(APawn* TargetPawn);
	void FireAtTarget(APawn* TargetPawn);

public:
	void HandleMatchFinished();
	void ForceRespawnNow();
	void SetWaitingForMatchStart(bool bWaiting);
};
