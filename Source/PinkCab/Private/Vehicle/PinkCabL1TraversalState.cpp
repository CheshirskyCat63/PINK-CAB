#include "Vehicle/PinkCabL1TraversalState.h"

#include "Vehicle/PinkCabL1TraversalTransitionPolicy.h"

FPinkCabL1TraversalState::FPinkCabL1TraversalState(
    const FPinkCabL1TraversalConfig& InConfig)
    : Config(InConfig)
{
}

FPinkCabL1TraversalResult FPinkCabL1TraversalState::Step(
    const FPinkCabL1TraversalInput& Input)
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

void FPinkCabL1TraversalState::Reset()
{
    Wallride.Abort();
    Phase = EPinkCabL1TraversalPhase::Road;
    Failure = EPinkCabL1TraversalFailure::None;
    CurrentContactId.Reset();
    PreviousContactId.Reset();
    bHasCurrentKind = false;
    LastMassKg = 0.0f;
}

FPinkCabL1TraversalResult FPinkCabL1TraversalState::HandleAir(
    const FPinkCabL1TraversalInput& Input)
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
    return BuildResult(
        Phase != EPinkCabL1TraversalPhase::Failed
        && Phase != EPinkCabL1TraversalPhase::Aborted);
}

FPinkCabL1TraversalResult FPinkCabL1TraversalState::HandleContact(
    const FPinkCabL1TraversalInput& Input)
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

FPinkCabL1TraversalResult FPinkCabL1TraversalState::EnterFromRoad(
    const FPinkCabL1TraversalInput& Input)
{
    if (!FPinkCabL1TraversalTransitionPolicy::CanEnterFromRoad(Config, Input.ContactKind))
    {
        return Fail(EPinkCabL1TraversalFailure::InvalidTransition);
    }
    if (FPinkCabL1TraversalTransitionPolicy::IsWallKind(Input.ContactKind))
    {
        SetCurrentContact(Input);
        Wallride.Update(true, Input.DeltaSeconds, Input.TotalMassKg);
        Phase = EPinkCabL1TraversalPhase::WallContact;
    }
    else if (Input.ContactKind == EPinkCabVerticalContactKind::FreightCeiling)
    {
        SetCurrentContact(Input);
        Phase = EPinkCabL1TraversalPhase::FreightContact;
    }
    else
    {
        SetCurrentContact(Input);
        Phase = EPinkCabL1TraversalPhase::GapTransfer;
    }
    Failure = EPinkCabL1TraversalFailure::None;
    return BuildResult(true);
}

FPinkCabL1TraversalResult FPinkCabL1TraversalState::EnterFromWall(
    const FPinkCabL1TraversalInput& Input)
{
    if (FPinkCabL1TraversalTransitionPolicy::IsWallKind(Input.ContactKind))
    {
        if (!bHasCurrentKind
            || Input.ContactKind != CurrentKind
            || Input.ContactId != CurrentContactId)
        {
            return Fail(EPinkCabL1TraversalFailure::InvalidTransition);
        }
        Wallride.Update(true, Input.DeltaSeconds, Input.TotalMassKg);
        Phase = EPinkCabL1TraversalPhase::WallContact;
        Failure = EPinkCabL1TraversalFailure::None;
        return BuildResult(true);
    }

    if (!FPinkCabL1TraversalTransitionPolicy::CanEnterFromWall(Config, Input.ContactKind))
    {
        return Fail(EPinkCabL1TraversalFailure::InvalidTransition);
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
    return Complete(Input);
}

FPinkCabL1TraversalResult FPinkCabL1TraversalState::EnterFromFreight(
    const FPinkCabL1TraversalInput& Input)
{
    if (Input.ContactKind == EPinkCabVerticalContactKind::FreightCeiling
        && Input.ContactId == CurrentContactId)
    {
        return BuildResult(true);
    }
    if (FPinkCabL1TraversalTransitionPolicy::CanEnterFromFreight(Config, Input.ContactKind))
    {
        return Complete(Input);
    }
    return Fail(EPinkCabL1TraversalFailure::InvalidTransition);
}

FPinkCabL1TraversalResult FPinkCabL1TraversalState::EnterFromGap(
    const FPinkCabL1TraversalInput& Input)
{
    if (FPinkCabL1TraversalTransitionPolicy::CanEnterFromGap(Config, Input.ContactKind))
    {
        return Complete(Input);
    }
    return Fail(EPinkCabL1TraversalFailure::InvalidTransition);
}

FPinkCabL1TraversalResult FPinkCabL1TraversalState::Complete(
    const FPinkCabL1TraversalInput& Input)
{
    Wallride.Abort();
    SetCurrentContact(Input);
    Phase = EPinkCabL1TraversalPhase::Completed;
    Failure = EPinkCabL1TraversalFailure::None;
    return BuildResult(true);
}

void FPinkCabL1TraversalState::SetCurrentContact(
    const FPinkCabL1TraversalInput& Input)
{
    if (!CurrentContactId.IsEmpty() && CurrentContactId != Input.ContactId)
    {
        PreviousContactId = CurrentContactId;
    }
    CurrentContactId = Input.ContactId;
    CurrentKind = Input.ContactKind;
    bHasCurrentKind = true;
}

void FPinkCabL1TraversalState::MoveCurrentToPrevious()
{
    if (!CurrentContactId.IsEmpty())
    {
        PreviousContactId = CurrentContactId;
    }
    CurrentContactId.Reset();
}

FPinkCabL1TraversalResult FPinkCabL1TraversalState::Fail(
    const EPinkCabL1TraversalFailure InFailure)
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

void FPinkCabL1TraversalState::AbortInternal()
{
    Wallride.Abort();
    CurrentContactId.Reset();
    PreviousContactId.Reset();
    bHasCurrentKind = false;
    LastMassKg = 0.0f;
    Phase = EPinkCabL1TraversalPhase::Aborted;
    Failure = EPinkCabL1TraversalFailure::None;
}

FPinkCabL1TraversalResult FPinkCabL1TraversalState::BuildResult(
    const bool bAccepted) const
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
