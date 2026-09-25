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
#include "Assets/RoadLaneAttributeMark.h"
#include "Assets/RoadMarkProfile.h"
#include "Assets/RoadProfile.h"
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


bool ResolveNativeBrokenMark(
    const TCHAR* RoadProfilePath,
    FRoadLaneMark& OutMark,
    FAutomationTestBase& Test)
{
    URoadProfile* RoadProfile =
        LoadObject<URoadProfile>(nullptr, RoadProfilePath);
    Test.TestNotNull(
        *FString::Printf(
            TEXT("native MetaRoad reference road profile loads: %s"),
            RoadProfilePath),
        RoadProfile);
    if (!RoadProfile)
    {
        return false;
    }

    auto FindBrokenMark = [&OutMark](const TArray<FRoadLaneProfile>& Lanes)
    {
        for (const FRoadLaneProfile& Lane : Lanes)
        {
            for (const FRoadLaneAttributeProfile& Attribute : Lane.Attributes)
            {
                UClass* Descriptor =
                    Attribute.AttributeDesctiptor.LoadSynchronous();
                if (Descriptor !=
                    URoadLaneAttributeMarkDescriptor::StaticClass())
                {
                    continue;
                }

                const FRoadLaneMark* Mark =
                    Attribute.AttributeValueTemplate.GetPtr<FRoadLaneMark>();
                if (!Mark ||
                    Mark->ProfileSource !=
                        ERoadLaneMarkProfile::UsePreset ||
                    Mark->Profile.IsNull())
                {
                    continue;
                }

                Mark->Profile.LoadSynchronous();
                if (Mark->GetProfile()
                        .GetPtr<FRoadLaneMarkProfileBroken>() != nullptr)
                {
                    OutMark = *Mark;
                    return true;
                }
            }
        }
        return false;
    };

    const bool bFound =
        FindBrokenMark(RoadProfile->Left) ||
        FindBrokenMark(RoadProfile->Right);
    Test.TestTrue(
        *FString::Printf(
            TEXT("native MetaRoad reference exposes broken mark preset: %s"),
            RoadProfilePath),
        bFound);
    if (bFound)
    {
        Test.AddInfo(FString::Printf(
            TEXT("CD869_R3_NATIVE_MARK_PROFILE=%s"),
            *OutMark.Profile.ToSoftObjectPath().ToString()));
    }
    return bFound;
}

void ApplyNativePresetMark(
    FRoadLane& Lane,
    const FRoadLaneMark& Mark)
{
    FRoadLaneAttribute Attribute(FRoadLaneMark::StaticStruct());
    Attribute.AddTypedKey(0.0, Mark);

    const TSoftClassPtr<URoadLaneAttributeDescriptor> MarkDescriptor(
        URoadLaneAttributeMarkDescriptor::StaticClass());
    Lane.Attributes.Add(MarkDescriptor, MoveTemp(Attribute));
}

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
    const double SectionLengthCm,
    const FRoadZoneType& ZoneType,
    const bool bRaised = false,
    const bool bInsideCurb = true,
    const bool bOutsideCurb = true)
{
    FRoadLane Lane;
    Lane.Width.Reset();
    Lane.Width.AddKey(0.0, WidthCm);
    Lane.Width.AddKey(SectionLengthCm, WidthCm);
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
    const double SectionStartCm,
    const double SectionEndCm,
    const bool bConnectorPavement,
    const FRoadZoneType& ZoneType,
    const bool bRaised = false,
    const bool bInsideCurb = true,
    const bool bOutsideCurb = true)
{
    FRoadLane Lane;
    Lane.Width.Reset();

    TArray<double> AbsoluteKeys = {
        SectionStartCm,
        SectionEndCm
    };
    const double AccessKeys[] = {
        FPinkCabL1EndlessRoadModel::AccessAStartCm,
        FPinkCabL1EndlessRoadModel::AccessAFullOpenStartCm,
        FPinkCabL1EndlessRoadModel::AccessAFullOpenEndCm,
        FPinkCabL1EndlessRoadModel::AccessAEndCm,
        FPinkCabL1EndlessRoadModel::AccessBStartCm,
        FPinkCabL1EndlessRoadModel::AccessBFullOpenStartCm,
        FPinkCabL1EndlessRoadModel::AccessBFullOpenEndCm,
        FPinkCabL1EndlessRoadModel::AccessBEndCm
    };

    for (const double X : AccessKeys)
    {
        if (X > SectionStartCm && X < SectionEndCm)
        {
            AbsoluteKeys.Add(X);
        }
    }
    AbsoluteKeys.Sort();

    for (const double X : AbsoluteKeys)
    {
        const double Width = bConnectorPavement
            ? FPinkCabL1EndlessRoadModel::ResolveAccessConnectorWidthCm(X)
            : FPinkCabL1EndlessRoadModel::ResolveAccessSeparatorWidthCm(X);
        Lane.Width.AddKey(X - SectionStartCm, Width);
    }

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

bool IsAccessOpeningSection(const double SectionStartCm)
{
    return FMath::IsNearlyEqual(
            SectionStartCm,
            FPinkCabL1EndlessRoadModel::AccessAFullOpenStartCm)
        || FMath::IsNearlyEqual(
            SectionStartCm,
            FPinkCabL1EndlessRoadModel::AccessBFullOpenStartCm);
}

void AddSideProfile(
    TArray<FRoadLane>& Lanes,
    const double SectionStartCm,
    const double SectionEndCm,
    const FRoadLaneMark& ExpressBrokenMark,
    const FRoadLaneMark& LocalBrokenMark)
{
    const double SectionLengthCm = SectionEndCm - SectionStartCm;
    const bool bAccessOpeningSection =
        IsAccessOpeningSection(SectionStartCm);

    // R2 native MetaRoad construction. The two 4m half-medians form one
    // 8m central raised island. Only the road-facing edge gets a curb so the
    // two halves meet cleanly at the road centerline.
    Lanes.Add(MakeSurfaceLane(
        HalfCentralMedianCm,
        SectionLengthCm,
        ERoadZoneTypes::Median,
        true,
        false,
        true));

    for (int32 LaneIndex = 0; LaneIndex < 5; ++LaneIndex)
    {
        FRoadLane ExpressLane = MakeSurfaceLane(
            ExpressLaneWidthCm,
            SectionLengthCm,
            ERoadZoneTypes::Driving);
        // R3: copy MetaRoad Free's own broken-mark preset from its
        // 4_Lanes+Borders profile. A mark lives on the lane's OUTER edge,
        // so the first four express lanes create exactly the four internal
        // separators of the accepted five-lane carriageway.
        if (LaneIndex < 4)
        {
            ApplyNativePresetMark(ExpressLane, ExpressBrokenMark);
        }
        Lanes.Add(MoveTemp(ExpressLane));
    }

    // Preserve the accepted R1 variable-width connector and separator.
    // The service separator remains a native raised MetaRoad sidewalk zone.
    // In the two access-window sections, BOTH service-separator curb sweeps
    // are intentionally disabled so MetaRoad leaves a true drivable opening
    // rather than carrying a curb line across the express<->local connector.
    // Outside those sections, the human-authored DefaultCurb profile remains.
    Lanes.Add(MakeAccessBandLane(
        SectionStartCm,
        SectionEndCm,
        true,
        ERoadZoneTypes::Driving));
    Lanes.Add(MakeAccessBandLane(
        SectionStartCm,
        SectionEndCm,
        false,
        ERoadZoneTypes::Median,
        true,
        !bAccessOpeningSection,
        !bAccessOpeningSection));

    for (int32 LaneIndex = 0; LaneIndex < 2; ++LaneIndex)
    {
        FRoadLane LocalLane = MakeSurfaceLane(
            LocalLaneWidthCm,
            SectionLengthCm,
            ERoadZoneTypes::Driving);
        // R3 local-road separator comes directly from MetaRoad Free's
        // 2_Lanes+Borders_Coodirect reference profile.
        if (LaneIndex == 0)
        {
            ApplyNativePresetMark(LocalLane, LocalBrokenMark);
        }
        Lanes.Add(MoveTemp(LocalLane));
    }

    // The accepted 1m outer shoulder becomes the native raised road-edge
    // treatment. Only the road-facing edge gets a curb; suppressing the outer
    // curb keeps the accepted 66.8m envelope exact.
    Lanes.Add(MakeSurfaceLane(
        OuterShoulderCm,
        SectionLengthCm,
        ERoadZoneTypes::Shoulder,
        true,
        true,
        false));
}

void AddRoadSection(
    FRoadLayout& Layout,
    const double SectionStartCm,
    const double SectionEndCm,
    const FRoadLaneMark& ExpressBrokenMark,
    const FRoadLaneMark& LocalBrokenMark)
{
    FRoadLaneSection Section;
    Section.Side = ERoadLaneSectionSide::Both;
    Section.SOffset = SectionStartCm;
    AddSideProfile(
        Section.Left,
        SectionStartCm,
        SectionEndCm,
        ExpressBrokenMark,
        LocalBrokenMark);
    AddSideProfile(
        Section.Right,
        SectionStartCm,
        SectionEndCm,
        ExpressBrokenMark,
        LocalBrokenMark);
    Layout.Sections.Add(MoveTemp(Section));
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

    FRoadLaneMark ExpressBrokenMark;
    FRoadLaneMark LocalBrokenMark;
    if (!ResolveNativeBrokenMark(
            TEXT("/MetaRoad/MetaRoad/Profiles/RoadProfiles/4_Lanes+Borders.4_Lanes+Borders"),
            ExpressBrokenMark,
            Test) ||
        !ResolveNativeBrokenMark(
            TEXT("/MetaRoad/MetaRoad/Profiles/RoadProfiles/2_Lanes+Borders_Coodirect.2_Lanes+Borders_Coodirect"),
            LocalBrokenMark,
            Test))
    {
        return false;
    }

    // Split only around the two 50m FULLY OPEN connector plateaus.
    // MetaRoad keeps its DefaultCurb on the separator through each taper, then
    // the zero-width separator lane disappears for the 50m crossing itself.
    // This preserves the authored curb along the taper while guaranteeing a
    // curb-free express<->local passage at 275-325m and 675-725m.
    const double SectionOffsets[] = {
        0.0,
        FPinkCabL1EndlessRoadModel::AccessAFullOpenStartCm,
        FPinkCabL1EndlessRoadModel::AccessAFullOpenEndCm,
        FPinkCabL1EndlessRoadModel::AccessBFullOpenStartCm,
        FPinkCabL1EndlessRoadModel::AccessBFullOpenEndCm,
        ChunkLengthCm
    };
    for (int32 Index = 0; Index < UE_ARRAY_COUNT(SectionOffsets) - 1; ++Index)
    {
        AddRoadSection(
            Layout,
            SectionOffsets[Index],
            SectionOffsets[Index + 1],
            ExpressBrokenMark,
            LocalBrokenMark);
    }

    Spline->UpdateRoadLayout();
    Spline->UpdateLaneSectionBounds();
    Spline->MarkRenderStateDirty();
    Road.MarkPackageDirty();

    Test.TestEqual(
        TEXT("five native MetaRoad lane sections preserve two curb openings"),
        Layout.Sections.Num(),
        5);
    if (Layout.Sections.Num() == 5)
    {
        const double ExpectedOffsets[] = {
            0.0,
            FPinkCabL1EndlessRoadModel::AccessAFullOpenStartCm,
            FPinkCabL1EndlessRoadModel::AccessAFullOpenEndCm,
            FPinkCabL1EndlessRoadModel::AccessBFullOpenStartCm,
            FPinkCabL1EndlessRoadModel::AccessBFullOpenEndCm
        };
        for (int32 Index = 0; Index < Layout.Sections.Num(); ++Index)
        {
            Test.TestTrue(
                *FString::Printf(
                    TEXT("section %d keeps accepted R1 longitudinal boundary"),
                    Index),
                FMath::IsNearlyEqual(
                    Layout.Sections[Index].SOffset,
                    ExpectedOffsets[Index],
                    0.1));
            const int32 ExpectedSurfaceCount =
                IsAccessOpeningSection(Layout.Sections[Index].SOffset)
                    ? 10
                    : 11;
            Test.TestEqual(
                *FString::Printf(
                    TEXT("section %d keeps expected native surface count"),
                    Index),
                Layout.Sections[Index].Left.Num(),
                ExpectedSurfaceCount);
            Test.TestEqual(
                *FString::Printf(
                    TEXT("section %d keeps mirrored native surface count"),
                    Index),
                Layout.Sections[Index].Right.Num(),
                ExpectedSurfaceCount);
            const TSoftClassPtr<URoadLaneAttributeDescriptor> MarkDescriptor(
                URoadLaneAttributeMarkDescriptor::StaticClass());
            for (const TArray<FRoadLane>* Side :
                { &Layout.Sections[Index].Left, &Layout.Sections[Index].Right })
            {
                int32 MarkedLaneCount = 0;
                for (const FRoadLane& Lane : *Side)
                {
                    if (Lane.Attributes.Contains(MarkDescriptor))
                    {
                        ++MarkedLaneCount;
                    }
                }
                Test.TestEqual(
                    *FString::Printf(
                        TEXT("section %d carries four express + one local native MetaRoad marks per side"),
                        Index),
                    MarkedLaneCount,
                    5);
            }
        }
        Test.TestTrue(
            TEXT("access A section disables service curbs"),
            IsAccessOpeningSection(Layout.Sections[1].SOffset));
        Test.TestTrue(
            TEXT("access B section disables service curbs"),
            IsAccessOpeningSection(Layout.Sections[3].SOffset));
        Test.TestFalse(
            TEXT("taper/straight separator section keeps service curbs"),
            IsAccessOpeningSection(Layout.Sections[2].SOffset));
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
    UMaterial* Shoulder,
    UMaterial* Mark)
{
    const FString SlotName = Slot.MaterialSlotName.ToString();
    if (SlotName.Contains(TEXT("Mark"), ESearchCase::IgnoreCase))
    {
        return Mark;
    }
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
            const FTransform RelativeTransform = Component->GetRelativeTransform();
            const FBoxSphereBounds LocalMeshBounds = Mesh->GetBounds();
            Test->AddInfo(FString::Printf(
                TEXT("CD869_METAROAD_COMPONENT package=%s rel_loc=(%.2f,%.2f,%.2f) rel_rot=(%.2f,%.2f,%.2f) rel_scale=(%.4f,%.4f,%.4f) local_origin=(%.2f,%.2f,%.2f) local_extent=(%.2f,%.2f,%.2f) world_origin=(%.2f,%.2f,%.2f) world_extent=(%.2f,%.2f,%.2f)"),
                *PackageName,
                RelativeTransform.GetLocation().X,
                RelativeTransform.GetLocation().Y,
                RelativeTransform.GetLocation().Z,
                RelativeTransform.Rotator().Pitch,
                RelativeTransform.Rotator().Yaw,
                RelativeTransform.Rotator().Roll,
                RelativeTransform.GetScale3D().X,
                RelativeTransform.GetScale3D().Y,
                RelativeTransform.GetScale3D().Z,
                LocalMeshBounds.Origin.X,
                LocalMeshBounds.Origin.Y,
                LocalMeshBounds.Origin.Z,
                LocalMeshBounds.BoxExtent.X,
                LocalMeshBounds.BoxExtent.Y,
                LocalMeshBounds.BoxExtent.Z,
                Component->Bounds.Origin.X,
                Component->Bounds.Origin.Y,
                Component->Bounds.Origin.Z,
                Component->Bounds.BoxExtent.X,
                Component->Bounds.BoxExtent.Y,
                Component->Bounds.BoxExtent.Z));
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
    UMaterial* Mark = CreateSimpleSurfaceMaterial(
        TEXT("/Game/World/L1/Road/Materials/M_PC_L1_Mark"),
        TEXT("M_PC_L1_Mark"),
        FLinearColor(0.82f, 0.82f, 0.78f, 1.0f),
        0.72f,
        0.12f,
        *this);

    if (!Asphalt || !Divider || !Shoulder || !Mark)
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
        TEXT("RoadCurbs31"),
        TEXT("RoadMarks")
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
                        Slots[Index], Asphalt, Divider, Shoulder, Mark));
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
