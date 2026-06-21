#include "MyFpsScoreboardWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Components/BorderSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "MyFpsPlayerState.h"


void UMyFpsScoreboardWidget::NativeConstruct()
{
	Super::NativeConstruct();
		UE_LOG(LogTemp, Warning, TEXT("[Scoreboard] NativeConstruct called."));

	RefreshScoreboard();
}
void UMyFpsScoreboardWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (GetVisibility() != ESlateVisibility::Visible)
	{
		return;
	}

	ScoreboardRefreshTimer += InDeltaTime;

	if (ScoreboardRefreshTimer >= ScoreboardRefreshInterval)
	{
		ScoreboardRefreshTimer = 0.0f;
		RefreshScoreboard();
	}
}


void UMyFpsScoreboardWidget::RefreshScoreboard()
{
	UE_LOG(LogTemp, Warning, TEXT("[Scoreboard] RefreshScoreboard called."));

	CachedEntries.Reset();

	APlayerController* PlayerController = GetOwningPlayer();
	if (!PlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Scoreboard] Owning PlayerController is null."));
		return;
	}

	UWorld* World = PlayerController->GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Scoreboard] World is null."));
		return;
	}

	AGameStateBase* GameState = World->GetGameState();
	if (!GameState)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Scoreboard] GameState is null."));
		return;
	}

	for (APlayerState* PlayerState : GameState->PlayerArray)
	{
		if (!PlayerState)
		{
			continue;
		}

		FMyFpsScoreboardEntry Entry;

		Entry.PlayerName = PlayerState->GetPlayerName().IsEmpty()
			? TEXT("Player")
			: PlayerState->GetPlayerName();

		Entry.bIsLocalPlayer = PlayerState == PlayerController->PlayerState;

		if (const AMyFpsPlayerState* MyPlayerState = Cast<AMyFpsPlayerState>(PlayerState))
		{
			Entry.Score = MyPlayerState->GetCurrentScore();
			Entry.Kills = MyPlayerState->GetKills();
			Entry.Deaths = MyPlayerState->GetDeaths();
		}
		else
		{
			Entry.Score = PlayerState->GetScore();
			Entry.Kills = 0;
			Entry.Deaths = 0;

			UE_LOG(
				LogTemp,
				Warning,
				TEXT("[Scoreboard] PlayerState is not AMyFpsPlayerState: %s"),
				*PlayerState->GetClass()->GetName()
			);
		}

		CachedEntries.Add(Entry);
	}

	CachedEntries.Sort([](const FMyFpsScoreboardEntry& A, const FMyFpsScoreboardEntry& B)
	{
		return A.Score > B.Score;
	});

	for (int32 Index = 0; Index < CachedEntries.Num(); ++Index)
	{
		CachedEntries[Index].Rank = Index + 1;
	}

	if (TitleTextBlock)
	{
		TitleTextBlock->SetText(TitleText);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Scoreboard] TitleTextBlock is null."));
	}

	if (RankHeaderTextBlock)
	{
		RankHeaderTextBlock->SetText(RankColumnText);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Scoreboard] RankHeaderTextBlock is null."));
	}

	if (PlayerHeaderTextBlock)
	{
		PlayerHeaderTextBlock->SetText(PlayerColumnText);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Scoreboard] PlayerHeaderTextBlock is null."));
	}

	if (ScoreHeaderTextBlock)
	{
		ScoreHeaderTextBlock->SetText(ScoreColumnText);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Scoreboard] ScoreHeaderTextBlock is null."));
	}

	if (KillsHeaderTextBlock)
	{
		KillsHeaderTextBlock->SetText(KillsColumnText);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Scoreboard] KillsHeaderTextBlock is null."));
	}

	if (DeathsHeaderTextBlock)
	{
		DeathsHeaderTextBlock->SetText(DeathsColumnText);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Scoreboard] DeathsHeaderTextBlock is null."));
	}

	if (!EntryListBox)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Scoreboard] EntryListBox is null."));
		return;
	}

	EntryListBox->ClearChildren();

	for (const FMyFpsScoreboardEntry& Entry : CachedEntries)
	{
		UHorizontalBox* RowBox = NewObject<UHorizontalBox>(this);
		if (!RowBox)
		{
			continue;
		}

		const FLinearColor RowColor = Entry.bIsLocalPlayer
			? FLinearColor(1.0f, 0.85f, 0.2f, 1.0f)
			: FLinearColor::White;

auto AddTextToRow = [&](UHorizontalBox* RowBox, const FString& Text, float Width, const FLinearColor& Color)
{
	if (!RowBox)
	{
		return;
	}

	UTextBlock* TextBlock = NewObject<UTextBlock>(this);
	if (!TextBlock)
	{
		return;
	}

	TextBlock->SetText(FText::FromString(Text));
	TextBlock->SetColorAndOpacity(FSlateColor(Color));
	TextBlock->SetShadowOffset(FVector2D(1.0f, 1.0f));
	TextBlock->SetAutoWrapText(false);
	TextBlock->SetClipping(EWidgetClipping::ClipToBounds);

	if (UHorizontalBoxSlot* Slot = RowBox->AddChildToHorizontalBox(TextBlock))
	{
		Slot->SetPadding(FMargin(6.0f, 3.0f));

		FSlateChildSize SlotSize;
		SlotSize.SizeRule = ESlateSizeRule::Fill;
		SlotSize.Value = Width;
		Slot->SetSize(SlotSize);

		Slot->SetHorizontalAlignment(HAlign_Left);
		Slot->SetVerticalAlignment(VAlign_Center);
	}
};



AddTextToRow(RowBox, FString::Printf(TEXT("%d"), Entry.Rank), 0.5f, RowColor);
AddTextToRow(RowBox, Entry.PlayerName.Left(16), 2.5f, RowColor);
AddTextToRow(RowBox, FString::Printf(TEXT("%.0f"), Entry.Score), 1.5f, RowColor);
AddTextToRow(RowBox, FString::Printf(TEXT("%d"), Entry.Kills), 0.5f, RowColor);
AddTextToRow(RowBox, FString::Printf(TEXT("%d"), Entry.Deaths), 0.5f, RowColor);

if (UVerticalBoxSlot* RowSlot = EntryListBox->AddChildToVerticalBox(RowBox))
{
	RowSlot->SetHorizontalAlignment(HAlign_Fill);
	RowSlot->SetVerticalAlignment(VAlign_Center);
}

		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[Scoreboard] Add row: Rank=%d, Player=%s, Score=%.0f, Kills=%d, Deaths=%d"),
			Entry.Rank,
			*Entry.PlayerName,
			Entry.Score,
			Entry.Kills,
			Entry.Deaths
		);
	}

	OnScoreboardUpdated();
}