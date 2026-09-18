#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Vehicle/PinkCabChaosTatraPawn.h"

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

#endif
