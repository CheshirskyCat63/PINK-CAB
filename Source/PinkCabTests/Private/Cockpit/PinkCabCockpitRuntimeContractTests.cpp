#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "EngineUtils.h"
#include "Camera/CameraComponent.h"
#include "Cockpit/PinkCabCockpitAssemblyComponent.h"
#include "Vehicle/PinkCabChaosTatraPawn.h"

class FPinkCabCockpitRuntimeContractCommand final : public IAutomationLatentCommand
{
public:
    explicit FPinkCabCockpitRuntimeContractCommand(FAutomationTestBase* InTest) : Test(InTest) {}

    virtual bool Update() override
    {
        UWorld* World = AutomationCommon::GetAnyGameWorld();
        if (!World)
        {
            return false;
        }

        APinkCabChaosTatraPawn* Pawn = nullptr;
        for (TActorIterator<APinkCabChaosTatraPawn> It(World); It; ++It)
        {
            Pawn = *It;
            break;
        }
        Test->TestNotNull(TEXT("playable map spawns Tatra pawn"), Pawn);
        if (!Pawn)
        {
            return true;
        }

        UPinkCabCockpitAssemblyComponent* Assembly = Pawn->GetCockpitAssembly();
        Test->TestNotNull(TEXT("Tatra pawn owns cockpit assembly"), Assembly);
        Test->TestNotNull(TEXT("Tatra pawn owns driver camera"), Pawn->GetDriverCamera());
        if (!Assembly)
        {
            return true;
        }

        for (uint8 Raw = 0; Raw <= static_cast<uint8>(EPinkCabCockpitSlot::RightMirror); ++Raw)
        {
            const EPinkCabCockpitSlot Slot = static_cast<EPinkCabCockpitSlot>(Raw);
            Test->TestNotNull(*FString::Printf(TEXT("slot %s has an anchor"), *PinkCabCockpitSlotId(Slot).ToString()), Assembly->GetSlotComponent(Slot));
        }
        Test->TestEqual(TEXT("all stable cockpit slots are registered"), Assembly->GetRegisteredSlotCount(), 22);

        const UCameraComponent* DriverCamera = Pawn->GetDriverCamera();
        Test->TestTrue(TEXT("driver camera is active in normal play"), DriverCamera && DriverCamera->IsActive());
        TInlineComponentArray<UCameraComponent*> Cameras(Pawn);
        for (const UCameraComponent* Camera : Cameras)
        {
            if (Camera && Camera->GetFName() == FName(TEXT("ChaseCamera")))
            {
                Test->TestFalse(TEXT("debug chase camera is inactive in normal play"), Camera->IsActive());
            }
        }
        return true;
    }

private:
    FAutomationTestBase* Test = nullptr;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabCockpitRuntimeContractTest,
    "PinkCab.Cockpit.Runtime.Contract",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabCockpitRuntimeContractTest::RunTest(const FString& Parameters)
{
    const bool bOpened = AutomationOpenMap(TEXT("/Game/Dev/Maps/L_PinkCab_ChaosWeave"), true);
    TestTrue(TEXT("cockpit runtime map opens"), bOpened);
    if (!bOpened)
    {
        return false;
    }

    ADD_LATENT_AUTOMATION_COMMAND(FPinkCabCockpitRuntimeContractCommand(this));
    return true;
}

#endif
