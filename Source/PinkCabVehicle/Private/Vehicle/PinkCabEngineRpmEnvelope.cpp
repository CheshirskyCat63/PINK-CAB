#include "Vehicle/PinkCabEngineRpmEnvelope.h"

bool FPinkCabEngineRpmEnvelope::IsValid() const
{
    return FMath::IsFinite(IdleRpm)
        && FMath::IsFinite(RedZoneStartRpm)
        && FMath::IsFinite(LimiterHardCutRpm)
        && FMath::IsFinite(DamageOverspeedRpm)
        && IdleRpm > 0.0f
        && IdleRpm < RedZoneStartRpm
        && RedZoneStartRpm < LimiterHardCutRpm
        && LimiterHardCutRpm <= DamageOverspeedRpm;
}
