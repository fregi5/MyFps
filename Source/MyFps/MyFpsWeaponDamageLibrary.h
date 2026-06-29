// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MyFpsWeaponDamageLibrary.generated.h"

class UMyFpsWeaponDefinition;

UCLASS()
class MYFPS_API UMyFpsWeaponDamageLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Weapon|Damage")
	static float CalculateDamageForHit(const UMyFpsWeaponDefinition* WeaponDefinition, const FHitResult& HitResult, const FVector& DamageStartLocation);

	UFUNCTION(BlueprintPure, Category = "Weapon|Damage")
	static float GetBodyPartDamageMultiplier(const UMyFpsWeaponDefinition* WeaponDefinition, const FHitResult& HitResult);

	UFUNCTION(BlueprintPure, Category = "Weapon|Damage")
	static float GetRangeDamageMultiplier(const UMyFpsWeaponDefinition* WeaponDefinition, float HitDistance);
};
