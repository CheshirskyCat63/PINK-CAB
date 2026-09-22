#pragma once

#include "CoreMinimal.h"

struct PINKCABINTERACTION_API FPinkCabWheelInputResponseConfig
{
    double BurstResetSeconds = 0.30;
    float GrowthBase = 1.35f;
    int32 MaxMultiplier = 6;
    int32 MaxOutputSteps = 8;
};

class PINKCABINTERACTION_API FPinkCabWheelInputResponse
{
public:
    explicit FPinkCabWheelInputResponse(
        const FPinkCabWheelInputResponseConfig& InConfig = {});

    int32 Apply(int32 RawSignedSteps, double NowSeconds);
    void Reset();

    int32 GetBurstCount() const { return BurstCount; }
    int32 GetLastDirection() const { return LastDirection; }

private:
    FPinkCabWheelInputResponseConfig Config;
    double LastStepSeconds = -1.0;
    int32 BurstCount = 0;
    int32 LastDirection = 0;
};
