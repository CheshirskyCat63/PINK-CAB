#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "EngineUtils.h"
#include "Vehicle/PinkCabChaosVehicleMovementComponent.h"
#include "Vehicle/PinkCabVehicleInputFrame.h"
#include "Interaction/PinkCabInteractionModel.h"
#include "Vehicle/PinkCabVehicleStateSnapshot.h"
#include "Runtime/PinkCabChaosTatraPawn.h"

class FPinkCabEngineRestoreCommand final : public IAutomationLatentCommand
{
public:
    explicit FPinkCabEngineRestoreCommand(FAutomationTestBase* InTest) : Test(InTest) {}
    virtual bool Update() override;
private:
    FAutomationTestBase* Test = nullptr;
    double RunStartSeconds = -1.0;
};

bool FPinkCabEngineRestoreCommand::Update()
{
    if (RunStartSeconds < 0.0) RunStartSeconds = FPlatformTime::Seconds();
    if (FPlatformTime::Seconds() - RunStartSeconds > 30.0)
    {
        Test->AddError(TEXT("Engine health restore fixture did not become available within 30 seconds"));
        return true;
    }
    UWorld* World = AutomationCommon::GetAnyGameWorld();
    if (!World) return false;
    APinkCabChaosTatraPawn* Pawn = nullptr;
    for (TActorIterator<APinkCabChaosTatraPawn> It(World); It; ++It) { Pawn = *It; break; }
    Test->TestNotNull(TEXT("live Tatra pawn exists"), Pawn);
    if (!Pawn) return true;
    Pawn->SetSystemMenuOpen(false);
    Pawn->SetActorTickEnabled(false);
    UPinkCabChaosVehicleMovementComponent* Movement =
        Cast<UPinkCabChaosVehicleMovementComponent>(Pawn->GetChaosMovement());
    Test->TestNotNull(TEXT("live Chaos movement exists"), Movement);
    if (!Movement) return true;

    Test->TestTrue(TEXT("ignition starts live mechanical sim"), Pawn->ApplyCockpitInteraction(
        {FName(TEXT("Ignition")), EPinkCabInteractionGesture::PressHold, 1}));
    Test->TestTrue(TEXT("mechanical sim running before damage"), Movement->bMechanicalSimEnabled);
    Test->TestTrue(TEXT("healthy running engine permits combustion"),
        Movement->GetPendingPinkCabDrivelineCommand().bCombustionAllowed);

    FPinkCabVehicleStateSnapshot HealthySnapshot;
    Test->TestTrue(TEXT("healthy running snapshot captures"), Pawn->CaptureVehicleSnapshot(HealthySnapshot));
    Test->TestTrue(TEXT("terminal engine-oil damage applies"), Pawn->ApplyVehicleHit(
        FPinkCabVehicleHitEvent(EPinkCabVehicleHealthChannel::EngineOil, 1.0f, false)));
    Test->TestTrue(TEXT("terminal engine damage preserves mechanical coast and back-drive"),
        Movement->bMechanicalSimEnabled);
    Test->TestFalse(TEXT("terminal engine damage removes combustion permission"),
        Movement->GetPendingPinkCabDrivelineCommand().bCombustionAllowed);

    const FPinkCabVehicleInputFrame ThrottleFrame =
        FPinkCabVehicleInputFrame::FromDigital(false, false, false, true);
    Pawn->ApplyVehicleInputFrame(ThrottleFrame, 0.0f);
    Test->TestFalse(TEXT("subsequent throttle sync cannot restore combustion through terminal damage"),
        Movement->GetPendingPinkCabDrivelineCommand().bCombustionAllowed);
    Test->TestEqual(TEXT("terminal engine damage keeps resolved Chaos throttle at zero"),
        Movement->GetThrottleInput(), 0.0f);
    Test->TestTrue(TEXT("subsequent sync preserves the mechanical simulation"),
        Movement->bMechanicalSimEnabled);

    for (int32 Press = 0; Press < 2; ++Press)
    {
        Test->TestTrue(TEXT("ignition switch remains operable while damaged"),
            Pawn->ApplyCockpitInteraction({FName(TEXT("Ignition")),
                EPinkCabInteractionGesture::PressHold, 1}));
        Test->TestFalse(TEXT("ignition cycling cannot bypass the health permission"),
            Movement->GetPendingPinkCabDrivelineCommand().bCombustionAllowed);
    }

    Test->TestTrue(TEXT("healthy snapshot restores"), Pawn->RestoreVehicleSnapshot(HealthySnapshot));
    Test->TestFalse(TEXT("restored vehicle no longer terminal"), Pawn->IsVehicleTerminal());
    Test->TestTrue(TEXT("healthy restore keeps mechanical simulation available"),
        Movement->bMechanicalSimEnabled);
    Test->TestTrue(TEXT("healthy restore recovers running combustion permission"),
        Movement->GetPendingPinkCabDrivelineCommand().bCombustionAllowed);
    Pawn->ApplyVehicleInputFrame(ThrottleFrame, 0.0f);
    Test->TestTrue(TEXT("healthy restored engine accepts deliberate throttle again"),
        Movement->GetThrottleInput() > 0.0f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabVehicleEngineRestoreRuntimeTest,
    "PinkCab.Vehicle.LiveState.EngineCapabilityRestore",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabVehicleEngineRestoreRuntimeTest::RunTest(const FString& Parameters)
{    const bool bOpened = AutomationOpenMap(TEXT("/Game/Dev/Maps/L_PinkCab_ChaosWeave"), true);
    TestTrue(TEXT("vehicle runtime map opens"), bOpened);
    if (!bOpened) return false;
    ADD_LATENT_AUTOMATION_COMMAND(FPinkCabEngineRestoreCommand(this));
    return true;
}
#endif