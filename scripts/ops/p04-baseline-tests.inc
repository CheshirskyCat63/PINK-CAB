
// P04 instrumentation uses the unchanged production profile. A successful
// capture is not acceptance of acceleration, a new tune, or a packaged drive.
#include "Runtime/PinkCabChaosTatraPawn.h"
#include "Vehicle/PinkCabChaosPhysicalProfile.h"
#include "Vehicle/PinkCabGearEngagementValidator.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabP04ProfileTableTest,
    "PinkCab.Vehicle.Physics.P04.ProfileDerivedTables",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabP04ProfileTableTest::RunTest(const FString&)
{
    const auto Profile = FPinkCabChaosPhysicalProfile::ForVariant(EPinkCabCalibrationVariant::Nominal);
    const auto* Movement = GetDefault<APinkCabChaosTatraPawn>()->GetChaosMovement();
    if (!TestNotNull(TEXT("P04 production movement exists"), Movement)) return false;
    if (!TestTrue(TEXT("P04 positive driven-wheel radius"), Profile.RearWheel.WheelRadiusCm.Value > 0.0f)) return false;
    if (!TestEqual(TEXT("P04 five forward gears"), Profile.ForwardGearRatios.Value.Num(), 5)) return false;
    if (!TestEqual(TEXT("P04 one reverse ratio"), Profile.ReverseGearRatios.Value.Num(), 1)) return false;
    const double CircumferenceM = 2.0 * PI * Profile.RearWheel.WheelRadiusCm.Value / 100.0;
    FPinkCabGearboxControllerConfig Config;
    Config.EngineRpmEnvelope = Profile.GetEngineRpmEnvelope();
    const float Samples[] = {925.0f, 2000.0f, 3500.0f, 5000.0f, 6500.0f, 7500.0f, Profile.EngineLimiterHardCutRpm.Value};
    for (int32 Gear = -1; Gear <= 5; ++Gear)
    {
        if (Gear == 0) continue;
        const float Ratio = Gear < 0 ? Profile.ReverseGearRatios.Value[0] : Profile.ForwardGearRatios.Value[Gear - 1];
        const double TotalRatio = Ratio * Profile.FinalDriveRatio.Value;
        if (!TestTrue(TEXT("P04 positive finite total ratio"), FMath::IsFinite(TotalRatio) && TotalRatio > 0.0)) return false;
        const double RpmPerKmh = 1000.0 * TotalRatio / (60.0 * CircumferenceM);
        for (const float Rpm : Samples)
        {
            const float SpeedKmh = static_cast<float>(Rpm / RpmPerKmh);
            const float Coupled = FPinkCabGearEngagementValidator::ExpectedEngineRpmForGear(Config, Gear, SpeedKmh);
            TestTrue(TEXT("P04 engagement table agrees with physical profile within 0.1 percent"),
                FMath::Abs(Coupled - Rpm) <= FMath::Max(1.0f, Rpm * 0.001f));
            AddInfo(FString::Printf(
                TEXT("P04_GEAR_TABLE gear=%d rpm=%.3f speed_kmh=%.3f total_ratio=%.6f radius_cm=%.3f model=NO_SLIP_KINEMATICS_NOT_TOP_SPEED"),
                Gear, Rpm, SpeedKmh, TotalRatio, Profile.RearWheel.WheelRadiusCm.Value));
        }
    }
    const FRichCurve* Curve = Movement->EngineSetup.TorqueCurve.GetRichCurveConst();
    if (!TestNotNull(TEXT("P04 applied engine curve exists"), Curve)) return false;
    for (const FVector2D& Key : Profile.NormalizedTorqueCurve.Value)
    {
        const float Factor = Curve->Eval(static_cast<float>(Key.X));
        TestTrue(TEXT("P04 applied torque key agrees with profile"), FMath::IsNearlyEqual(Factor, static_cast<float>(Key.Y), 0.0001f));
        const double TorqueNm = Movement->EngineSetup.MaxTorque * Factor;
        AddInfo(FString::Printf(
            TEXT("P04_TORQUE_TABLE rpm=%.3f torque_nm=%.3f power_kw=%.3f source=AUTHORED_CURVE_NOT_DYNAMOMETER"),
            Key.X, TorqueNm, TorqueNm * Key.X * (2.0 * PI / 60.0) / 1000.0));
    }
    AddInfo(FString::Printf(
        TEXT("P04_BASELINE_PROFILE hash=%016llX calibration=%d mass_kg=%.3f moi=%.3f rev_down=%.3f engine_brake=%.3f"),
        Profile.GetDeterministicProfileHash(), Profile.CalibrationVersion, Profile.ReferenceMassKg.Value,
        Profile.EngineRevUpMOI.Value, Profile.EngineRevDownRate.Value, Profile.EngineBrakeEffect.Value));
    return true;
}

class FPinkCabP04BaselineCommand final : public IAutomationLatentCommand
{
public:
    explicit FPinkCabP04BaselineCommand(FAutomationTestBase* InTest)
        : Test(InTest), WallStart(FPlatformTime::Seconds()) {}
    virtual bool Update() override
    {
        using namespace PinkCabDrivelineRuntimeTest;
        UWorld* World = AutomationCommon::GetAnyGameWorld();
        if (FPlatformTime::Seconds() - WallStart > 35.0)
        {
            Test->AddError(FString::Printf(TEXT("P04 baseline case timed out: %d/18"), CaseIndex));
            if (World) Cleanup(*World);
            return true;
        }
        if (!World) return false;
        if (!bInitialized)
        {
            UGameplayStatics::SetGamePaused(World, false);
            AActor* Floor = PinkCabPhysicsFixture::FindOrSpawnFlatFloor(*World);
            FloorBox = Floor ? Cast<UBoxComponent>(Floor->GetRootComponent()) : nullptr;
            if (!FloorBox.IsValid()) { Test->AddError(TEXT("P04 flat fixture unavailable")); return true; }
            OriginalExtent = FloorBox->GetUnscaledBoxExtent();
            // Only a transient automation fixture is extended; no game map is saved.
            FloorBox->SetBoxExtent(FVector(50000.0f, 2500.0f, 50.0f));
            bInitialized = true;
            BeginCase(*World);
            return false;
        }
        auto* Pawn = PinkCabPhysicsFixture::FindOrSpawnPawn(*World);
        if (!Pawn) { Test->AddError(TEXT("P04 fixture spawn failed")); Cleanup(*World); return true; }
        auto* Movement = Cast<UPinkCabChaosVehicleMovementComponent>(Pawn->GetChaosMovement());
        auto* Mesh = Pawn->GetMesh();
        if (!Movement || !Mesh || Movement->Wheels.Num() != 4)
        {
            Test->AddError(TEXT("P04 four-wheel drivetrain unavailable")); Cleanup(*World); return true;
        }
        PinkCabPhysicsFixture::KeepAwake(*Pawn);
        if (!Apply(*Pawn, Cockpit, Controls))
        {
            Test->AddError(TEXT("P04 normal cockpit actuation failed")); Cleanup(*World); return true;
        }
        if (bSettling)
        {
            if (!RestGate.Update(*Pawn)) return false;
            Start = Mesh->GetComponentLocation();
            Forward = Pawn->GetActorForwardVector();
            const auto Profile = FPinkCabChaosPhysicalProfile::ForVariant(EPinkCabCalibrationVariant::Nominal);
            Test->TestEqual(TEXT("P04 unchanged production mass"), Movement->Mass, Profile.ReferenceMassKg.Value);
            BodyMassKg = Mesh->GetMass();
            LastStep = Movement->GetPinkCabMechanicalIntegrationStepCount();
            bSettling = false;
            Controls.SetBrake(0.0f);
            Controls.SetThrottle(Dose());
            Controls.SetDriveline(Gear(), Gear(), 0.0f);
            return false;
        }
        const double PreviousTime = Elapsed;
        if (!AdvanceMechanicalTime(*Movement, LastStep, Elapsed)) return false;
        const float SignedSpeed = FVector::DotProduct(Mesh->GetPhysicsLinearVelocity(), Forward) * 0.036f;
        const float DirectionalSpeed = Gear() < 0 ? -SignedSpeed : SignedSpeed;
        if (!FMath::IsFinite(DirectionalSpeed) || !FMath::IsFinite(Movement->GetEngineRotationSpeed()))
        {
            Test->AddError(TEXT("P04 non-finite speed/RPM")); Cleanup(*World); return true;
        }
        const float Torque = FMath::Abs(MeanRearDriveTorqueNm(*Movement));
        if (PreviousTime < 5.0)
        {
            if (Time30 < 0.0 && DirectionalSpeed >= 30.0f) Time30 = Elapsed;
            if (Time60 < 0.0 && DirectionalSpeed >= 60.0f) Time60 = Elapsed;
            PeakRpm = FMath::Max(PeakRpm, Movement->GetEngineRotationSpeed());
            PeakRearTorque = FMath::Max(PeakRearTorque, Torque);
            EndDriveSpeed = DirectionalSpeed;
            EndDriveRpm = Movement->GetEngineRotationSpeed();
            float MeanWheelSpeedMps = 0.0f;
            for (int32 I = 2; I < 4; ++I)
            {
                const auto* Wheel = Movement->Wheels[I];
                if (!Wheel) { Test->AddError(TEXT("P04 rear wheel missing")); Cleanup(*World); return true; }
                MeanWheelSpeedMps += 0.5f * FMath::Abs(Wheel->GetWheelAngularVelocity()) * Wheel->WheelRadius / 100.0f;
            }
            PeakSlipSpeedMps = FMath::Max(PeakSlipSpeedMps, FMath::Abs(MeanWheelSpeedMps - FMath::Abs(SignedSpeed) / 3.6f));
        }
        if (Elapsed < 5.0)
        {
            Controls.SetDriveline(Gear(), Gear(), FMath::Clamp(static_cast<float>(Elapsed / 1.5), 0.0f, 1.0f));
            return false;
        }
        if (!bCoasting)
        {
            DriveEndTime = Elapsed;
            Controls.SetThrottle(0.0f);
            Controls.SetDriveline(0, 0, 0.0f);
            bCoasting = true;
            return false;
        }
        if (Elapsed - DriveEndTime < 2.0) return false;
        const float Travel = SignedForwardDistanceCm(Start, Mesh->GetComponentLocation(), Forward);
        Test->TestTrue(TEXT("P04 measurable intended-direction travel"), (Gear() < 0 ? -Travel : Travel) > 10.0f);
        Test->TestTrue(TEXT("P04 neutral removes drive without velocity reset"), Torque <= 0.1f);
        Test->AddInfo(FString::Printf(
            TEXT("P04_ACCEL_BASELINE gear=%d pedal=%.2f repeat=%d body_mass_kg=%.3f drive_s=%.4f speed_end_kmh=%.3f rpm_end=%.3f time30_s=%.4f time60_s=%.4f peak_rpm=%.3f peak_rear_torque_nm=%.3f peak_slip_speed_mps=%.3f neutral_speed_kmh=%.3f neutral_drive_torque_nm=%.5f timing=OBSERVED_MECHANICAL_STEPS not_reached=-1 fixture=STERILE_NATIVE_NOT_PACKAGED"),
            Gear(), Dose(), CaseIndex % 3 + 1, BodyMassKg, DriveEndTime, EndDriveSpeed, EndDriveRpm,
            Time30, Time60, PeakRpm, PeakRearTorque, PeakSlipSpeedMps, DirectionalSpeed, Torque));
        if (++CaseIndex == 18)
        {
            Test->AddInfo(TEXT("P04_BASELINE_COMPLETE cases=18 profile_changed=0 tuning_accepted=0"));
            Cleanup(*World);
            return true;
        }
        BeginCase(*World);
        return false;
    }
private:
    int32 Gear() const { return CaseIndex < 9 ? 1 : -1; }
    float Dose() const { const float Values[] = {0.25f, 0.50f, 1.00f}; return Values[(CaseIndex / 3) % 3]; }
    void BeginCase(UWorld& World)
    {
        PinkCabPhysicsFixture::DestroyPawns(World);
        Cockpit = {};
        Test->TestTrue(TEXT("P04 fresh engine starts"), Cockpit.StartEngine());
        Controls = {};
        Controls.SetBrake(1.0f);
        Controls.SetDriveline(0, 0, 0.0f);
        Controls.SetDrivetrainTorqueCapacity(1.0f);
        RestGate.Reset();
        bSettling = true; bCoasting = false;
        Elapsed = DriveEndTime = 0.0; Time30 = Time60 = -1.0; LastStep = -1;
        PeakRpm = PeakRearTorque = PeakSlipSpeedMps = EndDriveSpeed = EndDriveRpm = 0.0f;
        WallStart = FPlatformTime::Seconds();
    }
    void Cleanup(UWorld& World)
    {
        PinkCabPhysicsFixture::DestroyPawns(World);
        if (FloorBox.IsValid()) FloorBox->SetBoxExtent(OriginalExtent);
    }
    FAutomationTestBase* Test;
    FPinkCabCockpitState Cockpit;
    FPinkCabVehicleControlState Controls;
    FPinkCabPhysicsFixtureRestGate RestGate;
    TWeakObjectPtr<UBoxComponent> FloorBox;
    FVector OriginalExtent = FVector::ZeroVector, Start = FVector::ZeroVector, Forward = FVector::ForwardVector;
    int32 CaseIndex = 0;
    int64 LastStep = -1;
    double WallStart, Elapsed = 0.0, DriveEndTime = 0.0, Time30 = -1.0, Time60 = -1.0;
    float BodyMassKg = 0.0f, PeakRpm = 0.0f, PeakRearTorque = 0.0f, PeakSlipSpeedMps = 0.0f;
    float EndDriveSpeed = 0.0f, EndDriveRpm = 0.0f;
    bool bInitialized = false, bSettling = true, bCoasting = false;
};
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabP04BaselineRuntimeTest,
    "PinkCab.Vehicle.Physics.P04.UnchangedProfileBaseline",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabP04BaselineRuntimeTest::RunTest(const FString&)
{
    if (!TestTrue(TEXT("P04 sterile fixture map opens"), AutomationOpenMap(PinkCabPhysicsFixture::MapPath, true))) return false;
    ADD_LATENT_AUTOMATION_COMMAND(FPinkCabP04BaselineCommand(this));
    return true;
}
