#pragma once

#include "CoreMinimal.h"
#include "Vehicle/PinkCabWallrideController.h"
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

class FPinkCabL1TraversalState
{
public:
    FPinkCabL1TraversalResult Step(const FPinkCabL1TraversalInput& Input)
    {
        if (Input.bAbort)
        {
            AbortInternal();
            return BuildResult(true);
        }
        if (Input.bCollision)
        {
            return Fail(EPinkCabL1TraversalFailure::Collision);
        }
        if (!FMath::IsFinite(Input.DeltaSeconds) || Input.DeltaSeconds < 0.0f)
        {
            return Fail(EPinkCabL1TraversalFailure::InvalidDelta);
        }
        float IgnoredSeconds = 0.0f;
        if (!FPinkCabWallrideController::TryComputeResidualTimeoutSeconds(
                Input.TotalMassKg, IgnoredSeconds))
        {
            return Fail(EPinkCabL1TraversalFailure::UnsupportedMass);
        }
        LastMassKg = Input.TotalMassKg;
        if (!Input.bHasContact)
        {
            return HandleAir(Input);
        }
        if (Input.ContactId.TrimStartAndEnd().IsEmpty())
        {
            return Fail(EPinkCabL1TraversalFailure::MissingContactId);
        }
        return HandleContact(Input);
    }

    void Reset()
    {
        Wallride.Abort();
        Phase = EPinkCabL1TraversalPhase::Road;
        Failure = EPinkCabL1TraversalFailure::None;
        CurrentContactId.Reset();
        PreviousContactId.Reset();
        bHasCurrentKind = false;
        LastMassKg = 0.0f;
    }

private:
    static bool IsWallKind(EPinkCabVerticalContactKind Kind)
    {
        return Kind == EPinkCabVerticalContactKind::WallLeft
            || Kind == EPinkCabVerticalContactKind::WallRight;
    }

    FPinkCabL1TraversalResult HandleAir(const FPinkCabL1TraversalInput& Input)
    {
        if (Phase == EPinkCabL1TraversalPhase::WallContact
            || Phase == EPinkCabL1TraversalPhase::WallResidual)
        {
            Wallride.Update(false, Input.DeltaSeconds, Input.TotalMassKg);
            if (Wallride.GetState() == EPinkCabWallrideState::Expired)
            {
                return Fail(EPinkCabL1TraversalFailure::WallrideExpired);
            }
            Phase = EPinkCabL1TraversalPhase::WallResidual;
            Failure = EPinkCabL1TraversalFailure::None;
            return BuildResult(true);
        }
        if (Phase == EPinkCabL1TraversalPhase::FreightContact)
        {
            MoveCurrentToPrevious();
            Phase = EPinkCabL1TraversalPhase::Road;
            bHasCurrentKind = false;
        }
        return BuildResult(Phase != EPinkCabL1TraversalPhase::Failed
            && Phase != EPinkCabL1TraversalPhase::Aborted);
    }

    FPinkCabL1TraversalResult HandleContact(const FPinkCabL1TraversalInput& Input)
    {
        switch (Phase)
        {
        case EPinkCabL1TraversalPhase::Road:
            return EnterFromRoad(Input);
        case EPinkCabL1TraversalPhase::WallContact:
        case EPinkCabL1TraversalPhase::WallResidual:
            return EnterFromWall(Input);
        case EPinkCabL1TraversalPhase::FreightContact:
            return EnterFromFreight(Input);
        case EPinkCabL1TraversalPhase::GapTransfer:
            return EnterFromGap(Input);
        default:
            return Fail(EPinkCabL1TraversalFailure::InvalidTransition);
        }
    }

    FPinkCabL1TraversalResult EnterFromRoad(const FPinkCabL1TraversalInput& Input)
    {
        if (IsWallKind(Input.ContactKind))
        {
            SetCurrentContact(Input);
            Wallride.Update(true, Input.DeltaSeconds, Input.TotalMassKg);
            Phase = EPinkCabL1TraversalPhase::WallContact;
            Failure = EPinkCabL1TraversalFailure::None;
            return BuildResult(true);
        }
        if (Input.ContactKind == EPinkCabVerticalContactKind::FreightCeiling)
        {
            SetCurrentContact(Input);
            Phase = EPinkCabL1TraversalPhase::FreightContact;
            Failure = EPinkCabL1TraversalFailure::None;
            return BuildResult(true);
        }
        if (Input.ContactKind == EPinkCabVerticalContactKind::PoplarGapHook)
        {
            SetCurrentContact(Input);
            Phase = EPinkCabL1TraversalPhase::GapTransfer;
            Failure = EPinkCabL1TraversalFailure::None;
            return BuildResult(true);
        }
        return Fail(EPinkCabL1TraversalFailure::InvalidTransition);
    }

    FPinkCabL1TraversalResult EnterFromWall(const FPinkCabL1TraversalInput& Input)
    {
        if (IsWallKind(Input.ContactKind))
        {
            if (!bHasCurrentKind || Input.ContactKind != CurrentKind
                || Input.ContactId != CurrentContactId)
            {
                return Fail(EPinkCabL1TraversalFailure::InvalidTransition);
            }
            Wallride.Update(true, Input.DeltaSeconds, Input.TotalMassKg);
            Phase = EPinkCabL1TraversalPhase::WallContact;
            Failure = EPinkCabL1TraversalFailure::None;
            return BuildResult(true);
        }
        Wallride.Abort();
        if (Input.ContactKind == EPinkCabVerticalContactKind::FreightCeiling)
        {
            SetCurrentContact(Input);
            Phase = EPinkCabL1TraversalPhase::FreightContact;
            Failure = EPinkCabL1TraversalFailure::None;
            return BuildResult(true);
        }
        if (Input.ContactKind == EPinkCabVerticalContactKind::PoplarGapHook)
        {
            SetCurrentContact(Input);
            Phase = EPinkCabL1TraversalPhase::GapTransfer;
            Failure = EPinkCabL1TraversalFailure::None;
            return BuildResult(true);
        }
        if (Input.ContactKind == EPinkCabVerticalContactKind::ReceivingStrip)
        {
            return Complete(Input);
        }
        return Fail(EPinkCabL1TraversalFailure::InvalidTransition);
    }

    FPinkCabL1TraversalResult EnterFromFreight(const FPinkCabL1TraversalInput& Input)
    {
        if (Input.ContactKind == EPinkCabVerticalContactKind::FreightCeiling
            && Input.ContactId == CurrentContactId)
        {
            return BuildResult(true);
        }
        if (Input.ContactKind == EPinkCabVerticalContactKind::ReceivingStrip)
        {
            return Complete(Input);
        }
        return Fail(EPinkCabL1TraversalFailure::InvalidTransition);
    }

    FPinkCabL1TraversalResult EnterFromGap(const FPinkCabL1TraversalInput& Input)
    {
        if (Input.ContactKind == EPinkCabVerticalContactKind::ReceivingStrip)
        {
            return Complete(Input);
        }
        return Fail(EPinkCabL1TraversalFailure::InvalidTransition);
    }

    FPinkCabL1TraversalResult Complete(const FPinkCabL1TraversalInput& Input)
    {
        Wallride.Abort();
        SetCurrentContact(Input);
        Phase = EPinkCabL1TraversalPhase::Completed;
        Failure = EPinkCabL1TraversalFailure::None;
        return BuildResult(true);
    }

    void SetCurrentContact(const FPinkCabL1TraversalInput& Input)
    {
        if (!CurrentContactId.IsEmpty() && CurrentContactId != Input.ContactId)
        {
            PreviousContactId = CurrentContactId;
        }
        CurrentContactId = Input.ContactId;
        CurrentKind = Input.ContactKind;
        bHasCurrentKind = true;
    }

    void MoveCurrentToPrevious()
    {
        if (!CurrentContactId.IsEmpty())
        {
            PreviousContactId = CurrentContactId;
        }
        CurrentContactId.Reset();
    }
    FPinkCabL1TraversalResult Fail(EPinkCabL1TraversalFailure InFailure)
    {
        Wallride.Abort();
        CurrentContactId.Reset();
        PreviousContactId.Reset();
        bHasCurrentKind = false;
        LastMassKg = 0.0f;
        Phase = EPinkCabL1TraversalPhase::Failed;
        Failure = InFailure;
        return BuildResult(false);
    }

    void AbortInternal()
    {
        Wallride.Abort();
        CurrentContactId.Reset();
        PreviousContactId.Reset();
        bHasCurrentKind = false;
        LastMassKg = 0.0f;
        Phase = EPinkCabL1TraversalPhase::Aborted;
        Failure = EPinkCabL1TraversalFailure::None;
    }

    FPinkCabL1TraversalResult BuildResult(bool bAccepted) const
    {
        FPinkCabL1TraversalResult Result;
        Result.Phase = Phase;
        Result.Failure = Failure;
        Result.CurrentContactId = CurrentContactId;
        Result.PreviousContactId = PreviousContactId;
        Result.bAccepted = bAccepted;
        Result.bCompleted = Phase == EPinkCabL1TraversalPhase::Completed;
        Result.bForceApplied = false;
        if (Phase == EPinkCabL1TraversalPhase::WallContact
            || Phase == EPinkCabL1TraversalPhase::WallResidual)
        {
            const FPinkCabWallrideConstraintRequest Request =
                Wallride.BuildConstraintRequest(LastMassKg);
            Result.bConstraintRequested = Request.bRequested;
        }
        return Result;
    }

    FPinkCabWallrideController Wallride;
    EPinkCabL1TraversalPhase Phase = EPinkCabL1TraversalPhase::Road;
    EPinkCabL1TraversalFailure Failure = EPinkCabL1TraversalFailure::None;
    FString CurrentContactId;
    FString PreviousContactId;
    EPinkCabVerticalContactKind CurrentKind = EPinkCabVerticalContactKind::WallLeft;
    bool bHasCurrentKind = false;
    float LastMassKg = 0.0f;
};
