// Copyright Epic Games, Inc. All Rights Reserved.

#include "MyFpsWeaponDefinition.h"

const FPrimaryAssetType UMyFpsWeaponDefinition::WeaponDefinitionAssetType = TEXT("WeaponDefinition");

FPrimaryAssetId UMyFpsWeaponDefinition::GetPrimaryAssetId() const
{
	const FName AssetName = WeaponId.IsNone() ? GetFName() : WeaponId;
	return FPrimaryAssetId(WeaponDefinitionAssetType, AssetName);
}
