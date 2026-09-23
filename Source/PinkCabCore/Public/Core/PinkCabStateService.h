#pragma once

#include "CoreMinimal.h"
#include "Core/PinkCabStableId.h"

enum class EPinkCabResetScope : uint8
{
    Runtime,
    Workday,
    Profile
};

class IPinkCabStateService
{
public:
    virtual ~IPinkCabStateService() = default;

    virtual FPinkCabStableId GetServiceId() const = 0;
    virtual void ResetState(EPinkCabResetScope Scope) = 0;
};
