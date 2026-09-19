#include "Vehicle/PinkCabWallrideController.h"

bool FPinkCabWallrideController::TryComputeResidualTimeoutSeconds(float TotalMassKg, float& OutSeconds)
{
    OutSeconds = 0.0f;
    if (!FMath::IsFinite(TotalMassKg) || TotalMassKg <= 0.0f || TotalMassKg > 2107.0f)
    {
        return false;
    }
    if (TotalMassKg <= 1657.0f)
    {
        OutSeconds = 5.0f;
        return true;
    }

    OutSeconds = 5.0f - ((TotalMassKg - 1657.0f) / 450.0f);
    return true;
}

float FPinkCabWallrideController::ComputeResidualTimeoutSeconds(float TotalMassKg)
{
    float Seconds = 0.0f;
    TryComputeResidualTimeoutSeconds(TotalMassKg, Seconds);
    return Seconds;
}

void FPinkCabWallrideController::Update(bool bValidContact, float DeltaSeconds, float TotalMassKg)
{
    float ValidatedSeconds = 0.0f;
    if (!TryComputeResidualTimeoutSeconds(TotalMassKg, ValidatedSeconds))
    {
        Abort();
        return;
    }

    const float SafeDelta = FMath::Max(0.0f, DeltaSeconds);
    if (bValidContact) HandleContact(SafeDelta, TotalMassKg, ValidatedSeconds);
    else HandleNoContact(SafeDelta);
}

void FPinkCabWallrideController::Abort()
{
    State = EPinkCabWallrideState::Detached;
    ResidualSeconds = 0.0f;
    ReacquiredContactSeconds = 0.0f;
    bPendingReacquisitionReset = false;
}

EPinkCabWallrideState FPinkCabWallrideController::GetState() const { return State; }
float FPinkCabWallrideController::GetResidualSeconds() const { return ResidualSeconds; }

FPinkCabWallrideConstraintRequest FPinkCabWallrideController::BuildConstraintRequest(float TotalMassKg) const
{
    FPinkCabWallrideConstraintRequest Result;
    Result.State = State;
    Result.TotalMassKg = TotalMassKg;
    float IgnoredSeconds = 0.0f;
    const bool bMassValid = TryComputeResidualTimeoutSeconds(TotalMassKg, IgnoredSeconds);
    Result.bRequested = bMassValid
        && (State == EPinkCabWallrideState::Contact || State == EPinkCabWallrideState::Residual);
    Result.bForceApplied = false;
    return Result;
}

void FPinkCabWallrideController::HandleContact(
    float DeltaSeconds,
    float TotalMassKg,
    float ValidatedSeconds)
{
    if (State == EPinkCabWallrideState::Detached || State == EPinkCabWallrideState::Expired)
    {
        State = EPinkCabWallrideState::Contact;
        ResidualSeconds = ValidatedSeconds;
        ReacquiredContactSeconds = 0.0f;
        bPendingReacquisitionReset = false;
        return;
    }

    if (State == EPinkCabWallrideState::Residual)
    {
        State = EPinkCabWallrideState::Contact;
        ReacquiredContactSeconds = DeltaSeconds;
        bPendingReacquisitionReset = true;
    }
    else if (bPendingReacquisitionReset)
    {
        ReacquiredContactSeconds += DeltaSeconds;
    }

    if (bPendingReacquisitionReset && ReacquiredContactSeconds >= 0.25f)
    {
        ResidualSeconds = ValidatedSeconds;
        ReacquiredContactSeconds = 0.0f;
        bPendingReacquisitionReset = false;
    }
}

void FPinkCabWallrideController::HandleNoContact(float DeltaSeconds)
{
    if (State == EPinkCabWallrideState::Contact)
    {
        State = EPinkCabWallrideState::Residual;
        bPendingReacquisitionReset = false;
        ReacquiredContactSeconds = 0.0f;
    }

    if (State == EPinkCabWallrideState::Residual)
    {
        ResidualSeconds = FMath::Max(0.0f, ResidualSeconds - DeltaSeconds);
        if (ResidualSeconds <= KINDA_SMALL_NUMBER)
        {
            ResidualSeconds = 0.0f;
            State = EPinkCabWallrideState::Expired;
        }
    }
}
