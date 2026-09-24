#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Engine/StaticMesh.h"
#include "PhysicsEngine/BodySetup.h"
#include "StaticMeshResources.h"

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

    const FStaticMeshRenderData* RenderData = Construction->GetRenderData();
    TestNotNull(TEXT("R2 construction exposes render data for topology audit"),
        RenderData);
    if (RenderData && RenderData->LODResources.Num() > 0)
    {
        const FStaticMeshLODResources& LOD = RenderData->LODResources[0];
        const FIndexArrayView Indices = LOD.IndexBuffer.GetArrayView();

        int32 ReversedTriangles = 0;
        int32 AuditedTriangles = 0;
        TSet<FString> TrianglePositionKeys;
        int32 DuplicatePositionTriangles = 0;

        auto QuantizedVertexKey = [](const FVector3f& Position)
        {
            return FString::Printf(
                TEXT("%lld,%lld,%lld"),
                static_cast<long long>(FMath::RoundToInt64(Position.X * 10.0)),
                static_cast<long long>(FMath::RoundToInt64(Position.Y * 10.0)),
                static_cast<long long>(FMath::RoundToInt64(Position.Z * 10.0)));
        };

        for (int32 Triangle = 0;
             Triangle + 2 < Indices.Num();
             Triangle += 3)
        {
            const uint32 I0 = Indices[Triangle];
            const uint32 I1 = Indices[Triangle + 1];
            const uint32 I2 = Indices[Triangle + 2];

            const FVector3f P0 = LOD.VertexBuffers.PositionVertexBuffer.VertexPosition(I0);
            const FVector3f P1 = LOD.VertexBuffers.PositionVertexBuffer.VertexPosition(I1);
            const FVector3f P2 = LOD.VertexBuffers.PositionVertexBuffer.VertexPosition(I2);

            const FVector3f GeometricNormal =
                FVector3f::CrossProduct(P1 - P0, P2 - P0).GetSafeNormal();
            const FVector3f AuthoredNormal =
                (LOD.VertexBuffers.StaticMeshVertexBuffer.VertexTangentZ(I0)
                 + LOD.VertexBuffers.StaticMeshVertexBuffer.VertexTangentZ(I1)
                 + LOD.VertexBuffers.StaticMeshVertexBuffer.VertexTangentZ(I2))
                    .GetSafeNormal();

            if (!GeometricNormal.IsNearlyZero()
                && !AuthoredNormal.IsNearlyZero())
            {
                ++AuditedTriangles;
                if (FVector3f::DotProduct(
                        GeometricNormal,
                        AuthoredNormal) < 0.0f)
                {
                    ++ReversedTriangles;
                }
            }

            TArray<FString> VertexKeys = {
                QuantizedVertexKey(P0),
                QuantizedVertexKey(P1),
                QuantizedVertexKey(P2)};
            VertexKeys.Sort();
            const FString TriangleKey = FString::Join(VertexKeys, TEXT("|"));
            if (TrianglePositionKeys.Contains(TriangleKey))
            {
                ++DuplicatePositionTriangles;
            }
            else
            {
                TrianglePositionKeys.Add(TriangleKey);
            }
        }

        TestTrue(TEXT("R2 topology audit inspects rendered triangles"),
            AuditedTriangles > 0);
        TestEqual(TEXT("R2 triangle winding agrees with authored normals"),
            ReversedTriangles, 0);
        TestEqual(TEXT("R2 mesh contains no coincident duplicate triangles"),
            DuplicatePositionTriangles, 0);

        AddInfo(FString::Printf(
            TEXT("CD869_R2_TOPOLOGY_AUDIT triangles=%d reversed=%d duplicates=%d"),
            AuditedTriangles,
            ReversedTriangles,
            DuplicatePositionTriangles));
    }

    return true;
}

#endif
