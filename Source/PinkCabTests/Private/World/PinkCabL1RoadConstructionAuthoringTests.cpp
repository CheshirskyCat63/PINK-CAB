#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Algo/Reverse.h"
#include "Misc/PackageName.h"
#include "UObject/SavePackage.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/StaticMesh.h"
#include "MaterialEditingLibrary.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionConstant3Vector.h"
#include "PhysicsEngine/BodySetup.h"
#include "StaticMeshDescription.h"

#include "World/PinkCabL1RoadConstructionModel.h"

namespace PinkCabL1RoadConstructionAuthoring
{
const TCHAR* ConstructionPackage =
    TEXT("/Game/World/L1/Road/RoadConstruction");
const TCHAR* ConstructionAsset = TEXT("RoadConstruction");
const TCHAR* DividerMaterialPath =
    TEXT("/Game/World/L1/Road/Materials/M_PC_L1_GreenDivider.M_PC_L1_GreenDivider");
const TCHAR* CurbMaterialPackage =
    TEXT("/Game/World/L1/Road/Materials/M_PC_L1_CurbConcrete");
const TCHAR* CurbMaterialAsset = TEXT("M_PC_L1_CurbConcrete");

bool SaveAssetPackage(UObject& Asset, FAutomationTestBase& Test)
{
    UPackage* Package = Asset.GetOutermost();
    Test.TestNotNull(TEXT("R2 authored asset has package"), Package);
    if (!Package)
    {
        return false;
    }

    const FString Filename = FPackageName::LongPackageNameToFilename(
        Package->GetName(),
        FPackageName::GetAssetPackageExtension());
    Package->MarkPackageDirty();

    FSavePackageArgs SaveArgs;
    SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
    SaveArgs.SaveFlags = SAVE_NoError;
    return UPackage::SavePackage(Package, &Asset, *Filename, SaveArgs);
}

UMaterial* CreateOrLoadCurbMaterial(FAutomationTestBase& Test)
{
    const FString ObjectPath = FString::Printf(
        TEXT("%s.%s"), CurbMaterialPackage, CurbMaterialAsset);
    if (UMaterial* Existing = LoadObject<UMaterial>(nullptr, *ObjectPath))
    {
        return Existing;
    }

    UPackage* Package = CreatePackage(CurbMaterialPackage);
    Test.TestNotNull(TEXT("R2 curb material package created"), Package);
    if (!Package)
    {
        return nullptr;
    }

    UMaterial* Material = NewObject<UMaterial>(
        Package,
        FName(CurbMaterialAsset),
        RF_Public | RF_Standalone | RF_Transactional);
    Test.TestNotNull(TEXT("R2 curb material created"), Material);
    if (!Material)
    {
        return nullptr;
    }

    UMaterialExpressionConstant3Vector* Color =
        Cast<UMaterialExpressionConstant3Vector>(
            UMaterialEditingLibrary::CreateMaterialExpression(
                Material,
                UMaterialExpressionConstant3Vector::StaticClass(),
                -400,
                -80));
    UMaterialExpressionConstant* Roughness =
        Cast<UMaterialExpressionConstant>(
            UMaterialEditingLibrary::CreateMaterialExpression(
                Material,
                UMaterialExpressionConstant::StaticClass(),
                -400,
                20));
    UMaterialExpressionConstant* Specular =
        Cast<UMaterialExpressionConstant>(
            UMaterialEditingLibrary::CreateMaterialExpression(
                Material,
                UMaterialExpressionConstant::StaticClass(),
                -400,
                120));

    Test.TestNotNull(TEXT("R2 curb color node created"), Color);
    Test.TestNotNull(TEXT("R2 curb roughness node created"), Roughness);
    Test.TestNotNull(TEXT("R2 curb specular node created"), Specular);
    if (!Color || !Roughness || !Specular)
    {
        return nullptr;
    }

    Color->Constant = FLinearColor(0.18f, 0.17f, 0.15f, 1.0f);
    Roughness->R = 0.92f;
    Specular->R = 0.06f;

    UMaterialEditingLibrary::ConnectMaterialProperty(
        Color, FString(), MP_BaseColor);
    UMaterialEditingLibrary::ConnectMaterialProperty(
        Roughness, FString(), MP_Roughness);
    UMaterialEditingLibrary::ConnectMaterialProperty(
        Specular, FString(), MP_Specular);
    UMaterialEditingLibrary::RecompileMaterial(Material);

    FAssetRegistryModule::AssetCreated(Material);
    Material->PostEditChange();
    Test.TestTrue(TEXT("R2 curb material saved"),
        SaveAssetPackage(*Material, Test));
    return Material;
}

void AddFace(
    UStaticMeshDescription& Description,
    const TArray<FVertexID>& Vertices,
    const std::initializer_list<int32> Indices,
    const FVector& Normal,
    const FPolygonGroupID PolygonGroup)
{
    TArray<int32> OrderedIndices;
    for (const int32 VertexIndex : Indices)
    {
        OrderedIndices.Add(VertexIndex);
    }

    if (OrderedIndices.Num() >= 3)
    {
        const FVector P0(Description.GetVertexPosition(
            Vertices[OrderedIndices[0]]));
        const FVector P1(Description.GetVertexPosition(
            Vertices[OrderedIndices[1]]));
        const FVector P2(Description.GetVertexPosition(
            Vertices[OrderedIndices[2]]));
        const FVector GeometricNormal =
            FVector::CrossProduct(P1 - P0, P2 - P0).GetSafeNormal();
        if (!GeometricNormal.IsNearlyZero()
            && FVector::DotProduct(GeometricNormal, Normal) < 0.0)
        {
            Algo::Reverse(OrderedIndices);
        }
    }

    TArray<FVertexInstanceID> Instances;
    static const FVector2D UVs[4] = {
        FVector2D(0.0, 0.0),
        FVector2D(1.0, 0.0),
        FVector2D(1.0, 1.0),
        FVector2D(0.0, 1.0)
    };

    int32 UVIndex = 0;
    for (const int32 VertexIndex : OrderedIndices)
    {
        const FVertexInstanceID Instance =
            Description.CreateVertexInstance(Vertices[VertexIndex]);
        Description.GetVertexInstanceNormals()[Instance] =
            FVector3f(Normal);
        const FVector Tangent =
            FMath::Abs(Normal.X) > 0.9
                ? FVector(0.0, 1.0, 0.0)
                : FVector(1.0, 0.0, 0.0);
        Description.GetVertexInstanceTangents()[Instance] =
            FVector3f(Tangent);
        Description.GetVertexInstanceBinormalSigns()[Instance] = 1.0f;
        Description.SetVertexInstanceUV(
            Instance,
            UVs[FMath::Clamp(UVIndex, 0, 3)],
            0);
        Instances.Add(Instance);
        ++UVIndex;
    }

    TArray<FEdgeID> NewEdges;
    Description.CreatePolygon(
        PolygonGroup,
        Instances,
        NewEdges);
}

void AddCube(
    UStaticMeshDescription& Description,
    const FVector& Center,
    const FVector& HalfExtents,
    const FPolygonGroupID PolygonGroup)
{
    const FVector Min = Center - HalfExtents;
    const FVector Max = Center + HalfExtents;

    const FVector Positions[8] = {
        FVector(Min.X, Min.Y, Min.Z),
        FVector(Max.X, Min.Y, Min.Z),
        FVector(Max.X, Max.Y, Min.Z),
        FVector(Min.X, Max.Y, Min.Z),
        FVector(Min.X, Min.Y, Max.Z),
        FVector(Max.X, Min.Y, Max.Z),
        FVector(Max.X, Max.Y, Max.Z),
        FVector(Min.X, Max.Y, Max.Z)
    };

    TArray<FVertexID> Vertices;
    Vertices.Reserve(8);
    for (const FVector& Position : Positions)
    {
        const FVertexID Vertex = Description.CreateVertex();
        Description.SetVertexPosition(Vertex, Position);
        Vertices.Add(Vertex);
    }

    // Outward-facing quads. Visual materials are simple/two-sided-safe and
    // collision is configured double-sided, but keep deterministic normals.
    AddFace(Description, Vertices, {0, 3, 2, 1}, FVector(0, 0, -1), PolygonGroup);
    AddFace(Description, Vertices, {4, 5, 6, 7}, FVector(0, 0, 1), PolygonGroup);
    AddFace(Description, Vertices, {0, 4, 7, 3}, FVector(-1, 0, 0), PolygonGroup);
    AddFace(Description, Vertices, {1, 2, 6, 5}, FVector(1, 0, 0), PolygonGroup);
    AddFace(Description, Vertices, {0, 1, 5, 4}, FVector(0, -1, 0), PolygonGroup);
    AddFace(Description, Vertices, {3, 7, 6, 2}, FVector(0, 1, 0), PolygonGroup);
}

void AddPositionFace(
    UStaticMeshDescription& Description,
    TArray<FVector> Positions,
    const FVector& Normal,
    const FPolygonGroupID PolygonGroup)
{
    if (Positions.Num() < 3)
    {
        return;
    }

    FVector GeometricNormal =
        FVector::CrossProduct(
            Positions[1] - Positions[0],
            Positions[2] - Positions[0]).GetSafeNormal();
    if (!GeometricNormal.IsNearlyZero()
        && FVector::DotProduct(GeometricNormal, Normal) < 0.0)
    {
        Algo::Reverse(Positions);
    }

    TArray<FVertexID> Vertices;
    Vertices.Reserve(Positions.Num());
    for (const FVector& Position : Positions)
    {
        const FVertexID Vertex = Description.CreateVertex();
        Description.SetVertexPosition(Vertex, Position);
        Vertices.Add(Vertex);
    }

    TArray<FVertexInstanceID> Instances;
    Instances.Reserve(Vertices.Num());
    for (int32 Index = 0; Index < Vertices.Num(); ++Index)
    {
        const FVertexInstanceID Instance =
            Description.CreateVertexInstance(Vertices[Index]);
        Description.GetVertexInstanceNormals()[Instance] =
            FVector3f(Normal);
        const FVector Tangent =
            FMath::Abs(Normal.X) > 0.9
                ? FVector(0.0, 1.0, 0.0)
                : FVector(1.0, 0.0, 0.0);
        Description.GetVertexInstanceTangents()[Instance] =
            FVector3f(Tangent);
        Description.GetVertexInstanceBinormalSigns()[Instance] = 1.0f;
        Description.SetVertexInstanceUV(
            Instance,
            FVector2D(
                Index == 1 || Index == 2 ? 1.0 : 0.0,
                Index >= 2 ? 1.0 : 0.0),
            0);
        Instances.Add(Instance);
    }

    TArray<FEdgeID> NewEdges;
    Description.CreatePolygon(PolygonGroup, Instances, NewEdges);
}

void AddTaperedStripSegment(
    UStaticMeshDescription& Description,
    const double X0,
    const double X1,
    const double InnerAbsY0,
    const double OuterAbsY0,
    const double InnerAbsY1,
    const double OuterAbsY1,
    const double BottomZ,
    const double TopZ,
    const double SideSign,
    const FPolygonGroupID PolygonGroup)
{
    const double I0 = SideSign * InnerAbsY0;
    const double O0 = SideSign * OuterAbsY0;
    const double I1 = SideSign * InnerAbsY1;
    const double O1 = SideSign * OuterAbsY1;

    const double Width0 = FMath::Abs(OuterAbsY0 - InnerAbsY0);
    const double Width1 = FMath::Abs(OuterAbsY1 - InnerAbsY1);
    const bool bStartOpen = Width0 > KINDA_SMALL_NUMBER;
    const bool bEndOpen = Width1 > KINDA_SMALL_NUMBER;
    if (!bStartOpen && !bEndOpen)
    {
        return;
    }

    if (bStartOpen && bEndOpen)
    {
        AddPositionFace(
            Description,
            {
                FVector(X0, I0, TopZ),
                FVector(X0, O0, TopZ),
                FVector(X1, O1, TopZ),
                FVector(X1, I1, TopZ)
            },
            FVector(0, 0, 1),
            PolygonGroup);
        AddPositionFace(
            Description,
            {
                FVector(X0, I0, BottomZ),
                FVector(X1, I1, BottomZ),
                FVector(X1, O1, BottomZ),
                FVector(X0, O0, BottomZ)
            },
            FVector(0, 0, -1),
            PolygonGroup);
    }
    else if (bStartOpen)
    {
        const FVector TipTop(X1, O1, TopZ);
        const FVector TipBottom(X1, O1, BottomZ);
        AddPositionFace(
            Description,
            {
                FVector(X0, I0, TopZ),
                FVector(X0, O0, TopZ),
                TipTop
            },
            FVector(0, 0, 1),
            PolygonGroup);
        AddPositionFace(
            Description,
            {
                FVector(X0, O0, BottomZ),
                FVector(X0, I0, BottomZ),
                TipBottom
            },
            FVector(0, 0, -1),
            PolygonGroup);
    }
    else
    {
        const FVector TipTop(X0, O0, TopZ);
        const FVector TipBottom(X0, O0, BottomZ);
        AddPositionFace(
            Description,
            {
                TipTop,
                FVector(X1, O1, TopZ),
                FVector(X1, I1, TopZ)
            },
            FVector(0, 0, 1),
            PolygonGroup);
        AddPositionFace(
            Description,
            {
                TipBottom,
                FVector(X1, I1, BottomZ),
                FVector(X1, O1, BottomZ)
            },
            FVector(0, 0, -1),
            PolygonGroup);
    }

    const FVector OuterNormal(0, SideSign, 0);
    const FVector InnerNormal(0, -SideSign, 0);
    AddPositionFace(
        Description,
        {
            FVector(X0, O0, BottomZ),
            FVector(X1, O1, BottomZ),
            FVector(X1, O1, TopZ),
            FVector(X0, O0, TopZ)
        },
        OuterNormal,
        PolygonGroup);
    AddPositionFace(
        Description,
        {
            FVector(X0, I0, BottomZ),
            FVector(X0, I0, TopZ),
            FVector(X1, I1, TopZ),
            FVector(X1, I1, BottomZ)
        },
        InnerNormal,
        PolygonGroup);
}

void AddSymmetricTaperedStripSegment(
    UStaticMeshDescription& Description,
    const double X0,
    const double X1,
    const double InnerAbsY0,
    const double OuterAbsY0,
    const double InnerAbsY1,
    const double OuterAbsY1,
    const double BottomZ,
    const double TopZ,
    const FPolygonGroupID PolygonGroup)
{
    AddTaperedStripSegment(
        Description,
        X0,
        X1,
        InnerAbsY0,
        OuterAbsY0,
        InnerAbsY1,
        OuterAbsY1,
        BottomZ,
        TopZ,
        1.0,
        PolygonGroup);
    AddTaperedStripSegment(
        Description,
        X0,
        X1,
        InnerAbsY0,
        OuterAbsY0,
        InnerAbsY1,
        OuterAbsY1,
        BottomZ,
        TopZ,
        -1.0,
        PolygonGroup);
}

void AddSymmetricCube(
    UStaticMeshDescription& Description,
    const double CenterAbsY,
    const FVector& HalfExtents,
    const double CenterX,
    const double CenterZ,
    const FPolygonGroupID PolygonGroup)
{
    AddCube(
        Description,
        FVector(CenterX, CenterAbsY, CenterZ),
        HalfExtents,
        PolygonGroup);
    AddCube(
        Description,
        FVector(CenterX, -CenterAbsY, CenterZ),
        HalfExtents,
        PolygonGroup);
}

UStaticMesh* BuildConstructionMesh(
    UMaterialInterface& DividerMaterial,
    UMaterialInterface& CurbMaterial,
    FAutomationTestBase& Test,
    int32& OutBoxCount)
{
    UPackage* Package = CreatePackage(ConstructionPackage);
    Test.TestNotNull(TEXT("R2 construction package created"), Package);
    if (!Package)
    {
        return nullptr;
    }

    UStaticMesh* Mesh = LoadObject<UStaticMesh>(
        nullptr,
        TEXT("/Game/World/L1/Road/RoadConstruction.RoadConstruction"));
    const bool bNewMesh = Mesh == nullptr;
    if (!Mesh)
    {
        Mesh = NewObject<UStaticMesh>(
            Package,
            FName(ConstructionAsset),
            RF_Public | RF_Standalone | RF_Transactional);
    }
    Test.TestNotNull(TEXT("R2 construction mesh object created"), Mesh);
    if (!Mesh)
    {
        return nullptr;
    }

    // Always build from a fresh transient description. Failed CI authoring
    // passes can leave an unsaved/stale RoadConstruction asset in the warm
    // runner worktree; using the mesh as the description outer can reuse its
    // prior source description and accidentally accumulate geometry.
    Mesh->ClearMeshDescriptions();
    UStaticMeshDescription* Description =
        UStaticMesh::CreateStaticMeshDescription(GetTransientPackage());
    Test.TestNotNull(TEXT("R2 static mesh description created"), Description);
    if (!Description)
    {
        return nullptr;
    }

    Description->RegisterAttributes();
    const FPolygonGroupID DividerGroup = Description->CreatePolygonGroup();
    const FPolygonGroupID CurbGroup = Description->CreatePolygonGroup();
    Description->SetPolygonGroupMaterialSlotName(
        DividerGroup, TEXT("Divider"));
    Description->SetPolygonGroupMaterialSlotName(
        CurbGroup, TEXT("Curb"));

    OutBoxCount = 0;
    const double ChunkHalf =
        FPinkCabL1RoadConstructionModel::ChunkLengthCm * 0.5;

    // Continuous central raised median deck.
    AddCube(
        *Description,
        FVector(
            ChunkHalf,
            0.0,
            FPinkCabL1RoadConstructionModel::MedianDeckHeightCm * 0.5),
        FVector(
            ChunkHalf,
            FPinkCabL1RoadConstructionModel::CentralMedianWidthCm * 0.5,
            FPinkCabL1RoadConstructionModel::MedianDeckHeightCm * 0.5),
        DividerGroup);
    ++OutBoxCount;

    // Median curbs stay just inside the accepted 8m median.
    const double MedianCurbCenter =
        FPinkCabL1RoadConstructionModel::CentralMedianWidthCm * 0.5 -
        FPinkCabL1RoadConstructionModel::CurbWidthCm * 0.5;
    AddSymmetricCube(
        *Description,
        MedianCurbCenter,
        FVector(
            ChunkHalf,
            FPinkCabL1RoadConstructionModel::CurbWidthCm * 0.5,
            (FPinkCabL1RoadConstructionModel::CurbHeightCm -
             FPinkCabL1RoadConstructionModel::MedianDeckHeightCm) * 0.5),
        ChunkHalf,
        (FPinkCabL1RoadConstructionModel::CurbHeightCm +
         FPinkCabL1RoadConstructionModel::MedianDeckHeightCm) * 0.5,
        CurbGroup);
    OutBoxCount += 2;

    // Continuous outside curbs remain inside the frozen 66.8m envelope.
    AddSymmetricCube(
        *Description,
        FPinkCabL1RoadConstructionModel::ResolveOuterCurbCenterAbsYCm(),
        FVector(
            ChunkHalf,
            FPinkCabL1RoadConstructionModel::CurbWidthCm * 0.5,
            FPinkCabL1RoadConstructionModel::CurbHeightCm * 0.5),
        ChunkHalf,
        FPinkCabL1RoadConstructionModel::CurbHeightCm * 0.5,
        CurbGroup);
    OutBoxCount += 2;

    // R2 service construction follows the R1 connector continuously.
    // Generate tapered strips between exact 5m boundary samples: no closed
    // per-slice cubes, no +0.5cm overlap, and therefore no internal coplanar
    // end faces or repeating z-fighting artifacts.
    const int32 SliceCount = FMath::RoundToInt(
        FPinkCabL1RoadConstructionModel::ChunkLengthCm /
        FPinkCabL1RoadConstructionModel::ConstructionSliceLengthCm);
    const double SliceLength =
        FPinkCabL1RoadConstructionModel::ConstructionSliceLengthCm;
    const double ServiceOuter =
        FPinkCabL1RoadConstructionModel::ServiceOuterEdgeAbsYCm;

    for (int32 Slice = 0; Slice < SliceCount; ++Slice)
    {
        const double X0 = Slice * SliceLength;
        const double X1 = (Slice + 1) * SliceLength;
        const FPinkCabL1RoadConstructionBand Band0 =
            FPinkCabL1RoadConstructionModel::ResolveServiceSeparator(X0);
        const FPinkCabL1RoadConstructionBand Band1 =
            FPinkCabL1RoadConstructionModel::ResolveServiceSeparator(X1);

        const double Width0 = Band0.WidthCm;
        const double Width1 = Band1.WidthCm;
        if (Width0 <= KINDA_SMALL_NUMBER
            && Width1 <= KINDA_SMALL_NUMBER)
        {
            continue;
        }

        const double Inner0 = ServiceOuter - Width0;
        const double Inner1 = ServiceOuter - Width1;

        AddSymmetricTaperedStripSegment(
            *Description,
            X0,
            X1,
            Inner0,
            ServiceOuter,
            Inner1,
            ServiceOuter,
            0.0,
            FPinkCabL1RoadConstructionModel::ServiceDeckHeightCm,
            DividerGroup);
        OutBoxCount += 2;

        // Both curbs taper to zero with the separator instead of terminating
        // as block endcaps. Limit each curb to half the remaining band so the
        // two curb strips never overlap near the connector throat.
        const double CurbWidth0 = FMath::Min(
            FPinkCabL1RoadConstructionModel::CurbWidthCm,
            Width0 * 0.5);
        const double CurbWidth1 = FMath::Min(
            FPinkCabL1RoadConstructionModel::CurbWidthCm,
            Width1 * 0.5);

        if (CurbWidth0 > KINDA_SMALL_NUMBER
            || CurbWidth1 > KINDA_SMALL_NUMBER)
        {
            AddSymmetricTaperedStripSegment(
                *Description,
                X0,
                X1,
                Inner0,
                Inner0 + CurbWidth0,
                Inner1,
                Inner1 + CurbWidth1,
                FPinkCabL1RoadConstructionModel::ServiceDeckHeightCm,
                FPinkCabL1RoadConstructionModel::CurbHeightCm,
                CurbGroup);
            AddSymmetricTaperedStripSegment(
                *Description,
                X0,
                X1,
                ServiceOuter - CurbWidth0,
                ServiceOuter,
                ServiceOuter - CurbWidth1,
                ServiceOuter,
                FPinkCabL1RoadConstructionModel::ServiceDeckHeightCm,
                FPinkCabL1RoadConstructionModel::CurbHeightCm,
                CurbGroup);
            OutBoxCount += 4;
        }
    }

    TArray<FStaticMaterial> Materials;
    Materials.Emplace(
        &DividerMaterial,
        FName(TEXT("Divider")),
        FName(TEXT("Divider")));
    Materials.Emplace(
        &CurbMaterial,
        FName(TEXT("Curb")),
        FName(TEXT("Curb")));
    Mesh->SetStaticMaterials(Materials);

    TArray<UStaticMeshDescription*> Descriptions;
    Descriptions.Add(Description);
    Mesh->BuildFromStaticMeshDescriptions(
        Descriptions,
        false,
        false);

    Mesh->CreateBodySetup();
    Test.TestNotNull(TEXT("R2 construction body setup created"),
        Mesh->GetBodySetup());
    if (Mesh->GetBodySetup())
    {
        Mesh->GetBodySetup()->CollisionTraceFlag =
            CTF_UseComplexAsSimple;
        Mesh->GetBodySetup()->bDoubleSidedGeometry = true;
        Mesh->GetBodySetup()->InvalidatePhysicsData();
    }

    Mesh->MarkPackageDirty();
    Mesh->PostEditChange();

    if (bNewMesh)
    {
        FAssetRegistryModule::AssetCreated(Mesh);
    }

    Test.TestTrue(TEXT("R2 construction mesh saved"),
        SaveAssetPackage(*Mesh, Test));
    return Mesh;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabGenerateL1RoadConstructionR2,
    "PinkCab.Editor.GenerateL1RoadConstructionR2",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabGenerateL1RoadConstructionR2::RunTest(
    const FString& Parameters)
{
    using namespace PinkCabL1RoadConstructionAuthoring;

    UMaterialInterface* Divider = LoadObject<UMaterialInterface>(
        nullptr,
        DividerMaterialPath);
    TestNotNull(TEXT("R2 reuses project-owned divider material"), Divider);

    UMaterial* Curb = CreateOrLoadCurbMaterial(*this);
    if (!Divider || !Curb)
    {
        return false;
    }

    int32 BoxCount = 0;
    UStaticMesh* Construction = BuildConstructionMesh(
        *Divider,
        *Curb,
        *this,
        BoxCount);
    TestNotNull(TEXT("R2 construction mesh built"), Construction);
    if (!Construction)
    {
        return false;
    }

    const FVector Size = Construction->GetBounds().GetBox().GetSize();
    TestTrue(TEXT("R2 authored construction remains 1000m long"),
        FMath::IsNearlyEqual(Size.X, 100000.0, 10.0));
    TestTrue(TEXT("R2 authored construction remains 66.8m wide"),
        Size.Y >= 6670.0 && Size.Y <= 6682.0);
    TestTrue(TEXT("R2 authored construction reaches 12cm curb height"),
        Size.Z >= 11.5 && Size.Z <= 14.5);
    TestTrue(TEXT("R2 construction contains substantial built geometry"),
        BoxCount > 100);

    AddInfo(FString::Printf(
        TEXT("CD869_R2_CONSTRUCTION_BAKE=PASS boxes=%d bounds=%.1f,%.1f,%.1f"),
        BoxCount,
        Size.X,
        Size.Y,
        Size.Z));
    return true;
}

#endif
