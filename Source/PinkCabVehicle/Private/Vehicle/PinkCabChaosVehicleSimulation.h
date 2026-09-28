#pragma once

#include "Vehicle/PinkCabChaosVehicleMovementComponent.h"
#include "Vehicle/PinkCabEngineActuationResolver.h"

namespace Chaos
{
class FSimpleEngineSim;
class FSimpleTransmissionSim;
}

class FPinkCabChaosWheeledVehicleSimulation final
    : public UChaosWheeledVehicleSimulation
{
public:
    explicit FPinkCabChaosWheeledVehicleSimulation(
        FThreadSafeCounter64* InMechanicalIntegrationStepCounter,
        FThreadSafeCounter64* InMechanicalIntegrationDeltaMicros);

    void SetDrivelineCommand(
        const FPinkCabChaosDrivelineCommand& InCommand);
    void BeginEvidenceWindow(int32 InSettleSteps, int32 InSampleSteps);
    FPinkCabMechanicalEvidenceSnapshot ReadEvidenceWindow() const;

    virtual void ApplyInput(
        const FControlInputs& ControlInputs,
        float DeltaTime) override;
    virtual void ProcessMechanicalSimulation(float DeltaTime) override;

private:
    struct FDrivenWheelTorqueStats
    {
        float AbsTorqueSumNm = 0.0f;
        int32 DrivenWheelCount = 0;
    };

    void AdvanceAcceptedNativeEngine(
        Chaos::FSimpleTransmissionSim& Transmission,
        float DeltaTime);
    FPinkCabClutchDrivelineOutput SolveDrivelineStep(
        Chaos::FSimpleEngineSim& Engine,
        float DeltaTime);
    void ApplyEngineReaction(
        Chaos::FSimpleEngineSim& Engine,
        const FPinkCabClutchDrivelineOutput& Output);
    FDrivenWheelTorqueStats ApplyDrivenWheelTorque(
        const FPinkCabClutchDrivelineOutput& Output);
    void AccumulateEvidenceStep(
        const Chaos::FSimpleEngineSim& Engine,
        const FDrivenWheelTorqueStats& WheelStats,
        float DeltaTime);
    void PublishMechanicalStep(float DeltaTime);

    FThreadSafeCounter64* MechanicalIntegrationStepCounter = nullptr;
    FThreadSafeCounter64* MechanicalIntegrationDeltaMicros = nullptr;
    FPinkCabChaosDrivelineCommand Command;
    FPinkCabEngineActuationResult PhysicsThreadActuation;
    FPinkCabClutchDrivelineModel ClutchModel;
    int32 EvidenceSettleStepsRemaining = 0;
    int32 EvidenceTargetSteps = 0;
    int32 EvidenceCompletedSteps = 0;
    double EvidenceDrivenWheelTorqueSumNm = 0.0;
    double EvidenceEngineRpmSum = 0.0;
    double EvidenceDeltaSecondsSum = 0.0;
};
