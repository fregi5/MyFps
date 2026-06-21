// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MyFpsDamageFeedbackWidget.generated.h"

class UImage;
class UMaterialInstanceDynamic;

UCLASS()
class MYFPS_API UMyFpsDamageFeedbackWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Damage Feedback")
	void PlayDamageFeedback(const FVector& SourceWorldLocation);

protected:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> DamageRing = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage Feedback")
	float DisplayDuration = 0.6f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage Feedback", meta = (ClampMin = "1", ClampMax = "4"))
	int32 MaxSimultaneousSources = 4;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage Feedback", meta = (ClampMin = "0"))
	float SourceMergeDistance = 100.0f;

private:
	struct FActiveDamageSource
	{
		FVector SourceWorldLocation = FVector::ZeroVector;
		float StartTime = 0.0f;
	};

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	void HideDamageFeedback();
	bool UpdateRingMaterial();
	float CalculateDamageAngle(const FVector& SourceWorldLocation) const;

	TObjectPtr<UMaterialInstanceDynamic> DamageRingMaterial = nullptr;
	TArray<FActiveDamageSource> ActiveDamageSources;
};
