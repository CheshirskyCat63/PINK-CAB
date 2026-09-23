#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/PackageName.h"
#include "UObject/SavePackage.h"

#include "Components/StaticMeshComponent.h"
#include "Editor.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "FileHelpers.h"
#include "HAL/PlatformTime.h"
#include "MetaRoadActor.h"
#include "RoadSplineComponent.h"
#include "MetaRoadTypes.h"
#include "EditorMode/MetaRoadEditorMode.h"
#include "EditorModeManager.h"
#include "EditorMode/MetaRoadBakeSettings.h"
#include "UObject/StrongObjectPtr.h"

namespace PinkCabL1MetaRoadAuthoring
{
constexpr double ChunkLengthCm = 100000.0;
constexpr double ExpressLaneWidthCm = 360.0;
constexpr double LocalLaneWidthCm = 320.0;
constexpr double HalfCentralMedianCm = 400.0;
constexpr double ServiceSeparatorCm = 400.0;
constexpr double OuterShoulderCm = 100.0;
constexpr double ExpectedRoadWidthCm = 6680.0;
const TCHAR* AuthoringMapPackage = TEXT("/Game/Dev/Authoring/L_PC_L1_MetaRoadAuthoring");

FRoadLane MakeSurfaceLane(const double WidthCm, const FRoadZoneType& ZoneType)
{
    FRoadLane Lane;
    Lane.Width.Reset();
    Lane.Width.AddKey(0.0, WidthCm);
    Lane.Width.AddKey(ChunkLengthCm, WidthCm);
    Lane.RoadZone.InitializeAs<FRoadZoneDriving>();
    Lane.RoadZone.GetMutable<FRoadZoneDriving>().ZoneType = ZoneType;
    return Lane;
}

void AddSideProfile(TArray<FRoadLane>& Lanes)
{
    Lanes.Add(MakeSurfaceLane(HalfCentralMedianCm, ERoadZoneTypes::Median));
    for (int32 LaneIndex = 0; LaneIndex < 5; ++LaneIndex)
    {
        Lanes.Add(MakeSurfaceLane(ExpressLaneWidthCm, ERoadZoneTypes::Driving));
    }
    Lanes.Add(MakeSurfaceLane(ServiceSeparatorCm, ERoadZoneTypes::Median));
    for (int32 LaneIndex = 0; LaneIndex < 2; ++LaneIndex)
    {
        Lanes.Add(MakeSurfaceLane(LocalLaneWidthCm, ERoadZoneTypes::Driving));
    }
    Lanes.Add(MakeSurfaceLane(OuterShoulderCm, ERoadZoneTypes::Shoulder));
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
        Test.TestEqual(TEXT("ten authored surfaces on left side"), Layout.Sections[0].Left.Num(), 10);
        Test.TestEqual(TEXT("ten authored surfaces on right side"), Layout.Sections[0].Right.Num(), 10);
    }
    Test.TestTrue(TEXT("straight spline is one kilometre"),
        FMath::IsNearlyEqual(Spline->GetSplineLength(), ChunkLengthCm, 1.0));
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

#endif
