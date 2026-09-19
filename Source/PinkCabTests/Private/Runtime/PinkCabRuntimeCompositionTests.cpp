#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Runtime/PinkCabChaosTatraPawn.h"
#include "Runtime/PinkCabDriverUiComponent.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabRuntimePawnReflectedIdentityTest,
    "PinkCab.Runtime.Composition.PawnReflectedIdentity",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabRuntimePawnReflectedIdentityTest::RunTest(const FString& Parameters)
{
    TestEqual(
        TEXT("source relocation never changes the reflected asset identity"),
        APinkCabChaosTatraPawn::StaticClass()->GetPathName(),
        FString(TEXT("/Script/PinkCab.PinkCabChaosTatraPawn")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabDriverUiStateDefaultsTest,
    "PinkCab.Runtime.DriverUi.StateDefaults",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabDriverUiStateDefaultsTest::RunTest(const FString& Parameters)
{
    UPinkCabDriverUiComponent* Ui = NewObject<UPinkCabDriverUiComponent>();
    TestNotNull(TEXT("driver UI component can be constructed independently"), Ui);
    TestFalse(TEXT("system menu starts closed before runtime initialization"), Ui->IsSystemMenuOpen());
    TestFalse(TEXT("physical pointer capture starts released"), Ui->IsPointerCaptured());
    return true;
}

#endif
