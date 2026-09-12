#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "World/PinkCabL1GreyboxCorridor.h"
#include "FileHelpers.h"
#include "Editor.h"
#include "Engine/World.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabGenerateL1GreyboxZero,
    "PinkCab.Editor.GenerateL1GreyboxZero",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabGenerateL1GreyboxZero::RunTest(const FString& Parameters)
{
    UWorld* World = UEditorLoadingAndSavingUtils::NewBlankMap(false);
    TestNotNull(TEXT("blank editor world created"), World);
    if (!World) return false;

    FActorSpawnParameters SpawnParams;
    SpawnParams.OverrideLevel = World->PersistentLevel;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    APinkCabL1GreyboxCorridor* Corridor = World->SpawnActor<APinkCabL1GreyboxCorridor>(
        APinkCabL1GreyboxCorridor::StaticClass(),
        FTransform::Identity,
        SpawnParams);
    TestNotNull(TEXT("L1 contract corridor spawned"), Corridor);
    if (!Corridor) return false;

#if WITH_EDITOR
    Corridor->SetActorLabel(TEXT("PC_L1_GreyboxZero_NON_AUTHORITATIVE_GEOMETRY"));
#endif
    Corridor->SetFlags(RF_Transactional);
    Corridor->MarkPackageDirty();

    const bool bSaved = UEditorLoadingAndSavingUtils::SaveMap(
        World,
        TEXT("/Game/Dev/Maps/L_PinkCab_L1_GreyboxZero"));
    TestTrue(TEXT("L1 Greybox Zero map saved"), bSaved);
    return bSaved;
}

#endif
