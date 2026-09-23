#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "EngineUtils.h"
#include "ChaosWheeledVehicleMovementComponent.h"
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
};

bool FPinkCabEngineRestoreCommand::Update()
{
    UWorld* World = AutomationCommon::GetAnyGameWorld();
    if (!World) return false;
    APinkCabChaosTatraPawn* Pawn = nullptr;
    for (TActorIterator<APinkCabChaosTatraPawn> It(World); It; ++It) { Pawn = *It; break; }
    Test->TestNotNull(TEXT("live Tatra pawn exists"), Pawn);
    if (!Pawn) return true;
    Pawn->SetActorTickEnabled(false);    UChaosWheeledVehicleMovementComponent* Movement = Pawn->GetChaosMovement();
    Test->TestNotNull(TEXT("live Chaos movement exists"), Movement);
    if (!Movement) return true;

    Test->TestTrue(TEXT("ignition starts live mechanical sim"), Pawn->ApplyCockpitInteraction(
        {FName(TEXT("Ignition")), EPinkCabInteractionGesture::PressHold, 1}));
    Test->TestTrue(TEXT("mechanical sim running before damage"), Movement->bMechanicalSimEnabled);

    FPinkCabVehicleStateSnapshot HealthySnapshot;
    Test->TestTrue(TEXT("healthy running snapshot captures"), Pawn->CaptureVehicleSnapshot(HealthySnapshot));
    Test->TestTrue(TEXT("terminal engine-oil damage applies"), Pawn->ApplyVehicleHit(
        FPinkCabVehicleHitEvent(EPinkCabVehicleHealthChannel::EngineOil, 1.0f, false)));
    Test->TestFalse(TEXT("terminal engine capability disables mechanical sim"), Movement->bMechanicalSimEnabled);

    Test->TestTrue(TEXT("healthy snapshot restores"), Pawn->RestoreVehicleSnapshot(HealthySnapshot));
    Test->TestFalse(TEXT("restored vehicle no longer terminal"), Pawn->IsVehicleTerminal());
    Test->TestTrue(TEXT("running ignition restores mechanical sim after health restore"),
        Movement->bMechanicalSimEnabled);
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