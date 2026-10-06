#include "PinkCabClutchDrivelineIntegration.h"

namespace
{
constexpr float RpmToRadPerSecond = 2.0f * PI / 60.0f;
constexpr float RadPerSecondToRpm = 60.0f / (2.0f * PI);
constexpr double TimeEpsilonSeconds = 1.0e-9;
constexpr float TorqueRegionEpsilonNm = 1.0e-4f;

struct FState
{
    float SlipOmega = 0.0f;
    double RemainingSeconds = 0.0;
    double ClutchImpulse = 0.0;
    bool bAnyTorqueLimited = false;
};

float PositiveLambertW(const float Value)
{
    if (!FMath::IsFinite(Value) || Value <= KINDA_SMALL_NUMBER)
    {
        return 0.0f;
    }

    float X = Value < 3.0f
        ? Value / (1.0f + 0.5f * Value)
        : FMath::Loge(Value) - FMath::Loge(FMath::Loge(Value));
    X = FMath::Max(X, 0.0f);

    for (int32 Index = 0; Index < 8; ++Index)
    {
        const float ExpX = FMath::Exp(X);
        const float Denominator = ExpX * (X + 1.0f);
        if (!FMath::IsFinite(ExpX)
            || FMath::Abs(Denominator) <= KINDA_SMALL_NUMBER)
        {
            break;
        }

        const float Next =
            X - (X * ExpX - Value) / Denominator;
        if (!FMath::IsFinite(Next))
        {
            break;
        }
        X = FMath::Max(Next, 0.0f);
    }
    return X;
}

float SynchronizationGainPerSecond(
    const FPinkCabClutchDrivelineConfig& Config)
{
    if (Config.MaxClutchTorqueNm <= KINDA_SMALL_NUMBER
        || Config.EngineEffectiveInertia <= KINDA_SMALL_NUMBER
        || Config.SynchronizationTimeSeconds <= KINDA_SMALL_NUMBER)
    {
        return 0.0f;
    }

    const float LockedSlipOmega =
        Config.LockedSlipRpm * RpmToRadPerSecond;
    if (LockedSlipOmega <= KINDA_SMALL_NUMBER)
    {
        return 1.0f / Config.SynchronizationTimeSeconds;
    }

    const float Ratio =
        Config.MaxClutchTorqueNm
        * Config.SynchronizationTimeSeconds
        / (Config.EngineEffectiveInertia * LockedSlipOmega);
    return PositiveLambertW(Ratio)
        / Config.SynchronizationTimeSeconds;
}

float RequestedTorqueNm(
    const float NetEngineTorqueNm,
    const float Inertia,
    const float Gain,
    const float SlipOmega)
{
    return NetEngineTorqueNm
        + Inertia * Gain * SlipOmega;
}

bool AdvanceHighLimited(
    FState& State,
    const float NetEngineTorqueNm,
    const float Inertia,
    const float CapacityNm,
    const float Gain,
    const float HighBoundaryOmega)
{
    const float Requested = RequestedTorqueNm(
        NetEngineTorqueNm, Inertia, Gain, State.SlipOmega);
    const bool bLimited =
        Requested > CapacityNm + TorqueRegionEpsilonNm
        || (NetEngineTorqueNm > CapacityNm
            && Requested >= CapacityNm - TorqueRegionEpsilonNm);
    if (!bLimited)
    {
        return false;
    }

    State.bAnyTorqueLimited = true;
    const float SlipRate =
        (NetEngineTorqueNm - CapacityNm) / Inertia;
    double SegmentSeconds = State.RemainingSeconds;
    if (SlipRate < -KINDA_SMALL_NUMBER)
    {
        const double ToBoundarySeconds =
            static_cast<double>(
                HighBoundaryOmega - State.SlipOmega)
            / static_cast<double>(SlipRate);
        if (ToBoundarySeconds > TimeEpsilonSeconds
            && ToBoundarySeconds < SegmentSeconds)
        {
            SegmentSeconds = ToBoundarySeconds;
        }
    }

    State.ClutchImpulse +=
        static_cast<double>(CapacityNm) * SegmentSeconds;
    State.SlipOmega +=
        SlipRate * static_cast<float>(SegmentSeconds);
    State.RemainingSeconds -= SegmentSeconds;
    if (State.RemainingSeconds > TimeEpsilonSeconds)
    {
        State.SlipOmega = HighBoundaryOmega;
    }
    return true;
}

bool AdvanceLowLimited(
    FState& State,
    const float NetEngineTorqueNm,
    const float Inertia,
    const float CapacityNm,
    const float Gain,
    const float LowBoundaryOmega)
{
    const float Requested = RequestedTorqueNm(
        NetEngineTorqueNm, Inertia, Gain, State.SlipOmega);
    const bool bLimited =
        Requested < -CapacityNm - TorqueRegionEpsilonNm
        || (NetEngineTorqueNm < -CapacityNm
            && Requested <= -CapacityNm + TorqueRegionEpsilonNm);
    if (!bLimited)
    {
        return false;
    }

    State.bAnyTorqueLimited = true;
    const float SlipRate =
        (NetEngineTorqueNm + CapacityNm) / Inertia;
    double SegmentSeconds = State.RemainingSeconds;
    if (SlipRate > KINDA_SMALL_NUMBER)
    {
        const double ToBoundarySeconds =
            static_cast<double>(
                LowBoundaryOmega - State.SlipOmega)
            / static_cast<double>(SlipRate);
        if (ToBoundarySeconds > TimeEpsilonSeconds
            && ToBoundarySeconds < SegmentSeconds)
        {
            SegmentSeconds = ToBoundarySeconds;
        }
    }

    State.ClutchImpulse -=
        static_cast<double>(CapacityNm) * SegmentSeconds;
    State.SlipOmega +=
        SlipRate * static_cast<float>(SegmentSeconds);
    State.RemainingSeconds -= SegmentSeconds;
    if (State.RemainingSeconds > TimeEpsilonSeconds)
    {
        State.SlipOmega = LowBoundaryOmega;
    }
    return true;
}

void AdvanceUnclamped(
    FState& State,
    const float NetEngineTorqueNm,
    const float Inertia,
    const float CapacityNm,
    const float Gain,
    const float HighBoundaryOmega,
    const float LowBoundaryOmega)
{
    double SegmentSeconds = State.RemainingSeconds;
    bool bExitHigh = false;
    bool bExitLow = false;

    if (NetEngineTorqueNm > CapacityNm
        && HighBoundaryOmega < -KINDA_SMALL_NUMBER
        && State.SlipOmega < HighBoundaryOmega)
    {
        const double ToBoundarySeconds =
            FMath::Loge(static_cast<double>(
                State.SlipOmega / HighBoundaryOmega))
            / static_cast<double>(Gain);
        if (ToBoundarySeconds > TimeEpsilonSeconds
            && ToBoundarySeconds < SegmentSeconds)
        {
            SegmentSeconds = ToBoundarySeconds;
            bExitHigh = true;
        }
    }
    else if (NetEngineTorqueNm < -CapacityNm
        && LowBoundaryOmega > KINDA_SMALL_NUMBER
        && State.SlipOmega > LowBoundaryOmega)
    {
        const double ToBoundarySeconds =
            FMath::Loge(static_cast<double>(
                State.SlipOmega / LowBoundaryOmega))
            / static_cast<double>(Gain);
        if (ToBoundarySeconds > TimeEpsilonSeconds
            && ToBoundarySeconds < SegmentSeconds)
        {
            SegmentSeconds = ToBoundarySeconds;
            bExitLow = true;
        }
    }

    const float FinalSlip =
        State.SlipOmega
        * FMath::Exp(-Gain * static_cast<float>(SegmentSeconds));
    State.ClutchImpulse +=
        static_cast<double>(NetEngineTorqueNm) * SegmentSeconds
        + static_cast<double>(Inertia)
            * static_cast<double>(State.SlipOmega - FinalSlip);
    State.SlipOmega = FinalSlip;
    State.RemainingSeconds -= SegmentSeconds;

    if (bExitHigh)
    {
        State.SlipOmega = HighBoundaryOmega;
    }
    else if (bExitLow)
    {
        State.SlipOmega = LowBoundaryOmega;
    }
}
}

PinkCabClutchIntegration::FResult PinkCabClutchIntegration::Integrate(
    const FPinkCabClutchDrivelineConfig& Config,
    const FPinkCabClutchDrivelineInput& Input,
    const float TorqueCapacityNm,
    const float NetEngineTorqueNm)
{
    FResult Result;
    const float Inertia = Config.EngineEffectiveInertia;
    const float Gain = SynchronizationGainPerSecond(Config);
    if (Gain <= KINDA_SMALL_NUMBER)
    {
        return Result;
    }

    // Relative clutch slip is engine omega minus shaft-equivalent omega.
    // The old predictor treated the shaft target as stationary during each
    // physics step, creating a systematic lag whenever the driven wheels were
    // accelerating. Convert the measured same-step shaft acceleration into
    // the equivalent torque term required by ds/dt.
    const float ShaftAccelerationOmegaPerSecondSquared =
        Input.ShaftEquivalentEngineRpmPerSecond * RpmToRadPerSecond;
    const float RelativeNetTorqueNm =
        NetEngineTorqueNm
        - Inertia * ShaftAccelerationOmegaPerSecondSquared;

    FState State;
    State.SlipOmega =
        (Input.EngineRpm - Input.ShaftEquivalentEngineRpm)
        * RpmToRadPerSecond;
    State.RemainingSeconds =
        static_cast<double>(Input.DeltaSeconds);

    const float HighBoundary =
        (TorqueCapacityNm - RelativeNetTorqueNm)
        / (Inertia * Gain);
    const float LowBoundary =
        (-TorqueCapacityNm - RelativeNetTorqueNm)
        / (Inertia * Gain);
    Result.InitialRequestedTorqueNm = RequestedTorqueNm(
        RelativeNetTorqueNm, Inertia, Gain, State.SlipOmega);

    while (State.RemainingSeconds > TimeEpsilonSeconds)
    {
        if (AdvanceHighLimited(
                State, RelativeNetTorqueNm, Inertia,
                TorqueCapacityNm, Gain, HighBoundary))
        {
            continue;
        }
        if (AdvanceLowLimited(
                State, RelativeNetTorqueNm, Inertia,
                TorqueCapacityNm, Gain, LowBoundary))
        {
            continue;
        }
        AdvanceUnclamped(
            State, RelativeNetTorqueNm, Inertia,
            TorqueCapacityNm, Gain, HighBoundary, LowBoundary);
    }

    Result.AverageTransmittedTorqueNm = static_cast<float>(
        State.ClutchImpulse
        / static_cast<double>(Input.DeltaSeconds));
    Result.EngineReactionDeltaRpm = static_cast<float>(
        -State.ClutchImpulse / static_cast<double>(Inertia))
        * RadPerSecondToRpm;
    Result.FinalRequestedTorqueNm = RequestedTorqueNm(
        RelativeNetTorqueNm, Inertia, Gain, State.SlipOmega);
    Result.FinalSlipRpm =
        State.SlipOmega * RadPerSecondToRpm;
    Result.bAnyTorqueLimited = State.bAnyTorqueLimited;
    return Result;
}
