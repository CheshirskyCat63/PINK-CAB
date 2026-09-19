#include "Runtime/PinkCabDriverUiComponent.h"

#include "Cockpit/PinkCabCockpitVisualDriverComponent.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Misc/CoreDelegates.h"
#include "Styling/CoreStyle.h"
#include "Runtime/PinkCabChaosTatraPawn.h"
#include "Vehicle/PinkCabGearboxController.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
FString GearLabel(const int32 Gear)
{
    return Gear < 0 ? TEXT("R") : (Gear == 0 ? TEXT("N") : FString::FromInt(Gear));
}

const TCHAR* EngagementLabel(const EPinkCabGearEngagementResult Result)
{
    switch (Result)
    {
    case EPinkCabGearEngagementResult::Neutral: return TEXT("NEUTRAL");
    case EPinkCabGearEngagementResult::ClutchDisengagedAccepted: return TEXT("CLUTCH");
    case EPinkCabGearEngagementResult::MatchedClutchlessAccepted: return TEXT("MATCH");
    case EPinkCabGearEngagementResult::GrindRefused: return TEXT("GRIND");
    case EPinkCabGearEngagementResult::ReverseLockout: return TEXT("R-LOCK");
    case EPinkCabGearEngagementResult::DangerousOverrev: return TEXT("OVERREV");
    default: return TEXT("NONE");
    }
}

FString GaugeBar(const float Value, const int32 Segments = 10)
{
    const int32 Filled = FMath::Clamp(
        FMath::RoundToInt(FMath::Clamp(Value, 0.0f, 1.0f) * Segments), 0, Segments);
    FString Result(TEXT("["));
    for (int32 Index = 0; Index < Segments; ++Index)
    {
        Result.AppendChar(Index < Filled ? TEXT('#') : TEXT('-'));
    }
    Result.AppendChar(TEXT(']'));
    return Result;
}
}

UPinkCabDriverUiComponent::UPinkCabDriverUiComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

APinkCabChaosTatraPawn* UPinkCabDriverUiComponent::GetTaxiPawn() const
{
    return Cast<APinkCabChaosTatraPawn>(GetOwner());
}

void UPinkCabDriverUiComponent::BeginRuntime(APlayerController* PC)
{
    if (PC)
    {
        ApplyGameplayInputMode(*PC);
    }
    MountPlayableHud();
    if (!ApplicationWillDeactivateHandle.IsValid())
    {
        ApplicationWillDeactivateHandle = FCoreDelegates::ApplicationWillDeactivateDelegate.AddUObject(
            this, &UPinkCabDriverUiComponent::HandleApplicationWillDeactivate);
    }
}

void UPinkCabDriverUiComponent::EndRuntime(APlayerController* PC)
{
    if (ApplicationWillDeactivateHandle.IsValid())
    {
        FCoreDelegates::ApplicationWillDeactivateDelegate.Remove(ApplicationWillDeactivateHandle);
        ApplicationWillDeactivateHandle.Reset();
    }
    if (PC)
    {
        ResetTransientInput(*PC);
    }
    UnmountSystemMenu();
    UnmountPlayableHud();
    bSystemMenuOpen = false;
    bSystemMenuSettingsOpen = false;
}

void UPinkCabDriverUiComponent::EnsurePlayableHudMounted()
{
    MountPlayableHud();
}

void UPinkCabDriverUiComponent::ApplyGameplayInputMode(APlayerController& PC)
{
    FInputModeGameOnly InputMode;
    InputMode.SetConsumeCaptureMouseDown(false);
    PC.SetInputMode(InputMode);
    PC.bShowMouseCursor = false;
    if (GEngine && GEngine->GameViewport)
    {
        GEngine->GameViewport->SetMouseCaptureMode(EMouseCaptureMode::CapturePermanently_IncludingInitialMouseDown);
        GEngine->GameViewport->SetMouseLockMode(EMouseLockMode::LockOnCapture);
        GEngine->GameViewport->SetHideCursorDuringCapture(true);
    }
}

void UPinkCabDriverUiComponent::ApplySystemMenuInputMode(APlayerController& PC)
{
    FInputModeGameAndUI InputMode;
    InputMode.SetHideCursorDuringCapture(false);
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    if (SystemMenuOverlay.IsValid())
    {
        InputMode.SetWidgetToFocus(SystemMenuOverlay);
    }
    PC.SetInputMode(InputMode);
    PC.bShowMouseCursor = true;
    if (GEngine && GEngine->GameViewport)
    {
        GEngine->GameViewport->SetMouseCaptureMode(EMouseCaptureMode::NoCapture);
        GEngine->GameViewport->SetMouseLockMode(EMouseLockMode::DoNotLock);
        GEngine->GameViewport->SetHideCursorDuringCapture(false);
    }
}

void UPinkCabDriverUiComponent::SetPointerCapture(APlayerController& PC, const bool bCaptured)
{
    if (bPointerCaptured == bCaptured)
    {
        return;
    }
    bPointerCaptured = bCaptured;
    FInputModeGameOnly InputMode;
    InputMode.SetConsumeCaptureMouseDown(false);
    PC.SetInputMode(InputMode);
    PC.bShowMouseCursor = false;
    if (GEngine && GEngine->GameViewport)
    {
        GEngine->GameViewport->SetMouseCaptureMode(EMouseCaptureMode::CapturePermanently_IncludingInitialMouseDown);
        GEngine->GameViewport->SetMouseLockMode(bCaptured ? EMouseLockMode::LockAlways : EMouseLockMode::LockOnCapture);
        GEngine->GameViewport->SetHideCursorDuringCapture(true);
    }
}

void UPinkCabDriverUiComponent::ResetTransientInput(APlayerController& PC)
{
    SetPointerCapture(PC, false);
    if (!bSystemMenuOpen)
    {
        ApplyGameplayInputMode(PC);
    }
}

void UPinkCabDriverUiComponent::HandleApplicationWillDeactivate()
{
    if (APinkCabChaosTatraPawn* Pawn = GetTaxiPawn())
    {
        Pawn->ResetTransientCockpitInput();
    }
}

FText UPinkCabDriverUiComponent::BuildPromptText() const
{
    if (State.bGearLeverDragging)
    {
        const int32 PreviewGear =
            UPinkCabCockpitVisualDriverComponent::GearForCursor(State.GearLeverCursor);
        return FText::FromString(FString::Printf(TEXT("GEAR -> %s"), *GearLabel(PreviewGear)));
    }
    return State.CurrentTargetId.IsNone() ? FText::GetEmpty() : FText::FromName(State.CurrentTargetId);
}

FText UPinkCabDriverUiComponent::BuildSpeedText() const
{
    return FText::FromString(FString::Printf(
        TEXT("%03d  km/h"), FMath::RoundToInt(FMath::Abs(State.SpeedKmh))));
}

FText UPinkCabDriverUiComponent::BuildGearText() const
{
    return FText::FromString(GearLabel(State.EngagedGear));
}

FText UPinkCabDriverUiComponent::BuildRpmText() const
{
    return FText::FromString(FString::Printf(
        TEXT("%04d"), FMath::RoundToInt(FMath::Max(State.EngineRpm, 0.0f))));
}

FText UPinkCabDriverUiComponent::BuildEngineStatusText() const
{
    const TCHAR* Engine = State.bEngineRunning
        ? TEXT("ENGINE ON")
        : (State.bEngineStalled ? TEXT("ENGINE STALLED") : TEXT("ENGINE OFF"));
    return FText::FromString(FString::Printf(
        TEXT("%s   |   %s   |   FUEL %3d%%   |   TEMP %3d%%"),
        Engine,
        State.bMoving ? TEXT("MOVING") : TEXT("STATIONARY"),
        FMath::RoundToInt(State.Fuel01 * 100.0f),
        FMath::RoundToInt(State.EngineTemperature01 * 100.0f)));
}

FText UPinkCabDriverUiComponent::BuildPedalText() const
{
    return FText::FromString(FString::Printf(
        TEXT("CLUTCH %s %3d%%    GAS %s %3d%%"),
        *GaugeBar(State.Clutch), FMath::RoundToInt(State.Clutch * 100.0f),
        *GaugeBar(State.Throttle), FMath::RoundToInt(State.Throttle * 100.0f)));
}

FText UPinkCabDriverUiComponent::BuildBrakeText() const
{
    return FText::FromString(FString::Printf(
        TEXT("BRAKE  %s %3d%%    H-BRAKE %s %3d%%"),
        *GaugeBar(State.Brake), FMath::RoundToInt(State.Brake * 100.0f),
        *GaugeBar(State.Handbrake), FMath::RoundToInt(State.Handbrake * 100.0f)));
}

FText UPinkCabDriverUiComponent::BuildWarningsText() const
{
    TArray<FString> Warnings;
    Warnings.Add(FMath::Abs(State.Steering) < 0.02f
        ? TEXT("STEER CENTER")
        : FString::Printf(
            TEXT("STEER %s %d%%"),
            State.Steering > 0.0f ? TEXT("R") : TEXT("L"),
            FMath::RoundToInt(FMath::Abs(State.Steering) * 100.0f)));
    Warnings.Add(FString::Printf(
        TEXT("REQ %s / ENG %s"), *GearLabel(State.RequestedGear), *GearLabel(State.EngagedGear)));
    if (State.bEngineStalled) Warnings.Add(TEXT("STALL"));
    if (State.bRequiresThrottleDose) Warnings.Add(TEXT("SET THROTTLE"));
    if (State.Handbrake > 0.02f) Warnings.Add(TEXT("HANDBRAKE"));
    if (State.BrakeTemperature01 > 0.75f) Warnings.Add(TEXT("BRAKES HOT"));
    if (State.ClutchTemperature01 > 0.75f) Warnings.Add(TEXT("CLUTCH HOT"));
    if (State.GearResult == EPinkCabGearEngagementResult::GrindRefused
        || State.GearResult == EPinkCabGearEngagementResult::ReverseLockout
        || State.GearResult == EPinkCabGearEngagementResult::DangerousOverrev)
    {
        Warnings.Add(EngagementLabel(State.GearResult));
    }
    if (State.bParkingHandbrakeLatched) Warnings.Add(TEXT("PARK LATCH"));
    return FText::FromString(FString::Join(Warnings, TEXT("   |   ")));
}

TSharedRef<SWidget> UPinkCabDriverUiComponent::BuildTelemetryPanel()
{
    TWeakObjectPtr<UPinkCabDriverUiComponent> WeakThis(this);
    return SNew(SBorder).Padding(FMargin(16,12)).BorderBackgroundColor(FLinearColor(0.015f,0.018f,0.022f,0.88f))
        [ SNew(SVerticalBox)
          + SVerticalBox::Slot().AutoHeight()
          [ SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().Padding(0,0,28,0)
            [ SNew(SVerticalBox)
              + SVerticalBox::Slot().AutoHeight()[ SNew(STextBlock).Font(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"),9)).ColorAndOpacity(FLinearColor(1,1,1,0.62f)).Text(FText::FromString(TEXT("SPEED"))) ]
              + SVerticalBox::Slot().AutoHeight()[ SNew(STextBlock).Font(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"),30)).ColorAndOpacity(FLinearColor::White).Text_Lambda([WeakThis]() { return WeakThis.IsValid() ? WeakThis->BuildSpeedText() : FText::GetEmpty(); }) ] ]
            + SHorizontalBox::Slot().AutoWidth().Padding(0,0,28,0)
            [ SNew(SVerticalBox)
              + SVerticalBox::Slot().AutoHeight()[ SNew(STextBlock).Font(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"),9)).ColorAndOpacity(FLinearColor(1,1,1,0.62f)).Text(FText::FromString(TEXT("GEAR"))) ]
              + SVerticalBox::Slot().AutoHeight()[ SNew(STextBlock).Font(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"),30)).ColorAndOpacity(FLinearColor(0.92f,0.95f,1.0f,1.0f)).Text_Lambda([WeakThis]() { return WeakThis.IsValid() ? WeakThis->BuildGearText() : FText::GetEmpty(); }) ] ]
            + SHorizontalBox::Slot().AutoWidth()
            [ SNew(SVerticalBox)
              + SVerticalBox::Slot().AutoHeight()[ SNew(STextBlock).Font(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"),9)).ColorAndOpacity(FLinearColor(1,1,1,0.62f)).Text(FText::FromString(TEXT("ENGINE RPM"))) ]
              + SVerticalBox::Slot().AutoHeight()[ SNew(STextBlock).Font(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"),24)).ColorAndOpacity(FLinearColor::White).Text_Lambda([WeakThis]() { return WeakThis.IsValid() ? WeakThis->BuildRpmText() : FText::GetEmpty(); }) ] ] ]
          + SVerticalBox::Slot().AutoHeight().Padding(0,6,0,0)[ SNew(STextBlock).Font(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"),11)).ColorAndOpacity(FLinearColor(0.88f,0.92f,0.96f,1.0f)).Text_Lambda([WeakThis]() { return WeakThis.IsValid() ? WeakThis->BuildEngineStatusText() : FText::GetEmpty(); }) ]
          + SVerticalBox::Slot().AutoHeight().Padding(0,5,0,0)[ SNew(STextBlock).Font(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"),11)).ColorAndOpacity(FLinearColor(0.82f,0.9f,1.0f,1.0f)).Text_Lambda([WeakThis]() { return WeakThis.IsValid() ? WeakThis->BuildPedalText() : FText::GetEmpty(); }) ]
          + SVerticalBox::Slot().AutoHeight().Padding(0,3,0,0)[ SNew(STextBlock).Font(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"),11)).ColorAndOpacity(FLinearColor(0.82f,0.9f,1.0f,1.0f)).Text_Lambda([WeakThis]() { return WeakThis.IsValid() ? WeakThis->BuildBrakeText() : FText::GetEmpty(); }) ]
          + SVerticalBox::Slot().AutoHeight().Padding(0,5,0,0)[ SNew(STextBlock).Font(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"),10)).ColorAndOpacity(FLinearColor(1.0f,0.78f,0.30f,1.0f)).Text_Lambda([WeakThis]() { return WeakThis.IsValid() ? WeakThis->BuildWarningsText() : FText::GetEmpty(); }) ]
          + SVerticalBox::Slot().AutoHeight().Padding(0,7,0,0)[ SNew(STextBlock).Font(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"),9)).ColorAndOpacity(FLinearColor(1,1,1,0.58f)).Text(FText::FromString(TEXT("MOUSE STEER   Q CLUTCH   W BRAKE   E GAS   3 GEARBOX   4 HANDBRAKE   SPACE LOOK"))) ] ];
}

void UPinkCabDriverUiComponent::MountPlayableHud()
{
    if (PlayableHudOverlay.IsValid() || !GEngine || !GEngine->GameViewport) return;
    TWeakObjectPtr<UPinkCabDriverUiComponent> WeakThis(this);
    PlayableHudOverlay = SNew(SOverlay).Visibility(EVisibility::HitTestInvisible)
        + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
        [ SNew(STextBlock).Text(FText::FromString(FString::Chr(0x2022))).Font(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"),22)).ColorAndOpacity(FLinearColor::White) ]
        + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center).Padding(FMargin(0,34,0,0))
        [ SNew(STextBlock).Font(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"),10)).ColorAndOpacity(FLinearColor(1,1,1,0.82f)).Text_Lambda([WeakThis]() { return WeakThis.IsValid() ? WeakThis->BuildPromptText() : FText::GetEmpty(); }) ]
        + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Bottom).Padding(FMargin(20,0,20,28))
        [ BuildTelemetryPanel() ];
    GEngine->GameViewport->AddViewportWidgetContent(PlayableHudOverlay.ToSharedRef(), 1000);
}

void UPinkCabDriverUiComponent::UnmountPlayableHud()
{
    if (!PlayableHudOverlay.IsValid()) return;
    if (GEngine && GEngine->GameViewport)
    {
        GEngine->GameViewport->RemoveViewportWidgetContent(PlayableHudOverlay.ToSharedRef());
    }
    PlayableHudOverlay.Reset();
}

void UPinkCabDriverUiComponent::MountSystemMenu()
{
    if (SystemMenuOverlay.IsValid() || !GEngine || !GEngine->GameViewport) return;
    TWeakObjectPtr<UPinkCabDriverUiComponent> WeakThis(this);
    const FLinearColor Pink(1.0f, 0.05f, 0.42f, 1.0f);
    SystemMenuOverlay = SNew(SOverlay)
        + SOverlay::Slot()[ SNew(SBorder).BorderBackgroundColor(FLinearColor(0.01f, 0.01f, 0.015f, 0.90f)) ]
        + SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Center).Padding(FMargin(96.0f, 0.0f, 0.0f, 0.0f))
        [ SNew(SBorder).Padding(FMargin(30.0f, 26.0f)).BorderBackgroundColor(FLinearColor(0.025f, 0.025f, 0.035f, 0.97f))
          [ SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()[ SNew(STextBlock).Text(FText::FromString(TEXT("PINK CAB"))).Font(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"),42)).ColorAndOpacity(Pink) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0,0,0,24)[ SNew(STextBlock).Text(FText::FromString(TEXT("SYSTEM MENU"))).Font(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"),12)).ColorAndOpacity(FLinearColor(1,1,1,0.62f)) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0,4)[ SNew(SButton).ContentPadding(FMargin(22,10)).ButtonColorAndOpacity(Pink).OnClicked_Lambda([WeakThis]() { if (WeakThis.IsValid()) { if (APinkCabChaosTatraPawn* Pawn = WeakThis->GetTaxiPawn()) Pawn->SetSystemMenuOpen(false); } return FReply::Handled(); })[ SNew(STextBlock).Text(FText::FromString(TEXT("DRIVE"))).Font(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"),18)).ColorAndOpacity(FLinearColor::White) ] ]
            + SVerticalBox::Slot().AutoHeight().Padding(0,4)[ SNew(SButton).ContentPadding(FMargin(22,10)).OnClicked_Lambda([WeakThis]() { if (WeakThis.IsValid()) WeakThis->bSystemMenuSettingsOpen = !WeakThis->bSystemMenuSettingsOpen; return FReply::Handled(); })[ SNew(STextBlock).Text(FText::FromString(TEXT("SETTINGS"))).Font(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"),18)) ] ]
            + SVerticalBox::Slot().AutoHeight().Padding(0,4)[ SNew(SButton).ContentPadding(FMargin(22,10)).OnClicked_Lambda([WeakThis]() { if (WeakThis.IsValid()) { if (APinkCabChaosTatraPawn* Pawn = WeakThis->GetTaxiPawn()) { if (APlayerController* PC = Cast<APlayerController>(Pawn->GetController())) UKismetSystemLibrary::QuitGame(Pawn, PC, EQuitPreference::Quit, false); } } return FReply::Handled(); })[ SNew(STextBlock).Text(FText::FromString(TEXT("QUIT"))).Font(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"),18)) ] ]
            + SVerticalBox::Slot().AutoHeight().Padding(0,18,0,0)[ SNew(SBorder).Padding(FMargin(14,12)).BorderBackgroundColor(FLinearColor(1,1,1,0.055f)).Visibility_Lambda([WeakThis]() { return WeakThis.IsValid() && WeakThis->bSystemMenuSettingsOpen ? EVisibility::Visible : EVisibility::Collapsed; })[ SNew(SVerticalBox) + SVerticalBox::Slot().AutoHeight()[ SNew(STextBlock).Text(FText::FromString(TEXT("SETTINGS / CONTROLS"))).Font(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"),13)).ColorAndOpacity(Pink) ] + SVerticalBox::Slot().AutoHeight().Padding(0,6,0,0)[ SNew(STextBlock).Text(FText::FromString(TEXT("ESC  menu   MOUSE  steer   SPACE  look\nQ clutch   W brake   E throttle\nFull settings panel comes after the playable foundation."))).Font(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"),11)).ColorAndOpacity(FLinearColor(1,1,1,0.78f)) ] ] ]
            + SVerticalBox::Slot().AutoHeight().Padding(0,20,0,0)[ SNew(STextBlock).Text(FText::FromString(TEXT("ESC TO RESUME"))).Font(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"),10)).ColorAndOpacity(FLinearColor(1,1,1,0.45f)) ]
          ] ];
    GEngine->GameViewport->AddViewportWidgetContent(SystemMenuOverlay.ToSharedRef(), 2000);
}

void UPinkCabDriverUiComponent::UnmountSystemMenu()
{
    if (!SystemMenuOverlay.IsValid()) return;
    if (GEngine && GEngine->GameViewport)
    {
        GEngine->GameViewport->RemoveViewportWidgetContent(SystemMenuOverlay.ToSharedRef());
    }
    SystemMenuOverlay.Reset();
}

void UPinkCabDriverUiComponent::SetSystemMenuOpen(APlayerController& PC, const bool bOpen)
{
    if (bSystemMenuOpen == bOpen && SystemMenuOverlay.IsValid() == bOpen)
    {
        return;
    }
    bSystemMenuOpen = bOpen;
    bSystemMenuSettingsOpen = false;
    bPointerCaptured = false;
    if (bOpen)
    {
        MountSystemMenu();
        UGameplayStatics::SetGamePaused(this, true);
        ApplySystemMenuInputMode(PC);
        return;
    }
    UnmountSystemMenu();
    UGameplayStatics::SetGamePaused(this, false);
    ApplyGameplayInputMode(PC);
}
