#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "Vehicle/PinkCabChaosTatraPawn.h"
#include "EngineUtils.h"

class FPinkCabPrototypeVisualRuntimeCommand final : public IAutomationLatentCommand
{
public:
    explicit FPinkCabPrototypeVisualRuntimeCommand(FAutomationTestBase* InTest)
        : Test(InTest) {}

    virtual bool Update() override
    {
        UWorld* World = AutomationCommon::GetAnyGameWorld();
        if (!World) return false;

        APinkCabChaosTatraPawn* Pawn = nullptr;
        for (TActorIterator<APinkCabChaosTatraPawn> It(World); It; ++It)
        {
            Pawn = *It;
            break;
        }

        Test->TestNotNull(TEXT("prototype pawn exists"), Pawn);
        if (!Pawn) return true;

        Test->TestEqual(TEXT("prototype visual profile is explicit"),
            Pawn->GetPrototypeVisualProfileId(), FName(TEXT("Epic.SportsCar.Manny")));
        USkeletalMeshComponent* Driver = Pawn->GetPrototypeDriverVisual();
        Test->TestNotNull(TEXT("driver placeholder component exists"), Driver);
        Test->TestTrue(TEXT("driver placeholder has a skeletal mesh"),
            Driver && Driver->GetSkeletalMeshAsset() != nullptr);
        Test->TestTrue(TEXT("driver camera remains active"),
            Pawn->GetDriverCamera() && Pawn->GetDriverCamera()->IsActive());
        return true;
    }

private:
    FAutomationTestBase* Test = nullptr;
};
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPrototypeVisualRuntimeTest,
    "PinkCab.Cockpit.PrototypeVisual.Runtime",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabPrototypeVisualRuntimeTest::RunTest(const FString& Parameters)
{
    const bool bOpened = AutomationOpenMap(TEXT("/Game/Dev/Maps/L_PinkCab_ChaosWeave"), true);
    TestTrue(TEXT("prototype visual runtime map opens"), bOpened);
    if (!bOpened) return false;

    ADD_LATENT_AUTOMATION_COMMAND(FPinkCabPrototypeVisualRuntimeCommand(this));
    return true;
}

#endif

