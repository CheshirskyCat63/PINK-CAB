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
    void BeginEvidenceWindow(float InSettleSeconds, float InSampleSeconds);
    FPinkCabMechanicalEvidenceSnapshot ReadEvidenceWindow() const;
#if WITH_DEV_AUTOMATION_TESTS
    bool ResetFixtureWheelKinetics();
#endif

    virtual void ApplyInput(
        const FControlInputs& ControlInputs,
        float DeltaTime) override;
    virtual void ProcessMechanicalSimulation(float DeltaTime) override;

private:
    struct FDrivenWheelTorqueStats
    {
        float AbsTorqueSumNm = 0.0f;
        float InitialAbsTorqueSumNm = 0.0f;
        float SignedTorqueSumNm = 0.0f;
        float AbsWheelRpmSum = 0.0f;
        int32 DrivenWheelCount = 0;
    };

    void AdvanceAcceptedNativeEngine(
        Chaos::FSimpleTransmissionSim& Transmission,
        float DeltaTime);
    FPinkCabClutchDrivelineOutput SolveDrivelineStep(
        float EngineRpmBeforeNative,
        float ObservedFreeEngineNetTorqueNm,
        float DeltaTime);
    void ApplyEngineReaction(
        Chaos::FSimpleEngineSim& Engine,
        const FPinkCabClutchDrivelineOutput& Output);
    FDrivenWheelTorqueStats ApplyDrivenWheelTorque(
        const FPinkCabClutchDrivelineOutput& Output);
    double ConsumeEvidenceSampleSeconds(float DeltaTime);
    void AccumulateEvidenceValues(
        const Chaos::FSimpleEngineSim& Engine,
        const FPinkCabClutchDrivelineOutput& DrivelineOutput,
        float ObservedFreeEngineNetTorqueNm,
        const FDrivenWheelTorqueStats& WheelStats,
        double SampleWeightSeconds,
        float DeltaTime);
    void AccumulateEvidenceStep(
        const Chaos::FSimpleEngineSim& Engine,
        const FPinkCabClutchDrivelineOutput& DrivelineOutput,
        float ObservedFreeEngineNetTorqueNm,
        const FDrivenWheelTorqueStats& WheelStats,
        float DeltaTime);
    void PublishMechanicalStep(float DeltaTime);

    FThreadSafeCounter64* MechanicalIntegrationStepCounter = nullptr;
    FThreadSafeCounter64* MechanicalIntegrationDeltaMicros = nullptr;
    FPinkCabChaosDrivelineCommand Command;
    FPinkCabEngineActuationResult PhysicsThreadActuation;
    FPinkCabClutchDrivelineModel ClutchModel;
    double EvidenceSettleSecondsRemaining = 0.0;
    double EvidenceTargetSampleSeconds = 0.0;
    double EvidenceCompletedSampleSeconds = 0.0;
    int32 EvidenceCompletedSteps = 0;
    double EvidenceDrivenWheelTorqueTimeIntegral = 0.0;
    double EvidenceInitialDrivenWheelTorqueTimeIntegral = 0.0;
    double EvidenceSignedDrivenWheelTorqueTimeIntegral = 0.0;
    double EvidenceDrivenWheelRpmTimeIntegral = 0.0;
    double EvidenceEngineRpmTimeIntegral = 0.0;
    double EvidenceObservedFreeEngineNetTorqueTimeIntegral = 0.0;
    double EvidenceRequestedClutchTorqueTimeIntegral = 0.0;
    double EvidenceTransmittedClutchTorqueTimeIntegral = 0.0;
    double EvidenceClutchSlipRpmTimeIntegral = 0.0;
    double EvidenceAppliedWheelBrakeTorqueTimeIntegral = 0.0;
    double EvidenceObservedDeltaSecondsSum = 0.0;
    float LastMaxAppliedWheelBrakeTorqueNm = 0.0f;
    bool bLastParkingEnabled = false;
    bool bEvidenceAnyParkingEnabled = false;
};
