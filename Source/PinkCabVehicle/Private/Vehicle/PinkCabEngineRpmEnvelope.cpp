#include "Vehicle/PinkCabEngineRpmEnvelope.h"

// Validation stays private so the public contract remains data-only and the
// RPM envelope has one implementation boundary for every runtime consumer.
// This file is also the exact-head source trigger for the final P02 verification.
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
