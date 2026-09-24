#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/PackageName.h"
#include "UObject/SavePackage.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "MaterialEditingLibrary.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionConstant3Vector.h"
#include "Modules/ModuleManager.h"

#include "Components/StaticMeshComponent.h"
#include "Editor.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "FileHelpers.h"
#include "HAL/PlatformTime.h"
#include "MetaRoadActor.h"
#include "RoadSplineComponent.h"
#include "MetaRoadTypes.h"
#include "Assets/RoadCurbProfile.h"
#include "EditorMode/MetaRoadEditorMode.h"
#include "EditorModeManager.h"
#include "EditorMode/MetaRoadBakeSettings.h"
#include "UObject/StrongObjectPtr.h"
#include "World/PinkCabL1EndlessRoadModel.h"

namespace PinkCabL1MetaRoadAuthoring
{
constexpr double ChunkLengthCm = 100000.0;
constexpr double ExpressLaneWidthCm = 360.0;
constexpr double LocalLaneWidthCm = 320.0;
constexpr double HalfCentralMedianCm = 400.0;
constexpr double ServiceSeparatorCm = 400.0;
constexpr double OuterShoulderCm = 100.0;
constexpr double ExpectedRoadWidthCm = 6680.0;
constexpr double R2RaisedZoneHeightCm = 12.0;
const TCHAR* AuthoringMapPackage = TEXT("/Game/Dev/Authoring/L_PC_L1_MetaRoadAuthoring");

void ConfigureRaisedMetaRoadZone(
    FRoadLane& Lane,
    const FRoadZoneType& ZoneType,
    const bool bInsideCurb,
    const bool bOutsideCurb)
{
    Lane.RoadZone.InitializeAs<FRoadZoneSidewalk>();
    FRoadZoneSidewalk& Raised =
        Lane.RoadZone.GetMutable<FRoadZoneSidewalk>();
    Raised.ZoneType = ZoneType;
    Raised.DefaultHeight = R2RaisedZoneHeightCm;
    Raised.bInsideCurb = bInsideCurb;
    Raised.bOutsideCurb = bOutsideCurb;
    Raised.bBeginCurb = false;
    Raised.bEndCurb = false;
    // Keep MetaRoad's authored DefaultCurb soft profile. Do not replace it
    // with a project-generated mesh/profile.
}

FRoadLane MakeSurfaceLane(
    const double WidthCm,
    const FRoadZoneType& ZoneType,
    const bool bRaised = false,
    const bool bInsideCurb = true,
    const bool bOutsideCurb = true)
{
    FRoadLane Lane;
    Lane.Width.Reset();
    Lane.Width.AddKey(0.0, WidthCm);
    Lane.Width.AddKey(ChunkLengthCm, WidthCm);
    if (bRaised)
    {
        ConfigureRaisedMetaRoadZone(
            Lane, ZoneType, bInsideCurb, bOutsideCurb);
    }
    else
    {
        Lane.RoadZone.InitializeAs<FRoadZoneDriving>();
        Lane.RoadZone.GetMutable<FRoadZoneDriving>().ZoneType = ZoneType;
    }
    return Lane;
}

FRoadLane MakeAccessBandLane(
    const bool bConnectorPavement,
    const FRoadZoneType& ZoneType,
    const bool bRaised = false)
{
    FRoadLane Lane;
    Lane.Width.Reset();

    const double Keys[] = {
        0.0,
        FPinkCabL1EndlessRoadModel::AccessAStartCm,
        FPinkCabL1EndlessRoadModel::AccessAFullOpenStartCm,
        FPinkCabL1EndlessRoadModel::AccessAFullOpenEndCm,
        FPinkCabL1EndlessRoadModel::AccessAEndCm,
        FPinkCabL1EndlessRoadModel::AccessBStartCm,
        FPinkCabL1EndlessRoadModel::AccessBFullOpenStartCm,
        FPinkCabL1EndlessRoadModel::AccessBFullOpenEndCm,
        FPinkCabL1EndlessRoadModel::AccessBEndCm,
        ChunkLengthCm
    };

    for (const double X : Keys)
    {
        const double Width = bConnectorPavement
            ? FPinkCabL1EndlessRoadModel::ResolveAccessConnectorWidthCm(X)
            : FPinkCabL1EndlessRoadModel::ResolveAccessSeparatorWidthCm(X);
        Lane.Width.AddKey(X, Width);
    }

    if (bRaised)
    {
        ConfigureRaisedMetaRoadZone(Lane, ZoneType, true, true);
    }
    else
    {
        Lane.RoadZone.InitializeAs<FRoadZoneDriving>();
        Lane.RoadZone.GetMutable<FRoadZoneDriving>().ZoneType = ZoneType;
    }
    return Lane;
}

void AddSideProfile(TArray<FRoadLane>& Lanes)
{
    // R2 native MetaRoad construction. The two 4m half-medians form one
    // 8m central raised island. Only the road-facing edge gets a curb so the
    // two halves meet cleanly at the road centerline.
    Lanes.Add(MakeSurfaceLane(
        HalfCentralMedianCm,
        ERoadZoneTypes::Median,
        true,
        false,
        true));

    for (int32 LaneIndex = 0; LaneIndex < 5; ++LaneIndex)
    {
        Lanes.Add(MakeSurfaceLane(ExpressLaneWidthCm, ERoadZoneTypes::Driving));
    }

    // Preserve the accepted R1 access topology exactly: connector pavement
    // grows while the complementary service separator shrinks. In R2 the
    // separator itself is a native MetaRoad raised sidewalk-zone using the
    // plugin's DefaultCurb profile, so openings remain part of the same
    // authoring system rather than a second overlay mesh.
    Lanes.Add(MakeAccessBandLane(true, ERoadZoneTypes::Driving));
    Lanes.Add(MakeAccessBandLane(false, ERoadZoneTypes::Median, true));

    for (int32 LaneIndex = 0; LaneIndex < 2; ++LaneIndex)
    {
        Lanes.Add(MakeSurfaceLane(LocalLaneWidthCm, ERoadZoneTypes::Driving));
    }

    // The accepted 1m outer shoulder becomes the native raised road-edge
    // treatment. Only the road-facing edge gets a curb; suppressing the outer
    // curb keeps the accepted 66.8m envelope exact instead of adding one curb
    // profile width beyond it.
    Lanes.Add(MakeSurfaceLane(
        OuterShoulderCm,
        ERoadZoneTypes::Shoulder,
        true,
        true,
        false));
}

bool ConfigureStraightRoad(AMetaRoad& Road, FAutomationTestBase& Test)
{
    URoadSplineComponent* Spline = NewObject<URoadSplineComponent>(
        &Road, TEXT("L1StraightSpline"), RF_Transactional);
    Test.TestNotNull(TEXT("MetaRoad straight spline created"), Spline);
    if (!Spline)
    {
        return false;
    }

    Road.AddInstanceComponent(Spline);
    if (USceneComponent* ExistingRoot = Road.GetRootComponent())
    {
        Spline->SetupAttachment(ExistingRoot);
    }
    else
    {
        Road.SetRootComponent(Spline);
    }
    Spline->RegisterComponent();
    Spline->SetMobility(EComponentMobility::Static);

    Spline->ClearSplinePoints(false);
    Spline->AddSplinePoint(FVector(0.0, 0.0, 0.0), ESplineCoordinateSpace::Local, false);
    Spline->AddSplinePoint(FVector(ChunkLengthCm, 0.0, 0.0), ESplineCoordinateSpace::Local, false);
    Spline->SetRoadSplinePointType(0, ERoadSplinePointType::Linear, false);
    Spline->SetRoadSplinePointType(1, ERoadSplinePointType::Linear, false);
    Spline->UpdateSpline(-1);

    FRoadLayout& Layout = Spline->GetRoadLayout();
    Layout.Sections.Reset();
    Layout.Direction = ERoadDirection::RightHand;

    FRoadLaneSection Section;
    Section.Side = ERoadLaneSectionSide::Both;
    Section.SOffset = 0.0;
    AddSideProfile(Section.Left);
    AddSideProfile(Section.Right);
    Layout.Sections.Add(MoveTemp(Section));

    Spline->UpdateRoadLayout();
    Spline->UpdateLaneSectionBounds();
    Spline->MarkRenderStateDirty();
    Road.MarkPackageDirty();

    Test.TestEqual(TEXT("one MetaRoad lane section"), Layout.Sections.Num(), 1);
    if (Layout.Sections.Num() == 1)
    {
        Test.TestEqual(TEXT("R1 has eleven authored surfaces on left side"), Layout.Sections[0].Left.Num(), 11);
        Test.TestEqual(TEXT("R1 has eleven authored surfaces on right side"), Layout.Sections[0].Right.Num(), 11);
    }
    Test.TestTrue(TEXT("straight spline is one kilometre"),
        FMath::IsNearlyEqual(Spline->GetSplineLength(), ChunkLengthCm, 1.0));

    URoadCurbProfile* DefaultCurb = LoadObject<URoadCurbProfile>(
        nullptr,
        TEXT("/MetaRoad/MetaRoad/Profiles/Curbs/DefaultCurb.DefaultCurb"));
    Test.TestNotNull(TEXT("MetaRoad human-authored DefaultCurb profile loads"), DefaultCurb);
    if (DefaultCurb)
    {
        Test.TestTrue(TEXT("MetaRoad DefaultCurb has physical width"),
            DefaultCurb->Width > 0.0f);
        Test.TestTrue(TEXT("MetaRoad DefaultCurb contains an authored curve"),
            DefaultCurb->CurbCurve.GetRichCurveConst() != nullptr &&
            DefaultCurb->CurbCurve.GetRichCurveConst()->GetNumKeys() >= 2);
    }

    return true;
}

bool SaveGeneratedMeshPackage(UStaticMesh& Mesh, FAutomationTestBase& Test)
{
    UPackage* Package = Mesh.GetOutermost();
    Test.TestNotNull(TEXT("generated mesh has package"), Package);
    if (!Package)
    {
        return false;
    }

    const FString PackageName = Package->GetName();
    const FString Filename = FPackageName::LongPackageNameToFilename(
        PackageName, FPackageName::GetAssetPackageExtension());

    Package->MarkPackageDirty();

    FSavePackageArgs SaveArgs;
    SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
    SaveArgs.SaveFlags = SAVE_NoError;

    const bool bSaved = UPackage::SavePackage(Package, &Mesh, *Filename, SaveArgs);
    Test.TestTrue(
        *FString::Printf(TEXT("generated mesh package saved: %s"), *PackageName),
        bSaved);
    return bSaved;
}

bool SaveAssetPackage(UObject& Asset, FAutomationTestBase& Test)
{
    UPackage* Package = Asset.GetOutermost();
    Test.TestNotNull(TEXT("generated asset has package"), Package);
    if (!Package)
    {
        return false;
    }

    const FString PackageName = Package->GetName();
    const FString Filename = FPackageName::LongPackageNameToFilename(
        PackageName, FPackageName::GetAssetPackageExtension());
    Package->MarkPackageDirty();

    FSavePackageArgs SaveArgs;
    SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
    SaveArgs.SaveFlags = SAVE_NoError;
    const bool bSaved = UPackage::SavePackage(
        Package, &Asset, *Filename, SaveArgs);
    Test.TestTrue(
        *FString::Printf(TEXT("asset package saved: %s"), *PackageName),
        bSaved);
    return bSaved;
}

UMaterial* CreateSimpleSurfaceMaterial(
    const TCHAR* PackageName,
    const TCHAR* AssetName,
    const FLinearColor& BaseColor,
    const float Roughness,
    const float Specular,
    FAutomationTestBase& Test)
{
    const FString ObjectPath = FString::Printf(
        TEXT("%s.%s"), PackageName, AssetName);
    if (UMaterial* Existing = LoadObject<UMaterial>(nullptr, *ObjectPath))
    {
        return Existing;
    }

    UPackage* Package = CreatePackage(PackageName);
    Test.TestNotNull(TEXT("material package created"), Package);
    if (!Package)
    {
        return nullptr;
    }

    UMaterial* Material = NewObject<UMaterial>(
        Package,
        FName(AssetName),
        RF_Public | RF_Standalone | RF_Transactional);
    Test.TestNotNull(TEXT("project-owned road material created"), Material);
    if (!Material)
    {
        return nullptr;
    }

    UMaterialExpressionConstant3Vector* Color =
        Cast<UMaterialExpressionConstant3Vector>(
            UMaterialEditingLibrary::CreateMaterialExpression(
                Material,
                UMaterialExpressionConstant3Vector::StaticClass(),
                -420,
                -100));
    UMaterialExpressionConstant* RoughnessNode =
        Cast<UMaterialExpressionConstant>(
            UMaterialEditingLibrary::CreateMaterialExpression(
                Material,
                UMaterialExpressionConstant::StaticClass(),
                -420,
                20));
    UMaterialExpressionConstant* SpecularNode =
        Cast<UMaterialExpressionConstant>(
            UMaterialEditingLibrary::CreateMaterialExpression(
                Material,
                UMaterialExpressionConstant::StaticClass(),
                -420,
                120));

    Test.TestNotNull(TEXT("base-color expression created"), Color);
    Test.TestNotNull(TEXT("roughness expression created"), RoughnessNode);
    Test.TestNotNull(TEXT("specular expression created"), SpecularNode);
    if (!Color || !RoughnessNode || !SpecularNode)
    {
        return nullptr;
    }

    Color->Constant = BaseColor;
    RoughnessNode->R = Roughness;
    SpecularNode->R = Specular;

    UMaterialEditingLibrary::ConnectMaterialProperty(
        Color, FString(), MP_BaseColor);
    UMaterialEditingLibrary::ConnectMaterialProperty(
        RoughnessNode, FString(), MP_Roughness);
    UMaterialEditingLibrary::ConnectMaterialProperty(
        SpecularNode, FString(), MP_Specular);
    UMaterialEditingLibrary::RecompileMaterial(Material);

    FAssetRegistryModule::AssetCreated(Material);
    Material->PostEditChange();
    if (!SaveAssetPackage(*Material, Test))
    {
        return nullptr;
    }
    return Material;
}

UMaterial* ResolveProjectMaterialForSlot(
    const FStaticMaterial& Slot,
    UMaterial* Asphalt,
    UMaterial* Divider,
    UMaterial* Shoulder)
{
    const FString SlotName = Slot.MaterialSlotName.ToString();
    if (SlotName.Contains(TEXT("Median"), ESearchCase::IgnoreCase)
        || SlotName.Contains(TEXT("Divider"), ESearchCase::IgnoreCase))
    {
        return Divider;
    }
    if (SlotName.Contains(TEXT("Curb"), ESearchCase::IgnoreCase))
    {
        return Shoulder;
    }
    if (SlotName.Contains(TEXT("Shoulder"), ESearchCase::IgnoreCase))
    {
        return Shoulder;
    }
    return Asphalt;
}

struct FAuthoringState
{
    TWeakObjectPtr<UWorld> World;
    TWeakObjectPtr<AMetaRoad> Road;
    TWeakObjectPtr<UMetaRoadEditorMode> EditorMode;
    double StartedAtSeconds = 0.0;
};

class FFinishMetaRoadBakeCommand final : public IAutomationLatentCommand
{
public:
    FFinishMetaRoadBakeCommand(
        FAutomationTestBase* InTest,
        TSharedRef<FAuthoringState> InState)
        : Test(InTest), State(MoveTemp(InState))
    {
    }

    virtual bool Update() override
    {
        if (!State->EditorMode.IsValid() || !State->World.IsValid() || !State->Road.IsValid())
        {
            Test->AddError(TEXT("MetaRoad editor-mode authoring state became invalid during bake"));
            return true;
        }

        if ((FPlatformTime::Seconds() - State->StartedAtSeconds) > 120.0)
        {
            Test->AddError(TEXT("MetaRoad bake exceeded 120 second authoring timeout"));
            return true;
        }

        // UMetaRoadEditorMode owns and ticks its non-exported UMetaRoadBakeHost internally.
        // The exported IsBaking()/BakeAll() surface is the supported external integration point.
        if (State->EditorMode->IsBaking())
        {
            return false;
        }

        AActor* Generated = State->Road->LastGeneratedActor;
        Test->TestNotNull(TEXT("MetaRoad produced generated actor"), Generated);
        if (!Generated)
        {
            return true;
        }

        TArray<UStaticMeshComponent*> MeshComponents;
        Generated->GetComponents<UStaticMeshComponent>(MeshComponents);
        Test->TestTrue(TEXT("generated actor contains static mesh components"), MeshComponents.Num() > 0);

        FBox CombinedBounds(ForceInit);
        int32 SavedMeshCount = 0;
        int32 CollisionReadyCount = 0;
        for (UStaticMeshComponent* Component : MeshComponents)
        {
            if (!Component)
            {
                continue;
            }
            UStaticMesh* Mesh = Component->GetStaticMesh();
            if (!Mesh)
            {
                continue;
            }

            const FString PackageName = Mesh->GetOutermost()->GetName();
            Test->AddInfo(FString::Printf(
                TEXT("MetaRoad generated mesh: %s collision=%d"),
                *PackageName,
                static_cast<int32>(Component->GetCollisionEnabled())));
            Test->TestTrue(TEXT("generated mesh stored in canonical Level 1 road folder"),
                PackageName.StartsWith(TEXT("/Game/World/L1/Road/")));
            if (SaveGeneratedMeshPackage(*Mesh, *Test))
            {
                ++SavedMeshCount;
            }

            if (Component->GetCollisionEnabled() != ECollisionEnabled::NoCollision
                && Mesh->GetBodySetup() != nullptr)
            {
                ++CollisionReadyCount;
            }
            CombinedBounds += Component->Bounds.GetBox();
        }

        Test->TestTrue(TEXT("at least one generated road mesh saved"), SavedMeshCount > 0);
        Test->TestTrue(TEXT("at least one generated road mesh has collision"), CollisionReadyCount > 0);

        if (CombinedBounds.IsValid)
        {
            const FVector Size = CombinedBounds.GetSize();
            Test->AddInfo(FString::Printf(
                TEXT("MetaRoad generated bounds: X=%.2f Y=%.2f Z=%.2f"),
                Size.X, Size.Y, Size.Z));
            Test->TestTrue(TEXT("baked road remains approximately 1000m long"),
                FMath::IsNearlyEqual(Size.X, ChunkLengthCm, 250.0));
            Test->TestTrue(TEXT("baked road width matches approved 66.8m envelope"),
                FMath::IsNearlyEqual(Size.Y, ExpectedRoadWidthCm, 250.0));
            Test->TestTrue(TEXT("R2 native MetaRoad raised construction has real Z relief"),
                Size.Z >= R2RaisedZoneHeightCm - 1.0);
        }

        const bool bSaved = UEditorLoadingAndSavingUtils::SaveMap(
            State->World.Get(), AuthoringMapPackage);
        Test->TestTrue(TEXT("MetaRoad authoring map saved after bake"), bSaved);
        Test->AddInfo(TEXT("CD869_METAROAD_BAKE=PASS"));
        return true;
    }

private:
    FAutomationTestBase* Test = nullptr;
    TSharedRef<FAuthoringState> State;
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabGenerateL1EndlessMetaRoadAssets,
    "PinkCab.Editor.GenerateL1EndlessRoadAssets",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabGenerateL1EndlessMetaRoadAssets::RunTest(const FString& Parameters)
{
    using namespace PinkCabL1MetaRoadAuthoring;

    UWorld* World = UEditorLoadingAndSavingUtils::NewBlankMap(false);
    TestNotNull(TEXT("blank authoring world created"), World);
    if (!World)
    {
        return false;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.OverrideLevel = World->PersistentLevel;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AMetaRoad* Road = World->SpawnActor<AMetaRoad>(
        AMetaRoad::StaticClass(), FTransform::Identity, SpawnParams);
    TestNotNull(TEXT("MetaRoad source actor spawned"), Road);
    if (!Road)
    {
        return false;
    }

#if WITH_EDITOR
    Road->SetActorLabel(TEXT("PC_L1_Straight_1000m"));
#endif
    Road->bReplaceOnRegenerate = true;
    if (!ConfigureStraightRoad(*Road, *this))
    {
        return false;
    }

    TestTrue(TEXT("initial authoring map save"),
        UEditorLoadingAndSavingUtils::SaveMap(World, AuthoringMapPackage));

    UMetaRoadBakeSettings* Settings = UMetaRoadBakeSettings::Get();
    TestNotNull(TEXT("MetaRoad bake settings available"), Settings);
    if (!Settings)
    {
        return false;
    }
    Settings->GenerationLocation = EMetaRoadAssetGenerationLocation::Global;
    Settings->GenerationMode = EMetaRoadAssetGenerationBehavior::AutoGenerateButDoNotAutosave;
    Settings->AutoGeneratedAssetPath = TEXT("World/L1/Road");
    Settings->bStoreUnsavedLevelAssetsInTopLevelGameFolder = true;
    Settings->bUsePerUserAutogenSubfolder = false;
    Settings->bAppendRandomStringToName = false;
    Settings->CleanStrategy = EMetaRoadCleanStrategy::PermanentDelete;

    const TSharedRef<FAuthoringState> State = MakeShared<FAuthoringState>();
    State->World = World;
    State->Road = Road;
    State->StartedAtSeconds = FPlatformTime::Seconds();

    FEditorModeTools& ModeTools = GLevelEditorModeTools();
    ModeTools.ActivateMode(UMetaRoadEditorMode::EM_MetaRoadEditorModeId);
    UMetaRoadEditorMode* MetaRoadMode =
        Cast<UMetaRoadEditorMode>(ModeTools.GetActiveScriptableMode(UMetaRoadEditorMode::EM_MetaRoadEditorModeId));
    TestNotNull(TEXT("exported MetaRoad editor mode activated"), MetaRoadMode);
    if (!MetaRoadMode)
    {
        return false;
    }

    State->EditorMode = MetaRoadMode;
    MetaRoadMode->BakeAll();
    TestTrue(TEXT("MetaRoad bake enters running state through exported editor mode"), MetaRoadMode->IsBaking());
    if (!MetaRoadMode->IsBaking())
    {
        return false;
    }

    ADD_LATENT_AUTOMATION_COMMAND(FFinishMetaRoadBakeCommand(this, State));
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabGenerateL1EndlessRoadRuntimeMaterials,
    "PinkCab.Editor.GenerateL1EndlessRoadRuntimeMaterials",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabGenerateL1EndlessRoadRuntimeMaterials::RunTest(
    const FString& Parameters)
{
    using namespace PinkCabL1MetaRoadAuthoring;

    UMaterial* Asphalt = CreateSimpleSurfaceMaterial(
        TEXT("/Game/World/L1/Road/Materials/M_PC_L1_Asphalt"),
        TEXT("M_PC_L1_Asphalt"),
        FLinearColor(0.030f, 0.034f, 0.040f, 1.0f),
        0.70f,
        0.24f,
        *this);
    UMaterial* Divider = CreateSimpleSurfaceMaterial(
        TEXT("/Game/World/L1/Road/Materials/M_PC_L1_GreenDivider"),
        TEXT("M_PC_L1_GreenDivider"),
        FLinearColor(0.025f, 0.070f, 0.030f, 1.0f),
        0.88f,
        0.10f,
        *this);
    UMaterial* Shoulder = CreateSimpleSurfaceMaterial(
        TEXT("/Game/World/L1/Road/Materials/M_PC_L1_Shoulder"),
        TEXT("M_PC_L1_Shoulder"),
        FLinearColor(0.045f, 0.048f, 0.052f, 1.0f),
        0.78f,
        0.18f,
        *this);

    if (!Asphalt || !Divider || !Shoulder)
    {
        return false;
    }

    const TCHAR* GeneratedMeshNames[] = {
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

    int32 ReboundMeshCount = 0;
    for (const TCHAR* MeshName : GeneratedMeshNames)
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
                TEXT("native MetaRoad mesh loads for runtime ownership: %s"),
                MeshName),
            Mesh);
        if (!Mesh)
        {
            continue;
        }

        TArray<FStaticMaterial>& Slots = Mesh->GetStaticMaterials();
        TestTrue(
            *FString::Printf(TEXT("%s exposes material slots"), MeshName),
            Slots.Num() > 0);

        const bool bCurbMesh =
            FString(MeshName).StartsWith(TEXT("RoadCurbs"));
        for (int32 Index = 0; Index < Slots.Num(); ++Index)
        {
            const FString SlotName = Slots[Index].MaterialSlotName.ToString();
            AddInfo(FString::Printf(
                TEXT("CD869_NATIVE_SLOT[%s][%d]=%s"),
                MeshName,
                Index,
                *SlotName));
            Mesh->SetMaterial(
                Index,
                bCurbMesh
                    ? Shoulder
                    : ResolveProjectMaterialForSlot(
                        Slots[Index], Asphalt, Divider, Shoulder));
        }

        Mesh->PostEditChange();
        TestTrue(
            *FString::Printf(
                TEXT("%s saved after project material rebinding"),
                MeshName),
            SaveGeneratedMeshPackage(*Mesh, *this));

        for (int32 Index = 0; Index < Mesh->GetStaticMaterials().Num(); ++Index)
        {
            UMaterialInterface* Material =
                Mesh->GetStaticMaterials()[Index].MaterialInterface;
            TestNotNull(TEXT("runtime road material remains assigned"), Material);
            if (Material)
            {
                const FString Path = Material->GetPathName();
                TestTrue(
                    TEXT("runtime material is owned by PINK-CAB content"),
                    Path.StartsWith(
                        TEXT("/Game/World/L1/Road/Materials/")));
                TestFalse(
                    TEXT("runtime material no longer depends on MetaRoad content"),
                    Path.StartsWith(TEXT("/MetaRoad/")));
            }
        }
        ++ReboundMeshCount;
    }

    TestEqual(
        TEXT("all native MetaRoad R2 meshes rebound for runtime"),
        ReboundMeshCount,
        static_cast<int32>(UE_ARRAY_COUNT(GeneratedMeshNames)));

    AddInfo(TEXT("CD869_RUNTIME_MATERIAL_OWNERSHIP=PASS"));
    return true;
}

#endif
