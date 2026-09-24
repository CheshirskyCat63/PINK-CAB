#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Engine/StaticMesh.h"
#include "PhysicsEngine/BodySetup.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabL1RoadR2ConstructionAssetContractTest,
    "PinkCab.World.L1Road.R2.AssetContract",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabL1RoadR2ConstructionAssetContractTest::RunTest(
    const FString& Parameters)
{
    UStaticMesh* Construction = LoadObject<UStaticMesh>(
        nullptr,
        TEXT("/Game/World/L1/Road/RoadConstruction.RoadConstruction"));

    TestNotNull(TEXT("R2 construction asset exists"), Construction);
    if (!Construction)
    {
        return false;
    }

    const FVector Size = Construction->GetBounds().GetBox().GetSize();
    TestTrue(TEXT("R2 construction remains one kilometre long"),
        FMath::IsNearlyEqual(Size.X, 100000.0, 10.0));
    TestTrue(TEXT("R2 construction remains inside frozen 66.8m envelope"),
        Size.Y <= 6680.0 + 2.0);
    TestTrue(TEXT("R2 construction reaches both outside road edges"),
        Size.Y >= 6670.0);
    TestTrue(TEXT("R2 raised construction has visible vertical depth"),
        Size.Z >= 11.5 && Size.Z <= 14.5);

    UBodySetup* BodySetup = Construction->GetBodySetup();
    TestNotNull(TEXT("R2 construction has collision body setup"), BodySetup);
    if (BodySetup)
    {
        TestEqual(TEXT("R2 construction uses exact geometry for curb collision"),
            BodySetup->CollisionTraceFlag,
            CTF_UseComplexAsSimple);
    }

    const TArray<FStaticMaterial>& Materials =
        Construction->GetStaticMaterials();
    TestTrue(TEXT("R2 construction exposes project material slots"),
        Materials.Num() >= 2);
    for (const FStaticMaterial& Slot : Materials)
    {
        TestNotNull(TEXT("R2 construction material assigned"),
            Slot.MaterialInterface.Get());
        if (Slot.MaterialInterface)
        {
            TestTrue(TEXT("R2 construction material is PINK-CAB owned"),
                Slot.MaterialInterface->GetPathName().StartsWith(
                    TEXT("/Game/World/L1/Road/Materials/")));
        }
    }

    TestTrue(TEXT("R2 construction has render data"),
        Construction->HasValidRenderData(true, 0));
    TestTrue(TEXT("R2 construction has enough geometry to represent curbs"),
        Construction->GetNumVertices(0) > 100);

    return true;
}

#endif
