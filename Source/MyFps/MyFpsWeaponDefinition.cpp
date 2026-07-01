// Copyright Epic Games, Inc. All Rights Reserved.

#include "MyFpsWeaponDefinition.h"

#include "Misc/FileHelper.h"
#include "Serialization/Csv/CsvParser.h"

const FPrimaryAssetType UMyFpsWeaponDefinition::WeaponDefinitionAssetType = TEXT("WeaponDefinition");

namespace MyFpsWeaponDefinitionCsv
{
	FString NormalizeHeader(const FString& Header)
	{
		FString Normalized = Header;
		Normalized.TrimStartAndEndInline();
		Normalized.RemoveSpacesInline();
		return Normalized.ToLower();
	}

	int32 GetColumnIndex(const TArray<const TCHAR*>& Row, const FString& ColumnName)
	{
		const FString NormalizedColumnName = NormalizeHeader(ColumnName);
		for (int32 Index = 0; Index < Row.Num(); ++Index)
		{
			if (NormalizeHeader(Row[Index]) == NormalizedColumnName)
			{
				return Index;
			}
		}

		return INDEX_NONE;
	}

	FString GetCellString(const TArray<const TCHAR*>& Row, int32 ColumnIndex)
	{
		return Row.IsValidIndex(ColumnIndex) ? FString(Row[ColumnIndex]).TrimStartAndEnd() : FString();
	}

	bool TryGetInt(const TArray<const TCHAR*>& Row, int32 ColumnIndex, int32& OutValue)
	{
		const FString CellValue = GetCellString(Row, ColumnIndex);
		if (CellValue.IsEmpty())
		{
			return false;
		}

		OutValue = FCString::Atoi(*CellValue);
		return true;
	}

	float GetFloat(const TArray<const TCHAR*>& Row, int32 ColumnIndex, float DefaultValue = 0.0f)
	{
		const FString CellValue = GetCellString(Row, ColumnIndex);
		return CellValue.IsEmpty() ? DefaultValue : FCString::Atof(*CellValue);
	}
}

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

float UMyFpsWeaponDefinition::GetFireCooldown(bool bIncludeBoltAction) const
{
	const float BaseCooldown = FireCooldown >= 0.0f
		? FireCooldown
		: (bAutomaticFire ? GetFireInterval() : 0.0f);
	if (!bIncludeBoltAction)
	{
		return BaseCooldown;
	}

	const bool bHasBoltAction = BoltActionAnimation != nullptr
		|| ThirdPersonBoltActionAnimation != nullptr
		|| BoltActionTime > 0.0f;
	if (!bHasBoltAction)
	{
		return BaseCooldown;
	}

	return FMath::Max(BaseCooldown, FMath::Max(0.0f, BoltActionDelay) + FMath::Max(0.0f, BoltActionTime));
}

float UMyFpsWeaponDefinition::GetTraceDistance() const
{
	return MaxRange > 0.0f
		? MaxRange
		: FMath::Max(0.0f, HitscanDistance);
}

bool UMyFpsWeaponDefinition::ImportRecoilPatternFromCsv()
{
	return ImportRecoilPatternFromCsvPath(RecoilPatternCsvFile.FilePath);
}

bool UMyFpsWeaponDefinition::ImportRecoilPatternFromCsvPath(const FString& CsvFilePath)
{
	if (CsvFilePath.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[WeaponDefinition] Recoil CSV import failed: file path is empty. Asset=%s"), *GetNameSafe(this));
		return false;
	}

	FString CsvText;
	if (!FFileHelper::LoadFileToString(CsvText, *CsvFilePath))
	{
		UE_LOG(LogTemp, Warning, TEXT("[WeaponDefinition] Recoil CSV import failed: cannot read file. Path=%s Asset=%s"), *CsvFilePath, *GetNameSafe(this));
		return false;
	}

	FCsvParser CsvParser(CsvText);
	const FCsvParser::FRows& Rows = CsvParser.GetRows();
	if (Rows.Num() < 2)
	{
		UE_LOG(LogTemp, Warning, TEXT("[WeaponDefinition] Recoil CSV import failed: CSV has no data rows. Path=%s Asset=%s"), *CsvFilePath, *GetNameSafe(this));
		return false;
	}

	const TArray<const TCHAR*>& HeaderRow = Rows[0];
	const int32 StartShotColumn = MyFpsWeaponDefinitionCsv::GetColumnIndex(HeaderRow, TEXT("StartShot"));
	const int32 EndShotColumn = MyFpsWeaponDefinitionCsv::GetColumnIndex(HeaderRow, TEXT("EndShot"));
	const int32 LegacyShotIndexColumn = MyFpsWeaponDefinitionCsv::GetColumnIndex(HeaderRow, TEXT("ShotIndex"));
	if (StartShotColumn == INDEX_NONE && LegacyShotIndexColumn == INDEX_NONE)
	{
		UE_LOG(LogTemp, Warning, TEXT("[WeaponDefinition] Recoil CSV import failed: missing StartShot column. Path=%s Asset=%s"), *CsvFilePath, *GetNameSafe(this));
		return false;
	}

	const int32 PitchUpColumn = MyFpsWeaponDefinitionCsv::GetColumnIndex(HeaderRow, TEXT("PitchUp"));
	const int32 YawOffsetColumn = MyFpsWeaponDefinitionCsv::GetColumnIndex(HeaderRow, TEXT("YawOffset"));
	const int32 RandomYawMinColumn = MyFpsWeaponDefinitionCsv::GetColumnIndex(HeaderRow, TEXT("RandomYawMin"));
	const int32 RandomYawMaxColumn = MyFpsWeaponDefinitionCsv::GetColumnIndex(HeaderRow, TEXT("RandomYawMax"));
	const int32 SpreadAddColumn = MyFpsWeaponDefinitionCsv::GetColumnIndex(HeaderRow, TEXT("SpreadAdd"));
	const int32 RecoveryDelayColumn = MyFpsWeaponDefinitionCsv::GetColumnIndex(HeaderRow, TEXT("RecoveryDelay"));
	const int32 RecoverySpeedColumn = MyFpsWeaponDefinitionCsv::GetColumnIndex(HeaderRow, TEXT("RecoverySpeed"));
	const int32 CameraShakeScaleColumn = MyFpsWeaponDefinitionCsv::GetColumnIndex(HeaderRow, TEXT("CameraShakeScale"));
	const int32 ViewKickScaleColumn = MyFpsWeaponDefinitionCsv::GetColumnIndex(HeaderRow, TEXT("ViewKickScale"));
	const int32 WeaponKickScaleColumn = MyFpsWeaponDefinitionCsv::GetColumnIndex(HeaderRow, TEXT("WeaponKickScale"));
	const int32 RemarkColumn = MyFpsWeaponDefinitionCsv::GetColumnIndex(HeaderRow, TEXT("Remark"));

	TArray<FMyFpsRecoilPatternShot> ImportedShots;
	for (int32 RowIndex = 1; RowIndex < Rows.Num(); ++RowIndex)
	{
		const TArray<const TCHAR*>& DataRow = Rows[RowIndex];
		int32 StartShot = 0;
		if (StartShotColumn != INDEX_NONE)
		{
			if (!MyFpsWeaponDefinitionCsv::TryGetInt(DataRow, StartShotColumn, StartShot))
			{
				continue;
			}
		}
		else if (!MyFpsWeaponDefinitionCsv::TryGetInt(DataRow, LegacyShotIndexColumn, StartShot))
		{
			continue;
		}

		int32 EndShot = StartShot;
		if (EndShotColumn != INDEX_NONE)
		{
			MyFpsWeaponDefinitionCsv::TryGetInt(DataRow, EndShotColumn, EndShot);
		}

		FMyFpsRecoilPatternShot Shot;
		Shot.StartShot = FMath::Max(1, StartShot);
		Shot.EndShot = FMath::Max(Shot.StartShot, EndShot);
		Shot.PitchUp = MyFpsWeaponDefinitionCsv::GetFloat(DataRow, PitchUpColumn);
		Shot.YawOffset = MyFpsWeaponDefinitionCsv::GetFloat(DataRow, YawOffsetColumn);
		Shot.RandomYawMin = MyFpsWeaponDefinitionCsv::GetFloat(DataRow, RandomYawMinColumn);
		Shot.RandomYawMax = MyFpsWeaponDefinitionCsv::GetFloat(DataRow, RandomYawMaxColumn);
		Shot.SpreadAdd = MyFpsWeaponDefinitionCsv::GetFloat(DataRow, SpreadAddColumn);
		Shot.RecoveryDelay = MyFpsWeaponDefinitionCsv::GetFloat(DataRow, RecoveryDelayColumn, -1.0f);
		Shot.RecoverySpeed = MyFpsWeaponDefinitionCsv::GetFloat(DataRow, RecoverySpeedColumn, -1.0f);
		Shot.CameraShakeScale = MyFpsWeaponDefinitionCsv::GetFloat(DataRow, CameraShakeScaleColumn, 1.0f);
		Shot.ViewKickScale = MyFpsWeaponDefinitionCsv::GetFloat(DataRow, ViewKickScaleColumn, 1.0f);
		Shot.WeaponKickScale = MyFpsWeaponDefinitionCsv::GetFloat(DataRow, WeaponKickScaleColumn, 1.0f);
		Shot.Remark = MyFpsWeaponDefinitionCsv::GetCellString(DataRow, RemarkColumn);
		ImportedShots.Add(Shot);
	}

	ImportedShots.Sort([](const FMyFpsRecoilPatternShot& Left, const FMyFpsRecoilPatternShot& Right)
	{
		if (Left.StartShot == Right.StartShot)
		{
			return Left.EndShot < Right.EndShot;
		}

		return Left.StartShot < Right.StartShot;
	});

	Modify();
	RecoilPatternShots = MoveTemp(ImportedShots);
	RecoilPatternMode = EMyFpsRecoilPatternMode::PerShotArray;
	bUseRecoilPattern = true;
	MarkPackageDirty();

	UE_LOG(LogTemp, Log, TEXT("[WeaponDefinition] Imported %d recoil pattern rows from CSV. Path=%s Asset=%s"), RecoilPatternShots.Num(), *CsvFilePath, *GetNameSafe(this));
	return true;
}
