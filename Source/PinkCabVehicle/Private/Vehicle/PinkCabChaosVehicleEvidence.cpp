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
    EvidenceRequestedEngineTorqueTimeIntegral = 0.0;
    EvidenceRequestedClutchTorqueTimeIntegral = 0.0;
    EvidenceTransmittedClutchTorqueTimeIntegral = 0.0;
    EvidenceClutchSlipRpmTimeIntegral = 0.0;
    EvidenceAppliedWheelBrakeTorqueTimeIntegral = 0.0;
    EvidenceObservedDeltaSecondsSum = 0.0;
    EvidenceMaxDeltaSeconds = 0.0f;
    EvidenceMaxEngineInputStateErrorRpm = 0.0f;
    bEvidenceAnyParkingEnabled = false;
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
        Result.MeanRequestedEngineTorqueAfterLimiterHealthNm =
            static_cast<float>(
                EvidenceRequestedEngineTorqueTimeIntegral
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
        Result.MeanAppliedWheelBrakeTorqueNm = static_cast<float>(
            EvidenceAppliedWheelBrakeTorqueTimeIntegral
            / EvidenceCompletedSampleSeconds);
    }
    Result.MaxEngineInputStateErrorRpm = EvidenceMaxEngineInputStateErrorRpm;
    Result.MaxDeltaSeconds = EvidenceMaxDeltaSeconds;
    Result.bAnyParkingEnabled = bEvidenceAnyParkingEnabled;
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

double FPinkCabChaosWheeledVehicleSimulation::ConsumeEvidenceSampleSeconds(
    const float DeltaTime)
{
    if (EvidenceTargetSampleSeconds <= 0.0
        || EvidenceCompletedSampleSeconds
            + 1.0e-9 >= EvidenceTargetSampleSeconds
        || DeltaTime <= KINDA_SMALL_NUMBER)
    {
        return 0.0;
    }

    double RemainingSeconds = static_cast<double>(DeltaTime);
    if (EvidenceSettleSecondsRemaining > 0.0)
    {
        const double SettleConsumed = FMath::Min(
            RemainingSeconds,
            EvidenceSettleSecondsRemaining);
        EvidenceSettleSecondsRemaining -= SettleConsumed;
        RemainingSeconds -= SettleConsumed;
    }
    if (RemainingSeconds <= 1.0e-9)
    {
        return 0.0;
    }

    return FMath::Min(
        RemainingSeconds,
        EvidenceTargetSampleSeconds - EvidenceCompletedSampleSeconds);
}

void FPinkCabChaosWheeledVehicleSimulation::AccumulateEvidenceValues(
    const Chaos::FSimpleEngineSim& Engine,
    const FPinkCabClutchDrivelineOutput& DrivelineOutput,
    const float ObservedFreeEngineNetTorqueNm,
    const FDrivenWheelTorqueStats& WheelStats,
    const double SampleWeightSeconds,
    const float DeltaTime)
{
    const float WheelDivisor =
        WheelStats.DrivenWheelCount > 0
            ? static_cast<float>(WheelStats.DrivenWheelCount)
            : 1.0f;
    const float MeanDrivenWheelTorqueNm =
        WheelStats.AbsTorqueSumNm / WheelDivisor;
    const float MeanInitialDrivenWheelTorqueNm =
        WheelStats.InitialAbsTorqueSumNm / WheelDivisor;
    const float MeanSignedDrivenWheelTorqueNm =
        WheelStats.SignedTorqueSumNm / WheelDivisor;
    const float MeanDrivenWheelRpm =
        WheelStats.AbsWheelRpmSum / WheelDivisor;

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
    EvidenceRequestedEngineTorqueTimeIntegral +=
        static_cast<double>(
            PhysicsThreadActuation.RequestedEngineTorqueAfterLimiterHealthNm)
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
    EvidenceAppliedWheelBrakeTorqueTimeIntegral +=
        static_cast<double>(LastMaxAppliedWheelBrakeTorqueNm)
        * SampleWeightSeconds;
    EvidenceMaxEngineInputStateErrorRpm = FMath::Max(
        EvidenceMaxEngineInputStateErrorRpm, LastEngineInputStateErrorRpm);
    bEvidenceAnyParkingEnabled |= bLastParkingEnabled;
    EvidenceCompletedSampleSeconds += SampleWeightSeconds;
    EvidenceObservedDeltaSecondsSum += static_cast<double>(DeltaTime);
    EvidenceMaxDeltaSeconds = FMath::Max(EvidenceMaxDeltaSeconds, DeltaTime);
    ++EvidenceCompletedSteps;
}

void FPinkCabChaosWheeledVehicleSimulation::AccumulateEvidenceStep(
    const Chaos::FSimpleEngineSim& Engine,
    const FPinkCabClutchDrivelineOutput& DrivelineOutput,
    const float ObservedFreeEngineNetTorqueNm,
    const FDrivenWheelTorqueStats& WheelStats,
    const float DeltaTime)
{
    const double SampleWeightSeconds =
        ConsumeEvidenceSampleSeconds(DeltaTime);
    if (SampleWeightSeconds <= 1.0e-9)
    {
        return;
    }

    AccumulateEvidenceValues(
        Engine,
        DrivelineOutput,
        ObservedFreeEngineNetTorqueNm,
        WheelStats,
        SampleWeightSeconds,
        DeltaTime);
}
