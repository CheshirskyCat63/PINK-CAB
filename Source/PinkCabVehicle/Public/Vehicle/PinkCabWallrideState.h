#pragma once

#include "CoreMinimal.h"

enum class EPinkCabWallrideState : uint8
{
    Detached,
    Contact,
    Residual,
    Expired
};

struct FPinkCabWallrideConstraintRequest
{
    bool bRequested = false;
    bool bForceApplied = false;
    float TotalMassKg = 0.0f;
    EPinkCabWallrideState State = EPinkCabWallrideState::Detached;
};
