// Copyright Epic Games, Inc. All Rights Reserved.

#include "MyFpsWeaponDefinition.h"

const FPrimaryAssetType UMyFpsWeaponDefinition::WeaponDefinitionAssetType = TEXT("WeaponDefinition");

FPrimaryAssetId UMyFpsWeaponDefinition::GetPrimaryAssetId() const
{
	const FName AssetName = WeaponId.IsNone() ? GetFName() : WeaponId;
	return FPrimaryAssetId(WeaponDefinitionAssetType, AssetName);
}

float UMyFpsWeaponDefinition::GetFireInterval() const
{
	return FireRateRPM > 0.0f
		? 60.0f / FireRateRPM
		: FMath::Max(0.01f, FireInterval);
}

float UMyFpsWeaponDefinition::GetTraceDistance() const
{
	return MaxRange > 0.0f
		? MaxRange
		: FMath::Max(0.0f, HitscanDistance);
}
