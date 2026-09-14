#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Cockpit/PinkCabCockpitSlot.h"
#include "Cockpit/PinkCabCockpitPresentationState.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabCockpitStableSlotContractTest,
    "PinkCab.Cockpit.Contract.StableSlots",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabCockpitStableSlotContractTest::RunTest(const FString& Parameters)
{
    TArray<FPinkCabCockpitSlotDefinition> Definitions;
    for (uint8 Raw = 0; Raw <= static_cast<uint8>(EPinkCabCockpitSlot::RightMirror); ++Raw)
    {
        const EPinkCabCockpitSlot Slot = static_cast<EPinkCabCockpitSlot>(Raw);
        Definitions.Emplace(Slot, PinkCabCockpitSlotId(Slot));
    }

    TestEqual(TEXT("all required cockpit slots are defined"), Definitions.Num(), 22);
    TestTrue(TEXT("unique stable slot definitions validate"), FPinkCabCockpitSlotDefinition::ValidateUnique(Definitions));
    const FPinkCabCockpitSlotDefinition Duplicate = Definitions[0];
    Definitions.Add(Duplicate);
    TestFalse(TEXT("duplicate stable slot IDs are rejected"), FPinkCabCockpitSlotDefinition::ValidateUnique(Definitions));

    FPinkCabCockpitPresentationState Presentation;
    TestEqual(TEXT("presentation starts with neutral steering"), Presentation.Steering, 0.0f);
    TestFalse(TEXT("presentation does not invent meter availability"), Presentation.bMeterAvailable);
    return true;
}

#endif

