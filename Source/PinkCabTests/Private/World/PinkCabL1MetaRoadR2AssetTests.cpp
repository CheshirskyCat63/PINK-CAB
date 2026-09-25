#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/PackageName.h"

#include "Assets/RoadCurbProfile.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "PhysicsEngine/BodySetup.h"
#include "Components/StaticMeshComponent.h"
#include "World/PinkCabL1RoadChunkActor.h"
#include "World/PinkCabL1EndlessRoadModel.h"

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
        TEXT("RoadCurbs9"),
        TEXT("RoadCurbs10"),
        TEXT("RoadCurbs11"),
        TEXT("RoadCurbs12"),
        TEXT("RoadCurbs13"),
        TEXT("RoadCurbs14"),
        TEXT("RoadCurbs15"),
        TEXT("RoadCurbs16"),
        TEXT("RoadCurbs17"),
        TEXT("RoadCurbs18"),
        TEXT("RoadCurbs19"),
        TEXT("RoadCurbs20"),
        TEXT("RoadCurbs21"),
        TEXT("RoadCurbs22"),
        TEXT("RoadCurbs23"),
        TEXT("RoadCurbs24"),
        TEXT("RoadCurbs25"),
        TEXT("RoadCurbs26"),
        TEXT("RoadCurbs27"),
        TEXT("RoadCurbs28"),
        TEXT("RoadCurbs29"),
        TEXT("RoadCurbs30"),
        TEXT("RoadCurbs31")
    };

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
            bRaisedSurfacePresent =
                FMath::IsNearlyEqual(
                    Mesh->GetBounds().Origin.Z,
                    12.0,
                    0.25);
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
                const FString Name(MeshName);
                const FString ExpectedPath =
                    Name == TEXT("RoadSurface")
                        ? TEXT("/MetaRoad/MetaRoad/Materials/MI_DriveSurface.MI_DriveSurface")
                        : Name == TEXT("RoadSidewalks")
                            ? TEXT("/MetaRoad/MetaRoad/Materials/M_Sidewolk.M_Sidewolk")
                            : TEXT("/MetaRoad/MetaRoad/Materials/M_Curb.M_Curb");
                TestEqual(
                    *FString::Printf(
                        TEXT("%s runtime material stays native MetaRoad"),
                        MeshName),
                    Material->GetPathName(),
                    ExpectedPath);
            }
        }
    }

    TestEqual(
        TEXT("all native MetaRoad R2 mesh layers are present"),
        LoadedMeshCount,
        static_cast<int32>(UE_ARRAY_COUNT(NativeMeshNames)));
    TestEqual(
        TEXT("thirty-two access-aware native MetaRoad curb spans are present"),
        CurbMeshCount,
        32);
    TestTrue(
        TEXT("native MetaRoad sidewalk layer contains raised construction"),
        bRaisedSurfacePresent);

    const APinkCabL1RoadChunkActor* RuntimeChunk =
        GetDefault<APinkCabL1RoadChunkActor>();
    TestNotNull(
        TEXT("runtime chunk CDO is available for native MetaRoad assembly"),
        RuntimeChunk);

    FBox RuntimeAssemblyBounds(ForceInit);
    int32 RuntimeMeshComponentCount = 0;
    TArray<UStaticMeshComponent*> RuntimeComponents;
    if (RuntimeChunk)
    {
        RuntimeChunk->GetComponents<UStaticMeshComponent>(
            RuntimeComponents);

        for (UStaticMeshComponent* Component : RuntimeComponents)
        {
            if (!Component || !Component->GetStaticMesh())
            {
                continue;
            }

            // R3 is additive visual-only state. Keep the frozen R2
            // construction count/bounds independent from RoadMarks.
            if (Component->GetStaticMesh()->GetPathName().StartsWith(
                    TEXT("/Game/World/L1/Road/RoadMarks")))
            {
                TestEqual(
                    TEXT("R3 RoadMarks remains collision-free inside R2 regression"),
                    Component->GetCollisionEnabled(),
                    ECollisionEnabled::NoCollision);
                continue;
            }

            ++RuntimeMeshComponentCount;
            const FBox MeshBox =
                Component->GetStaticMesh()->GetBounds().GetBox();
            const FBox PlacedBox =
                MeshBox.TransformBy(Component->GetRelativeTransform());
            RuntimeAssemblyBounds += PlacedBox;

            AddInfo(FString::Printf(
                TEXT("CD869_R2_RUNTIME_COMPONENT[%s] rel=(%.2f,%.2f,%.2f) placed_origin=(%.2f,%.2f,%.2f) placed_extent=(%.2f,%.2f,%.2f)"),
                *Component->GetName(),
                Component->GetRelativeLocation().X,
                Component->GetRelativeLocation().Y,
                Component->GetRelativeLocation().Z,
                PlacedBox.GetCenter().X,
                PlacedBox.GetCenter().Y,
                PlacedBox.GetCenter().Z,
                PlacedBox.GetExtent().X,
                PlacedBox.GetExtent().Y,
                PlacedBox.GetExtent().Z));
        }
    }

    TestEqual(
        TEXT("runtime assembles surface + sidewalks + thirty-two native curb spans"),
        RuntimeMeshComponentCount,
        34);
    TestTrue(
        TEXT("runtime native MetaRoad assembly bounds are valid"),
        RuntimeAssemblyBounds.IsValid != 0);

    if (RuntimeAssemblyBounds.IsValid)
    {
        const FVector RuntimeSize = RuntimeAssemblyBounds.GetSize();
        AddInfo(FString::Printf(
            TEXT("CD869_R2_RUNTIME_ASSEMBLY_BOUNDS=%.2f,%.2f,%.2f"),
            RuntimeSize.X,
            RuntimeSize.Y,
            RuntimeSize.Z));
        TestTrue(
            TEXT("runtime native R2 road remains exactly one 1000m module"),
            FMath::IsNearlyEqual(RuntimeSize.X, 100000.0, 5.0));
        TestTrue(
            TEXT("runtime native R2 assembly includes the 7.5cm outer curb profile beyond the 79.0m design envelope"),
            FMath::IsNearlyEqual(RuntimeSize.Y, 7915.0, 5.0));
        TestTrue(
            TEXT("runtime native R2 construction preserves MetaRoad vertical relief"),
            FMath::IsNearlyEqual(RuntimeSize.Z, 16.5, 0.5));
    }

    // Regression for the owner-found R2 blocker: at the two fully-open
    // connector centres (300m / 700m), no curb span may occupy the
    // express/local service band around |Y| = 2650..3050cm after the
    // owner-requested +25% lane-width calibration.
    const double AccessCenters[] = {
        0.5 * (
            FPinkCabL1EndlessRoadModel::AccessAFullOpenStartCm +
            FPinkCabL1EndlessRoadModel::AccessAFullOpenEndCm),
        0.5 * (
            FPinkCabL1EndlessRoadModel::AccessBFullOpenStartCm +
            FPinkCabL1EndlessRoadModel::AccessBFullOpenEndCm)
    };
    for (const double AccessX : AccessCenters)
    {
        for (UStaticMeshComponent* Component : RuntimeComponents)
        {
            if (!Component || !Component->GetStaticMesh() ||
                !Component->GetName().StartsWith(TEXT("NativeRoadCurbs")))
            {
                continue;
            }

            const FBox PlacedBox =
                Component->GetStaticMesh()->GetBounds().GetBox().TransformBy(
                    Component->GetRelativeTransform());
            const bool bCrossesAccessX =
                AccessX >= PlacedBox.Min.X - 1.0 &&
                AccessX <= PlacedBox.Max.X + 1.0;
            const double AbsCenterY = FMath::Abs(PlacedBox.GetCenter().Y);
            const bool bServiceBand =
                AbsCenterY >= 2600.0 && AbsCenterY <= 3100.0;
            TestFalse(
                *FString::Printf(
                    TEXT("no service curb blocks access at X=%.0f: %s"),
                    AccessX,
                    *Component->GetName()),
                bCrossesAccessX && bServiceBand);
        }
    }
    AddInfo(TEXT("CD869_R2_ACCESS_OPENINGS=PASS"));

    AddInfo(TEXT("CD869_R2_NATIVE_METAROAD_ASSET=PASS"));
    return true;
}

#endif
