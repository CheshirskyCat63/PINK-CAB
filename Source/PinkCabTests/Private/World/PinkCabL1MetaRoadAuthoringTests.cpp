#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Vehicle/PinkCabHGateGeometry.h"
#include "Misc/PackageName.h"
#include "UObject/SavePackage.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "MaterialEditingLibrary.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpression.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionConstant3Vector.h"
#include "Materials/MaterialExpressionComponentMask.h"
#include "Materials/MaterialExpressionAdd.h"
#include "Materials/MaterialExpressionLinearInterpolate.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "Materials/MaterialExpressionNoise.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Materials/MaterialExpressionWorldPosition.h"
#include "Materials/MaterialExpressionTextureCoordinate.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Modules/ModuleManager.h"

#include "Components/StaticMeshComponent.h"
#include "Editor.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture.h"
#include "Engine/TextureDefines.h"
#include "Engine/Texture2D.h"
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
constexpr double ExpressLaneWidthCm = 450.0;
constexpr double LocalLaneWidthCm = 400.0;
constexpr double HalfCentralMedianCm = 400.0;
constexpr double ServiceSeparatorCm = 400.0;
constexpr double OuterShoulderCm = 100.0;
constexpr double ExpectedRoadWidthCm = 7900.0;
constexpr double R2RaisedZoneHeightCm = 12.0;

static_assert(
    ExpressLaneWidthCm == 450.0,
    "CD-869 express lanes must remain exactly +25% from the 360 cm HUMAN baseline");
static_assert(
    LocalLaneWidthCm == 400.0,
    "CD-869 local lanes must remain exactly +25% from the 320 cm HUMAN baseline");
static_assert(
    ExpectedRoadWidthCm == 7900.0,
    "CD-869 widened L1 cross-section must remain 79.0 m");
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
    // curb keeps the owner-requested 79.0m widened envelope exact.
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


uint32 AsphaltHash2D(
    const int32 X,
    const int32 Y,
    const uint32 Seed)
{
    uint32 H =
        static_cast<uint32>(X) * 374761393u +
        static_cast<uint32>(Y) * 668265263u +
        Seed * 2246822519u;
    H = (H ^ (H >> 13u)) * 1274126177u;
    return H ^ (H >> 16u);
}

float AsphaltHash01(
    const int32 X,
    const int32 Y,
    const uint32 Seed)
{
    return static_cast<float>(AsphaltHash2D(X, Y, Seed) & 0x00FFFFFFu) /
        static_cast<float>(0x00FFFFFFu);
}

int32 WrapCell(
    const int32 Value,
    const int32 Period)
{
    const int32 M = Value % Period;
    return M < 0 ? M + Period : M;
}

float PeriodicValueNoise(
    const float U,
    const float V,
    const int32 Cells,
    const uint32 Seed)
{
    const float X = U * static_cast<float>(Cells);
    const float Y = V * static_cast<float>(Cells);
    const int32 X0 = FMath::FloorToInt(X);
    const int32 Y0 = FMath::FloorToInt(Y);
    const int32 X1 = X0 + 1;
    const int32 Y1 = Y0 + 1;

    const float Tx0 = X - static_cast<float>(X0);
    const float Ty0 = Y - static_cast<float>(Y0);
    const float Tx = Tx0 * Tx0 * (3.0f - 2.0f * Tx0);
    const float Ty = Ty0 * Ty0 * (3.0f - 2.0f * Ty0);

    const float A = AsphaltHash01(
        WrapCell(X0, Cells), WrapCell(Y0, Cells), Seed);
    const float B = AsphaltHash01(
        WrapCell(X1, Cells), WrapCell(Y0, Cells), Seed);
    const float C = AsphaltHash01(
        WrapCell(X0, Cells), WrapCell(Y1, Cells), Seed);
    const float D = AsphaltHash01(
        WrapCell(X1, Cells), WrapCell(Y1, Cells), Seed);

    return FMath::Lerp(
        FMath::Lerp(A, B, Tx),
        FMath::Lerp(C, D, Tx),
        Ty);
}

struct FAsphaltTextureSet
{
    TArray<FColor> Albedo;
    TArray<FColor> Roughness;
    TArray<FColor> Normal;
};

FAsphaltTextureSet BuildAsphaltTextureSet(
    const int32 Size)
{
    FAsphaltTextureSet Result;
    Result.Albedo.SetNumUninitialized(Size * Size);
    Result.Roughness.SetNumUninitialized(Size * Size);
    Result.Normal.SetNumUninitialized(Size * Size);

    TArray<float> Height;
    Height.SetNumUninitialized(Size * Size);

    for (int32 Y = 0; Y < Size; ++Y)
    {
        const float V = static_cast<float>(Y) / static_cast<float>(Size);
        for (int32 X = 0; X < Size; ++X)
        {
            const float U = static_cast<float>(X) / static_cast<float>(Size);
            const int32 Index = Y * Size + X;

            const float Macro =
                PeriodicValueNoise(U, V, 4, 0xA51u);
            const float Mid =
                PeriodicValueNoise(U, V, 18, 0x71Bu);
            const float Fine =
                PeriodicValueNoise(U, V, 72, 0xC31u);
            const float Grain =
                AsphaltHash01(X, Y, 0x2D9u);
            const float PatchField =
                PeriodicValueNoise(U, V, 7, 0x99Du);
            const float Patch =
                FMath::SmoothStep(0.58f, 0.82f, PatchField);

            const float Warp =
                PeriodicValueNoise(U, V, 5, 0xB17u) - 0.5f;
            const float CrackWave = FMath::Abs(
                FMath::Sin(
                    (V * 5.0f + U * 1.45f + Warp * 0.9f) *
                    2.0f * PI));
            const float CrackGate =
                FMath::SmoothStep(
                    0.63f,
                    0.82f,
                    PeriodicValueNoise(U, V, 3, 0xE45u));
            const float Crack =
                (1.0f - FMath::SmoothStep(0.0f, 0.075f, CrackWave)) *
                CrackGate;

            const float LightChip =
                FMath::SmoothStep(0.976f, 0.997f, Grain);
            const float DarkPit =
                1.0f - FMath::SmoothStep(0.010f, 0.032f, Grain);

            const float Base =
                52.0f +
                (Macro - 0.5f) * 17.0f +
                (Mid - 0.5f) * 11.0f +
                (Fine - 0.5f) * 7.0f -
                Patch * 7.0f -
                Crack * 23.0f +
                LightChip * 21.0f -
                DarkPit * 15.0f;

            const uint8 R = static_cast<uint8>(FMath::Clamp(
                FMath::RoundToInt(Base + 2.0f), 24, 96));
            const uint8 G = static_cast<uint8>(FMath::Clamp(
                FMath::RoundToInt(Base + 1.0f), 23, 94));
            const uint8 B = static_cast<uint8>(FMath::Clamp(
                FMath::RoundToInt(Base), 22, 92));
            Result.Albedo[Index] = FColor(R, G, B, 255);

            const float Rough =
                0.89f +
                (Fine - 0.5f) * 0.09f +
                (Mid - 0.5f) * 0.05f +
                Patch * 0.035f +
                Crack * 0.045f -
                LightChip * 0.025f;
            const uint8 RoughByte = static_cast<uint8>(FMath::Clamp(
                FMath::RoundToInt(Rough * 255.0f), 205, 247));
            Result.Roughness[Index] =
                FColor(RoughByte, RoughByte, RoughByte, 255);

            Height[Index] =
                (Mid - 0.5f) * 0.30f +
                (Fine - 0.5f) * 0.52f +
                (Grain - 0.5f) * 0.16f -
                Crack * 0.38f -
                DarkPit * 0.20f +
                LightChip * 0.10f;
        }
    }

    auto HeightAt = [&Height, Size](const int32 X, const int32 Y)
    {
        const int32 WrappedX = WrapCell(X, Size);
        const int32 WrappedY = WrapCell(Y, Size);
        return Height[WrappedY * Size + WrappedX];
    };

    constexpr float NormalStrength = 1.85f;
    for (int32 Y = 0; Y < Size; ++Y)
    {
        for (int32 X = 0; X < Size; ++X)
        {
            const float Dx =
                (HeightAt(X + 1, Y) - HeightAt(X - 1, Y)) *
                NormalStrength;
            const float Dy =
                (HeightAt(X, Y + 1) - HeightAt(X, Y - 1)) *
                NormalStrength;

            const FVector3f N =
                FVector3f(-Dx, -Dy, 1.0f).GetSafeNormal();
            const uint8 NX = static_cast<uint8>(FMath::Clamp(
                FMath::RoundToInt((N.X * 0.5f + 0.5f) * 255.0f), 0, 255));
            const uint8 NY = static_cast<uint8>(FMath::Clamp(
                FMath::RoundToInt((N.Y * 0.5f + 0.5f) * 255.0f), 0, 255));
            const uint8 NZ = static_cast<uint8>(FMath::Clamp(
                FMath::RoundToInt((N.Z * 0.5f + 0.5f) * 255.0f), 0, 255));
            Result.Normal[Y * Size + X] = FColor(NX, NY, NZ, 255);
        }
    }

    return Result;
}

UTexture2D* CreateOrUpdateAsphaltTexture(
    const TCHAR* PackageName,
    const TCHAR* AssetName,
    const TArray<FColor>& Pixels,
    const int32 Size,
    const bool bSRGB,
    const TextureCompressionSettings CompressionSettings,
    FAutomationTestBase& Test)
{
    const FString ObjectPath = FString::Printf(
        TEXT("%s.%s"), PackageName, AssetName);
    UTexture2D* Texture = LoadObject<UTexture2D>(
        nullptr, *ObjectPath);
    bool bNewAsset = false;

    if (!Texture)
    {
        UPackage* Package = CreatePackage(PackageName);
        Test.TestNotNull(
            *FString::Printf(TEXT("%s package created"), AssetName),
            Package);
        if (!Package)
        {
            return nullptr;
        }

        Texture = NewObject<UTexture2D>(
            Package,
            FName(AssetName),
            RF_Public | RF_Standalone | RF_Transactional);
        bNewAsset = true;
    }

    Test.TestNotNull(
        *FString::Printf(TEXT("%s texture created"), AssetName),
        Texture);
    if (!Texture)
    {
        return nullptr;
    }

    Texture->PreEditChange(nullptr);
    Texture->Source.Init(
        Size,
        Size,
        1,
        1,
        TSF_BGRA8,
        reinterpret_cast<const uint8*>(Pixels.GetData()));
    Texture->SRGB = bSRGB;
    Texture->CompressionSettings = CompressionSettings;
    Texture->bNormalizeNormals = CompressionSettings == TC_Normalmap;
    Texture->LODGroup = TEXTUREGROUP_World;
    Texture->AddressX = TA_Wrap;
    Texture->AddressY = TA_Wrap;
    Texture->MipGenSettings = TMGS_FromTextureGroup;

    if (bNewAsset)
    {
        FAssetRegistryModule::AssetCreated(Texture);
    }
    Texture->PostEditChange();

    if (!SaveAssetPackage(*Texture, Test))
    {
        return nullptr;
    }
    return Texture;
}

UMaterial* CreateRoadVisualV2Material(
    FAutomationTestBase& Test)
{
    constexpr int32 TextureSize = 1024;
    const FAsphaltTextureSet Pixels =
        BuildAsphaltTextureSet(TextureSize);

    UTexture2D* AlbedoTexture = CreateOrUpdateAsphaltTexture(
        TEXT("/Game/World/L1/Road/Textures/T_PC_L1_Asphalt_Albedo"),
        TEXT("T_PC_L1_Asphalt_Albedo"),
        Pixels.Albedo,
        TextureSize,
        true,
        TC_Default,
        Test);
    UTexture2D* RoughnessTexture = CreateOrUpdateAsphaltTexture(
        TEXT("/Game/World/L1/Road/Textures/T_PC_L1_Asphalt_Roughness"),
        TEXT("T_PC_L1_Asphalt_Roughness"),
        Pixels.Roughness,
        TextureSize,
        false,
        TC_Masks,
        Test);
    UTexture2D* NormalTexture = CreateOrUpdateAsphaltTexture(
        TEXT("/Game/World/L1/Road/Textures/T_PC_L1_Asphalt_Normal"),
        TEXT("T_PC_L1_Asphalt_Normal"),
        Pixels.Normal,
        TextureSize,
        false,
        TC_Normalmap,
        Test);

    const bool bTexturesReady =
        AlbedoTexture && RoughnessTexture && NormalTexture;
    Test.TestTrue(
        TEXT("R4 asphalt texture set generated"),
        bTexturesReady);
    if (!bTexturesReady)
    {
        return nullptr;
    }

    const TCHAR* PackageName =
        TEXT("/Game/World/L1/Road/Materials/M_PC_L1_Asphalt");
    const TCHAR* AssetName = TEXT("M_PC_L1_Asphalt");
    const FString ObjectPath = FString::Printf(
        TEXT("%s.%s"), PackageName, AssetName);

    UMaterial* Material = LoadObject<UMaterial>(
        nullptr, *ObjectPath);
    if (!Material)
    {
        UPackage* Package = CreatePackage(PackageName);
        Test.TestNotNull(TEXT("R4 asphalt package created"), Package);
        if (!Package)
        {
            return nullptr;
        }

        Material = NewObject<UMaterial>(
            Package,
            FName(AssetName),
            RF_Public | RF_Standalone | RF_Transactional);
        Test.TestNotNull(
            TEXT("R4 project-owned asphalt material created"),
            Material);
        if (!Material)
        {
            return nullptr;
        }
        FAssetRegistryModule::AssetCreated(Material);
    }

    Material->PreEditChange(nullptr);
    UMaterialEditingLibrary::DeleteAllMaterialExpressions(Material);

    auto* WorldPosition = Cast<UMaterialExpressionWorldPosition>(
        UMaterialEditingLibrary::CreateMaterialExpression(
            Material,
            UMaterialExpressionWorldPosition::StaticClass(),
            -1200,
            0));
    auto* WorldXY = Cast<UMaterialExpressionComponentMask>(
        UMaterialEditingLibrary::CreateMaterialExpression(
            Material,
            UMaterialExpressionComponentMask::StaticClass(),
            -1000,
            0));
    auto* TextureFrequency = Cast<UMaterialExpressionScalarParameter>(
        UMaterialEditingLibrary::CreateMaterialExpression(
            Material,
            UMaterialExpressionScalarParameter::StaticClass(),
            -1000,
            120));
    auto* TextureUV = Cast<UMaterialExpressionMultiply>(
        UMaterialEditingLibrary::CreateMaterialExpression(
            Material,
            UMaterialExpressionMultiply::StaticClass(),
            -760,
            0));

    auto* AlbedoSample = Cast<UMaterialExpressionTextureSample>(
        UMaterialEditingLibrary::CreateMaterialExpression(
            Material,
            UMaterialExpressionTextureSample::StaticClass(),
            -500,
            -160));
    auto* RoughnessSample = Cast<UMaterialExpressionTextureSample>(
        UMaterialEditingLibrary::CreateMaterialExpression(
            Material,
            UMaterialExpressionTextureSample::StaticClass(),
            -500,
            40));
    auto* NormalSample = Cast<UMaterialExpressionTextureSample>(
        UMaterialEditingLibrary::CreateMaterialExpression(
            Material,
            UMaterialExpressionTextureSample::StaticClass(),
            -500,
            240));

    auto* MacroFrequency = Cast<UMaterialExpressionScalarParameter>(
        UMaterialEditingLibrary::CreateMaterialExpression(
            Material,
            UMaterialExpressionScalarParameter::StaticClass(),
            -1000,
            420));
    auto* MacroPosition = Cast<UMaterialExpressionMultiply>(
        UMaterialEditingLibrary::CreateMaterialExpression(
            Material,
            UMaterialExpressionMultiply::StaticClass(),
            -760,
            420));
    auto* MacroNoise = Cast<UMaterialExpressionNoise>(
        UMaterialEditingLibrary::CreateMaterialExpression(
            Material,
            UMaterialExpressionNoise::StaticClass(),
            -520,
            420));
    auto* MacroDark = Cast<UMaterialExpressionVectorParameter>(
        UMaterialEditingLibrary::CreateMaterialExpression(
            Material,
            UMaterialExpressionVectorParameter::StaticClass(),
            -260,
            350));
    auto* MacroLight = Cast<UMaterialExpressionVectorParameter>(
        UMaterialEditingLibrary::CreateMaterialExpression(
            Material,
            UMaterialExpressionVectorParameter::StaticClass(),
            -260,
            460));
    auto* MacroTint = Cast<UMaterialExpressionLinearInterpolate>(
        UMaterialEditingLibrary::CreateMaterialExpression(
            Material,
            UMaterialExpressionLinearInterpolate::StaticClass(),
            0,
            390));
    auto* TintedAlbedo = Cast<UMaterialExpressionMultiply>(
        UMaterialEditingLibrary::CreateMaterialExpression(
            Material,
            UMaterialExpressionMultiply::StaticClass(),
            260,
            -100));
    auto* Specular = Cast<UMaterialExpressionConstant>(
        UMaterialEditingLibrary::CreateMaterialExpression(
            Material,
            UMaterialExpressionConstant::StaticClass(),
            260,
            160));

    const bool bAllNodes =
        WorldPosition && WorldXY && TextureFrequency && TextureUV &&
        AlbedoSample && RoughnessSample && NormalSample &&
        MacroFrequency && MacroPosition && MacroNoise &&
        MacroDark && MacroLight && MacroTint && TintedAlbedo &&
        Specular;
    Test.TestTrue(
        TEXT("R4 asphalt texture-backed graph nodes created"),
        bAllNodes);
    if (!bAllNodes)
    {
        return nullptr;
    }

    WorldXY->R = true;
    WorldXY->G = true;
    WorldXY->B = false;
    WorldXY->A = false;

    TextureFrequency->ParameterName =
        TEXT("TextureFrequency");
    // 0.000833 world-cm scale = roughly one 1024 texture tile per 12 m.
    TextureFrequency->DefaultValue = 0.0008333333f;

    AlbedoSample->Texture = AlbedoTexture;
    AlbedoSample->SamplerType = SAMPLERTYPE_Color;
    RoughnessSample->Texture = RoughnessTexture;
    RoughnessSample->SamplerType = SAMPLERTYPE_Masks;
    NormalSample->Texture = NormalTexture;
    NormalSample->SamplerType = SAMPLERTYPE_Normal;

    MacroFrequency->ParameterName =
        TEXT("MacroFrequency");
    MacroFrequency->DefaultValue = 0.000055f;
    MacroNoise->NoiseFunction =
        static_cast<ENoiseFunction>(1);
    MacroNoise->Quality = 1;
    MacroNoise->Levels = 2;
    MacroNoise->Scale = 1.0f;
    MacroNoise->OutputMin = 0.0f;
    MacroNoise->OutputMax = 1.0f;
    MacroNoise->bTurbulence = false;

    MacroDark->ParameterName =
        TEXT("MacroDarkTint");
    MacroDark->DefaultValue =
        FLinearColor(0.78f, 0.79f, 0.80f, 1.0f);
    MacroLight->ParameterName =
        TEXT("MacroLightTint");
    MacroLight->DefaultValue =
        FLinearColor(1.10f, 1.08f, 1.04f, 1.0f);

    // Keep dielectric reflection low. Wetness comes from roughness variation,
    // not from a uniform oily specular coat.
    Specular->R = 0.07f;

    UMaterialEditingLibrary::ConnectMaterialExpressions(
        WorldPosition, FString(), WorldXY, TEXT("Input"));
    UMaterialEditingLibrary::ConnectMaterialExpressions(
        WorldXY, FString(), TextureUV, TEXT("A"));
    UMaterialEditingLibrary::ConnectMaterialExpressions(
        TextureFrequency, FString(), TextureUV, TEXT("B"));

    UMaterialEditingLibrary::ConnectMaterialExpressions(
        TextureUV, FString(), AlbedoSample, TEXT("Coordinates"));
    UMaterialEditingLibrary::ConnectMaterialExpressions(
        TextureUV, FString(), RoughnessSample, TEXT("Coordinates"));
    UMaterialEditingLibrary::ConnectMaterialExpressions(
        TextureUV, FString(), NormalSample, TEXT("Coordinates"));

    UMaterialEditingLibrary::ConnectMaterialExpressions(
        WorldPosition, FString(), MacroPosition, TEXT("A"));
    UMaterialEditingLibrary::ConnectMaterialExpressions(
        MacroFrequency, FString(), MacroPosition, TEXT("B"));
    UMaterialEditingLibrary::ConnectMaterialExpressions(
        MacroPosition, FString(), MacroNoise, TEXT("Position"));
    UMaterialEditingLibrary::ConnectMaterialExpressions(
        MacroDark, FString(), MacroTint, TEXT("A"));
    UMaterialEditingLibrary::ConnectMaterialExpressions(
        MacroLight, FString(), MacroTint, TEXT("B"));
    UMaterialEditingLibrary::ConnectMaterialExpressions(
        MacroNoise, FString(), MacroTint, TEXT("Alpha"));
    UMaterialEditingLibrary::ConnectMaterialExpressions(
        AlbedoSample, FString(), TintedAlbedo, TEXT("A"));
    UMaterialEditingLibrary::ConnectMaterialExpressions(
        MacroTint, FString(), TintedAlbedo, TEXT("B"));

    UMaterialEditingLibrary::ConnectMaterialProperty(
        TintedAlbedo, FString(), MP_BaseColor);
    UMaterialEditingLibrary::ConnectMaterialProperty(
        RoughnessSample, TEXT("R"), MP_Roughness);
    UMaterialEditingLibrary::ConnectMaterialProperty(
        NormalSample, FString(), MP_Normal);
    UMaterialEditingLibrary::ConnectMaterialProperty(
        Specular, FString(), MP_Specular);

    const TArray<FString> CompileErrors =
        UMaterialEditingLibrary::RecompileMaterial(Material);
    for (const FString& Error : CompileErrors)
    {
        Test.AddError(
            FString::Printf(
                TEXT("R4 asphalt compile: %s"),
                *Error));
    }
    if (CompileErrors.Num() > 0)
    {
        return nullptr;
    }

    Material->PostEditChange();
    if (!SaveAssetPackage(*Material, Test))
    {
        return nullptr;
    }

    Test.AddInfo(FString::Printf(
        TEXT("CD869_R4_ASPHALT_TEXTURE_GRAPH=PASS expressions=%d textures=3 specular=0.07"),
        UMaterialEditingLibrary::GetNumMaterialExpressions(Material)));
    return Material;
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
            Test->TestTrue(TEXT("baked road width matches owner-requested 79.0m envelope"),
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
    FPinkCabGenerateL1RoadR4VisualMaterial,
    "PinkCab.Editor.GenerateL1RoadR4VisualMaterial",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabGenerateL1RoadR4VisualMaterial::RunTest(
    const FString& Parameters)
{
    using namespace PinkCabL1MetaRoadAuthoring;

    UMaterial* Asphalt = CreateRoadVisualV2Material(*this);
    TestNotNull(TEXT("R4 asphalt material generated"), Asphalt);
    if (!Asphalt)
    {
        return false;
    }

    // RoadSurface may already be loaded in this editor process while the legacy
    // asphalt file is intentionally absent for clean regeneration. Its serialized
    // material reference is verified in a fresh editor process by the R4 GREEN test.
    AddInfo(TEXT("CD869_R4_VISUAL_MATERIAL_GENERATED=PASS"));
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

    UMaterial* Asphalt = CreateRoadVisualV2Material(*this);
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

    TArray<FString> GeneratedMeshNames = {
        TEXT("RoadSurface"),
        TEXT("RoadSidewalks")
    };
    for (int32 Index = 0; Index < 32; ++Index)
    {
        GeneratedMeshNames.Add(
            Index == 0
                ? TEXT("RoadCurbs")
                : FString::Printf(TEXT("RoadCurbs%d"), Index));
    }
    for (int32 Index = 0; Index < 50; ++Index)
    {
        GeneratedMeshNames.Add(
            Index == 0
                ? TEXT("RoadMarks")
                : FString::Printf(TEXT("RoadMarks%d"), Index));
    }

    int32 ReboundMeshCount = 0;
    for (const FString& MeshName : GeneratedMeshNames)
    {
        const FString ObjectPath = FString::Printf(
            TEXT("/Game/World/L1/Road/%s.%s"),
            *MeshName,
            *MeshName);
        UStaticMesh* Mesh = LoadObject<UStaticMesh>(
            nullptr,
            *ObjectPath);
        TestNotNull(
            *FString::Printf(
                TEXT("native MetaRoad mesh loads for runtime ownership: %s"),
                *MeshName),
            Mesh);
        if (!Mesh)
        {
            continue;
        }

        TArray<FStaticMaterial>& Slots = Mesh->GetStaticMaterials();
        TestTrue(
            *FString::Printf(TEXT("%s exposes material slots"), *MeshName),
            Slots.Num() > 0);

        const bool bCurbMesh =
            MeshName.StartsWith(TEXT("RoadCurbs"));
        const bool bMarkMesh =
            MeshName.StartsWith(TEXT("RoadMarks"));
        for (int32 Index = 0; Index < Slots.Num(); ++Index)
        {
            const FString SlotName = Slots[Index].MaterialSlotName.ToString();
            AddInfo(FString::Printf(
                TEXT("CD869_NATIVE_SLOT[%s][%d]=%s"),
                *MeshName,
                Index,
                *SlotName));
            Mesh->SetMaterial(
                Index,
                bMarkMesh
                    ? Mark
                    : bCurbMesh
                        ? Shoulder
                        : ResolveProjectMaterialForSlot(
                            Slots[Index],
                            Asphalt,
                            Divider,
                            Shoulder,
                            Mark));
        }

        Mesh->PostEditChange();
        TestTrue(
            *FString::Printf(
                TEXT("%s saved after project material rebinding"),
                *MeshName),
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
        TEXT("all native MetaRoad R2 construction and R3 mark meshes rebound for runtime"),
        ReboundMeshCount,
        GeneratedMeshNames.Num());

    AddInfo(TEXT("CD869_RUNTIME_MATERIAL_OWNERSHIP=PASS"));
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabL1RoadR4RoadVisualV2,
    "PinkCab.World.L1Road.R4.RoadVisualV2",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabL1RoadR4RoadVisualV2::RunTest(const FString& Parameters)
{
    UMaterial* Asphalt = LoadObject<UMaterial>(
        nullptr,
        TEXT("/Game/World/L1/Road/Materials/M_PC_L1_Asphalt.M_PC_L1_Asphalt"));
    TestNotNull(TEXT("R4 project-owned asphalt material loads"), Asphalt);
    if (!Asphalt)
    {
        return false;
    }

    UMaterialExpression* BaseColorInput =
        UMaterialEditingLibrary::GetMaterialPropertyInputNode(Asphalt, MP_BaseColor);
    UMaterialExpression* RoughnessInput =
        UMaterialEditingLibrary::GetMaterialPropertyInputNode(Asphalt, MP_Roughness);
    UMaterialExpression* NormalInput =
        UMaterialEditingLibrary::GetMaterialPropertyInputNode(Asphalt, MP_Normal);
    UMaterialExpression* SpecularInput =
        UMaterialEditingLibrary::GetMaterialPropertyInputNode(Asphalt, MP_Specular);

    TestNotNull(TEXT("R4 asphalt has BaseColor graph input"), BaseColorInput);
    TestNotNull(TEXT("R4 asphalt has Roughness graph input"), RoughnessInput);
    TestNotNull(TEXT("R4 asphalt has real Normal graph input"), NormalInput);
    TestNotNull(TEXT("R4 asphalt has explicit low-specular input"), SpecularInput);
    if (!BaseColorInput || !RoughnessInput || !NormalInput || !SpecularInput)
    {
        return false;
    }

    TestFalse(
        TEXT("R4 BaseColor is no longer a flat constant"),
        BaseColorInput->IsA(UMaterialExpressionConstant3Vector::StaticClass()));
    TestFalse(
        TEXT("R4 Roughness is no longer a flat constant"),
        RoughnessInput->IsA(UMaterialExpressionConstant::StaticClass()));

    UTexture2D* AlbedoTexture = LoadObject<UTexture2D>(
        nullptr,
        TEXT("/Game/World/L1/Road/Textures/T_PC_L1_Asphalt_Albedo.T_PC_L1_Asphalt_Albedo"));
    UTexture2D* RoughnessTexture = LoadObject<UTexture2D>(
        nullptr,
        TEXT("/Game/World/L1/Road/Textures/T_PC_L1_Asphalt_Roughness.T_PC_L1_Asphalt_Roughness"));
    UTexture2D* NormalTexture = LoadObject<UTexture2D>(
        nullptr,
        TEXT("/Game/World/L1/Road/Textures/T_PC_L1_Asphalt_Normal.T_PC_L1_Asphalt_Normal"));
    TestNotNull(TEXT("R4 asphalt owns a real albedo texture"), AlbedoTexture);
    TestNotNull(TEXT("R4 asphalt owns a real roughness texture"), RoughnessTexture);
    TestNotNull(TEXT("R4 asphalt owns a real normal texture"), NormalTexture);

    const UMaterialExpressionConstant* SpecularConstant =
        Cast<UMaterialExpressionConstant>(SpecularInput);
    TestNotNull(TEXT("R4 asphalt specular remains an explicit constant"), SpecularConstant);
    if (SpecularConstant)
    {
        TestTrue(
            TEXT("R4 asphalt specular is low enough to avoid an oil-polished road"),
            SpecularConstant->R <= 0.10f);
    }

    UStaticMesh* RoadSurface = LoadObject<UStaticMesh>(
        nullptr,
        TEXT("/Game/World/L1/Road/RoadSurface.RoadSurface"));
    TestNotNull(TEXT("accepted R3 RoadSurface still loads"), RoadSurface);
    if (RoadSurface && RoadSurface->GetStaticMaterials().Num() > 0)
    {
        UMaterialInterface* RoadMaterial =
            RoadSurface->GetStaticMaterials()[0].MaterialInterface;
        TestNotNull(TEXT("RoadSurface keeps assigned runtime material"), RoadMaterial);
        if (RoadMaterial)
        {
            TestEqual(
                TEXT("RoadSurface uses project-owned R4 asphalt"),
                RoadMaterial->GetPathName(),
                FString(TEXT("/Game/World/L1/Road/Materials/M_PC_L1_Asphalt.M_PC_L1_Asphalt")));
        }
    }

    int32 VerifiedMarks = 0;
    for (int32 Index = 0; Index < 50; ++Index)
    {
        const FString Name =
            Index == 0 ? TEXT("RoadMarks") : FString::Printf(TEXT("RoadMarks%d"), Index);
        UStaticMesh* Mark = LoadObject<UStaticMesh>(
            nullptr,
            *FString::Printf(TEXT("/Game/World/L1/Road/%s.%s"), *Name, *Name));
        TestNotNull(*FString::Printf(TEXT("frozen R3 mark %s loads"), *Name), Mark);
        if (!Mark)
        {
            continue;
        }

        for (const FStaticMaterial& Slot : Mark->GetStaticMaterials())
        {
            UMaterialInterface* Material = Slot.MaterialInterface;
            TestNotNull(TEXT("frozen R3 mark retains material"), Material);
            if (Material)
            {
                TestEqual(
                    TEXT("R3 mark material identity remains frozen"),
                    Material->GetPathName(),
                    FString(TEXT("/Game/World/L1/Road/Materials/M_PC_L1_Mark.M_PC_L1_Mark")));
            }
        }
        ++VerifiedMarks;
    }

    TestEqual(TEXT("all 50 frozen R3 mark meshes verified"), VerifiedMarks, 50);
    AddInfo(TEXT("CD869_R4_VISUAL_CONTRACT_CHECKED=1"));
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabL1RoadR4FrozenHGate,
    "PinkCab.World.L1Road.R4.FrozenHGate",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabL1RoadR4FrozenHGate::RunTest(const FString& Parameters)
{
    auto SelectFromNeutral = [this](
        const float HorizontalCounts,
        const float VerticalCounts,
        const int32 ExpectedGear,
        const TCHAR* Label)
    {
        FPinkCabHGateState State;
        FPinkCabHGateGeometry::ResetToGear(State, 0);

        if (!FMath::IsNearlyZero(HorizontalCounts))
        {
            FPinkCabHGateGeometry::ApplyDriverDelta(
                State, HorizontalCounts, 0.0f);
            TestEqual(
                *FString::Printf(TEXT("%s stays neutral across cross-gate"), Label),
                State.RequestedGear,
                0);
        }

        FPinkCabHGateGeometry::ApplyDriverDelta(
            State, 0.0f, VerticalCounts);
        TestEqual(
            *FString::Printf(TEXT("%s resolves accepted gear"), Label),
            State.RequestedGear,
            ExpectedGear);
    };

    SelectFromNeutral(-640.0f, 480.0f, 1, TEXT("extreme-left forward"));
    SelectFromNeutral(-640.0f, -480.0f, 2, TEXT("extreme-left back"));
    SelectFromNeutral(0.0f, 480.0f, 3, TEXT("broad-middle forward"));
    SelectFromNeutral(0.0f, -480.0f, 4, TEXT("broad-middle back"));
    SelectFromNeutral(320.0f, 480.0f, 5, TEXT("extreme-right forward"));
    SelectFromNeutral(320.0f, -480.0f, -1, TEXT("extreme-right back"));

    FPinkCabHGateState State;
    FPinkCabHGateGeometry::ResetToGear(State, 5);
    FPinkCabHGateGeometry::ApplyDriverDelta(State, 0.0f, -480.0f);
    TestEqual(
        TEXT("accepted fore-aft travel leaves fifth through neutral"),
        State.RequestedGear,
        0);

    FPinkCabHGateGeometry::ApplyDriverDelta(State, -960.0f, 0.0f);
    TestEqual(
        TEXT("accepted neutral cross-gate stays neutral"),
        State.RequestedGear,
        0);

    FPinkCabHGateGeometry::ApplyDriverDelta(State, 0.0f, -480.0f);
    TestEqual(
        TEXT("accepted left/back after neutral selects second"),
        State.RequestedGear,
        2);

    AddInfo(TEXT("CD869_R4_FROZEN_HGATE=PASS"));
    return true;
}

#endif
