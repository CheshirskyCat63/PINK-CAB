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

    constexpr int32 ExpectedNativeMarkMeshCount = 50;
    TSet<UStaticMesh*> NativeMarkMeshes;
    for (int32 Index = 0; Index < ExpectedNativeMarkMeshCount; ++Index)
    {
        const FString AssetName =
            Index == 0
                ? TEXT("RoadMarks")
                : FString::Printf(TEXT("RoadMarks%d"), Index);
        const FString ObjectPath = FString::Printf(
            TEXT("/Game/World/L1/Road/%s.%s"),
            *AssetName,
            *AssetName);
        UStaticMesh* MarkMesh = LoadObject<UStaticMesh>(
            nullptr,
            *ObjectPath);
        TestNotNull(
            *FString::Printf(
                TEXT("MetaRoad MarksOp baked native mark span %s"),
                *AssetName),
            MarkMesh);
        if (!MarkMesh)
        {
            continue;
        }

        NativeMarkMeshes.Add(MarkMesh);
        for (const FStaticMaterial& Slot : MarkMesh->GetStaticMaterials())
        {
            UMaterialInterface* Material = Slot.MaterialInterface;
            TestNotNull(
                *FString::Printf(TEXT("%s material assigned"), *AssetName),
                Material);
            if (Material)
            {
                TestTrue(
                    *FString::Printf(
                        TEXT("%s runtime material is project-owned"),
                        *AssetName),
                    Material->GetPathName().StartsWith(
                        TEXT("/Game/World/L1/Road/Materials/")));
            }
        }
    }
    TestEqual(
        TEXT("MetaRoad emits all fifty native R3 mark spans"),
        NativeMarkMeshes.Num(),
        ExpectedNativeMarkMeshCount);

    const APinkCabL1RoadChunkActor* RuntimeChunk =
        GetDefault<APinkCabL1RoadChunkActor>();
    TestNotNull(TEXT("runtime L1 chunk CDO exists"), RuntimeChunk);

    FBox RuntimeMarkBounds(ForceInit);
    int32 RuntimeMarkComponentCount = 0;
    if (RuntimeChunk)
    {
        TArray<UStaticMeshComponent*> Components;
        RuntimeChunk->GetComponents<UStaticMeshComponent>(Components);
        for (UStaticMeshComponent* Component : Components)
        {
            if (!Component ||
                !Component->GetStaticMesh() ||
                !NativeMarkMeshes.Contains(Component->GetStaticMesh()))
            {
                continue;
            }

            ++RuntimeMarkComponentCount;
            TestEqual(
                *FString::Printf(
                    TEXT("%s remains collision-free"),
                    *Component->GetName()),
                Component->GetCollisionEnabled(),
                ECollisionEnabled::NoCollision);
            TestFalse(
                *FString::Printf(
                    TEXT("%s does not generate overlaps"),
                    *Component->GetName()),
                Component->GetGenerateOverlapEvents());

            const FBox PlacedBox =
                Component->GetStaticMesh()->GetBounds().GetBox().TransformBy(
                    Component->GetRelativeTransform());
            RuntimeMarkBounds += PlacedBox;
        }
    }

    TestEqual(
        TEXT("runtime chunk mounts all native MetaRoad mark spans"),
        RuntimeMarkComponentCount,
        ExpectedNativeMarkMeshCount);
    TestTrue(
        TEXT("runtime native mark assembly bounds are valid"),
        RuntimeMarkBounds.IsValid != 0);

    if (RuntimeMarkBounds.IsValid)
    {
        const FVector Size = RuntimeMarkBounds.GetSize();
        AddInfo(FString::Printf(
            TEXT("CD869_R3_RUNTIME_MARK_BOUNDS=%.2f,%.2f,%.2f"),
            Size.X,
            Size.Y,
            Size.Z));
        TestTrue(
            TEXT("native broken-mark assembly covers the full kilometre pattern"),
            Size.X >= 99000.0 && Size.X <= 100005.0);
        TestTrue(
            TEXT("native mark assembly stays inside accepted road envelope"),
            Size.Y <= 6680.0 + 5.0);
        TestTrue(
            TEXT("native marks remain a visual surface layer"),
            Size.Z <= 1.0);
    }

    AddInfo(TEXT("CD869_R3_NATIVE_METAROAD_MARKINGS=PASS"));
    return true;
}

#endif
