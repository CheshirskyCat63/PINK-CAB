#include "Vehicle/PinkCabClutchDrivelineModel.h"

namespace
{
constexpr float PinkCabClutchRpmToRadPerSecond = 2.0f * PI / 60.0f;
constexpr float PinkCabClutchRadPerSecondToRpm = 60.0f / (2.0f * PI);
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
    Output.SlipRpm = Input.EngineRpm - Input.ShaftEquivalentEngineRpm;

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
    const float ShaftOmega =
        Input.ShaftEquivalentEngineRpm
        * PinkCabClutchRpmToRadPerSecond;
    float VirtualEngineOmega =
        Input.EngineRpm * PinkCabClutchRpmToRadPerSecond;

    // Numerical integration is local to the clutch domain. It does not alter
    // the project/Chaos timestep and is derived from the authored physical
    // synchronization horizon rather than from render cadence.
    constexpr float IntegrationSlicesPerSynchronizationHorizon = 32.0f;
    const float MaxIntegrationStepSeconds =
        Config.SynchronizationTimeSeconds
        / IntegrationSlicesPerSynchronizationHorizon;
    const int32 IntegrationSteps = FMath::Max(
        1,
        FMath::CeilToInt(
            Input.DeltaSeconds
            / FMath::Max(
                MaxIntegrationStepSeconds,
                KINDA_SMALL_NUMBER)));
    const float IntegrationDeltaSeconds =
        Input.DeltaSeconds
        / static_cast<float>(IntegrationSteps);

    const float InitialSlipOmega =
        VirtualEngineOmega - ShaftOmega;
    Output.RequestedClutchTorqueNm =
        NetEngineTorqueNm
        + Config.EngineEffectiveInertia
            * InitialSlipOmega
            / Config.SynchronizationTimeSeconds;

    double TransmittedTorqueImpulseNmSeconds = 0.0;
    bool bAnyTorqueLimited = false;
    float FinalRequestedTorqueNm =
        Output.RequestedClutchTorqueNm;

    for (int32 StepIndex = 0;
         StepIndex < IntegrationSteps;
         ++StepIndex)
    {
        const float SlipOmega =
            VirtualEngineOmega - ShaftOmega;
        const float SynchronizationTorqueNm =
            Config.EngineEffectiveInertia
            * SlipOmega
            / Config.SynchronizationTimeSeconds;
        const float RequestedTorqueNm =
            NetEngineTorqueNm + SynchronizationTorqueNm;
        const float TransmittedTorqueNm = FMath::Clamp(
            RequestedTorqueNm,
            -Output.TorqueCapacityNm,
            Output.TorqueCapacityNm);

        bAnyTorqueLimited |= !FMath::IsNearlyEqual(
            TransmittedTorqueNm,
            RequestedTorqueNm,
            1.0e-3f);
        TransmittedTorqueImpulseNmSeconds +=
            static_cast<double>(TransmittedTorqueNm)
            * static_cast<double>(IntegrationDeltaSeconds);

        // The virtual state evolves with engine demand and clutch load
        // simultaneously. Runtime applies only the equal/opposite clutch
        // impulse to Chaos; accepted P01 native engine evolution remains the
        // actual combustion/free-engine authority.
        const float VirtualNetTorqueNm =
            NetEngineTorqueNm - TransmittedTorqueNm;
        VirtualEngineOmega +=
            (VirtualNetTorqueNm
                / Config.EngineEffectiveInertia)
            * IntegrationDeltaSeconds;
        FinalRequestedTorqueNm = RequestedTorqueNm;
    }

    Output.TransmittedClutchTorqueNm =
        static_cast<float>(
            TransmittedTorqueImpulseNmSeconds
            / static_cast<double>(Input.DeltaSeconds));
    Output.bTorqueLimited = bAnyTorqueLimited;

    const float Efficiency =
        FMath::Clamp(Input.TransmissionEfficiency, 0.0f, 1.0f);
    Output.RearAxleTorqueNm =
        Output.TransmittedClutchTorqueNm
        * Input.EffectiveGearRatio
        * Efficiency;

    const float EngineReactionDeltaOmega =
        static_cast<float>(
            -TransmittedTorqueImpulseNmSeconds
            / static_cast<double>(
                Config.EngineEffectiveInertia));
    Output.EngineReactionDeltaRpm =
        EngineReactionDeltaOmega
        * PinkCabClutchRadPerSecondToRpm;

    const float FinalSlipRpm =
        (VirtualEngineOmega - ShaftOmega)
        * PinkCabClutchRadPerSecondToRpm;
    const bool bNearLockedSpeed =
        FMath::Abs(FinalSlipRpm)
            <= Config.LockedSlipRpm;
    const bool bHasStaticCapacity =
        FMath::Abs(FinalRequestedTorqueNm)
            <= Output.TorqueCapacityNm + 1.0e-3f;
    Output.State =
        (bNearLockedSpeed && bHasStaticCapacity)
            ? EPinkCabClutchDrivelineState::Locked
            : EPinkCabClutchDrivelineState::Slipping;
    return Output;
}
