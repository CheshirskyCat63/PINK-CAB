#pragma once

#include "CoreMinimal.h"
#include "Vehicle/PinkCabWallrideState.h"

class FPinkCabWallrideController
{
public:
    static float ComputeResidualTimeoutSeconds(float TotalMassKg)
    {
        if (TotalMassKg <= 1657.0f) return 5.0f;
        const float Alpha = FMath::Clamp((TotalMassKg - 1657.0f) / 450.0f, 0.0f, 1.0f);
        return FMath::Lerp(5.0f, 4.0f, Alpha);
    }

    void Update(bool bValidContact, float DeltaSeconds, float TotalMassKg)
    {
        const float SafeDelta = FMath::Max(0.0f, DeltaSeconds);
        if (bValidContact)
        {
            HandleContact(SafeDelta, TotalMassKg);
        }
        else
        {
            HandleNoContact(SafeDelta);
        }
    }
    EPinkCabWallrideState GetState() const { return State; }
    float GetResidualSeconds() const { return ResidualSeconds; }

    FPinkCabWallrideConstraintRequest BuildConstraintRequest(float TotalMassKg) const
    {
        FPinkCabWallrideConstraintRequest Result;
        Result.State = State;
        Result.TotalMassKg = TotalMassKg;
        Result.bRequested = State == EPinkCabWallrideState::Contact || State == EPinkCabWallrideState::Residual;
        Result.bForceApplied = false;
        return Result;
    }

private:
    void HandleContact(float DeltaSeconds, float TotalMassKg)
    {
        if (State == EPinkCabWallrideState::Detached || State == EPinkCabWallrideState::Expired)
        {
            State = EPinkCabWallrideState::Contact;
            ResidualSeconds = ComputeResidualTimeoutSeconds(TotalMassKg);
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
            ResidualSeconds = ComputeResidualTimeoutSeconds(TotalMassKg);
            ReacquiredContactSeconds = 0.0f;
            bPendingReacquisitionReset = false;
        }
    }

    void HandleNoContact(float DeltaSeconds)
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

    EPinkCabWallrideState State = EPinkCabWallrideState::Detached;
    float ResidualSeconds = 0.0f;
    float ReacquiredContactSeconds = 0.0f;
    bool bPendingReacquisitionReset = false;
};
