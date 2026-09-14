#include "Cockpit/PinkCabCockpitInteractionComponent.h"

#include "Interaction/PinkCabInteractionModel.h"

UPinkCabCockpitInteractionComponent::UPinkCabCockpitInteractionComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    QuickSlots.SetNum(4);
}

FName UPinkCabCockpitInteractionComponent::TargetForQuickSlot(const int32 Slot)
{
    switch (Slot)
    {
    case 1: return TEXT("TurnSignals");
    case 2: return TEXT("Horn");
    case 3: return TEXT("Gearbox");
    case 4: return TEXT("Handbrake");
    default: return NAME_None;
    }
}

void UPinkCabCockpitInteractionComponent::SetQuickSlotHeld(const int32 Slot, const bool bHeld)
{
    if (Slot < 1 || Slot > QuickSlots.Num())
    {
        return;
    }
    FQuickSlotState& State = QuickSlots[Slot - 1];
    if (bHeld && !State.bHeld)
    {
        State.PressSerial = NextPressSerial++;
    }
    State.bHeld = bHeld;
}

FName UPinkCabCockpitInteractionComponent::GetCurrentQuickTargetId() const
{
    int32 BestIndex = INDEX_NONE;
    uint32 BestSerial = 0;
    for (int32 Index = 0; Index < QuickSlots.Num(); ++Index)
    {
        const FQuickSlotState& State = QuickSlots[Index];
        if (State.bHeld && State.PressSerial >= BestSerial)
        {
            BestSerial = State.PressSerial;
            BestIndex = Index;
        }
    }
    return BestIndex == INDEX_NONE ? NAME_None : TargetForQuickSlot(BestIndex + 1);
}

bool UPinkCabCockpitInteractionComponent::CommitAttention()
{
    if (CandidateTargetId.IsNone())
    {
        return false;
    }
    AttentionTargetId = CandidateTargetId;
    return true;
}

bool UPinkCabCockpitInteractionComponent::BeginGo(FPinkCabInteractionEvent& OutEvent)
{
    const FName Target = !AttentionTargetId.IsNone() ? AttentionTargetId : GetCurrentQuickTargetId();
    if (Target.IsNone())
    {
        return false;
    }

    ActiveGoTargetId = Target;
    OutEvent = {Target, EPinkCabInteractionGesture::PressHold, 1};
    return true;
}

bool UPinkCabCockpitInteractionComponent::EndGo(FPinkCabInteractionEvent& OutEvent)
{
    if (ActiveGoTargetId.IsNone())
    {
        return false;
    }

    OutEvent = {ActiveGoTargetId, EPinkCabInteractionGesture::PressHold, -1};
    ActiveGoTargetId = NAME_None;
    return true;
}

bool UPinkCabCockpitInteractionComponent::BuildWheelEvent(
    const int32 SignedSteps,
    FPinkCabInteractionEvent& OutEvent) const
{
    if (SignedSteps == 0)
    {
        return false;
    }
    const FName Target = !AttentionTargetId.IsNone() ? AttentionTargetId : GetCurrentQuickTargetId();
    if (Target.IsNone())
    {
        return false;
    }
    OutEvent = {Target, EPinkCabInteractionGesture::WheelIncrement, SignedSteps};
    return true;
}
