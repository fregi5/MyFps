// Copyright Epic Games, Inc. All Rights Reserved.

#include "TP_PickUpComponent.h"
#include "Components/WidgetComponent.h"
#include "MyFpsPickupPromptWidget.h"
#include "MyFpsWeaponDefinition.h"
#include "MyFpsWeaponPickupActor.h"
UTP_PickUpComponent::UTP_PickUpComponent()
{
	// Setup the Sphere Collision
	InitSphereRadius(50.f);
	bPickupEnabled = true;
	SetIsReplicatedByDefault(true);
}	

void UTP_PickUpComponent::TryPickUp(AMyFpsCharacter* Character)
{
	if (!bPickupEnabled || Character == nullptr)
	{
		return;
	}

	OnPickUp.Broadcast(Character);
}

void UTP_PickUpComponent::SetPickupEnabled(bool bEnabled)
{
	bPickupEnabled = bEnabled;
	RefreshPickupState();
}

void UTP_PickUpComponent::OnRep_PickupEnabled()
{
	RefreshPickupState();
}

void UTP_PickUpComponent::RefreshPickupState()
{
	SetCollisionEnabled(bPickupEnabled ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	SetCollisionObjectType(ECC_WorldDynamic);
	SetCollisionResponseToAllChannels(ECR_Ignore);
	SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SetGenerateOverlapEvents(bPickupEnabled);

	if (!bPickupEnabled)
	{
		HidePickupPrompt();
	}
}

void UTP_PickUpComponent::HidePickupPrompt() const
{
	AActor* WeaponActor = GetOwner();
	if (!WeaponActor)
	{
		return;
	}

	if (UWidgetComponent* PromptWidgetComponent = WeaponActor->FindComponentByClass<UWidgetComponent>())
	{
		PromptWidgetComponent->SetVisibility(false);
	}
}

void UTP_PickUpComponent::BeginPlay()
{
	Super::BeginPlay();
	RefreshPickupState();
	// Register our Overlap Event
	OnComponentBeginOverlap.AddDynamic(this, &UTP_PickUpComponent::OnSphereBeginOverlap);
	OnComponentEndOverlap.AddDynamic(this, &UTP_PickUpComponent::OnSphereEndOverlap);

	if (AActor* WeaponActor = GetOwner())
	{
		if (UWidgetComponent* PromptWidget = WeaponActor->FindComponentByClass<UWidgetComponent>())
		{
			PromptWidget->SetVisibility(false);
		}
	}
}

void UTP_PickUpComponent::OnSphereBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!bPickupEnabled)
	{
		return;
	}

	AMyFpsCharacter* Character = Cast<AMyFpsCharacter>(OtherActor);
	if (!Character)
	{
		return;
	}

	Character->SetAvailablePickup(this);

	if (!Character->IsLocallyControlled())
	{
		return;
	}

	AActor* WeaponActor = GetOwner();
	if (!WeaponActor)
	{
		return;
	}

	FText DisplayName = FText::FromString(TEXT("Weapon"));

	if (AMyFpsWeaponPickupActor* WeaponPickupActor = Cast<AMyFpsWeaponPickupActor>(WeaponActor))
	{
		if (UMyFpsWeaponDefinition* WeaponDefinition = WeaponPickupActor->GetWeaponDefinition())
		{
			DisplayName = WeaponDefinition->DisplayName;
		}
	}

	if (UWidgetComponent* PromptWidgetComponent = WeaponActor->FindComponentByClass<UWidgetComponent>())
	{
		PromptWidgetComponent->InitWidget();
		if (UMyFpsPickupPromptWidget* PromptWidget = Cast<UMyFpsPickupPromptWidget>(PromptWidgetComponent->GetUserWidgetObject()))
		{
			const FText PromptText = Character->HasWeaponEquipped()
				? FText::Format(FText::FromString(TEXT("按E替换 {0}")), DisplayName)
				: FText::Format(FText::FromString(TEXT("按E捡起 {0}")), DisplayName);
			PromptWidget->SetPromptText(PromptText);
		}

		PromptWidgetComponent->SetVisibility(true);
	}
}

void UTP_PickUpComponent::OnSphereEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	AMyFpsCharacter* Character = Cast<AMyFpsCharacter>(OtherActor);
	if (!Character)
	{
		return;
	}

	Character->ClearAvailablePickup(this);

	if (!Character->IsLocallyControlled())
	{
		return;
	}

	AActor* WeaponActor = GetOwner();
	if (!WeaponActor)
	{
		return;
	}

	HidePickupPrompt();
}

void UTP_PickUpComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UTP_PickUpComponent, bPickupEnabled);
}
