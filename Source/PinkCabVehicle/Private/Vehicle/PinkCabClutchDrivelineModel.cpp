#include "Vehicle/PinkCabClutchDrivelineModel.h"

namespace
{
constexpr float RpmToRadPerSecond = 2.0f * PI / 60.0f;
constexpr float RadPerSecondToRpm = 60.0f / (2.0f * PI);
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

    const float Coupling = FMath::Clamp(Input.ClutchCoupling01, 0.0f, 1.0f);
    const float ConditionCapacity =
        FMath::Clamp(Input.DrivetrainTorqueCapacity01, 0.0f, 1.0f);
    Output.TorqueCapacityNm =
        Config.MaxClutchTorqueNm * Coupling * ConditionCapacity;

    const float SlipOmega =
        Output.SlipRpm * RpmToRadPerSecond;
    const float SyncHorizon =
        FMath::Max(Config.SynchronizationTimeSeconds, Input.DeltaSeconds);

    // Positive clutch torque flows engine -> shaft. The synchronization term
    // opposes relative plate speed while the engine-demand term preserves
    // steady propulsion at zero slip. Equal/opposite engine reaction is
    // returned explicitly so the adapter can feed wheel load back to engine.
    const float SynchronizationTorqueNm =
        Config.EngineEffectiveInertia * SlipOmega / SyncHorizon;
    Output.RequestedClutchTorqueNm =
        FMath::Max(Input.AvailableEngineTorqueNm, 0.0f)
        + SynchronizationTorqueNm;

    Output.TransmittedClutchTorqueNm = FMath::Clamp(
        Output.RequestedClutchTorqueNm,
        -Output.TorqueCapacityNm,
        Output.TorqueCapacityNm);
    Output.bTorqueLimited = !FMath::IsNearlyEqual(
        Output.TransmittedClutchTorqueNm,
        Output.RequestedClutchTorqueNm,
        1.0e-3f);

    const float Efficiency =
        FMath::Clamp(Input.TransmissionEfficiency, 0.0f, 1.0f);
    Output.RearAxleTorqueNm =
        Output.TransmittedClutchTorqueNm
        * Input.EffectiveGearRatio
        * Efficiency;

    const float EngineReactionDeltaOmega =
        (-Output.TransmittedClutchTorqueNm
            / Config.EngineEffectiveInertia)
        * Input.DeltaSeconds;
    Output.EngineReactionDeltaRpm =
        EngineReactionDeltaOmega * RadPerSecondToRpm;

    const bool bNearLockedSpeed =
        FMath::Abs(Output.SlipRpm) <= Config.LockedSlipRpm;
    const bool bHasStaticCapacity =
        FMath::Abs(Output.RequestedClutchTorqueNm)
            <= Output.TorqueCapacityNm + 1.0e-3f;
    Output.State = (bNearLockedSpeed && bHasStaticCapacity)
        ? EPinkCabClutchDrivelineState::Locked
        : EPinkCabClutchDrivelineState::Slipping;
    return Output;
}
