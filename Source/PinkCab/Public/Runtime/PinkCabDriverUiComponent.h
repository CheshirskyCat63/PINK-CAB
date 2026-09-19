#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "PinkCabDriverUiComponent.generated.h"

class APlayerController;
class APinkCabChaosTatraPawn;
class SWidget;

enum class EPinkCabGearEngagementResult : uint8;

struct FPinkCabDriverUiState
{
    FName CurrentTargetId = NAME_None;
    FVector2D GearLeverCursor = FVector2D::ZeroVector;
    float SpeedKmh = 0.0f;
    float EngineRpm = 0.0f;
    float Clutch = 0.0f;
    float Brake = 0.0f;
    float Throttle = 0.0f;
    float Handbrake = 0.0f;
    float Steering = 0.0f;
    float Fuel01 = 0.0f;
    float EngineTemperature01 = 0.0f;
    float BrakeTemperature01 = 0.0f;
    float ClutchTemperature01 = 0.0f;
    int32 RequestedGear = 0;
    int32 EngagedGear = 0;
    bool bGearLeverDragging = false;
    bool bEngineRunning = false;
    bool bEngineStalled = false;
    bool bMoving = false;
    bool bRequiresThrottleDose = false;
    bool bParkingHandbrakeLatched = false;
    EPinkCabGearEngagementResult GearResult = static_cast<EPinkCabGearEngagementResult>(0);
};

UCLASS(ClassGroup=(PinkCab), meta=(BlueprintSpawnableComponent))
class PINKCAB_API UPinkCabDriverUiComponent final : public UActorComponent
{
    GENERATED_BODY()

public:
    UPinkCabDriverUiComponent();

    void BeginRuntime(APlayerController* PC);
    void EndRuntime(APlayerController* PC);
    void UpdateState(const FPinkCabDriverUiState& InState) { State = InState; }
    void EnsurePlayableHudMounted();

    void SetSystemMenuOpen(APlayerController& PC, bool bOpen);
    bool IsSystemMenuOpen() const { return bSystemMenuOpen; }

    void SetPointerCapture(APlayerController& PC, bool bCaptured);
    void ResetTransientInput(APlayerController& PC);
    bool IsPointerCaptured() const { return bPointerCaptured; }

private:
    void MountPlayableHud();
    void UnmountPlayableHud();
    TSharedRef<SWidget> BuildTelemetryPanel();
    FText BuildPromptText() const;
    FText BuildSpeedText() const;
    FText BuildGearText() const;
    FText BuildRpmText() const;
    FText BuildEngineStatusText() const;
    FText BuildPedalText() const;
    FText BuildBrakeText() const;
    FText BuildWarningsText() const;
    void MountSystemMenu();
    void UnmountSystemMenu();
    void ApplyGameplayInputMode(APlayerController& PC);
    void ApplySystemMenuInputMode(APlayerController& PC);
    void HandleApplicationWillDeactivate();
    APinkCabChaosTatraPawn* GetTaxiPawn() const;

    FPinkCabDriverUiState State;
    TSharedPtr<SWidget> PlayableHudOverlay;
    TSharedPtr<SWidget> SystemMenuOverlay;
    FDelegateHandle ApplicationWillDeactivateHandle;
    bool bSystemMenuOpen = false;
    bool bSystemMenuSettingsOpen = false;
    bool bPointerCaptured = false;
};
