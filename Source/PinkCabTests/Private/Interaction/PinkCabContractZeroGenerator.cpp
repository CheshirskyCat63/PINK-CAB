#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Interaction/PinkCabContractCabinPrimitive.h"
#include "Editor.h"
#include "FileHelpers.h"
#include "EngineUtils.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabGenerateContractZeroCabinPrimitive,
    "PinkCab.Editor.GenerateContractZeroCabinPrimitive",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabGenerateContractZeroCabinPrimitive::RunTest(const FString& Parameters)
{
    if (!FEditorFileUtils::LoadMap(TEXT("/Game/Dev/Maps/L_PinkCab_ContractZero"), false, true))
    {
        AddError(TEXT("ContractZero map failed to load"));
        return false;
    }

    UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
    if (!World)
    {
        AddError(TEXT("Editor world unavailable"));
        return false;
    }

    for (TActorIterator<APinkCabContractCabinPrimitive> It(World); It; ++It)
    {
        TestTrue(TEXT("existing cabin primitive keeps map valid"), true);
        return true;
    }

    FActorSpawnParameters Params;
    Params.OverrideLevel = World->PersistentLevel;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    APinkCabContractCabinPrimitive* Primitive = World->SpawnActor<APinkCabContractCabinPrimitive>(
        APinkCabContractCabinPrimitive::StaticClass(),
        FTransform(FRotator::ZeroRotator, FVector(0.0f, 0.0f, 120.0f)),
        Params);
    if (!Primitive)
    {
        AddError(TEXT("Failed to spawn contract cabin primitive"));
        return false;
    }

#if WITH_EDITOR
    Primitive->SetActorLabel(TEXT("PC_ContractCabin_HeadlampToggle"));
#endif
    Primitive->SetFlags(RF_Transactional);
    Primitive->MarkPackageDirty();

    if (!FEditorFileUtils::SaveLevel(World->PersistentLevel))
    {
        AddError(TEXT("Failed to save ContractZero after cabin primitive placement"));
        return false;
    }

    return true;
}

#endif
