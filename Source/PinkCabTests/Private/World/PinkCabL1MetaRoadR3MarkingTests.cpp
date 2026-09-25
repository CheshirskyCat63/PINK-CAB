#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "Assets/RoadLaneAttributeMark.h"
#include "Assets/RoadMarkProfile.h"
#include "Assets/RoadProfile.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "PhysicsEngine/BodySetup.h"
#include "World/PinkCabL1RoadChunkActor.h"

namespace PinkCabL1MetaRoadR3
{
const FRoadLaneMark* FindPresetMark(
    const URoadProfile& Profile,
    const bool bRequireBroken)
{
    auto FindInLanes = [bRequireBroken](
        const TArray<FRoadLaneProfile>& Lanes) -> const FRoadLaneMark*
    {
        for (const FRoadLaneProfile& Lane : Lanes)
        {
            for (const FRoadLaneAttributeProfile& Attribute : Lane.Attributes)
            {
                UClass* Descriptor = Attribute.AttributeDesctiptor.LoadSynchronous();
                if (Descriptor != URoadLaneAttributeMarkDescriptor::StaticClass())
                {
                    continue;
                }

                const FRoadLaneMark* Mark =
                    Attribute.AttributeValueTemplate.GetPtr<FRoadLaneMark>();
                if (!Mark ||
                    Mark->ProfileSource != ERoadLaneMarkProfile::UsePreset ||
                    Mark->Profile.IsNull())
                {
                    continue;
                }

                Mark->Profile.LoadSynchronous();
                if (!bRequireBroken ||
                    Mark->GetProfile().GetPtr<FRoadLaneMarkProfileBroken>() != nullptr)
                {
                    return Mark;
                }
            }
        }
        return nullptr;
    };

    if (const FRoadLaneMark* Mark = FindInLanes(Profile.Left))
    {
        return Mark;
    }
    return FindInLanes(Profile.Right);
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabL1NativeMetaRoadR3MarkingTest,
    "PinkCab.World.L1Road.R3.NativeMetaRoadMarkings",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabL1NativeMetaRoadR3MarkingTest::RunTest(const FString& Parameters)
{
    using namespace PinkCabL1MetaRoadR3;

    URoadProfile* ExpressProfile = LoadObject<URoadProfile>(
        nullptr,
        TEXT("/MetaRoad/MetaRoad/Profiles/RoadProfiles/4_Lanes+Borders.4_Lanes+Borders"));
    URoadProfile* LocalProfile = LoadObject<URoadProfile>(
        nullptr,
        TEXT("/MetaRoad/MetaRoad/Profiles/RoadProfiles/2_Lanes+Borders_Coodirect.2_Lanes+Borders_Coodirect"));

    TestNotNull(TEXT("native MetaRoad 4_Lanes+Borders profile loads"), ExpressProfile);
    TestNotNull(TEXT("native MetaRoad 2_Lanes+Borders_Coodirect profile loads"), LocalProfile);

    const FRoadLaneMark* ExpressBroken =
        ExpressProfile ? FindPresetMark(*ExpressProfile, true) : nullptr;
    const FRoadLaneMark* LocalBroken =
        LocalProfile ? FindPresetMark(*LocalProfile, true) : nullptr;

    TestNotNull(TEXT("express reference exposes a native broken mark preset"), ExpressBroken);
    TestNotNull(TEXT("local reference exposes a native broken mark preset"), LocalBroken);

    for (const FRoadLaneMark* Mark : { ExpressBroken, LocalBroken })
    {
        if (!Mark)
        {
            continue;
        }
        TestEqual(
            TEXT("R3 uses MetaRoad mark preset rather than custom profile"),
            Mark->ProfileSource,
            ERoadLaneMarkProfile::UsePreset);
        TestTrue(
            TEXT("native marking preset comes from MetaRoad Profiles/Marks"),
            Mark->Profile.ToSoftObjectPath().ToString().StartsWith(
                TEXT("/MetaRoad/MetaRoad/Profiles/Marks/")));
        Mark->Profile.LoadSynchronous();
        TestNotNull(
            TEXT("native marking preset resolves to broken geometry"),
            Mark->GetProfile().GetPtr<FRoadLaneMarkProfileBroken>());
    }

    UStaticMesh* RoadMarks = LoadObject<UStaticMesh>(
        nullptr,
        TEXT("/Game/World/L1/Road/RoadMarks.RoadMarks"));
    TestNotNull(TEXT("MetaRoad MarksOp baked canonical RoadMarks mesh"), RoadMarks);

    if (RoadMarks)
    {
        const FVector Size = RoadMarks->GetBounds().GetBox().GetSize();
        TestTrue(
            TEXT("RoadMarks spans the 1000m repeated module"),
            FMath::IsNearlyEqual(Size.X, 100000.0, 250.0));
        TestTrue(
            TEXT("RoadMarks remains inside accepted 66.8m road envelope"),
            Size.Y <= 6680.0 + 5.0);

        for (const FStaticMaterial& Slot : RoadMarks->GetStaticMaterials())
        {
            UMaterialInterface* Material = Slot.MaterialInterface;
            TestNotNull(TEXT("RoadMarks material assigned"), Material);
            if (Material)
            {
                TestTrue(
                    TEXT("runtime RoadMarks material is project-owned"),
                    Material->GetPathName().StartsWith(
                        TEXT("/Game/World/L1/Road/Materials/")));
            }
        }
    }

    const APinkCabL1RoadChunkActor* RuntimeChunk =
        GetDefault<APinkCabL1RoadChunkActor>();
    TestNotNull(TEXT("runtime L1 chunk CDO exists"), RuntimeChunk);

    UStaticMeshComponent* RuntimeMarks = nullptr;
    if (RuntimeChunk)
    {
        TArray<UStaticMeshComponent*> Components;
        RuntimeChunk->GetComponents<UStaticMeshComponent>(Components);
        for (UStaticMeshComponent* Component : Components)
        {
            if (Component &&
                Component->GetStaticMesh() == RoadMarks)
            {
                RuntimeMarks = Component;
                break;
            }
        }
    }

    TestNotNull(TEXT("runtime chunk mounts native MetaRoad RoadMarks"), RuntimeMarks);
    if (RuntimeMarks)
    {
        TestEqual(
            TEXT("R3 road markings never participate in vehicle collision"),
            RuntimeMarks->GetCollisionEnabled(),
            ECollisionEnabled::NoCollision);
        TestFalse(
            TEXT("R3 road markings do not generate overlap events"),
            RuntimeMarks->GetGenerateOverlapEvents());
    }

    AddInfo(TEXT("CD869_R3_NATIVE_METAROAD_MARKINGS=PASS"));
    return true;
}

#endif
