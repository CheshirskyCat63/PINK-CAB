#include "PinkCabChaosVehicleSimulation.h"

#include "EngineSystem.h"

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
    EvidenceInitialDrivenWheelTorqueTimeIntegral = 0.0;
    EvidenceSignedDrivenWheelTorqueTimeIntegral = 0.0;
    EvidenceDrivenWheelRpmTimeIntegral = 0.0;
    EvidenceEngineRpmTimeIntegral = 0.0;
    EvidenceObservedFreeEngineNetTorqueTimeIntegral = 0.0;
    EvidenceRequestedClutchTorqueTimeIntegral = 0.0;
    EvidenceTransmittedClutchTorqueTimeIntegral = 0.0;
    EvidenceClutchSlipRpmTimeIntegral = 0.0;
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
        Result.MeanInitialDrivenWheelTorqueNm = static_cast<float>(
            EvidenceInitialDrivenWheelTorqueTimeIntegral
            / EvidenceCompletedSampleSeconds);
        Result.MeanSignedDrivenWheelTorqueNm = static_cast<float>(
            EvidenceSignedDrivenWheelTorqueTimeIntegral
            / EvidenceCompletedSampleSeconds);
        Result.MeanDrivenWheelRpm = static_cast<float>(
            EvidenceDrivenWheelRpmTimeIntegral
            / EvidenceCompletedSampleSeconds);
        Result.MeanEngineRpm = static_cast<float>(
            EvidenceEngineRpmTimeIntegral
            / EvidenceCompletedSampleSeconds);
        Result.MeanObservedFreeEngineNetTorqueNm = static_cast<float>(
            EvidenceObservedFreeEngineNetTorqueTimeIntegral
            / EvidenceCompletedSampleSeconds);
        Result.MeanRequestedClutchTorqueNm = static_cast<float>(
            EvidenceRequestedClutchTorqueTimeIntegral
            / EvidenceCompletedSampleSeconds);
        Result.MeanTransmittedClutchTorqueNm = static_cast<float>(
            EvidenceTransmittedClutchTorqueTimeIntegral
            / EvidenceCompletedSampleSeconds);
        Result.MeanClutchSlipRpm = static_cast<float>(
            EvidenceClutchSlipRpmTimeIntegral
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

void FPinkCabChaosWheeledVehicleSimulation::AccumulateEvidenceStep(
    const Chaos::FSimpleEngineSim& Engine,
    const FPinkCabClutchDrivelineOutput& DrivelineOutput,
    const float ObservedFreeEngineNetTorqueNm,
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
    const float MeanInitialDrivenWheelTorqueNm =
        WheelStats.DrivenWheelCount > 0
            ? WheelStats.InitialAbsTorqueSumNm
                / static_cast<float>(WheelStats.DrivenWheelCount)
            : 0.0f;
    const float MeanSignedDrivenWheelTorqueNm =
        WheelStats.DrivenWheelCount > 0
            ? WheelStats.SignedTorqueSumNm
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
    EvidenceInitialDrivenWheelTorqueTimeIntegral +=
        static_cast<double>(MeanInitialDrivenWheelTorqueNm)
        * SampleWeightSeconds;
    EvidenceSignedDrivenWheelTorqueTimeIntegral +=
        static_cast<double>(MeanSignedDrivenWheelTorqueNm)
        * SampleWeightSeconds;
    EvidenceDrivenWheelRpmTimeIntegral +=
        static_cast<double>(MeanDrivenWheelRpm)
        * SampleWeightSeconds;
    EvidenceEngineRpmTimeIntegral +=
        static_cast<double>(Engine.GetEngineRPM())
        * SampleWeightSeconds;
    EvidenceObservedFreeEngineNetTorqueTimeIntegral +=
        static_cast<double>(ObservedFreeEngineNetTorqueNm)
        * SampleWeightSeconds;
    EvidenceRequestedClutchTorqueTimeIntegral +=
        static_cast<double>(DrivelineOutput.RequestedClutchTorqueNm)
        * SampleWeightSeconds;
    EvidenceTransmittedClutchTorqueTimeIntegral +=
        static_cast<double>(DrivelineOutput.TransmittedClutchTorqueNm)
        * SampleWeightSeconds;
    EvidenceClutchSlipRpmTimeIntegral +=
        static_cast<double>(DrivelineOutput.SlipRpm)
        * SampleWeightSeconds;
    EvidenceCompletedSampleSeconds += SampleWeightSeconds;
    EvidenceObservedDeltaSecondsSum +=
        static_cast<double>(DeltaTime);
    ++EvidenceCompletedSteps;
}

