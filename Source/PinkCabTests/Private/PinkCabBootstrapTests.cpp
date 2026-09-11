#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/StructOnScope.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabProjectIdentityTest,
    "PinkCab.Bootstrap.ProjectIdentity",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabProjectIdentityTest::RunTest(const FString& Parameters)
{
    UScriptStruct* IdentityStruct = FindObject<UScriptStruct>(nullptr, TEXT("/Script/PinkCab.PinkCabBuildIdentity"));
    TestNotNull(TEXT("PinkCabBuildIdentity reflected struct must exist"), IdentityStruct);

    if (!IdentityStruct)
    {
        return false;
    }

    FStructOnScope Scope(IdentityStruct);
    const FStrProperty* ProductNameProperty = FindFProperty<FStrProperty>(IdentityStruct, TEXT("ProductName"));
    const FStrProperty* SchemaVersionProperty = FindFProperty<FStrProperty>(IdentityStruct, TEXT("SchemaVersion"));
    const FStrProperty* BuildChannelProperty = FindFProperty<FStrProperty>(IdentityStruct, TEXT("BuildChannel"));
    const FStrProperty* RuntimeVersionProperty = FindFProperty<FStrProperty>(IdentityStruct, TEXT("RuntimeVersion"));
    TestNotNull(TEXT("ProductName property must exist"), ProductNameProperty);
    TestNotNull(TEXT("SchemaVersion property must exist"), SchemaVersionProperty);
    TestNotNull(TEXT("BuildChannel property must exist"), BuildChannelProperty);
    TestNotNull(TEXT("RuntimeVersion property must exist"), RuntimeVersionProperty);

    if (!ProductNameProperty || !SchemaVersionProperty || !BuildChannelProperty || !RuntimeVersionProperty)
    {
        return false;
    }

    const FString ProductName = ProductNameProperty->GetPropertyValue_InContainer(Scope.GetStructMemory());
    const FString SchemaVersion = SchemaVersionProperty->GetPropertyValue_InContainer(Scope.GetStructMemory());
    const FString BuildChannel = BuildChannelProperty->GetPropertyValue_InContainer(Scope.GetStructMemory());
    const FString RuntimeVersion = RuntimeVersionProperty->GetPropertyValue_InContainer(Scope.GetStructMemory());
    TestEqual(TEXT("Product identity"), ProductName, FString(TEXT("PINK CAB")));
    TestTrue(TEXT("Schema version must be non-empty"), !SchemaVersion.IsEmpty());
    TestTrue(TEXT("Build channel must be non-empty"), !BuildChannel.IsEmpty());
    TestTrue(TEXT("Runtime version must be non-empty"), !RuntimeVersion.IsEmpty());
    return true;
}

#endif
