#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Vehicle/PinkCabPhysicsFixturePawn.h"
#include "Vehicle/PinkCabChaosCockpitBridge.h"
#include "Vehicle/PinkCabChaosVehicleDynamicsProvider.h"
#include "Vehicle/PinkCabChaosVehicleMovementComponent.h"
#include "Vehicle/PinkCabCockpitState.h"
#include "Vehicle/PinkCabVehicleControlState.h"

namespace
{
struct FPinkCabD3CapacityRun
{
    float Capacity01 = 1.0f;
    float Coupling01 = 0.0f;
    int32 Gear = 0;
    int32 Repeat = 0;
    float RearTorqueNm = 0.0f;
    float EngineRpm = 0.0f;
    float ResetEngineRpm = 0.0f;
    float ResetMaxDrivenWheelRpm = 0.0f;
    float MeanDrivenWheelRpm = 0.0f;
    int32 SampleSteps = 0;
    int32 ChaosCurrentGear = 0;
    int32 ChaosTargetGear = 0;
};

float MedianCapacity(TArray<float> Values)
{
    Values.Sort();
    if (Values.IsEmpty())
    {
        return 0.0f;
    }
    const int32 Middle = Values.Num() / 2;
    return Values.Num() % 2 == 0
        ? 0.5f * (Values[Middle - 1] + Values[Middle])
        : Values[Middle];
}

float RelativeCapacityDelta(const float A, const float B, const float Floor)
{
    return FMath::Abs(A - B)
        / FMath::Max3(FMath::Abs(A), FMath::Abs(B), Floor);
}
}

class FPinkCabD3CapacityCommand final : public IAutomationLatentCommand
{
public:
    explicit FPinkCabD3CapacityCommand(FAutomationTestBase* InTest)
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

        AActor* Floor = PinkCabPhysicsFixture::FindOrSpawnFlatFloor(*World);
        APinkCabPhysicsFixturePawn* Pawn =
            PinkCabPhysicsFixture::FindOrSpawnPawn(*World);
        if (!Floor || !Pawn)
        {
            Test->AddError(TEXT("D3 capacity fixture failed to spawn"));
            return true;
        }
        PinkCabPhysicsFixture::KeepAwake(*Pawn);

        UChaosWheeledVehicleMovementComponent* Movement =
            Pawn->GetChaosMovement();
        UPinkCabChaosVehicleMovementComponent* PinkCabMovement =
            Cast<UPinkCabChaosVehicleMovementComponent>(Movement);
        if (!Movement || !PinkCabMovement)
        {
            Test->AddError(TEXT("D3 capacity fixture has no PinkCab movement"));
            return true;
        }

        if (!bInitialized)
        {
            UGameplayStatics::SetGamePaused(World, false);
            Test->TestTrue(TEXT("D3 capacity engine starts"), Cockpit.StartEngine());
            bInitialized = true;
            BeginFreshRun(*World);
            return false;
        }

        FPinkCabChaosVehicleDynamicsProvider Provider(Movement);
        if (!FPinkCabChaosCockpitBridge::Apply(Cockpit, *Movement, Controls, Provider))
        {
            Test->AddError(TEXT("D3 capacity authoritative actuation failed"));
            return true;
        }

        if (bResetting)
        {
            ++ResetPollCount;
            if (!RestGate.Update(*Pawn))
            {
                if (RestGate.GetObservation().ElapsedMechanicalSteps
                        < ResetTimeoutMechanicalSteps
                    && ResetPollCount < ResetPollLimit)
                {
                    return false;
                }
                Test->AddError(TEXT("D3 capacity physical reset did not settle"));
                return true;
            }

            const FPinkCabPhysicsFixtureRestObservation& Rest =
                RestGate.GetObservation();
            CurrentResetEngineRpm = Rest.EngineRpm;
            CurrentResetWheelRpm = Rest.MaxDrivenWheelRpm;

            Controls = {};
            Controls.SetThrottle(1.0f);
            Controls.SetBrake(0.0f);
            Controls.SetHandbrake(0.0f);
            Controls.SetDriveline(CurrentGear(), CurrentGear(), CurrentCoupling());
            Controls.SetDrivetrainTorqueCapacity(CurrentCapacity());

            FPinkCabChaosVehicleDynamicsProvider ConditionProvider(Movement);
            if (!FPinkCabChaosCockpitBridge::Apply(
                    Cockpit, *Movement, Controls, ConditionProvider))
            {
                Test->AddError(TEXT("D3 capacity measured condition failed"));
                return true;
            }
            if (!PinkCabMovement->BeginPinkCabMechanicalEvidenceWindow(
                    0.0f, SingleStepSampleSeconds))
            {
                Test->AddError(TEXT("D3 capacity evidence window failed"));
                return true;
            }

            bResetting = false;
            return false;
        }

        FPinkCabMechanicalEvidenceSnapshot Evidence;
        if (!PinkCabMovement->ReadPinkCabMechanicalEvidenceWindow(Evidence))
        {
            Test->AddError(TEXT("D3 capacity evidence read failed"));
            return true;
        }
        if (!Evidence.bComplete)
        {
            return false;
        }

        FPinkCabD3CapacityRun Run;
        Run.Capacity01 = CurrentCapacity();
        Run.Coupling01 = CurrentCoupling();
        Run.Gear = CurrentGear();
        Run.Repeat = RepeatIndex;
        Run.RearTorqueNm = Evidence.MeanDrivenWheelTorqueNm;
        Run.EngineRpm = Evidence.MeanEngineRpm;
        Run.ResetEngineRpm = CurrentResetEngineRpm;
        Run.ResetMaxDrivenWheelRpm = CurrentResetWheelRpm;
        Run.MeanDrivenWheelRpm = Evidence.MeanDrivenWheelRpm;
        Run.SampleSteps = Evidence.CompletedSampleSteps;
        Run.ChaosCurrentGear = Movement->GetCurrentGear();
        Run.ChaosTargetGear = Movement->GetTargetGear();
        Runs.Add(Run);

        Test->AddInfo(FString::Printf(
            TEXT("P02_D3_CAPACITY capacity=%.3f coupling=%.3f gear=%d repeat=%d torque_nm=%.3f engine_rpm=%.3f wheel_rpm=%.3f steps=%d"),
            Run.Capacity01, Run.Coupling01, Run.Gear, Run.Repeat + 1,
            Run.RearTorqueNm, Run.EngineRpm, Run.MeanDrivenWheelRpm, Run.SampleSteps));

        Advance();
        if (IsFinished())
        {
            return Evaluate();
        }
        BeginFreshRun(*World);
        return false;
    }

private:
    float CurrentCapacity() const { return Capacities[CapacityIndex]; }
    float CurrentCoupling() const { return Couplings[CouplingIndex]; }
    int32 CurrentGear() const { return Gears[GearIndex]; }
    bool IsFinished() const { return CapacityIndex >= Capacities.Num(); }

    void Advance()
    {
        if (++RepeatIndex < RepeatsPerCell) return;
        RepeatIndex = 0;
        if (++GearIndex < Gears.Num()) return;
        GearIndex = 0;
        if (++CouplingIndex < Couplings.Num()) return;
        CouplingIndex = 0;
        ++CapacityIndex;
    }

    void BeginFreshRun(UWorld& World)
    {
        PinkCabPhysicsFixture::DestroyPawns(World);
        Controls = {};
        Controls.SetBrake(1.0f);
        Controls.SetDriveline(0, 0, 0.0f);
        Controls.SetDrivetrainTorqueCapacity(1.0f);
        CurrentResetEngineRpm = 0.0f;
        CurrentResetWheelRpm = 0.0f;
        ResetPollCount = 0;
        RestGate.Reset();
        bResetting = true;
    }

    TArray<float> Values(
        const float Capacity,
        const float Coupling,
        const int32 Gear,
        const bool bRpm) const
    {
        TArray<float> Result;
        for (const FPinkCabD3CapacityRun& Run : Runs)
        {
            if (FMath::IsNearlyEqual(Run.Capacity01, Capacity, 1.0e-4f)
                && FMath::IsNearlyEqual(Run.Coupling01, Coupling, 1.0e-4f)
                && Run.Gear == Gear)
            {
                Result.Add(bRpm ? Run.EngineRpm : Run.RearTorqueNm);
            }
        }
        return Result;
    }

    bool Evaluate()
    {
        Test->TestEqual(
            TEXT("D3 capacity executes complete 40-run matrix"),
            Runs.Num(),
            Capacities.Num() * Couplings.Num() * Gears.Num() * RepeatsPerCell);

        for (const FPinkCabD3CapacityRun& Run : Runs)
        {
            Test->TestTrue(
                TEXT("D3 capacity every repeat starts at warm idle"),
                FMath::Abs(Run.ResetEngineRpm - InitialEngineRpm)
                    <= ResetEngineRpmTolerance);
            Test->TestTrue(
                TEXT("D3 capacity every repeat starts with stopped driven wheels"),
                Run.ResetMaxDrivenWheelRpm <= ResetWheelRpmTolerance);
            Test->TestEqual(
                TEXT("D3 capacity measures exactly one mechanical step"),
                Run.SampleSteps, 1);
            Test->TestTrue(
                TEXT("D3 capacity first-step shaft remains effectively stationary"),
                Run.MeanDrivenWheelRpm <= SampleWheelRpmTolerance);
            Test->TestEqual(TEXT("D3 capacity native Chaos current gear is neutral"),
                Run.ChaosCurrentGear, 0);
            Test->TestEqual(TEXT("D3 capacity native Chaos target gear is neutral"),
                Run.ChaosTargetGear, 0);
        }

        for (const float Capacity : Capacities)
        {
            for (const int32 Gear : Gears)
            {
                const float NearTorque = MedianCapacity(
                    Values(Capacity, 0.999f, Gear, false));
                const float FullTorque = MedianCapacity(
                    Values(Capacity, 1.0f, Gear, false));
                const float NearRpm = MedianCapacity(
                    Values(Capacity, 0.999f, Gear, true));
                const float FullRpm = MedianCapacity(
                    Values(Capacity, 1.0f, Gear, true));
                Test->TestTrue(
                    TEXT("D3 hot/worn torque is continuous at full clutch"),
                    RelativeCapacityDelta(NearTorque, FullTorque, 25.0f)
                        <= FullBoundaryRelativeTolerance);
                Test->TestTrue(
                    TEXT("D3 hot/worn engine RPM is continuous at full clutch"),
                    RelativeCapacityDelta(NearRpm, FullRpm, 250.0f)
                        <= FullBoundaryRelativeTolerance);
            }
        }

        for (const int32 Gear : Gears)
        {
            const float Healthy = MedianCapacity(Values(1.0f, 1.0f, Gear, false));
            const float Worn = MedianCapacity(Values(0.5f, 1.0f, Gear, false));
            Test->TestTrue(
                TEXT("D3 reduced clutch capacity limits transmitted torque"),
                Worn + CapacityEffectFloorNm < Healthy);
        }
        return true;
    }

    FAutomationTestBase* Test = nullptr;
    FPinkCabCockpitState Cockpit;
    FPinkCabVehicleControlState Controls;
    FPinkCabPhysicsFixtureRestGate RestGate;
    TArray<FPinkCabD3CapacityRun> Runs;
    const TArray<float> Capacities{1.0f, 0.5f};
    const TArray<float> Couplings{0.999f, 1.0f};
    const TArray<int32> Gears{1, -1};

    static constexpr int32 RepeatsPerCell = 5;
    static constexpr float InitialEngineRpm = 925.0f;
    static constexpr float SingleStepSampleSeconds = 1.0e-4f;
    static constexpr float ResetEngineRpmTolerance = 30.0f;
    static constexpr float ResetWheelRpmTolerance = 2.0f;
    static constexpr float SampleWheelRpmTolerance = 2.0f;
    static constexpr float FullBoundaryRelativeTolerance = 0.05f;
    static constexpr float CapacityEffectFloorNm = 25.0f;
    static constexpr int64 ResetTimeoutMechanicalSteps = 240;
    static constexpr int32 ResetPollLimit = 2400;

    int32 CapacityIndex = 0;
    int32 CouplingIndex = 0;
    int32 GearIndex = 0;
    int32 RepeatIndex = 0;
    int32 ResetPollCount = 0;
    bool bInitialized = false;
    bool bResetting = false;
    float CurrentResetEngineRpm = 0.0f;
    float CurrentResetWheelRpm = 0.0f;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabD3CapacityRuntimeTest,
    "PinkCab.Vehicle.Physics.P02.D3.HotWornCapacity",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabD3CapacityRuntimeTest::RunTest(const FString&)
{
    const bool bOpened = AutomationOpenMap(PinkCabPhysicsFixture::MapPath, true);
    TestTrue(TEXT("D3 capacity map opens"), bOpened);
    if (!bOpened)
    {
        return false;
    }
    ADD_LATENT_AUTOMATION_COMMAND(FPinkCabD3CapacityCommand(this));
    return true;
}

#endif
