#pragma once

#include "CoreMinimal.h"
#include "Vehicle/PinkCabWallrideState.h"

class PINKCABVEHICLE_API FPinkCabWallrideController
{
public:
    static bool TryComputeResidualTimeoutSeconds(float TotalMassKg, float& OutSeconds);
    static float ComputeResidualTimeoutSeconds(float TotalMassKg);
    void Update(bool bValidContact, float DeltaSeconds, float TotalMassKg);
    void Abort();
    EPinkCabWallrideState GetState() const;
    float GetResidualSeconds() const;
    FPinkCabWallrideConstraintRequest BuildConstraintRequest(float TotalMassKg) const;

private:
    void HandleContact(float DeltaSeconds, float TotalMassKg, float ValidatedSeconds);
    void HandleNoContact(float DeltaSeconds);

    EPinkCabWallrideState State = EPinkCabWallrideState::Detached;
    float ResidualSeconds = 0.0f;
    float ReacquiredContactSeconds = 0.0f;
    bool bPendingReacquisitionReset = false;
};
