#include "PinkCabChaosVehicleSimulation.h"

#include "EngineSystem.h"

#if WITH_DEV_AUTOMATION_TESTS
void FPinkCabChaosWheeledVehicleSimulation::SetDynamometerBrakeTorqueForTests(
    const float BrakeTorqueNm)
{
    TestDynamometerBrakeTorqueNm =
        FMath::Max(BrakeTorqueNm, 0.0f);
}
#endif

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
    EvidenceDrivenWheelRpmTimeIntegral = 0.0;
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
        Result.MeanDrivenWheelRpm = static_cast<float>(
            EvidenceDrivenWheelRpmTimeIntegral
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
#if WITH_DEV_AUTOMATION_TESTS
    Result.DynamometerBrakeTorqueNm =
        TestDynamometerBrakeTorqueNm;
#endif
    Result.bComplete =
        EvidenceTargetSampleSeconds > 0.0
        && EvidenceCompletedSampleSeconds
            + 1.0e-9 >= EvidenceTargetSampleSeconds;
    return Result;
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
    const float MeanDrivenWheelRpm =
        WheelStats.DrivenWheelCount > 0
            ? WheelStats.AbsWheelRpmSum
                / static_cast<float>(WheelStats.DrivenWheelCount)
            : 0.0f;
    EvidenceDrivenWheelTorqueTimeIntegral +=
        static_cast<double>(MeanDrivenWheelTorqueNm)
        * SampleWeightSeconds;
    EvidenceDrivenWheelRpmTimeIntegral +=
        static_cast<double>(MeanDrivenWheelRpm)
        * SampleWeightSeconds;
    EvidenceEngineRpmTimeIntegral +=
        static_cast<double>(Engine.GetEngineRPM())
        * SampleWeightSeconds;
    EvidenceCompletedSampleSeconds += SampleWeightSeconds;
    EvidenceObservedDeltaSecondsSum +=
        static_cast<double>(DeltaTime);
    ++EvidenceCompletedSteps;
}

