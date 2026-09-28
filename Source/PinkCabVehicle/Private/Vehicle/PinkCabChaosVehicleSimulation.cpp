#include "PinkCabChaosVehicleSimulation.h"

#include "EngineSystem.h"
#include "SimpleVehicle.h"
#include "VehicleUtility.h"
#include "WheelSystem.h"

namespace
{
constexpr float PinkCabChaosRpmToRadPerSecond = 2.0f * PI / 60.0f;

float MeanDrivenWheelRpm(Chaos::FSimpleWheeledVehicle& Vehicle)
{
    float Sum = 0.0f;
    int32 Count = 0;
    for (int32 Index = 0; Index < Vehicle.Wheels.Num(); ++Index)
    {
        Chaos::FSimpleWheelSim& Wheel = Vehicle.Wheels[Index];
        if (!Wheel.Setup().EngineEnabled)
        {
            continue;
        }
        Sum += FMath::Abs(Wheel.GetWheelRPM());
        ++Count;
    }
    return Count > 0 ? Sum / static_cast<float>(Count) : 0.0f;
}
}

FPinkCabChaosWheeledVehicleSimulation::FPinkCabChaosWheeledVehicleSimulation(
    FThreadSafeCounter64* InMechanicalIntegrationStepCounter,
    FThreadSafeCounter64* InMechanicalIntegrationDeltaMicros)
    : MechanicalIntegrationStepCounter(InMechanicalIntegrationStepCounter)
    , MechanicalIntegrationDeltaMicros(InMechanicalIntegrationDeltaMicros)
{
}

void FPinkCabChaosWheeledVehicleSimulation::SetDrivelineCommand(
    const FPinkCabChaosDrivelineCommand& InCommand)
{
    Command = InCommand;
    ClutchModel.SetConfig(Command.ClutchConfig);
}

void FPinkCabChaosWheeledVehicleSimulation::BeginEvidenceWindow(
    const float InSettleSeconds,
    const float InSampleSeconds)
{
    EvidenceSettleSecondsRemaining =
        FMath::Max(static_cast<double>(InSettleSeconds), 0.0);
    EvidenceTargetSampleSeconds =
        FMath::Max(static_cast<double>(InSampleSeconds), 1.0e-4);
    EvidenceCompletedSampleSeconds = 0.0;
    EvidenceCompletedSteps = 0;
    EvidenceDrivenWheelTorqueTimeIntegral = 0.0;
    EvidenceEngineRpmTimeIntegral = 0.0;
    EvidenceObservedDeltaSecondsSum = 0.0;
}

FPinkCabMechanicalEvidenceSnapshot
FPinkCabChaosWheeledVehicleSimulation::ReadEvidenceWindow() const
{
    FPinkCabMechanicalEvidenceSnapshot Result;
    Result.CompletedSampleSteps = EvidenceCompletedSteps;
    Result.TargetSampleSeconds =
        static_cast<float>(EvidenceTargetSampleSeconds);
    Result.CompletedSampleSeconds =
        static_cast<float>(EvidenceCompletedSampleSeconds);
    if (EvidenceCompletedSampleSeconds > 0.0)
    {
        Result.MeanDrivenWheelTorqueNm = static_cast<float>(
            EvidenceDrivenWheelTorqueTimeIntegral
            / EvidenceCompletedSampleSeconds);
        Result.MeanEngineRpm = static_cast<float>(
            EvidenceEngineRpmTimeIntegral
            / EvidenceCompletedSampleSeconds);
    }
    if (EvidenceCompletedSteps > 0)
    {
        Result.MeanDeltaSeconds = static_cast<float>(
            EvidenceObservedDeltaSecondsSum
            / static_cast<double>(EvidenceCompletedSteps));
    }
    Result.bComplete =
        EvidenceTargetSampleSeconds > 0.0
        && EvidenceCompletedSampleSeconds
            + 1.0e-9 >= EvidenceTargetSampleSeconds;
    return Result;
}

void FPinkCabChaosWheeledVehicleSimulation::ApplyInput(
    const FControlInputs& ControlInputs,
    const float DeltaTime)
{
    using namespace Chaos;

    UChaosWheeledVehicleSimulation::ApplyInput(ControlInputs, DeltaTime);
    if (!PVehicle)
    {
        return;
    }

    PhysicsThreadActuation = {};
    if (PVehicle->HasEngine())
    {
        FSimpleEngineSim& Engine = PVehicle->GetEngine();
        const float EngineRpm = Engine.GetEngineRPM();

        FPinkCabEngineActuationInput ActuationInput;
        ActuationInput.bCombustionAllowed = Command.bCombustionAllowed;
        ActuationInput.HealthClampedControlThrottle01 =
            Command.HealthClampedControlThrottle01;
        ActuationInput.EngineRpm = EngineRpm;
        ActuationInput.MaxRpm = Engine.Setup().MaxRPM;
        ActuationInput.EngineTorqueCurveNm =
            Engine.GetTorqueFromRPM(EngineRpm, false);
        PhysicsThreadActuation =
            FPinkCabEngineActuationResolver::Resolve(ActuationInput);
        Engine.SetThrottle(PhysicsThreadActuation.EngineThrottleFinal01);
    }

    for (int32 WheelIndex = 0;
         WheelIndex < PVehicle->Wheels.Num();
         ++WheelIndex)
    {
        FSimpleWheelSim& Wheel = PVehicle->Wheels[WheelIndex];
        const FSimpleWheelConfig& Setup = Wheel.Setup();

        float BrakeTorqueNm = 0.0f;
        if (Setup.BrakeEnabled)
        {
            BrakeTorqueNm = Setup.MaxBrakeTorque
                * FMath::Clamp(ControlInputs.BrakeInput, 0.0f, 1.0f);
        }

        if ((Command.Handbrake01 > KINDA_SMALL_NUMBER
                && Setup.HandbrakeEnabled)
            || ControlInputs.ParkingEnabled)
        {
            const float HandbrakeTorqueNm =
                ControlInputs.ParkingEnabled
                    ? Setup.HandbrakeTorque
                    : Setup.HandbrakeTorque
                        * FMath::Clamp(Command.Handbrake01, 0.0f, 1.0f);
            BrakeTorqueNm =
                FMath::Max(BrakeTorqueNm, HandbrakeTorqueNm);
        }

        Wheel.SetBrakeTorque(TorqueMToCm(BrakeTorqueNm), false);
    }
}

void FPinkCabChaosWheeledVehicleSimulation::ProcessMechanicalSimulation(
    const float DeltaTime)
{
    using namespace Chaos;

    if (!PVehicle || !PVehicle->HasEngine() || !PVehicle->HasTransmission())
    {
        return;
    }

    FSimpleEngineSim& Engine = PVehicle->GetEngine();
    FSimpleTransmissionSim& Transmission = PVehicle->GetTransmission();
    Transmission.SetGear(0, true);

    AdvanceAcceptedNativeEngine(Transmission, DeltaTime);

    const FPinkCabClutchDrivelineOutput Output =
        SolveDrivelineStep(Engine, DeltaTime);
    ApplyEngineReaction(Engine, Output);

    const FDrivenWheelTorqueStats WheelStats =
        ApplyDrivenWheelTorque(Output);
    AccumulateEvidenceStep(Engine, WheelStats, DeltaTime);
    PublishMechanicalStep(DeltaTime);
}

void FPinkCabChaosWheeledVehicleSimulation::AdvanceAcceptedNativeEngine(
    Chaos::FSimpleTransmissionSim& Transmission,
    const float DeltaTime)
{
    if (!Command.bCombustionAllowed)
    {
        return;
    }

    // P01 accepted the stock Chaos engine transient. Native transmission stays
    // neutral before and after the stock mechanical step, so this preserves the
    // engine authority without restoring a second propulsion path.
    Transmission.SetGear(0, true);
    UChaosWheeledVehicleSimulation::ProcessMechanicalSimulation(DeltaTime);
    Transmission.SetGear(0, true);
}

FPinkCabClutchDrivelineOutput
FPinkCabChaosWheeledVehicleSimulation::SolveDrivelineStep(
    Chaos::FSimpleEngineSim& Engine,
    const float DeltaTime)
{
    const float DrivenWheelRpm = MeanDrivenWheelRpm(*PVehicle);
    const float ShaftEquivalentEngineRpm =
        FMath::Abs(Command.EffectiveGearRatio) > KINDA_SMALL_NUMBER
            ? DrivenWheelRpm * FMath::Abs(Command.EffectiveGearRatio)
            : 0.0f;

    FPinkCabClutchDrivelineInput Input;
    Input.DeltaSeconds = DeltaTime;
    Input.EngineRpm = FMath::Max(Engine.GetEngineRPM(), 0.0f);
    Input.ShaftEquivalentEngineRpm = ShaftEquivalentEngineRpm;
    Input.AvailableEngineTorqueNm =
        Command.bCombustionAllowed
            ? FMath::Max(
                PhysicsThreadActuation
                    .RequestedEngineTorqueAfterLimiterHealthNm,
                0.0f)
            : 0.0f;
    Input.EngineDragTorqueNm =
        FMath::Max(Input.EngineRpm * Command.EngineBrakeEffect, 0.0f);
    Input.ClutchCoupling01 = Command.ClutchCoupling01;
    Input.DrivetrainTorqueCapacity01 =
        Command.DrivetrainTorqueCapacity01;
    Input.EffectiveGearRatio = Command.EffectiveGearRatio;
    Input.TransmissionEfficiency = Command.TransmissionEfficiency;
    return ClutchModel.Step(Input);
}

void FPinkCabChaosWheeledVehicleSimulation::ApplyEngineReaction(
    Chaos::FSimpleEngineSim& Engine,
    const FPinkCabClutchDrivelineOutput& Output)
{
    const float EngineOmegaAfterReaction = FMath::Max(
        0.0f,
        Engine.GetEngineOmega()
            + Output.EngineReactionDeltaRpm
                * PinkCabChaosRpmToRadPerSecond);
    Engine.SetEngineOmega(EngineOmegaAfterReaction);
}

FPinkCabChaosWheeledVehicleSimulation::FDrivenWheelTorqueStats
FPinkCabChaosWheeledVehicleSimulation::ApplyDrivenWheelTorque(
    const FPinkCabClutchDrivelineOutput& Output)
{
    using namespace Chaos;

    FDrivenWheelTorqueStats Stats;
    for (int32 WheelIndex = 0;
         WheelIndex < PVehicle->Wheels.Num();
         ++WheelIndex)
    {
        FSimpleWheelSim& Wheel = PVehicle->Wheels[WheelIndex];
        const bool bDriven = Wheel.Setup().EngineEnabled;
        const float WheelDriveTorqueNm =
            bDriven
                ? Output.RearAxleTorqueNm * Wheel.Setup().TorqueRatio
                : 0.0f;
        Wheel.SetDriveTorque(TorqueMToCm(WheelDriveTorqueNm));
        if (bDriven)
        {
            Stats.AbsTorqueSumNm += FMath::Abs(WheelDriveTorqueNm);
            ++Stats.DrivenWheelCount;
        }
    }
    return Stats;
}

void FPinkCabChaosWheeledVehicleSimulation::AccumulateEvidenceStep(
    const Chaos::FSimpleEngineSim& Engine,
    const FDrivenWheelTorqueStats& WheelStats,
    const float DeltaTime)
{
    if (EvidenceTargetSampleSeconds <= 0.0
        || EvidenceCompletedSampleSeconds
            + 1.0e-9 >= EvidenceTargetSampleSeconds
        || DeltaTime <= KINDA_SMALL_NUMBER)
    {
        return;
    }

    double RemainingSeconds = static_cast<double>(DeltaTime);
    if (EvidenceSettleSecondsRemaining > 0.0)
    {
        const double SettleConsumed = FMath::Min(
            RemainingSeconds,
            EvidenceSettleSecondsRemaining);
        EvidenceSettleSecondsRemaining -= SettleConsumed;
        RemainingSeconds -= SettleConsumed;
        if (RemainingSeconds <= 1.0e-9)
        {
            return;
        }
    }

    const double SampleRemaining =
        EvidenceTargetSampleSeconds
        - EvidenceCompletedSampleSeconds;
    const double SampleWeightSeconds =
        FMath::Min(RemainingSeconds, SampleRemaining);
    if (SampleWeightSeconds <= 1.0e-9)
    {
        return;
    }

    const float MeanDrivenWheelTorqueNm =
        WheelStats.DrivenWheelCount > 0
            ? WheelStats.AbsTorqueSumNm
                / static_cast<float>(WheelStats.DrivenWheelCount)
            : 0.0f;
    EvidenceDrivenWheelTorqueTimeIntegral +=
        static_cast<double>(MeanDrivenWheelTorqueNm)
        * SampleWeightSeconds;
    EvidenceEngineRpmTimeIntegral +=
        static_cast<double>(Engine.GetEngineRPM())
        * SampleWeightSeconds;
    EvidenceCompletedSampleSeconds += SampleWeightSeconds;
    EvidenceObservedDeltaSecondsSum +=
        static_cast<double>(DeltaTime);
    ++EvidenceCompletedSteps;
}

void FPinkCabChaosWheeledVehicleSimulation::PublishMechanicalStep(
    const float DeltaTime)
{
    if (MechanicalIntegrationDeltaMicros)
    {
        MechanicalIntegrationDeltaMicros->Set(
            FMath::RoundToInt64(
                FMath::Max(DeltaTime, 0.0f) * 1000000.0));
    }
    if (MechanicalIntegrationStepCounter)
    {
        MechanicalIntegrationStepCounter->Increment();
    }
}
