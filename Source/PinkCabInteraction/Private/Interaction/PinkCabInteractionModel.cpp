#include "Interaction/PinkCabInteractionModel.h"

FPinkCabReticleFeedback FPinkCabReticleFeedback::Resolve(
    bool bValidTarget)
{
    return bValidTarget
        ? FPinkCabReticleFeedback{0.78f, true, false}
        : FPinkCabReticleFeedback{};
}

FPinkCabInteractionControlSpec::FPinkCabInteractionControlSpec(
    FName InId,
    bool bInGrip,
    bool bInMomentary,
    bool bInWheel)
    : Id(InId),
      bSupportsGrip(bInGrip),
      bSupportsMomentary(bInMomentary),
      bSupportsWheel(bInWheel)
{
}

FPinkCabInteractionControlSpec
PinkCabInteractionSpecForTargetId(FName TargetId)
{
    if (TargetId == TEXT("TurnSignals"))
    {
        return {TargetId, false, false, true};
    }
    if (TargetId == TEXT("Horn"))
    {
        return {TargetId, false, true, false};
    }
    if (TargetId == TEXT("Gearbox"))
    {
        return {TargetId, true, false, false};
    }
    if (TargetId == TEXT("Handbrake"))
    {
        return {TargetId, true, false, false};
    }
    if (TargetId == TEXT("ClutchPedal"))
    {
        return {TargetId, false, false, true};
    }
    if (TargetId == TEXT("Ignition"))
    {
        return {TargetId, false, true, false};
    }
    if (TargetId == TEXT("PassengerDoor"))
    {
        return {TargetId, true, false, true};
    }
    if (TargetId == TEXT("Meter") || TargetId == TEXT("Taximeter"))
    {
        return {TargetId, false, true, false};
    }
    if (TargetId == TEXT("Lights") || TargetId == TEXT("Wipers"))
    {
        return {TargetId, false, false, true};
    }
    if (TargetId == TEXT("Washer"))
    {
        return {TargetId, false, true, false};
    }
    return {TargetId, false, false, false};
}

FPinkCabInteractionCandidate::FPinkCabInteractionCandidate(
    FName InId,
    float InDistanceCm,
    float InAngularScore,
    bool bInValid)
    : Id(InId),
      DistanceCm(InDistanceCm),
      AngularScore(InAngularScore),
      bValid(bInValid)
{
}
FName FPinkCabInteractionTargetSelector::SelectCurrentTarget(
    const TArray<FPinkCabInteractionCandidate>& Candidates,
    int32 MaxCandidates,
    float MaxDistanceCm)
{
    if (MaxCandidates <= 0 || MaxDistanceCm < 0.0f)
    {
        return NAME_None;
    }

    const int32 Count = FMath::Min(
        MaxCandidates,
        Candidates.Num());
    FName Best = NAME_None;
    float BestAngularScore = TNumericLimits<float>::Max();
    for (int32 Index = 0; Index < Count; ++Index)
    {
        const FPinkCabInteractionCandidate& Candidate =
            Candidates[Index];
        if (!Candidate.bValid
            || Candidate.Id.IsNone()
            || Candidate.DistanceCm > MaxDistanceCm)
        {
            continue;
        }
        if (Candidate.AngularScore < BestAngularScore)
        {
            BestAngularScore = Candidate.AngularScore;
            Best = Candidate.Id;
        }
    }
    return Best;
}

EPinkCabMouseOwner FPinkCabInteractionState::GetMouseOwner() const
{
    return MouseOwner;
}

FName FPinkCabInteractionState::GetCurrentTargetId() const
{
    return CurrentTarget.Id;
}

uint32 FPinkCabInteractionState::GetActuationSerial() const
{
    return ActuationSerial;
}
bool FPinkCabInteractionState::IsGripActive() const
{
    return bGripActive;
}

bool FPinkCabInteractionState::IsMomentaryHeld() const
{
    return bMomentaryHeld;
}

FPinkCabInteractionEvent
FPinkCabInteractionState::GetLastEvent() const
{
    return LastEvent;
}

void FPinkCabInteractionState::SetGazeHeld(bool bHeld)
{
    MouseOwner = bHeld
        ? EPinkCabMouseOwner::Gaze
        : EPinkCabMouseOwner::Steering;
}

void FPinkCabInteractionState::SetCurrentTarget(
    const FPinkCabInteractionControlSpec& Spec)
{
    CurrentTarget = Spec;
    bGripActive = false;
    bMomentaryHeld = false;
    LastEvent = {};
}

bool FPinkCabInteractionState::RecallQuickTarget(int32 Slot)
{
    FName Target = NAME_None;
    switch (Slot)
    {
    case 1:
        Target = FName(TEXT("TurnSignals"));
        break;
    case 2:
        Target = FName(TEXT("Horn"));
        break;
    case 3:
        Target = FName(TEXT("Gearbox"));
        break;
    case 4:
        Target = FName(TEXT("Handbrake"));
        break;
    default:
        return false;
    }

    CurrentTarget = PinkCabInteractionSpecForTargetId(Target);
    bGripActive = false;
    bMomentaryHeld = false;
    return true;
}

bool FPinkCabInteractionState::TryBeginGrip()
{
    if (!CurrentTarget.bSupportsGrip || CurrentTarget.Id.IsNone())
    {
        return false;
    }
    bGripActive = true;
    return true;
}

bool FPinkCabInteractionState::TryMomentaryPress(bool bPressed)
{
    if (!CurrentTarget.bSupportsMomentary || CurrentTarget.Id.IsNone())
    {
        return false;
    }

    bMomentaryHeld = bPressed;
    ++ActuationSerial;
    LastEvent = {
        CurrentTarget.Id,
        EPinkCabInteractionGesture::PressHold,
        bPressed ? 1 : 0};
    return true;
}
int32 FPinkCabInteractionState::ApplyWheelSteps(
    int32 SignedSteps)
{
    if (!CurrentTarget.bSupportsWheel
        || CurrentTarget.Id.IsNone()
        || SignedSteps == 0)
    {
        return 0;
    }
    ++ActuationSerial;
    LastEvent = {
        CurrentTarget.Id,
        EPinkCabInteractionGesture::WheelIncrement,
        SignedSteps};
    return SignedSteps;
}
