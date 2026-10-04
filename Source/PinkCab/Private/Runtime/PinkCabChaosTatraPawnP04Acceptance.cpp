#include "Runtime/PinkCabChaosTatraPawn.h"

#include "ChaosWheeledVehicleMovementComponent.h"
#include "HAL/PlatformMisc.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Vehicle/PinkCabChaosPhysicalProfile.h"
#include "Vehicle/PinkCabVehicleInputFrame.h"

namespace
{
bool IsReverseAcceptanceCase(const FString& CaseName)
{
    return CaseName.StartsWith(TEXT("reverse"));
}

bool IsForwardComparisonCase(const FString& CaseName)
{
    return CaseName == TEXT("forward_candidate")
        || CaseName == TEXT("forward_baseline");
}

bool IsKnownAcceptanceCase(const FString& CaseName)
{
    return IsForwardComparisonCase(CaseName)
        || IsReverseAcceptanceCase(CaseName)
        || CaseName == TEXT("lift")
        || CaseName == TEXT("top")
        || CaseName.StartsWith(TEXT("counter_"));
}

float AbsMeanRearDriveTorqueNm(const UChaosWheeledVehicleMovementComponent& Movement)
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

void APinkCabChaosTatraPawn::InitializeP04PackagedAcceptance()
{
    FString CaseName;
    if (!FParse::Value(FCommandLine::Get(), TEXT("PinkCabP04Acceptance="), CaseName))
    {
        return;
    }
    CaseName.TrimStartAndEndInline();
    CaseName.ToLowerInline();
    if (!IsKnownAcceptanceCase(CaseName))
    {
        UE_LOG(LogTemp, Error, TEXT("PINKCAB_P04_RESULT case=%s pass=0 reason=UNKNOWN_CASE"), *CaseName);
        FPlatformMisc::RequestExit(false);
        return;
    }

    bP04PackagedAcceptanceEnabled = true;
    bP04PackagedAcceptanceFinished = false;
    bPackagedGateTelemetryEnabled = true;
    P04PackagedAcceptanceCase = CaseName;
    P04PackagedAcceptancePhase = 0;
    P04PackagedAcceptanceDriveGear = IsReverseAcceptanceCase(CaseName) ? -1 : 1;
    P04PackagedAcceptanceElapsedSeconds = 0.0;
    P04PackagedAcceptancePhaseSeconds = 0.0;
    P04PackagedAcceptanceDriveSeconds = 0.0;
    P04PackagedAcceptanceTime30Seconds = -1.0f;
    P04PackagedAcceptanceTime60Seconds = -1.0f;
    P04PackagedAcceptancePeakSpeedKmh = 0.0f;
    P04PackagedAcceptanceNeutralEntrySpeedKmh = 0.0f;
    P04PackagedAcceptanceLiftEntrySpeedKmh = 0.0f;
    P04PackagedAcceptanceGear5SpeedKmh = 0.0f;
    P04PackagedAcceptanceGear5Rpm = 0.0f;
    P04PackagedAcceptanceRightSteering = 0.0f;
    P04PackagedAcceptanceCounterSteering = 0.0f;
    P04PackagedAcceptancePostLiftMaxRearDriveTorqueNm = -MAX_flt;
    P04PackagedAcceptanceStartForward = GetActorForwardVector().GetSafeNormal();

    SetSystemMenuOpen(false);
    UGameplayStatics::SetGamePaused(this, false);
    CockpitState.StartEngine();
    VehicleControlRuntime.ResetEngineTransition();
    CockpitState.SetSelectedGear(0);
    VehicleControlRuntime.ResetHandbrake(0.0f, false, CockpitState);
    SyncCockpitToChaos();

    UChaosWheeledVehicleMovementComponent* Movement = GetChaosMovement();
    if (CaseName == TEXT("forward_baseline")
        && Movement
        && Movement->TransmissionSetup.ForwardGearRatios.Num() > 0)
    {
        Movement->TransmissionSetup.ForwardGearRatios[0] = 4.60f;
    }

    const FPinkCabChaosPhysicalProfile Profile =
        FPinkCabChaosPhysicalProfile::ForVariant(EPinkCabCalibrationVariant::Nominal);
    const float FirstRatio = Movement && Movement->TransmissionSetup.ForwardGearRatios.Num() > 0
        ? Movement->TransmissionSetup.ForwardGearRatios[0] : -1.0f;
    UE_LOG(
        LogTemp, Display,
        TEXT("PINKCAB_P04_BEGIN case=%s profile_hash=%016llX calibration=%d mass_kg=%.3f first_ratio=%.4f final_drive=%.4f source=PACKAGED_RUNTIME_CONTROL_PATH"),
        *CaseName, Profile.GetDeterministicProfileHash(), Profile.CalibrationVersion,
        Movement ? Movement->Mass : -1.0f, FirstRatio, Profile.FinalDriveRatio.Value);
}

void APinkCabChaosTatraPawn::ApplyP04PackagedAcceptanceInputs(
    const int32 Gear,
    const float Throttle,
    const float Brake,
    const float Clutch,
    const float SteeringMouseCounts,
    const float DeltaSeconds)
{
    CockpitState.SetSelectedGear(Gear);
    FPinkCabVehicleInputFrame Frame;
    Frame.Throttle = FMath::Clamp(Throttle, 0.0f, 1.0f);
    Frame.Brake = FMath::Clamp(Brake, 0.0f, 1.0f);
    Frame.Clutch = FMath::Clamp(Clutch, 0.0f, 1.0f);
    ApplyVehicleInputFrame(Frame, SteeringMouseCounts, DeltaSeconds);
}

void APinkCabChaosTatraPawn::EnterP04PackagedAcceptancePhase(const int32 Phase)
{
    P04PackagedAcceptancePhase = Phase;
    P04PackagedAcceptancePhaseSeconds = 0.0;
}

float APinkCabChaosTatraPawn::GetP04PackagedAcceptanceDirectionalSpeedKmh() const
{
    const float SignedSpeedKmh =
        FVector::DotProduct(GetVelocity(), P04PackagedAcceptanceStartForward) * 0.036f;
    return IsReverseAcceptanceCase(P04PackagedAcceptanceCase)
        ? -SignedSpeedKmh : SignedSpeedKmh;
}

void APinkCabChaosTatraPawn::FinishP04PackagedAcceptance(
    const bool bPass,
    const TCHAR* Reason)
{
    UChaosWheeledVehicleMovementComponent* Movement = GetChaosMovement();
    const FPinkCabChaosPhysicalProfile Profile =
        FPinkCabChaosPhysicalProfile::ForVariant(EPinkCabCalibrationVariant::Nominal);
    float Gear5RatioErrorPct = -1.0f;
    if (P04PackagedAcceptanceGear5SpeedKmh > 1.0f
        && P04PackagedAcceptanceGear5Rpm > 1.0f
        && Profile.ForwardGearRatios.Value.Num() >= 5)
    {
        const double CircumferenceM =
            2.0 * PI * Profile.RearWheel.WheelRadiusCm.Value / 100.0;
        const double TotalRatio =
            Profile.ForwardGearRatios.Value[4] * Profile.FinalDriveRatio.Value;
        const double ExpectedRpm =
            P04PackagedAcceptanceGear5SpeedKmh * 1000.0 * TotalRatio
            / (60.0 * CircumferenceM);
        Gear5RatioErrorPct = static_cast<float>(
            FMath::Abs(P04PackagedAcceptanceGear5Rpm - ExpectedRpm)
            / FMath::Max(ExpectedRpm, 1.0) * 100.0);
    }

    UE_LOG(
        LogTemp, Display,
        TEXT("PINKCAB_P04_RESULT case=%s pass=%d reason=%s drive_s=%.4f time30_s=%.4f time60_s=%.4f peak_speed_kmh=%.3f final_speed_kmh=%.3f neutral_entry_kmh=%.3f lift_entry_kmh=%.3f gear5_speed_kmh=%.3f gear5_rpm=%.3f gear5_ratio_error_pct=%.3f right_steer=%.4f counter_steer=%.4f post_lift_max_rear_drive_nm=%.3f final_rear_drive_abs_nm=%.3f final_gear=%d mass_kg=%.3f profile_asset_mutated=0 transient_ab_ratio_override=%d force_injection=0"),
        *P04PackagedAcceptanceCase, static_cast<int32>(bPass), Reason,
        P04PackagedAcceptanceDriveSeconds, P04PackagedAcceptanceTime30Seconds,
        P04PackagedAcceptanceTime60Seconds, P04PackagedAcceptancePeakSpeedKmh,
        GetP04PackagedAcceptanceDirectionalSpeedKmh(),
        P04PackagedAcceptanceNeutralEntrySpeedKmh, P04PackagedAcceptanceLiftEntrySpeedKmh,
        P04PackagedAcceptanceGear5SpeedKmh, P04PackagedAcceptanceGear5Rpm,
        Gear5RatioErrorPct, P04PackagedAcceptanceRightSteering,
        P04PackagedAcceptanceCounterSteering,
        P04PackagedAcceptancePostLiftMaxRearDriveTorqueNm,
        Movement ? AbsMeanRearDriveTorqueNm(*Movement) : -1.0f,
        GetEngagedGear(), Movement ? Movement->Mass : -1.0f,
        static_cast<int32>(P04PackagedAcceptanceCase == TEXT("forward_baseline")));

    bP04PackagedAcceptanceFinished = true;
    FPlatformMisc::RequestExit(false);
}

bool APinkCabChaosTatraPawn::TickP04PackagedAcceptance(const float DeltaSeconds)
{
    if (!bP04PackagedAcceptanceEnabled || bP04PackagedAcceptanceFinished)
    {
        return bP04PackagedAcceptanceEnabled;
    }
    UChaosWheeledVehicleMovementComponent* Movement = GetChaosMovement();
    if (!Movement || Movement->GetNumWheels() != 4)
    {
        FinishP04PackagedAcceptance(false, TEXT("MOVEMENT_UNAVAILABLE"));
        return true;
    }

    P04PackagedAcceptanceElapsedSeconds += DeltaSeconds;
    P04PackagedAcceptancePhaseSeconds += DeltaSeconds;
    const float SpeedKmh = FMath::Abs(GetP04PackagedAcceptanceDirectionalSpeedKmh());
    P04PackagedAcceptancePeakSpeedKmh =
        FMath::Max(P04PackagedAcceptancePeakSpeedKmh, SpeedKmh);
    if (P04PackagedAcceptanceElapsedSeconds > 100.0)
    {
        FinishP04PackagedAcceptance(false, TEXT("CASE_TIMEOUT"));
        return true;
    }
    if (TickP04PackagedAcceptancePreparation(DeltaSeconds)
        || TickP04PackagedAcceptanceShift(DeltaSeconds)
        || TickP04PackagedAcceptanceProof(DeltaSeconds))
    {
        return true;
    }
    if (P04PackagedAcceptancePhase != 2)
    {
        FinishP04PackagedAcceptance(false, TEXT("INVALID_PHASE"));
        return true;
    }
    return TickP04PackagedAcceptanceDrive(DeltaSeconds);
}
