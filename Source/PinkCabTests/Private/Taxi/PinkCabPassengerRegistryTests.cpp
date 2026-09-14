#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Taxi/PinkCabPassengerRecord.h"
#include "Taxi/PinkCabPassengerTemplate.h"

static FPinkCabPassengerTemplate MakeCommuterTemplate()
{
    return FPinkCabPassengerTemplate(FName(TEXT("Commuter")), 72.0f);
}

static TArray<FName> MakePreferences()
{
    return {FName(TEXT("quiet")), FName(TEXT("fast")), FName(TEXT("quiet"))};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPassengerRegistryCreateTest,
    "PinkCab.Taxi.PassengerRegistry.Create",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabPassengerRegistryCreateTest::RunTest(const FString& Parameters)
{
    FPinkCabPassengerRegistry Registry(4, 4);
    const FPinkCabStableId Id(TEXT("passenger:canonical:1"));
    FPinkCabPassengerRecord* Record = nullptr;
    TestTrue(TEXT("record created"), Registry.TryCreate(
        Id, MakeCommuterTemplate(), TEXT("city:first-euro:block-a"), MakePreferences(), Record));
    TestNotNull(TEXT("record returned"), Record);
    TestEqual(TEXT("stable identity"), Record->IdentityId.Serialize(), Id.Serialize());
    TestEqual(TEXT("template retained"), Record->TemplateId, FName(TEXT("Commuter")));
    TestTrue(TEXT("seed non-zero"), Record->IdentitySeed != 0);
    TestEqual(TEXT("resolved mass from template"), Record->ResolvedMassKg, 72.0f);
    TestEqual(TEXT("preferences unique and bounded"), Record->PreferenceTags.Num(), 2);
    TestEqual(TEXT("preferences sorted"), Record->PreferenceTags[0], FName(TEXT("fast")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPassengerRegistryValidationTest,
    "PinkCab.Taxi.PassengerRegistry.Validation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabPassengerRegistryValidationTest::RunTest(const FString& Parameters)
{
    FPinkCabPassengerRegistry Registry(2, 2);
    FPinkCabPassengerRecord* Record = nullptr;
    TestFalse(TEXT("invalid id rejected"), Registry.TryCreate(
        FPinkCabStableId(), MakeCommuterTemplate(), TEXT("ctx"), {}, Record));
    TestFalse(TEXT("empty context rejected"), Registry.TryCreate(
        FPinkCabStableId(TEXT("p:1")), MakeCommuterTemplate(), TEXT("  "), {}, Record));
    FPinkCabPassengerTemplate MissingTemplate;
    TestFalse(TEXT("missing template rejected"), Registry.TryCreate(
        FPinkCabStableId(TEXT("p:2")), MissingTemplate, TEXT("ctx"), {}, Record));
    FPinkCabPassengerTemplate ZeroMass(FName(TEXT("Broken")), 0.0f);
    TestFalse(TEXT("zero mass rejected"), Registry.TryCreate(
        FPinkCabStableId(TEXT("p:3")), ZeroMass, TEXT("ctx"), {}, Record));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPassengerRegistryDuplicateCapacityTest,
    "PinkCab.Taxi.PassengerRegistry.DuplicateCapacity",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabPassengerRegistryDuplicateCapacityTest::RunTest(const FString& Parameters)
{
    FPinkCabPassengerRegistry Registry(2, 4);
    FPinkCabPassengerRecord* A = nullptr;
    FPinkCabPassengerRecord* B = nullptr;
    TestTrue(TEXT("first"), Registry.TryCreate(FPinkCabStableId(TEXT("p:a")), MakeCommuterTemplate(), TEXT("ctx:a"), {}, A));
    TestFalse(TEXT("duplicate rejected"), Registry.TryCreate(FPinkCabStableId(TEXT("p:a")), MakeCommuterTemplate(), TEXT("ctx:b"), {}, B));
    TestEqual(TEXT("duplicate leaves one record"), Registry.Num(), 1);
    TestTrue(TEXT("second"), Registry.TryCreate(FPinkCabStableId(TEXT("p:b")), MakeCommuterTemplate(), TEXT("ctx:b"), {}, B));
    TestFalse(TEXT("capacity enforced"), Registry.TryCreate(FPinkCabStableId(TEXT("p:c")), MakeCommuterTemplate(), TEXT("ctx:c"), {}, B));
    TestEqual(TEXT("capacity leaves two records"), Registry.Num(), 2);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPassengerRegistryDeterminismTest,
    "PinkCab.Taxi.PassengerRegistry.Determinism",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabPassengerRegistryDeterminismTest::RunTest(const FString& Parameters)
{
    FPinkCabPassengerRegistry A(4, 3);
    FPinkCabPassengerRegistry B(4, 3);
    FPinkCabPassengerRecord* RA = nullptr;
    FPinkCabPassengerRecord* RB = nullptr;
    const FPinkCabStableId Id(TEXT("p:deterministic"));
    const auto Template = MakeCommuterTemplate();
    const TArray<FName> Prefs = {FName(TEXT("window")), FName(TEXT("quiet"))};
    TestTrue(TEXT("A create"), A.TryCreate(Id, Template, TEXT("district:42"), Prefs, RA));
    TestTrue(TEXT("B create"), B.TryCreate(Id, Template, TEXT("district:42"), Prefs, RB));
    TestEqual(TEXT("same seed"), RA->IdentitySeed, RB->IdentitySeed);
    TestEqual(TEXT("same mass"), RA->ResolvedMassKg, RB->ResolvedMassKg);
    TestEqual(TEXT("same signature"), A.GetReconstructionSignature(), B.GetReconstructionSignature());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPassengerRegistryPreferenceCapTest,
    "PinkCab.Taxi.PassengerRegistry.PreferenceCap",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabPassengerRegistryPreferenceCapTest::RunTest(const FString& Parameters)
{
    FPinkCabPassengerRegistry Registry(2, 2);
    FPinkCabPassengerRecord* Record = nullptr;
    const TArray<FName> TooMany = {FName(TEXT("a")), FName(TEXT("b")), FName(TEXT("c"))};
    TestFalse(TEXT("preference overflow rejected"), Registry.TryCreate(
        FPinkCabStableId(TEXT("p:prefs")), MakeCommuterTemplate(), TEXT("ctx"), TooMany, Record));
    TestEqual(TEXT("failed create does not mutate registry"), Registry.Num(), 0);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPassengerRegistryAppearancePersistenceTest,
    "PinkCab.Taxi.PassengerRegistry.AppearancePersistence",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabPassengerRegistryAppearancePersistenceTest::RunTest(const FString& Parameters)
{
    FPinkCabPassengerRegistry A(4, 4), B(4, 4);
    FPinkCabPassengerRecord* RA = nullptr; FPinkCabPassengerRecord* RB = nullptr;
    const FPinkCabStableId Id(TEXT("p:appearance:1"));
    TestTrue(TEXT("A create"), A.TryCreate(Id, MakeCommuterTemplate(), TEXT("district:appearance"), {}, RA));
    TestTrue(TEXT("B recreate"), B.TryCreate(Id, MakeCommuterTemplate(), TEXT("district:appearance"), {}, RB));
    TestTrue(TEXT("appearance seed stable"), RA->AppearanceSeed != 0 && RA->AppearanceSeed == RB->AppearanceSeed);
    TestEqual(TEXT("appearance profile stable"), RA->AppearanceProfileId, RB->AppearanceProfileId);
    TestEqual(TEXT("appearance traits stable"), RA->AppearanceTraitIds, RB->AppearanceTraitIds);
    TestTrue(TEXT("appearance traits present"), RA->AppearanceTraitIds.Num() >= 3);
    return true;
}
#endif
