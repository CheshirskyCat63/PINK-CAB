#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Core/PinkCabFeatureConfig.h"
#include "Core/PinkCabResult.h"
#include "Core/PinkCabSchemaVersion.h"
#include "Core/PinkCabStableId.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabStableIdContractTest,
    "PinkCab.Core.StableId",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabStableIdContractTest::RunTest(const FString& Parameters)
{
    const FPinkCabStableId First(TEXT("vehicle:tatra-603-hero"));
    const FPinkCabStableId Same(TEXT("vehicle:tatra-603-hero"));
    const FPinkCabStableId Other(TEXT("vehicle:traffic-001"));

    TestTrue(TEXT("same logical IDs compare equal"), First == Same);
    TestTrue(TEXT("different logical IDs compare unequal"), First != Other);
    TestEqual(TEXT("serialization is deterministic"), First.Serialize(), FString(TEXT("vehicle:tatra-603-hero")));

    FPinkCabStableId RoundTrip;
    TestTrue(TEXT("serialized stable ID parses"), FPinkCabStableId::TryParse(First.Serialize(), RoundTrip));
    TestTrue(TEXT("round-trip preserves equality"), First == RoundTrip);
    TestFalse(TEXT("empty stable ID is rejected"), FPinkCabStableId::TryParse(TEXT(""), RoundTrip));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabSchemaVersionContractTest,
    "PinkCab.Core.SchemaVersion",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabSchemaVersionContractTest::RunTest(const FString& Parameters)
{
    const FPinkCabSchemaVersion V100(1, 0, 0);
    const FPinkCabSchemaVersion V110(1, 1, 0);
    const FPinkCabSchemaVersion V200(2, 0, 0);

    TestTrue(TEXT("schema ordering works"), V100 < V110 && V110 < V200);
    TestEqual(TEXT("schema serialization"), V110.Serialize(), FString(TEXT("1.1.0")));

    FPinkCabSchemaVersion Parsed;
    TestTrue(TEXT("valid schema parses"), FPinkCabSchemaVersion::TryParse(TEXT("1.1.0"), Parsed));
    TestTrue(TEXT("schema round-trip preserves value"), Parsed == V110);
    TestFalse(TEXT("invalid schema text is rejected"), FPinkCabSchemaVersion::TryParse(TEXT("1.bad.0"), Parsed));
    TestFalse(TEXT("negative schema component is rejected"), FPinkCabSchemaVersion::TryParse(TEXT("1.-1.0"), Parsed));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabResultContractTest,
    "PinkCab.Core.Result",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabResultContractTest::RunTest(const FString& Parameters)
{
    const FPinkCabResult Ok = FPinkCabResult::Success();
    const FPinkCabResult Failed = FPinkCabResult::Failure(
        EPinkCabErrorCode::InvalidInput,
        TEXT("bad input"));

    TestTrue(TEXT("success is explicit"), Ok.IsSuccess());
    TestEqual(TEXT("success error code is None"), Ok.GetErrorCode(), EPinkCabErrorCode::None);
    TestFalse(TEXT("failure is explicit"), Failed.IsSuccess());
    TestEqual(TEXT("failure preserves error code"), Failed.GetErrorCode(), EPinkCabErrorCode::InvalidInput);
    TestEqual(TEXT("failure preserves message"), Failed.GetMessage(), FString(TEXT("bad input")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabFeatureConfigContractTest,
    "PinkCab.Core.FeatureConfig",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabFeatureConfigContractTest::RunTest(const FString& Parameters)
{
    FPinkCabFeatureConfig Config;
    Config.SetEnabled(EPinkCabFeature::L2, true);
    Config.SetEnabled(EPinkCabFeature::Damage, false);
    Config.SetEnabled(EPinkCabFeature::Neural, true);
    Config.SetEnabled(EPinkCabFeature::MovingFuel, false);
    Config.SetEnabled(EPinkCabFeature::ServiceNodes, true);

    TestTrue(TEXT("L2 flag reads from Core FeatureConfig"), Config.IsEnabled(EPinkCabFeature::L2));
    TestFalse(TEXT("Damage flag reads from Core FeatureConfig"), Config.IsEnabled(EPinkCabFeature::Damage));
    TestTrue(TEXT("Neural flag reads from Core FeatureConfig"), Config.IsEnabled(EPinkCabFeature::Neural));
    TestFalse(TEXT("MovingFuel flag reads from Core FeatureConfig"), Config.IsEnabled(EPinkCabFeature::MovingFuel));
    TestTrue(TEXT("ServiceNodes flag reads from Core FeatureConfig"), Config.IsEnabled(EPinkCabFeature::ServiceNodes));
    return true;
}

#endif
