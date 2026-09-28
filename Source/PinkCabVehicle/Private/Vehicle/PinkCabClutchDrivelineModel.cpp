#include "Vehicle/PinkCabClutchDrivelineModel.h"

namespace
{
constexpr float PinkCabClutchRpmToRadPerSecond = 2.0f * PI / 60.0f;
constexpr float PinkCabClutchRadPerSecondToRpm = 60.0f / (2.0f * PI);
constexpr float PinkCabIntegrationSlicesPerSyncHorizon = 32.0f;

struct FPinkCabClutchIntegrationResult
{
    float InitialRequestedTorqueNm = 0.0f;
    float AverageTransmittedTorqueNm = 0.0f;
    float EngineReactionDeltaRpm = 0.0f;
    float FinalRequestedTorqueNm = 0.0f;
    float FinalSlipRpm = 0.0f;
    bool bAnyTorqueLimited = false;
};

FPinkCabClutchIntegrationResult IntegrateClutchReaction(
    const FPinkCabClutchDrivelineConfig& Config,
    const FPinkCabClutchDrivelineInput& Input,
    const float TorqueCapacityNm,
    const float NetEngineTorqueNm)
{
    FPinkCabClutchIntegrationResult Result;
    const float ShaftOmega =
        Input.ShaftEquivalentEngineRpm * PinkCabClutchRpmToRadPerSecond;
    float VirtualEngineOmega =
        Input.EngineRpm * PinkCabClutchRpmToRadPerSecond;

    const float MaxStep =
        Config.SynchronizationTimeSeconds
        / PinkCabIntegrationSlicesPerSyncHorizon;
    const int32 Steps = FMath::Max(
        1,
        FMath::CeilToInt(
            Input.DeltaSeconds
            / FMath::Max(MaxStep, KINDA_SMALL_NUMBER)));
    const float StepSeconds =
        Input.DeltaSeconds / static_cast<float>(Steps);

    double ClutchImpulse = 0.0;
    for (int32 Index = 0; Index < Steps; ++Index)
    {
        const float SlipOmega = VirtualEngineOmega - ShaftOmega;
        const float RequestedTorque =
            NetEngineTorqueNm
            + Config.EngineEffectiveInertia
                * SlipOmega
                / Config.SynchronizationTimeSeconds;
        const float TransmittedTorque = FMath::Clamp(
            RequestedTorque,
            -TorqueCapacityNm,
            TorqueCapacityNm);

        if (Index == 0)
        {
            Result.InitialRequestedTorqueNm = RequestedTorque;
        }
        Result.FinalRequestedTorqueNm = RequestedTorque;
        Result.bAnyTorqueLimited |= !FMath::IsNearlyEqual(
            TransmittedTorque,
            RequestedTorque,
            1.0e-3f);
        ClutchImpulse +=
            static_cast<double>(TransmittedTorque)
            * static_cast<double>(StepSeconds);

        const float VirtualNetTorque =
            NetEngineTorqueNm - TransmittedTorque;
        VirtualEngineOmega +=
            VirtualNetTorque
            / Config.EngineEffectiveInertia
            * StepSeconds;
    }

    Result.AverageTransmittedTorqueNm = static_cast<float>(
        ClutchImpulse / static_cast<double>(Input.DeltaSeconds));
    Result.EngineReactionDeltaRpm = static_cast<float>(
        -ClutchImpulse
        / static_cast<double>(Config.EngineEffectiveInertia))
        * PinkCabClutchRadPerSecondToRpm;
    Result.FinalSlipRpm =
        (VirtualEngineOmega - ShaftOmega)
        * PinkCabClutchRadPerSecondToRpm;
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
