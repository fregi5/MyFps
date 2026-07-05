// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "MyFpsSpawnPointToolEditorWidget.generated.h"

class AEnemyCharacter;
class AMyFpsEnemySpawnPoint;
class AMyFpsSpawnPointManager;
class AMyFpsWeaponPickupActor;
class AMyFpsWeaponSpawnPoint;
class UButton;
class UMyFpsSpawnGenerationDefinition;
class UMyFpsWeaponDefinition;
class UTextBlock;

UCLASS(BlueprintType, Blueprintable)
class MYFPSEDITOR_API UMyFpsSpawnPointToolEditorWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()

public:
	UMyFpsSpawnPointToolEditorWidget();

	virtual void NativeConstruct() override;

	// 勾选后使用编辑器当前选中的 Actor 作为生成中心；否则使用 ManualCenterLocation。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Point Tool|Common")
	bool bUseSelectedActorAsCenter = true;

	// 没有使用选中 Actor 时的手动生成中心。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Point Tool|Common")
	FVector ManualCenterLocation = FVector::ZeroVector;

	// 在中心点周围这个半径内随机生成刷新点。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Point Tool|Common", meta = (ClampMin = "0.0", ForceUnits = "cm"))
	float SpawnRadius = 2000.0f;

	// 单次生成按钮要生成的点数；按 DA 生成时使用 DA 每条规则的 Count。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Point Tool|Common", meta = (ClampMin = "1"))
	int32 SpawnCount = 10;

	// 点与点之间的最小 2D 间距。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Point Tool|Common", meta = (ClampMin = "0.0", ForceUnits = "cm"))
	float MinDistanceBetweenPoints = 300.0f;

	// 最大尝试次数，避免规则放不下点时卡死编辑器。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Point Tool|Common", meta = (ClampMin = "1"))
	int32 MaxPlacementAttempts = 500;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Point Tool|Common")
	int32 RandomSeed = 1337;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Point Tool|Common")
	bool bUseFixedRandomSeed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Point Tool|Common")
	bool bAvoidExistingGeneratedSpawnPoints = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Point Tool|Common")
	bool bSnapToGround = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Point Tool|Common", meta = (ClampMin = "0.0", ForceUnits = "cm"))
	float GroundTraceUpDistance = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Point Tool|Common", meta = (ClampMin = "0.0", ForceUnits = "cm"))
	float GroundTraceDownDistance = 5000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Point Tool|Common")
	bool bRandomYaw = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Point Tool|Common")
	bool bClearBeforeGenerate = false;

	// 工具产物的公共 Tag。清理、选择、避让、Manager 汇总都依赖这个 Tag。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Point Tool|Common")
	FName GeneratedActorTag = TEXT("MyFpsGeneratedSpawnPoint");

	// 生成或清理后是否自动刷新关卡中的 SpawnPointManager。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Point Tool|Management")
	bool bUpdateSpawnPointManager = true;

	// 找不到 Manager 时是否自动在关卡中创建一个。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Point Tool|Management")
	bool bAutoCreateSpawnPointManager = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Point Tool|Management")
	TSubclassOf<AMyFpsSpawnPointManager> SpawnPointManagerClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Point Tool|Management")
	FName SpawnPointManagerTag = TEXT("MyFpsSpawnPointManager");

	// 成熟流程入口：策划配置 DA，EUW 根据 DA 一次性生成多类武器/敌人刷新点。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Point Tool|Generation Definition")
	TObjectPtr<UMyFpsSpawnGenerationDefinition> SpawnGenerationDefinition = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Point Tool|Weapon")
	TSubclassOf<AMyFpsWeaponSpawnPoint> WeaponSpawnPointClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Point Tool|Weapon")
	TSubclassOf<AMyFpsWeaponPickupActor> WeaponPickupActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Point Tool|Weapon")
	TObjectPtr<UMyFpsWeaponDefinition> WeaponDefinition = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Point Tool|Weapon")
	bool bWeaponSpawnOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Point Tool|Weapon")
	bool bWeaponRespawnAfterPickup = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Point Tool|Weapon", meta = (ClampMin = "0.0", ForceUnits = "s"))
	float WeaponRespawnDelay = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Point Tool|Enemy")
	TSubclassOf<AMyFpsEnemySpawnPoint> EnemySpawnPointClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Point Tool|Enemy")
	TSubclassOf<AEnemyCharacter> EnemyActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Point Tool|Enemy")
	bool bEnemySpawnOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Point Tool|Enemy")
	bool bEnemyRespawnAfterDestroyed = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Point Tool|Enemy", meta = (ClampMin = "0.0", ForceUnits = "s"))
	float EnemyRespawnDelay = 10.0f;

	// 按当前 Widget 面板上的单武器配置生成刷新点。
	UFUNCTION(BlueprintCallable, Category = "Spawn Point Tool")
	int32 GenerateWeaponSpawnPoints();

	// 按当前 Widget 面板上的单敌人配置生成刷新点。
	UFUNCTION(BlueprintCallable, Category = "Spawn Point Tool")
	int32 GenerateEnemySpawnPoints();

	// 按 SpawnGenerationDefinition 中的规则数组批量生成多种刷新点。
	UFUNCTION(BlueprintCallable, Category = "Spawn Point Tool")
	int32 GenerateSpawnPointsFromDefinition();

	// 删除所有带 GeneratedActorTag 的刷新点。
	UFUNCTION(BlueprintCallable, Category = "Spawn Point Tool")
	int32 ClearGeneratedSpawnPoints();

	// 在编辑器中选中所有带 GeneratedActorTag 的刷新点，方便批量查看和修改。
	UFUNCTION(BlueprintCallable, Category = "Spawn Point Tool")
	int32 SelectGeneratedSpawnPoints();

	// 创建或刷新关卡中的 SpawnPointManager。
	UFUNCTION(BlueprintCallable, Category = "Spawn Point Tool")
	AMyFpsSpawnPointManager* UpdateSpawnPointManager();

	// 刷新 Manager 后将其选中，方便策划直接看汇总信息。
	UFUNCTION(BlueprintCallable, Category = "Spawn Point Tool")
	AMyFpsSpawnPointManager* SelectSpawnPointManager();

	// 获取当前生成中心：优先使用编辑器选中的 Actor，否则使用 ManualCenterLocation。
	UFUNCTION(BlueprintCallable, Category = "Spawn Point Tool")
	FVector GetSpawnCenter() const;

	// TODO: 后续实现点位合法性校验，例如距离过近、未贴地、缺少配置等。
	UFUNCTION(BlueprintCallable, Category = "Spawn Point Tool")
	int32 ValidateGeneratedSpawnPoints();

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> GenerateWeaponSpawnPointsButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> GenerateEnemySpawnPointsButton = nullptr;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> ValidateGeneratedSpawnPointsButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> GenerateSpawnPointsFromDefinitionButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> ClearGeneratedSpawnPointsButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> SelectGeneratedSpawnPointsButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> UpdateSpawnPointManagerButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> SelectSpawnPointManagerButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> StatusTextBlock = nullptr;


private:
	UFUNCTION()
	void HandleGenerateWeaponSpawnPointsClicked();

	UFUNCTION()
	void HandleValidateGeneratedSpawnPointsClicked();

	UFUNCTION()
	void HandleGenerateEnemySpawnPointsClicked();

	UFUNCTION()
	void HandleGenerateSpawnPointsFromDefinitionClicked();

	UFUNCTION()
	void HandleClearGeneratedSpawnPointsClicked();

	UFUNCTION()
	void HandleSelectGeneratedSpawnPointsClicked();

	UFUNCTION()
	void HandleUpdateSpawnPointManagerClicked();

	UFUNCTION()
	void HandleSelectSpawnPointManagerClicked();

	UWorld* GetEditorWorld() const;
	int32 MakeGenerationSeed();
	AMyFpsSpawnPointManager* FindOrCreateSpawnPointManager(UWorld* World);
	void ApplyGenerationDefinitionSettings();

	// 单点创建武器刷新点，供“单武器生成”和“按 DA 生成”复用。
	AMyFpsWeaponSpawnPoint* CreateWeaponSpawnPoint(
		UWorld* World,
		const FVector& SpawnLocation,
		const FRotator& SpawnRotation,
		UMyFpsWeaponDefinition* NewWeaponDefinition,
		TSubclassOf<AMyFpsWeaponPickupActor> NewWeaponPickupActorClass,
		bool bNewSpawnOnBeginPlay,
		bool bNewRespawnAfterPickup,
		float NewRespawnDelay,
		int32 SpawnIndex);

	// 单点创建敌人刷新点，供“单敌人生成”和“按 DA 生成”复用。
	AMyFpsEnemySpawnPoint* CreateEnemySpawnPoint(
		UWorld* World,
		const FVector& SpawnLocation,
		const FRotator& SpawnRotation,
		TSubclassOf<AEnemyCharacter> NewEnemyActorClass,
		bool bNewSpawnOnBeginPlay,
		bool bNewRespawnAfterDestroyed,
		float NewRespawnDelay,
		int32 SpawnIndex);
	void AppendExistingGeneratedLocations(UWorld* World, TArray<FVector>& InOutLocations) const;
	FVector MakeRandomPointInCircle(FRandomStream& RandomStream, const FVector& Center) const;
	bool SnapLocationToGround(UWorld* World, FVector& InOutLocation) const;
	FRotator MakeSpawnRotation(FRandomStream& RandomStream) const;
	bool IsFarEnoughFromExistingPoints(const FVector& CandidateLocation, const TArray<FVector>& ExistingLocations) const;
	TArray<AActor*> GetGeneratedSpawnPointActors(UWorld* World) const;
	void SetStatusText(const FText& NewStatusText);

	int32 GenerationSerial = 0;
};
