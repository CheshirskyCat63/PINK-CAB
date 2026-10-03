#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/IConsoleManager.h"
#include "Vehicle/PinkCabDrivelineRuntimeTestUtils.h"

namespace
{
AActor* SpawnEightPercentRamp(UWorld& World)
{
    AActor* Ramp = World.SpawnActor<AActor>();
    if (!Ramp)
    {
        return nullptr;
    }

    constexpr float Grade = 0.08f;
    const float PitchDeg = FMath::RadiansToDegrees(FMath::Atan(Grade));
    UBoxComponent* Box = NewObject<UBoxComponent>(Ramp);
    Ramp->SetRootComponent(Box);
    Box->SetBoxExtent(FVector(1200.0f, 350.0f, 60.0f));
    Box->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Box->SetCollisionObjectType(ECC_WorldStatic);
    Box->SetCollisionResponseToAllChannels(ECR_Block);
    Box->SetGenerateOverlapEvents(false);
    Box->RegisterComponent();
    Ramp->SetActorRotation(FRotator(PitchDeg, 0.0f, 0.0f));
    Ramp->SetActorLocation(FVector(0.0f, 0.0f, 12000.0f));
    Box->RecreatePhysicsState();
    return Ramp;
}
}

class FPinkCabD4FlatReverseCommand final : public IAutomationLatentCommand
{
public:
    explicit FPinkCabD4FlatReverseCommand(FAutomationTestBase* InTest)
        : Test(InTest) {}

    virtual bool Update() override
    {
        using namespace PinkCabDrivelineRuntimeTest;
        UWorld* World = AutomationCommon::GetAnyGameWorld();
        if (!World) return false;

        AActor* Floor = PinkCabPhysicsFixture::FindOrSpawnFlatFloor(*World);
        APinkCabPhysicsFixturePawn* Pawn =
            PinkCabPhysicsFixture::FindOrSpawnPawn(*World);
        if (!Floor || !Pawn)
        {
            Test->AddError(TEXT("D4 launch fixture failed to spawn"));
            return true;
        }
        PinkCabPhysicsFixture::KeepAwake(*Pawn);

        UChaosWheeledVehicleMovementComponent* Movement =
            Pawn->GetChaosMovement();
        UPinkCabChaosVehicleMovementComponent* PinkCabMovement =
            Cast<UPinkCabChaosVehicleMovementComponent>(Movement);
        USkeletalMeshComponent* Mesh = Pawn->GetMesh();
        if (!Movement || !PinkCabMovement || !Mesh)
        {
            Test->AddError(TEXT("D4 launch fixture is incomplete"));
            return true;
        }

        if (!bInitialized)
        {
            UGameplayStatics::SetGamePaused(World, false);
            Test->TestTrue(TEXT("D4 launch engine starts"), Cockpit.StartEngine());
            bInitialized = true;
            BeginRun(*World);
            return false;
        }

        if (!Apply(*Pawn, Cockpit, Controls))
        {
            Test->AddError(TEXT("D4 launch actuation failed"));
            return true;
        }

        if (bSettling)
        {
            if (!RestGate.Update(*Pawn)) return false;

            StartLocation = Mesh->GetComponentLocation();
            StartForward = Pawn->GetActorForwardVector();
            Controls = {};
            Controls.SetThrottle(0.55f);
            Controls.SetBrake(0.0f);
            Controls.SetHandbrake(0.0f);
            Controls.SetDriveline(CurrentGear(), CurrentGear(), 0.75f);
            Controls.SetDrivetrainTorqueCapacity(1.0f);
            LastMechanicalStep =
                PinkCabMovement->GetPinkCabMechanicalIntegrationStepCount();
            PhaseSimSeconds = 0.0;
            bSettling = false;
            return false;
        }

        if (!AdvanceMechanicalTime(
                *PinkCabMovement, LastMechanicalStep, PhaseSimSeconds))
        {
            return false;
        }

        const float SignedTravel = SignedForwardDistanceCm(
            StartLocation, Mesh->GetComponentLocation(), StartForward);
        const float Speed = HorizontalSpeedCmPerSec(*Mesh);
        const float RearTorque = MeanRearDriveTorqueNm(*Movement);
        const bool bReached = CurrentGear() > 0
            ? SignedTravel > 100.0f
            : SignedTravel < -100.0f;

        if ((!bReached || Speed <= 100.0f) && PhaseSimSeconds < 6.0)
        {
            return false;
        }

        Test->AddInfo(FString::Printf(
            TEXT("P02_D4_LAUNCH gear=%d sim_s=%.3f travel_cm=%.3f speed_cm_s=%.3f rear_torque_nm=%.3f"),
            CurrentGear(), PhaseSimSeconds, SignedTravel, Speed, RearTorque));
        Test->TestTrue(
            CurrentGear() > 0
                ? TEXT("D4 first gear produces forward travel")
                : TEXT("D4 reverse produces backward travel"),
            bReached);
        Test->TestTrue(TEXT("D4 launch reaches measurable chassis speed"),
            Speed > 100.0f);
        Test->TestTrue(
            TEXT("D4 launch carries measurable rear-wheel drive torque"),
            FMath::Abs(RearTorque) > 1.0f);

        if (++GearIndex >= Gears.Num())
        {
            return true;
        }
        BeginRun(*World);
        return false;
    }

private:
    int32 CurrentGear() const { return Gears[GearIndex]; }

    void BeginRun(UWorld& World)
    {
        PinkCabPhysicsFixture::DestroyPawns(World);
        Controls = {};
        Controls.SetBrake(1.0f);
        Controls.SetDriveline(0, 0, 0.0f);
        Controls.SetDrivetrainTorqueCapacity(1.0f);
        RestGate.Reset();
        bSettling = true;
        LastMechanicalStep = -1;
        PhaseSimSeconds = 0.0;
    }

    FAutomationTestBase* Test = nullptr;
    FPinkCabCockpitState Cockpit;
    FPinkCabVehicleControlState Controls;
    FPinkCabPhysicsFixtureRestGate RestGate;
    const TArray<int32> Gears{1, -1};
    int32 GearIndex = 0;
    int64 LastMechanicalStep = -1;
    double PhaseSimSeconds = 0.0;
    bool bInitialized = false;
    bool bSettling = false;
    FVector StartLocation = FVector::ZeroVector;
    FVector StartForward = FVector::ForwardVector;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabD4FlatReverseRuntimeTest,
    "PinkCab.Vehicle.Physics.P02.D4.FlatAndReverseLaunch",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabD4FlatReverseRuntimeTest::RunTest(const FString&)
{
    const bool bOpened = AutomationOpenMap(PinkCabPhysicsFixture::MapPath, true);
    TestTrue(TEXT("D4 launch map opens"), bOpened);
    if (!bOpened) return false;
    ADD_LATENT_AUTOMATION_COMMAND(FPinkCabD4FlatReverseCommand(this));
    return true;
}

class FPinkCabD4InclineCommand final : public IAutomationLatentCommand
{
public:
    explicit FPinkCabD4InclineCommand(FAutomationTestBase* InTest)
        : Test(InTest) {}

    virtual bool Update() override
    {
        using namespace PinkCabDrivelineRuntimeTest;
        UWorld* World = AutomationCommon::GetAnyGameWorld();
        if (!World) return false;

        if (!Ramp)
        {
            Ramp = SpawnEightPercentRamp(*World);
            Test->TestNotNull(TEXT("D4 8% incline spawns"), Ramp);
            if (!Ramp) return true;
        }

        APinkCabPhysicsFixturePawn* Pawn = nullptr;
        if (!bInitialized)
        {
            PinkCabPhysicsFixture::DestroyPawns(*World);
            constexpr float Grade = 0.08f;
            const float PitchDeg =
                FMath::RadiansToDegrees(FMath::Atan(Grade));
            Pawn = PinkCabPhysicsFixture::SpawnFreshPawn(
                *World,
                FVector(0.0f, 0.0f, 12280.0f),
                FRotator(PitchDeg, 0.0f, 0.0f));
        }
        else
        {
            Pawn = PinkCabPhysicsFixture::FindOrSpawnPawn(*World);
        }
        if (!Pawn) return false;
        PinkCabPhysicsFixture::KeepAwake(*Pawn);

        UChaosWheeledVehicleMovementComponent* Movement =
            Pawn->GetChaosMovement();
        UPinkCabChaosVehicleMovementComponent* PinkCabMovement =
            Cast<UPinkCabChaosVehicleMovementComponent>(Movement);
        USkeletalMeshComponent* Mesh = Pawn->GetMesh();
        if (!Movement || !PinkCabMovement || !Mesh)
        {
            Test->AddError(TEXT("D4 incline fixture is incomplete"));
            return true;
        }

        if (!bInitialized)
        {
            UGameplayStatics::SetGamePaused(World, false);
            Test->TestTrue(TEXT("D4 incline engine starts"), Cockpit.StartEngine());
            Controls = {};
            Controls.SetBrake(1.0f);
            Controls.SetDriveline(0, 0, 0.0f);
            Controls.SetDrivetrainTorqueCapacity(1.0f);
            RestGate.Reset();
            bInitialized = true;
            return false;
        }

        if (!Apply(*Pawn, Cockpit, Controls))
        {
            Test->AddError(TEXT("D4 incline actuation failed"));
            return true;
        }

        if (!bLaunching)
        {
            if (!RestGate.Update(*Pawn)) return false;
            StartLocation = Mesh->GetComponentLocation();
            StartForward = Pawn->GetActorForwardVector();
            Controls = {};
            Controls.SetThrottle(0.70f);
            Controls.SetDriveline(1, 1, 0.75f);
            Controls.SetDrivetrainTorqueCapacity(1.0f);
            if (!Apply(*Pawn, Cockpit, Controls))
            {
                Test->AddError(
                    TEXT("D4 incline launch command failed authoritative apply"));
                return true;
            }
            if (!PinkCabMovement->BeginPinkCabMechanicalEvidenceWindow(
                    0.0f,
                    static_cast<float>(LaunchTorqueEvidenceSeconds)))
            {
                Test->AddError(
                    TEXT("D4 incline physics-thread torque evidence failed to start"));
                return true;
            }
            LastMechanicalStep =
                PinkCabMovement->GetPinkCabMechanicalIntegrationStepCount();
            PhaseSimSeconds = 0.0;
            bLaunching = true;
            return false;
        }

        if (!AdvanceMechanicalTime(
                *PinkCabMovement, LastMechanicalStep, PhaseSimSeconds))
        {
            return false;
        }

        FPinkCabMechanicalEvidenceSnapshot Evidence;
        if (!PinkCabMovement->ReadPinkCabMechanicalEvidenceWindow(Evidence))
        {
            Test->AddError(
                TEXT("D4 incline physics-thread torque evidence read failed"));
            return true;
        }
        if (Evidence.bComplete && !bLaunchTorqueEvidenceCaptured)
        {
            LaunchMeanSignedRearTorqueNm =
                Evidence.MeanSignedDrivenWheelTorqueNm;
            LaunchMeanAbsRearTorqueNm =
                Evidence.MeanDrivenWheelTorqueNm;
            LaunchEvidenceSteps = Evidence.CompletedSampleSteps;
            bLaunchTorqueEvidenceCaptured = true;
        }

        const float SignedTravel = SignedForwardDistanceCm(
            StartLocation, Mesh->GetComponentLocation(), StartForward);
        if ((!bLaunchTorqueEvidenceCaptured || SignedTravel < 75.0f)
            && PhaseSimSeconds < 6.0)
        {
            return false;
        }

        Test->AddInfo(FString::Printf(
            TEXT("P02_D4_INCLINE grade=0.08 sim_s=%.3f travel_cm=%.3f speed_cm_s=%.3f signed_rear_torque_nm=%.3f abs_rear_torque_nm=%.3f evidence_steps=%d"),
            PhaseSimSeconds,
            SignedTravel,
            HorizontalSpeedCmPerSec(*Mesh),
            LaunchMeanSignedRearTorqueNm,
            LaunchMeanAbsRearTorqueNm,
            LaunchEvidenceSteps));
        Test->TestTrue(TEXT("D4 incline captures completed launch torque evidence"),
            bLaunchTorqueEvidenceCaptured);
        Test->TestTrue(TEXT("D4 drivetrain moves uphill on 8% grade"),
            SignedTravel > 75.0f);
        Test->TestTrue(TEXT("D4 incline sustains positive rear torque under load"),
            LaunchMeanSignedRearTorqueNm > 1.0f);
        Ramp->Destroy();
        Ramp = nullptr;
        return true;
    }

private:
    FAutomationTestBase* Test = nullptr;
    AActor* Ramp = nullptr;
    FPinkCabCockpitState Cockpit;
    FPinkCabVehicleControlState Controls;
    FPinkCabPhysicsFixtureRestGate RestGate;
    static constexpr double LaunchTorqueEvidenceSeconds = 1.0;

    bool bInitialized = false;
    bool bLaunching = false;
    bool bLaunchTorqueEvidenceCaptured = false;
    int32 LaunchEvidenceSteps = 0;
    float LaunchMeanSignedRearTorqueNm = 0.0f;
    float LaunchMeanAbsRearTorqueNm = 0.0f;
    int64 LastMechanicalStep = -1;
    double PhaseSimSeconds = 0.0;
    FVector StartLocation = FVector::ZeroVector;
    FVector StartForward = FVector::ForwardVector;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabD4InclineRuntimeTest,
    "PinkCab.Vehicle.Physics.P02.D4.InclineLoad",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabD4InclineRuntimeTest::RunTest(const FString&)
{
    const bool bOpened = AutomationOpenMap(PinkCabPhysicsFixture::MapPath, true);
    TestTrue(TEXT("D4 incline map opens"), bOpened);
    if (!bOpened) return false;
    ADD_LATENT_AUTOMATION_COMMAND(FPinkCabD4InclineCommand(this));
    return true;
}

// P04 instrumentation uses the unchanged production profile. A successful
// capture is not acceptance of acceleration, a new tune, or a packaged drive.
#include "Runtime/PinkCabChaosTatraPawn.h"
#include "Vehicle/PinkCabChaosPhysicalProfile.h"
#include "Vehicle/PinkCabGearEngagementValidator.h"
#include "PhysicsEngine/BodyInstance.h"

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
    explicit FPinkCabP04BaselineCommand(
        FAutomationTestBase* InTest, bool bInRatioProbe = false,
        bool bInEngineCoherence = false, bool bInStableTraction = false)
        : Test(InTest), WallStart(FPlatformTime::Seconds())
        , bRatioProbe(bInRatioProbe)
        , bEngineCoherence(bInEngineCoherence || bInStableTraction)
        , bStableTraction(bInStableTraction) {}
    virtual ~FPinkCabP04BaselineCommand() override
    {
        RestoreFrameCap();
    }
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
            if (bStableTraction)
            {
                FrameCap = IConsoleManager::Get().FindConsoleVariable(TEXT("t.MaxFPS"));
                if (!FrameCap) { Test->AddError(TEXT("P04 frame cap unavailable")); return true; }
                OriginalFrameCap = FrameCap->GetFloat();
            }
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
        if (bRatioProbe && bSettling)
        {
            // Only this fresh fixture instance changes; the native solver consumes
            // the explicit effective ratio through the normal cockpit/provider command.
            Movement->TransmissionSetup.ForwardGearRatios[0] = ProbeRatio();
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
            FBodyInstance* RootBody = Mesh->GetBodyInstance();
            if (!RootBody) { Test->AddError(TEXT("P04 root body unavailable")); Cleanup(*World); return true; }
            RootMassKg = RootBody->GetBodyMass();
            StartYaw = Mesh->GetComponentRotation().Yaw;
            LastStep = Movement->GetPinkCabMechanicalIntegrationStepCount();
            bSettling = false;
            Controls.SetBrake(0.0f);
            Controls.SetThrottle(Dose());
            Controls.SetDriveline(Gear(), Gear(), 0.0f);
            if (bEngineCoherence
                && !Movement->BeginPinkCabMechanicalEvidenceWindow(
                    bStableTraction ? 2.0f : 0.0f, bStableTraction ? 2.0f : 4.0f))
            {
                Test->AddError(TEXT("P04 engine coherence evidence unavailable"));
                Cleanup(*World);
                return true;
            }
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
            if (bStableTraction)
            {
                GameDeltaSum += FApp::GetDeltaTime();
                ++GameDeltaCount;
            }
            if (Time30 < 0.0 && DirectionalSpeed >= 30.0f) Time30 = Elapsed;
            if (Time60 < 0.0 && DirectionalSpeed >= 60.0f) Time60 = Elapsed;
            PeakRpm = FMath::Max(PeakRpm, Movement->GetEngineRotationSpeed());
            PeakRearTorque = FMath::Max(PeakRearTorque, Torque);
            EndDriveSpeed = DirectionalSpeed;
            EndDriveRpm = Movement->GetEngineRotationSpeed();
            HorizontalEndKmh = HorizontalSpeedCmPerSec(*Mesh) * 0.036f;
            MaxYawDeg = FMath::Max(MaxYawDeg, FMath::Abs(FMath::FindDeltaAngleDegrees(StartYaw, static_cast<float>(Mesh->GetComponentRotation().Yaw))));
            int32 Contacts = 0;
            for (int32 I = 0; I < 4; ++I) Contacts += Movement->GetWheelState(I).bInContact ? 1 : 0;
            MinimumContacts = FMath::Min(MinimumContacts, Contacts);
            if (Elapsed >= 2.0) { TorqueSumNm += Torque; ++TorqueSamples; }
            float MeanWheelSpeedMps = 0.0f;
            for (int32 I = 2; I < 4; ++I)
            {
                const UChaosVehicleWheel* Wheel = Movement->Wheels[I].Get();
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
            TEXT("P04_CASE_DIAGNOSTIC mode=%s ratio=%.4f pedal=%.2f repeat=%d root_mass_kg=%.3f aggregate_mesh_mass_kg=%.3f horizontal_end_kmh=%.3f max_yaw_deg=%.3f min_contacts=%d mean_rear_torque_after2s_nm=%.3f"),
            bRatioProbe ? TEXT("RATIO_PROBE") : TEXT("NOMINAL"),
            Movement->TransmissionSetup.ForwardGearRatios[0], Dose(), CaseIndex % 3 + 1,
            RootMassKg, BodyMassKg, HorizontalEndKmh, MaxYawDeg, MinimumContacts,
            TorqueSamples > 0 ? TorqueSumNm / TorqueSamples : 0.0f));
        Test->AddInfo(FString::Printf(
            TEXT("%s ratio=%.4f gear=%d pedal=%.2f repeat=%d body_mass_kg=%.3f drive_s=%.4f speed_end_kmh=%.3f rpm_end=%.3f time30_s=%.4f time60_s=%.4f peak_rpm=%.3f peak_rear_torque_nm=%.3f peak_slip_speed_mps=%.3f neutral_speed_kmh=%.3f neutral_drive_torque_nm=%.5f timing=OBSERVED_MECHANICAL_STEPS not_reached=-1 fixture=STERILE_NATIVE_NOT_PACKAGED"),
            bRatioProbe ? TEXT("P04_RATIO_PROBE") : TEXT("P04_ACCEL_BASELINE"),
            Movement->TransmissionSetup.ForwardGearRatios[0], Gear(), Dose(), CaseIndex % 3 + 1, BodyMassKg, DriveEndTime, EndDriveSpeed, EndDriveRpm,
            Time30, Time60, PeakRpm, PeakRearTorque, PeakSlipSpeedMps, DirectionalSpeed, Torque));
        if (bEngineCoherence)
        {
            FPinkCabMechanicalEvidenceSnapshot Evidence;
            const bool bRead = Movement->ReadPinkCabMechanicalEvidenceWindow(Evidence);
            Test->TestTrue(TEXT("P04 engine coherence window completed"), bRead && Evidence.bComplete);
            Test->AddInfo(FString::Printf(
                TEXT("P04_ENGINE_COHERENCE repeat=%d steps=%d max_input_state_error_rpm=%.6f speed_end_kmh=%.3f"),
                CaseIndex + 1, Evidence.CompletedSampleSteps,
                Evidence.MaxEngineInputStateErrorRpm, EndDriveSpeed));
            Test->TestTrue(
                TEXT("P04 clutch predictor consumes the actual same-step engine angular state"),
                bRead && Evidence.bComplete
                    && Evidence.MaxEngineInputStateErrorRpm <= 1.0f);
            if (bStableTraction)
            {
                // This steady, level, no-brake window must not alternate
                // propulsion and braking every physics step at constant input.
                Test->TestTrue(TEXT("P04 steady half-throttle carries positive traction"),
                    Evidence.MeanSignedDrivenWheelTorqueNm > 1.0f);
                Test->TestTrue(TEXT("P04 steady traction has no spurious opposing impulse"),
                    Evidence.MeanDrivenWheelTorqueNm
                        <= Evidence.MeanSignedDrivenWheelTorqueNm * 1.01f + 1.0f);
                Test->TestTrue(TEXT("P04 clutch and tyre exchange use resolved substeps"),
                    Evidence.MaxDeltaSeconds <= 1.0f / 240.0f + 1.0e-6f);
                Test->TestTrue(TEXT("P04 no brake or parking injection masks the defect"),
                    Evidence.MeanAppliedWheelBrakeTorqueNm <= 0.1f
                        && !Evidence.bAnyParkingEnabled);
                Test->AddInfo(FString::Printf(
                    TEXT("P04_STABLE_TRACTION fps_cap=%d repeat=%d game_dt_ms=%.6f abs_rear_nm=%.6f signed_rear_nm=%.6f max_step_ms=%.6f speed_end_kmh=%.3f"),
                    StableFrameCap(), CaseIndex % 3 + 1,
                    GameDeltaCount > 0 ? GameDeltaSum / GameDeltaCount * 1000.0 : 0.0,
                    Evidence.MeanDrivenWheelTorqueNm,
                    Evidence.MeanSignedDrivenWheelTorqueNm,
                    Evidence.MaxDeltaSeconds * 1000.0f, EndDriveSpeed));
                StableSpeeds.Add(EndDriveSpeed);
            }
        }
        if (++CaseIndex == (bStableTraction ? 9 : bEngineCoherence ? 3 : 18))
        {
            if (bStableTraction)
            {
                Test->TestEqual(TEXT("P04 executes three repeats at all three frame caps"), StableSpeeds.Num(), 9);
                auto Median = [this](int32 Offset)
                {
                    TArray<float> Group{StableSpeeds[Offset], StableSpeeds[Offset + 1], StableSpeeds[Offset + 2]};
                    Group.Sort();
                    return Group[1];
                };
                if (StableSpeeds.Num() == 9)
                {
                    const float Reference = Median(0);
                    for (int32 Offset : {3, 6})
                    {
                        Test->TestTrue(TEXT("P04 five-second traction is render-cadence invariant"),
                            FMath::Abs(Median(Offset) - Reference) / FMath::Max(Reference, 1.0f) <= 0.05f);
                    }
                }
            }
            Test->AddInfo(bStableTraction
                ? TEXT("P04_STABLE_TRACTION_COMPLETE cases=9 frame_caps=30,60,120 profile_changed=0")
                : bEngineCoherence
                ? TEXT("P04_ENGINE_COHERENCE_COMPLETE cases=3 profile_changed=0")
                : bRatioProbe
                ? TEXT("P04_RATIO_PROBE_COMPLETE cases=18 production_profile_changed=0 tuning_accepted=0")
                : TEXT("P04_BASELINE_COMPLETE cases=18 profile_changed=0 tuning_accepted=0"));
            Cleanup(*World);
            return true;
        }
        BeginCase(*World);
        return false;
    }
private:
    int32 Gear() const { return bRatioProbe || CaseIndex < 9 ? 1 : -1; }
    float Dose() const
    {
        if (bEngineCoherence) return 0.50f;
        if (bRatioProbe) return (CaseIndex / 3) % 2 == 0 ? 0.50f : 1.00f;
        const float Values[] = {0.25f, 0.50f, 1.00f}; return Values[(CaseIndex / 3) % 3];
    }
    float ProbeRatio() const { const float Ratios[] = {4.60f, 4.00f, 3.60f}; return Ratios[CaseIndex / 6]; }
    int32 StableFrameCap() const
    {
        const int32 Caps[] = {30, 60, 120};
        return Caps[CaseIndex / 3];
    }
    void RestoreFrameCap()
    {
        if (FrameCap)
        {
            FrameCap->Set(OriginalFrameCap, ECVF_SetByCode);
            FrameCap = nullptr;
        }
    }
    void BeginCase(UWorld& World)
    {
        if (FrameCap) FrameCap->Set(static_cast<float>(StableFrameCap()), ECVF_SetByCode);
        GameDeltaSum = 0.0;
        GameDeltaCount = 0;
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
        RootMassKg = HorizontalEndKmh = MaxYawDeg = TorqueSumNm = 0.0f;
        TorqueSamples = 0; MinimumContacts = 4;
        WallStart = FPlatformTime::Seconds();
    }
    void Cleanup(UWorld& World)
    {
        RestoreFrameCap();
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
    float RootMassKg = 0.0f, StartYaw = 0.0f, HorizontalEndKmh = 0.0f, MaxYawDeg = 0.0f, TorqueSumNm = 0.0f;
    int32 TorqueSamples = 0, MinimumContacts = 4;
    bool bInitialized = false, bSettling = true, bCoasting = false;
    const bool bRatioProbe;
    const bool bEngineCoherence;
    const bool bStableTraction;
    IConsoleVariable* FrameCap = nullptr;
    float OriginalFrameCap = 0.0f;
    double GameDeltaSum = 0.0;
    int32 GameDeltaCount = 0;
    TArray<float> StableSpeeds;
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabP04RatioProbeRuntimeTest,
    "PinkCab.Vehicle.Physics.P04.GearRatioProbe",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabP04RatioProbeRuntimeTest::RunTest(const FString&)
{
    if (!TestTrue(TEXT("P04 comparison fixture map opens"), AutomationOpenMap(PinkCabPhysicsFixture::MapPath, true))) return false;
    ADD_LATENT_AUTOMATION_COMMAND(FPinkCabP04BaselineCommand(this, true));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabP04EngineStateCoherenceTest,
    "PinkCab.Vehicle.Physics.P04.EngineStateCoherence",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabP04EngineStateCoherenceTest::RunTest(const FString&)
{
    if (!TestTrue(TEXT("P04 engine coherence fixture opens"),
        AutomationOpenMap(PinkCabPhysicsFixture::MapPath, true))) return false;
    ADD_LATENT_AUTOMATION_COMMAND(FPinkCabP04BaselineCommand(this, false, true));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabP04StableTractionTest,
    "PinkCab.Vehicle.Physics.P04.StableHalfThrottleTraction",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabP04StableTractionTest::RunTest(const FString&)
{
    if (!TestTrue(TEXT("P04 steady traction fixture opens"),
        AutomationOpenMap(PinkCabPhysicsFixture::MapPath, true))) return false;
    ADD_LATENT_AUTOMATION_COMMAND(FPinkCabP04BaselineCommand(this, false, false, true));
    return true;
}

#endif
