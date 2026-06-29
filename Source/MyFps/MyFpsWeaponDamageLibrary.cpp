// Copyright Epic Games, Inc. All Rights Reserved.

#include "MyFpsWeaponDamageLibrary.h"

#include "MyFpsWeaponDefinition.h"

float UMyFpsWeaponDamageLibrary::CalculateDamageForHit(
	const UMyFpsWeaponDefinition* WeaponDefinition,
	const FHitResult& HitResult,
	const FVector& DamageStartLocation)
{
	if (!WeaponDefinition)
	{
		return 0.0f;
	}

	const float BodyPartMultiplier = GetBodyPartDamageMultiplier(WeaponDefinition, HitResult);
	const float HitDistance = FVector::Dist(DamageStartLocation, HitResult.ImpactPoint);
	const float RangeMultiplier = GetRangeDamageMultiplier(WeaponDefinition, HitDistance);

	return FMath::Max(0.0f, WeaponDefinition->Damage * BodyPartMultiplier * RangeMultiplier);
}

float UMyFpsWeaponDamageLibrary::GetBodyPartDamageMultiplier(
	const UMyFpsWeaponDefinition* WeaponDefinition,
	const FHitResult& HitResult)
{
	if (!WeaponDefinition)
	{
		return 1.0f;
	}

	const UPhysicalMaterial* HitPhysicalMaterial = HitResult.PhysMaterial.Get();
	if (!HitPhysicalMaterial)
	{
		return 1.0f;
	}

	for (const FMyFpsHitZoneMaterialRule& HitZoneRule : WeaponDefinition->HitZoneRules)
	{
		if (HitZoneRule.PhysicalMaterials.Contains(HitPhysicalMaterial))
		{
			return FMath::Max(0.0f, HitZoneRule.DamageMultiplier);
		}
	}

	return 1.0f;
}

float UMyFpsWeaponDamageLibrary::GetRangeDamageMultiplier(
	const UMyFpsWeaponDefinition* WeaponDefinition,
	float HitDistance)
{
	if (!WeaponDefinition)
	{
		return 1.0f;
	}

	const float CloseRangeEnd = FMath::Max(0.0f, WeaponDefinition->CloseRangeEnd);
	const float MidRangeEnd = FMath::Max(CloseRangeEnd, WeaponDefinition->MidRangeEnd);
	const float LongRangeEnd = WeaponDefinition->LongRangeEnd > 0.0f
		? FMath::Max(MidRangeEnd, WeaponDefinition->LongRangeEnd)
		: FMath::Max(MidRangeEnd, WeaponDefinition->GetTraceDistance());

	if (HitDistance <= CloseRangeEnd)
	{
		return FMath::Max(0.0f, WeaponDefinition->CloseRangeDamageMultiplier);
	}

	if (HitDistance <= MidRangeEnd)
	{
		return FMath::Max(0.0f, WeaponDefinition->MidRangeDamageMultiplier);
	}

	if (HitDistance <= LongRangeEnd)
	{
		return FMath::Max(0.0f, WeaponDefinition->LongRangeDamageMultiplier);
	}

	return 0.0f;
}
