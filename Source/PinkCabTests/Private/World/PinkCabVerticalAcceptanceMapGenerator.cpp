#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/PackageName.h"
#include "World/PinkCabVerticalAcceptanceCourse.h"
#include "Runtime/PinkCabChaosTatraPawn.h"
#include "FileHelpers.h"
#include "Editor.h"
#include "Engine/World.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabGenerateVerticalAcceptanceMap,
    "PinkCab.Editor.GenerateVerticalAcceptanceMap",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabGenerateVerticalAcceptanceMap::RunTest(const FString& Parameters)
{
    const FString MapPackage = TEXT("/Game/Dev/Maps/L_PinkCab_VerticalAcceptance");
    if (FPackageName::DoesPackageExist(MapPackage))
    {
        TestTrue(TEXT("existing vertical acceptance map reused"), true);
        return true;
    }

    UWorld* World = UEditorLoadingAndSavingUtils::NewBlankMap(false);
    TestNotNull(TEXT("blank editor world created"), World);
    if (!World) return false;

    FActorSpawnParameters SpawnParams;
    SpawnParams.OverrideLevel = World->PersistentLevel;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    APinkCabVerticalAcceptanceCourse* Course = World->SpawnActor<APinkCabVerticalAcceptanceCourse>(
        APinkCabVerticalAcceptanceCourse::StaticClass(), FTransform::Identity, SpawnParams);
    TestNotNull(TEXT("vertical acceptance course spawned"), Course);
    if (!Course) return false;

    const FTransform PawnTransform(
        FRotator::ZeroRotator,
        FVector(-5000.0f, -2500.0f, 180.0f));
    APinkCabChaosTatraPawn* Pawn = World->SpawnActor<APinkCabChaosTatraPawn>(
        APinkCabChaosTatraPawn::StaticClass(), PawnTransform, SpawnParams);
    TestNotNull(TEXT("Chaos Tatra placeholder spawned"), Pawn);
    if (!Pawn) return false;

#if WITH_EDITOR
    Course->SetActorLabel(TEXT("PC_Vertical_Acceptance_NON_AUTHORITATIVE_GEOMETRY"));
    Pawn->SetActorLabel(TEXT("PC_Vertical_Acceptance_Tatra"));
#endif
    Course->SetFlags(RF_Transactional);
    Pawn->SetFlags(RF_Transactional);
    Course->MarkPackageDirty();
    Pawn->MarkPackageDirty();

    const bool bSaved = UEditorLoadingAndSavingUtils::SaveMap(World, MapPackage);
    TestTrue(TEXT("vertical acceptance map saved"), bSaved);
    return bSaved;
}

#endif
