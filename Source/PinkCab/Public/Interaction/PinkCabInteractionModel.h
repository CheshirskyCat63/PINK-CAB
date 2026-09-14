#pragma once

#include "CoreMinimal.h"

enum class EPinkCabMouseOwner : uint8
{
    Steering,
    Gaze
};

enum class EPinkCabInteractionGesture : uint8
{
    None,
    GripBegin,
    PressHold,
    WheelIncrement
};

struct FPinkCabReticleFeedback
{
    float Opacity = 0.28f;
    bool bWhiteMatte = false;
    bool bShowTextLabel = false;

    static FPinkCabReticleFeedback Resolve(const bool bValidTarget)
    {
        return bValidTarget
            ? FPinkCabReticleFeedback{0.78f, true, false}
            : FPinkCabReticleFeedback{};
    }
};
struct FPinkCabInteractionControlSpec
{
    FPinkCabInteractionControlSpec() = default;
    FPinkCabInteractionControlSpec(FName InId, bool bInGrip, bool bInMomentary, bool bInWheel)
        : Id(InId), bSupportsGrip(bInGrip), bSupportsMomentary(bInMomentary), bSupportsWheel(bInWheel) {}

    FName Id = NAME_None;
    bool bSupportsGrip = false;
    bool bSupportsMomentary = false;
    bool bSupportsWheel = false;
};

inline FPinkCabInteractionControlSpec PinkCabInteractionSpecForTargetId(const FName TargetId)
{
    if (TargetId == TEXT("TurnSignals")) return {TargetId, false, false, true};
    if (TargetId == TEXT("Horn")) return {TargetId, false, true, false};
    if (TargetId == TEXT("Gearbox")) return {TargetId, true, false, true};
    if (TargetId == TEXT("Handbrake")) return {TargetId, true, false, true};
    if (TargetId == TEXT("Ignition")) return {TargetId, false, true, false};
    if (TargetId == TEXT("PassengerDoor")) return {TargetId, true, false, true};
    if (TargetId == TEXT("Meter") || TargetId == TEXT("Taximeter")) return {TargetId, false, true, false};
    if (TargetId == TEXT("Lights") || TargetId == TEXT("Wipers")) return {TargetId, false, false, true};
    if (TargetId == TEXT("Washer")) return {TargetId, false, true, false};
    return {TargetId, false, false, false};
}

struct FPinkCabInteractionCandidate
{
    FPinkCabInteractionCandidate() = default;
    FPinkCabInteractionCandidate(FName InId, float InDistanceCm, float InAngularScore, bool bInValid)
        : Id(InId), DistanceCm(InDistanceCm), AngularScore(InAngularScore), bValid(bInValid) {}

    FName Id = NAME_None;
    float DistanceCm = 0.0f;
    float AngularScore = 0.0f;
    bool bValid = false;
};

struct FPinkCabInteractionEvent
{
    FName TargetId = NAME_None;
    EPinkCabInteractionGesture Gesture = EPinkCabInteractionGesture::None;
    int32 SignedValue = 0;
};
struct FPinkCabInteractionTargetSelector
{
    static FName SelectCurrentTarget(
        const TArray<FPinkCabInteractionCandidate>& Candidates,
        int32 MaxCandidates,
        float MaxDistanceCm)
    {
        if (MaxCandidates <= 0 || MaxDistanceCm < 0.0f)
        {
            return NAME_None;
        }

        const int32 Count = FMath::Min(MaxCandidates, Candidates.Num());
        FName Best = NAME_None;
        float BestAngularScore = TNumericLimits<float>::Max();
        for (int32 Index = 0; Index < Count; ++Index)
        {
            const FPinkCabInteractionCandidate& Candidate = Candidates[Index];
            if (!Candidate.bValid || Candidate.Id.IsNone() || Candidate.DistanceCm > MaxDistanceCm)
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
};
struct FPinkCabInteractionState
{
    EPinkCabMouseOwner GetMouseOwner() const { return MouseOwner; }
    FName GetCurrentTargetId() const { return CurrentTarget.Id; }
    uint32 GetActuationSerial() const { return ActuationSerial; }
    bool IsGripActive() const { return bGripActive; }
    bool IsMomentaryHeld() const { return bMomentaryHeld; }
    FPinkCabInteractionEvent GetLastEvent() const { return LastEvent; }

    void SetGazeHeld(bool bHeld)
    {
        MouseOwner = bHeld ? EPinkCabMouseOwner::Gaze : EPinkCabMouseOwner::Steering;
    }

    void SetCurrentTarget(const FPinkCabInteractionControlSpec& Spec)
    {
        CurrentTarget = Spec;
        bGripActive = false;
        bMomentaryHeld = false;
        LastEvent = {};
    }

    bool RecallQuickTarget(int32 Slot)
    {
        FName Target = NAME_None;
        switch (Slot)
        {
        case 1: Target = FName(TEXT("TurnSignals")); break;
        case 2: Target = FName(TEXT("Horn")); break;
        case 3: Target = FName(TEXT("Gearbox")); break;
        case 4: Target = FName(TEXT("Handbrake")); break;
        default: return false;
        }
        CurrentTarget = PinkCabInteractionSpecForTargetId(Target);
        bGripActive = false;
        bMomentaryHeld = false;
        return true;
    }
    bool TryBeginGrip()
    {
        if (!CurrentTarget.bSupportsGrip || CurrentTarget.Id.IsNone())
        {
            return false;
        }
        bGripActive = true;
        return true;
    }

    bool TryMomentaryPress(bool bPressed)
    {
        if (!CurrentTarget.bSupportsMomentary || CurrentTarget.Id.IsNone())
        {
            return false;
        }
        bMomentaryHeld = bPressed;
        ++ActuationSerial;
        LastEvent = {CurrentTarget.Id, EPinkCabInteractionGesture::PressHold, bPressed ? 1 : 0};
        return true;
    }

    int32 ApplyWheelSteps(int32 SignedSteps)
    {
        if (!CurrentTarget.bSupportsWheel || CurrentTarget.Id.IsNone() || SignedSteps == 0)
        {
            return 0;
        }
        if (CurrentTarget.bSupportsGrip && !bGripActive)
        {
            return 0;
        }
        ++ActuationSerial;
        LastEvent = {CurrentTarget.Id, EPinkCabInteractionGesture::WheelIncrement, SignedSteps};
        return SignedSteps;
    }

private:
    EPinkCabMouseOwner MouseOwner = EPinkCabMouseOwner::Steering;
    FPinkCabInteractionControlSpec CurrentTarget;
    FPinkCabInteractionEvent LastEvent;
    uint32 ActuationSerial = 0;
    bool bGripActive = false;
    bool bMomentaryHeld = false;
};
