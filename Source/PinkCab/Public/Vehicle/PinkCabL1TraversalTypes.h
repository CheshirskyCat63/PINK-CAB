#pragma once

#include "CoreMinimal.h"
#include "World/PinkCabVerticalContactRegistry.h"

enum class EPinkCabL1TraversalPhase : uint8
{
    Road,
    WallContact,
    WallResidual,
    FreightContact,
    GapTransfer,
    Completed,
    Failed,
    Aborted
};

enum class EPinkCabL1TraversalFailure : uint8
{
    None,
    InvalidDelta,
    UnsupportedMass,
    MissingContactId,
    InvalidTransition,
    WallrideExpired,
    Collision
};

struct FPinkCabL1TraversalInput
{
    bool bHasContact = false;
    EPinkCabVerticalContactKind ContactKind = EPinkCabVerticalContactKind::WallLeft;
    FString ContactId;
    float TotalMassKg = 0.0f;
    float DeltaSeconds = 0.0f;
    bool bAbort = false;
    bool bCollision = false;
};

struct FPinkCabL1TraversalResult
{
    EPinkCabL1TraversalPhase Phase = EPinkCabL1TraversalPhase::Road;
    EPinkCabL1TraversalFailure Failure = EPinkCabL1TraversalFailure::None;
    FString CurrentContactId;
    FString PreviousContactId;
    bool bAccepted = false;
    bool bConstraintRequested = false;
    bool bForceApplied = false;
    bool bCompleted = false;
};
