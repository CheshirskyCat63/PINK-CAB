#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "EngineUtils.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Runtime/PinkCabChaosTatraPawn.h"
#include "Runtime/PinkCabVehicleVisualShellComponent.h"

namespace
{
struct FPinkCabG1TMoveSpawnState
{
    TWeakObjectPtr<APinkCabChaosTatraPawn> Pawn;
    FVector MenuLocation = FVector::ZeroVector;
    FVector ReleaseLocation = FVector::ZeroVector;
    double MenuObservedSeconds = -1.0;
    double ReleaseSeconds = -1.0;
    bool bMenuReleased = false;
};

class FPinkCabG1TMoveSpawnSettledCommand final : public IAutomationLatentCommand
{
public:
    FPinkCabG1TMoveSpawnSettledCommand(
        FAutomationTestBase* InTest,
        TSharedRef<FPinkCabG1TMoveSpawnState> InState)
        : Test(InTest), State(InState)
    {
    }

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
                State->MenuLocation = It->GetActorLocation();
                State->MenuObservedSeconds = FPlatformTime::Seconds();
                Test->TestTrue(
                    TEXT("G1 T-MOVE car begins in the system-menu state"),
                    It->IsSystemMenuOpen());
                break;
            }
        }

        APinkCabChaosTatraPawn* Pawn = State->Pawn.Get();
        if (!Pawn)
        {
            return false;
        }

        UChaosWheeledVehicleMovementComponent* Movement = Pawn->GetChaosMovement();
        if (!Movement || Movement->GetNumWheels() != 4)
        {
            return false;
        }

        if (!State->bMenuReleased)
        {
            // Observe the actual menu state first. The game is intentionally
            // paused here, so Chaos wheel contact telemetry is not a valid
            // groundedness signal until DRIVE releases the pause.
            if ((FPlatformTime::Seconds() - State->MenuObservedSeconds) < 1.0)
            {
                return false;
            }

            State->ReleaseLocation = Pawn->GetActorLocation();
            Pawn->SetSystemMenuOpen(false);
            Test->TestFalse(
                TEXT("DRIVE releases the system-menu pause"),
                Pawn->IsSystemMenuOpen());
            State->ReleaseSeconds = FPlatformTime::Seconds();
            State->bMenuReleased = true;
            return false;
        }

        // Give Chaos one real second after DRIVE to establish suspension and
        // wheel contacts. A visible spawn drop here is the historical defect.
        if ((FPlatformTime::Seconds() - State->ReleaseSeconds) < 1.0)
        {
            return false;
        }

        const FVector EndLocation = Pawn->GetActorLocation();
        const FVector FromMenu = EndLocation - State->MenuLocation;
        const FVector AfterRelease = EndLocation - State->ReleaseLocation;

        int32 ContactCount = 0;
        for (int32 WheelIndex = 0; WheelIndex < Movement->GetNumWheels(); ++WheelIndex)
        {
            ContactCount += Movement->GetWheelState(WheelIndex).bInContact ? 1 : 0;
        }

        Test->AddInfo(FString::Printf(
            TEXT("G1 T-MOVE spawn diagnostics: menu=%s release=%s end=%s menuDelta=(%.1f,%.1f,%.1f) postReleaseDelta=(%.1f,%.1f,%.1f) contacts=%d speed=%.1fcm/s"),
            *State->MenuLocation.ToCompactString(),
            *State->ReleaseLocation.ToCompactString(),
            *EndLocation.ToCompactString(),
            FromMenu.X,
            FromMenu.Y,
            FromMenu.Z,
            AfterRelease.X,
            AfterRelease.Y,
            AfterRelease.Z,
            ContactCount,
            Movement->GetForwardSpeed()));

        Test->TestTrue(TEXT("car remains above the road failure floor"), EndLocation.Z > -200.0f);
        Test->TestTrue(TEXT("DRIVE transition has no visible vertical spawn drop"),
            FMath::Abs(AfterRelease.Z) < 12.0f);
        Test->TestTrue(TEXT("untouched car does not jump longitudinally after DRIVE"),
            FMath::Abs(AfterRelease.X) < 30.0f);
        Test->TestTrue(TEXT("untouched car does not jump laterally after DRIVE"),
            FMath::Abs(AfterRelease.Y) < 30.0f);
        Test->TestTrue(TEXT("after DRIVE at least two physical wheels are grounded"), ContactCount >= 2);
        Test->TestTrue(TEXT("untouched car remains effectively stationary"),
            FMath::Abs(Movement->GetForwardSpeed()) < 20.0f);

        UPinkCabVehicleVisualShellComponent* Shell = Pawn->GetVehicleVisualShell();
        Test->TestNotNull(TEXT("vehicle visual shell exists"), Shell);
        if (!Shell)
        {
            return true;
        }

        const FName WheelIds[4] = {
            TEXT("WheelFL"), TEXT("WheelFR"), TEXT("WheelRL"), TEXT("WheelRR")};
        for (const FName WheelId : WheelIds)
        {
            UStaticMeshComponent* Wheel = Shell->GetPresentationPartComponent(WheelId);
            Test->TestNotNull(
                *FString::Printf(TEXT("%s presentation wheel exists"), *WheelId.ToString()),
                Wheel);
            if (Wheel)
            {
                UStaticMesh* WheelMesh = Wheel->GetStaticMesh().Get();
                Test->TestNotNull(
                    *FString::Printf(TEXT("%s presentation wheel mesh resolves"), *WheelId.ToString()),
                    WheelMesh);
                Test->TestTrue(
                    *FString::Printf(TEXT("%s presentation wheel is visible"), *WheelId.ToString()),
                    Wheel->IsVisible() && !Wheel->bHiddenInGame);
            }
        }

        return true;
    }

private:
    FAutomationTestBase* Test = nullptr;
    TSharedRef<FPinkCabG1TMoveSpawnState> State;
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabG1TMoveSpawnSettledMenuCarTest,
    "PinkCab.G1.TMove.SpawnSettledMenuCar",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabG1TMoveSpawnSettledMenuCarTest::RunTest(const FString& Parameters)
{
    const bool bOpened = AutomationOpenMap(TEXT("/Game/Dev/Maps/L_PinkCab_ChaosWeave"), true);
    TestTrue(TEXT("G1 T-MOVE map opens"), bOpened);
    if (!bOpened)
    {
        return false;
    }

    const TSharedRef<FPinkCabG1TMoveSpawnState> State =
        MakeShared<FPinkCabG1TMoveSpawnState>();
    ADD_LATENT_AUTOMATION_COMMAND(FPinkCabG1TMoveSpawnSettledCommand(this, State));
    return true;
}

#endif
