#include "EnemyCharacter.h"
#include "MyFpsCharacter.h"
#include "MyFpsGameMode.h"
#include "MyFpsGameState.h"
#include "MyFpsProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "AIController.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"

AEnemyCharacter::AEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	MaxHealth = 50.0f;
	Health = MaxHealth;
	bIsDead = false;
	bUseMeleeAttack = AttackMode == EEnemyAttackMode::MeleeOnly;
}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	InitialSpawnTransform = GetActorTransform();
	Health = MaxHealth;

	UE_LOG(LogTemp, Warning, TEXT("Enemy BeginPlay, Health = %.1f"), Health);

	ApplyLifeState();

	if (HasAuthority())
	{
		const AMyFpsGameState* MyGameState = GetWorld() ? GetWorld()->GetGameState<AMyFpsGameState>() : nullptr;
		if (MyGameState && MyGameState->IsMyFpsMatchStarted())
		{
			StartChasing();
		}
		else
		{
			SetWaitingForMatchStart(true);
		}
	}
}

void AEnemyCharacter::StartChasing()
{
	if (!HasAuthority())
	{
		return;
	}

	GetWorldTimerManager().SetTimer(
		ChaseTimerHandle,
		this,
		&AEnemyCharacter::ChasePlayer,
		ChaseUpdateInterval,
		true
	);
}

float AEnemyCharacter::TakeDamage(
	float DamageAmount,
	FDamageEvent const& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser
)
{
	const AMyFpsGameState* MyGameState = GetWorld() ? GetWorld()->GetGameState<AMyFpsGameState>() : nullptr;
	if (!HasAuthority() || !MyGameState || !MyGameState->IsMyFpsMatchStarted() || MyGameState->IsMatchFinished())
	{
		return 0.0f;
	}

	const float ActualDamage = Super::TakeDamage(
		DamageAmount,
		DamageEvent,
		EventInstigator,
		DamageCauser
	);

	if (ActualDamage <= 0.0f || bIsDead)
	{
		return 0.0f;
	}

	Health = FMath::Clamp(Health - ActualDamage, 0.0f, MaxHealth);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("Enemy took %.1f damage, Health = %.1f"),
		ActualDamage,
		Health
	);

	if (Health <= 0.0f)
	{
		Die(EventInstigator);
	}

	return ActualDamage;
}

void AEnemyCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AEnemyCharacter, Health);
	DOREPLIFETIME(AEnemyCharacter, bIsDead);
}

void AEnemyCharacter::OnRep_HealthState()
{
	ApplyLifeState();
}

void AEnemyCharacter::ApplyLifeState()
{
	if (bIsDead)
	{
		SetActorEnableCollision(false);
		SetActorHiddenInGame(true);

		if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
		{
			MovementComponent->DisableMovement();
		}

		return;
	}

	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->SetMovementMode(MOVE_Walking);
	}
}

void AEnemyCharacter::Die(AController* KillerController)
{
	if (!HasAuthority())
	{
		return;
	}

	if (bIsDead)
	{
		return;
	}

	bIsDead = true;
	Health = 0.0f;
	GetWorldTimerManager().ClearTimer(ChaseTimerHandle);
	GetWorldTimerManager().ClearTimer(AttackTimerHandle);

	if (KillerController && Cast<APlayerController>(KillerController))
	{
		if (AMyFpsGameMode* GameMode = GetWorld()->GetAuthGameMode<AMyFpsGameMode>())
		{
			GameMode->RegisterKill(KillerController);
			GameMode->AddScore(KillerController, KillScore);
		}
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("Enemy died. KillerController = %s"),
		KillerController ? *KillerController->GetName() : TEXT("None")
	);

	StopAttacking();
	CurrentTarget = nullptr;
	ApplyLifeState();

	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		AIController->StopMovement();
	}

	if (!bRespawnOnDeath)
	{
		Destroy();
		return;
	}

	GetWorldTimerManager().SetTimer(
		RespawnTimerHandle,
		this,
		&AEnemyCharacter::RespawnEnemy,
		RespawnDelay,
		false
	);
}

void AEnemyCharacter::RespawnEnemy()
{
	if (!HasAuthority())
	{
		return;
	}

	bIsDead = false;
	Health = MaxHealth;
	CurrentTarget = nullptr;
	bIsAttacking = false;
	bUseMeleeAttack = AttackMode == EEnemyAttackMode::MeleeOnly;

	FTransform RespawnTransform = InitialSpawnTransform;
	RespawnTransform.SetLocation(GetNextRespawnLocation());
	SetActorTransform(RespawnTransform);
	ApplyLifeState();

	if (!GetController())
	{
		SpawnDefaultController();
	}

	StartChasing();
}

FVector AEnemyCharacter::GetNextRespawnLocation() const
{
	FVector RespawnLocation = InitialSpawnTransform.GetLocation();
	if (TryFindRespawnLocationNearPlayer(RespawnLocation))
	{
		return RespawnLocation;
	}

	if (RespawnLocationRadius <= 0.0f)
	{
		return InitialSpawnTransform.GetLocation();
	}

	const FVector InitialLocation = InitialSpawnTransform.GetLocation();
	const float CapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	for (int32 Attempt = 0; Attempt < RespawnSearchAttempts; ++Attempt)
	{
		const FVector2D RandomOffset = FMath::RandPointInCircle(RespawnLocationRadius);
		const FVector CandidateLocation = InitialLocation + FVector(RandomOffset.X, RandomOffset.Y, CapsuleHalfHeight);
		if (IsRespawnLocationClear(CandidateLocation))
		{
			return CandidateLocation;
		}
	}

	return InitialLocation + FVector(0.0f, 0.0f, CapsuleHalfHeight);
}

bool AEnemyCharacter::TryFindRespawnLocationNearPlayer(FVector& OutRespawnLocation) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	APawn* PlayerPawn = GetRespawnReferencePlayer();
	if (!PlayerPawn)
	{
		return false;
	}

	UNavigationSystemV1* NavigationSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
	if (!NavigationSystem)
	{
		return false;
	}

	const FVector PlayerLocation = PlayerPawn->GetActorLocation();
	const float SearchRadius = FMath::Max(MaxRespawnDistanceFromPlayer, MinRespawnDistanceFromPlayer + 100.0f);

	for (int32 Attempt = 0; Attempt < RespawnSearchAttempts; ++Attempt)
	{
		const FVector RandomDirection = FMath::VRand();
		const float RandomDistance = FMath::FRandRange(MinRespawnDistanceFromPlayer, SearchRadius);
		const FVector CandidateLocation = PlayerLocation + FVector(RandomDirection.X, RandomDirection.Y, 0.0f) * RandomDistance;

		FNavLocation NavLocation;
		if (!NavigationSystem->ProjectPointToNavigation(CandidateLocation, NavLocation, FVector(200.0f, 200.0f, 400.0f)))
		{
			continue;
		}

		const float DistanceToPlayer = FVector::Dist2D(NavLocation.Location, PlayerLocation);
		if (DistanceToPlayer < MinRespawnDistanceFromPlayer || DistanceToPlayer > MaxRespawnDistanceFromPlayer)
		{
			continue;
		}

		const FVector AdjustedRespawnLocation = NavLocation.Location + FVector(0.0f, 0.0f, GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
		if (!IsRespawnLocationClear(AdjustedRespawnLocation))
		{
			continue;
		}

		OutRespawnLocation = AdjustedRespawnLocation;
		return true;
	}

	return false;
}

APawn* AEnemyCharacter::GetBestTargetPlayer() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	APawn* BestTarget = nullptr;
	float BestDistanceSq = TNumericLimits<float>::Max();

	for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		const APlayerController* PlayerController = Iterator->Get();
		if (!PlayerController)
		{
			continue;
		}

		APawn* CandidatePawn = PlayerController->GetPawn();
		const AMyFpsCharacter* CandidateCharacter = Cast<AMyFpsCharacter>(CandidatePawn);
		if (!CandidatePawn || (CandidateCharacter && CandidateCharacter->IsDead()))
		{
			continue;
		}

		const float DistanceSq = FVector::DistSquared(GetActorLocation(), CandidatePawn->GetActorLocation());
		if (DistanceSq < BestDistanceSq)
		{
			BestDistanceSq = DistanceSq;
			BestTarget = CandidatePawn;
		}
	}

	return BestTarget;
}

APawn* AEnemyCharacter::GetRespawnReferencePlayer() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	TArray<APawn*> AlivePlayers;
	for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		const APlayerController* PlayerController = Iterator->Get();
		if (!PlayerController)
		{
			continue;
		}

		APawn* CandidatePawn = PlayerController->GetPawn();
		const AMyFpsCharacter* CandidateCharacter = Cast<AMyFpsCharacter>(CandidatePawn);
		if (!CandidatePawn || (CandidateCharacter && CandidateCharacter->IsDead()))
		{
			continue;
		}

		AlivePlayers.Add(CandidatePawn);
	}

	if (AlivePlayers.Num() <= 0)
	{
		return nullptr;
	}

	return AlivePlayers[FMath::RandRange(0, AlivePlayers.Num() - 1)];
}

bool AEnemyCharacter::IsRespawnLocationClear(const FVector& CandidateLocation) const
{
	const UWorld* World = GetWorld();
	const UCapsuleComponent* EnemyCapsuleComponent = GetCapsuleComponent();
	if (!World || !EnemyCapsuleComponent)
	{
		return false;
	}

	const float CapsuleRadius = EnemyCapsuleComponent->GetScaledCapsuleRadius();
	const float CapsuleHalfHeight = EnemyCapsuleComponent->GetScaledCapsuleHalfHeight();
	const FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(EnemyRespawnOverlap), false, this);
	QueryParams.AddIgnoredActor(this);

	return !World->OverlapBlockingTestByChannel(
		CandidateLocation,
		FQuat::Identity,
		ECC_Pawn,
		CapsuleShape,
		QueryParams
	);
}

void AEnemyCharacter::ChasePlayer()
{
	if (!HasAuthority() || bIsDead)
	{
		return;
	}

	APawn* PlayerPawn = GetBestTargetPlayer();
	if (!PlayerPawn)
	{
		StopAttacking();
		return;
	}

	AAIController* AIController = Cast<AAIController>(GetController());
	if (!AIController)
	{
		return;
	}

	const float DistanceToPlayer = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
	if (CanAttackTarget(DistanceToPlayer))
	{
		AIController->StopMovement();
		StartAttacking(PlayerPawn);
		return;
	}

	StopAttacking();
	AIController->MoveToActor(PlayerPawn, AcceptanceRadius);
}

void AEnemyCharacter::StartAttacking(APawn* PlayerPawn)
{
	if (!HasAuthority())
	{
		return;
	}

	CurrentTarget = PlayerPawn;

	if (!CurrentTarget)
	{
		return;
	}

	bUseMeleeAttack = AttackMode == EEnemyAttackMode::MeleeOnly;

	if (bIsAttacking)
	{
		return;
	}

	bIsAttacking = true;

	PerformAttack();

	GetWorldTimerManager().SetTimer(
		AttackTimerHandle,
		this,
		&AEnemyCharacter::PerformAttack,
		GetCurrentAttackInterval(),
		true
	);
}

void AEnemyCharacter::StopAttacking()
{
	if (!HasAuthority())
	{
		return;
	}

	if (!bIsAttacking)
	{
		return;
	}

	bIsAttacking = false;
	CurrentTarget = nullptr;
	GetWorldTimerManager().ClearTimer(AttackTimerHandle);
}

void AEnemyCharacter::PerformAttack()
{
	if (!HasAuthority() || bIsDead || !CurrentTarget)
	{
		StopAttacking();
		return;
	}

	const float DistanceToTarget = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
	if (!CanAttackTarget(DistanceToTarget))
	{
		StopAttacking();
		return;
	}

	if (bUseMeleeAttack)
	{
		PerformMeleeAttack(CurrentTarget);
		return;
	}

	PerformRangedAttack(CurrentTarget);
}

bool AEnemyCharacter::CanAttackTarget(float DistanceToTarget) const
{
	if (AttackMode == EEnemyAttackMode::MeleeOnly)
	{
		return DistanceToTarget <= MeleeAttackRange;
	}

	return ProjectileClass != nullptr && DistanceToTarget <= RangedAttackRange;
}

float AEnemyCharacter::GetCurrentAttackInterval() const
{
	return bUseMeleeAttack ? MeleeAttackInterval : RangedAttackInterval;
}

void AEnemyCharacter::PerformMeleeAttack(APawn* TargetPawn)
{
	if (!HasAuthority() || !TargetPawn)
	{
		return;
	}

	const FVector TargetLocation = TargetPawn->GetActorLocation();
	const FRotator FacingRotation = (TargetLocation - GetActorLocation()).Rotation();
	SetActorRotation(FRotator(0.0f, FacingRotation.Yaw, 0.0f));

	UGameplayStatics::ApplyDamage(
		TargetPawn,
		MeleeDamage,
		GetController(),
		this,
		UDamageType::StaticClass()
	);
}

void AEnemyCharacter::PerformRangedAttack(APawn* TargetPawn)
{
	FireAtTarget(TargetPawn);
}

void AEnemyCharacter::FireAtTarget(APawn* TargetPawn)
{
	if (!HasAuthority() || bIsDead || !ProjectileClass || !TargetPawn)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FVector SpawnLocation = GetActorLocation() + GetActorRotation().RotateVector(ProjectileSpawnOffset);
	const FVector AimLocation = TargetPawn->GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);
	const FRotator SpawnRotation = (AimLocation - SpawnLocation).Rotation();
	SetActorRotation(FRotator(0.0f, SpawnRotation.Yaw, 0.0f));

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

	AMyFpsProjectile* Projectile = World->SpawnActor<AMyFpsProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
	if (!Projectile)
	{
		return;
	}

	Projectile->SetDamage(ProjectileDamage);

	if (UProjectileMovementComponent* ProjectileMovement = Projectile->GetProjectileMovement())
	{
		ProjectileMovement->Velocity = SpawnRotation.Vector() * ProjectileMovement->InitialSpeed;
	}
}

void AEnemyCharacter::HandleMatchFinished()
{
	if (!HasAuthority())
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(ChaseTimerHandle);
	GetWorldTimerManager().ClearTimer(AttackTimerHandle);
	StopAttacking();

	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		AIController->StopMovement();
	}

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
		MovementComponent->DisableMovement();
	}
}

void AEnemyCharacter::ForceRespawnNow()
{
	if (!HasAuthority())
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(RespawnTimerHandle);
	RespawnEnemy();
}

void AEnemyCharacter::SetWaitingForMatchStart(bool bWaiting)
{
	if (!HasAuthority())
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(ChaseTimerHandle);
	GetWorldTimerManager().ClearTimer(AttackTimerHandle);
	StopAttacking();
	bIsDead = bWaiting;
	Health = bWaiting ? 0.0f : MaxHealth;
	CurrentTarget = nullptr;
	ApplyLifeState();

	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		AIController->StopMovement();
	}
}
