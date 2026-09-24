#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/PackageName.h"

#include "Assets/RoadCurbProfile.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "PhysicsEngine/BodySetup.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabL1NativeMetaRoadR2AssetTest,
    "PinkCab.World.L1Road.R2.NativeMetaRoadAsset",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabL1NativeMetaRoadR2AssetTest::RunTest(const FString& Parameters)
{
    const FString PrimitiveOverlayPackage =
        TEXT("/Game/World/L1/Road/RoadConstruction");
    TestFalse(
        TEXT("primitive RoadConstruction overlay is absent"),
        FPackageName::DoesPackageExist(PrimitiveOverlayPackage));

    URoadCurbProfile* DefaultCurb = LoadObject<URoadCurbProfile>(
        nullptr,
        TEXT("/MetaRoad/MetaRoad/Profiles/Curbs/DefaultCurb.DefaultCurb"));
    TestNotNull(
        TEXT("MetaRoad human-authored DefaultCurb profile is available"),
        DefaultCurb);
    if (DefaultCurb)
    {
        TestTrue(
            TEXT("MetaRoad DefaultCurb has authored width"),
            DefaultCurb->Width > 0.0f);
        const FRichCurve* Curve =
            DefaultCurb->CurbCurve.GetRichCurveConst();
        TestTrue(
            TEXT("MetaRoad DefaultCurb has authored curve keys"),
            Curve != nullptr && Curve->GetNumKeys() >= 2);
    }

    UStaticMesh* Road = LoadObject<UStaticMesh>(
        nullptr,
        TEXT("/Game/World/L1/Road/RoadSurface.RoadSurface"));
    TestNotNull(TEXT("canonical MetaRoad RoadSurface loads"), Road);
    if (!Road)
    {
        return false;
    }

    const FVector Size = Road->GetBounds().GetBox().GetSize();
    AddInfo(FString::Printf(
        TEXT("CD869_R2_NATIVE_ROADSURFACE_BOUNDS=%.2f,%.2f,%.2f"),
        Size.X,
        Size.Y,
        Size.Z));

    TestTrue(
        TEXT("canonical road remains 1000m long"),
        FMath::IsNearlyEqual(Size.X, 100000.0, 250.0));
    TestTrue(
        TEXT("canonical road remains inside 66.8m accepted envelope"),
        FMath::IsNearlyEqual(Size.Y, 6680.0, 250.0));
    TestTrue(
        TEXT("canonical RoadSurface itself contains raised R2 relief"),
        Size.Z >= 11.0);

    TestNotNull(
        TEXT("native MetaRoad road has collision body setup"),
        Road->GetBodySetup());

    bool bHasCurbSlot = false;
    bool bHasRaisedSurfaceSlot = false;
    for (int32 Index = 0; Index < Road->GetStaticMaterials().Num(); ++Index)
    {
        const FStaticMaterial& Slot = Road->GetStaticMaterials()[Index];
        const FString SlotName = Slot.MaterialSlotName.ToString();
        AddInfo(FString::Printf(
            TEXT("CD869_R2_NATIVE_SLOT[%d]=%s"),
            Index,
            *SlotName));

        bHasCurbSlot |= SlotName.Contains(
            TEXT("Curb"), ESearchCase::IgnoreCase);
        bHasRaisedSurfaceSlot |=
            SlotName.Contains(TEXT("Median"), ESearchCase::IgnoreCase)
            || SlotName.Contains(TEXT("Sidewalk"), ESearchCase::IgnoreCase)
            || SlotName.Contains(TEXT("Shoulder"), ESearchCase::IgnoreCase);

        UMaterialInterface* Material = Slot.MaterialInterface;
        TestNotNull(TEXT("runtime road material assigned"), Material);
        if (Material)
        {
            TestTrue(
                TEXT("runtime road material remains project-owned"),
                Material->GetPathName().StartsWith(
                    TEXT("/Game/World/L1/Road/Materials/")));
        }
    }

    TestTrue(
        TEXT("canonical RoadSurface contains native MetaRoad curb layer"),
        bHasCurbSlot);
    TestTrue(
        TEXT("canonical RoadSurface contains native raised surface layer"),
        bHasRaisedSurfaceSlot);

    AddInfo(TEXT("CD869_R2_NATIVE_METAROAD_ASSET=PASS"));
    return true;
}

#endif
