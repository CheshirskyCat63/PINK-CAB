#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "EngineUtils.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Runtime/PinkCabChaosTatraPawn.h"
#include "Runtime/PinkCabVehicleVisualShellComponent.h"
#include "Vehicle/PinkCabVehicleDefinition.h"
#include "Vehicle/PinkCabVehicleDefinitionTestUtils.h"

class FPinkCabCookedVehicleDefinitionSwapCommand final : public IAutomationLatentCommand
{
public:
    explicit FPinkCabCookedVehicleDefinitionSwapCommand(FAutomationTestBase* InTest)
        : Test(InTest)
    {
    }

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
        Test->TestNotNull(TEXT("same generic vehicle pawn exists"), Pawn);
        if (!Pawn) return true;
        Pawn->SetActorTickEnabled(false);

        const UPinkCabVehicleDefinition* Tatra =
            PinkCabVehicleDefinitionTestUtils::LoadTatra();
        const UPinkCabVehicleDefinition* Fixture =
            PinkCabVehicleDefinitionTestUtils::LoadFixture();
        Test->TestNotNull(TEXT("cooked Tatra definition resolves"), Tatra);
        Test->TestNotNull(TEXT("cooked fixture definition resolves"), Fixture);
        if (!Tatra || !Fixture) return true;

        FString Failure;
        Test->TestTrue(TEXT("cooked Tatra definition validates"), Tatra->IsValid(&Failure));
        Failure.Reset();
        Test->TestTrue(TEXT("cooked fixture definition validates"), Fixture->IsValid(&Failure));

        Test->TestTrue(TEXT("Tatra applies through generic definition API"),
            Pawn->ApplyVehicleDefinition(*Tatra));
        Test->TestEqual(TEXT("Tatra definition id is active"),
            Pawn->GetVehicleDefinitionId(), Tatra->VehicleId);
        Test->TestEqual(TEXT("Tatra visual inventory comes from definition"),
            Pawn->GetVehicleVisualShell()->GetPresentationPartCount(),
            Tatra->VisualProfile.PresentationParts.Num());
        Test->TestEqual(TEXT("Tatra articulation inventory comes from definition"),
            Pawn->GetVehicleVisualShell()->GetProfile().Articulations.Num(),
            Tatra->VisualProfile.Articulations.Num());

        Test->TestTrue(TEXT("fixture applies to the exact same pawn instance"),
            Pawn->ApplyVehicleDefinition(*Fixture));
        Test->TestEqual(TEXT("fixture definition id replaces Tatra"),
            Pawn->GetVehicleDefinitionId(), Fixture->VehicleId);
        Test->TestEqual(TEXT("fixture visual inventory comes from definition"),
            Pawn->GetVehicleVisualShell()->GetPresentationPartCount(),
            Fixture->VisualProfile.PresentationParts.Num());
        Test->TestEqual(TEXT("fixture articulation inventory comes from definition"),
            Pawn->GetVehicleVisualShell()->GetProfile().Articulations.Num(),
            Fixture->VisualProfile.Articulations.Num());

        UChaosWheeledVehicleMovementComponent* Movement = Pawn->GetChaosMovement();
        Test->TestNotNull(TEXT("same pawn retains Chaos movement"), Movement);
        if (Movement)
        {
            Test->TestEqual(TEXT("fixture keeps four wheel setups"), Movement->WheelSetups.Num(), 4);
            for (int32 Index = 0; Index < 4 && Index < Movement->WheelSetups.Num(); ++Index)
            {
                Test->TestEqual(
                    *FString::Printf(TEXT("fixture wheel %d binding comes from definition"), Index),
                    Movement->WheelSetups[Index].BoneName,
                    Fixture->Wheels[Index].BoneName);
            }
        }

        Test->TestTrue(TEXT("Tatra can be restored with the same generic API"),
            Pawn->ApplyVehicleDefinition(*Tatra));
        Test->TestEqual(TEXT("Tatra id restores after fixture proof"),
            Pawn->GetVehicleDefinitionId(), Tatra->VehicleId);
        return true;
    }

private:
    FAutomationTestBase* Test = nullptr;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabCookedVehicleDefinitionSwapTest,
    "PinkCab.Vehicle.Definition.CookedTwoVehicleSwap",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabCookedVehicleDefinitionSwapTest::RunTest(const FString& Parameters)
{
    const bool bOpened =
        AutomationOpenMap(TEXT("/Game/Dev/Maps/L_PinkCab_ChaosWeave"), true);
    TestTrue(TEXT("vehicle runtime map opens"), bOpened);
    if (!bOpened) return false;

    ADD_LATENT_AUTOMATION_COMMAND(FPinkCabCookedVehicleDefinitionSwapCommand(this));
    return true;
}

#endif
