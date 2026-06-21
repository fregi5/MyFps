#include "MyFpsPickupPromptWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"

void UMyFpsPickupPromptWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!PickupTextBlock && WidgetTree)
	{
		PickupTextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("PickupTextBlock"));
		if (PickupTextBlock)
		{
			PickupTextBlock->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.92f, 0.35f, 1.0f)));
			PickupTextBlock->SetShadowOffset(FVector2D(1.0f, 1.0f));
			WidgetTree->RootWidget = PickupTextBlock;
		}
	}

	RefreshPromptText();
}

void UMyFpsPickupPromptWidget::SetWeaponName(const FText& InWeaponName)
{
	CachedWeaponName = InWeaponName;
	RefreshPromptText();
}

void UMyFpsPickupPromptWidget::SetPromptText(const FText& InPromptText)
{
	CachedPromptText = InPromptText;

	if (PickupTextBlock)
	{
		PickupTextBlock->SetText(CachedPromptText);
	}

	OnPickupPromptUpdated();
}

void UMyFpsPickupPromptWidget::SetPickupText(const FText& WeaponName)
{
	UE_LOG(LogTemp, Warning, TEXT("[PickupPrompt] SetPickupText called. WeaponName = %s"),
		*WeaponName.ToString());

	if (!PickupTextBlock)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PickupPrompt] PickupTextBlock is null."));
		return;
	}

	const FText PromptText = FText::Format(
		FText::FromString(TEXT("PRESS E to PICKUP {0} ")),
		WeaponName
	);
	
	PickupTextBlock->SetText(PromptText);
	CachedWeaponName = WeaponName;
	RefreshPromptText();
	UE_LOG(LogTemp, Warning, TEXT("[PickupPrompt] Final Text = %s"),
		*PromptText.ToString());
}

void UMyFpsPickupPromptWidget::RefreshPromptText()
{
	CachedPromptText = CachedWeaponName.IsEmpty()
		? PickupPrefix
		: FText::Format(
			FText::FromString(TEXT("PRESS E to PICKUP {0} ")),
			CachedWeaponName
		);

	if (PickupTextBlock)
	{
		PickupTextBlock->SetText(CachedPromptText);
	}

	OnPickupPromptUpdated();
}
