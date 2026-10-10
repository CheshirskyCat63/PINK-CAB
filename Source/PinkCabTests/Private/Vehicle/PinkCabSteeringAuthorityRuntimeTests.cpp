#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "EngineUtils.h"
#include "ChaosVehicleWheel.h"
#include "SteeringSystem.h"
#include "Runtime/PinkCabChaosTatraPawn.h"
#include "Vehicle/PinkCabChaosPhysicalProfile.h"
#include "Vehicle/PinkCabChaosVehicleMovementComponent.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPinkCabNativeSteeringAuthorityTest,
    "PinkCab.Vehicle.Actuation.NativeSteeringAuthority",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabNativeSteeringAuthorityTest::RunTest(const FString& Parameters)
{
    auto* Movement = NewObject<UPinkCabChaosVehicleMovementComponent>();
    const auto Profile = FPinkCabChaosPhysicalProfile::ForVariant(EPinkCabCalibrationVariant::Nominal);
    Profile.ApplyToMovement(*Movement);
    auto Mutated = Profile;
    Mutated.SteeringSpeedScaleCurve.Value[1].Y = 0.4f;
    TestTrue(TEXT("steering speed curve is fingerprinted"),
        Mutated.GetDeterministicProfileHash() != Profile.GetDeterministicProfileHash());
    Mutated = Profile;
    Mutated.SteeringSpeedScaleCurve.Authority = EPinkCabPhysicalParameterAuthority::Unspecified;
    TestFalse(TEXT("steering curve requires explicit provenance"), Mutated.HasCompleteProvenance());
    auto Native = Movement->SteeringSetup.GetPhysicsSteeringConfig(
        FVector2D(Profile.WheelbaseMm.Value * 0.1f, Profile.FrontTrackMm.Value * 0.1f));
    Native.MaxSteeringAngle = Profile.FrontWheel.MaxSteerAngleDeg.Value;
    Chaos::FSimpleSteeringSim Steering(&Native);
    for (const float Input : {-0.4f, 0.4f})
    for (const float Side : {-1.0f, 1.0f})
    {
        const float AtRest = Steering.GetSteeringAngle(Input, Native.MaxSteeringAngle, Side);
        TestTrue(TEXT("native wheel sign follows semantic steering"), AtRest * Input > 0.0f);
        for (const float SpeedMph : {0.0f, 20.0f, 60.0f, 120.0f, 180.0f})
        {
            const float Scale = Steering.GetSteeringFromVelocity(SpeedMph);
            const float Actual = Steering.GetSteeringAngle(Input * Scale, Native.MaxSteeringAngle, Side);
            AddInfo(FString::Printf(TEXT("T6_STEER_NATIVE input=%.2f side=%.0f mph=%.0f scale=%.5f angle=%.5f rest=%.5f"), Input, Side, SpeedMph, Scale, Actual, AtRest));
            TestTrue(TEXT("speed alone cannot rewrite the held physical steering target"), FMath::IsNearlyEqual(Actual, AtRest, 0.0001f));
        }
    }
    return true;
}

namespace
{
class FPinkCabHeldSteeringCommand final : public IAutomationLatentCommand
{
public:
    explicit FPinkCabHeldSteeringCommand(FAutomationTestBase* InTest, int32 InGear = 1, float InSteering = 0.12f)
        : Test(InTest), Started(FPlatformTime::Seconds()), DriveGear(InGear), SteeringTarget(InSteering) {}
    bool Update() override
    {
        if (FPlatformTime::Seconds() - Started > 25.0) { Test->AddError(TEXT("held steering fixture timed out")); return true; }
        UWorld* World = AutomationCommon::GetAnyGameWorld();
        if (!World) return false;
        APinkCabChaosTatraPawn* Pawn = nullptr;
        for (TActorIterator<APinkCabChaosTatraPawn> It(World); It; ++It) { Pawn = *It; break; }
        if (!Pawn) return false;
        auto* Movement = Pawn->GetChaosMovement();
        if (!Movement || Movement->Wheels.Num() != 4) return false;
        if (StageStarted < 0.0)
        {
            Pawn->SetSystemMenuOpen(false);
            Pawn->SetActorTickEnabled(false);
            FPinkCabVehicleControlState Controls;
            Controls.SetSteering(SteeringTarget);
            Controls.SetHandbrake(0.0f);
            Controls.SetBrake(Stage == 0 ? 1.0f : 0.0f);
            Controls.SetClutch(0.0f);
            Controls.SetDriveline(Stage == 0 ? 0 : DriveGear, Stage == 0 ? 0 : DriveGear, 1.0f);
            const float Throttle = Stage == 0 ? 0.0f : 0.5f;
            Controls.SetThrottle(Throttle);
            Controls.SetResolvedEngineActuation(true, Throttle, Throttle,
                Movement->EngineSetup.MaxTorque, Movement->EngineSetup.MaxTorque * Throttle);
            if (!Test->TestTrue(TEXT("real vehicle accepts constant steering command"), Pawn->GetPinkCabDynamicsProvider().ApplyControls(Controls))) return true;
            StageStarted = World->GetTimeSeconds();
            return false;
        }
        const double Elapsed = World->GetTimeSeconds() - StageStarted;
        if (Stage == 0)
        {
            if (Elapsed < 1.0) return false;
            for (int32 Index = 0; Index < 2; ++Index) StartAngles[Index] = Movement->Wheels[Index]->GetSteerAngle();
            Stage = 1;
            StageStarted = -1.0;
            return false;
        }
        if (Elapsed > 0.25)
        {
            ++Samples;
            MaxSpeed = FMath::Max(MaxSpeed, FMath::Abs(Movement->GetForwardSpeed()));
            for (int32 Index = 0; Index < 2; ++Index)
                MaxAngleDelta = FMath::Max(MaxAngleDelta, FMath::Abs(Movement->Wheels[Index]->GetSteerAngle() - StartAngles[Index]));
            for (int32 Index = 0; Index < 4; ++Index) AllContacts &= Movement->GetWheelState(Index).bInContact;
        }
        if (Elapsed < 3.0) return false;
        Pawn->SetActorTickEnabled(true);
        Test->AddInfo(FString::Printf(TEXT("T6_STEER_HELD gear=%d target=%.3f start_deg=(%.5f,%.5f) final_deg=(%.5f,%.5f) max_delta_deg=%.5f speed_cm_s=%.3f samples=%d contacts=%d"),
            DriveGear, SteeringTarget, StartAngles[0], StartAngles[1], Movement->Wheels[0]->GetSteerAngle(), Movement->Wheels[1]->GetSteerAngle(), MaxAngleDelta, MaxSpeed, Samples, AllContacts));
        Test->TestTrue(TEXT("native propulsion changes actual road speed"), MaxSpeed > 150.0f);
        Test->TestTrue(TEXT("physical travel follows selected forward or reverse gear"), Movement->GetForwardSpeed() * DriveGear > 50.0f);
        Test->TestTrue(TEXT("both physical front wheels follow the steering sign"),
            StartAngles[0] * SteeringTarget > 0.0f && StartAngles[1] * SteeringTarget > 0.0f);
        Test->TestTrue(TEXT("steering comparison contains live four-contact samples"), Samples >= 20 && AllContacts);
        Test->TestTrue(TEXT("held physical wheel angles survive speed change without new steering"), MaxAngleDelta < 0.05f);
        return true;
    }
private:
    FAutomationTestBase* Test;
    double Started;
    double StageStarted = -1.0;
    int32 DriveGear;
    float SteeringTarget;
    int32 Stage = 0;
    int32 Samples = 0;
    float StartAngles[2] = {};
    float MaxAngleDelta = 0.0f;
    float MaxSpeed = 0.0f;
    bool AllContacts = true;
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPinkCabHeldSteeringRuntimeTest,
    "PinkCab.Vehicle.Actuation.HeldSteeringRuntime",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabHeldSteeringRuntimeTest::RunTest(const FString& Parameters)
{
    if (!AutomationOpenMap(TEXT("/Game/Dev/Maps/L_PinkCab_L1_EndlessStraight"), true)) return false;
    ADD_LATENT_AUTOMATION_COMMAND(FPinkCabHeldSteeringCommand(this));
    return true;
}
#endif

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPinkCabHeldSteeringReverseTest,
    "PinkCab.Vehicle.Actuation.HeldSteeringReverse",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabHeldSteeringReverseTest::RunTest(const FString& Parameters)
{
    if (!AutomationOpenMap(TEXT("/Game/Dev/Maps/L_PinkCab_L1_EndlessStraight"), true)) return false;
    ADD_LATENT_AUTOMATION_COMMAND(FPinkCabHeldSteeringCommand(this, -1, -0.12f));
    return true;
}
#endif

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPinkCabSteeringCurveOwnershipTest,
    "PinkCab.Vehicle.Actuation.NativeSteeringCurveOwnership",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabSteeringCurveOwnershipTest::RunTest(const FString& Parameters)
{
    auto* Movement = NewObject<UPinkCabChaosVehicleMovementComponent>();
    auto* SharedCurve = NewObject<UCurveFloat>();
    SharedCurve->FloatCurve.AddKey(0.0f, 1.0f);
    SharedCurve->FloatCurve.AddKey(120.0f, 0.3f);
    Movement->SteeringSetup.SteeringCurve.ExternalCurve = SharedCurve;
    const auto Profile = FPinkCabChaosPhysicalProfile::ForVariant(EPinkCabCalibrationVariant::Nominal);
    Profile.ApplyToMovement(*Movement);
    TestNull(TEXT("versioned native profile is the only steering curve owner"), Movement->SteeringSetup.SteeringCurve.ExternalCurve.Get());
    TestTrue(TEXT("applying a vehicle profile never mutates a shared steering asset"),
        FMath::IsNearlyEqual(SharedCurve->FloatCurve.Eval(120.0f), 0.3f, 0.0001f));
    TestTrue(TEXT("applied native curve preserves full authored authority at speed"),
        FMath::IsNearlyEqual(Movement->SteeringSetup.SteeringCurve.GetRichCurveConst()->Eval(120.0f), 1.0f, 0.0001f));
    return true;
}
#endif
