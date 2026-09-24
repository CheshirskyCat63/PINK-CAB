#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
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

void AddCube(
    UStaticMeshDescription& Description,
    const FVector& Center,
    const FVector& HalfExtents,
    const FPolygonGroupID PolygonGroup)
{
    FPolygonID PlusX;
    FPolygonID MinusX;
    FPolygonID PlusY;
    FPolygonID MinusY;
    FPolygonID PlusZ;
    FPolygonID MinusZ;
    Description.CreateCube(
        Center,
        HalfExtents,
        PolygonGroup,
        PlusX,
        MinusX,
        PlusY,
        MinusY,
        PlusZ,
        MinusZ);
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

    UStaticMeshDescription* Description =
        UStaticMesh::CreateStaticMeshDescription(Mesh);
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
            FPinkCabL1RoadConstructionModel::CurbHeightCm * 0.5),
        ChunkHalf,
        FPinkCabL1RoadConstructionModel::CurbHeightCm * 0.5,
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

    // Side service separators are sampled every 5m. The R1 connector model is
    // authoritative: when it opens, this raised construction shrinks from the
    // express side while remaining pinned to the local-road edge.
    const int32 SliceCount = FMath::RoundToInt(
        FPinkCabL1RoadConstructionModel::ChunkLengthCm /
        FPinkCabL1RoadConstructionModel::ConstructionSliceLengthCm);
    const double SliceHalf =
        FPinkCabL1RoadConstructionModel::ConstructionSliceLengthCm * 0.5;

    for (int32 Slice = 0; Slice < SliceCount; ++Slice)
    {
        const double CenterX =
            Slice * FPinkCabL1RoadConstructionModel::ConstructionSliceLengthCm +
            SliceHalf;
        const FPinkCabL1RoadConstructionBand Band =
            FPinkCabL1RoadConstructionModel::ResolveServiceSeparator(CenterX);
        if (!Band.bPresent || Band.WidthCm < 1.0)
        {
            continue;
        }

        AddSymmetricCube(
            *Description,
            Band.CenterAbsYCm,
            FVector(
                SliceHalf + 0.5,
                Band.WidthCm * 0.5,
                FPinkCabL1RoadConstructionModel::ServiceDeckHeightCm * 0.5),
            CenterX,
            FPinkCabL1RoadConstructionModel::ServiceDeckHeightCm * 0.5,
            DividerGroup);
        OutBoxCount += 2;

        if (Band.WidthCm <
            FPinkCabL1RoadConstructionModel::CurbWidthCm * 2.0)
        {
            continue;
        }

        const double InnerEdgeAbsY =
            FPinkCabL1RoadConstructionModel::ServiceOuterEdgeAbsYCm -
            Band.WidthCm;
        const double InnerCurbCenterAbsY =
            InnerEdgeAbsY +
            FPinkCabL1RoadConstructionModel::CurbWidthCm * 0.5;
        const double OuterCurbCenterAbsY =
            FPinkCabL1RoadConstructionModel::ServiceOuterEdgeAbsYCm -
            FPinkCabL1RoadConstructionModel::CurbWidthCm * 0.5;

        AddSymmetricCube(
            *Description,
            InnerCurbCenterAbsY,
            FVector(
                SliceHalf + 0.5,
                FPinkCabL1RoadConstructionModel::CurbWidthCm * 0.5,
                FPinkCabL1RoadConstructionModel::CurbHeightCm * 0.5),
            CenterX,
            FPinkCabL1RoadConstructionModel::CurbHeightCm * 0.5,
            CurbGroup);
        AddSymmetricCube(
            *Description,
            OuterCurbCenterAbsY,
            FVector(
                SliceHalf + 0.5,
                FPinkCabL1RoadConstructionModel::CurbWidthCm * 0.5,
                FPinkCabL1RoadConstructionModel::CurbHeightCm * 0.5),
            CenterX,
            FPinkCabL1RoadConstructionModel::CurbHeightCm * 0.5,
            CurbGroup);
        OutBoxCount += 4;
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
