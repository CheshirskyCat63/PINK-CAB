#pragma once

#include "CoreMinimal.h"
#include "Vehicle/PinkCabL1TraversalConfig.h"
#include "Vehicle/PinkCabL1TraversalTypes.h"
#include "Vehicle/PinkCabWallrideController.h"

class PINKCAB_API FPinkCabL1TraversalState
{
public:
    explicit FPinkCabL1TraversalState(const FPinkCabL1TraversalConfig& InConfig = {});

    FPinkCabL1TraversalResult Step(const FPinkCabL1TraversalInput& Input);
    void Reset();

private:
    FPinkCabL1TraversalResult HandleAir(const FPinkCabL1TraversalInput& Input);
    FPinkCabL1TraversalResult HandleContact(const FPinkCabL1TraversalInput& Input);
    FPinkCabL1TraversalResult EnterFromRoad(const FPinkCabL1TraversalInput& Input);
    FPinkCabL1TraversalResult EnterFromWall(const FPinkCabL1TraversalInput& Input);
    FPinkCabL1TraversalResult EnterFromFreight(const FPinkCabL1TraversalInput& Input);
    FPinkCabL1TraversalResult EnterFromGap(const FPinkCabL1TraversalInput& Input);
    FPinkCabL1TraversalResult Complete(const FPinkCabL1TraversalInput& Input);
    void SetCurrentContact(const FPinkCabL1TraversalInput& Input);
    void MoveCurrentToPrevious();
    FPinkCabL1TraversalResult Fail(EPinkCabL1TraversalFailure InFailure);
    void AbortInternal();
    FPinkCabL1TraversalResult BuildResult(bool bAccepted) const;

    FPinkCabL1TraversalConfig Config;
    FPinkCabWallrideController Wallride;
    EPinkCabL1TraversalPhase Phase = EPinkCabL1TraversalPhase::Road;
    EPinkCabL1TraversalFailure Failure = EPinkCabL1TraversalFailure::None;
    FString CurrentContactId;
    FString PreviousContactId;
    EPinkCabVerticalContactKind CurrentKind = EPinkCabVerticalContactKind::WallLeft;
    bool bHasCurrentKind = false;
    float LastMassKg = 0.0f;
};
