#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "EngineUtils.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Interaction/PinkCabInteractionModel.h"
#include "Runtime/PinkCabChaosTatraPawn.h"
#include "Vehicle/PinkCabVehicleInputFrame.h"
#include "World/PinkCabChaosWeaveCourse.h"

struct FPinkCabChaosRuntimeState
{
    TWeakObjectPtr<APinkCabChaosTatraPawn> Pawn;
    TWeakObjectPtr<APinkCabChaosWeaveCourse> Course;
    FVector StartLocation = FVector::ZeroVector;
    FRotator StartRotation = FRotator::ZeroRotator;
    bool bCockpitPrimed = false;
};

class FPinkCabDrivePhaseCommand final : public IAutomationLatentCommand
{
public:
    FPinkCabDrivePhaseCommand(
        FAutomationTestBase* InTest,
        TSharedRef<FPinkCabChaosRuntimeState> InState,
        float InDurationSeconds,
        float InThrottle,
        float InSteering)
        : Test(InTest), State(InState), DurationSeconds(InDurationSeconds),
          Throttle(InThrottle), Steering(InSteering) {}
    virtual bool Update() override
    {
        UWorld* World = AutomationCommon::GetAnyGameWorld();
        if (!World)
        {
            return false;
        }

        if (!State->Pawn.IsValid())
        {
            for (TActorIterator<APinkCabChaosTatraPawn> It(World); It; ++It)
            {
                State->Pawn = *It;
                State->StartLocation = It->GetActorLocation();
                State->StartRotation = It->GetActorRotation();
                AController* Controller = It->GetController();
                Test->TestNotNull(TEXT("PIE pawn remains possessed for Chaos input processing"), Controller);
                Test->TestTrue(TEXT("PIE controller is local"), Controller && Controller->IsLocalController());
                Test->TestTrue(TEXT("system menu starts open before Chaos drive smoke"), It->IsSystemMenuOpen());
                It->SetSystemMenuOpen(false);
                Test->TestFalse(TEXT("Chaos drive smoke resumes from system menu"), It->IsSystemMenuOpen());
                It->SetActorTickEnabled(false);
                break;
            }
            for (TActorIterator<APinkCabChaosWeaveCourse> It(World); It; ++It)
            {
                State->Course = *It;
                break;
            }
        }

        APinkCabChaosTatraPawn* Pawn = State->Pawn.Get();
        if (!Pawn)
        {
            return false;
        }

        if (!State->bCockpitPrimed)
        {
            UChaosWheeledVehicleMovementComponent* Movement = Pawn->GetChaosMovement();
            Test->TestNotNull(TEXT("live cockpit has Chaos movement"), Movement);
            if (Movement)
            {
                Test->TestFalse(TEXT("live cockpit begins with engine off"), Movement->bMechanicalSimEnabled);
                Test->TestFalse(TEXT("legacy bool handbrake path is never authoritative"), Movement->GetHandbrakeInput());
            }
            Test->TestTrue(TEXT("live cockpit begins with parking lever engaged"),
                Pawn->GetCockpitState().GetHandbrakeAmount() > 0.99f);
            Test->TestTrue(TEXT("ignition interaction reaches live Chaos pawn"),
                Pawn->ApplyCockpitInteraction({FName(TEXT("Ignition")), EPinkCabInteractionGesture::PressHold, 1}));

            Pawn->ApplyPhysicalControlMouseDelta(TEXT("Handbrake"), true, 0.0f, 500.0f, 0.1f);
            Pawn->ApplyPhysicalControlMouseDelta(NAME_None, false, 0.0f, 0.0f, 0.1f);
            Test->TestEqual(TEXT("mouse actuator releases parking lever"),
                Pawn->GetCockpitState().GetHandbrakeAmount(), 0.0f);

            Pawn->ApplyPhysicalControlMouseDelta(TEXT("Gearbox"), true, -160.0f, 0.0f, 0.05f);
            Pawn->ApplyPhysicalControlMouseDelta(TEXT("Gearbox"), true, 0.0f, 140.0f, 0.05f);
            const FPinkCabVehicleInputFrame ClutchFrame =
                FPinkCabVehicleInputFrame::FromDigital(false, true, false, false);
            Pawn->ApplyVehicleInputFrame(ClutchFrame, 0.0f);
            const FPinkCabVehicleInputFrame CoupledFrame =
                FPinkCabVehicleInputFrame::FromDigital(false, false, false, false);
            Pawn->ApplyVehicleInputFrame(CoupledFrame, 0.0f);

            if (Movement)
            {
                Test->TestTrue(TEXT("ignition enables live mechanical simulation"), Movement->bMechanicalSimEnabled);
                Test->TestFalse(TEXT("legacy bool handbrake stays disabled after release"), Movement->GetHandbrakeInput());
                Test->TestEqual(TEXT("physical H-gate plus clutch engages first"), Movement->GetTargetGear(), 1);
            }
            State->bCockpitPrimed = true;
        }

        FPinkCabVehicleControlState Controls;
        Controls.SetThrottle(Throttle);
        Controls.SetSteering(Steering);
        Controls.SetBrake(0.0f);
        Controls.SetHandbrake(0.0f);
        Pawn->GetPinkCabDynamicsProvider().ApplyControls(Controls);

        if (PhaseStartSeconds < 0.0)
        {
            Test->TestNotNull(TEXT("weave course exists in PIE world"), State->Course.Get());
            if (UChaosWheeledVehicleMovementComponent* Movement = Pawn->GetChaosMovement())
            {
                float TorqueMin = 0.0f;
                float TorqueMax = 0.0f;
                Movement->EngineSetup.TorqueCurve.GetRichCurveConst()->GetValueRange(TorqueMin, TorqueMax);
                Test->TestTrue(TEXT("PIE pawn torque curve has positive output"), TorqueMax > 0.0f);
                Test->TestEqual(TEXT("PIE pawn has four simulated wheel setups"), Movement->WheelSetups.Num(), 4);

                int32 ContactCount = 0;
                for (int32 WheelIndex = 0; WheelIndex < Movement->GetNumWheels(); ++WheelIndex)
                {
                    ContactCount += Movement->GetWheelState(WheelIndex).bInContact ? 1 : 0;
                }
                if (Movement->GetCurrentGear() != 1 || ContactCount < 2)
                {
                    return false;
                }

                Test->AddInfo(FString::Printf(
                    TEXT("PIE phase ready: throttle=%.2f torqueMax=%.3f gear=%d rpm=%.1f contacts=%d"),
                    Throttle, TorqueMax, Movement->GetCurrentGear(),
                    Movement->GetEngineRotationSpeed(), ContactCount));
            }
            PhaseStartSeconds = FPlatformTime::Seconds();
        }

        return (FPlatformTime::Seconds() - PhaseStartSeconds) >= DurationSeconds;
    }

private:
    FAutomationTestBase* Test = nullptr;
    TSharedRef<FPinkCabChaosRuntimeState> State;
    double PhaseStartSeconds = -1.0;
    float DurationSeconds = 0.0f;
    float Throttle = 0.0f;
    float Steering = 0.0f;
};
class FPinkCabVerifyDriveCommand final : public IAutomationLatentCommand
{
public:
    FPinkCabVerifyDriveCommand(
        FAutomationTestBase* InTest,
        TSharedRef<FPinkCabChaosRuntimeState> InState)
        : Test(InTest), State(InState) {}

    virtual bool Update() override
    {
        APinkCabChaosTatraPawn* Pawn = State->Pawn.Get();
        Test->TestNotNull(TEXT("Chaos pawn remains valid after drive"), Pawn);
        if (!Pawn)
        {
            return true;
        }

        UChaosWheeledVehicleMovementComponent* Movement = Pawn->GetChaosMovement();
        Test->TestNotNull(TEXT("Chaos movement remains valid"), Movement);
        if (!Movement)
        {
            return true;
        }

        const FVector EndLocation = Pawn->GetActorLocation();
        const float ForwardTravelCm = EndLocation.X - State->StartLocation.X;
        const float LateralTravelCm = FMath::Abs(EndLocation.Y - State->StartLocation.Y);
        const FWheelStatus FrontLeft = Movement->GetWheelState(0);
        const FWheelStatus FrontRight = Movement->GetWheelState(1);
        const FWheelStatus RearLeft = Movement->GetWheelState(2);
        const FWheelStatus RearRight = Movement->GetWheelState(3);
        Test->AddInfo(FString::Printf(
            TEXT("quarter-pedal diagnostics: forward=%.1fcm lateral=%.1fcm speed=%.1fcm/s rpm=%.1f brakes=(%.1f,%.1f,%.1f,%.1f) rearL[slip=%d mag=%.1f skid=%d drive=%.1f] rearR[slip=%d mag=%.1f skid=%d drive=%.1f]"),
            ForwardTravelCm, LateralTravelCm, Movement->GetForwardSpeed(),
            Movement->GetEngineRotationSpeed(),
            FrontLeft.BrakeTorque, FrontRight.BrakeTorque,
            RearLeft.BrakeTorque, RearRight.BrakeTorque,
            RearLeft.bIsSlipping ? 1 : 0, RearLeft.SlipMagnitude,
            RearLeft.bIsSkidding ? 1 : 0, RearLeft.DriveTorque,
            RearRight.bIsSlipping ? 1 : 0, RearRight.SlipMagnitude,
            RearRight.bIsSkidding ? 1 : 0, RearRight.DriveTorque));
        Test->TestTrue(TEXT("quarter pedal moves the heavy car without demanding a launch trick"), ForwardTravelCm > 50.0f);
        Test->TestTrue(TEXT("quarter-pedal steering remains physically observable"), LateralTravelCm > 1.0f);
        Test->TestTrue(TEXT("quarter pedal keeps useful forward speed"), FMath::Abs(Movement->GetForwardSpeed()) > 20.0f);
        Test->TestFalse(TEXT("quarter pedal remains below rear-left wheelspin threshold"), RearLeft.bIsSlipping);
        Test->TestFalse(TEXT("quarter pedal remains below rear-right wheelspin threshold"), RearRight.bIsSlipping);
        Test->TestTrue(TEXT("vehicle stays above failure floor"), EndLocation.Z > -200.0f);
        Test->TestFalse(TEXT("torque arcade control stays off"), Movement->TorqueControl.Enabled);
        Test->TestFalse(TEXT("target rotation arcade control stays off"), Movement->TargetRotationControl.Enabled);
        Test->TestFalse(TEXT("stabilize arcade control stays off"), Movement->StabilizeControl.Enabled);

        FPinkCabVehicleTelemetry Telemetry;
        Test->TestTrue(TEXT("provider returns live PIE telemetry"),
            Pawn->GetPinkCabDynamicsProvider().ReadTelemetry(Telemetry));
        Test->TestEqual(TEXT("telemetry mirrors live current gear"), Telemetry.CurrentGear, Movement->GetCurrentGear());
        Test->TestTrue(TEXT("telemetry exposes running engine rpm"), Telemetry.EngineRpm > 750.0f);
        Test->TestEqual(TEXT("telemetry exposes four wheel slots"), Telemetry.Wheels.Num(), 4);
        int32 ContactCount = 0;
        for (const FPinkCabWheelTelemetry& Wheel : Telemetry.Wheels)
        {
            ContactCount += Wheel.bInContact ? 1 : 0;
            Test->TestTrue(TEXT("wheel suspension telemetry is finite"), FMath::IsFinite(Wheel.NormalizedSuspensionLength));
            Test->TestTrue(TEXT("wheel spring telemetry is finite"), FMath::IsFinite(Wheel.SpringForce));
            Test->TestTrue(TEXT("wheel slip telemetry is finite"), FMath::IsFinite(Wheel.SlipMagnitude));
            Test->TestTrue(TEXT("wheel torque telemetry is finite"), FMath::IsFinite(Wheel.DriveTorque));
        }
        Test->TestTrue(TEXT("at least two wheels remain in road contact"), ContactCount >= 2);

        FPinkCabVehicleControlState StopControls;
        StopControls.SetBrake(1.0f);
        Pawn->GetPinkCabDynamicsProvider().ApplyControls(StopControls);
        return true;
    }

private:
    FAutomationTestBase* Test = nullptr;
    TSharedRef<FPinkCabChaosRuntimeState> State;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabChaosRuntimeDriveSmokeTest,
    "PinkCab.Vehicle.ChaosBaseline.Runtime.DriveSmoke",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabChaosRuntimeDriveSmokeTest::RunTest(const FString& Parameters)
{
    const bool bOpened = AutomationOpenMap(TEXT("/Game/Dev/Maps/L_PinkCab_ChaosWeave"), true);
    TestTrue(TEXT("Chaos weave map opens for PIE"), bOpened);
    if (!bOpened)
    {
        return false;
    }

    const TSharedRef<FPinkCabChaosRuntimeState> State = MakeShared<FPinkCabChaosRuntimeState>();
    // Baseline drivability is intentionally proven at quarter throttle.
    // Full throttle is now a power-oversteer/burnout case, not the normal
    // straight-line acceleration contract.
    ADD_LATENT_AUTOMATION_COMMAND(FPinkCabDrivePhaseCommand(this, State, 2.5f, 0.25f, 0.0f));
    ADD_LATENT_AUTOMATION_COMMAND(FPinkCabDrivePhaseCommand(this, State, 1.5f, 0.25f, 0.35f));
    ADD_LATENT_AUTOMATION_COMMAND(FPinkCabVerifyDriveCommand(this, State));
    return true;
}

class FPinkCabVerifyHalfPedalSlipCommand final : public IAutomationLatentCommand
{
public:
    FPinkCabVerifyHalfPedalSlipCommand(
        FAutomationTestBase* InTest,
        TSharedRef<FPinkCabChaosRuntimeState> InState)
        : Test(InTest), State(InState) {}

    virtual bool Update() override
    {
        APinkCabChaosTatraPawn* Pawn = State->Pawn.Get();
        Test->TestNotNull(TEXT("half-pedal Chaos pawn remains valid"), Pawn);
        if (!Pawn || !Pawn->GetChaosMovement())
        {
            return true;
        }

        UChaosWheeledVehicleMovementComponent* Movement = Pawn->GetChaosMovement();
        const FWheelStatus RearLeft = Movement->GetWheelState(2);
        const FWheelStatus RearRight = Movement->GetWheelState(3);
        const float ForwardTravelCm = Pawn->GetActorLocation().X - State->StartLocation.X;
        Test->AddInfo(FString::Printf(
            TEXT("half-pedal power diagnostics: forward=%.1fcm speed=%.1fcm/s rearL[slip=%d mag=%.1f drive=%.1f] rearR[slip=%d mag=%.1f drive=%.1f]"),
            ForwardTravelCm, Movement->GetForwardSpeed(),
            RearLeft.bIsSlipping ? 1 : 0, RearLeft.SlipMagnitude, RearLeft.DriveTorque,
            RearRight.bIsSlipping ? 1 : 0, RearRight.SlipMagnitude, RearRight.DriveTorque));
        Test->TestTrue(TEXT("half pedal moves the car"), ForwardTravelCm > 30.0f);
        Test->TestTrue(TEXT("half pedal reaches rear-axle wheelspin threshold"),
            RearLeft.bIsSlipping || RearRight.bIsSlipping);
        Test->TestFalse(TEXT("half-pedal power test keeps torque arcade control off"), Movement->TorqueControl.Enabled);
        Test->TestFalse(TEXT("half-pedal power test keeps stabilize assist off"), Movement->StabilizeControl.Enabled);
        return true;
    }

private:
    FAutomationTestBase* Test = nullptr;
    TSharedRef<FPinkCabChaosRuntimeState> State;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabChaosRuntimeHalfPedalSlipTest,
    "PinkCab.Vehicle.ChaosBaseline.Runtime.HalfPedalRearSlip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabChaosRuntimeHalfPedalSlipTest::RunTest(const FString& Parameters)
{
    const bool bOpened = AutomationOpenMap(TEXT("/Game/Dev/Maps/L_PinkCab_ChaosWeave"), true);
    TestTrue(TEXT("Chaos weave map opens for half-pedal power test"), bOpened);
    if (!bOpened)
    {
        return false;
    }
    const TSharedRef<FPinkCabChaosRuntimeState> State = MakeShared<FPinkCabChaosRuntimeState>();
    ADD_LATENT_AUTOMATION_COMMAND(FPinkCabDrivePhaseCommand(this, State, 2.5f, 0.50f, 0.0f));
    ADD_LATENT_AUTOMATION_COMMAND(FPinkCabVerifyHalfPedalSlipCommand(this, State));
    return true;
}


class FPinkCabReverseDriveCommand final : public IAutomationLatentCommand
{
public:
    FPinkCabReverseDriveCommand(
        FAutomationTestBase* InTest,
        TSharedRef<FPinkCabChaosRuntimeState> InState)
        : Test(InTest), State(InState) {}

    virtual bool Update() override
    {
        UWorld* World = AutomationCommon::GetAnyGameWorld();
        if (!World)
        {
            return false;
        }
        if (!State->Pawn.IsValid())
        {
            for (TActorIterator<APinkCabChaosTatraPawn> It(World); It; ++It)
            {
                State->Pawn = *It;
                State->StartLocation = It->GetActorLocation();
                State->StartRotation = It->GetActorRotation();
                It->SetSystemMenuOpen(false);
                It->SetActorTickEnabled(false);
                break;
            }
        }

        APinkCabChaosTatraPawn* Pawn = State->Pawn.Get();
        if (!Pawn)
        {
            return false;
        }
        UChaosWheeledVehicleMovementComponent* Movement = Pawn->GetChaosMovement();
        if (!Movement)
        {
            return false;
        }

        if (!State->bCockpitPrimed)
        {
            Test->TestTrue(TEXT("reverse smoke ignition interaction succeeds"),
                Pawn->ApplyCockpitInteraction({FName(TEXT("Ignition")), EPinkCabInteractionGesture::PressHold, 1}));
            Pawn->ApplyPhysicalControlMouseDelta(TEXT("Handbrake"), true, 0.0f, 500.0f, 0.1f);
            Pawn->ApplyPhysicalControlMouseDelta(NAME_None, false, 0.0f, 0.0f, 0.1f);

            Pawn->ApplyPhysicalControlMouseDelta(TEXT("Gearbox"), true, 320.0f, 0.0f, 0.05f);
            Pawn->ApplyPhysicalControlMouseDelta(TEXT("Gearbox"), true, 0.0f, -140.0f, 0.05f);
            const FPinkCabVehicleInputFrame ClutchFrame =
                FPinkCabVehicleInputFrame::FromDigital(false, true, false, false);
            Pawn->ApplyVehicleInputFrame(ClutchFrame, 0.0f);
            const FPinkCabVehicleInputFrame CoupledFrame =
                FPinkCabVehicleInputFrame::FromDigital(false, false, false, false);
            Pawn->ApplyVehicleInputFrame(CoupledFrame, 0.0f);

            Test->TestEqual(TEXT("physical H-gate requests reverse"), Pawn->GetRequestedGear(), -1);
            Test->TestEqual(TEXT("clutch engagement accepts reverse"), Pawn->GetEngagedGear(), -1);
            State->bCockpitPrimed = true;
        }

        int32 ContactCount = 0;
        for (int32 WheelIndex = 0; WheelIndex < Movement->GetNumWheels(); ++WheelIndex)
        {
            ContactCount += Movement->GetWheelState(WheelIndex).bInContact ? 1 : 0;
        }
        if (Movement->GetCurrentGear() != -1 || ContactCount < 2)
        {
            return false;
        }

        FPinkCabVehicleControlState Controls;
        Controls.SetThrottle(0.25f);
        Controls.SetBrake(0.0f);
        Controls.SetHandbrake(0.0f);
        Pawn->GetPinkCabDynamicsProvider().ApplyControls(Controls);

        if (PhaseStartSeconds < 0.0)
        {
            PhaseStartSeconds = FPlatformTime::Seconds();
            return false;
        }
        if ((FPlatformTime::Seconds() - PhaseStartSeconds) < 2.0)
        {
            return false;
        }

        const FVector Travel = Pawn->GetActorLocation() - State->StartLocation;
        const float ForwardAxisTravelCm = FVector::DotProduct(Travel, State->StartRotation.Vector());
        Test->AddInfo(FString::Printf(
            TEXT("reverse diagnostics: travel=%.1fcm speed=%.1fcm/s gear=%d contacts=%d"),
            ForwardAxisTravelCm,
            Movement->GetForwardSpeed(),
            Movement->GetCurrentGear(),
            ContactCount));
        Test->TestTrue(TEXT("reverse gear produces negative longitudinal travel"), ForwardAxisTravelCm < -30.0f);
        Test->TestTrue(TEXT("reverse gear produces negative live Chaos speed"), Movement->GetForwardSpeed() < -20.0f);
        Test->TestEqual(TEXT("live Chaos remains in reverse"), Movement->GetCurrentGear(), -1);
        Test->TestTrue(TEXT("reverse drive keeps at least two wheels in contact"), ContactCount >= 2);

        FPinkCabVehicleTelemetry Telemetry;
        Test->TestTrue(TEXT("provider returns reverse telemetry"),
            Pawn->GetPinkCabDynamicsProvider().ReadTelemetry(Telemetry));
        Test->TestTrue(TEXT("normalized telemetry reports negative reverse speed"), Telemetry.SpeedKmh < 0.0f);
        Test->TestEqual(TEXT("normalized telemetry reports reverse gear"), Telemetry.CurrentGear, -1);
        Test->TestEqual(TEXT("reverse telemetry preserves four wheel slots"), Telemetry.Wheels.Num(), 4);

        FPinkCabVehicleControlState StopControls;
        StopControls.SetBrake(1.0f);
        Pawn->GetPinkCabDynamicsProvider().ApplyControls(StopControls);
        return true;
    }

private:
    FAutomationTestBase* Test = nullptr;
    TSharedRef<FPinkCabChaosRuntimeState> State;
    double PhaseStartSeconds = -1.0;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabChaosRuntimeReverseDriveSmokeTest,
    "PinkCab.Vehicle.ChaosBaseline.Runtime.ReverseDriveSmoke",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabChaosRuntimeReverseDriveSmokeTest::RunTest(const FString& Parameters)
{
    const bool bOpened = AutomationOpenMap(TEXT("/Game/Dev/Maps/L_PinkCab_ChaosWeave"), true);
    TestTrue(TEXT("Chaos weave map opens for reverse smoke"), bOpened);
    if (!bOpened)
    {
        return false;
    }
    const TSharedRef<FPinkCabChaosRuntimeState> State = MakeShared<FPinkCabChaosRuntimeState>();
    ADD_LATENT_AUTOMATION_COMMAND(FPinkCabReverseDriveCommand(this, State));
    return true;
}

#endif
