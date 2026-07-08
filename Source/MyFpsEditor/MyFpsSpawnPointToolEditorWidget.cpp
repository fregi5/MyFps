// Copyright Epic Games, Inc. All Rights Reserved.

#include "MyFpsSpawnPointToolEditorWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Editor.h"
#include "EnemyCharacter.h"
#include "Engine/Selection.h"
#include "EngineUtils.h"
#include "Misc/DateTime.h"
#include "MyFpsEnemySpawnPoint.h"
#include "MyFpsSpawnGenerationDefinition.h"
#include "MyFpsSpawnPointManager.h"
#include "MyFpsWeaponDefinition.h"
#include "MyFpsWeaponPickupActor.h"
#include "MyFpsWeaponSpawnPoint.h"
#include "ScopedTransaction.h"

UMyFpsSpawnPointToolEditorWidget::UMyFpsSpawnPointToolEditorWidget()
{
	// 给可配置 Class 设置默认值，避免 EUW 第一次打开时因为 Class 为空导致生成失败。
	WeaponSpawnPointClass = AMyFpsWeaponSpawnPoint::StaticClass();
	WeaponPickupActorClass = AMyFpsWeaponPickupActor::StaticClass();
	EnemySpawnPointClass = AMyFpsEnemySpawnPoint::StaticClass();
	EnemyActorClass = AEnemyCharacter::StaticClass();
	SpawnPointManagerClass = AMyFpsSpawnPointManager::StaticClass();
}

void UMyFpsSpawnPointToolEditorWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 先 Remove 再 Add，防止 Editor Utility Widget 多次 Construct 后重复绑定按钮事件。
	if (ValidateGeneratedSpawnPointsButton)
	{
		ValidateGeneratedSpawnPointsButton->OnClicked.RemoveDynamic(this, &UMyFpsSpawnPointToolEditorWidget::HandleValidateGeneratedSpawnPointsClicked);
		ValidateGeneratedSpawnPointsButton->OnClicked.AddDynamic(this, &UMyFpsSpawnPointToolEditorWidget::HandleValidateGeneratedSpawnPointsClicked);
	}

	if (GenerateWeaponSpawnPointsButton)
	{
		GenerateWeaponSpawnPointsButton->OnClicked.RemoveDynamic(this, &UMyFpsSpawnPointToolEditorWidget::HandleGenerateWeaponSpawnPointsClicked);
		GenerateWeaponSpawnPointsButton->OnClicked.AddDynamic(this, &UMyFpsSpawnPointToolEditorWidget::HandleGenerateWeaponSpawnPointsClicked);
	}

	if (GenerateEnemySpawnPointsButton)
	{
		GenerateEnemySpawnPointsButton->OnClicked.RemoveDynamic(this, &UMyFpsSpawnPointToolEditorWidget::HandleGenerateEnemySpawnPointsClicked);
		GenerateEnemySpawnPointsButton->OnClicked.AddDynamic(this, &UMyFpsSpawnPointToolEditorWidget::HandleGenerateEnemySpawnPointsClicked);
	}

	if (GenerateSpawnPointsFromDefinitionButton)
	{
		GenerateSpawnPointsFromDefinitionButton->OnClicked.RemoveDynamic(this, &UMyFpsSpawnPointToolEditorWidget::HandleGenerateSpawnPointsFromDefinitionClicked);
		GenerateSpawnPointsFromDefinitionButton->OnClicked.AddDynamic(this, &UMyFpsSpawnPointToolEditorWidget::HandleGenerateSpawnPointsFromDefinitionClicked);
	}

	if (ClearGeneratedSpawnPointsButton)
	{
		ClearGeneratedSpawnPointsButton->OnClicked.RemoveDynamic(this, &UMyFpsSpawnPointToolEditorWidget::HandleClearGeneratedSpawnPointsClicked);
		ClearGeneratedSpawnPointsButton->OnClicked.AddDynamic(this, &UMyFpsSpawnPointToolEditorWidget::HandleClearGeneratedSpawnPointsClicked);
	}

	if (SelectGeneratedSpawnPointsButton)
	{
		SelectGeneratedSpawnPointsButton->OnClicked.RemoveDynamic(this, &UMyFpsSpawnPointToolEditorWidget::HandleSelectGeneratedSpawnPointsClicked);
		SelectGeneratedSpawnPointsButton->OnClicked.AddDynamic(this, &UMyFpsSpawnPointToolEditorWidget::HandleSelectGeneratedSpawnPointsClicked);
	}

	if (UpdateSpawnPointManagerButton)
	{
		UpdateSpawnPointManagerButton->OnClicked.RemoveDynamic(this, &UMyFpsSpawnPointToolEditorWidget::HandleUpdateSpawnPointManagerClicked);
		UpdateSpawnPointManagerButton->OnClicked.AddDynamic(this, &UMyFpsSpawnPointToolEditorWidget::HandleUpdateSpawnPointManagerClicked);
	}

	if (SelectSpawnPointManagerButton)
	{
		SelectSpawnPointManagerButton->OnClicked.RemoveDynamic(this, &UMyFpsSpawnPointToolEditorWidget::HandleSelectSpawnPointManagerClicked);
		SelectSpawnPointManagerButton->OnClicked.AddDynamic(this, &UMyFpsSpawnPointToolEditorWidget::HandleSelectSpawnPointManagerClicked);
	}
}

int32 UMyFpsSpawnPointToolEditorWidget::GenerateWeaponSpawnPoints()
{
	UWorld* World = GetEditorWorld();
	if (!World || !WeaponSpawnPointClass || !WeaponDefinition)
	{
		SetStatusText(FText::FromString(TEXT("Generate weapon spawn points failed: world, class, or WeaponDefinition is missing.")));
		return 0;
	}

	if (bClearBeforeGenerate)
	{
		ClearGeneratedSpawnPoints();
	}

	const FScopedTransaction Transaction(NSLOCTEXT("MyFpsSpawnPointTool", "GenerateWeaponSpawnPoints", "Generate MyFps Weapon Spawn Points"));
	World->Modify();

	// GeneratedLocations 会同时包含旧点和本次新点，用同一套距离检查避免点位堆叠。
	FRandomStream RandomStream(MakeGenerationSeed());
	TArray<FVector> GeneratedLocations;
	AppendExistingGeneratedLocations(World, GeneratedLocations);
	int32 SpawnedCount = 0;
	int32 AttemptCount = 0;
	const FVector Center = GetSpawnCenter();
	while (SpawnedCount < SpawnCount && AttemptCount < MaxPlacementAttempts)
	{
		++AttemptCount;
		FVector SpawnLocation = MakeRandomPointInCircle(RandomStream, Center);
		if (bSnapToGround)
		{
			SnapLocationToGround(World, SpawnLocation);
		}

		if (!IsFarEnoughFromExistingPoints(SpawnLocation, GeneratedLocations))
		{
			continue;
		}

		AMyFpsWeaponSpawnPoint* SpawnPoint = CreateWeaponSpawnPoint(
			World,
			SpawnLocation,
			MakeSpawnRotation(RandomStream),
			WeaponDefinition,
			WeaponPickupActorClass,
			bWeaponSpawnOnBeginPlay,
			bWeaponRespawnAfterPickup,
			WeaponRespawnDelay,
			SpawnedCount + 1);
		if (!SpawnPoint)
		{
			continue;
		}

		GeneratedLocations.Add(SpawnLocation);
		++SpawnedCount;
	}

	SetStatusText(FText::Format(
		FText::FromString(TEXT("Generated {0}/{1} weapon spawn points. Attempts={2}.")),
		FText::AsNumber(SpawnedCount),
		FText::AsNumber(SpawnCount),
		FText::AsNumber(AttemptCount)));
	if (bUpdateSpawnPointManager)
	{
		UpdateSpawnPointManager();
	}
	return SpawnedCount;
}

int32 UMyFpsSpawnPointToolEditorWidget::GenerateEnemySpawnPoints()
{
	UWorld* World = GetEditorWorld();
	if (!World || !EnemySpawnPointClass || !EnemyActorClass)
	{
		SetStatusText(FText::FromString(TEXT("Generate enemy spawn points failed: world, class, or EnemyActorClass is missing.")));
		return 0;
	}

	if (bClearBeforeGenerate)
	{
		ClearGeneratedSpawnPoints();
	}

	const FScopedTransaction Transaction(NSLOCTEXT("MyFpsSpawnPointTool", "GenerateEnemySpawnPoints", "Generate MyFps Enemy Spawn Points"));
	World->Modify();

	// 敌人点和武器点使用同一套位置生成规则，只是最终落成的 SpawnPoint 类型不同。
	FRandomStream RandomStream(MakeGenerationSeed());
	TArray<FVector> GeneratedLocations;
	AppendExistingGeneratedLocations(World, GeneratedLocations);
	int32 SpawnedCount = 0;
	int32 AttemptCount = 0;
	const FVector Center = GetSpawnCenter();
	while (SpawnedCount < SpawnCount && AttemptCount < MaxPlacementAttempts)
	{
		++AttemptCount;
		FVector SpawnLocation = MakeRandomPointInCircle(RandomStream, Center);
		if (bSnapToGround)
		{
			SnapLocationToGround(World, SpawnLocation);
		}

		if (!IsFarEnoughFromExistingPoints(SpawnLocation, GeneratedLocations))
		{
			continue;
		}

		AMyFpsEnemySpawnPoint* SpawnPoint = CreateEnemySpawnPoint(
			World,
			SpawnLocation,
			MakeSpawnRotation(RandomStream),
			EnemyActorClass,
			bEnemySpawnOnBeginPlay,
			bEnemyRespawnAfterDestroyed,
			EnemyRespawnDelay,
			SpawnedCount + 1);
		if (!SpawnPoint)
		{
			continue;
		}

		GeneratedLocations.Add(SpawnLocation);
		++SpawnedCount;
	}

	SetStatusText(FText::Format(
		FText::FromString(TEXT("Generated {0}/{1} enemy spawn points. Attempts={2}.")),
		FText::AsNumber(SpawnedCount),
		FText::AsNumber(SpawnCount),
		FText::AsNumber(AttemptCount)));
	if (bUpdateSpawnPointManager)
	{
		UpdateSpawnPointManager();
	}
	return SpawnedCount;
}

int32 UMyFpsSpawnPointToolEditorWidget::ValidateGeneratedSpawnPoints()
{
	UWorld* World = GetEditorWorld();
	if (!World || !GEditor)
	{
		SetStatusText(FText::FromString(TEXT("Validate failed: editor world is missing.")));
		return 0;
	}

	const TArray<AActor*> GeneratedActors = GetGeneratedSpawnPointActors(World);
	TArray<AActor*> InvalidActors;
	int32 MissingConfigCount = 0;
	int32 NotGroundedCount = 0;
	int32 TooCloseCount = 0;
	int32 UnknownTypeCount = 0;

	// 检查 1：刷新点配置是否完整。武器点必须有 WeaponDefinition，敌人点必须有 EnemyActorClass。
	for (AActor* Actor : GeneratedActors)
	{
		if (!Actor)
		{
			continue;
		}

		bool bHasValidType = false;
		if (AMyFpsWeaponSpawnPoint* WeaponSpawnPoint = Cast<AMyFpsWeaponSpawnPoint>(Actor))
		{
			bHasValidType = true;
			if (!WeaponSpawnPoint->GetWeaponDefinition())
			{
				InvalidActors.AddUnique(Actor);
				++MissingConfigCount;
			}
		}

		if (AMyFpsEnemySpawnPoint* EnemySpawnPoint = Cast<AMyFpsEnemySpawnPoint>(Actor))
		{
			bHasValidType = true;
			if (!EnemySpawnPoint->GetEnemyActorClass())
			{
				InvalidActors.AddUnique(Actor);
				++MissingConfigCount;
			}
		}

		if (!bHasValidType)
		{
			InvalidActors.AddUnique(Actor);
			++UnknownTypeCount;
		}
	}

	// 检查 2：刷新点是否贴近地面。这里不要求完全等于地面高度，给 50cm 容差，避免斜坡和模型表面误判。
	constexpr float GroundCheckUpDistance = 50.0f;
	constexpr float GroundCheckDownDistance = 120.0f;
	constexpr float GroundTolerance = 50.0f;
	for (AActor* Actor : GeneratedActors)
	{
		if (!Actor)
		{
			continue;
		}

		const FVector ActorLocation = Actor->GetActorLocation();
		const FVector TraceStart = ActorLocation + FVector::UpVector * GroundCheckUpDistance;
		const FVector TraceEnd = ActorLocation - FVector::UpVector * GroundCheckDownDistance;

		FHitResult HitResult;
		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(MyFpsSpawnPointToolValidateGroundTrace), false);
		QueryParams.AddIgnoredActor(Actor);
		const bool bHitGround = World->LineTraceSingleByChannel(
			HitResult,
			TraceStart,
			TraceEnd,
			ECC_Visibility,
			QueryParams);

		if (!bHitGround || FMath::Abs(HitResult.ImpactPoint.Z - ActorLocation.Z) > GroundTolerance)
		{
			InvalidActors.AddUnique(Actor);
			++NotGroundedCount;
		}
	}

	// 检查 3：刷新点之间是否小于最小间距。这里使用 2D 距离，和生成阶段的避让规则保持一致。
	const float MinDistanceSquared = FMath::Square(FMath::Max(0.0f, MinDistanceBetweenPoints));
	for (int32 IndexA = 0; IndexA < GeneratedActors.Num(); ++IndexA)
	{
		AActor* ActorA = GeneratedActors[IndexA];
		if (!ActorA)
		{
			continue;
		}

		for (int32 IndexB = IndexA + 1; IndexB < GeneratedActors.Num(); ++IndexB)
		{
			AActor* ActorB = GeneratedActors[IndexB];
			if (!ActorB)
			{
				continue;
			}

			if (FVector::DistSquared2D(ActorA->GetActorLocation(), ActorB->GetActorLocation()) < MinDistanceSquared)
			{
				InvalidActors.AddUnique(ActorA);
				InvalidActors.AddUnique(ActorB);
				++TooCloseCount;
			}
		}
	}

	// 校验结果直接反映到编辑器选择集：有问题就选中问题点，没有问题就保留一个清晰状态提示。
	GEditor->SelectNone(false, true);
	for (AActor* InvalidActor : InvalidActors)
	{
		GEditor->SelectActor(InvalidActor, true, false);
	}
	GEditor->NoteSelectionChange();

	SetStatusText(FText::Format(
		FText::FromString(TEXT("Validated {0} spawn points. Invalid={1}. MissingConfig={2}, NotGrounded={3}, TooClosePairs={4}, UnknownType={5}.")),
		FText::AsNumber(GeneratedActors.Num()),
		FText::AsNumber(InvalidActors.Num()),
		FText::AsNumber(MissingConfigCount),
		FText::AsNumber(NotGroundedCount),
		FText::AsNumber(TooCloseCount),
		FText::AsNumber(UnknownTypeCount)));
	return InvalidActors.Num();
}

int32 UMyFpsSpawnPointToolEditorWidget::GenerateSpawnPointsFromDefinition()
{
	UWorld* World = GetEditorWorld();
	if (!World || !SpawnGenerationDefinition)
	{
		SetStatusText(FText::FromString(TEXT("Generate from definition failed: editor world or SpawnGenerationDefinition is missing.")));
		return 0;
	}

	if (!WeaponSpawnPointClass || !EnemySpawnPointClass)
	{
		SetStatusText(FText::FromString(TEXT("Generate from definition failed: spawn point class is missing.")));
		return 0;
	}

	if (bClearBeforeGenerate)
	{
		ClearGeneratedSpawnPoints();
	}

	ApplyGenerationDefinitionSettings();

	const FScopedTransaction Transaction(NSLOCTEXT("MyFpsSpawnPointTool", "GenerateFromDefinition", "Generate MyFps Spawn Points From Definition"));
	World->Modify();

	// 按 DA 生成时，所有规则共用一个 GeneratedLocations 列表，保证武器点和敌人点之间也会互相避让。
	FRandomStream RandomStream(MakeGenerationSeed());
	TArray<FVector> GeneratedLocations;
	AppendExistingGeneratedLocations(World, GeneratedLocations);
	int32 SpawnedCount = 0;
	int32 AttemptCount = 0;
	const FVector Center = GetSpawnCenter();

	for (const FMyFpsWeaponSpawnGenerationRule& Rule : SpawnGenerationDefinition->WeaponRules)
	{
		if (!Rule.WeaponDefinition || Rule.Count <= 0)
		{
			continue;
		}

		int32 RuleSpawnedCount = 0;
		while (RuleSpawnedCount < Rule.Count && AttemptCount < MaxPlacementAttempts)
		{
			++AttemptCount;
			FVector SpawnLocation = MakeRandomPointInCircle(RandomStream, Center);
			if (bSnapToGround)
			{
				SnapLocationToGround(World, SpawnLocation);
			}

			if (!IsFarEnoughFromExistingPoints(SpawnLocation, GeneratedLocations))
			{
				continue;
			}

			AMyFpsWeaponSpawnPoint* SpawnPoint = CreateWeaponSpawnPoint(
				World,
				SpawnLocation,
				MakeSpawnRotation(RandomStream),
				Rule.WeaponDefinition,
				// 单条规则没有指定拾取类时，使用工具面板里的默认拾取类。
				Rule.WeaponPickupActorClass ? Rule.WeaponPickupActorClass : WeaponPickupActorClass,
				Rule.bSpawnOnBeginPlay,
				Rule.bRespawnAfterPickup,
				Rule.RespawnDelay,
				SpawnedCount + 1);
			if (!SpawnPoint)
			{
				continue;
			}

			GeneratedLocations.Add(SpawnLocation);
			++RuleSpawnedCount;
			++SpawnedCount;
		}
	}

	for (const FMyFpsEnemySpawnGenerationRule& Rule : SpawnGenerationDefinition->EnemyRules)
	{
		if (!Rule.EnemyActorClass || Rule.Count <= 0)
		{
			continue;
		}

		int32 RuleSpawnedCount = 0;
		while (RuleSpawnedCount < Rule.Count && AttemptCount < MaxPlacementAttempts)
		{
			++AttemptCount;
			FVector SpawnLocation = MakeRandomPointInCircle(RandomStream, Center);
			if (bSnapToGround)
			{
				SnapLocationToGround(World, SpawnLocation);
			}

			if (!IsFarEnoughFromExistingPoints(SpawnLocation, GeneratedLocations))
			{
				continue;
			}

			AMyFpsEnemySpawnPoint* SpawnPoint = CreateEnemySpawnPoint(
				World,
				SpawnLocation,
				MakeSpawnRotation(RandomStream),
				Rule.EnemyActorClass,
				Rule.bSpawnOnBeginPlay,
				Rule.bRespawnAfterDestroyed,
				Rule.RespawnDelay,
				SpawnedCount + 1);
			if (!SpawnPoint)
			{
				continue;
			}

			GeneratedLocations.Add(SpawnLocation);
			++RuleSpawnedCount;
			++SpawnedCount;
		}
	}

	if (bUpdateSpawnPointManager)
	{
		if (AMyFpsSpawnPointManager* SpawnPointManager = UpdateSpawnPointManager())
		{
			SpawnPointManager->SpawnGenerationDefinition = SpawnGenerationDefinition;
			SpawnPointManager->RefreshSummary();
		}
	}

	SetStatusText(FText::Format(
		FText::FromString(TEXT("Generated {0} spawn points from {1}. Attempts={2}.")),
		FText::AsNumber(SpawnedCount),
		FText::FromString(SpawnGenerationDefinition->GetName()),
		FText::AsNumber(AttemptCount)));
	return SpawnedCount;
}

int32 UMyFpsSpawnPointToolEditorWidget::ClearGeneratedSpawnPoints()
{
	UWorld* World = GetEditorWorld();
	if (!World)
	{
		SetStatusText(FText::FromString(TEXT("Clear failed: editor world is missing.")));
		return 0;
	}

	const FScopedTransaction Transaction(NSLOCTEXT("MyFpsSpawnPointTool", "ClearGeneratedSpawnPoints", "Clear MyFps Generated Spawn Points"));
	World->Modify();

	// 只清理工具生成的点，避免误删手工放置的其他 Actor。
	const TArray<AActor*> ActorsToDestroy = GetGeneratedSpawnPointActors(World);
	for (AActor* Actor : ActorsToDestroy)
	{
		if (Actor)
		{
			Actor->Modify();
			World->EditorDestroyActor(Actor, true);
		}
	}

	SetStatusText(FText::Format(
		FText::FromString(TEXT("Cleared {0} generated spawn points.")),
		FText::AsNumber(ActorsToDestroy.Num())));
	if (bUpdateSpawnPointManager)
	{
		UpdateSpawnPointManager();
	}
	return ActorsToDestroy.Num();
}

int32 UMyFpsSpawnPointToolEditorWidget::SelectGeneratedSpawnPoints()
{
	UWorld* World = GetEditorWorld();
	if (!World || !GEditor)
	{
		SetStatusText(FText::FromString(TEXT("Select failed: editor world is missing.")));
		return 0;
	}

	GEditor->SelectNone(false, true);
	const TArray<AActor*> GeneratedActors = GetGeneratedSpawnPointActors(World);
	for (AActor* Actor : GeneratedActors)
	{
		GEditor->SelectActor(Actor, true, false);
	}
	GEditor->NoteSelectionChange();

	SetStatusText(FText::Format(
		FText::FromString(TEXT("Selected {0} generated spawn points.")),
		FText::AsNumber(GeneratedActors.Num())));
	return GeneratedActors.Num();
}

AMyFpsSpawnPointManager* UMyFpsSpawnPointToolEditorWidget::UpdateSpawnPointManager()
{
	UWorld* World = GetEditorWorld();
	AMyFpsSpawnPointManager* SpawnPointManager = FindOrCreateSpawnPointManager(World);
	if (!SpawnPointManager)
	{
		SetStatusText(FText::FromString(TEXT("Update manager failed: manager class or editor world is missing.")));
		return nullptr;
	}

	SpawnPointManager->Modify();
	SpawnPointManager->ManagedActorTag = GeneratedActorTag;
	SpawnPointManager->SpawnGenerationDefinition = SpawnGenerationDefinition;
	// Manager 不保存点位副本，只扫描关卡里的 SpawnPoint Actor 并缓存引用。
	SpawnPointManager->RebuildFromWorld();

	SetStatusText(FText::Format(
		FText::FromString(TEXT("Updated spawn point manager. Weapons={0}, Enemies={1}.")),
		FText::AsNumber(SpawnPointManager->GetWeaponSpawnPointCount()),
		FText::AsNumber(SpawnPointManager->GetEnemySpawnPointCount())));
	return SpawnPointManager;
}

AMyFpsSpawnPointManager* UMyFpsSpawnPointToolEditorWidget::SelectSpawnPointManager()
{
	AMyFpsSpawnPointManager* SpawnPointManager = UpdateSpawnPointManager();
	if (!SpawnPointManager || !GEditor)
	{
		return SpawnPointManager;
	}

	GEditor->SelectNone(false, true);
	GEditor->SelectActor(SpawnPointManager, true, false);
	GEditor->NoteSelectionChange();
	return SpawnPointManager;
}

FVector UMyFpsSpawnPointToolEditorWidget::GetSpawnCenter() const
{
	if (bUseSelectedActorAsCenter && GEditor)
	{
		if (USelection* SelectedActors = GEditor->GetSelectedActors())
		{
			if (AActor* SelectedActor = Cast<AActor>(SelectedActors->GetTop<AActor>()))
			{
				return SelectedActor->GetActorLocation();
			}
		}
	}

	return ManualCenterLocation;
}

void UMyFpsSpawnPointToolEditorWidget::HandleGenerateWeaponSpawnPointsClicked()
{
	GenerateWeaponSpawnPoints();
}

void UMyFpsSpawnPointToolEditorWidget::HandleValidateGeneratedSpawnPointsClicked()
{
     ValidateGeneratedSpawnPoints();
}

void UMyFpsSpawnPointToolEditorWidget::HandleGenerateEnemySpawnPointsClicked()
{
	GenerateEnemySpawnPoints();
}

void UMyFpsSpawnPointToolEditorWidget::HandleGenerateSpawnPointsFromDefinitionClicked()
{
	GenerateSpawnPointsFromDefinition();
}

void UMyFpsSpawnPointToolEditorWidget::HandleClearGeneratedSpawnPointsClicked()
{
	ClearGeneratedSpawnPoints();
}

void UMyFpsSpawnPointToolEditorWidget::HandleSelectGeneratedSpawnPointsClicked()
{
	SelectGeneratedSpawnPoints();
}

void UMyFpsSpawnPointToolEditorWidget::HandleUpdateSpawnPointManagerClicked()
{
	UpdateSpawnPointManager();
}

void UMyFpsSpawnPointToolEditorWidget::HandleSelectSpawnPointManagerClicked()
{
	SelectSpawnPointManager();
}

UWorld* UMyFpsSpawnPointToolEditorWidget::GetEditorWorld() const
{
	// EUW 在编辑器环境运行，这里取的是编辑器世界，不是 PIE 运行时世界。
	return GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
}

int32 UMyFpsSpawnPointToolEditorWidget::MakeGenerationSeed()
{
	if (bUseFixedRandomSeed)
	{
		return RandomSeed;
	}

	const int64 TimeTicks = FDateTime::Now().GetTicks();
	return RandomSeed ^ static_cast<int32>(TimeTicks & 0x7fffffff) ^ (++GenerationSerial * 1013);
}

AMyFpsSpawnPointManager* UMyFpsSpawnPointToolEditorWidget::FindOrCreateSpawnPointManager(UWorld* World)
{
	if (!World)
	{
		return nullptr;
	}

	for (TActorIterator<AMyFpsSpawnPointManager> ActorIterator(World); ActorIterator; ++ActorIterator)
	{
		AMyFpsSpawnPointManager* SpawnPointManager = *ActorIterator;
		if (SpawnPointManager && SpawnPointManager->Tags.Contains(SpawnPointManagerTag))
		{
			return SpawnPointManager;
		}
	}

	if (!bAutoCreateSpawnPointManager || !SpawnPointManagerClass)
	{
		return nullptr;
	}

	// 关卡里没有 Manager 时自动创建一个，作为策划查看刷新点汇总的入口。
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AMyFpsSpawnPointManager* SpawnPointManager = World->SpawnActor<AMyFpsSpawnPointManager>(
		SpawnPointManagerClass,
		GetSpawnCenter(),
		FRotator::ZeroRotator,
		SpawnParameters);
	if (!SpawnPointManager)
	{
		return nullptr;
	}

	SpawnPointManager->Modify();
	SpawnPointManager->Tags.AddUnique(SpawnPointManagerTag);
	SpawnPointManager->SetFolderPath(TEXT("MyFpsGenerated"));
#if WITH_EDITOR
	SpawnPointManager->SetActorLabel(TEXT("SP_Manager_MyFps"));
#endif
	return SpawnPointManager;
}

void UMyFpsSpawnPointToolEditorWidget::ApplyGenerationDefinitionSettings()
{
	if (!SpawnGenerationDefinition)
	{
		return;
	}

	// DA 是规则源；点击“按 DA 生成”时，用 DA 覆盖工具面板里的通用生成参数。
	SpawnRadius = SpawnGenerationDefinition->SpawnRadius;
	MinDistanceBetweenPoints = SpawnGenerationDefinition->MinDistanceBetweenPoints;
	MaxPlacementAttempts = SpawnGenerationDefinition->MaxPlacementAttempts;
	RandomSeed = SpawnGenerationDefinition->RandomSeed;
	bUseFixedRandomSeed = SpawnGenerationDefinition->bUseFixedRandomSeed;
	bAvoidExistingGeneratedSpawnPoints = SpawnGenerationDefinition->bAvoidExistingGeneratedSpawnPoints;
	bSnapToGround = SpawnGenerationDefinition->bSnapToGround;
	GroundTraceUpDistance = SpawnGenerationDefinition->GroundTraceUpDistance;
	GroundTraceDownDistance = SpawnGenerationDefinition->GroundTraceDownDistance;
	bRandomYaw = SpawnGenerationDefinition->bRandomYaw;
}

AMyFpsWeaponSpawnPoint* UMyFpsSpawnPointToolEditorWidget::CreateWeaponSpawnPoint(
	UWorld* World,
	const FVector& SpawnLocation,
	const FRotator& SpawnRotation,
	UMyFpsWeaponDefinition* NewWeaponDefinition,
	TSubclassOf<AMyFpsWeaponPickupActor> NewWeaponPickupActorClass,
	bool bNewSpawnOnBeginPlay,
	bool bNewRespawnAfterPickup,
	float NewRespawnDelay,
	int32 SpawnIndex)
{
	if (!World || !WeaponSpawnPointClass || !NewWeaponDefinition)
	{
		return nullptr;
	}

	// Editor World 中生成的是刷新点 Actor，不是运行时实际武器；真正武器由刷新点在游戏开始后生成。
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AMyFpsWeaponSpawnPoint* SpawnPoint = World->SpawnActor<AMyFpsWeaponSpawnPoint>(
		WeaponSpawnPointClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParameters);
	if (!SpawnPoint)
	{
		return nullptr;
	}

	SpawnPoint->Modify();
	SpawnPoint->Tags.AddUnique(GeneratedActorTag);
	SpawnPoint->Tags.AddUnique(TEXT("MyFpsGeneratedWeaponSpawnPoint"));
	// 把策划规则写入 SpawnPoint，后续保存到关卡里。
	SpawnPoint->ConfigureWeaponSpawnPoint(NewWeaponDefinition, NewWeaponPickupActorClass);
	SpawnPoint->ConfigureWeaponSpawnSettings(
		true,
		bNewSpawnOnBeginPlay,
		bNewRespawnAfterPickup,
		NewRespawnDelay);
#if WITH_EDITOR
	SpawnPoint->SetActorLabel(FString::Printf(
		TEXT("SP_Weapon_%s_%03d"),
		*NewWeaponDefinition->GetName(),
		SpawnIndex));
#endif
	SpawnPoint->SetFolderPath(TEXT("MyFpsGenerated/WeaponSpawnPoints"));
	return SpawnPoint;
}

AMyFpsEnemySpawnPoint* UMyFpsSpawnPointToolEditorWidget::CreateEnemySpawnPoint(
	UWorld* World,
	const FVector& SpawnLocation,
	const FRotator& SpawnRotation,
	TSubclassOf<AEnemyCharacter> NewEnemyActorClass,
	bool bNewSpawnOnBeginPlay,
	bool bNewRespawnAfterDestroyed,
	float NewRespawnDelay,
	int32 SpawnIndex)
{
	if (!World || !EnemySpawnPointClass || !NewEnemyActorClass)
	{
		return nullptr;
	}

	// Editor World 中生成的是敌人刷新点；真正敌人由刷新点在运行时服务器侧生成。
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AMyFpsEnemySpawnPoint* SpawnPoint = World->SpawnActor<AMyFpsEnemySpawnPoint>(
		EnemySpawnPointClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParameters);
	if (!SpawnPoint)
	{
		return nullptr;
	}

	SpawnPoint->Modify();
	SpawnPoint->Tags.AddUnique(GeneratedActorTag);
	SpawnPoint->Tags.AddUnique(TEXT("MyFpsGeneratedEnemySpawnPoint"));
	SpawnPoint->ConfigureEnemySpawnPoint(NewEnemyActorClass);
	SpawnPoint->ConfigureEnemySpawnSettings(
		true,
		bNewSpawnOnBeginPlay,
		bNewRespawnAfterDestroyed,
		NewRespawnDelay);
#if WITH_EDITOR
	SpawnPoint->SetActorLabel(FString::Printf(
		TEXT("SP_Enemy_%s_%03d"),
		*NewEnemyActorClass->GetName(),
		SpawnIndex));
#endif
	SpawnPoint->SetFolderPath(TEXT("MyFpsGenerated/EnemySpawnPoints"));
	return SpawnPoint;
}

void UMyFpsSpawnPointToolEditorWidget::AppendExistingGeneratedLocations(UWorld* World, TArray<FVector>& InOutLocations) const
{
	if (!bAvoidExistingGeneratedSpawnPoints)
	{
		return;
	}

	for (AActor* Actor : GetGeneratedSpawnPointActors(World))
	{
		if (Actor)
		{
			InOutLocations.Add(Actor->GetActorLocation());
		}
	}
}

FVector UMyFpsSpawnPointToolEditorWidget::MakeRandomPointInCircle(FRandomStream& RandomStream, const FVector& Center) const
{
	// 使用 sqrt 修正半径分布，让点在圆面积内更均匀，而不是集中在圆心附近。
	const float Angle = RandomStream.FRandRange(0.0f, 2.0f * PI);
	const float Distance = FMath::Sqrt(RandomStream.FRand()) * FMath::Max(0.0f, SpawnRadius);
	return Center + FVector(FMath::Cos(Angle) * Distance, FMath::Sin(Angle) * Distance, 0.0f);
}

bool UMyFpsSpawnPointToolEditorWidget::SnapLocationToGround(UWorld* World, FVector& InOutLocation) const
{
	if (!World)
	{
		return false;
	}

	const FVector TraceStart = InOutLocation + FVector::UpVector * FMath::Max(0.0f, GroundTraceUpDistance);
	const FVector TraceEnd = InOutLocation - FVector::UpVector * FMath::Max(0.0f, GroundTraceDownDistance);

	// 从候选点上方向下 Trace，把刷新点吸附到命中的地面位置。
	FHitResult HitResult;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(MyFpsSpawnPointToolGroundTrace), false);
	if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams)
		&& HitResult.bBlockingHit)
	{
		InOutLocation = HitResult.ImpactPoint;
		return true;
	}

	return false;
}

FRotator UMyFpsSpawnPointToolEditorWidget::MakeSpawnRotation(FRandomStream& RandomStream) const
{
	return bRandomYaw
		? FRotator(0.0f, RandomStream.FRandRange(-180.0f, 180.0f), 0.0f)
		: FRotator::ZeroRotator;
}

bool UMyFpsSpawnPointToolEditorWidget::IsFarEnoughFromExistingPoints(const FVector& CandidateLocation, const TArray<FVector>& ExistingLocations) const
{
	// 使用平方距离避免每次比较都开方；这里只关心 XY 平面的点位间距。
	const float MinDistanceSquared = FMath::Square(FMath::Max(0.0f, MinDistanceBetweenPoints));
	for (const FVector& ExistingLocation : ExistingLocations)
	{
		if (FVector::DistSquared2D(CandidateLocation, ExistingLocation) < MinDistanceSquared)
		{
			return false;
		}
	}

	return true;
}

TArray<AActor*> UMyFpsSpawnPointToolEditorWidget::GetGeneratedSpawnPointActors(UWorld* World) const
{
	TArray<AActor*> GeneratedActors;
	if (!World)
	{
		return GeneratedActors;
	}

	for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
	{
		AActor* Actor = *ActorIterator;
		if (Actor && Actor->Tags.Contains(GeneratedActorTag))
		{
			GeneratedActors.Add(Actor);
		}
	}

	return GeneratedActors;
}

void UMyFpsSpawnPointToolEditorWidget::SetStatusText(const FText& NewStatusText)
{
	if (StatusTextBlock)
	{
		StatusTextBlock->SetText(NewStatusText);
	}
}
