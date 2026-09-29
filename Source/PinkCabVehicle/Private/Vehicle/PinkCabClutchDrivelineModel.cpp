#include "Vehicle/PinkCabClutchDrivelineModel.h"

namespace
{
constexpr float PinkCabClutchRpmToRadPerSecond = 2.0f * PI / 60.0f;
constexpr float PinkCabClutchRadPerSecondToRpm = 60.0f / (2.0f * PI);
constexpr double PinkCabClutchTimeEpsilonSeconds = 1.0e-9;
constexpr float PinkCabClutchTorqueRegionEpsilonNm = 1.0e-4f;

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

        const float Residual = X * ExpX - Value;
        const float Next = X - Residual / Denominator;
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
        Config.LockedSlipRpm * PinkCabClutchRpmToRadPerSecond;
    if (LockedSlipOmega <= KINDA_SMALL_NUMBER)
    {
        return 1.0f / Config.SynchronizationTimeSeconds;
    }

    // Full healthy capacity defines compliance; coupling/wear only clamp torque.
    // s_ref=C_max/(I*k), s_ref*exp(-k*T)=s_lock.
    const float CapacityHorizonRatio =
        Config.MaxClutchTorqueNm
        * Config.SynchronizationTimeSeconds
        / (Config.EngineEffectiveInertia * LockedSlipOmega);
    const float HorizonExponent =
        PositiveLambertW(CapacityHorizonRatio);
    return HorizonExponent / Config.SynchronizationTimeSeconds;
}

struct FPinkCabClutchIntegrationResult
{
    float InitialRequestedTorqueNm = 0.0f;
    float AverageTransmittedTorqueNm = 0.0f;
    float EngineReactionDeltaRpm = 0.0f;
    float FinalRequestedTorqueNm = 0.0f;
    float FinalSlipRpm = 0.0f;
    bool bAnyTorqueLimited = false;
};

float RequestedTorqueNm(
    const float NetEngineTorqueNm,
    const float EngineInertia,
    const float SynchronizationGain,
    const float SlipOmega)
{
    return NetEngineTorqueNm
        + EngineInertia * SynchronizationGain * SlipOmega;
}

FPinkCabClutchIntegrationResult IntegrateClutchReaction(
    const FPinkCabClutchDrivelineConfig& Config,
    const FPinkCabClutchDrivelineInput& Input,
    const float TorqueCapacityNm,
    const float NetEngineTorqueNm)
{
    FPinkCabClutchIntegrationResult Result;
    const float Inertia = Config.EngineEffectiveInertia;
    const float Gain = SynchronizationGainPerSecond(Config);
    const float ShaftOmega =
        Input.ShaftEquivalentEngineRpm * PinkCabClutchRpmToRadPerSecond;
    float SlipOmega =
        Input.EngineRpm * PinkCabClutchRpmToRadPerSecond
        - ShaftOmega;

    if (Gain <= KINDA_SMALL_NUMBER)
    {
        return Result;
    }

    const float HighSlipBoundaryOmega =
        (TorqueCapacityNm - NetEngineTorqueNm)
        / (Inertia * Gain);
    const float LowSlipBoundaryOmega =
        (-TorqueCapacityNm - NetEngineTorqueNm)
        / (Inertia * Gain);

    Result.InitialRequestedTorqueNm = RequestedTorqueNm(
        NetEngineTorqueNm, Inertia, Gain, SlipOmega);

    double RemainingSeconds = static_cast<double>(Input.DeltaSeconds);
    double ClutchImpulse = 0.0;

    while (RemainingSeconds > PinkCabClutchTimeEpsilonSeconds)
    {
        const float Requested = RequestedTorqueNm(
            NetEngineTorqueNm, Inertia, Gain, SlipOmega);
        const bool bHighLimited =
            Requested > TorqueCapacityNm + PinkCabClutchTorqueRegionEpsilonNm
            || (NetEngineTorqueNm > TorqueCapacityNm
                && Requested
                    >= TorqueCapacityNm
                        - PinkCabClutchTorqueRegionEpsilonNm);
        const bool bLowLimited =
            Requested < -TorqueCapacityNm - PinkCabClutchTorqueRegionEpsilonNm
            || (NetEngineTorqueNm < -TorqueCapacityNm
                && Requested
                    <= -TorqueCapacityNm
                        + PinkCabClutchTorqueRegionEpsilonNm);

        if (bHighLimited)
        {
            Result.bAnyTorqueLimited = true;
            const float SlipRate =
                (NetEngineTorqueNm - TorqueCapacityNm) / Inertia;
            double SegmentSeconds = RemainingSeconds;
            if (SlipRate < -KINDA_SMALL_NUMBER)
            {
                const double ToBoundarySeconds =
                    static_cast<double>(
                        HighSlipBoundaryOmega - SlipOmega)
                    / static_cast<double>(SlipRate);
                if (ToBoundarySeconds
                        > PinkCabClutchTimeEpsilonSeconds
                    && ToBoundarySeconds < SegmentSeconds)
                {
                    SegmentSeconds = ToBoundarySeconds;
                }
            }

            ClutchImpulse +=
                static_cast<double>(TorqueCapacityNm) * SegmentSeconds;
            SlipOmega += SlipRate * static_cast<float>(SegmentSeconds);
            RemainingSeconds -= SegmentSeconds;
            if (RemainingSeconds > PinkCabClutchTimeEpsilonSeconds)
            {
                SlipOmega = HighSlipBoundaryOmega;
            }
            continue;
        }

        if (bLowLimited)
        {
            Result.bAnyTorqueLimited = true;
            const float SlipRate =
                (NetEngineTorqueNm + TorqueCapacityNm) / Inertia;
            double SegmentSeconds = RemainingSeconds;
            if (SlipRate > KINDA_SMALL_NUMBER)
            {
                const double ToBoundarySeconds =
                    static_cast<double>(
                        LowSlipBoundaryOmega - SlipOmega)
                    / static_cast<double>(SlipRate);
                if (ToBoundarySeconds
                        > PinkCabClutchTimeEpsilonSeconds
                    && ToBoundarySeconds < SegmentSeconds)
                {
                    SegmentSeconds = ToBoundarySeconds;
                }
            }

            ClutchImpulse -=
                static_cast<double>(TorqueCapacityNm) * SegmentSeconds;
            SlipOmega += SlipRate * static_cast<float>(SegmentSeconds);
            RemainingSeconds -= SegmentSeconds;
            if (RemainingSeconds > PinkCabClutchTimeEpsilonSeconds)
            {
                SlipOmega = LowSlipBoundaryOmega;
            }
            continue;
        }

        double SegmentSeconds = RemainingSeconds;
        bool bExitToHighLimit = false;
        bool bExitToLowLimit = false;

        if (NetEngineTorqueNm > TorqueCapacityNm
            && HighSlipBoundaryOmega < -KINDA_SMALL_NUMBER
            && SlipOmega < HighSlipBoundaryOmega)
        {
            const double ToBoundarySeconds =
                FMath::Loge(
                    static_cast<double>(
                        SlipOmega / HighSlipBoundaryOmega))
                / static_cast<double>(Gain);
            if (ToBoundarySeconds
                    > PinkCabClutchTimeEpsilonSeconds
                && ToBoundarySeconds < SegmentSeconds)
            {
                SegmentSeconds = ToBoundarySeconds;
                bExitToHighLimit = true;
            }
        }
        else if (NetEngineTorqueNm < -TorqueCapacityNm
            && LowSlipBoundaryOmega > KINDA_SMALL_NUMBER
            && SlipOmega > LowSlipBoundaryOmega)
        {
            const double ToBoundarySeconds =
                FMath::Loge(
                    static_cast<double>(
                        SlipOmega / LowSlipBoundaryOmega))
                / static_cast<double>(Gain);
            if (ToBoundarySeconds
                    > PinkCabClutchTimeEpsilonSeconds
                && ToBoundarySeconds < SegmentSeconds)
            {
                SegmentSeconds = ToBoundarySeconds;
                bExitToLowLimit = true;
            }
        }

        const float FinalSegmentSlipOmega =
            SlipOmega
            * FMath::Exp(
                -Gain * static_cast<float>(SegmentSeconds));
        ClutchImpulse +=
            static_cast<double>(NetEngineTorqueNm) * SegmentSeconds
            + static_cast<double>(Inertia)
                * static_cast<double>(
                    SlipOmega - FinalSegmentSlipOmega);
        SlipOmega = FinalSegmentSlipOmega;
        RemainingSeconds -= SegmentSeconds;

        if (bExitToHighLimit)
        {
            SlipOmega = HighSlipBoundaryOmega;
        }
        else if (bExitToLowLimit)
        {
            SlipOmega = LowSlipBoundaryOmega;
        }
    }

    Result.AverageTransmittedTorqueNm = static_cast<float>(
        ClutchImpulse / static_cast<double>(Input.DeltaSeconds));
    Result.EngineReactionDeltaRpm = static_cast<float>(
        -ClutchImpulse / static_cast<double>(Inertia))
        * PinkCabClutchRadPerSecondToRpm;
    Result.FinalRequestedTorqueNm = RequestedTorqueNm(
        NetEngineTorqueNm, Inertia, Gain, SlipOmega);
    Result.FinalSlipRpm =
        SlipOmega * PinkCabClutchRadPerSecondToRpm;
    return Result;
}
}

bool FPinkCabClutchDrivelineConfig::IsValid() const
{
    return EngineEffectiveInertia > KINDA_SMALL_NUMBER
        && MaxClutchTorqueNm > KINDA_SMALL_NUMBER
        && SynchronizationTimeSeconds > KINDA_SMALL_NUMBER
        && LockedSlipRpm >= 0.0f;
}

FPinkCabClutchDrivelineModel::FPinkCabClutchDrivelineModel(
    const FPinkCabClutchDrivelineConfig& InConfig)
    : Config(InConfig)
{
}

void FPinkCabClutchDrivelineModel::SetConfig(
    const FPinkCabClutchDrivelineConfig& InConfig)
{
    Config = InConfig;
}

FPinkCabClutchDrivelineOutput FPinkCabClutchDrivelineModel::Step(
    const FPinkCabClutchDrivelineInput& Input) const
{
    FPinkCabClutchDrivelineOutput Output;
    Output.SlipRpm =
        Input.EngineRpm - Input.ShaftEquivalentEngineRpm;

    if (!Config.IsValid()
        || Input.DeltaSeconds <= KINDA_SMALL_NUMBER
        || FMath::Abs(Input.EffectiveGearRatio) <= KINDA_SMALL_NUMBER
        || Input.ClutchCoupling01 <= KINDA_SMALL_NUMBER
        || Input.DrivetrainTorqueCapacity01 <= KINDA_SMALL_NUMBER)
    {
        return Output;
    }

    const float Coupling =
        FMath::Clamp(Input.ClutchCoupling01, 0.0f, 1.0f);
    const float ConditionCapacity =
        FMath::Clamp(Input.DrivetrainTorqueCapacity01, 0.0f, 1.0f);
    Output.TorqueCapacityNm =
        Config.MaxClutchTorqueNm * Coupling * ConditionCapacity;

    const float NetEngineTorqueNm =
        FMath::Max(Input.AvailableEngineTorqueNm, 0.0f)
        - FMath::Max(Input.EngineDragTorqueNm, 0.0f);
    const FPinkCabClutchIntegrationResult Integrated =
        IntegrateClutchReaction(
            Config,
            Input,
            Output.TorqueCapacityNm,
            NetEngineTorqueNm);

    Output.RequestedClutchTorqueNm =
        Integrated.InitialRequestedTorqueNm;
    Output.TransmittedClutchTorqueNm =
        Integrated.AverageTransmittedTorqueNm;
    Output.EngineReactionDeltaRpm =
        Integrated.EngineReactionDeltaRpm;
    Output.bTorqueLimited =
        Integrated.bAnyTorqueLimited;

    const float Efficiency =
        FMath::Clamp(Input.TransmissionEfficiency, 0.0f, 1.0f);
    Output.RearAxleTorqueNm =
        Output.TransmittedClutchTorqueNm
        * Input.EffectiveGearRatio
        * Efficiency;

    const bool bNearLockedSpeed =
        FMath::Abs(Integrated.FinalSlipRpm)
            <= Config.LockedSlipRpm;
    const bool bHasStaticCapacity =
        FMath::Abs(Integrated.FinalRequestedTorqueNm)
            <= Output.TorqueCapacityNm + 1.0e-3f;
    Output.State =
        (bNearLockedSpeed && bHasStaticCapacity)
            ? EPinkCabClutchDrivelineState::Locked
            : EPinkCabClutchDrivelineState::Slipping;
    return Output;
}
