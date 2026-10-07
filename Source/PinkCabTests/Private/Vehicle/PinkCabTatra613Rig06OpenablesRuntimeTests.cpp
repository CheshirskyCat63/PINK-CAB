#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "EngineUtils.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "HAL/PlatformTime.h"
#include "Runtime/PinkCabChaosTatraPawn.h"
#include "Runtime/PinkCabVehicleVisualShellComponent.h"

class FPinkCabTatra613Rig06OpenablesCommand final : public IAutomationLatentCommand
{
public:
    explicit FPinkCabTatra613Rig06OpenablesCommand(FAutomationTestBase* InTest)
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
        if (!Pawn) return false;

        UPinkCabVehicleVisualShellComponent* Shell = Pawn->GetVehicleVisualShell();
        if (!Shell) return false;

        if (Phase == 0)
        {
            Test->TestEqual(
                TEXT("live pawn uses RIG06"),
                Pawn->GetVehicleVisualProfileId(),
                FName(TEXT("PinkCab.Visual.Tatra613.Rig06.TexturedOpenables")));
            Test->TestNotNull(
                TEXT("RIG06 exterior is poseable"),
                Shell->GetExteriorPoseablePresentation());
            Test->TestNotNull(
                TEXT("RIG06 cabin render exists"),
                Shell->GetCabinPresentation());
            Test->TestEqual(
                TEXT("RIG06 full-car asset reuses one poseable component for exterior and cabin"),
                Shell->GetCabinPresentation(),
                Shell->GetExteriorPresentation());
            Test->TestNotNull(
                TEXT("RIG06 shared cabin/exterior component is poseable"),
                Shell->GetCabinPoseablePresentation());
            TArray<USkeletalMeshComponent*> SkeletalParts;
            Pawn->GetComponents(SkeletalParts);
            bool bFoundPrototypeDriver = false;
            for (const USkeletalMeshComponent* Part : SkeletalParts)
            {
                if (Part->GetFName() != TEXT("PrototypeDriverVisual")) continue;
                bFoundPrototypeDriver = true;
                Test->TestTrue(TEXT("authored Tatra hides the standing prototype driver"), Part->bHiddenInGame);
            }
            Test->TestTrue(TEXT("prototype driver visibility was checked"), bFoundPrototypeDriver);

            static const FName Bones[] = {
                TEXT("Door_FL"), TEXT("Door_FR"),
                TEXT("Door_RL"), TEXT("Door_RR"),
                TEXT("Trunk_Front"), TEXT("Hood_Rear")
            };
            for (const FName Bone : Bones)
            {
                FTransform Rest;
                Test->TestTrue(
                    *FString::Printf(TEXT("%s live pose exists"), *Bone.ToString()),
                    Shell->GetPoseableBoneTransform(Bone, Rest));
                Closed.Add(Bone, Rest);
                Test->TestEqual(
                    *FString::Printf(TEXT("%s starts closed"), *Bone.ToString()),
                    Pawn->GetTatraOpenable(Bone),
                    0.0f);
            }

            Test->TestFalse(
                TEXT("unknown panel cannot be opened"),
                Pawn->SetTatraOpenable(TEXT("NotATatraPanel"), 1.0f));

            Pawn->SetSystemMenuOpen(false);
            Test->TestTrue(TEXT("driver door accepts open target"),
                Pawn->SetTatraOpenable(TEXT("Door_FL"), 1.0f));
            Test->TestTrue(TEXT("front luggage lid accepts open target"),
                Pawn->SetTatraOpenable(TEXT("Trunk_Front"), 1.0f));
            Test->TestTrue(TEXT("rear engine hood accepts open target"),
                Pawn->SetTatraOpenable(TEXT("Hood_Rear"), 1.0f));
            Started = FPlatformTime::Seconds();
            Phase = 1;
            return false;
        }

        if (Phase == 1)
        {
            const bool bOpen =
                Pawn->GetTatraOpenable(TEXT("Door_FL")) > 0.99f
                && Pawn->GetTatraOpenable(TEXT("Trunk_Front")) > 0.99f
                && Pawn->GetTatraOpenable(TEXT("Hood_Rear")) > 0.99f;
            if (!bOpen && (FPlatformTime::Seconds() - Started) < 1.5)
            {
                return false;
            }
            Test->TestTrue(TEXT("selected openables reach open pose"), bOpen);

            for (const FName Bone : {
                FName(TEXT("Door_FL")),
                FName(TEXT("Trunk_Front")),
                FName(TEXT("Hood_Rear")) })
            {
                FTransform Open;
                Test->TestTrue(
                    *FString::Printf(TEXT("%s open pose readable"), *Bone.ToString()),
                    Shell->GetPoseableBoneTransform(Bone, Open));
                const FTransform* Rest = Closed.Find(Bone);
                if (!Rest) continue;
                Test->TestTrue(
                    *FString::Printf(TEXT("%s hinge pivot position stays fixed"), *Bone.ToString()),
                    Open.GetLocation().Equals(Rest->GetLocation(), 0.001f));
                Test->TestFalse(
                    *FString::Printf(TEXT("%s rotates around authored hinge"), *Bone.ToString()),
                    Open.GetRotation().Equals(Rest->GetRotation(), 0.001f));
            }

            Pawn->SetTatraOpenable(TEXT("Door_FL"), 0.0f);
            Pawn->SetTatraOpenable(TEXT("Trunk_Front"), 0.0f);
            Pawn->SetTatraOpenable(TEXT("Hood_Rear"), 0.0f);
            Started = FPlatformTime::Seconds();
            Phase = 2;
            return false;
        }

        const bool bClosed =
            Pawn->GetTatraOpenable(TEXT("Door_FL")) < 0.01f
            && Pawn->GetTatraOpenable(TEXT("Trunk_Front")) < 0.01f
            && Pawn->GetTatraOpenable(TEXT("Hood_Rear")) < 0.01f;
        if (!bClosed && (FPlatformTime::Seconds() - Started) < 1.5)
        {
            return false;
        }
        Test->TestTrue(TEXT("selected openables return closed"), bClosed);
        return true;
    }

private:
    FAutomationTestBase* Test = nullptr;
    int32 Phase = 0;
    double Started = 0.0;
    TMap<FName, FTransform> Closed;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabTatra613Rig06OpenablesRuntimeTest,
    "PinkCab.Vehicle.Visual.Tatra613Rig06OpenablesRuntime",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabTatra613Rig06OpenablesRuntimeTest::RunTest(const FString& Parameters)
{
    const bool bOpened =
        AutomationOpenMap(TEXT("/Game/Dev/Maps/L_PinkCab_L1_EndlessStraight"), true);
    TestTrue(TEXT("RIG06 openables runtime map opens"), bOpened);
    if (!bOpened) return false;

    ADD_LATENT_AUTOMATION_COMMAND(
        FPinkCabTatra613Rig06OpenablesCommand(this));
    return true;
}

#endif
