#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PinkCabCockpitInteractionComponent.generated.h"

struct FPinkCabInteractionEvent;

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

    void SetCandidateTarget(FName TargetId) { CandidateTargetId = TargetId; }
    bool CommitAttention();
    FName GetAttentionTargetId() const { return AttentionTargetId; }

    bool BeginGo(FPinkCabInteractionEvent& OutEvent);
    bool EndGo(FPinkCabInteractionEvent& OutEvent);
    bool BuildWheelEvent(int32 SignedSteps, FPinkCabInteractionEvent& OutEvent) const;

private:
    struct FQuickSlotState
    {
        bool bHeld = false;
        uint32 PressSerial = 0;
    };

    static FName TargetForQuickSlot(int32 Slot);

    TArray<FQuickSlotState> QuickSlots;
    uint32 NextPressSerial = 1;
    FName CandidateTargetId = NAME_None;
    FName AttentionTargetId = NAME_None;
    FName ActiveGoTargetId = NAME_None;
    bool bGazeHeld = false;
};
