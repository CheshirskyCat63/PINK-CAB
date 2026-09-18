#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Cockpit/PinkCabCockpitSlot.h"
#include "Cockpit/PinkCabCockpitPresentationState.h"
#include "Interaction/PinkCabInteractionModel.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabCockpitStableSlotContractTest,
    "PinkCab.Cockpit.Contract.StableSlots",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabCockpitStableSlotContractTest::RunTest(const FString& Parameters)
{
    TArray<FPinkCabCockpitSlotDefinition> Definitions;
    for (uint8 Raw = 0; Raw <= static_cast<uint8>(EPinkCabCockpitSlot::TachometerNeedle); ++Raw)
    {
        const EPinkCabCockpitSlot Slot = static_cast<EPinkCabCockpitSlot>(Raw);
        Definitions.Emplace(Slot, PinkCabCockpitSlotId(Slot));
    }

    TestEqual(TEXT("all required cockpit slots are defined"), Definitions.Num(),
        static_cast<int32>(EPinkCabCockpitSlot::TachometerNeedle) + 1);
    TestTrue(TEXT("unique stable slot definitions validate"), FPinkCabCockpitSlotDefinition::ValidateUnique(Definitions));
    const FPinkCabCockpitSlotDefinition Duplicate = Definitions[0];
    Definitions.Add(Duplicate);
    TestFalse(TEXT("duplicate stable slot IDs are rejected"), FPinkCabCockpitSlotDefinition::ValidateUnique(Definitions));

    FPinkCabCockpitPresentationState Presentation;
    TestEqual(TEXT("presentation starts with neutral steering"), Presentation.Steering, 0.0f);
    TestFalse(TEXT("presentation does not invent meter availability"), Presentation.bMeterAvailable);
    return true;
}

#endif


#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabCockpitSlotMetadataContractTest,
    "PinkCab.Cockpit.Contract.SlotMetadata",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabCockpitSlotMetadataContractTest::RunTest(const FString& Parameters)
{
    FPinkCabCockpitSlotDefinition Definition(EPinkCabCockpitSlot::Gearbox, TEXT("Gearbox"));
    Definition.LocalTransform = FTransform(FRotator(0.0f, 15.0f, 0.0f), FVector(10.0f, 20.0f, 30.0f));
    Definition.MotionAxis = FVector::UpVector;
    Definition.MotionPivot = FVector(1.0f, 2.0f, 3.0f);
    Definition.MotionRange = FVector2D(-1.0f, 5.0f);
    Definition.bSupportsGrip = true;
    Definition.bSupportsWheel = true;
    const FPinkCabInteractionControlSpec Spec = Definition.ToInteractionSpec();
    TestEqual(TEXT("slot metadata preserves stable interaction id"), Spec.Id, FName(TEXT("Gearbox")));
    TestTrue(TEXT("slot metadata exposes grip capability"), Spec.bSupportsGrip);
    TestTrue(TEXT("slot metadata exposes wheel capability"), Spec.bSupportsWheel);
    return true;
}
#endif

#if WITH_DEV_AUTOMATION_TESTS
#include "Cockpit/PinkCabCockpitAssemblyComponent.h"
#include "Components/SceneComponent.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabCockpitBoundedGazeRegistryTest,
    "PinkCab.Cockpit.Contract.BoundedGazeRegistry",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabCockpitBoundedGazeRegistryTest::RunTest(const FString& Parameters)
{
    UPinkCabCockpitAssemblyComponent* Assembly = NewObject<UPinkCabCockpitAssemblyComponent>();
    USceneComponent* Horn = NewObject<USceneComponent>();
    USceneComponent* Gearbox = NewObject<USceneComponent>();
    Horn->SetRelativeLocation(FVector(100.0f, 0.0f, 0.0f));
    Gearbox->SetRelativeLocation(FVector(100.0f, 100.0f, 0.0f));
    Assembly->RegisterExternalSlot(EPinkCabCockpitSlot::Horn, Horn);
    Assembly->RegisterExternalSlot(EPinkCabCockpitSlot::Gearbox, Gearbox);
    const FName Target = Assembly->ResolveGazeTarget(
        FVector::ZeroVector, FVector::ForwardVector, 200.0f, 22);
    TestEqual(TEXT("gaze resolves only from bounded registered cockpit slots"), Target, FName(TEXT("Horn")));
    return true;
}
#endif
