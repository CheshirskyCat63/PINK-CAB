#pragma once

#include "CoreMinimal.h"

struct PINKCABVEHICLE_API FPinkCabEngineRpmEnvelope
{
    float IdleRpm = 0.0f;
    float RedZoneStartRpm = 0.0f;
    float LimiterHardCutRpm = 0.0f;
    float DamageOverspeedRpm = 0.0f;

    bool IsValid() const;
};
