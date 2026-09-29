#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "EngineUtils.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "ChaosVehicleWheel.h"
#include "ChaosVehicleManagerAsyncCallback.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Vehicle/PinkCabPhysicsFixturePawn.h"
#include "SnapshotData.h"
#include "Vehicle/PinkCabChaosCockpitBridge.h"
#include "Vehicle/PinkCabChaosVehicleDynamicsProvider.h"
#include "Vehicle/PinkCabChaosVehicleMovementComponent.h"
#include "Vehicle/PinkCabCockpitState.h"
#include "Vehicle/PinkCabVehicleControlState.h"

namespace
{
struct FPinkCabClutchBoundaryRun
{
    float Coupling = 0.0f;
    float MeanRearDriveTorqueNm = 0.0f;
    float MeanEngineRpm = 0.0f;
    float MeanChaosEngineTorqueNm = 0.0f;
    float MeanChaosTransmissionTorqueNm = 0.0f;
    float MeanChaosTransmissionRpm = 0.0f;
    float ResolvedEngineThrottle01 = 0.0f;
    float AuthoritativeAvailableEngineTorqueNm = 0.0f;
    double EndTranslationalKineticEnergyJ = 0.0;
    float EndSpeedCmPerSec = 0.0f;
    float MeasurementSeconds = 0.0f;
    int32 MeasurementSteps = 0;
};

float Median(TArray<float> Values)
{
    Values.Sort();
    if (Values.Num() == 0)
    {
        return 0.0f;
    }
    const int32 Middle = Values.Num() / 2;
    return Values.Num() % 2 == 0
        ? 0.5f * (Values[Middle - 1] + Values[Middle])
        : Values[Middle];
}

double MedianDouble(TArray<double> Values)
{
    Values.Sort();
    if (Values.Num() == 0)
    {
        return 0.0;
    }
    const int32 Middle = Values.Num() / 2;
    return Values.Num() % 2 == 0
        ? 0.5 * (Values[Middle - 1] + Values[Middle])
        : Values[Middle];
}

float RelativeDelta(const double A, const double B, const double Floor)
{
    const double Denominator = FMath::Max3(FMath::Abs(A), FMath::Abs(B), Floor);
    return static_cast<float>(FMath::Abs(A - B) / Denominator);
}
}

class FPinkCabClutchBoundaryContinuityCommand final : public IAutomationLatentCommand
{
public:
    explicit FPinkCabClutchBoundaryContinuityCommand(FAutomationTestBase* InTest)
        : Test(InTest)
    {
    }

    virtual bool Update() override
    {
        UWorld* World = AutomationCommon::GetAnyGameWorld();
        if (!World)
        {
            return false;
        }

        AActor* FixtureFloor =
            PinkCabPhysicsFixture::FindOrSpawnFlatFloor(*World);
        APinkCabPhysicsFixturePawn* Pawn =
            PinkCabPhysicsFixture::FindOrSpawnPawn(*World);
        if (!FixtureFloor || !Pawn)
        {
            Test->AddError(TEXT("sterile physics fixture failed to spawn"));
            return true;
        }
        PinkCabPhysicsFixture::KeepAwake(*Pawn);

        UChaosWheeledVehicleMovementComponent* Movement =
            Pawn->GetChaosMovement();
        UPinkCabChaosVehicleMovementComponent* PinkCabMovement =
            Cast<UPinkCabChaosVehicleMovementComponent>(Movement);
        USkeletalMeshComponent* Mesh = Pawn->GetMesh();
        Test->TestNotNull(TEXT("P02 live Chaos movement exists"), Movement);
        Test->TestNotNull(TEXT("P02 live PinkCab movement exists"), PinkCabMovement);
        Test->TestNotNull(TEXT("P02 live vehicle mesh exists"), Mesh);
        if (!Movement || !PinkCabMovement || !Mesh)
        {
            return true;
        }

        if (!bInitialized)
        {
            UGameplayStatics::SetGamePaused(World, false);
            Test->TestTrue(
                TEXT("P02 fixture starts local engine"),
                Cockpit.StartEngine());
            bInitialized = true;
            BeginFreshRun(*World);
            return false;
        }

        FPinkCabChaosVehicleDynamicsProvider Provider(Movement);
        if (!FPinkCabChaosCockpitBridge::Apply(
                Cockpit, *Movement, Controls, Provider))
        {
            Test->AddError(TEXT("P02 clutch boundary actuation refresh failed"));
            return true;
        }

        if (!bMeasuring)
        {
            ++RestPollCount;
            if (!RestGate.Update(*Pawn))
            {
                const auto& Rest = RestGate.GetObservation();
                if (Rest.ElapsedMechanicalSteps < RestTimeoutMechanicalSteps
                    && RestPollCount < RestPollLimit)
                {
                    return false;
                }
                Test->AddError(FString::Printf(
                    TEXT("P02 boundary fresh fixture did not settle engine_rpm=%.3f target_idle_rpm=%.3f wheel_rpm=%.3f body_linear_cm_s=%.3f body_angular_deg_s=%.3f mechanical_steps=%lld stable_steps=%d"),
                    Rest.EngineRpm,
                    Rest.TargetIdleRpm,
                    Rest.MaxDrivenWheelRpm,
                    Rest.BodyLinearSpeedCmPerSec,
                    Rest.BodyAngularSpeedDegPerSec,
                    static_cast<long long>(Rest.ElapsedMechanicalSteps),
                    RestGate.GetStableMechanicalSteps()));
                return true;
            }

            FWheeledSnaphotData Seed = Movement->GetSnapshot();
            Seed.LinearVelocity = FVector::ZeroVector;
            Seed.AngularVelocity = FVector::ZeroVector;
            Seed.EngineRPM = InitialEngineRpm;
            Seed.SelectedGear = 0;
            for (FWheelSnapshot& Wheel : Seed.WheelSnapshots)
            {
                Wheel.WheelAngularVelocity = 0.0f;
            }
            Movement->SetSnapshot(Seed);
            Mesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
            Mesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
            Mesh->WakeAllRigidBodies();

            Controls = {};
            Controls.SetThrottle(0.50f);
            Controls.SetBrake(0.0f);
            Controls.SetHandbrake(0.0f);
            Controls.SetDriveline(1, 1, CurrentCoupling());
            Controls.SetDrivetrainTorqueCapacity(1.0f);

            FPinkCabChaosVehicleDynamicsProvider ConditionProvider(Movement);
            if (!FPinkCabChaosCockpitBridge::Apply(
                    Cockpit, *Movement, Controls, ConditionProvider))
            {
                Test->AddError(TEXT("P02 clutch boundary initial actuation failed"));
                return true;
            }

            LastMechanicalStep =
                PinkCabMovement->GetPinkCabMechanicalIntegrationStepCount();
            MeasurementSettleSecondsRemaining = MeasurementSettleSeconds;
            ResetMeasurementSums();
            bMeasuring = true;
            return false;
        }

        const int64 MechanicalStep =
            PinkCabMovement->GetPinkCabMechanicalIntegrationStepCount();
        if (MechanicalStep == LastMechanicalStep)
        {
            return false;
        }
        const int64 Delta = MechanicalStep - LastMechanicalStep;
        if (Delta != 1)
        {
            Test->AddError(FString::Printf(
                TEXT("P02 boundary lost mechanical-step alignment coupling=%.3f repeat=%d previous=%lld current=%lld delta=%lld"),
                CurrentCoupling(),
                RepeatIndex + 1,
                static_cast<long long>(LastMechanicalStep),
                static_cast<long long>(MechanicalStep),
                static_cast<long long>(Delta)));
            return true;
        }
        LastMechanicalStep = MechanicalStep;

        const float MechanicalDeltaSeconds =
            PinkCabMovement->GetPinkCabLastMechanicalIntegrationDeltaSeconds();
        if (!FMath::IsFinite(MechanicalDeltaSeconds)
            || MechanicalDeltaSeconds <= KINDA_SMALL_NUMBER)
        {
            Test->AddError(TEXT("P02 boundary observed invalid mechanical dt"));
            return true;
        }

        if (MeasurementSettleSecondsRemaining > 0.0)
        {
            MeasurementSettleSecondsRemaining = FMath::Max(
                0.0,
                MeasurementSettleSecondsRemaining
                    - static_cast<double>(MechanicalDeltaSeconds));
            if (MeasurementSettleSecondsRemaining <= 0.0)
            {
                PreviousTranslationalKineticEnergyJ =
                    TranslationalKineticEnergyJ(*Movement, *Mesh);
                PreviousSpeedCmPerSec =
                    Mesh->GetPhysicsLinearVelocity().Size2D();
            }
            return false;
        }

        const double RemainingSeconds =
            MeasurementSampleSeconds - MeasurementElapsedSeconds;
        const double UsedSeconds = FMath::Min(
            static_cast<double>(MechanicalDeltaSeconds),
            RemainingSeconds);
        if (UsedSeconds <= 0.0)
        {
            Test->AddError(TEXT("P02 boundary exhausted measurement window"));
            return true;
        }

        const FWheelStatus RearLeft = Movement->GetWheelState(2);
        const FWheelStatus RearRight = Movement->GetWheelState(3);
        const float RearTorqueNm = 0.5f
            * (FMath::Abs(RearLeft.DriveTorque)
                + FMath::Abs(RearRight.DriveTorque));
        RearTorqueTimeIntegral +=
            static_cast<double>(RearTorqueNm) * UsedSeconds;
        EngineRpmTimeIntegral +=
            static_cast<double>(Movement->GetEngineRotationSpeed())
                * UsedSeconds;
        ResolvedThrottleTimeIntegral +=
            static_cast<double>(Controls.GetResolvedEngineThrottle01())
                * UsedSeconds;
        AvailableTorqueTimeIntegral +=
            static_cast<double>(Controls.GetAvailableEngineTorqueNm())
                * UsedSeconds;

        if (const TUniquePtr<FPhysicsVehicleOutput>& PhysicsOutput =
                Movement->PhysicsVehicleOutput())
        {
            if (PhysicsOutput.IsValid())
            {
                ChaosEngineTorqueTimeIntegral +=
                    static_cast<double>(PhysicsOutput->EngineTorque)
                        * UsedSeconds;
                ChaosTransmissionTorqueTimeIntegral +=
                    static_cast<double>(PhysicsOutput->TransmissionTorque)
                        * UsedSeconds;
                ChaosTransmissionRpmTimeIntegral +=
                    static_cast<double>(PhysicsOutput->TransmissionRPM)
                        * UsedSeconds;
            }
        }

        ++SampleCount;
        const double CurrentEnergyJ =
            TranslationalKineticEnergyJ(*Movement, *Mesh);
        const float CurrentSpeedCmPerSec =
            Mesh->GetPhysicsLinearVelocity().Size2D();

        if (MeasurementElapsedSeconds
                + static_cast<double>(MechanicalDeltaSeconds)
            < MeasurementSampleSeconds)
        {
            MeasurementElapsedSeconds +=
                static_cast<double>(MechanicalDeltaSeconds);
            PreviousTranslationalKineticEnergyJ = CurrentEnergyJ;
            PreviousSpeedCmPerSec = CurrentSpeedCmPerSec;
            return false;
        }

        const double Alpha = FMath::Clamp(
            RemainingSeconds
                / static_cast<double>(MechanicalDeltaSeconds),
            0.0,
            1.0);
        TargetWindowTranslationalKineticEnergyJ =
            FMath::Lerp(
                PreviousTranslationalKineticEnergyJ,
                CurrentEnergyJ,
                Alpha);
        TargetWindowSpeedCmPerSec = FMath::Lerp(
            PreviousSpeedCmPerSec,
            CurrentSpeedCmPerSec,
            static_cast<float>(Alpha));
        MeasurementElapsedSeconds = MeasurementSampleSeconds;

        FPinkCabClutchBoundaryRun Result;
        Result.Coupling = CurrentCoupling();
        Result.MeanRearDriveTorqueNm = static_cast<float>(
            RearTorqueTimeIntegral / MeasurementElapsedSeconds);
        Result.MeanEngineRpm = static_cast<float>(
            EngineRpmTimeIntegral / MeasurementElapsedSeconds);
        Result.MeanChaosEngineTorqueNm = static_cast<float>(
            ChaosEngineTorqueTimeIntegral / MeasurementElapsedSeconds);
        Result.MeanChaosTransmissionTorqueNm = static_cast<float>(
            ChaosTransmissionTorqueTimeIntegral / MeasurementElapsedSeconds);
        Result.MeanChaosTransmissionRpm = static_cast<float>(
            ChaosTransmissionRpmTimeIntegral / MeasurementElapsedSeconds);
        Result.ResolvedEngineThrottle01 = static_cast<float>(
            ResolvedThrottleTimeIntegral / MeasurementElapsedSeconds);
        Result.AuthoritativeAvailableEngineTorqueNm = static_cast<float>(
            AvailableTorqueTimeIntegral / MeasurementElapsedSeconds);
        Result.EndSpeedCmPerSec = TargetWindowSpeedCmPerSec;
        Result.EndTranslationalKineticEnergyJ =
            TargetWindowTranslationalKineticEnergyJ;
        Result.MeasurementSeconds =
            static_cast<float>(MeasurementElapsedSeconds);
        Result.MeasurementSteps = SampleCount;
        Runs.Add(Result);

        Test->AddInfo(FString::Printf(
            TEXT("P02_PHY009_BOUNDARY coupling=%.3f repeat=%d mean_rear_drive_torque_nm=%.3f mean_engine_rpm=%.3f mean_resolved_throttle=%.6f mean_authoritative_available_engine_torque_nm=%.3f chaos_engine_torque_nm=%.3f chaos_transmission_torque_nm=%.3f chaos_transmission_rpm=%.3f end_speed_cm_s=%.3f end_ke_j=%.3f measurement_s=%.6f measurement_steps=%d"),
            Result.Coupling,
            RepeatIndex + 1,
            Result.MeanRearDriveTorqueNm,
            Result.MeanEngineRpm,
            Result.ResolvedEngineThrottle01,
            Result.AuthoritativeAvailableEngineTorqueNm,
            Result.MeanChaosEngineTorqueNm,
            Result.MeanChaosTransmissionTorqueNm,
            Result.MeanChaosTransmissionRpm,
            Result.EndSpeedCmPerSec,
            Result.EndTranslationalKineticEnergyJ,
            Result.MeasurementSeconds,
            Result.MeasurementSteps));

        ++RepeatIndex;
        if (RepeatIndex >= RepeatsPerCondition)
        {
            RepeatIndex = 0;
            ++ConditionIndex;
        }

        if (ConditionIndex < Couplings.Num())
        {
            BeginFreshRun(*World);
            return false;
        }

        return EvaluateBoundary();
    }

private:
    float CurrentCoupling() const
    {
        return Couplings[FMath::Clamp(
            ConditionIndex, 0, Couplings.Num() - 1)];
    }

    void BeginFreshRun(UWorld& World)
    {
        PinkCabPhysicsFixture::DestroyPawns(World);
        Controls = {};
        Controls.SetThrottle(0.0f);
        // Use the real service brake only while establishing the sterile
        // stationary initial condition. Every measured condition explicitly
        // releases it before the next mechanical integration.
        Controls.SetBrake(1.0f);
        Controls.SetHandbrake(0.0f);
        Controls.SetDriveline(0, 0, 0.0f);
        Controls.SetDrivetrainTorqueCapacity(1.0f);
        RestGate.Reset();
        RestPollCount = 0;
        LastMechanicalStep = -1;
        MeasurementSettleSecondsRemaining = 0.0;
        ResetMeasurementSums();
        bMeasuring = false;
    }

    static double TranslationalKineticEnergyJ(
        const UChaosWheeledVehicleMovementComponent& Movement,
        const USkeletalMeshComponent& Mesh)
    {
        const double SpeedMps =
            static_cast<double>(Mesh.GetPhysicsLinearVelocity().Size2D())
                * 0.01;
        return 0.5
            * static_cast<double>(Movement.Mass)
            * SpeedMps
            * SpeedMps;
    }

    void ResetMeasurementSums()
    {
        RearTorqueTimeIntegral = 0.0;
        EngineRpmTimeIntegral = 0.0;
        ChaosEngineTorqueTimeIntegral = 0.0;
        ChaosTransmissionTorqueTimeIntegral = 0.0;
        ChaosTransmissionRpmTimeIntegral = 0.0;
        ResolvedThrottleTimeIntegral = 0.0;
        AvailableTorqueTimeIntegral = 0.0;
        MeasurementElapsedSeconds = 0.0;
        PreviousTranslationalKineticEnergyJ = 0.0;
        PreviousSpeedCmPerSec = 0.0f;
        TargetWindowTranslationalKineticEnergyJ = 0.0;
        TargetWindowSpeedCmPerSec = 0.0f;
        SampleCount = 0;
    }

    bool EvaluateBoundary()
    {
        TArray<float> PartialTorque;
        TArray<float> FullTorque;
        TArray<float> PartialRpm;
        TArray<float> FullRpm;
        TArray<double> PartialEnergy;
        TArray<double> FullEnergy;

        for (const FPinkCabClutchBoundaryRun& Run : Runs)
        {
            Test->TestTrue(
                TEXT("P02 boundary evidence retains combustion authority"),
                Run.ResolvedEngineThrottle01 > 0.10f);
            Test->TestTrue(
                TEXT("P02 boundary evidence retains available engine torque"),
                Run.AuthoritativeAvailableEngineTorqueNm > 25.0f);

            TArray<float>& Torque =
                Run.Coupling < 1.0f ? PartialTorque : FullTorque;
            TArray<float>& Rpm =
                Run.Coupling < 1.0f ? PartialRpm : FullRpm;
            TArray<double>& Energy =
                Run.Coupling < 1.0f ? PartialEnergy : FullEnergy;
            Torque.Add(Run.MeanRearDriveTorqueNm);
            Rpm.Add(Run.MeanEngineRpm);
            Energy.Add(Run.EndTranslationalKineticEnergyJ);
        }

        const float PartialTorqueMedian = Median(PartialTorque);
        const float FullTorqueMedian = Median(FullTorque);
        const float PartialRpmMedian = Median(PartialRpm);
        const float FullRpmMedian = Median(FullRpm);
        const double PartialEnergyMedian = MedianDouble(PartialEnergy);
        const double FullEnergyMedian = MedianDouble(FullEnergy);

        const float TorqueDelta = RelativeDelta(
            PartialTorqueMedian, FullTorqueMedian, 25.0);
        const float RpmDelta = RelativeDelta(
            PartialRpmMedian, FullRpmMedian, 250.0);
        const float EnergyDelta = RelativeDelta(
            PartialEnergyMedian, FullEnergyMedian, 250.0);

        Test->AddInfo(FString::Printf(
            TEXT("P02_PHY009_BOUNDARY_MEDIAN partial_torque=%.3f full_torque=%.3f torque_delta=%.6f partial_rpm=%.3f full_rpm=%.3f rpm_delta=%.6f partial_ke=%.3f full_ke=%.3f ke_delta=%.6f"),
            PartialTorqueMedian,
            FullTorqueMedian,
            TorqueDelta,
            PartialRpmMedian,
            FullRpmMedian,
            RpmDelta,
            PartialEnergyMedian,
            FullEnergyMedian,
            EnergyDelta));

        Test->TestEqual(TEXT("P02 has five 0.999 repeats"),
            PartialTorque.Num(), RepeatsPerCondition);
        Test->TestEqual(TEXT("P02 has five 1.000 repeats"),
            FullTorque.Num(), RepeatsPerCondition);
        Test->TestTrue(TEXT("P02 0.999 path produces measurable rear torque"),
            PartialTorqueMedian > 25.0f);
        Test->TestTrue(TEXT("P02 1.000 path produces measurable rear torque"),
            FullTorqueMedian > 25.0f);
        Test->TestTrue(
            TEXT("PHY-009 rear-wheel torque is continuous across 0.999 to 1.000"),
            TorqueDelta <= MaxBoundaryStep);
        Test->TestTrue(
            TEXT("PHY-009 engine RPM response is continuous across 0.999 to 1.000"),
            RpmDelta <= MaxBoundaryStep);
        Test->TestTrue(
            TEXT("PHY-009 translational energy is continuous across 0.999 to 1.000"),
            EnergyDelta <= MaxBoundaryStep);
        return true;
    }

    FAutomationTestBase* Test = nullptr;
    FPinkCabCockpitState Cockpit;
    FPinkCabVehicleControlState Controls;
    FPinkCabPhysicsFixtureRestGate RestGate;
    TArray<FPinkCabClutchBoundaryRun> Runs;

    const TArray<float> Couplings{0.999f, 1.0f};
    static constexpr int32 RepeatsPerCondition = 5;
    static constexpr float InitialEngineRpm = 2500.0f;
    static constexpr double MeasurementSettleSeconds = 0.05;
    static constexpr double MeasurementSampleSeconds = 0.50;
    static constexpr int64 RestTimeoutMechanicalSteps = 240;
    static constexpr int32 RestPollLimit = 2400;
    static constexpr float MaxBoundaryStep = 0.10f;

    bool bInitialized = false;
    bool bMeasuring = false;
    int32 ConditionIndex = 0;
    int32 RepeatIndex = 0;
    int32 RestPollCount = 0;
    int32 SampleCount = 0;
    double MeasurementSettleSecondsRemaining = 0.0;
    double MeasurementElapsedSeconds = 0.0;
    int64 LastMechanicalStep = -1;
    double RearTorqueTimeIntegral = 0.0;
    double EngineRpmTimeIntegral = 0.0;
    double ChaosEngineTorqueTimeIntegral = 0.0;
    double ChaosTransmissionTorqueTimeIntegral = 0.0;
    double ChaosTransmissionRpmTimeIntegral = 0.0;
    double ResolvedThrottleTimeIntegral = 0.0;
    double AvailableTorqueTimeIntegral = 0.0;
    double PreviousTranslationalKineticEnergyJ = 0.0;
    float PreviousSpeedCmPerSec = 0.0f;
    double TargetWindowTranslationalKineticEnergyJ = 0.0;
    float TargetWindowSpeedCmPerSec = 0.0f;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabClutchBoundaryContinuityRuntimeTest,
    "PinkCab.Vehicle.Physics.P02.ClutchBoundaryContinuity",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabClutchBoundaryContinuityRuntimeTest::RunTest(const FString& Parameters)
{
    const bool bOpened = AutomationOpenMap(
        PinkCabPhysicsFixture::MapPath,
        true);
    TestTrue(TEXT("P02 vehicle runtime map opens"), bOpened);
    if (!bOpened)
    {
        return false;
    }

    ADD_LATENT_AUTOMATION_COMMAND(
        FPinkCabClutchBoundaryContinuityCommand(this));
    return true;
}


struct FPinkCabPartialClutchReactionRun
{
    float Coupling01 = 0.0f;
    float SeedWheelDerivedEngineRpm = 0.0f;
    float MeanEngineRpm = 0.0f;
    float MeanDrivenWheelRpm = 0.0f;
    float MeanWheelDerivedEngineRpm = 0.0f;
};

class FPinkCabPartialClutchWheelReactionCommand final : public IAutomationLatentCommand
{
public:
    explicit FPinkCabPartialClutchWheelReactionCommand(
        FAutomationTestBase* InTest)
        : Test(InTest)
    {
    }

    virtual bool Update() override
    {
        UWorld* World = AutomationCommon::GetAnyGameWorld();
        if (!World)
        {
            return false;
        }

        AActor* FixtureFloor =
            PinkCabPhysicsFixture::FindOrSpawnFlatFloor(*World);
        APinkCabPhysicsFixturePawn* Pawn =
            PinkCabPhysicsFixture::FindOrSpawnPawn(*World);
        if (!FixtureFloor || !Pawn)
        {
            Test->AddError(TEXT("sterile physics fixture failed to spawn"));
            return true;
        }
        PinkCabPhysicsFixture::KeepAwake(*Pawn);

        UChaosWheeledVehicleMovementComponent* Movement =
            Pawn->GetChaosMovement();
        UPinkCabChaosVehicleMovementComponent* PinkCabMovement =
            Cast<UPinkCabChaosVehicleMovementComponent>(Movement);
        USkeletalMeshComponent* Mesh = Pawn->GetMesh();
        Test->TestNotNull(TEXT("P02 reaction movement exists"), Movement);
        Test->TestNotNull(TEXT("P02 reaction PinkCab movement exists"), PinkCabMovement);
        Test->TestNotNull(TEXT("P02 reaction mesh exists"), Mesh);
        if (!Movement || !PinkCabMovement || !Mesh)
        {
            return true;
        }

        if (!bInitialized)
        {
            UGameplayStatics::SetGamePaused(World, false);
            Test->TestTrue(
                TEXT("P02 reaction fixture starts local engine"),
                Cockpit.StartEngine());
            bInitialized = true;
            BeginFreshRun(*World);
            return false;
        }

        FPinkCabChaosVehicleDynamicsProvider Provider(Movement);
        if (!FPinkCabChaosCockpitBridge::Apply(
                Cockpit, *Movement, Controls, Provider))
        {
            Test->AddError(
                TEXT("P02 partial-clutch reaction actuation refresh failed"));
            return true;
        }

        if (!bMeasuring)
        {
            ++RestPollCount;
            if (!RestGate.Update(*Pawn))
            {
                const auto& Rest = RestGate.GetObservation();
                if (Rest.ElapsedMechanicalSteps < RestTimeoutMechanicalSteps
                    && RestPollCount < RestPollLimit)
                {
                    return false;
                }
                Test->AddError(FString::Printf(
                    TEXT("P02 reaction fresh fixture did not settle coupling=%.3f repeat=%d engine_rpm=%.3f wheel_rpm=%.3f body_linear_cm_s=%.3f body_angular_deg_s=%.3f mechanical_steps=%lld stable_steps=%d"),
                    CurrentCoupling(),
                    RepeatIndex + 1,
                    Rest.EngineRpm,
                    Rest.MaxDrivenWheelRpm,
                    Rest.BodyLinearSpeedCmPerSec,
                    Rest.BodyAngularSpeedDegPerSec,
                    static_cast<long long>(Rest.ElapsedMechanicalSteps),
                    RestGate.GetStableMechanicalSteps()));
                return true;
            }

            FWheeledSnaphotData RunSnapshot = Movement->GetSnapshot();
            const float SpeedMps = HighSpeedKmh / 3.6f;
            const float SpeedCmPerSec = SpeedMps * 100.0f;
            RunSnapshot.LinearVelocity =
                Pawn->GetActorForwardVector() * SpeedCmPerSec;
            RunSnapshot.AngularVelocity = FVector::ZeroVector;
            RunSnapshot.EngineRPM = InitialEngineRpm;
            RunSnapshot.SelectedGear = 0;

            float SeedDrivenWheelRpmSum = 0.0f;
            int32 SeedDrivenWheelCount = 0;
            for (int32 WheelIndex = 0;
                 WheelIndex < RunSnapshot.WheelSnapshots.Num()
                    && WheelIndex < Movement->Wheels.Num();
                 ++WheelIndex)
            {
                const UChaosVehicleWheel* Wheel =
                    Movement->Wheels[WheelIndex];
                if (!Wheel)
                {
                    continue;
                }
                const float RadiusM =
                    FMath::Max(
                        Wheel->WheelRadius * 0.01f,
                        KINDA_SMALL_NUMBER);
                const float SeedAngularVelocityRadPerSec =
                    SpeedMps / RadiusM;
                RunSnapshot.WheelSnapshots[WheelIndex]
                    .WheelAngularVelocity =
                    SeedAngularVelocityRadPerSec;

                if (Wheel->bAffectedByEngine)
                {
                    SeedDrivenWheelRpmSum +=
                        FMath::Abs(SeedAngularVelocityRadPerSec)
                        * (60.0f / (2.0f * PI));
                    ++SeedDrivenWheelCount;
                }
            }

            const float SeedDrivenWheelRpm =
                SeedDrivenWheelCount > 0
                    ? SeedDrivenWheelRpmSum
                        / static_cast<float>(SeedDrivenWheelCount)
                    : 0.0f;
            const float EffectiveRatio =
                FMath::Abs(Movement->TransmissionSetup.GetGearRatio(1));
            CurrentSeedWheelDerivedEngineRpm =
                SeedDrivenWheelRpm * EffectiveRatio;

            Movement->SetSnapshot(RunSnapshot);
            Mesh->SetPhysicsLinearVelocity(
                Pawn->GetActorForwardVector() * SpeedCmPerSec);
            Mesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
            Mesh->WakeAllRigidBodies();

            Controls = {};
            Controls.SetThrottle(0.0f);
            Controls.SetBrake(0.0f);
            Controls.SetHandbrake(0.0f);
            Controls.SetDriveline(1, 1, CurrentCoupling());
            Controls.SetDrivetrainTorqueCapacity(1.0f);

            FPinkCabChaosVehicleDynamicsProvider ConditionProvider(Movement);
            if (!FPinkCabChaosCockpitBridge::Apply(
                    Cockpit, *Movement, Controls, ConditionProvider))
            {
                Test->AddError(
                    TEXT("P02 partial-clutch reaction initial actuation failed"));
                return true;
            }

            LastMechanicalStep =
                PinkCabMovement->GetPinkCabMechanicalIntegrationStepCount();
            MeasurementSettleStepsRemaining =
                MeasurementSettleMechanicalSteps;
            EngineRpmSum = 0.0f;
            DrivenWheelRpmSumSamples = 0.0f;
            WheelDerivedEngineRpmSum = 0.0f;
            SampleCount = 0;
            bMeasuring = true;
            return false;
        }

        const int64 MechanicalStep =
            PinkCabMovement->GetPinkCabMechanicalIntegrationStepCount();
        if (MechanicalStep == LastMechanicalStep)
        {
            return false;
        }
        const int64 Delta = MechanicalStep - LastMechanicalStep;
        if (Delta != 1)
        {
            Test->AddError(FString::Printf(
                TEXT("P02 reaction lost mechanical-step alignment coupling=%.3f repeat=%d previous=%lld current=%lld delta=%lld"),
                CurrentCoupling(),
                RepeatIndex + 1,
                static_cast<long long>(LastMechanicalStep),
                static_cast<long long>(MechanicalStep),
                static_cast<long long>(Delta)));
            return true;
        }
        LastMechanicalStep = MechanicalStep;

        if (MeasurementSettleStepsRemaining > 0)
        {
            --MeasurementSettleStepsRemaining;
            return false;
        }

        float DrivenWheelRpmSum = 0.0f;
        int32 DrivenWheelCount = 0;
        for (int32 WheelIndex = 0;
             WheelIndex < Movement->Wheels.Num();
             ++WheelIndex)
        {
            const UChaosVehicleWheel* Wheel =
                Movement->Wheels[WheelIndex];
            if (!Wheel || !Wheel->bAffectedByEngine)
            {
                continue;
            }
            DrivenWheelRpmSum += FMath::Abs(
                Wheel->GetWheelAngularVelocity()
                    * (60.0f / (2.0f * PI)));
            ++DrivenWheelCount;
        }

        if (DrivenWheelCount > 0)
        {
            const float MeanWheelRpm =
                DrivenWheelRpmSum
                / static_cast<float>(DrivenWheelCount);
            DrivenWheelRpmSumSamples += MeanWheelRpm;
            const float EffectiveRatio =
                FMath::Abs(
                    Movement->TransmissionSetup.GetGearRatio(1));
            WheelDerivedEngineRpmSum +=
                MeanWheelRpm * EffectiveRatio;
        }
        EngineRpmSum += Movement->GetEngineRotationSpeed();
        ++SampleCount;

        if (SampleCount < MeasurementSampleMechanicalSteps)
        {
            return false;
        }

        FPinkCabPartialClutchReactionRun Result;
        Result.Coupling01 = CurrentCoupling();
        Result.SeedWheelDerivedEngineRpm =
            CurrentSeedWheelDerivedEngineRpm;
        Result.MeanEngineRpm =
            EngineRpmSum / static_cast<float>(SampleCount);
        Result.MeanDrivenWheelRpm =
            DrivenWheelRpmSumSamples
                / static_cast<float>(SampleCount);
        Result.MeanWheelDerivedEngineRpm =
            WheelDerivedEngineRpmSum
                / static_cast<float>(SampleCount);
        Runs.Add(Result);

        Test->AddInfo(FString::Printf(
            TEXT("P02_PHY009_REACTION coupling=%.3f speed_kmh=%.1f repeat=%d seed_wheel_derived_engine_rpm=%.3f engine_rpm=%.3f driven_wheel_rpm=%.3f measured_wheel_derived_engine_rpm=%.3f"),
            Result.Coupling01,
            HighSpeedKmh,
            RepeatIndex + 1,
            Result.SeedWheelDerivedEngineRpm,
            Result.MeanEngineRpm,
            Result.MeanDrivenWheelRpm,
            Result.MeanWheelDerivedEngineRpm));

        ++RepeatIndex;
        if (RepeatIndex >= RepeatsPerCondition)
        {
            RepeatIndex = 0;
            ++ConditionIndex;
        }

        if (ConditionIndex < Couplings.Num())
        {
            BeginFreshRun(*World);
            return false;
        }

        return EvaluateReaction();
    }

private:
    float CurrentCoupling() const
    {
        return Couplings[FMath::Clamp(
            ConditionIndex, 0, Couplings.Num() - 1)];
    }

    void BeginFreshRun(UWorld& World)
    {
        PinkCabPhysicsFixture::DestroyPawns(World);
        Controls = {};
        Controls.SetThrottle(0.0f);
        // Use the real service brake only while establishing the sterile
        // stationary initial condition. Every measured condition explicitly
        // releases it before the next mechanical integration.
        Controls.SetBrake(1.0f);
        Controls.SetHandbrake(0.0f);
        Controls.SetDriveline(0, 0, 0.0f);
        Controls.SetDrivetrainTorqueCapacity(1.0f);
        RestGate.Reset();
        RestPollCount = 0;
        LastMechanicalStep = -1;
        MeasurementSettleStepsRemaining = 0;
        EngineRpmSum = 0.0f;
        DrivenWheelRpmSumSamples = 0.0f;
        WheelDerivedEngineRpmSum = 0.0f;
        SampleCount = 0;
        bMeasuring = false;
    }

    bool EvaluateReaction()
    {
        TArray<float> OpenEngineRpm;
        TArray<float> PartialEngineRpm;
        TArray<float> OpenSeedWheelDerivedRpm;
        TArray<float> PartialSeedWheelDerivedRpm;
        TArray<float> OpenMeasuredWheelDerivedRpm;
        TArray<float> PartialMeasuredWheelDerivedRpm;

        for (const FPinkCabPartialClutchReactionRun& Run : Runs)
        {
            const bool bOpen = FMath::IsNearlyZero(
                Run.Coupling01,
                KINDA_SMALL_NUMBER);
            (bOpen ? OpenEngineRpm : PartialEngineRpm).Add(
                Run.MeanEngineRpm);
            (bOpen
                ? OpenSeedWheelDerivedRpm
                : PartialSeedWheelDerivedRpm)
                .Add(Run.SeedWheelDerivedEngineRpm);
            (bOpen
                ? OpenMeasuredWheelDerivedRpm
                : PartialMeasuredWheelDerivedRpm)
                .Add(Run.MeanWheelDerivedEngineRpm);
        }

        const float OpenEngineMedian = Median(OpenEngineRpm);
        const float PartialEngineMedian = Median(PartialEngineRpm);
        const float OpenSeedWheelMedian =
            Median(OpenSeedWheelDerivedRpm);
        const float PartialSeedWheelMedian =
            Median(PartialSeedWheelDerivedRpm);
        const float OpenMeasuredWheelMedian =
            Median(OpenMeasuredWheelDerivedRpm);
        const float PartialMeasuredWheelMedian =
            Median(PartialMeasuredWheelDerivedRpm);
        const float MeanSeedWheelRpm =
            0.5f * (OpenSeedWheelMedian + PartialSeedWheelMedian);
        const float ShaftStimulusRpm =
            MeanSeedWheelRpm - InitialEngineRpm;
        const float EngineReactionRpm =
            PartialEngineMedian - OpenEngineMedian;
        const float ReactionFraction =
            ShaftStimulusRpm > KINDA_SMALL_NUMBER
                ? EngineReactionRpm / ShaftStimulusRpm
                : 0.0f;
        const float SeedMismatchRpm =
            FMath::Abs(
                OpenSeedWheelMedian - PartialSeedWheelMedian);

        Test->AddInfo(FString::Printf(
            TEXT("P02_PHY009_REACTION_MEDIAN open_engine_rpm=%.3f partial_engine_rpm=%.3f open_seed_shaft_rpm=%.3f partial_seed_shaft_rpm=%.3f open_measured_shaft_rpm=%.3f partial_measured_shaft_rpm=%.3f shaft_stimulus_rpm=%.3f engine_reaction_rpm=%.3f reaction_fraction=%.6f seed_mismatch_rpm=%.3f"),
            OpenEngineMedian,
            PartialEngineMedian,
            OpenSeedWheelMedian,
            PartialSeedWheelMedian,
            OpenMeasuredWheelMedian,
            PartialMeasuredWheelMedian,
            ShaftStimulusRpm,
            EngineReactionRpm,
            ReactionFraction,
            SeedMismatchRpm));

        Test->TestEqual(TEXT("P02 reaction has five open-clutch repeats"),
            OpenEngineRpm.Num(), RepeatsPerCondition);
        Test->TestEqual(TEXT("P02 reaction has five partial-clutch repeats"),
            PartialEngineRpm.Num(), RepeatsPerCondition);
        Test->TestTrue(
            TEXT("P02 reaction compares the same high-speed shaft stimulus"),
            SeedMismatchRpm <= MaxSeedMismatchRpm);
        Test->TestTrue(
            TEXT("P02 reaction fixture creates a meaningful shaft-speed stimulus"),
            ShaftStimulusRpm >= MinShaftStimulusRpm);
        Test->TestTrue(
            TEXT("PHY-009 partial clutch transmits wheel-to-engine RPM reaction"),
            EngineReactionRpm >= MinEngineReactionRpm
                && ReactionFraction >= MinReactionFraction);
        return true;
    }

    FAutomationTestBase* Test = nullptr;
    FPinkCabCockpitState Cockpit;
    FPinkCabVehicleControlState Controls;
    FPinkCabPhysicsFixtureRestGate RestGate;
    TArray<FPinkCabPartialClutchReactionRun> Runs;

    const TArray<float> Couplings{0.0f, PartialCoupling};
    static constexpr int32 RepeatsPerCondition = 5;
    static constexpr float HighSpeedKmh = 50.0f;
    static constexpr float InitialEngineRpm = 3000.0f;
    static constexpr float PartialCoupling = 0.50f;
    static constexpr int32 MeasurementSettleMechanicalSteps = 3;
    static constexpr int32 MeasurementSampleMechanicalSteps = 12;
    static constexpr int64 RestTimeoutMechanicalSteps = 240;
    static constexpr int32 RestPollLimit = 2400;
    static constexpr float MaxSeedMismatchRpm = 5.0f;
    static constexpr float MinShaftStimulusRpm = 1500.0f;
    static constexpr float MinEngineReactionRpm = 300.0f;
    static constexpr float MinReactionFraction = 0.10f;

    bool bInitialized = false;
    bool bMeasuring = false;
    int32 ConditionIndex = 0;
    int32 RepeatIndex = 0;
    int32 RestPollCount = 0;
    int32 SampleCount = 0;
    int32 MeasurementSettleStepsRemaining = 0;
    int64 LastMechanicalStep = -1;
    float EngineRpmSum = 0.0f;
    float DrivenWheelRpmSumSamples = 0.0f;
    float WheelDerivedEngineRpmSum = 0.0f;
    float CurrentSeedWheelDerivedEngineRpm = 0.0f;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPartialClutchWheelReactionRuntimeTest,
    "PinkCab.Vehicle.Physics.P02.PartialClutchWheelReaction",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabPartialClutchWheelReactionRuntimeTest::RunTest(const FString& Parameters)
{
    const bool bOpened = AutomationOpenMap(
        PinkCabPhysicsFixture::MapPath,
        true);
    TestTrue(TEXT("P02 partial reaction runtime map opens"), bOpened);
    if (!bOpened)
    {
        return false;
    }

    ADD_LATENT_AUTOMATION_COMMAND(
        FPinkCabPartialClutchWheelReactionCommand(this));
    return true;
}

#endif
