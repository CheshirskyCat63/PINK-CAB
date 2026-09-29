#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Kismet/GameplayStatics.h"
#include "Vehicle/PinkCabDrivelineRuntimeTestUtils.h"
#include "Vehicle/PinkCabVehicleControlRuntime.h"
#include "Vehicle/PinkCabVehicleHealthState.h"
#include "Vehicle/PinkCabVehicleInputFrame.h"

class FPinkCabD4EngineBrakeStallCommand final
    : public IAutomationLatentCommand
{
public:
    explicit FPinkCabD4EngineBrakeStallCommand(FAutomationTestBase* InTest)
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
            Test->AddError(TEXT("D4 engine-brake fixture failed to spawn"));
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
            Test->AddError(TEXT("D4 engine-brake fixture is incomplete"));
            return true;
        }

        if (!bInitialized)
        {
            UGameplayStatics::SetGamePaused(World, false);
            Test->TestTrue(TEXT("D4 engine-brake engine starts"), Cockpit.StartEngine());
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
            Test->AddError(TEXT("D4 engine-brake actuation failed"));
            return true;
        }

        if (Phase == EPhase::Settling)
        {
            if (!RestGate.Update(*Pawn)) return false;
            Controls = {};
            Controls.SetThrottle(0.60f);
            Controls.SetDriveline(1, 1, 0.75f);
            Controls.SetDrivetrainTorqueCapacity(1.0f);
            ResetPhaseClock(*PinkCabMovement);
            Phase = EPhase::Launching;
            return false;
        }

        if (!AdvanceMechanicalTime(
                *PinkCabMovement, LastMechanicalStep, PhaseSimSeconds))
        {
            return false;
        }

        if (Phase == EPhase::Launching)
        {
            const float Speed = HorizontalSpeedCmPerSec(*Mesh);
            if (Speed < 300.0f && PhaseSimSeconds < 6.0)
            {
                return false;
            }
            if (Speed < 300.0f)
            {
                Test->AddError(FString::Printf(
                    TEXT("D4 could not establish engine-braking speed speed_cm_s=%.3f"),
                    Speed));
                return true;
            }

            Controls.SetThrottle(0.60f);
            Controls.SetBrake(0.0f);
            Controls.SetDriveline(1, 1, 1.0f);
            ResetPhaseClock(*PinkCabMovement);
            Phase = EPhase::Synchronizing;
            return false;
        }

        if (Phase == EPhase::Synchronizing)
        {
            if (PhaseSimSeconds < FullCouplingSyncSeconds)
            {
                return false;
            }

            LiftStartSpeedCmPerSec = HorizontalSpeedCmPerSec(*Mesh);
            Controls.SetThrottle(0.0f);
            Controls.SetBrake(0.0f);
            Controls.SetDriveline(1, 1, 1.0f);
            if (!PinkCabMovement->BeginPinkCabMechanicalEvidenceWindow(
                    0.0f,
                    static_cast<float>(EngineBrakeMeasurementSeconds)))
            {
                Test->AddError(TEXT("D4 engine-brake physics-thread evidence window failed"));
                return true;
            }
            ResetPhaseClock(*PinkCabMovement);
            Phase = EPhase::LiftOff;
            return false;
        }

        if (Phase == EPhase::LiftOff)
        {
            FPinkCabMechanicalEvidenceSnapshot Evidence;
            if (!PinkCabMovement->ReadPinkCabMechanicalEvidenceWindow(Evidence))
            {
                Test->AddError(TEXT("D4 engine-brake evidence read failed"));
                return true;
            }
            if (!Evidence.bComplete)
            {
                if (PhaseSimSeconds < EngineBrakeEvidenceTimeoutSeconds)
                {
                    return false;
                }
                Test->AddError(FString::Printf(
                    TEXT("D4 engine-brake evidence did not complete sim_s=%.3f completed_s=%.6f target_s=%.6f steps=%d"),
                    PhaseSimSeconds,
                    Evidence.CompletedSampleSeconds,
                    Evidence.TargetSampleSeconds,
                    Evidence.CompletedSampleSteps));
                return true;
            }

            const float EndSpeed = HorizontalSpeedCmPerSec(*Mesh);
            Test->AddInfo(FString::Printf(
                TEXT("P02_D4_ENGINE_BRAKE sim_s=%.3f start_speed_cm_s=%.3f end_speed_cm_s=%.3f signed_rear_torque_nm=%.3f abs_rear_torque_nm=%.3f evidence_s=%.6f evidence_steps=%d"),
                PhaseSimSeconds,
                LiftStartSpeedCmPerSec,
                EndSpeed,
                Evidence.MeanSignedDrivenWheelTorqueNm,
                Evidence.MeanDrivenWheelTorqueNm,
                Evidence.CompletedSampleSeconds,
                Evidence.CompletedSampleSteps));
            Test->TestTrue(TEXT("D4 lift-off sends sustained negative rear torque"),
                Evidence.MeanSignedDrivenWheelTorqueNm
                    < -EngineBrakeTorqueThresholdNm);
            Test->TestTrue(TEXT("D4 lift-off physically reduces speed"),
                EndSpeed < LiftStartSpeedCmPerSec);

            Controls.SetBrake(1.0f);
            Controls.SetThrottle(0.0f);
            Controls.SetDriveline(1, 1, 1.0f);
            ResetPhaseClock(*PinkCabMovement);
            Phase = EPhase::Stopping;
            return false;
        }

        if (Phase == EPhase::Stopping)
        {
            const float Speed = HorizontalSpeedCmPerSec(*Mesh);
            if (Speed > 70.0f && PhaseSimSeconds < 4.0)
            {
                return false;
            }
            if (Speed > 70.0f)
            {
                Test->AddError(FString::Printf(
                    TEXT("D4 service brake did not reach stall window speed_cm_s=%.3f"),
                    Speed));
                return true;
            }

            Runtime.ForceGearState(1, 1, Cockpit);
            FPinkCabVehicleControlTickInput Input;
            Input.Frame = FPinkCabVehicleInputFrame::FromDigital(
                false, false, true, false);
            Input.DeltaSeconds = FMath::Max(
                PinkCabMovement->GetPinkCabLastMechanicalIntegrationDeltaSeconds(),
                1.0e-4f);
            const FPinkCabVehicleTelemetry Telemetry =
                BuildTelemetry(*Movement, *Mesh);
            FPinkCabVehicleControlOutput Output =
                Runtime.Update(Input, Telemetry, Cockpit, Health);

            FPinkCabVehicleControlState StallControls = Output.Controls;
            Test->TestTrue(
                TEXT("D4 stall state applies through authoritative bridge"),
                Apply(*Pawn, Cockpit, StallControls));
            Test->AddInfo(FString::Printf(
                TEXT("P02_D4_STALL speed_kmh=%.3f engine_rpm=%.3f ignition=%d"),
                Telemetry.SpeedKmh, Telemetry.EngineRpm,
                static_cast<int32>(Cockpit.GetIgnitionState())));
            Test->TestEqual(
                TEXT("D4 stop in gear with clutch engaged stalls engine"),
                Cockpit.GetIgnitionState(), EPinkCabIgnitionState::Stalled);
            Test->TestFalse(TEXT("D4 stalled engine denies combustion"),
                StallControls.IsCombustionAllowed());
            Test->TestTrue(TEXT("D4 stalled engine keeps mechanics alive"),
                Movement->bMechanicalSimEnabled);
            return true;
        }

        return true;
    }

private:
    enum class EPhase : uint8
    {
        Settling,
        Launching,
        Synchronizing,
        LiftOff,
        Stopping
    };

    void ResetPhaseClock(UPinkCabChaosVehicleMovementComponent& Movement)
    {
        LastMechanicalStep = Movement.GetPinkCabMechanicalIntegrationStepCount();
        PhaseSimSeconds = 0.0;
    }

    FAutomationTestBase* Test = nullptr;
    FPinkCabCockpitState Cockpit;
    FPinkCabVehicleControlState Controls;
    FPinkCabVehicleControlRuntime Runtime;
    FPinkCabVehicleHealthState Health;
    FPinkCabPhysicsFixtureRestGate RestGate;
    EPhase Phase = EPhase::Settling;
    bool bInitialized = false;
    int64 LastMechanicalStep = -1;
    double PhaseSimSeconds = 0.0;
    static constexpr double FullCouplingSyncSeconds = 0.50;
    static constexpr double EngineBrakeMeasurementSeconds = 1.00;
    static constexpr double EngineBrakeEvidenceTimeoutSeconds = 1.50;
    static constexpr float EngineBrakeTorqueThresholdNm = 1.0f;

    float LiftStartSpeedCmPerSec = 0.0f;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabD4EngineBrakeStallRuntimeTest,
    "PinkCab.Vehicle.Physics.P02.D4.EngineBrakingAndStopStall",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabD4EngineBrakeStallRuntimeTest::RunTest(const FString&)
{
    const bool bOpened = AutomationOpenMap(PinkCabPhysicsFixture::MapPath, true);
    TestTrue(TEXT("D4 engine-brake/stall map opens"), bOpened);
    if (!bOpened) return false;
    ADD_LATENT_AUTOMATION_COMMAND(FPinkCabD4EngineBrakeStallCommand(this));
    return true;
}

#endif
