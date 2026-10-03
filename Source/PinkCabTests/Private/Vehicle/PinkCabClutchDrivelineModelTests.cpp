#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Vehicle/PinkCabClutchDrivelineModel.h"
#include "Vehicle/PinkCabGearEngagementValidator.h"
#include "Vehicle/PinkCabVehicleInputResponse.h"
#include "Vehicle/PinkCabVehicleControlRuntime.h"

namespace
{
FPinkCabClutchDrivelineConfig AdultClutchConfig()
{
    FPinkCabClutchDrivelineConfig Config;
    Config.EngineEffectiveInertia = 5.0f;
    Config.MaxClutchTorqueNm = 320.0f;
    Config.SynchronizationTimeSeconds = 0.20f;
    Config.LockedSlipRpm = 25.0f;
    return Config;
}

FPinkCabClutchDrivelineInput BaseInput()
{
    FPinkCabClutchDrivelineInput Input;
    Input.DeltaSeconds = 1.0f / 60.0f;
    Input.EngineRpm = 3000.0f;
    Input.ShaftEquivalentEngineRpm = 3000.0f;
    Input.AvailableEngineTorqueNm = 180.0f;
    Input.EngineDragTorqueNm = 0.0f;
    Input.ClutchCoupling01 = 1.0f;
    Input.DrivetrainTorqueCapacity01 = 1.0f;
    Input.EffectiveGearRatio = 14.72f;
    Input.TransmissionEfficiency = 0.90f;
    return Input;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabClutchDrivelineOpenTest,
    "PinkCab.Vehicle.Physics.P02.ClutchModel.OpenDisconnectsBothDirections",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabClutchDrivelineOpenTest::RunTest(const FString&)
{
    FPinkCabClutchDrivelineModel Model(AdultClutchConfig());
    FPinkCabClutchDrivelineInput Input = BaseInput();
    Input.ClutchCoupling01 = 0.0f;
    Input.ShaftEquivalentEngineRpm = 6000.0f;
    const FPinkCabClutchDrivelineOutput Out = Model.Step(Input);

    TestEqual(TEXT("open clutch transmits zero torque"), Out.TransmittedClutchTorqueNm, 0.0f);
    TestEqual(TEXT("open clutch applies zero engine reaction"), Out.EngineReactionDeltaRpm, 0.0f);
    TestEqual(TEXT("open clutch state is explicit"), Out.State, EPinkCabClutchDrivelineState::Open);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabClutchDrivelineSteadyDriveTest,
    "PinkCab.Vehicle.Physics.P02.ClutchModel.SteadyLockedDrive",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabClutchDrivelineSteadyDriveTest::RunTest(const FString&)
{
    FPinkCabClutchDrivelineModel Model(AdultClutchConfig());
    const FPinkCabClutchDrivelineOutput Out = Model.Step(BaseInput());

    TestEqual(TEXT("zero-slip clutch carries authoritative engine torque"),
        Out.TransmittedClutchTorqueNm, 180.0f);
    TestTrue(TEXT("rear axle receives ratio and efficiency"),
        FMath::IsNearlyEqual(Out.RearAxleTorqueNm, 180.0f * 14.72f * 0.90f, 0.01f));
    TestEqual(TEXT("steady zero-slip state is locked"),
        Out.State, EPinkCabClutchDrivelineState::Locked);
    TestTrue(TEXT("engine receives equal/opposite reaction"),
        Out.EngineReactionDeltaRpm < 0.0f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabClutchDrivelineBidirectionalReactionTest,
    "PinkCab.Vehicle.Physics.P02.ClutchModel.BidirectionalReaction",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabClutchDrivelineBidirectionalReactionTest::RunTest(const FString&)
{
    FPinkCabClutchDrivelineModel Model(AdultClutchConfig());

    FPinkCabClutchDrivelineInput EngineFast = BaseInput();
    EngineFast.AvailableEngineTorqueNm = 0.0f;
    EngineFast.EngineRpm = 4500.0f;
    EngineFast.ShaftEquivalentEngineRpm = 2500.0f;
    const auto EngineFastOut = Model.Step(EngineFast);
    TestTrue(TEXT("engine-fast slip sends positive torque to shaft"),
        EngineFastOut.TransmittedClutchTorqueNm > 0.0f);
    TestTrue(TEXT("engine-fast slip decelerates engine"),
        EngineFastOut.EngineReactionDeltaRpm < 0.0f);

    FPinkCabClutchDrivelineInput WheelsFast = BaseInput();
    WheelsFast.AvailableEngineTorqueNm = 0.0f;
    WheelsFast.EngineRpm = 2500.0f;
    WheelsFast.ShaftEquivalentEngineRpm = 4500.0f;
    const auto WheelsFastOut = Model.Step(WheelsFast);
    TestTrue(TEXT("wheel-fast slip reverses clutch torque"),
        WheelsFastOut.TransmittedClutchTorqueNm < 0.0f);
    TestTrue(TEXT("wheel-fast slip back-drives engine"),
        WheelsFastOut.EngineReactionDeltaRpm > 0.0f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabClutchDrivelineCapacityTest,
    "PinkCab.Vehicle.Physics.P02.ClutchModel.CapacityAndCondition",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabClutchDrivelineCapacityTest::RunTest(const FString&)
{
    FPinkCabClutchDrivelineModel Model(AdultClutchConfig());
    FPinkCabClutchDrivelineInput Input = BaseInput();
    Input.EngineRpm = 7000.0f;
    Input.ShaftEquivalentEngineRpm = 1000.0f;
    Input.ClutchCoupling01 = 0.50f;
    Input.DrivetrainTorqueCapacity01 = 0.50f;
    const auto Out = Model.Step(Input);

    TestEqual(TEXT("coupling and condition multiply physical torque capacity"),
        Out.TorqueCapacityNm, 80.0f);
    TestTrue(TEXT("large slip is capacity limited"), Out.bTorqueLimited);
    TestEqual(TEXT("transmitted torque respects capacity"),
        FMath::Abs(Out.TransmittedClutchTorqueNm), 80.0f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabClutchDrivelineContinuousBoundaryTest,
    "PinkCab.Vehicle.Physics.P02.ClutchModel.ContinuousAtFullCoupling",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabClutchDrivelineContinuousBoundaryTest::RunTest(const FString&)
{
    FPinkCabClutchDrivelineModel Model(AdultClutchConfig());
    FPinkCabClutchDrivelineInput Partial = BaseInput();
    Partial.EngineRpm = 3100.0f;
    Partial.ShaftEquivalentEngineRpm = 3000.0f;
    Partial.ClutchCoupling01 = 0.999f;
    const auto A = Model.Step(Partial);

    FPinkCabClutchDrivelineInput Full = Partial;
    Full.ClutchCoupling01 = 1.0f;
    const auto B = Model.Step(Full);

    const float TorqueStep = FMath::Abs(B.TransmittedClutchTorqueNm - A.TransmittedClutchTorqueNm);
    TestTrue(TEXT("0.999 to 1.000 uses same equation with no path switch"),
        TorqueStep <= AdultClutchConfig().MaxClutchTorqueNm * 0.0011f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabClutchDrivelineReverseTest,
    "PinkCab.Vehicle.Physics.P02.ClutchModel.ReversePreservesEngineReaction",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabClutchDrivelineReverseTest::RunTest(const FString&)
{
    FPinkCabClutchDrivelineModel Model(AdultClutchConfig());
    FPinkCabClutchDrivelineInput Input = BaseInput();
    Input.EffectiveGearRatio = -14.72f;
    const auto Out = Model.Step(Input);

    TestTrue(TEXT("reverse changes axle torque direction"),
        Out.RearAxleTorqueNm < 0.0f);
    TestTrue(TEXT("engine-side clutch torque remains positive in reverse"),
        Out.TransmittedClutchTorqueNm > 0.0f);
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabClutchDrivelineEngineBrakingTest,
    "PinkCab.Vehicle.Physics.P02.ClutchModel.EngineBrakingUsesClutch",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabClutchDrivelineEngineBrakingTest::RunTest(const FString&)
{
    FPinkCabClutchDrivelineModel Model(AdultClutchConfig());

    FPinkCabClutchDrivelineInput Open = BaseInput();
    Open.AvailableEngineTorqueNm = 0.0f;
    Open.EngineDragTorqueNm = 45.0f;
    Open.ClutchCoupling01 = 0.0f;
    const auto OpenOut = Model.Step(Open);
    TestEqual(TEXT("open clutch cannot send engine drag to wheels"),
        OpenOut.RearAxleTorqueNm, 0.0f);

    FPinkCabClutchDrivelineInput Closed = Open;
    Closed.ClutchCoupling01 = 1.0f;
    const auto ClosedOut = Model.Step(Closed);
    TestTrue(TEXT("closed clutch sends negative engine drag through driveline"),
        ClosedOut.TransmittedClutchTorqueNm < 0.0f);
    TestTrue(TEXT("engine braking produces negative forward axle torque"),
        ClosedOut.RearAxleTorqueNm < 0.0f);
    TestTrue(TEXT("wheel reaction offsets engine drag rather than hidden wheel brake"),
        ClosedOut.EngineReactionDeltaRpm > 0.0f);
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabClutchDrivelineSynchronizationHorizonTest,
    "PinkCab.Vehicle.Physics.P02.ClutchModel.SynchronizationHorizon",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabClutchDrivelineSynchronizationHorizonTest::RunTest(
    const FString&)
{
    FPinkCabClutchDrivelineConfig Config;
    Config.EngineEffectiveInertia = 0.17f;
    Config.MaxClutchTorqueNm = 390.0f;
    Config.SynchronizationTimeSeconds = 0.20f;
    Config.LockedSlipRpm = 25.0f;
    FPinkCabClutchDrivelineModel Model(Config);

    constexpr float ShaftRpm = 1700.0f;
    constexpr float InitialSlipRpm = 600.0f;
    constexpr float OuterDeltaSeconds = 1.0f / 60.0f;

    float EngineRpm = ShaftRpm + InitialSlipRpm;
    float ElapsedSeconds = 0.0f;
    bool bAnyTorqueLimited = false;

    while (ElapsedSeconds
        < Config.SynchronizationTimeSeconds - KINDA_SMALL_NUMBER)
    {
        const float DeltaSeconds = FMath::Min(
            OuterDeltaSeconds,
            Config.SynchronizationTimeSeconds - ElapsedSeconds);

        FPinkCabClutchDrivelineInput Input;
        Input.DeltaSeconds = DeltaSeconds;
        Input.EngineRpm = EngineRpm;
        Input.ShaftEquivalentEngineRpm = ShaftRpm;
        Input.AvailableEngineTorqueNm = 0.0f;
        Input.EngineDragTorqueNm = 0.0f;
        Input.ClutchCoupling01 = 1.0f;
        Input.DrivetrainTorqueCapacity01 = 1.0f;
        Input.EffectiveGearRatio = 14.72f;
        Input.TransmissionEfficiency = 0.90f;

        const FPinkCabClutchDrivelineOutput Out = Model.Step(Input);
        bAnyTorqueLimited |= Out.bTorqueLimited;
        EngineRpm += Out.EngineReactionDeltaRpm;
        ElapsedSeconds += DeltaSeconds;
    }

    const float FinalSlipRpm = FMath::Abs(EngineRpm - ShaftRpm);
    TestFalse(
        TEXT("synchronization-horizon fixture remains below clutch capacity"),
        bAnyTorqueLimited);
    TestTrue(
        *FString::Printf(
            TEXT("unconstrained full clutch reaches locked-slip band within authored synchronization horizon final_slip_rpm=%.3f locked_slip_rpm=%.3f"),
            FinalSlipRpm,
            Config.LockedSlipRpm),
        FinalSlipRpm <= Config.LockedSlipRpm);
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabClutchDrivelineCapacityIndependentComplianceTest,
    "PinkCab.Vehicle.Physics.P02.ClutchModel.CapacityIndependentCompliance",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabClutchDrivelineCapacityIndependentComplianceTest::RunTest(
    const FString&)
{
    FPinkCabClutchDrivelineConfig Config;
    Config.EngineEffectiveInertia = 0.17f;
    Config.MaxClutchTorqueNm = 390.0f;
    Config.SynchronizationTimeSeconds = 0.20f;
    Config.LockedSlipRpm = 25.0f;
    FPinkCabClutchDrivelineModel Model(Config);

    FPinkCabClutchDrivelineInput Full;
    Full.DeltaSeconds = 1.0f / 60.0f;
    Full.EngineRpm = 1800.0f;
    Full.ShaftEquivalentEngineRpm = 1700.0f;
    Full.AvailableEngineTorqueNm = 0.0f;
    Full.EngineDragTorqueNm = 0.0f;
    Full.ClutchCoupling01 = 1.0f;
    Full.DrivetrainTorqueCapacity01 = 1.0f;
    Full.EffectiveGearRatio = 14.72f;
    Full.TransmissionEfficiency = 0.90f;

    FPinkCabClutchDrivelineInput Partial = Full;
    Partial.ClutchCoupling01 = 0.75f;

    const FPinkCabClutchDrivelineOutput FullOut = Model.Step(Full);
    const FPinkCabClutchDrivelineOutput PartialOut = Model.Step(Partial);

    TestFalse(TEXT("full fixture is below torque capacity"),
        FullOut.bTorqueLimited);
    TestFalse(TEXT("partial fixture is below torque capacity"),
        PartialOut.bTorqueLimited);

    const float RelativeTorqueDelta =
        FMath::Abs(
            FullOut.TransmittedClutchTorqueNm
            - PartialOut.TransmittedClutchTorqueNm)
        / FMath::Max(
            FMath::Abs(FullOut.TransmittedClutchTorqueNm),
            1.0f);
    TestTrue(
        *FString::Printf(
            TEXT("clutch compliance is independent of unused torque capacity full_nm=%.6f partial_nm=%.6f relative_delta=%.6f"),
            FullOut.TransmittedClutchTorqueNm,
            PartialOut.TransmittedClutchTorqueNm,
            RelativeTorqueDelta),
        RelativeTorqueDelta <= 0.001f);
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabClutchDrivelineTimestepConsistencyTest,
    "PinkCab.Vehicle.Physics.P02.ClutchModel.TimestepConsistency",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabClutchDrivelineTimestepConsistencyTest::RunTest(const FString&)
{
    FPinkCabClutchDrivelineConfig Config;
    Config.EngineEffectiveInertia = 0.17f;
    Config.MaxClutchTorqueNm = 390.0f;
    Config.SynchronizationTimeSeconds = 0.20f;
    Config.LockedSlipRpm = 25.0f;
    FPinkCabClutchDrivelineModel Model(Config);

    struct FResult
    {
        float FinalEngineRpm = 0.0f;
        float MeanClutchTorqueNm = 0.0f;
    };

    const auto Simulate = [&Model, &Config](
        const float OuterDeltaSeconds) -> FResult
    {
        constexpr float DurationSeconds = 0.24f;
        constexpr float AvailableTorqueNm = 180.0f;
        constexpr float EngineDragTorqueNm = 50.0f;
        constexpr float ShaftEquivalentRpm = 0.0f;
        constexpr float Coupling01 = 0.75f;
        constexpr float EffectiveRatio = 14.72f;
        constexpr float Efficiency = 0.90f;
        constexpr float RadPerSecondToRpm =
            60.0f / (2.0f * PI);

        float EngineRpm = 925.0f;
        float ElapsedSeconds = 0.0f;
        double ClutchImpulseNmSeconds = 0.0;

        while (ElapsedSeconds
            < DurationSeconds - KINDA_SMALL_NUMBER)
        {
            const float Dt = FMath::Min(
                OuterDeltaSeconds,
                DurationSeconds - ElapsedSeconds);

            FPinkCabClutchDrivelineInput Input;
            Input.DeltaSeconds = Dt;
            Input.EngineRpm = EngineRpm;
            Input.ShaftEquivalentEngineRpm =
                ShaftEquivalentRpm;
            Input.AvailableEngineTorqueNm =
                AvailableTorqueNm;
            Input.EngineDragTorqueNm =
                EngineDragTorqueNm;
            Input.ClutchCoupling01 = Coupling01;
            Input.DrivetrainTorqueCapacity01 = 1.0f;
            Input.EffectiveGearRatio = EffectiveRatio;
            Input.TransmissionEfficiency = Efficiency;

            const FPinkCabClutchDrivelineOutput Out =
                Model.Step(Input);

            const float FreeEngineDeltaRpm =
                ((AvailableTorqueNm
                    - EngineDragTorqueNm)
                    / Config.EngineEffectiveInertia)
                * Dt
                * RadPerSecondToRpm;
            EngineRpm +=
                FreeEngineDeltaRpm
                + Out.EngineReactionDeltaRpm;
            ClutchImpulseNmSeconds +=
                static_cast<double>(
                    Out.TransmittedClutchTorqueNm)
                * static_cast<double>(Dt);
            ElapsedSeconds += Dt;
        }

        FResult Result;
        Result.FinalEngineRpm = EngineRpm;
        Result.MeanClutchTorqueNm =
            static_cast<float>(
                ClutchImpulseNmSeconds
                / static_cast<double>(
                    DurationSeconds));
        return Result;
    };

    const FResult At30 = Simulate(1.0f / 30.0f);
    const FResult At60 = Simulate(1.0f / 60.0f);
    const FResult At120 = Simulate(1.0f / 120.0f);

    const auto RelativeDelta = [](const float A, const float B)
    {
        return FMath::Abs(A - B)
            / FMath::Max3(
                FMath::Abs(A),
                FMath::Abs(B),
                1.0f);
    };

    TestTrue(
        TEXT("clutch engine reaction is timestep-consistent 30 vs 120"),
        RelativeDelta(
            At30.FinalEngineRpm,
            At120.FinalEngineRpm)
            <= 0.01f);
    TestTrue(
        TEXT("clutch engine reaction is timestep-consistent 60 vs 120"),
        RelativeDelta(
            At60.FinalEngineRpm,
            At120.FinalEngineRpm)
            <= 0.01f);
    TestTrue(
        TEXT("time-weighted clutch torque is timestep-consistent 30 vs 120"),
        RelativeDelta(
            At30.MeanClutchTorqueNm,
            At120.MeanClutchTorqueNm)
            <= 0.01f);
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabClutchPressReactionBudgetTest,
    "PinkCab.Vehicle.Physics.P03.ClutchPressReactionBudget",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabClutchPressReactionBudgetTest::RunTest(const FString&)
{
    // Human gate 2026-10-02: pressing Q while the engine is spinning above
    // shaft speed must unload the clutch promptly. A long disengagement ramp
    // lets equal/opposite clutch reaction pull RPM toward the shaft before the
    // plates are actually open, which feels like a false snap to idle.
    const FPinkCabVehicleControlRuntimeConfig ControlConfig;
    TestTrue(TEXT("Q press disengages within the bounded physical-response window"),
        ControlConfig.ClutchPressSeconds <= 0.10f);

    FPinkCabClutchDrivelineConfig ClutchConfig;
    ClutchConfig.EngineEffectiveInertia = 0.17f;
    ClutchConfig.MaxClutchTorqueNm = 390.0f;
    ClutchConfig.SynchronizationTimeSeconds = 0.20f;
    ClutchConfig.LockedSlipRpm = 25.0f;
    FPinkCabClutchDrivelineModel Model(ClutchConfig);

    FPinkCabGearboxControllerConfig GearboxConfig;
    float Pedal = 0.0f;
    float EngineRpm = 3000.0f;
    constexpr float ShaftRpm = 1500.0f;
    constexpr float Dt = 1.0f / 60.0f;
    constexpr int32 MaxSteps = 16;
    float TotalReactionDropRpm = 0.0f;
    int32 Steps = 0;

    while (Pedal < 1.0f - KINDA_SMALL_NUMBER && Steps < MaxSteps)
    {
        Pedal = FPinkCabVehicleInputResponse::StepAxis(
            Pedal,
            1.0f,
            Dt,
            ControlConfig.ClutchPressSeconds,
            1.0f);
        const float Coupling =
            FPinkCabGearEngagementValidator::ComputeClutchCoupling(
                GearboxConfig,
                Pedal);

        FPinkCabClutchDrivelineInput Input;
        Input.DeltaSeconds = Dt;
        Input.EngineRpm = EngineRpm;
        Input.ShaftEquivalentEngineRpm = ShaftRpm;
        Input.AvailableEngineTorqueNm = 0.0f;
        Input.EngineDragTorqueNm = 0.0f;
        Input.ClutchCoupling01 = Coupling;
        Input.DrivetrainTorqueCapacity01 = 1.0f;
        Input.EffectiveGearRatio = 1.0f;
        Input.TransmissionEfficiency = 0.90f;

        const FPinkCabClutchDrivelineOutput Out = Model.Step(Input);
        if (Out.EngineReactionDeltaRpm < 0.0f)
        {
            TotalReactionDropRpm += -Out.EngineReactionDeltaRpm;
        }
        EngineRpm += Out.EngineReactionDeltaRpm;
        ++Steps;
    }

    TestTrue(TEXT("Q press reaches fully open clutch in bounded mechanical steps"),
        Pedal >= 1.0f - KINDA_SMALL_NUMBER && Steps <= 6);
    TestTrue(
        *FString::Printf(
            TEXT("Q disengagement limits transient clutch-driven RPM loss total_drop_rpm=%.3f final_rpm=%.3f steps=%d"),
            TotalReactionDropRpm,
            EngineRpm,
            Steps),
        TotalReactionDropRpm <= 1050.0f);
    return true;
}

#endif
