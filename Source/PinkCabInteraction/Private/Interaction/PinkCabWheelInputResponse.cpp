#include "Interaction/PinkCabWheelInputResponse.h"

FPinkCabWheelInputResponse::FPinkCabWheelInputResponse(
    const FPinkCabWheelInputResponseConfig& InConfig)
    : Config(InConfig)
{
}

int32 FPinkCabWheelInputResponse::Apply(
    const int32 RawSignedSteps,
    const double NowSeconds)
{
    if (RawSignedSteps == 0)
    {
        return 0;
    }

    const int32 Direction = FMath::Sign(RawSignedSteps);
    const int32 RawMagnitude = FMath::Abs(RawSignedSteps);
    const bool bBurstContinues =
        Direction == LastDirection
        && LastStepSeconds >= 0.0
        && NowSeconds >= LastStepSeconds
        && (NowSeconds - LastStepSeconds) <= FMath::Max(Config.BurstResetSeconds, 0.0);

    BurstCount = bBurstContinues
        ? BurstCount + RawMagnitude
        : RawMagnitude;
    LastDirection = Direction;
    LastStepSeconds = NowSeconds;

    const float Growth = FMath::Pow(
        FMath::Max(Config.GrowthBase, 1.0f),
        static_cast<float>(FMath::Max(BurstCount - 1, 0)));
    const int32 Multiplier = FMath::Clamp(
        FMath::RoundToInt(Growth),
        1,
        FMath::Max(Config.MaxMultiplier, 1));

    const int32 AcceleratedMagnitude = FMath::Clamp(
        RawMagnitude * Multiplier,
        1,
        FMath::Max(Config.MaxOutputSteps, 1));
    return Direction * AcceleratedMagnitude;
}

void FPinkCabWheelInputResponse::Reset()
{
    LastStepSeconds = -1.0;
    BurstCount = 0;
    LastDirection = 0;
}
