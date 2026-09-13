#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Taxi/PinkCabFarePassengerManifest.h"
#include "Taxi/PinkCabFarePassengerPresentation.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabFarePassengerPresentationLayoutTest,
    "PinkCab.Taxi.FareLoop.Presentation.InstanceLayout",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabFarePassengerPresentationLayoutTest::RunTest(const FString& Parameters)
{
    const TArray<FPinkCabFarePassengerInput> Inputs = {
        {FPinkCabStableId(TEXT("p1")), 60.0f},
        {FPinkCabStableId(TEXT("p2")), 61.0f},
        {FPinkCabStableId(TEXT("p3")), 62.0f},
        {FPinkCabStableId(TEXT("p4")), 63.0f},
        {FPinkCabStableId(TEXT("p5")), 64.0f}};
    FPinkCabFarePassengerManifest Manifest;
    TestTrue(TEXT("five-passenger manifest"),
        FPinkCabFarePassengerManifest::TryCreate(FPinkCabStableId(TEXT("fare-present")), Inputs, Manifest));

    FPinkCabPassengerPlaceholderLayout Layout;
    Layout.SpacingCm = 60.0f;
    Layout.HeightCm = 90.0f;
    FPinkCabPassengerPlaceholderTransforms Transforms;
    TestTrue(TEXT("layout builds"),
        FPinkCabFarePassengerPresentation::BuildLocalTransforms(Manifest, Layout, Transforms));
    TestEqual(TEXT("five transforms"), Transforms.Num(), 5);
    TestEqual(TEXT("first passenger centered left"), Transforms[0].GetLocation().Y, -120.0);
    TestEqual(TEXT("last passenger centered right"), Transforms[4].GetLocation().Y, 120.0);
    TestEqual(TEXT("placeholder height"), Transforms[2].GetLocation().Z, 90.0);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabFarePassengerPresentationSyncTest,
    "PinkCab.Taxi.FareLoop.Presentation.SingleISM",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabFarePassengerPresentationSyncTest::RunTest(const FString& Parameters)
{
    const TArray<FPinkCabFarePassengerInput> Inputs = {
        {FPinkCabStableId(TEXT("p1")), 60.0f},
        {FPinkCabStableId(TEXT("p2")), 70.0f},
        {FPinkCabStableId(TEXT("p3")), 80.0f}};
    FPinkCabFarePassengerManifest Manifest;
    TestTrue(TEXT("manifest builds"),
        FPinkCabFarePassengerManifest::TryCreate(FPinkCabStableId(TEXT("fare-ism")), Inputs, Manifest));

    UInstancedStaticMeshComponent* ISM = NewObject<UInstancedStaticMeshComponent>();
    TestNotNull(TEXT("single ISM component exists"), ISM);
    const int32 Added = FPinkCabFarePassengerPresentation::SyncInstances(*ISM, Manifest, {});
    TestEqual(TEXT("three instances added"), Added, 3);
    TestEqual(TEXT("component contains exactly three passenger instances"), ISM->GetInstanceCount(), 3);

    const int32 Resynced = FPinkCabFarePassengerPresentation::SyncInstances(*ISM, Manifest, {});
    TestEqual(TEXT("resync still reports three"), Resynced, 3);
    TestEqual(TEXT("resync replaces instead of accumulating"), ISM->GetInstanceCount(), 3);

    FPinkCabFarePassengerPresentation::Clear(*ISM);
    TestEqual(TEXT("clear removes presentation only"), ISM->GetInstanceCount(), 0);
    TestEqual(TEXT("manifest gameplay state remains intact"), Manifest.GetRecords().Num(), 3);
    return true;
}

#endif
