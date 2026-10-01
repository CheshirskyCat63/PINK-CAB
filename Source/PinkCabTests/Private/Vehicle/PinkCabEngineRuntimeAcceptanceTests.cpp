#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "EngineUtils.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Vehicle/PinkCabPhysicsFixturePawn.h"
#include "Vehicle/PinkCabChaosCockpitBridge.h"
#include "Vehicle/PinkCabChaosVehicleDynamicsProvider.h"
#include "Vehicle/PinkCabChaosVehicleMovementComponent.h"
#include "Vehicle/PinkCabCockpitState.h"
#include "Vehicle/PinkCabVehicleControlState.h"

namespace
{
APinkCabPhysicsFixturePawn* FindTatra(UWorld& World)
{
    PinkCabPhysicsFixture::FindOrSpawnFlatFloor(World);
    return PinkCabPhysicsFixture::FindOrSpawnPawn(World);
}

bool ApplyEngineState(
    APinkCabPhysicsFixturePawn& Pawn,
    FPinkCabCockpitState& Cockpit,
    FPinkCabVehicleControlState& Controls)
{
    UChaosWheeledVehicleMovementComponent* Movement = Pawn.GetChaosMovement();
    if (!Movement)
    {
        return false;
    }
    FPinkCabChaosVehicleDynamicsProvider Provider(Movement);
    return FPinkCabChaosCockpitBridge::Apply(
        Cockpit, *Movement, Controls, Provider);
}

bool AdvanceMechanicalTime(
    UPinkCabChaosVehicleMovementComponent& Movement,
    int64& LastStep,
    double& ElapsedSimSeconds)
{
    const int64 CurrentStep =
        Movement.GetPinkCabMechanicalIntegrationStepCount();
    if (LastStep < 0)
    {
        LastStep = CurrentStep;
        return false;
    }
    if (CurrentStep <= LastStep)
    {
        return false;
    }

    const int64 StepDelta = CurrentStep - LastStep;
    const float DeltaSeconds =
        Movement.GetPinkCabLastMechanicalIntegrationDeltaSeconds();
    LastStep = CurrentStep;
    if (DeltaSeconds <= 0.0f)
    {
        return false;
    }

    ElapsedSimSeconds +=
        static_cast<double>(StepDelta)
        * static_cast<double>(DeltaSeconds);
    return true;
}
}

class FPinkCabEngineOffSlopeCommand final : public IAutomationLatentCommand
{
public:
    explicit FPinkCabEngineOffSlopeCommand(FAutomationTestBase* InTest)
        : Test(InTest) {}

    virtual bool Update() override
    {
        UWorld* World = AutomationCommon::GetAnyGameWorld();
        if (!World)
        {
            return false;
        }

        if (!bCleanupPrepared)
        {
            UGameplayStatics::SetGamePaused(World, false);
            PinkCabPhysicsFixture::DestroyPawns(*World);

            // Destroy() is deferred until the world finishes the frame. In a
            // full automation suite the previous sterile Chaos pawn can still
            // own a physics proxy during this Update(), even though the actor
            // is already marked for destruction. Give teardown two complete
            // world frames before creating the slope fixture so this test has
            // the same physics-scene baseline as a fresh process.
            CleanupFramesRemaining = 2;
            bCleanupPrepared = true;
            return false;
        }

        if (CleanupFramesRemaining > 0)
        {
            --CleanupFramesRemaining;
            return false;
        }

        if (!bInitialized)
        {
            constexpr float RampPitchDeg = 20.0f;

            Ramp = World->SpawnActor<AActor>();
            Test->TestNotNull(TEXT("isolated slope actor spawns"), Ramp);
            if (!Ramp)
            {
                return true;
            }

            UBoxComponent* RampBox = NewObject<UBoxComponent>(Ramp);
            Ramp->SetRootComponent(RampBox);
            RampBox->SetBoxExtent(FVector(1200.0f, 350.0f, 60.0f));
            RampBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            RampBox->SetCollisionObjectType(ECC_WorldStatic);
            RampBox->SetCollisionResponseToAllChannels(ECR_Block);
            RampBox->SetGenerateOverlapEvents(false);
            RampBox->RegisterComponent();
            Ramp->SetActorRotation(FRotator(RampPitchDeg, 0.0f, 0.0f));
            Ramp->SetActorLocation(FVector(0.0f, 0.0f, 12000.0f));
            RampBox->RecreatePhysicsState();

            FHitResult RampProbe;
            const bool bRampProbeHit = World->LineTraceSingleByChannel(
                RampProbe,
                FVector(0.0f, 0.0f, 12500.0f),
                FVector(0.0f, 0.0f, 11500.0f),
                ECC_Visibility);
            Test->TestTrue(
                TEXT("isolated slope fixture has query collision"),
                bRampProbeHit);
            if (!bRampProbeHit)
            {
                return true;
            }

            APinkCabPhysicsFixturePawn* SpawnedPawn =
                PinkCabPhysicsFixture::SpawnFreshPawn(
                    *World,
                    FVector(0.0f, 0.0f, 12280.0f),
                    FRotator(RampPitchDeg, 0.0f, 0.0f));
            Test->TestNotNull(
                TEXT("slope fixture fresh pawn spawns in-place"),
                SpawnedPawn);
            if (!SpawnedPawn)
            {
                return true;
            }

            FixturePawn = SpawnedPawn;
            SpawnedPawn->SetActorTickEnabled(false);

            UChaosWheeledVehicleMovementComponent* Movement =
                SpawnedPawn->GetChaosMovement();
            UPinkCabChaosVehicleMovementComponent* PinkCabMovement =
                Cast<UPinkCabChaosVehicleMovementComponent>(Movement);
            USkeletalMeshComponent* Mesh = SpawnedPawn->GetMesh();
            Test->TestNotNull(TEXT("slope fixture has movement"), Movement);
            Test->TestNotNull(
                TEXT("slope fixture has exact mechanical clock"),
                PinkCabMovement);
            Test->TestNotNull(TEXT("slope fixture has physics mesh"), Mesh);
            if (!Movement || !PinkCabMovement || !Mesh)
            {
                return true;
            }

            // Preserve the accepted P01 gravity-coast fixture: no service or
            // parking brake is allowed to create an artificial static state.
            Controls.SetThrottle(1.0f);
            Controls.SetBrake(0.0f);
            Controls.SetHandbrake(0.0f);
            Controls.SetDriveline(0, 0, 0.0f);
            Test->TestTrue(TEXT("off neutral slope gravity state applies"),
                ApplyEngineState(*SpawnedPawn, Cockpit, Controls));
            Test->TestFalse(TEXT("off slope denies combustion"),
                Controls.IsCombustionAllowed());
            Test->TestEqual(TEXT("off slope final throttle is zero"),
                Controls.GetResolvedEngineThrottle01(), 0.0f);
            Test->TestEqual(TEXT("off slope external drive torque is zero"),
                Controls.ExternalRearDriveTorquePerWheelNm, 0.0f);
            Test->TestTrue(
                TEXT("off slope keeps physical driveline simulation alive"),
                Movement->bMechanicalSimEnabled);
            Test->TestTrue(
                TEXT("slope brake-path evidence window starts"),
                PinkCabMovement->BeginPinkCabMechanicalEvidenceWindow(
                    0.0f, static_cast<float>(BrakeEvidenceSeconds)));

            const FVector Downhill3D =
                FVector::VectorPlaneProject(
                    FVector::DownVector,
                    Ramp->GetActorUpVector())
                    .GetSafeNormal();
            DownhillDirection2D =
                FVector2D(Downhill3D.X, Downhill3D.Y).GetSafeNormal();
            Test->TestTrue(
                TEXT("slope fixture exposes gravity-projected downhill direction"),
                !DownhillDirection2D.IsNearlyZero());

            LastMechanicalStep =
                PinkCabMovement->GetPinkCabMechanicalIntegrationStepCount();
            PhaseSimSeconds = 0.0;
            StableContactObservations = 0;
            bInitialized = true;
            return false;
        }

        APinkCabPhysicsFixturePawn* Pawn = FixturePawn.Get();
        if (!Pawn || !Ramp)
        {
            Test->AddError(TEXT("slope fixture disappeared during runtime"));
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
            Test->AddError(TEXT("slope fixture runtime state is incomplete"));
            return true;
        }

        if (!AdvanceMechanicalTime(
                *PinkCabMovement,
                LastMechanicalStep,
                PhaseSimSeconds))
        {
            return false;
        }

        if (!bContactReady)
        {
            int32 ContactWheels = 0;
            float TotalSpringForce = 0.0f;
            for (int32 WheelIndex = 0;
                 WheelIndex < Movement->GetNumWheels();
                 ++WheelIndex)
            {
                const FWheelStatus& Wheel =
                    Movement->GetWheelState(WheelIndex);
                if (Wheel.bInContact)
                {
                    ++ContactWheels;
                    TotalSpringForce += FMath::Max(Wheel.SpringForce, 0.0f);
                }
            }

            const FVector BodyVelocity =
                Mesh->GetPhysicsLinearVelocity();
            const float NormalSpeedCmPerSec = FMath::Abs(
                FVector::DotProduct(
                    BodyVelocity,
                    Ramp->GetActorUpVector()));
            const FVector2D HorizontalVelocity(
                BodyVelocity.X, BodyVelocity.Y);
            const float DownhillSpeedCmPerSec =
                FVector2D::DotProduct(
                    HorizontalVelocity, DownhillDirection2D);
            const float BodyAngularSpeedDegPerSec =
                Mesh->GetPhysicsAngularVelocityInDegrees().Size();

            float CurrentMaxAbsWheelAngularVelocity = 0.0f;
            for (const UChaosVehicleWheel* Wheel : Movement->Wheels)
            {
                if (Wheel)
                {
                    CurrentMaxAbsWheelAngularVelocity = FMath::Max(
                        CurrentMaxAbsWheelAngularVelocity,
                        FMath::Abs(Wheel->GetWheelAngularVelocity()));
                }
            }
            if (ContactWheels == Movement->GetNumWheels()
                && TotalSpringForce > KINDA_SMALL_NUMBER)
            {
                MaxSupportedDownhillSpeedCmPerSec = FMath::Max(
                    MaxSupportedDownhillSpeedCmPerSec,
                    DownhillSpeedCmPerSec);
                MaxSupportedWheelAngularVelocity = FMath::Max(
                    MaxSupportedWheelAngularVelocity,
                    CurrentMaxAbsWheelAngularVelocity);
            }

            const bool bSupportedRollingContact =
                ContactWheels == Movement->GetNumWheels()
                && TotalSpringForce > KINDA_SMALL_NUMBER
                && NormalSpeedCmPerSec
                    <= ContactNormalSpeedToleranceCmPerSec
                && DownhillSpeedCmPerSec
                    >= ContactDownhillSpeedMinimumCmPerSec;

            if (PhaseSimSeconds >= MinimumContactSettleSeconds
                && bSupportedRollingContact)
            {
                ++StableContactObservations;
            }
            else
            {
                StableContactObservations = 0;
            }

            if (StableContactObservations
                < RequiredContactObservations)
            {
                if (PhaseSimSeconds < ContactReadyTimeoutSeconds)
                {
                    return false;
                }

                Test->AddError(FString::Printf(
                    TEXT("P01 slope fixture never reached supported downhill rolling sim_s=%.3f contacts=%d/%d spring_force=%.3f normal_speed_cm_s=%.3f downhill_speed_cm_s=%.3f max_supported_downhill_speed_cm_s=%.3f max_supported_wheel_rad_s=%.3f brake_input=%.3f handbrake_command=%.3f body_angular_deg_s=%.3f stable_observations=%d"),
                    PhaseSimSeconds,
                    ContactWheels,
                    Movement->GetNumWheels(),
                    TotalSpringForce,
                    NormalSpeedCmPerSec,
                    DownhillSpeedCmPerSec,
                    MaxSupportedDownhillSpeedCmPerSec,
                    MaxSupportedWheelAngularVelocity,
                    Movement->GetBrakeInput(),
                    PinkCabMovement->GetPendingPinkCabDrivelineCommand().Handbrake01,
                    BodyAngularSpeedDegPerSec,
                    StableContactObservations));
                return true;
            }

            StartLocation = Mesh->GetComponentLocation();
            StartVelocity = Mesh->GetPhysicsLinearVelocity();

            FPinkCabMechanicalEvidenceSnapshot BrakeEvidence;
            if (!PinkCabMovement->ReadPinkCabMechanicalEvidenceWindow(
                    BrakeEvidence))
            {
                Test->AddError(TEXT("slope brake-path evidence read failed"));
                return true;
            }
            Test->AddInfo(FString::Printf(
                TEXT("P01_SLOPE_CONTACT_READY sim_s=%.3f contacts=%d/%d spring_force=%.3f normal_speed_cm_s=%.3f downhill_speed_cm_s=%.3f body_angular_deg_s=%.3f stable_observations=%d mean_applied_brake_torque_nm=%.6f parking_enabled=%d evidence_s=%.6f start=%s"),
                PhaseSimSeconds,
                ContactWheels,
                Movement->GetNumWheels(),
                TotalSpringForce,
                NormalSpeedCmPerSec,
                DownhillSpeedCmPerSec,
                BodyAngularSpeedDegPerSec,
                StableContactObservations,
                BrakeEvidence.MeanAppliedWheelBrakeTorqueNm,
                BrakeEvidence.bAnyParkingEnabled ? 1 : 0,
                BrakeEvidence.CompletedSampleSeconds,
                *StartLocation.ToString()));
            Test->TestFalse(
                TEXT("slope physics thread has no parking brake state"),
                BrakeEvidence.bAnyParkingEnabled);
            Test->TestTrue(
                TEXT("slope physics thread applies zero wheel brake torque"),
                BrakeEvidence.MeanAppliedWheelBrakeTorqueNm <= 1.0e-3f);

            bContactReady = true;
            PhaseSimSeconds = 0.0;
            return false;
        }

        if (PhaseSimSeconds < MeasurementSeconds)
        {
            return false;
        }

        const FVector EndLocation = Mesh->GetComponentLocation();
        const FVector EndVelocity = Mesh->GetPhysicsLinearVelocity();
        const FVector2D HorizontalDelta(
            EndLocation.X - StartLocation.X,
            EndLocation.Y - StartLocation.Y);
        const FVector2D EndHorizontalVelocity(
            EndVelocity.X, EndVelocity.Y);
        const FVector2D StartHorizontalVelocity(
            StartVelocity.X, StartVelocity.Y);
        const float DownhillTravelCm =
            FVector2D::DotProduct(HorizontalDelta, DownhillDirection2D);
        const float DownhillSpeedCmPerSec =
            FVector2D::DotProduct(
                EndHorizontalVelocity, DownhillDirection2D);
        const float StartDownhillSpeedCmPerSec =
            FVector2D::DotProduct(
                StartHorizontalVelocity, DownhillDirection2D);

        Test->AddInfo(FString::Printf(
            TEXT("P01_SLOPE downhill_travel_cm=%.3f start_downhill_speed_cm_s=%.3f end_downhill_speed_cm_s=%.3f start=%s end=%s"),
            DownhillTravelCm,
            StartDownhillSpeedCmPerSec,
            DownhillSpeedCmPerSec,
            *StartLocation.ToString(),
            *EndLocation.ToString()));

        float MaxAbsWheelAngularVelocity = 0.0f;
        for (const UChaosVehicleWheel* Wheel : Movement->Wheels)
        {
            if (Wheel)
            {
                MaxAbsWheelAngularVelocity = FMath::Max(
                    MaxAbsWheelAngularVelocity,
                    FMath::Abs(Wheel->GetWheelAngularVelocity()));
            }
        }
        FPinkCabMechanicalEvidenceSnapshot FinalBrakeEvidence;
        if (!PinkCabMovement->ReadPinkCabMechanicalEvidenceWindow(
                FinalBrakeEvidence))
        {
            Test->AddError(TEXT("slope final brake-path evidence read failed"));
            return true;
        }
        Test->AddInfo(FString::Printf(
            TEXT("P01_SLOPE max_abs_wheel_rad_s=%.3f mean_applied_brake_torque_nm=%.6f parking_enabled=%d evidence_s=%.6f"),
            MaxAbsWheelAngularVelocity,
            FinalBrakeEvidence.MeanAppliedWheelBrakeTorqueNm,
            FinalBrakeEvidence.bAnyParkingEnabled ? 1 : 0,
            FinalBrakeEvidence.CompletedSampleSeconds));
        Test->TestFalse(
            TEXT("slope final physics evidence has no parking state"),
            FinalBrakeEvidence.bAnyParkingEnabled);
        Test->TestTrue(
            TEXT("slope final physics evidence has zero wheel brake torque"),
            FinalBrakeEvidence.MeanAppliedWheelBrakeTorqueNm <= 1.0e-3f);

        Test->TestTrue(TEXT("off neutral vehicle moves downhill under gravity"),
            DownhillTravelCm > 20.0f);
        Test->TestTrue(TEXT("slope vehicle remains physically rolling downhill"),
            DownhillSpeedCmPerSec > 10.0f);
        Test->TestTrue(
            TEXT("gravity increases downhill speed after supported contact"),
            DownhillSpeedCmPerSec
                > StartDownhillSpeedCmPerSec + 5.0f);
        Test->TestTrue(TEXT("slope motion includes wheel rotation"),
            MaxAbsWheelAngularVelocity > 0.05f);
        Test->TestEqual(TEXT("slope coast remains zero engine throttle"),
            Movement->GetThrottleInput(), 0.0f);
        Test->TestEqual(TEXT("slope coast remains zero external drive torque"),
            Controls.ExternalRearDriveTorquePerWheelNm, 0.0f);
        Test->TestEqual(TEXT("slope coast releases service brake"),
            Movement->GetBrakeInput(), 0.0f);

        if (APinkCabPhysicsFixturePawn* FinishedPawn = FixturePawn.Get())
        {
            FinishedPawn->Destroy();
        }
        FixturePawn.Reset();

        Ramp->Destroy();
        Ramp = nullptr;
        return true;
    }

private:
    FAutomationTestBase* Test = nullptr;
    AActor* Ramp = nullptr;
    TWeakObjectPtr<APinkCabPhysicsFixturePawn> FixturePawn;
    FPinkCabCockpitState Cockpit;
    FPinkCabVehicleControlState Controls;

    static constexpr double MinimumContactSettleSeconds = 0.10;
    static constexpr double ContactReadyTimeoutSeconds = 3.00;
    // Measure a sustained supported coast, not one suspension-bounce phase.
    static constexpr double MeasurementSeconds = 2.40;
    static constexpr double BrakeEvidenceSeconds = 3.00;
    static constexpr float ContactNormalSpeedToleranceCmPerSec = 10.0f;
    static constexpr float ContactDownhillSpeedMinimumCmPerSec = 10.0f;
    static constexpr int32 RequiredContactObservations = 3;

    bool bCleanupPrepared = false;
    int32 CleanupFramesRemaining = 0;
    bool bInitialized = false;
    bool bContactReady = false;
    int32 StableContactObservations = 0;
    int64 LastMechanicalStep = -1;
    double PhaseSimSeconds = 0.0;
    FVector StartLocation = FVector::ZeroVector;
    FVector StartVelocity = FVector::ZeroVector;
    FVector2D DownhillDirection2D = FVector2D::ZeroVector;
    float MaxSupportedDownhillSpeedCmPerSec = 0.0f;
    float MaxSupportedWheelAngularVelocity = 0.0f;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabEngineOffNeutralSlopeRuntimeTest,
    "PinkCab.Vehicle.Physics.EngineState.LiveOffNeutralSlope",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabEngineOffNeutralSlopeRuntimeTest::RunTest(const FString& Parameters)
{
    const bool bOpened = AutomationOpenMap(
        PinkCabPhysicsFixture::MapPath, true);
    TestTrue(TEXT("slope runtime map opens"), bOpened);
    if (!bOpened)
    {
        return false;
    }
    ADD_LATENT_AUTOMATION_COMMAND(FPinkCabEngineOffSlopeCommand(this));
    return true;
}

class FPinkCabWarmIdleBlipCommand final : public IAutomationLatentCommand
{
public:
    explicit FPinkCabWarmIdleBlipCommand(FAutomationTestBase* InTest)
        : Test(InTest) {}

    virtual bool Update() override
    {
        UWorld* World = AutomationCommon::GetAnyGameWorld();
        if (!World)
        {
            return false;
        }

        APinkCabPhysicsFixturePawn* Pawn = FindTatra(*World);
        if (!Pawn)
        {
            return false;
        }
        PinkCabPhysicsFixture::KeepAwake(*Pawn);
        UChaosWheeledVehicleMovementComponent* Movement =
            Pawn->GetChaosMovement();
        UPinkCabChaosVehicleMovementComponent* PinkCabMovement =
            Cast<UPinkCabChaosVehicleMovementComponent>(Movement);
        Test->TestNotNull(TEXT("idle fixture has movement"), Movement);
        Test->TestNotNull(
            TEXT("idle fixture has exact mechanical clock"),
            PinkCabMovement);
        if (!Movement || !PinkCabMovement)
        {
            return true;
        }

        if (!bInitialized)
        {
                        UGameplayStatics::SetGamePaused(World, false);
            Pawn->SetActorTickEnabled(false);
            if (USkeletalMeshComponent* Mesh = Pawn->GetMesh())
            {
                Mesh->WakeAllRigidBodies();
            }
            Cockpit.StartEngine();
            Controls.SetBrake(0.0f);
            Controls.SetHandbrake(1.0f);
            Controls.SetDriveline(0, 0, 0.0f);
            Controls.SetThrottle(0.0f);
            Test->TestTrue(TEXT("warm idle engine state applies"),
                ApplyEngineState(*Pawn, Cockpit, Controls));
            Test->TestTrue(TEXT("warm idle enables mechanical sim"),
                Movement->bMechanicalSimEnabled);
            Phase = EPhase::SettlingIdle;
            LastMechanicalStep =
                PinkCabMovement->GetPinkCabMechanicalIntegrationStepCount();
            PhaseSimSeconds = 0.0;
            bInitialized = true;
            return false;
        }

        if (!AdvanceMechanicalTime(
                *PinkCabMovement,
                LastMechanicalStep,
                PhaseSimSeconds))
        {
            return false;
        }

        if (Phase == EPhase::SettlingIdle)
        {
            if (PhaseSimSeconds < 1.50)
            {
                return false;
            }
            IdleBeforeBlip = Movement->GetEngineRotationSpeed();
            Test->AddInfo(FString::Printf(
                TEXT("P01_IDLE settled_rpm=%.3f configured_idle=%.3f"),
                IdleBeforeBlip,
                Movement->EngineSetup.EngineIdleRPM));
            Test->TestTrue(TEXT("warm healthy neutral idle settles inside 900-950 rpm"),
                IdleBeforeBlip >= 900.0f && IdleBeforeBlip <= 950.0f);

            Controls.SetThrottle(0.55f);
            Test->TestTrue(TEXT("throttle blip applies through authoritative bridge"),
                ApplyEngineState(*Pawn, Cockpit, Controls));
            Phase = EPhase::Blipping;
            PhaseSimSeconds = 0.0;
            return false;
        }

        if (Phase == EPhase::Blipping)
        {
            if (PhaseSimSeconds < 0.70)
            {
                return false;
            }
            const float BlipRpm = Movement->GetEngineRotationSpeed();
            Test->AddInfo(FString::Printf(TEXT("P01_IDLE blip_rpm=%.3f"), BlipRpm));
            Test->TestTrue(TEXT("neutral throttle blip raises rpm above idle"),
                BlipRpm > IdleBeforeBlip + 150.0f);

            Controls.SetThrottle(0.0f);
            Test->TestTrue(TEXT("blip release applies zero driver throttle"),
                ApplyEngineState(*Pawn, Cockpit, Controls));
            Phase = EPhase::ReturningIdle;
            PhaseSimSeconds = 0.0;
            return false;
        }

        const float ReturnedIdleRpm = Movement->GetEngineRotationSpeed();
        if (ReturnedIdleRpm >= 900.0f && ReturnedIdleRpm <= 950.0f)
        {
            Test->AddInfo(FString::Printf(
                TEXT("P01_IDLE returned_rpm=%.3f return_seconds=%.3f"),
                ReturnedIdleRpm,
                PhaseSimSeconds));
            Test->TestEqual(TEXT("released blip has zero final throttle"),
                Controls.GetResolvedEngineThrottle01(), 0.0f);
            return true;
        }

        if (PhaseSimSeconds < 8.00)
        {
            return false;
        }

        Test->AddInfo(FString::Printf(
            TEXT("P01_IDLE return_timeout_rpm=%.3f return_seconds=%.3f"),
            ReturnedIdleRpm,
            PhaseSimSeconds));
        Test->TestTrue(TEXT("engine returns to warm carb idle band after blip"),
            ReturnedIdleRpm >= 900.0f && ReturnedIdleRpm <= 950.0f);
        Test->TestEqual(TEXT("released blip has zero final throttle"),
            Controls.GetResolvedEngineThrottle01(), 0.0f);
        return true;
    }

private:
    enum class EPhase : uint8
    {
        SettlingIdle,
        Blipping,
        ReturningIdle
    };

    FAutomationTestBase* Test = nullptr;
    FPinkCabCockpitState Cockpit;
    FPinkCabVehicleControlState Controls;
    bool bInitialized = false;
    EPhase Phase = EPhase::SettlingIdle;
    int64 LastMechanicalStep = -1;
    double PhaseSimSeconds = 0.0;
    float IdleBeforeBlip = 0.0f;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabWarmIdleBlipRuntimeTest,
    "PinkCab.Vehicle.Physics.EngineState.LiveWarmIdleBlipReturn",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabWarmIdleBlipRuntimeTest::RunTest(const FString& Parameters)
{
    const bool bOpened = AutomationOpenMap(
        PinkCabPhysicsFixture::MapPath, true);
    TestTrue(TEXT("idle runtime map opens"), bOpened);
    if (!bOpened)
    {
        return false;
    }
    ADD_LATENT_AUTOMATION_COMMAND(FPinkCabWarmIdleBlipCommand(this));
    return true;
}

#endif
