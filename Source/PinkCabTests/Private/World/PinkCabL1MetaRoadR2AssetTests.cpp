#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/PackageName.h"

#include "Assets/RoadCurbProfile.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "PhysicsEngine/BodySetup.h"
#include "Components/StaticMeshComponent.h"
#include "World/PinkCabL1RoadChunkActor.h"

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
        TEXT("RoadCurbs7")
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
        TEXT("eight native MetaRoad curb meshes are present"),
        CurbMeshCount,
        8);
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
    if (RuntimeChunk)
    {
        TArray<UStaticMeshComponent*> RuntimeComponents;
        RuntimeChunk->GetComponents<UStaticMeshComponent>(
            RuntimeComponents);

        for (UStaticMeshComponent* Component : RuntimeComponents)
        {
            if (!Component || !Component->GetStaticMesh())
            {
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
        TEXT("runtime assembles surface + sidewalks + eight native curbs"),
        RuntimeMeshComponentCount,
        10);
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
            TEXT("runtime native R2 road preserves the accepted 66.8m envelope"),
            FMath::IsNearlyEqual(RuntimeSize.Y, 6680.0, 5.0));
        TestTrue(
            TEXT("runtime native R2 construction preserves MetaRoad vertical relief"),
            FMath::IsNearlyEqual(RuntimeSize.Z, 16.5, 0.5));
    }

    AddInfo(TEXT("CD869_R2_NATIVE_METAROAD_ASSET=PASS"));
    return true;
}

#endif
