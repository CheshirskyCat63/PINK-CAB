#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "EngineUtils.h"
#include "Runtime/PinkCabDriverUiComponent.h"
#include "Misc/CoreDelegates.h"
#include "GameFramework/PlayerController.h"
#include "Engine/GameViewportClient.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Runtime/PinkCabChaosTatraPawn.h"

class FPinkCabSystemMenuStartupPauseCommand final : public IAutomationLatentCommand
{
public:
    explicit FPinkCabSystemMenuStartupPauseCommand(FAutomationTestBase* InTest) : Test(InTest) {}
    virtual bool Update() override
    {
        UWorld* World = AutomationCommon::GetAnyGameWorld();
        if (!World) return false;
        Test->TestTrue(TEXT("startup system menu hard-pauses gameplay"), UGameplayStatics::IsGamePaused(World));
        for (TActorIterator<APinkCabChaosTatraPawn> It(World); It; ++It)
        {
            Test->TestTrue(TEXT("startup system menu state is open"), It->IsSystemMenuOpen());
            return true;
        }
        Test->AddError(TEXT("playable Tatra pawn was not found"));
        return true;
    }
private:
    FAutomationTestBase* Test = nullptr;
};

class FPinkCabSystemMenuDriveCommand final : public IAutomationLatentCommand
{
public:
    explicit FPinkCabSystemMenuDriveCommand(FAutomationTestBase* InTest) : Test(InTest) {}
    virtual bool Update() override
    {
        UWorld* World = AutomationCommon::GetAnyGameWorld();
        if (!World) return false;
        for (TActorIterator<APinkCabChaosTatraPawn> It(World); It; ++It)
        {
            Test->TestTrue(TEXT("menu starts open before DRIVE"), It->IsSystemMenuOpen());
            It->SetSystemMenuOpen(false);
            Test->TestFalse(TEXT("DRIVE closes system menu"), It->IsSystemMenuOpen());
            Test->TestFalse(TEXT("DRIVE resumes gameplay"), UGameplayStatics::IsGamePaused(World));
            return true;
        }
        Test->AddError(TEXT("playable Tatra pawn was not found"));
        return true;
    }
private:
    FAutomationTestBase* Test = nullptr;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPinkCabSystemMenuStartupPauseTest,
    "PinkCab.UI.SystemMenu.StartupHardPause",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabSystemMenuStartupPauseTest::RunTest(const FString& Parameters)
{
    const bool bOpened = AutomationOpenMap(TEXT("/Game/Dev/Maps/L_PinkCab_ChaosWeave"), true);
    TestTrue(TEXT("system menu runtime map opens"), bOpened);
    if (!bOpened) return false;
    ADD_LATENT_AUTOMATION_COMMAND(FPinkCabSystemMenuStartupPauseCommand(this));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPinkCabSystemMenuDriveTest,
    "PinkCab.UI.SystemMenu.DriveResumesGameplay",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabSystemMenuDriveTest::RunTest(const FString& Parameters)
{
    const bool bOpened = AutomationOpenMap(TEXT("/Game/Dev/Maps/L_PinkCab_ChaosWeave"), true);
    TestTrue(TEXT("system menu runtime map opens"), bOpened);
    if (!bOpened) return false;
    ADD_LATENT_AUTOMATION_COMMAND(FPinkCabSystemMenuDriveCommand(this));
    return true;
}


class FPinkCabDriverUiInputModeCommand final : public IAutomationLatentCommand
{
public:
    explicit FPinkCabDriverUiInputModeCommand(FAutomationTestBase* InTest) : Test(InTest) {}

    virtual bool Update() override
    {
        UWorld* World = AutomationCommon::GetAnyGameWorld();
        if (!World || !GEngine || !GEngine->GameViewport) return false;
        for (TActorIterator<APinkCabChaosTatraPawn> It(World); It; ++It)
        {
            APinkCabChaosTatraPawn* Pawn = *It;
            APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
            UPinkCabDriverUiComponent* Ui = Pawn->GetDriverUi();
            Test->TestNotNull(TEXT("runtime pawn owns driver UI component"), Ui);
            Test->TestNotNull(TEXT("runtime pawn remains player-controlled"), PC);
            if (!Ui || !PC) return true;

            Test->TestTrue(TEXT("startup menu exposes cursor"), PC->bShowMouseCursor);
            Test->TestEqual(TEXT("startup menu disables mouse capture"),
                GEngine->GameViewport->GetMouseCaptureMode(), EMouseCaptureMode::NoCapture);
            Test->TestEqual(TEXT("startup menu does not lock mouse"),
                GEngine->GameViewport->GetMouseLockMode(), EMouseLockMode::DoNotLock);
            Test->TestFalse(TEXT("startup menu never hides cursor during capture"),
                GEngine->GameViewport->HideCursorDuringCapture());

            Pawn->SetSystemMenuOpen(false);
            Test->TestFalse(TEXT("gameplay mode hides cursor"), PC->bShowMouseCursor);
            Test->TestEqual(TEXT("gameplay captures mouse permanently"),
                GEngine->GameViewport->GetMouseCaptureMode(),
                EMouseCaptureMode::CapturePermanently_IncludingInitialMouseDown);
            Test->TestEqual(TEXT("gameplay locks mouse on capture"),
                GEngine->GameViewport->GetMouseLockMode(), EMouseLockMode::LockOnCapture);
            Test->TestTrue(TEXT("gameplay hides cursor during capture"),
                GEngine->GameViewport->HideCursorDuringCapture());

            Ui->SetPointerCapture(*PC, true);
            Test->TestTrue(TEXT("physical control capture is owned by driver UI"), Ui->IsPointerCaptured());
            Test->TestEqual(TEXT("physical control capture locks always"),
                GEngine->GameViewport->GetMouseLockMode(), EMouseLockMode::LockAlways);

            FCoreDelegates::ApplicationWillDeactivateDelegate.Broadcast();
            Test->TestFalse(TEXT("focus loss releases physical pointer capture"), Ui->IsPointerCaptured());
            Test->TestEqual(TEXT("focus loss restores normal gameplay lock"),
                GEngine->GameViewport->GetMouseLockMode(), EMouseLockMode::LockOnCapture);
            Test->TestFalse(TEXT("focus loss cleanup keeps gameplay cursor hidden"), PC->bShowMouseCursor);
            return true;
        }
        Test->AddError(TEXT("playable Tatra pawn was not found for driver UI input-mode test"));
        return true;
    }
private:
    FAutomationTestBase* Test = nullptr;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPinkCabDriverUiInputModeTest,
    "PinkCab.UI.SystemMenu.InputModeAndFocusCleanup",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabDriverUiInputModeTest::RunTest(const FString& Parameters)
{
    const bool bOpened = AutomationOpenMap(TEXT("/Game/Dev/Maps/L_PinkCab_ChaosWeave"), true);
    TestTrue(TEXT("driver UI input-mode map opens"), bOpened);
    if (!bOpened) return false;
    ADD_LATENT_AUTOMATION_COMMAND(FPinkCabDriverUiInputModeCommand(this));
    return true;
}

#endif
