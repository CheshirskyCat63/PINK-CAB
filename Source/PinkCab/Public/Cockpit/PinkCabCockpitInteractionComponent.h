#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interaction/PinkCabInteractionModel.h"
#include "PinkCabCockpitInteractionComponent.generated.h"

class UPinkCabCockpitAssemblyComponent;

struct FPinkCabCockpitInteractionFrame
{
    bool bGazeHeld = false;
    bool bQuickRecall1Held = false;
    bool bQuickRecall2Held = false;
    bool bQuickRecall3Held = false;
    bool bQuickRecall4Held = false;
    bool bGearboxStageFromClutchHeld = false;
    bool bGripHeld = false;
    bool bMomentaryHeld = false;
    int32 WheelSteps = 0;
    double NowSeconds = 0.0;
    FVector GazeOrigin = FVector::ZeroVector;
    FVector GazeForward = FVector::ForwardVector;
    float GazeMaxDistanceCm = 250.0f;
    int32 GazeCandidateBudget = 22;
};

UCLASS(ClassGroup=(PinkCab), meta=(BlueprintSpawnableComponent))
class PINKCAB_API UPinkCabCockpitInteractionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPinkCabCockpitInteractionComponent();

    void SetQuickSlotHeld(int32 Slot, bool bHeld);
    FName GetCurrentQuickTargetId() const;
    void SetGazeHeld(bool bHeld) { bGazeHeld = bHeld; }
    bool IsGazeHeld() const { return bGazeHeld; }

    void SetCurrentTarget(const FPinkCabInteractionControlSpec& Spec);
    FName GetCurrentTargetId() const;
    uint32 GetActuationSerial() const { return ActuationSerial; }

    bool BeginGrip(FPinkCabInteractionEvent& OutEvent);
    bool EndGrip(FPinkCabInteractionEvent& OutEvent);
    bool IsGripActive() const { return bGripActive; }
    FName GetActiveGripTargetId() const { return ActiveGripTargetId; }
    bool IsManipulationActive() const { return bManipulationActive; }
    FName GetActiveManipulationTargetId() const { return ActiveManipulationTargetId; }
    bool IsGearboxStageFromClutchHeld() const { return bGearboxStageFromClutchHeld; }

    bool BeginMomentary(double NowSeconds, FPinkCabInteractionEvent& OutEvent);
    bool EndMomentary(double NowSeconds, FPinkCabInteractionEvent& OutEvent);
    bool IsMomentaryActive() const { return bMomentaryActive; }
    double GetLastMomentaryHoldSeconds() const { return LastMomentaryHoldSeconds; }

    bool BuildWheelEvent(int32 SignedSteps, FPinkCabInteractionEvent& OutEvent);
    void ProcessFrame(
        const FPinkCabCockpitInteractionFrame& Frame,
        const UPinkCabCockpitAssemblyComponent* Assembly,
        TArray<FPinkCabInteractionEvent>& OutActuationEvents);
    void ResetTransientInputState(TArray<FPinkCabInteractionEvent>* OutReleaseEvents = nullptr);

    static FPinkCabInteractionControlSpec SpecForTargetId(FName TargetId);

private:
    struct FQuickSlotState { bool bHeld = false; uint32 PressSerial = 0; };
    static FName TargetForQuickSlot(int32 Slot);
    static bool IsLeverManipulationTarget(FName TargetId);
    FPinkCabInteractionControlSpec ResolveActiveSpec() const;
    void MarkRecalledTargetConsumed(FName TargetId);
    void ClearConsumedRecallIfIdle();
    bool TrySelectContextualTarget(
        const FPinkCabCockpitInteractionFrame& Frame,
        const UPinkCabCockpitAssemblyComponent* Assembly);
    void UpdateTargetSelection(
        const FPinkCabCockpitInteractionFrame& Frame,
        const UPinkCabCockpitAssemblyComponent* Assembly);
    void UpdateGripState(
        const FPinkCabCockpitInteractionFrame& Frame,
        TArray<FPinkCabInteractionEvent>& OutActuationEvents);
    void UpdateManipulationState(const FPinkCabCockpitInteractionFrame& Frame);
    void UpdateMomentaryState(
        const FPinkCabCockpitInteractionFrame& Frame,
        TArray<FPinkCabInteractionEvent>& OutActuationEvents);
    void AppendWheelEvent(
        const FPinkCabCockpitInteractionFrame& Frame,
        TArray<FPinkCabInteractionEvent>& OutActuationEvents);

    TArray<FQuickSlotState> QuickSlots;
    uint32 NextPressSerial = 1;
    uint32 ActuationSerial = 0;
    FPinkCabInteractionControlSpec CurrentTarget;
    FName ActiveGripTargetId = NAME_None;
    FName ActiveManipulationTargetId = NAME_None;
    FName ActiveMomentaryTargetId = NAME_None;
    double MomentaryStartSeconds = 0.0;
    double LastMomentaryHoldSeconds = 0.0;
    bool bGazeHeld = false;
    bool bGripActive = false;
    bool bManipulationActive = false;
    bool bMomentaryActive = false;
    bool bCurrentTargetFromQuickRecall = false;
    bool bCurrentTargetRecallConsumed = false;
    bool bGearboxStageFromClutchHeld = false;
};
