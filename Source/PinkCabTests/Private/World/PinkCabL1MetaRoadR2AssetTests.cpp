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

    const TCHAR* NativeMeshNames[] = {
        TEXT("RoadSurface"),
        TEXT("RoadSidewalks"),
        TEXT("RoadCurbs"),
        TEXT("RoadCurbs1"),
        TEXT("RoadCurbs2"),
        TEXT("RoadCurbs3"),
        TEXT("RoadCurbs4"),
        TEXT("RoadCurbs5"),
        TEXT("RoadCurbs6"),
        TEXT("RoadCurbs7"),
        TEXT("RoadCurbs8"),
        TEXT("RoadCurbs9")
    };

    FBox CombinedBounds(ForceInit);
    int32 LoadedMeshCount = 0;
    int32 CurbMeshCount = 0;
    bool bRaisedSurfacePresent = false;

    for (const TCHAR* MeshName : NativeMeshNames)
    {
        const FString ObjectPath = FString::Printf(
            TEXT("/Game/World/L1/Road/%s.%s"),
            MeshName,
            MeshName);
        UStaticMesh* Mesh = LoadObject<UStaticMesh>(
            nullptr,
            *ObjectPath);
        TestNotNull(
            *FString::Printf(
                TEXT("native MetaRoad R2 mesh loads: %s"),
                MeshName),
            Mesh);
        if (!Mesh)
        {
            continue;
        }

        ++LoadedMeshCount;
        const FBox LocalBox = Mesh->GetBounds().GetBox();
        CombinedBounds += LocalBox;
        const FVector Size = LocalBox.GetSize();

        AddInfo(FString::Printf(
            TEXT("CD869_R2_NATIVE_MESH[%s]=%.2f,%.2f,%.2f"),
            MeshName,
            Size.X,
            Size.Y,
            Size.Z));

        TestNotNull(
            *FString::Printf(
                TEXT("%s has collision body setup"),
                MeshName),
            Mesh->GetBodySetup());

        const FString Name(MeshName);
        if (Name == TEXT("RoadSidewalks"))
        {
            bRaisedSurfacePresent = Size.Z >= 11.0;
        }
        if (Name.StartsWith(TEXT("RoadCurbs")))
        {
            ++CurbMeshCount;
            TestTrue(
                *FString::Printf(
                    TEXT("%s has physical curb relief"),
                    MeshName),
                Size.Z > 0.0);
        }

        for (int32 Index = 0; Index < Mesh->GetStaticMaterials().Num(); ++Index)
        {
            UMaterialInterface* Material =
                Mesh->GetStaticMaterials()[Index].MaterialInterface;
            TestNotNull(
                *FString::Printf(
                    TEXT("%s runtime material assigned"),
                    MeshName),
                Material);
            if (Material)
            {
                TestTrue(
                    *FString::Printf(
                        TEXT("%s runtime material is project-owned"),
                        MeshName),
                    Material->GetPathName().StartsWith(
                        TEXT("/Game/World/L1/Road/Materials/")));
            }
        }
    }

    TestEqual(
        TEXT("all native MetaRoad R2 mesh layers are present"),
        LoadedMeshCount,
        static_cast<int32>(UE_ARRAY_COUNT(NativeMeshNames)));
    TestEqual(
        TEXT("ten native MetaRoad curb meshes are present"),
        CurbMeshCount,
        10);
    TestTrue(
        TEXT("native MetaRoad sidewalk layer contains raised construction"),
        bRaisedSurfacePresent);

    TestTrue(TEXT("combined native R2 bounds are valid"), CombinedBounds.IsValid);
    if (CombinedBounds.IsValid)
    {
        const FVector CombinedSize = CombinedBounds.GetSize();
        AddInfo(FString::Printf(
            TEXT("CD869_R2_NATIVE_COMBINED_BOUNDS=%.2f,%.2f,%.2f"),
            CombinedSize.X,
            CombinedSize.Y,
            CombinedSize.Z));
        TestTrue(
            TEXT("combined native R2 road remains 1000m long"),
            FMath::IsNearlyEqual(CombinedSize.X, 100000.0, 250.0));
        TestTrue(
            TEXT("combined native R2 road preserves 66.8m envelope"),
            FMath::IsNearlyEqual(CombinedSize.Y, 6680.0, 10.0));
        TestTrue(
            TEXT("combined native R2 construction has real vertical relief"),
            CombinedSize.Z >= 11.0);
    }

    AddInfo(TEXT("CD869_R2_NATIVE_METAROAD_ASSET=PASS"));
    return true;
}

#endif
