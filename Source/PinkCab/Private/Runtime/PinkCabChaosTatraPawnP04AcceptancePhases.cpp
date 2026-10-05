#include "Runtime/PinkCabChaosTatraPawn.h"

#include "ChaosWheeledVehicleMovementComponent.h"
#include "Vehicle/PinkCabVehicleTelemetry.h"

namespace
{
bool IsReverseCase(const FString& CaseName)
{
    return CaseName.StartsWith(TEXT("reverse"));
}

bool IsCountersteerCase(const FString& CaseName)
{
    return CaseName.StartsWith(TEXT("counter_"));
}

bool IsForwardComparisonCaseLocal(const FString& CaseName)
{
    return CaseName == TEXT("forward_candidate")
        || CaseName == TEXT("forward_baseline");
}

float AcceptanceDose(const FString& CaseName)
{
    if (CaseName == TEXT("reverse25")) return 0.25f;
    if (CaseName == TEXT("reverse50")) return 0.50f;
    if (CaseName == TEXT("reverse100")) return 1.00f;
    if (CaseName == TEXT("lift")) return 0.50f;
    return 1.00f;
}

float CounterTargetKmh(const FString& CaseName)
{
    if (CaseName == TEXT("counter_low")) return 15.0f;
    if (CaseName == TEXT("counter_urban")) return 60.0f;
    if (CaseName == TEXT("counter_high")) return 120.0f;
    return 0.0f;
}


float AbsMeanRearDriveTorqueNmLocal(const UChaosWheeledVehicleMovementComponent& Movement)
{
    if (Movement.GetNumWheels() < 4)
    {
        return 0.0f;
    }
    return 0.5f * (
        FMath::Abs(Movement.GetWheelState(2).DriveTorque)
        + FMath::Abs(Movement.GetWheelState(3).DriveTorque));
}
}

bool APinkCabChaosTatraPawn::TickP04PackagedAcceptancePreparation(
    const float DeltaSeconds)
{
    if (P04PackagedAcceptancePhase == 0)
    {
        ApplyP04PackagedAcceptanceInputs(0, 0.0f, 1.0f, 1.0f, 0.0f, DeltaSeconds);
        if (P04PackagedAcceptancePhaseSeconds >= 1.0)
        {
            EnterP04PackagedAcceptancePhase(1);
        }
        return true;
    }
    if (P04PackagedAcceptancePhase == 1)
    {
        ApplyP04PackagedAcceptanceInputs(
            P04PackagedAcceptanceDriveGear, 0.0f, 0.0f, 1.0f, 0.0f, DeltaSeconds);
        if (P04PackagedAcceptancePhaseSeconds >= 0.45)
        {
            EnterP04PackagedAcceptancePhase(2);
        }
        return true;
    }
    return false;
}

bool APinkCabChaosTatraPawn::TickP04PackagedAcceptanceShift(
    const float DeltaSeconds)
{
    if (P04PackagedAcceptancePhase == 3)
    {
        P04PackagedAcceptanceDriveSeconds += DeltaSeconds;
        ApplyP04PackagedAcceptanceInputs(
            P04PackagedAcceptanceDriveGear, 0.0f, 0.0f, 1.0f, 0.0f, DeltaSeconds);
        if (P04PackagedAcceptancePhaseSeconds >= 0.30)
        {
            EnterP04PackagedAcceptancePhase(4);
        }
        return true;
    }
    if (P04PackagedAcceptancePhase != 4)
    {
        return false;
    }

    P04PackagedAcceptanceDriveSeconds += DeltaSeconds;
    const float ReleaseAlpha = FMath::Clamp(
        static_cast<float>(P04PackagedAcceptancePhaseSeconds / 0.70),
        0.0f, 1.0f);
    ApplyP04PackagedAcceptanceInputs(
        P04PackagedAcceptanceDriveGear,
        AcceptanceDose(P04PackagedAcceptanceCase),
        0.0f,
        1.0f - ReleaseAlpha,
        0.0f,
        DeltaSeconds);
    if (P04PackagedAcceptancePhaseSeconds >= 0.70)
    {
        EnterP04PackagedAcceptancePhase(2);
    }
    return true;
}

bool APinkCabChaosTatraPawn::TickP04PackagedAcceptanceProof(
    const float DeltaSeconds)
{
    if (P04PackagedAcceptancePhase == 5)
    {
        return TickP04PackagedAcceptanceNeutralOrLiftProof(DeltaSeconds);
    }
    if (P04PackagedAcceptancePhase >= 6
        && P04PackagedAcceptancePhase <= 9)
    {
        return TickP04PackagedAcceptanceCountersteerProof(DeltaSeconds);
    }
    return false;
}

bool APinkCabChaosTatraPawn::TickP04PackagedAcceptanceNeutralOrLiftProof(
    const float DeltaSeconds)
{
    UChaosWheeledVehicleMovementComponent* Movement = GetChaosMovement();
    if (!Movement)
    {
        FinishP04PackagedAcceptance(false, TEXT("MOVEMENT_LOST"));
        return true;
    }

    const bool bNeutralProof = IsReverseCase(P04PackagedAcceptanceCase);
    ApplyP04PackagedAcceptanceInputs(
        bNeutralProof ? 0 : P04PackagedAcceptanceDriveGear,
        0.0f,
        0.0f,
        bNeutralProof ? 1.0f : 0.0f,
        0.0f,
        DeltaSeconds);

    if (!bNeutralProof
        && !SampleP04PackagedAcceptancePostLiftTorque(*Movement))
    {
        return true;
    }

    const double RequiredSeconds = bNeutralProof ? 1.25 : 2.0;
    if (P04PackagedAcceptancePhaseSeconds < RequiredSeconds)
    {
        return true;
    }

    const float SpeedKmh =
        FMath::Abs(GetP04PackagedAcceptanceDirectionalSpeedKmh());
    if (bNeutralProof)
    {
        const bool bPass =
            P04PackagedAcceptanceNeutralEntrySpeedKmh > 1.0f
            && SpeedKmh > 0.10f
            && AbsMeanRearDriveTorqueNmLocal(*Movement) <= 0.10f
            && GetEngagedGear() == 0;
        FinishP04PackagedAcceptance(
            bPass,
            bPass ? TEXT("REVERSE_NEUTRAL_CAUSAL_PASS")
                  : TEXT("REVERSE_NEUTRAL_CAUSAL_FAIL"));
        return true;
    }

    const bool bPass =
        P04PackagedAcceptancePostLiftMaxRearDriveTorqueNm <= 1.0f
        && SpeedKmh < P04PackagedAcceptanceLiftEntrySpeedKmh - 0.25f;
    FinishP04PackagedAcceptance(
        bPass,
        bPass ? TEXT("LIFT_CAUSAL_PASS") : TEXT("LIFT_CAUSAL_FAIL"));
    return true;
}

bool APinkCabChaosTatraPawn::TickP04PackagedAcceptanceCountersteerProof(
    const float DeltaSeconds)
{
    if (P04PackagedAcceptancePhase == 6)
    {
        ApplyP04PackagedAcceptanceInputs(
            P04PackagedAcceptanceDriveGear, 0.20f, 0.0f, 0.0f, 12000.0f, DeltaSeconds);
        P04PackagedAcceptanceRightSteering = GetSteeringCommand();
        EnterP04PackagedAcceptancePhase(7);
        return true;
    }
    if (P04PackagedAcceptancePhase == 7)
    {
        ApplyP04PackagedAcceptanceInputs(
            P04PackagedAcceptanceDriveGear, 0.20f, 0.0f, 0.0f, 0.0f, DeltaSeconds);
        if (P04PackagedAcceptancePhaseSeconds >= 0.15)
        {
            EnterP04PackagedAcceptancePhase(8);
        }
        return true;
    }
    if (P04PackagedAcceptancePhase == 8)
    {
        ApplyP04PackagedAcceptanceInputs(
            P04PackagedAcceptanceDriveGear, 0.20f, 0.0f, 0.0f, -24000.0f, DeltaSeconds);
        P04PackagedAcceptanceCounterSteering = GetSteeringCommand();
        EnterP04PackagedAcceptancePhase(9);
        return true;
    }

    ApplyP04PackagedAcceptanceInputs(
        P04PackagedAcceptanceDriveGear, 0.20f, 0.0f, 0.0f, 0.0f, DeltaSeconds);
    if (P04PackagedAcceptancePhaseSeconds >= 0.15)
    {
        const bool bPass =
            P04PackagedAcceptanceRightSteering >= 0.99f
            && P04PackagedAcceptanceCounterSteering <= -0.99f;
        FinishP04PackagedAcceptance(
            bPass,
            bPass ? TEXT("COUNTERSTEER_AUTHORITY_PASS")
                  : TEXT("COUNTERSTEER_AUTHORITY_FAIL"));
    }
    return true;
}

bool APinkCabChaosTatraPawn::TickP04PackagedAcceptanceDrive(
    const float DeltaSeconds)
{
    P04PackagedAcceptanceDriveSeconds += DeltaSeconds;
    const float SpeedKmh =
        FMath::Abs(GetP04PackagedAcceptanceDirectionalSpeedKmh());
    if (P04PackagedAcceptanceTime30Seconds < 0.0f && SpeedKmh >= 30.0f)
    {
        P04PackagedAcceptanceTime30Seconds =
            static_cast<float>(P04PackagedAcceptanceDriveSeconds);
    }
    if (P04PackagedAcceptanceTime60Seconds < 0.0f && SpeedKmh >= 60.0f)
    {
        P04PackagedAcceptanceTime60Seconds =
            static_cast<float>(P04PackagedAcceptanceDriveSeconds);
    }

    const float LaunchAlpha = FMath::Clamp(
        static_cast<float>(P04PackagedAcceptanceDriveSeconds), 0.0f, 1.0f);
    ApplyP04PackagedAcceptanceInputs(
        P04PackagedAcceptanceDriveGear,
        AcceptanceDose(P04PackagedAcceptanceCase),
        0.0f,
        1.0f - LaunchAlpha,
        0.0f,
        DeltaSeconds);

    if (IsReverseCase(P04PackagedAcceptanceCase))
    {
        if (P04PackagedAcceptanceDriveSeconds >= 6.0)
        {
            P04PackagedAcceptanceNeutralEntrySpeedKmh = SpeedKmh;
            EnterP04PackagedAcceptancePhase(5);
        }
        return true;
    }
    return TickP04PackagedAcceptanceForwardDrive(DeltaSeconds);
}

bool APinkCabChaosTatraPawn::TickP04PackagedAcceptanceForwardDrive(
    const float DeltaSeconds)
{
    FPinkCabVehicleTelemetry Telemetry;
    if (!DynamicsProvider.ReadTelemetry(Telemetry))
    {
        return true;
    }
    const float SpeedKmh =
        FMath::Abs(GetP04PackagedAcceptanceDirectionalSpeedKmh());
    if (GetEngagedGear() == 5 && SpeedKmh >= 80.0f)
    {
        P04PackagedAcceptanceGear5SpeedKmh = SpeedKmh;
        P04PackagedAcceptanceGear5Rpm = Telemetry.EngineRpm;
    }
    if (EvaluateP04PackagedAcceptanceForwardCase(SpeedKmh))
    {
        return true;
    }
    if (P04PackagedAcceptanceDriveSeconds >= 1.25
        && P04PackagedAcceptanceDriveGear > 0
        && P04PackagedAcceptanceDriveGear < 5
        && Telemetry.EngineRpm >= 6000.0f)
    {
        ++P04PackagedAcceptanceDriveGear;
        EnterP04PackagedAcceptancePhase(3);
    }
    return true;
}

bool APinkCabChaosTatraPawn::EvaluateP04PackagedAcceptanceForwardCase(
    const float SpeedKmh)
{
    if (P04PackagedAcceptanceCase == TEXT("lift") && SpeedKmh >= 30.0f)
    {
        P04PackagedAcceptanceLiftEntrySpeedKmh = SpeedKmh;
        EnterP04PackagedAcceptancePhase(5);
        return true;
    }
    if (IsCountersteerCase(P04PackagedAcceptanceCase)
        && SpeedKmh >= CounterTargetKmh(P04PackagedAcceptanceCase))
    {
        EnterP04PackagedAcceptancePhase(6);
        return true;
    }
    if (IsForwardComparisonCaseLocal(P04PackagedAcceptanceCase)
        && P04PackagedAcceptanceTime60Seconds >= 0.0f
        && P04PackagedAcceptanceDriveSeconds
            >= P04PackagedAcceptanceTime60Seconds + 0.50)
    {
        FinishP04PackagedAcceptance(true, TEXT("FORWARD_0_60_CAPTURED"));
        return true;
    }
    if (IsForwardComparisonCaseLocal(P04PackagedAcceptanceCase)
        && P04PackagedAcceptanceDriveSeconds >= 30.0)
    {
        FinishP04PackagedAcceptance(false, TEXT("FORWARD_0_60_NOT_REACHED"));
        return true;
    }
    if (P04PackagedAcceptanceCase == TEXT("top") && SpeedKmh >= 195.0f)
    {
        P04PackagedAcceptanceLiftEntrySpeedKmh = SpeedKmh;
        EnterP04PackagedAcceptancePhase(5);
        return true;
    }
    if (P04PackagedAcceptanceCase == TEXT("top")
        && P04PackagedAcceptanceDriveSeconds >= 80.0)
    {
        FinishP04PackagedAcceptance(false, TEXT("TOP_195_NOT_REACHED"));
        return true;
    }
    return false;
}
