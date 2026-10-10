#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "InputCoreTypes.h"
#include "Interaction/PinkCabSemanticInputRouter.h"
#include "Vehicle/PinkCabVehicleInputFrame.h"
#include "Vehicle/PinkCabVehicleInputResponse.h"
#include "Vehicle/PinkCabChaosVehicleDynamicsProvider.h"
#include "Vehicle/PinkCabChaosVehicleMovementComponent.h"
#include "Runtime/PinkCabChaosTatraPawn.h"
#include "Tests/AutomationCommon.h"
#include "EngineUtils.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabCanonicalPedalMappingTest,
    "PinkCab.Vehicle.Input.CanonicalPedals",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabCanonicalPedalMappingTest::RunTest(const FString& Parameters)
{
    const FPinkCabSemanticInputRouter Router = FPinkCabSemanticInputRouter::CreateDefaults();
    TestEqual(TEXT("Q is clutch intent"), Router.Resolve(EKeys::Q), EPinkCabSemanticAction::Clutch);
    TestEqual(TEXT("W is brake"), Router.Resolve(EKeys::W), EPinkCabSemanticAction::Brake);
    TestEqual(TEXT("E is throttle"), Router.Resolve(EKeys::E), EPinkCabSemanticAction::Throttle);
    TestEqual(TEXT("legacy S pedal is unbound"), Router.Resolve(EKeys::S), EPinkCabSemanticAction::None);
    TestEqual(TEXT("reverse lookup returns brake key"), Router.GetKeyForAction(EPinkCabSemanticAction::Brake), EKeys::W);
    TestEqual(TEXT("reverse lookup returns throttle key"), Router.GetKeyForAction(EPinkCabSemanticAction::Throttle), EKeys::E);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabVehicleInputFrameTest,
    "PinkCab.Vehicle.Input.Frame",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabVehicleInputFrameTest::RunTest(const FString& Parameters)
{
    const FPinkCabVehicleInputFrame Frame = FPinkCabVehicleInputFrame::FromDigital(
        true,  // gaze
        true,  // clutch
        true,  // brake
        false  // throttle
    );

    TestTrue(TEXT("gaze intent is preserved"), Frame.bGazeHeld);
    TestEqual(TEXT("digital clutch becomes normalized clutch intent"), Frame.Clutch, 1.0f);
    TestEqual(TEXT("digital brake becomes normalized brake"), Frame.Brake, 1.0f);
    TestEqual(TEXT("released throttle stays zero"), Frame.Throttle, 0.0f);

    const FPinkCabVehicleControlState Controls = Frame.ToControlState(0.35f, 1.0f);
    TestEqual(TEXT("frame preserves steering"), Controls.Steering, 0.35f);
    TestEqual(TEXT("frame carries brake"), Controls.Brake, 1.0f);
    TestEqual(TEXT("frame carries clutch intent"), Controls.Clutch, 1.0f);
    TestEqual(TEXT("cockpit handbrake feeds control state"), Controls.Handbrake, 1.0f);
    return true;
}
namespace
{
class FVerifyLiveClutchCapability final : public IAutomationLatentCommand
{
public:
    explicit FVerifyLiveClutchCapability(FAutomationTestBase* InTest)
        : Test(InTest), Started(FPlatformTime::Seconds()) {}
    bool Update() override
    {
        if (FPlatformTime::Seconds() - Started > 15.0)
        {
            Test->AddError(TEXT("real clutch capability fixture did not initialize"));
            return true;
        }
        UWorld* World = AutomationCommon::GetAnyGameWorld();
        if (!World) return false;
        for (TActorIterator<APinkCabChaosTatraPawn> It(World); It; ++It)
        {
            const auto* Movement = Cast<UPinkCabChaosVehicleMovementComponent>(It->GetChaosMovement());
            if (!Movement || !Movement->IsPhysicsStateCreated() || Movement->Wheels.Num() != 4) continue;
            Test->TestEqual(TEXT("configured real car exposes native integration availability, not full-lock acceptance"),
                It->GetPinkCabDynamicsProvider().GetMechanicalClutchCapability(),
                EPinkCabMechanicalClutchCapability::NativeConstraintExtension);
            return true;
        }
        return false;
    }
private:
    FAutomationTestBase* Test;
    double Started;
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPinkCabChaosClutchCapabilityTest,
    "PinkCab.Vehicle.Input.ClutchCapability",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabChaosClutchCapabilityTest::RunTest(const FString& Parameters)
{
    if (!AutomationOpenMap(TEXT("/Game/Dev/Maps/L_PinkCab_L1_EndlessStraight"), true)) return false;
    ADD_LATENT_AUTOMATION_COMMAND(FVerifyLiveClutchCapability(this));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPinkCabUnconfiguredClutchCapabilityTest,
    "PinkCab.Vehicle.Input.UnconfiguredClutchCapability",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabUnconfiguredClutchCapabilityTest::RunTest(const FString& Parameters)
{
    const FPinkCabChaosVehicleDynamicsProvider NullProvider(nullptr);
    TestEqual(TEXT("null provider cannot expose a usable clutch"), NullProvider.GetMechanicalClutchCapability(),
        EPinkCabMechanicalClutchCapability::Unsupported);
    auto* Movement = NewObject<UPinkCabChaosVehicleMovementComponent>();
    const FPinkCabChaosVehicleDynamicsProvider Unregistered(Movement);
    TestEqual(TEXT("uncreated physics is not a configured runtime capability"), Unregistered.GetMechanicalClutchCapability(),
        EPinkCabMechanicalClutchCapability::Unsupported);
    FPinkCabClutchDrivelineConfig Invalid;
    Invalid.MaxClutchTorqueNm = 0.0f;
    Movement->ConfigurePinkCabClutch(Invalid);
    TestEqual(TEXT("invalid clutch configuration cannot advertise availability"), Unregistered.GetMechanicalClutchCapability(),
        EPinkCabMechanicalClutchCapability::Unsupported);
    auto* Stock = NewObject<UChaosWheeledVehicleMovementComponent>();
    const FPinkCabChaosVehicleDynamicsProvider WrongType(Stock);
    TestEqual(TEXT("stock movement has no project clutch extension"), WrongType.GetMechanicalClutchCapability(),
        EPinkCabMechanicalClutchCapability::Unsupported);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPedalResponseTest,
    "PinkCab.Vehicle.Input.PedalResponse",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabPedalResponseTest::RunTest(const FString& Parameters)
{
    TestEqual(TEXT("press ramps continuously"),
        FPinkCabVehicleInputResponse::StepAxis(0.0f, 1.0f, 0.10f, 0.20f, 1.00f), 0.5f);
    TestEqual(TEXT("release uses independent duration"),
        FPinkCabVehicleInputResponse::StepAxis(1.0f, 0.0f, 0.25f, 0.20f, 1.00f), 0.75f);
    TestEqual(TEXT("fast clutch release envelope reaches zero in 0.20 seconds"),
        FPinkCabVehicleInputResponse::StepAxis(1.0f, 0.0f, 0.20f, 0.16f, 0.20f), 0.0f);
    TestTrue(TEXT("slow clutch release remains partially engaged at 0.20 seconds"),
        FPinkCabVehicleInputResponse::StepAxis(1.0f, 0.0f, 0.20f, 0.16f, 1.20f) > 0.8f);
    return true;
}

#endif

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabRouterDrivenInputFrameTest,
    "PinkCab.Vehicle.Input.RouterDrivenFrame",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabRouterDrivenInputFrameTest::RunTest(const FString& Parameters)
{
    const FPinkCabSemanticInputRouter Router = FPinkCabSemanticInputRouter::CreateDefaults();
    const TSet<FKey> DownKeys = {EKeys::SpaceBar, EKeys::Q, EKeys::E};
    const FPinkCabVehicleInputFrame Frame = FPinkCabVehicleInputFrame::FromRouter(
        Router,
        [&DownKeys](const FKey& Key) { return DownKeys.Contains(Key); });

    TestTrue(TEXT("router drives gaze from Space"), Frame.bGazeHeld);
    TestEqual(TEXT("router drives clutch intent from Q"), Frame.Clutch, 1.0f);
    TestEqual(TEXT("router drives throttle from E"), Frame.Throttle, 1.0f);
    TestEqual(TEXT("W brake stays released when W is not down"), Frame.Brake, 0.0f);
    return true;
}
#endif
