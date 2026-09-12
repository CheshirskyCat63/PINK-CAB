#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "World/PinkCabGreyboxAuthorityTag.h"
#include "World/PinkCabL1GreyboxCorridor.h"
#include "FileHelpers.h"
#include "Editor.h"
#include "Editor/EditorEngine.h"
#include "EngineUtils.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabL1GreyboxAuthorityTest,
    "PinkCab.World.L1Greybox.AuthorityTag",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabL1GreyboxAuthorityTest::RunTest(const FString& Parameters)
{
    TestEqual(TEXT("greybox authority marker"),
        FPinkCabGreyboxAuthorityTag::Marker(),
        FString(TEXT("NON_AUTHORITATIVE_GEOMETRY")));
    TestFalse(TEXT("greybox never claims canonical geometry"),
        FPinkCabGreyboxAuthorityTag::IsAuthoritativeGeometry());
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabL1GreyboxSurfaceContractTest,
    "PinkCab.World.L1Greybox.SurfaceContract",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabL1GreyboxSurfaceContractTest::RunTest(const FString& Parameters)
{
    const APinkCabL1GreyboxCorridor* Corridor = GetDefault<APinkCabL1GreyboxCorridor>();
    TestNotNull(TEXT("corridor class default exists"), Corridor);
    if (!Corridor) return false;

    TestEqual(TEXT("L1 contract exposes five logical lane surfaces"), Corridor->GetLaneSurfaceCount(), 5);
    for (int32 LaneIndex = 0; LaneIndex < Corridor->GetLaneSurfaceCount(); ++LaneIndex)
    {
        const FPinkCabLaneId LaneId = Corridor->GetContractLaneId(LaneIndex);
        TestTrue(TEXT("contract lane id valid"), LaneId.IsValid());
        TestTrue(TEXT("logical lane resolves to primitive surface"), Corridor->HasSurfaceForLane(LaneId));
    }
    TestTrue(TEXT("left contact/debug surface exists"), Corridor->HasLeftContactSurface());
    TestTrue(TEXT("right contact/debug surface exists"), Corridor->HasRightContactSurface());
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabL1GreyboxMapAcceptanceTest,
    "PinkCab.World.L1Greybox.MapAcceptance",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabL1GreyboxMapAcceptanceTest::RunTest(const FString& Parameters)
{
    TestTrue(TEXT("L1 Greybox Zero map loads"),
        FEditorFileUtils::LoadMap(TEXT("/Game/Dev/Maps/L_PinkCab_L1_GreyboxZero"), false, true));
    UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
    TestNotNull(TEXT("editor world exists"), World);
    if (!World) return false;

    int32 CorridorCount = 0;
    for (TActorIterator<APinkCabL1GreyboxCorridor> It(World); It; ++It)
    {
        ++CorridorCount;
        TestEqual(TEXT("saved actor keeps authority marker"), It->GetAuthorityMarker(), FString(TEXT("NON_AUTHORITATIVE_GEOMETRY")));
        TestEqual(TEXT("saved actor keeps five lane surfaces"), It->GetLaneSurfaceCount(), 5);
    }
    TestEqual(TEXT("map contains one contract corridor"), CorridorCount, 1);
    TestTrue(TEXT("MapCheck executes"), GEditor->Exec(World, TEXT("MAP CHECK"), *GLog));
    return true;
}

#endif
