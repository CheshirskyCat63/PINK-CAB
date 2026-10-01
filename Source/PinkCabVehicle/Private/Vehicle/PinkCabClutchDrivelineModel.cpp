#include "Vehicle/PinkCabClutchDrivelineModel.h"

#include "PinkCabClutchDrivelineIntegration.h"

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
    const PinkCabClutchIntegration::FResult Integrated =
        PinkCabClutchIntegration::Integrate(
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
