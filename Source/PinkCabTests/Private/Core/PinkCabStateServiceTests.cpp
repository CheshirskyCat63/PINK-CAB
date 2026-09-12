#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Core/PinkCabStateService.h"

class FPinkCabFakeStateService final : public IPinkCabStateService
{
public:
    FPinkCabStableId GetServiceId() const override
    {
        return FPinkCabStableId(TEXT("service:test"));
    }

    void ResetState(EPinkCabResetScope Scope) override
    {
        LastScope = Scope;
        ++ResetCount;
    }

    EPinkCabResetScope LastScope = EPinkCabResetScope::Runtime;
    int32 ResetCount = 0;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabStateServiceContractTest,
    "PinkCab.Core.Foundation.StateService",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabStateServiceContractTest::RunTest(const FString& Parameters)
{
    FPinkCabFakeStateService Service;
    TestTrue(TEXT("service id is stable and valid"), Service.GetServiceId().IsValid());

    Service.ResetState(EPinkCabResetScope::Workday);
    TestEqual(TEXT("reset scope is delivered"), Service.LastScope, EPinkCabResetScope::Workday);
    TestEqual(TEXT("reset is invoked exactly once"), Service.ResetCount, 1);

    Service.ResetState(EPinkCabResetScope::Profile);
    TestEqual(TEXT("profile reset scope is delivered"), Service.LastScope, EPinkCabResetScope::Profile);
    TestEqual(TEXT("second explicit reset is observable"), Service.ResetCount, 2);
    return true;
}

#endif
