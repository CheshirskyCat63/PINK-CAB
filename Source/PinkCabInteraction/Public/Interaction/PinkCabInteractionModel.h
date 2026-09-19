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

struct PINKCABINTERACTION_API FPinkCabReticleFeedback
{
    float Opacity = 0.28f;
    bool bWhiteMatte = false;
    bool bShowTextLabel = false;

    static FPinkCabReticleFeedback Resolve(bool bValidTarget);
};

struct PINKCABINTERACTION_API FPinkCabInteractionControlSpec
{
    FPinkCabInteractionControlSpec() = default;
    FPinkCabInteractionControlSpec(
        FName InId,
        bool bInGrip,
        bool bInMomentary,
        bool bInWheel);

    FName Id = NAME_None;
    bool bSupportsGrip = false;
    bool bSupportsMomentary = false;
    bool bSupportsWheel = false;
};

PINKCABINTERACTION_API FPinkCabInteractionControlSpec
PinkCabInteractionSpecForTargetId(FName TargetId);

struct PINKCABINTERACTION_API FPinkCabInteractionCandidate
{
    FPinkCabInteractionCandidate() = default;
    FPinkCabInteractionCandidate(
        FName InId,
        float InDistanceCm,
        float InAngularScore,
        bool bInValid);

    FName Id = NAME_None;
    float DistanceCm = 0.0f;
    float AngularScore = 0.0f;
    bool bValid = false;
};

struct FPinkCabInteractionEvent
{
    FName TargetId = NAME_None;
    EPinkCabInteractionGesture Gesture =
        EPinkCabInteractionGesture::None;
    int32 SignedValue = 0;
};

struct PINKCABINTERACTION_API FPinkCabInteractionTargetSelector
{
    static FName SelectCurrentTarget(
        const TArray<FPinkCabInteractionCandidate>& Candidates,
        int32 MaxCandidates,
        float MaxDistanceCm);
};

struct PINKCABINTERACTION_API FPinkCabInteractionState
{
    EPinkCabMouseOwner GetMouseOwner() const;
    FName GetCurrentTargetId() const;
    uint32 GetActuationSerial() const;
    bool IsGripActive() const;
    bool IsMomentaryHeld() const;
    FPinkCabInteractionEvent GetLastEvent() const;

    void SetGazeHeld(bool bHeld);
    void SetCurrentTarget(
        const FPinkCabInteractionControlSpec& Spec);
    bool RecallQuickTarget(int32 Slot);
    bool TryBeginGrip();
    bool TryMomentaryPress(bool bPressed);
    int32 ApplyWheelSteps(int32 SignedSteps);

private:
    EPinkCabMouseOwner MouseOwner =
        EPinkCabMouseOwner::Steering;
    FPinkCabInteractionControlSpec CurrentTarget;
    FPinkCabInteractionEvent LastEvent;
    uint32 ActuationSerial = 0;
    bool bGripActive = false;
    bool bMomentaryHeld = false;
};
