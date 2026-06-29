// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MyFpsCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class USceneComponent;
class UCameraComponent;
class UAnimMontage;
class USoundBase;
class UMyFpsDeathWidget;
class UMyFpsDamageFeedbackWidget;
class UMyFpsGameInstance;
class UMyFpsHitMarkerWidget;
class UMyFpsMatchResultWidget;
class UMyFpsScoreWidget;
class UMyFpsScoreboardWidget;
class UMyFpsWeaponCombatComponent;
class UMyFpsWeaponDefinition;
class UMyFpsWeaponInventoryComponent;
class UMyFpsWeaponRecoilComponent;
class UMyFpsWeaponViewComponent;
class UUserWidget;
class UTP_PickUpComponent;
class AMyFpsGameState;
class AMyFpsWeaponPickupActor;
// Declaration of the delegate that will be called when the Primary Action is triggered
// It is declared as dynamic so it can be accessed also in Blueprints
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUseItem);

UCLASS(config=Game)
class AMyFpsCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	USkeletalMeshComponent* Mesh1P;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMyFpsWeaponInventoryComponent> WeaponInventoryComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMyFpsWeaponViewComponent> WeaponViewComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMyFpsWeaponCombatComponent> WeaponCombatComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMyFpsWeaponRecoilComponent> WeaponRecoilComponent = nullptr;

public:
	AMyFpsCharacter();

    virtual float TakeDamage(
        float DamageAmount,
        struct FDamageEvent const& DamageEvent,
        class AController* EventInstigator,
        AActor* DamageCauser
    ) override;
	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetHealth() const;

	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetMaxHealth() const;

	UFUNCTION(BlueprintCallable, Category = "Health")
	bool IsDead() const;

	UFUNCTION(BlueprintCallable, Category = "Health")
	void ResetHealthToMax();

	void ScheduleAutoRespawn(float DelaySeconds);
	void CancelAutoRespawn();

	UFUNCTION(BlueprintPure, Category = "Interaction")
	bool HasAvailablePickup() const;

	UFUNCTION(BlueprintPure, Category = "Interaction")
	bool HasWeaponEquipped() const;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	int32 GetCurrentAmmoInClip() const;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	int32 GetCurrentReserveAmmo() const;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool IsCurrentWeaponReloading() const;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool IsCurrentWeaponEquipping() const;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool IsCurrentWeaponFiring() const;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	int32 GetCurrentWeaponFireSequence() const;

	UFUNCTION(BlueprintPure, Category = "Match")
	bool IsMatchFinished() const;

	UFUNCTION(BlueprintPure, Category = "Match")
	bool IsMatchInProgress() const;

	UFUNCTION(BlueprintCallable, Category = "Match")
	void RequestReadyForNextRound();

	void HandleMatchFinished();
	void HandleMatchStarted();
	void ShowMatchResultFromServer(const FString& WinnerName);
	void ShowHitMarkerFromServer(bool bKill);
	void ShowDamageFeedbackFromServer(const FVector& SourceWorldLocation);
	void ShowRespawnCountdownFromServer(float DelaySeconds);
	void RefreshMatchResultWidget();

	void SetAvailablePickup(UTP_PickUpComponent* PickUpComponent);
	void ClearAvailablePickup(UTP_PickUpComponent* PickUpComponent);

protected:
	virtual void BeginPlay();
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void PawnClientRestart() override;
	virtual void OnRep_Controller() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health")
	float MaxHealth = 100.0f;

	UPROPERTY(ReplicatedUsing = OnRep_Health, BlueprintReadOnly, Category = "Health")
	float Health = 0.0f;

	UPROPERTY(ReplicatedUsing = OnRep_IsDead, BlueprintReadOnly, Category = "Health")
	bool bIsDead = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UMyFpsScoreWidget> ScoreWidgetClass;
	
	UPROPERTY()
	TObjectPtr<UMyFpsScoreWidget> ScoreWidgetInstance = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UMyFpsDeathWidget> DeathWidgetClass;

	UPROPERTY()
	TObjectPtr<UMyFpsDeathWidget> DeathWidgetInstance = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UMyFpsMatchResultWidget> MatchResultWidgetClass;

	UPROPERTY()
	TObjectPtr<UMyFpsMatchResultWidget> MatchResultWidgetInstance = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UMyFpsScoreboardWidget> ScoreboardWidgetClass;

	UPROPERTY()
	TObjectPtr<UMyFpsScoreboardWidget> ScoreboardWidgetInstance = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> CrosshairWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> CrosshairWidgetInstance = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UMyFpsHitMarkerWidget> HitMarkerWidgetClass;

	UPROPERTY()
	TObjectPtr<UMyFpsHitMarkerWidget> HitMarkerWidgetInstance = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UMyFpsDamageFeedbackWidget> DamageFeedbackWidgetClass;

	UPROPERTY()
	TObjectPtr<UMyFpsDamageFeedbackWidget> DamageFeedbackWidgetInstance = nullptr;

	UPROPERTY()
	TObjectPtr<UTP_PickUpComponent> AvailablePickup = nullptr;

	UPROPERTY()
	TObjectPtr<AMyFpsWeaponPickupActor> AvailableWeaponPickupActor = nullptr;

	TWeakObjectPtr<AMyFpsGameState> CachedGameState;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction")
	float WeaponFocusTraceDistance = 2500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction")
	float WeaponHighlightMaxDistance = 500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Drop")
	TSubclassOf<AMyFpsWeaponPickupActor> WeaponPickupActorClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Drop")
	float DroppedWeaponForwardOffset = 80.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Drop")
	float DroppedWeaponUpOffset = 20.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Drop")
	float DroppedWeaponForwardImpulse = 120.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Drop")
	float DroppedWeaponUpImpulse = 60.0f;

	UFUNCTION()
	void OnRep_Health();

	UFUNCTION()
	void OnRep_IsDead();

	void Die(AController* KillerController);
	void ApplyDeathState();
	void PerformAutoRespawn();
	void DropEquippedWeapons();
	bool DropInventoryWeapon();
	AMyFpsWeaponPickupActor* SpawnDroppedInventoryWeapon(UMyFpsWeaponDefinition* WeaponDefinition);
	void RefreshFirstPersonMeshVisibility();
	void CreateScoreWidget();
	void CreateDeathWidget();
	void CreateMatchResultWidget();
	void CreateScoreboardWidget();
	void CreateCrosshairWidget();
	void CreateHitMarkerWidget();
	void CreateDamageFeedbackWidget();
	void SetCrosshairVisible(bool bVisible);
	void ShowScoreboard();
	void HideScoreboard();
	void InteractPickup();
	void DropCurrentWeaponInput();
	void RespawnInput();
	void HandleRespawnRequest();
	void PerformPickupInteraction(AActor* DesiredPickupActor = nullptr);
	void PerformDropCurrentWeapon();
	void UpdateFocusedWeaponHighlight();
	void SetFocusedWeaponPickupActor(AMyFpsWeaponPickupActor* NewFocusedWeaponPickupActor);
	AActor* GetDesiredPickupActor() const;
	bool IsPickupActorReachable(AActor* PickupActor) const;
	void BindToMatchState();
	void UnbindFromMatchState();
	void HandleMatchStateChanged();
	void ToggleHostControlMenu();

	FTimerHandle AutoRespawnTimerHandle;

	UFUNCTION(Server, Reliable)
	void ServerRequestRespawn();

	UFUNCTION(Server, Reliable)
	void ServerInteractPickup(AActor* DesiredPickupActor);

	UFUNCTION(Server, Reliable)
	void ServerDropCurrentWeapon();

	UFUNCTION(Server, Reliable)
	void ServerReadyForNextRound();

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps
	) const override;
public:
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float TurnRateGamepad;



	/** Delegate to whom anyone can subscribe to receive this event */
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOnUseItem OnUseItem;
protected:
	
	/** Fires a projectile. */
	void OnPrimaryAction();
	void StopPrimaryAction();
	void StartJump();
	void StopJumpInput();
	void TurnMouse(float Value);
	void LookUpMouse(float Value);

	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles strafing movement, left and right */
	void MoveRight(float Val);

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	struct TouchData
	{
		TouchData() { bIsPressed = false;Location=FVector::ZeroVector;}
		bool bIsPressed;
		ETouchIndex::Type FingerIndex;
		FVector Location;
		bool bMoved;
	};
	void BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	void EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	void TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location);
	TouchData	TouchItem;
	
protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

	/* 
	 * Configures input for touchscreen devices if there is a valid touch interface for doing so 
	 *
	 * @param	InputComponent	The input component pointer to bind controls to
	 * @returns true if touch controls were enabled.
	 */
	bool EnableTouchscreenMovement(UInputComponent* InputComponent);

public:
	/** Returns Mesh1P subobject **/
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	UMyFpsWeaponInventoryComponent* GetWeaponInventoryComponent() const { return WeaponInventoryComponent; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	UMyFpsWeaponRecoilComponent* GetWeaponRecoilComponent() const { return WeaponRecoilComponent; }


};
