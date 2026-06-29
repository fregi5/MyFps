// Copyright Epic Games, Inc. All Rights Reserved.

#include "MyFpsCharacter.h"
#include "MyFpsProjectile.h"
#include "MyFpsDamageFeedbackWidget.h"
#include "MyFpsDeathWidget.h"
#include "MyFpsGameInstance.h"
#include "MyFpsGameMode.h"
#include "MyFpsGameState.h"
#include "MyFpsHitMarkerWidget.h"
#include "MyFpsMatchResultWidget.h"
#include "MyFpsPlayerController.h"
#include "MyFpsScoreWidget.h"
#include "MyFpsWeaponCombatComponent.h"
#include "MyFpsWeaponDefinition.h"
#include "MyFpsWeaponInventoryComponent.h"
#include "MyFpsWeaponRecoilComponent.h"
#include "MyFpsWeaponViewComponent.h"
#include "MyFpsWeaponPickupActor.h"
#include "TP_PickUpComponent.h"
#include "MyFpsScoreboardWidget.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/MeshComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputCoreTypes.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/InputSettings.h"
#include "Net/UnrealNetwork.h"
#include "Blueprint/UserWidget.h"
#include "UObject/ConstructorHelpers.h"

//////////////////////////////////////////////////////////////////////////
// AMyFpsCharacter

AMyFpsCharacter::AMyFpsCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	TurnRateGamepad = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	WeaponInventoryComponent = CreateDefaultSubobject<UMyFpsWeaponInventoryComponent>(TEXT("WeaponInventoryComponent"));
	WeaponViewComponent = CreateDefaultSubobject<UMyFpsWeaponViewComponent>(TEXT("WeaponViewComponent"));
	WeaponCombatComponent = CreateDefaultSubobject<UMyFpsWeaponCombatComponent>(TEXT("WeaponCombatComponent"));
	WeaponRecoilComponent = CreateDefaultSubobject<UMyFpsWeaponRecoilComponent>(TEXT("WeaponRecoilComponent"));

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetOwnerNoSee(false);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));
	Mesh1P->SetVisibility(false, true);

	GetMesh()->SetOnlyOwnerSee(false);
	GetMesh()->SetOwnerNoSee(true);

	bReplicates = true;

	DeathWidgetClass = UMyFpsDeathWidget::StaticClass();
	MatchResultWidgetClass = UMyFpsMatchResultWidget::StaticClass();
	ScoreWidgetClass = UMyFpsScoreWidget::StaticClass();
	ScoreboardWidgetClass = UMyFpsScoreboardWidget::StaticClass();

}



void AMyFpsCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
	ResetHealthToMax();
	UE_LOG(LogTemp, Warning, TEXT("Player BeginPlay, Health = %.1f"), Health);

	CreateDeathWidget();
	CreateMatchResultWidget();
	CreateScoreWidget();
	CreateScoreboardWidget();
	CreateCrosshairWidget();
	CreateHitMarkerWidget();
	CreateDamageFeedbackWidget();
	BindToMatchState();
	RefreshFirstPersonMeshVisibility();
	if (WeaponViewComponent)
	{
		WeaponViewComponent->RefreshWeaponVisuals();
	}
}

void AMyFpsCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CancelAutoRespawn();
	UnbindFromMatchState();
	SetFocusedWeaponPickupActor(nullptr);

	if (DeathWidgetInstance)
	{
		DeathWidgetInstance->RemoveFromParent();
		DeathWidgetInstance = nullptr;
	}

	if (ScoreWidgetInstance)
	{
		ScoreWidgetInstance->RemoveFromParent();
		ScoreWidgetInstance = nullptr;
	}

	if (MatchResultWidgetInstance)
	{
		MatchResultWidgetInstance->RemoveFromParent();
		MatchResultWidgetInstance = nullptr;
	}

	if (ScoreboardWidgetInstance)
	{
		ScoreboardWidgetInstance->RemoveFromParent();
		ScoreboardWidgetInstance = nullptr;
	}

	if (CrosshairWidgetInstance)
	{
		CrosshairWidgetInstance->RemoveFromParent();
		CrosshairWidgetInstance = nullptr;
	}

	if (HitMarkerWidgetInstance)
	{
		HitMarkerWidgetInstance->RemoveFromParent();
		HitMarkerWidgetInstance = nullptr;
	}

	if (DamageFeedbackWidgetInstance)
	{
		DamageFeedbackWidgetInstance->RemoveFromParent();
		DamageFeedbackWidgetInstance = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void AMyFpsCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();
	CreateDeathWidget();
	CreateMatchResultWidget();
	CreateScoreWidget();
	CreateScoreboardWidget();
	CreateCrosshairWidget();
	CreateHitMarkerWidget();
	CreateDamageFeedbackWidget();
	BindToMatchState();
	SetCrosshairVisible(!bIsDead && !IsMatchFinished());
	RefreshFirstPersonMeshVisibility();
	if (WeaponViewComponent)
	{
		WeaponViewComponent->RefreshWeaponVisuals();
	}
}

void AMyFpsCharacter::OnRep_Controller()
{
	Super::OnRep_Controller();
	CreateDeathWidget();
	CreateMatchResultWidget();
	CreateScoreWidget();
	CreateScoreboardWidget();
	CreateCrosshairWidget();
	BindToMatchState();
	SetCrosshairVisible(!bIsDead && !IsMatchFinished());
	RefreshFirstPersonMeshVisibility();
	if (WeaponViewComponent)
	{
		WeaponViewComponent->RefreshWeaponVisuals();
	}
}

void AMyFpsCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	BindToMatchState();
	UpdateFocusedWeaponHighlight();
}

void AMyFpsCharacter::BindToMatchState()
{
	if (CachedGameState.IsValid())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	AMyFpsGameState* GameState = World->GetGameState<AMyFpsGameState>();
	if (!GameState)
	{
		return;
	}

	GameState->OnMatchStateChanged().AddUObject(this, &AMyFpsCharacter::HandleMatchStateChanged);
	CachedGameState = GameState;
	HandleMatchStateChanged();
}

void AMyFpsCharacter::UnbindFromMatchState()
{
	if (AMyFpsGameState* GameState = CachedGameState.Get())
	{
		GameState->OnMatchStateChanged().RemoveAll(this);
	}

	CachedGameState.Reset();
}

void AMyFpsCharacter::HandleMatchStateChanged()
{
	if (IsMatchFinished())
	{
		CreateMatchResultWidget();
		if (MatchResultWidgetInstance)
		{
			MatchResultWidgetInstance->RefreshFromGameState();
		}

		HandleMatchFinished();
		return;
	}

	if (IsMatchInProgress())
	{
		HandleMatchStarted();
	}
}

//////////////////////////////////////////////////////////////////////////// Input

void AMyFpsCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	CreateDeathWidget();
	CreateScoreWidget();
	CreateScoreboardWidget();
	CreateCrosshairWidget();
	SetCrosshairVisible(!bIsDead && !IsMatchFinished());

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMyFpsCharacter::StartJump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &AMyFpsCharacter::StopJumpInput);

	// Bind fire event
	PlayerInputComponent->BindAction("PrimaryAction", IE_Pressed, this, &AMyFpsCharacter::OnPrimaryAction);
	PlayerInputComponent->BindAction("PrimaryAction", IE_Released, this, &AMyFpsCharacter::StopPrimaryAction);

	// Enable touchscreen input
	EnableTouchscreenMovement(PlayerInputComponent);

	// Bind movement events
	PlayerInputComponent->BindAxis("Move Forward / Backward", this, &AMyFpsCharacter::MoveForward);
	PlayerInputComponent->BindAxis("Move Right / Left", this, &AMyFpsCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "Mouse" versions handle devices that provide an absolute delta, such as a mouse.
	// "Gamepad" versions are for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn Right / Left Mouse", this, &AMyFpsCharacter::TurnMouse);
	PlayerInputComponent->BindAxis("Look Up / Down Mouse", this, &AMyFpsCharacter::LookUpMouse);
	PlayerInputComponent->BindAxis("Turn Right / Left Gamepad", this, &AMyFpsCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("Look Up / Down Gamepad", this, &AMyFpsCharacter::LookUpAtRate);
PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AMyFpsCharacter::InteractPickup);
	PlayerInputComponent->BindAction("DropWeapon", IE_Pressed, this, &AMyFpsCharacter::DropCurrentWeaponInput);
	PlayerInputComponent->BindKey(EKeys::Tab, IE_Pressed, this, &AMyFpsCharacter::ShowScoreboard);
	PlayerInputComponent->BindKey(EKeys::Tab, IE_Released, this, &AMyFpsCharacter::HideScoreboard);
	PlayerInputComponent->BindKey(EKeys::R, IE_Pressed, this, &AMyFpsCharacter::RespawnInput);
	PlayerInputComponent->BindKey(EKeys::F6, IE_Pressed, this, &AMyFpsCharacter::ToggleHostControlMenu);
}

void AMyFpsCharacter::ToggleHostControlMenu()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController || !PlayerController->IsLocalController())
	{
		return;
	}

	if (UMyFpsGameInstance* GameInstance = GetGameInstance<UMyFpsGameInstance>())
	{
		GameInstance->ToggleHostControlMenu(PlayerController);
	}
}

void AMyFpsCharacter::OnPrimaryAction()
{
	if (bIsDead || !IsMatchInProgress())
	{
		return;
	}

	if (WeaponCombatComponent && WeaponInventoryComponent && WeaponInventoryComponent->HasWeapon())
	{
		WeaponCombatComponent->StartFire();
		return;
	}

	OnUseItem.Broadcast();
}

void AMyFpsCharacter::StopPrimaryAction()
{
	if (bIsDead || IsMatchFinished())
	{
		return;
	}

	if (WeaponCombatComponent && WeaponInventoryComponent && WeaponInventoryComponent->HasWeapon())
	{
		WeaponCombatComponent->StopFire();
	}
}

void AMyFpsCharacter::StartJump()
{
	if (bIsDead || IsMatchFinished())
	{
		return;
	}

	Jump();
}

void AMyFpsCharacter::StopJumpInput()
{
	if (bIsDead || IsMatchFinished())
	{
		return;
	}

	StopJumping();
}

void AMyFpsCharacter::BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == true)
	{
		return;
	}
	if ((FingerIndex == TouchItem.FingerIndex) && (TouchItem.bMoved == false))
	{
		OnPrimaryAction();
	}
	TouchItem.bIsPressed = true;
	TouchItem.FingerIndex = FingerIndex;
	TouchItem.Location = Location;
	TouchItem.bMoved = false;
}

void AMyFpsCharacter::EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == false)
	{
		return;
	}
	TouchItem.bIsPressed = false;
	StopPrimaryAction();
}

void AMyFpsCharacter::MoveForward(float Value)
{
	if (bIsDead || IsMatchFinished())
	{
		return;
	}

	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AMyFpsCharacter::MoveRight(float Value)
{
	if (bIsDead || IsMatchFinished())
	{
		return;
	}

	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AMyFpsCharacter::TurnMouse(float Value)
{
	if (bIsDead || IsMatchFinished())
	{
		return;
	}

	AddControllerYawInput(Value);
}

void AMyFpsCharacter::LookUpMouse(float Value)
{
	if (bIsDead || IsMatchFinished())
	{
		return;
	}

	AddControllerPitchInput(Value);
}

void AMyFpsCharacter::TurnAtRate(float Rate)
{
	if (bIsDead || IsMatchFinished())
	{
		return;
	}

	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void AMyFpsCharacter::LookUpAtRate(float Rate)
{
	if (bIsDead || IsMatchFinished())
	{
		return;
	}

	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

bool AMyFpsCharacter::EnableTouchscreenMovement(class UInputComponent* PlayerInputComponent)
{
	if (FPlatformMisc::SupportsTouchInput() || GetDefault<UInputSettings>()->bUseMouseForTouch)
	{
		PlayerInputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AMyFpsCharacter::BeginTouch);
		PlayerInputComponent->BindTouch(EInputEvent::IE_Released, this, &AMyFpsCharacter::EndTouch);

		return true;
	}
	
	return false;
}

void AMyFpsCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)
const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMyFpsCharacter, Health);
	DOREPLIFETIME(AMyFpsCharacter, bIsDead);
}

float AMyFpsCharacter::TakeDamage(float DamageAmount, FDamageEvent const &DamageEvent, AController *EventInstigator, AActor *DamageCauser)
{
	if (!HasAuthority() || !IsMatchInProgress())
	{
		return 0.0f;
	}

	const float ActualDamage = Super::TakeDamage(
        DamageAmount,
        DamageEvent,
        EventInstigator,
        DamageCauser
    );
	if(ActualDamage<=0.0f||bIsDead){
		return 0.0f;
	}
	Health = FMath::Clamp(Health - ActualDamage, 0.0f, MaxHealth);
	UE_LOG(
            LogTemp,
            Warning,
            TEXT("%s took %.1f damage, Health = %.1f"),
            *GetName(),
            ActualDamage,
            Health
        );

	FVector DamageSourceLocation = GetActorLocation();
	if (DamageCauser)
	{
		DamageSourceLocation = DamageCauser->GetActorLocation();
	}
	else if (EventInstigator && EventInstigator->GetPawn())
	{
		DamageSourceLocation = EventInstigator->GetPawn()->GetActorLocation();
	}

	if (AMyFpsPlayerController* VictimController = Cast<AMyFpsPlayerController>(GetController()))
	{
		VictimController->ClientShowDamageFeedback(DamageSourceLocation);
	}

	if (Health <= 0.0f)
	{
		Die(EventInstigator);
	}

	return ActualDamage;
}
void AMyFpsCharacter::OnRep_Health()
{
    UE_LOG(
        LogTemp,
        Warning,
        TEXT("%s Health replicated: %.1f"),
        *GetName(),
        Health
    );
}

void AMyFpsCharacter::OnRep_IsDead()
{
	ApplyDeathState();
}

void AMyFpsCharacter::Die(AController *KillerController)
{
	if(bIsDead){
		return;
	}
	bIsDead=true;
	Health=0.0f;

	if (AMyFpsGameMode* GameMode = GetWorld()->GetAuthGameMode<AMyFpsGameMode>())
	{
		GameMode->RegisterDeath(GetController());

		if (KillerController && KillerController != GetController())
		{
			GameMode->RegisterKill(KillerController);
			GameMode->RegisterKillFeed(KillerController, GetController());
			GameMode->AddScore(KillerController, GameMode->GetPlayerKillScore());
		}

		GameMode->SchedulePlayerRespawn(this);
	}

	UE_LOG(
        LogTemp,
        Warning,
        TEXT("%s died. KillerController = %s"),
        *GetName(),
        KillerController ? *KillerController->GetName() : TEXT("None")
    );

	DropEquippedWeapons();
	SetFocusedWeaponPickupActor(nullptr);
	ApplyDeathState();
}

void AMyFpsCharacter::ApplyDeathState()
{
	if (!bIsDead)
	{
		return;
	}

	// 先做一个最简单版本：死亡后隐藏角色并关闭碰撞，后期可以改布娃娃
	SetActorEnableCollision(false);
	SetActorHiddenInGame(true);
	RefreshFirstPersonMeshVisibility();
	if (WeaponViewComponent)
	{
		WeaponViewComponent->ClearWeaponVisuals();
	}
	if (WeaponRecoilComponent)
	{
		WeaponRecoilComponent->ResetRecoilState();
	}

	if (DeathWidgetInstance && IsLocallyControlled())
	{
		DeathWidgetInstance->SetVisibility(ESlateVisibility::Visible);
	}

	SetCrosshairVisible(false);
}

void AMyFpsCharacter::ScheduleAutoRespawn(float DelaySeconds)
{
	if (!HasAuthority() || !bIsDead)
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(AutoRespawnTimerHandle);
	GetWorldTimerManager().SetTimer(
		AutoRespawnTimerHandle,
		this,
		&AMyFpsCharacter::PerformAutoRespawn,
		FMath::Max(0.1f, DelaySeconds),
		false);

	if (AMyFpsPlayerController* PlayerController = Cast<AMyFpsPlayerController>(GetController()))
	{
		PlayerController->ClientStartRespawnCountdown(DelaySeconds);
	}
}

void AMyFpsCharacter::CancelAutoRespawn()
{
	GetWorldTimerManager().ClearTimer(AutoRespawnTimerHandle);
	if (DeathWidgetInstance)
	{
		DeathWidgetInstance->CancelRespawnCountdown();
	}
}

void AMyFpsCharacter::PerformAutoRespawn()
{
	if (!HasAuthority() || !bIsDead || IsMatchFinished())
	{
		return;
	}

	if (AMyFpsGameMode* GameMode = GetWorld()->GetAuthGameMode<AMyFpsGameMode>())
	{
		GameMode->RespawnPlayer(GetController());
	}
}
float AMyFpsCharacter::GetHealth() const
{
    return Health;
}

float AMyFpsCharacter::GetMaxHealth() const
{
    return MaxHealth;
}

bool AMyFpsCharacter::IsDead() const
{
    return bIsDead;
}

void AMyFpsCharacter::ResetHealthToMax()
{
	if (!HasAuthority())
	{
		return;
	}

	Health = FMath::Max(0.0f, MaxHealth);
	bIsDead = false;
	ForceNetUpdate();
}

bool AMyFpsCharacter::HasAvailablePickup() const
{
	return AvailablePickup != nullptr || AvailableWeaponPickupActor != nullptr;
}

bool AMyFpsCharacter::HasWeaponEquipped() const
{
	return WeaponInventoryComponent && WeaponInventoryComponent->HasWeapon();
}

int32 AMyFpsCharacter::GetCurrentAmmoInClip() const
{
	return WeaponInventoryComponent ? WeaponInventoryComponent->GetCurrentAmmoInClip() : 0;
}

int32 AMyFpsCharacter::GetCurrentReserveAmmo() const
{
	return WeaponInventoryComponent ? WeaponInventoryComponent->GetCurrentReserveAmmo() : 0;
}

bool AMyFpsCharacter::IsCurrentWeaponReloading() const
{
	return WeaponInventoryComponent ? WeaponInventoryComponent->IsReloading() : false;
}

bool AMyFpsCharacter::IsCurrentWeaponEquipping() const
{
	return false;
}

bool AMyFpsCharacter::IsCurrentWeaponFiring() const
{
	return WeaponInventoryComponent
		? WeaponInventoryComponent->GetCurrentWeaponState().bFiring
		: false;
}

int32 AMyFpsCharacter::GetCurrentWeaponFireSequence() const
{
	return WeaponInventoryComponent
		? WeaponInventoryComponent->GetCurrentWeaponState().FireSequence
		: 0;
}

bool AMyFpsCharacter::IsMatchFinished() const
{
	const AMyFpsGameState* GameState = GetWorld() ? GetWorld()->GetGameState<AMyFpsGameState>() : nullptr;
	return GameState ? GameState->IsMatchFinished() : false;
}

bool AMyFpsCharacter::IsMatchInProgress() const
{
	const AMyFpsGameState* GameState = GetWorld() ? GetWorld()->GetGameState<AMyFpsGameState>() : nullptr;
	return GameState && GameState->IsMyFpsMatchStarted() && !GameState->IsMatchFinished();
}

void AMyFpsCharacter::RequestReadyForNextRound()
{
	if (HasAuthority())
	{
		if (AMyFpsGameMode* GameMode = GetWorld()->GetAuthGameMode<AMyFpsGameMode>())
		{
			GameMode->MarkPlayerReadyForNextRound(GetController());
		}
		return;
	}

	if (AMyFpsPlayerController* MyPlayerController = Cast<AMyFpsPlayerController>(GetController()))
	{
		MyPlayerController->ServerSetReadyForNextRound();
	}
}

void AMyFpsCharacter::SetAvailablePickup(UTP_PickUpComponent* PickUpComponent)
{
	AvailablePickup = PickUpComponent;
}

void AMyFpsCharacter::ClearAvailablePickup(UTP_PickUpComponent* PickUpComponent)
{
	if (AvailablePickup == PickUpComponent)
	{
		AvailablePickup = nullptr;
	}
}

void AMyFpsCharacter::DropEquippedWeapons()
{
	DropInventoryWeapon();
}

bool AMyFpsCharacter::DropInventoryWeapon()
{
	if (!HasAuthority()
		|| !WeaponInventoryComponent
		|| !WeaponInventoryComponent->HasWeapon())
	{
		return false;
	}

	UMyFpsWeaponDefinition* WeaponDefinition = WeaponInventoryComponent->GetCurrentWeaponDefinition();
	if (!WeaponDefinition)
	{
		return false;
	}

	if (WeaponCombatComponent)
	{
		WeaponCombatComponent->StopFire();
	}
	if (WeaponRecoilComponent)
	{
		WeaponRecoilComponent->ResetRecoilState();
	}

	AMyFpsWeaponPickupActor* DroppedPickup = SpawnDroppedInventoryWeapon(WeaponDefinition);
	if (!DroppedPickup)
	{
		return false;
	}

	WeaponInventoryComponent->ClearCurrentWeapon();
	if (WeaponViewComponent)
	{
		WeaponViewComponent->RefreshWeaponVisuals();
	}
	RefreshFirstPersonMeshVisibility();
	SetFocusedWeaponPickupActor(nullptr);
	return true;
}

AMyFpsWeaponPickupActor* AMyFpsCharacter::SpawnDroppedInventoryWeapon(UMyFpsWeaponDefinition* WeaponDefinition)
{
	if (!HasAuthority() || !WeaponDefinition)
	{
		return nullptr;
	}

	if (!WeaponPickupActorClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Weapon] Cannot drop inventory weapon: WeaponPickupActorClass is not set on %s."), *GetNameSafe(this));
		return nullptr;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	const FVector SpawnLocation = GetActorLocation()
		+ GetActorForwardVector() * DroppedWeaponForwardOffset
		+ FVector::UpVector * DroppedWeaponUpOffset;
	const FRotator SpawnRotation(0.0f, GetActorRotation().Yaw, 0.0f);

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.Instigator = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AMyFpsWeaponPickupActor* DroppedPickup = World->SpawnActor<AMyFpsWeaponPickupActor>(
		WeaponPickupActorClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParameters);
	if (!DroppedPickup)
	{
		return nullptr;
	}

	DroppedPickup->SetWeaponDefinition(WeaponDefinition);
	DroppedPickup->SetPickupEnabled(true);
	DroppedPickup->StartDroppedPhysics(
		GetActorForwardVector() * DroppedWeaponForwardImpulse
		+ FVector::UpVector * DroppedWeaponUpImpulse);
	return DroppedPickup;
}

void AMyFpsCharacter::RefreshFirstPersonMeshVisibility()
{
	if (!Mesh1P)
	{
		return;
	}

	const bool bShouldShowFirstPersonMesh = IsLocallyControlled()
		&& WeaponInventoryComponent
		&& WeaponInventoryComponent->HasWeapon()
		&& !bIsDead;
	Mesh1P->SetVisibility(bShouldShowFirstPersonMesh, true);
}

void AMyFpsCharacter::CreateScoreWidget()
{
	if (!ScoreWidgetClass || ScoreWidgetInstance)
	{
		return;
	}

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController || !PlayerController->IsLocalController())
	{
		return;
	}

	ScoreWidgetInstance = CreateWidget<UMyFpsScoreWidget>(PlayerController, ScoreWidgetClass);
	if (!ScoreWidgetInstance)
	{
		return;
	}

	ScoreWidgetInstance->AddToViewport(0);
}

void AMyFpsCharacter::CreateDeathWidget()
{
	if (!DeathWidgetClass || DeathWidgetInstance)
	{
		return;
	}

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController || !PlayerController->IsLocalController())
	{
		return;
	}

	DeathWidgetInstance = CreateWidget<UMyFpsDeathWidget>(PlayerController, DeathWidgetClass);
	if (!DeathWidgetInstance)
	{
		return;
	}

	DeathWidgetInstance->AddToViewport(20);
	DeathWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
}

void AMyFpsCharacter::CreateMatchResultWidget()
{
	if (!MatchResultWidgetClass || MatchResultWidgetInstance)
	{
		return;
	}

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController || !PlayerController->IsLocalController())
	{
		return;
	}

	MatchResultWidgetInstance = CreateWidget<UMyFpsMatchResultWidget>(PlayerController, MatchResultWidgetClass);
	if (!MatchResultWidgetInstance)
	{
		return;
	}

	MatchResultWidgetInstance->AddToViewport(100);
	MatchResultWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
}

void AMyFpsCharacter::CreateScoreboardWidget()
{
	if (!ScoreboardWidgetClass || ScoreboardWidgetInstance)
	{
		return;
	}

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController || !PlayerController->IsLocalController())
	{
		return;
	}

	ScoreboardWidgetInstance = CreateWidget<UMyFpsScoreboardWidget>(PlayerController, ScoreboardWidgetClass);
	if (!ScoreboardWidgetInstance)
	{
		return;
	}

	ScoreboardWidgetInstance->AddToViewport(10);
	ScoreboardWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
}

void AMyFpsCharacter::CreateCrosshairWidget()
{
	if (!CrosshairWidgetClass || CrosshairWidgetInstance)
	{
		return;
	}

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController || !PlayerController->IsLocalController())
	{
		return;
	}

	CrosshairWidgetInstance = CreateWidget<UUserWidget>(PlayerController, CrosshairWidgetClass);
	if (!CrosshairWidgetInstance)
	{
		return;
	}

	CrosshairWidgetInstance->AddToViewport(5);
	SetCrosshairVisible(true);
}

void AMyFpsCharacter::CreateHitMarkerWidget()
{
	if (HitMarkerWidgetInstance)
	{
		return;
	}
	if (!HitMarkerWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[HitMarker] HitMarkerWidgetClass is not set on %s."), *GetNameSafe(this));
		return;
	}

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController || !PlayerController->IsLocalController())
	{
		UE_LOG(LogTemp, Warning, TEXT("[HitMarker] Cannot create widget. Character=%s Controller=%s Local=%d"), *GetNameSafe(this), *GetNameSafe(PlayerController), PlayerController && PlayerController->IsLocalController());
		return;
	}

	HitMarkerWidgetInstance = CreateWidget<UMyFpsHitMarkerWidget>(PlayerController, HitMarkerWidgetClass);
	if (!HitMarkerWidgetInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("[HitMarker] CreateWidget failed. WidgetClass=%s"), *GetNameSafe(HitMarkerWidgetClass));
		return;
	}

	HitMarkerWidgetInstance->AddToViewport(15);
	HitMarkerWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
	UE_LOG(LogTemp, Warning, TEXT("[HitMarker] Widget created. Character=%s Widget=%s Class=%s"), *GetNameSafe(this), *GetNameSafe(HitMarkerWidgetInstance), *GetNameSafe(HitMarkerWidgetClass));
}

void AMyFpsCharacter::CreateDamageFeedbackWidget()
{
	if (DamageFeedbackWidgetInstance || !DamageFeedbackWidgetClass)
	{
		return;
	}

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController || !PlayerController->IsLocalController())
	{
		return;
	}

	DamageFeedbackWidgetInstance = CreateWidget<UMyFpsDamageFeedbackWidget>(PlayerController, DamageFeedbackWidgetClass);
	if (!DamageFeedbackWidgetInstance)
	{
		return;
	}

	DamageFeedbackWidgetInstance->AddToViewport(16);
	DamageFeedbackWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
}

void AMyFpsCharacter::SetCrosshairVisible(bool bVisible)
{
	if (!CrosshairWidgetInstance)
	{
		return;
	}

	CrosshairWidgetInstance->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

void AMyFpsCharacter::ShowScoreboard()
{
	CreateScoreboardWidget();

	if (!ScoreboardWidgetInstance)
	{
		return;
	}

	ScoreboardWidgetInstance->RefreshScoreboard();
	ScoreboardWidgetInstance->SetVisibility(ESlateVisibility::Visible);
}

void AMyFpsCharacter::HideScoreboard()
{
	CreateScoreboardWidget();
	if (!ScoreboardWidgetInstance)
	{
		return;
	}

	ScoreboardWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
}

void AMyFpsCharacter::InteractPickup()
{
	UE_LOG(LogTemp, Warning, TEXT("[Pickup] E pressed. InteractPickup called."));

	AActor* DesiredPickupActor = GetDesiredPickupActor();
	if (bIsDead || !DesiredPickupActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Pickup] Cannot pickup: player is dead or no pickup."));
		return;
	}

	if (!HasAuthority())
	{
		ServerInteractPickup(DesiredPickupActor);
		return;
	}

	PerformPickupInteraction(DesiredPickupActor);
}

void AMyFpsCharacter::DropCurrentWeaponInput()
{
	UE_LOG(LogTemp, Warning, TEXT("[Weapon] G pressed. DropCurrentWeaponInput called."));

	if (bIsDead)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Weapon] Cannot drop: player is dead."));
		return;
	}

	if (!HasAuthority())
	{
		ServerDropCurrentWeapon();
		return;
	}

	PerformDropCurrentWeapon();
}

void AMyFpsCharacter::RespawnInput()
{
	if (IsMatchFinished())
	{
		return;
	}

	if (bIsDead)
	{
		HandleRespawnRequest();
		return;
	}

	if (WeaponCombatComponent && WeaponInventoryComponent && WeaponInventoryComponent->HasWeapon())
	{
		WeaponCombatComponent->Reload();
	}
}

void AMyFpsCharacter::PerformPickupInteraction(AActor* DesiredPickupActor)
{
	if (AMyFpsWeaponPickupActor* NewPickupActor = Cast<AMyFpsWeaponPickupActor>(DesiredPickupActor))
	{
		if (!IsPickupActorReachable(NewPickupActor))
		{
			return;
		}

		if (WeaponInventoryComponent
			&& WeaponInventoryComponent->HasWeapon()
			&& !DropInventoryWeapon())
		{
			return;
		}

		NewPickupActor->TryPickup(this);
		SetFocusedWeaponPickupActor(nullptr);
		return;
	}

	UTP_PickUpComponent* DesiredPickup = DesiredPickupActor
		? DesiredPickupActor->FindComponentByClass<UTP_PickUpComponent>()
		: AvailablePickup.Get();
	if (!DesiredPickup)
	{
		return;
	}

	if (!IsPickupActorReachable(DesiredPickup->GetOwner()))
	{
		return;
	}

	if (WeaponInventoryComponent
		&& WeaponInventoryComponent->HasWeapon()
		&& !DropInventoryWeapon())
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[Pickup] TryPickUp called."));
	DesiredPickup->TryPickUp(this);
	AvailablePickup = nullptr;
}

void AMyFpsCharacter::PerformDropCurrentWeapon()
{
	if (DropInventoryWeapon())
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[Weapon] Cannot drop: no equipped weapon."));
}

void AMyFpsCharacter::HandleRespawnRequest()
{
	if (HasAuthority())
	{
		CancelAutoRespawn();

		if (AMyFpsGameMode* GameMode = GetWorld()->GetAuthGameMode<AMyFpsGameMode>())
		{
			GameMode->RespawnPlayer(GetController());
		}
		return;
	}

	ServerRequestRespawn();
}

void AMyFpsCharacter::HandleMatchFinished()
{
	CancelAutoRespawn();
	SetFocusedWeaponPickupActor(nullptr);

	if (DeathWidgetInstance)
	{
		DeathWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
	}

	if (WeaponCombatComponent)
	{
		WeaponCombatComponent->StopFire();
	}
	if (WeaponRecoilComponent)
	{
		WeaponRecoilComponent->ResetRecoilState();
	}

	SetCrosshairVisible(false);

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
	}

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		StopJumping();
		DisableInput(PlayerController);
		PlayerController->SetIgnoreMoveInput(true);
		PlayerController->SetIgnoreLookInput(true);
		PlayerController->FlushPressedKeys();
	}
}

void AMyFpsCharacter::HandleMatchStarted()
{
	if (bIsDead)
	{
		RefreshFirstPersonMeshVisibility();
		return;
	}

	if (DeathWidgetInstance)
	{
		DeathWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
	}

	SetCrosshairVisible(true);
	if (WeaponRecoilComponent)
	{
		WeaponRecoilComponent->ResetRecoilState();
	}
	RefreshFirstPersonMeshVisibility();
	if (WeaponViewComponent)
	{
		WeaponViewComponent->RefreshWeaponVisuals();
	}

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		EnableInput(PlayerController);
		PlayerController->ResetIgnoreMoveInput();
		PlayerController->ResetIgnoreLookInput();
	}
}

void AMyFpsCharacter::ShowMatchResultFromServer(const FString& WinnerName)
{
	CreateMatchResultWidget();
	if (MatchResultWidgetInstance)
	{
		MatchResultWidgetInstance->ShowExplicitResult(WinnerName);
	}

	HandleMatchFinished();
}

void AMyFpsCharacter::ShowHitMarkerFromServer(bool bKill)
{
	if (!IsLocallyControlled())
	{
		UE_LOG(LogTemp, Warning, TEXT("[HitMarker] Ignored on non-local character %s."), *GetNameSafe(this));
		return;
	}

	CreateHitMarkerWidget();
	if (HitMarkerWidgetInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("[HitMarker] Playing widget feedback. Widget=%s Kill=%d"), *GetNameSafe(HitMarkerWidgetInstance), bKill);
		HitMarkerWidgetInstance->PlayHitMarker(bKill);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[HitMarker] No widget instance is available to play."));
	}
}

void AMyFpsCharacter::ShowDamageFeedbackFromServer(const FVector& SourceWorldLocation)
{
	if (!IsLocallyControlled())
	{
		return;
	}

	CreateDamageFeedbackWidget();
	if (DamageFeedbackWidgetInstance)
	{
		DamageFeedbackWidgetInstance->PlayDamageFeedback(SourceWorldLocation);
	}
}

void AMyFpsCharacter::ShowRespawnCountdownFromServer(float DelaySeconds)
{
	if (!IsLocallyControlled())
	{
		return;
	}

	CreateDeathWidget();
	if (DeathWidgetInstance)
	{
		DeathWidgetInstance->StartRespawnCountdown(DelaySeconds);
	}
}

void AMyFpsCharacter::RefreshMatchResultWidget()
{
	if (MatchResultWidgetInstance)
	{
		MatchResultWidgetInstance->RefreshReadyDisplay();
	}
}

void AMyFpsCharacter::ServerRequestRespawn_Implementation()
{
	HandleRespawnRequest();
}

void AMyFpsCharacter::ServerInteractPickup_Implementation(AActor* DesiredPickupActor)
{
	PerformPickupInteraction(DesiredPickupActor);
}

void AMyFpsCharacter::ServerDropCurrentWeapon_Implementation()
{
	PerformDropCurrentWeapon();
}

void AMyFpsCharacter::ServerReadyForNextRound_Implementation()
{
	if (AMyFpsGameMode* GameMode = GetWorld()->GetAuthGameMode<AMyFpsGameMode>())
	{
		GameMode->MarkPlayerReadyForNextRound(GetController());
	}
}

void AMyFpsCharacter::UpdateFocusedWeaponHighlight()
{
	if (!IsLocallyControlled() || bIsDead || IsMatchFinished() || !FirstPersonCameraComponent)
	{
		SetFocusedWeaponPickupActor(nullptr);
		return;
	}

	const FVector TraceStart = FirstPersonCameraComponent->GetComponentLocation();
	const FVector TraceEnd = TraceStart + FirstPersonCameraComponent->GetForwardVector() * WeaponFocusTraceDistance;

	FHitResult HitResult;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(WeaponFocusTrace), false, this);
	QueryParams.AddIgnoredActor(this);
	QueryParams.bTraceComplex = true;

	AMyFpsWeaponPickupActor* NewFocusedWeaponPickupActor = nullptr;
	if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
	{
		const float HitDistance = FVector::Distance(TraceStart, HitResult.ImpactPoint);
		UPrimitiveComponent* HitPrimitiveComponent = HitResult.GetComponent();
		if (HitDistance <= WeaponHighlightMaxDistance
			&& HitPrimitiveComponent
			&& !HitPrimitiveComponent->IsA<UTP_PickUpComponent>()
			&& !HitPrimitiveComponent->IsA<UWidgetComponent>())
		{
			if (AActor* HitActor = HitPrimitiveComponent->GetOwner())
			{
				if (AMyFpsWeaponPickupActor* WeaponPickupActor = Cast<AMyFpsWeaponPickupActor>(HitActor))
				{
					if (WeaponPickupActor->IsPickupEnabled()
						&& WeaponPickupActor->IsPickupMeshComponent(HitPrimitiveComponent))
					{
						NewFocusedWeaponPickupActor = WeaponPickupActor;
					}
				}
			}
		}
	}

	SetFocusedWeaponPickupActor(NewFocusedWeaponPickupActor);
}

void AMyFpsCharacter::SetFocusedWeaponPickupActor(AMyFpsWeaponPickupActor* NewFocusedWeaponPickupActor)
{
	if (AvailableWeaponPickupActor == NewFocusedWeaponPickupActor)
	{
		if (AvailableWeaponPickupActor)
		{
			AvailableWeaponPickupActor->SetMeshHighlight(true);
		}
		return;
	}

	if (AvailableWeaponPickupActor)
	{
		AvailableWeaponPickupActor->SetMeshHighlight(false);
	}

	AvailableWeaponPickupActor = NewFocusedWeaponPickupActor;

	if (AvailableWeaponPickupActor)
	{
		AvailableWeaponPickupActor->SetMeshHighlight(true);
	}
}

AActor* AMyFpsCharacter::GetDesiredPickupActor() const
{
	if (AvailableWeaponPickupActor)
	{
		return AvailableWeaponPickupActor;
	}

	return AvailablePickup ? AvailablePickup->GetOwner() : nullptr;
}

bool AMyFpsCharacter::IsPickupActorReachable(AActor* PickupActor) const
{
	if (!PickupActor)
	{
		return false;
	}

	const float ReachDistance = FMath::Max(WeaponHighlightMaxDistance, 100.0f) + GetSimpleCollisionRadius();
	return FVector::DistSquared(GetActorLocation(), PickupActor->GetActorLocation()) <= FMath::Square(ReachDistance);
}
