#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/PackageName.h"
#include "World/PinkCabChaosWeaveCourse.h"
#include "Vehicle/PinkCabChaosTatraPawn.h"
#include "FileHelpers.h"
#include "Editor.h"
#include "Engine/World.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabGenerateChaosWeaveMap,
    "PinkCab.Editor.GenerateChaosWeaveMap",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabGenerateChaosWeaveMap::RunTest(const FString& Parameters)
{
    const FString MapPackage = TEXT("/Game/Dev/Maps/L_PinkCab_ChaosWeave");
    if (FPackageName::DoesPackageExist(MapPackage))
    {
        TestTrue(TEXT("existing Chaos weave map is reused"), true);
        return true;
    }

    UWorld* World = UEditorLoadingAndSavingUtils::NewBlankMap(false);
    TestNotNull(TEXT("blank editor world created"), World);
    if (!World) return false;

    FActorSpawnParameters SpawnParams;
    SpawnParams.OverrideLevel = World->PersistentLevel;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    APinkCabChaosWeaveCourse* Course = World->SpawnActor<APinkCabChaosWeaveCourse>(
        APinkCabChaosWeaveCourse::StaticClass(), FTransform::Identity, SpawnParams);
    TestNotNull(TEXT("weave course spawned"), Course);
    if (!Course) return false;

    APinkCabChaosTatraPawn* Pawn = World->SpawnActor<APinkCabChaosTatraPawn>(
        APinkCabChaosTatraPawn::StaticClass(),
        APinkCabChaosWeaveCourse::GetPawnSpawnTransform(),
        SpawnParams);
    TestNotNull(TEXT("PinkCab Chaos pawn spawned"), Pawn);
    if (!Pawn) return false;

#if WITH_EDITOR
    Course->SetActorLabel(TEXT("PC_Chaos_Weave_Course"));
    Pawn->SetActorLabel(TEXT("PC_Chaos_Tatra_Placeholder"));
#endif
    Course->SetFlags(RF_Transactional);
    Pawn->SetFlags(RF_Transactional);
    Course->MarkPackageDirty();
    Pawn->MarkPackageDirty();

    const bool bSaved = UEditorLoadingAndSavingUtils::SaveMap(World, MapPackage);
    TestTrue(TEXT("Chaos weave map saved"), bSaved);
    return bSaved;
}

#endif
