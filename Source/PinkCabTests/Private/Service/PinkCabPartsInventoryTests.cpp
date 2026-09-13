#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Service/PinkCabPartCatalog.h"
#include "Service/PinkCabServiceInventory.h"
#include "Service/PinkCabVehicleBuild.h"

static FPinkCabPartDefinition MakeBrakePart(const TCHAR* Id = TEXT("part.brake.street.v1"))
{
    FPinkCabPartDefinition Part;
    Part.PartId = Id;
    Part.SlotId = FName(TEXT("FrontBrake"));
    Part.PriceMinor = 1500;
    Part.CompatibilityTag = FName(TEXT("Tatra603"));
    return Part;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPinkCabPartCatalogTest,
    "PinkCab.Service.Parts.Catalog",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabPartCatalogTest::RunTest(const FString& Parameters)
{
    FPinkCabPartCatalog Catalog(2);
    const FPinkCabPartDefinition Part = MakeBrakePart();
    TestTrue(TEXT("valid part added"), Catalog.TryAdd(Part));
    TestFalse(TEXT("duplicate part rejected"), Catalog.TryAdd(Part));
    FPinkCabPartDefinition Found;
    TestTrue(TEXT("stable part resolves"), Catalog.TryGet(Part.PartId, Found));
    TestEqual(TEXT("slot preserved"), Found.SlotId, Part.SlotId);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPinkCabServiceInventoryTest,
    "PinkCab.Service.Parts.Inventory",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabServiceInventoryTest::RunTest(const FString& Parameters)
{
    FPinkCabServiceInventory Inventory(2);
    const FPinkCabStableId OpA(TEXT("op:purchase-a"));
    const FPinkCabStableId OpB(TEXT("op:purchase-b"));
    const FPinkCabStableId OpC(TEXT("op:purchase-c"));
    TestEqual(TEXT("first purchase applies"),
        Inventory.AddOwnedPartOnce(OpA, TEXT("part.a")), EPinkCabInventoryMutationResult::Applied);
    TestEqual(TEXT("replay duplicate"),
        Inventory.AddOwnedPartOnce(OpA, TEXT("part.a")), EPinkCabInventoryMutationResult::Duplicate);
    TestEqual(TEXT("second item applies"),
        Inventory.AddOwnedPartOnce(OpB, TEXT("part.b")), EPinkCabInventoryMutationResult::Applied);
    TestEqual(TEXT("capacity enforced"),
        Inventory.AddOwnedPartOnce(OpC, TEXT("part.c")), EPinkCabInventoryMutationResult::CapacityExceeded);
    TestEqual(TEXT("bounded quantity"), Inventory.GetTotalQuantity(), 2);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPinkCabCompatibleInstallTest,
    "PinkCab.Service.Parts.CompatibleInstall",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabCompatibleInstallTest::RunTest(const FString& Parameters)
{
    FPinkCabVehicleBuild Build;
    const FPinkCabPartDefinition Part = MakeBrakePart();
    const FPinkCabStableId Op(TEXT("op:install-a"));
    TestEqual(TEXT("compatible install applies"),
        Build.TryInstallPartOnce(Op, Part, FName(TEXT("Tatra603"))),
        EPinkCabPartInstallResult::Applied);
    TestEqual(TEXT("installed part visible"), Build.GetInstalledPartId(Part.SlotId), Part.PartId);
    TestEqual(TEXT("same operation replay is duplicate"),
        Build.TryInstallPartOnce(Op, Part, FName(TEXT("Tatra603"))),
        EPinkCabPartInstallResult::Duplicate);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPinkCabIncompatibleInstallTest,
    "PinkCab.Service.Parts.IncompatibleInstall",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabIncompatibleInstallTest::RunTest(const FString& Parameters)
{
    FPinkCabVehicleBuild Build;
    const FPinkCabPartDefinition Part = MakeBrakePart();
    TestEqual(TEXT("wrong vehicle tag rejected"),
        Build.TryInstallPartOnce(FPinkCabStableId(TEXT("op:wrong-tag")), Part, FName(TEXT("OtherCar"))),
        EPinkCabPartInstallResult::Incompatible);
    TestTrue(TEXT("rejection leaves slot empty"), Build.GetInstalledPartId(Part.SlotId).IsEmpty());

    FPinkCabPartDefinition WrongSlot = Part;
    WrongSlot.SlotId = NAME_None;
    TestEqual(TEXT("invalid slot rejected deterministically"),
        Build.TryInstallPartOnce(FPinkCabStableId(TEXT("op:wrong-slot")), WrongSlot, FName(TEXT("Tatra603"))),
        EPinkCabPartInstallResult::InvalidDefinition);
    return true;
}

#endif
