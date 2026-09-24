#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/PackageName.h"
#include "FileHelpers.h"
#include "Editor.h"
#include "Engine/World.h"
#include "Runtime/PinkCabChaosTatraPawn.h"
#include "World/PinkCabL1EndlessRoadStreamer.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabGenerateL1EndlessRoadMap,
    "PinkCab.Editor.GenerateL1EndlessRoadMap",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabGenerateL1EndlessRoadMap::RunTest(const FString& Parameters)
{
    const FString MapPackage = TEXT("/Game/Dev/Maps/L_PinkCab_L1_EndlessStraight");
    if (FPackageName::DoesPackageExist(MapPackage))
    {
        TestTrue(TEXT("existing endless Level 1 candidate map is reused"), true);
        return true;
    }

    UWorld* World = UEditorLoadingAndSavingUtils::NewBlankMap(false);
    TestNotNull(TEXT("blank editor world created"), World);
    if (!World)
    {
        return false;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.OverrideLevel = World->PersistentLevel;
    SpawnParams.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    // Start halfway through logical chunk zero, on the middle positive-X
    // express lane. Z matches the proven Chaos-weave spawn clearance.
    const FTransform PawnTransform(
        FRotator::ZeroRotator,
        FVector(50000.0, 1300.0, 180.0));

    APinkCabChaosTatraPawn* Pawn =
        World->SpawnActor<APinkCabChaosTatraPawn>(
            APinkCabChaosTatraPawn::StaticClass(),
            PawnTransform,
            SpawnParams);
    TestNotNull(TEXT("current PinkCab Chaos Tatra spawned"), Pawn);
    if (!Pawn)
    {
        return false;
    }

    APinkCabL1EndlessRoadStreamer* Streamer =
        World->SpawnActor<APinkCabL1EndlessRoadStreamer>(
            APinkCabL1EndlessRoadStreamer::StaticClass(),
            FTransform::Identity,
            SpawnParams);
    TestNotNull(TEXT("endless-road streamer spawned"), Streamer);
    if (!Streamer)
    {
        return false;
    }

    Streamer->SetTrackedActor(Pawn);

#if WITH_EDITOR
    Pawn->SetActorLabel(TEXT("PC_L1_Tatra"));
    Streamer->SetActorLabel(TEXT("PC_L1_EndlessRoadStreamer"));
#endif

    Pawn->SetFlags(RF_Transactional);
    Streamer->SetFlags(RF_Transactional);
    Pawn->MarkPackageDirty();
    Streamer->MarkPackageDirty();

    const bool bSaved =
        UEditorLoadingAndSavingUtils::SaveMap(World, MapPackage);
    TestTrue(TEXT("endless Level 1 candidate map saved"), bSaved);
    return bSaved;
}

#endif
