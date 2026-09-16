#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "EngineUtils.h"
#include "Components/PrimitiveComponent.h"
#include "Cockpit/PinkCabCockpitAssemblyComponent.h"
#include "Cockpit/PinkCabCockpitPresentationState.h"
#include "Cockpit/PinkCabCockpitVisualBinding.h"
#include "Cockpit/PinkCabCockpitVisualDriverComponent.h"
#include "Vehicle/PinkCabChaosTatraPawn.h"
#include "Vehicle/PinkCabVehicleVisualProfile.h"
#include "Vehicle/PinkCabVehicleVisualShellComponent.h"

class FPinkCabVehiclePresentationBindingCommand final : public IAutomationLatentCommand
{
public:
    explicit FPinkCabVehiclePresentationBindingCommand(FAutomationTestBase* InTest) : Test(InTest) {}
    virtual bool Update() override;
private:
    FAutomationTestBase* Test = nullptr;
};

bool FPinkCabVehiclePresentationBindingCommand::Update()
{
    UWorld* World = AutomationCommon::GetAnyGameWorld();
    if (!World) return false;
    APinkCabChaosTatraPawn* Pawn = nullptr;
    for (TActorIterator<APinkCabChaosTatraPawn> It(World); It; ++It) { Pawn = *It; break; }
    Test->TestNotNull(TEXT("playable map owns Tatra pawn"), Pawn);
    if (!Pawn) return true;
    UPinkCabCockpitAssemblyComponent* Assembly = Pawn->GetCockpitAssembly();
    UPinkCabCockpitVisualDriverComponent* VisualDriver = Pawn->GetCockpitVisualDriver();
    UPinkCabVehicleVisualShellComponent* Shell = Pawn->GetVehicleVisualShell();
    Test->TestNotNull(TEXT("cockpit assembly exists"), Assembly);
    Test->TestNotNull(TEXT("cockpit visual driver exists"), VisualDriver);
    Test->TestNotNull(TEXT("vehicle visual shell exists"), Shell);
    if (!Assembly || !VisualDriver || !Shell) return true;

    USceneComponent* Steering = Assembly->GetSlotComponent(EPinkCabCockpitSlot::SteeringWheel);
    const FPinkCabCockpitSlotDefinition* Definition = Assembly->GetSlotDefinition(EPinkCabCockpitSlot::SteeringWheel);
    Test->TestNotNull(TEXT("steering anchor exists"), Steering);
    Test->TestNotNull(TEXT("steering definition exists"), Definition);
    if (!Steering || !Definition) return true;

    const FTransform OriginalTransform = Steering->GetRelativeTransform();
    const FName StableIdBefore = Definition->StableId;
    const bool bGripBefore = Definition->bSupportsGrip;
    const bool bMomentaryBefore = Definition->bSupportsMomentary;
    const bool bWheelBefore = Definition->bSupportsWheel;

    FPinkCabCockpitPresentationState NeutralState;
    VisualDriver->Apply(*Assembly, NeutralState);
    FPinkCabVehicleVisualProfile Donor = FPinkCabVehicleVisualProfile::Fallback();
    Donor.ProfileId = TEXT("PinkCab.Visual.CD855BindingProof");
    Donor.ExteriorStaticMesh = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Engine/BasicShapes/Cube.Cube")));
    Donor.bUseExteriorAsCabinWhenCabinMissing = true;

    FPinkCabCockpitVisualBinding SteeringBinding;
    SteeringBinding.Slot = EPinkCabCockpitSlot::SteeringWheel;
    SteeringBinding.LocalTransform = FTransform(
        FRotator(0.0f, 90.0f, 0.0f), FVector(25.0f, -44.0f, 116.0f), FVector(0.34f, 0.34f, 0.06f));
    SteeringBinding.bShowAnchorMesh = false;
    Donor.CockpitBindings.Add(SteeringBinding);

    Test->TestTrue(TEXT("donor profile applies"), Pawn->ApplyVehicleVisualProfile(Donor));
    Test->TestFalse(TEXT("physics chassis hides without hiding children"), Pawn->GetMesh()->IsVisible());
    Test->TestTrue(TEXT("cockpit remains visible when physics chassis hides"), Assembly->IsVisible());
    Test->TestNotNull(TEXT("donor exterior presentation exists"), Shell->GetExteriorPresentation());
    Test->TestNotNull(TEXT("single donor mesh is reused for owner cabin"), Shell->GetCabinPresentation());
    Test->TestTrue(TEXT("steering anchor moves to donor control location"),
        Steering->GetRelativeTransform().Equals(SteeringBinding.LocalTransform, 0.01f));

    const UPrimitiveComponent* SteeringPrimitive = Cast<UPrimitiveComponent>(Steering);
    Test->TestTrue(TEXT("baked donor control can hide primitive mesh while keeping anchor"),
        SteeringPrimitive && SteeringPrimitive->bHiddenInGame);
    Definition = Assembly->GetSlotDefinition(EPinkCabCockpitSlot::SteeringWheel);
    Test->TestEqual(TEXT("visual binding preserves stable id"), Definition->StableId, StableIdBefore);
    Test->TestEqual(TEXT("visual binding preserves grip semantics"), Definition->bSupportsGrip, bGripBefore);
    Test->TestEqual(TEXT("visual binding preserves momentary semantics"), Definition->bSupportsMomentary, bMomentaryBefore);
    Test->TestEqual(TEXT("visual binding preserves wheel semantics"), Definition->bSupportsWheel, bWheelBefore);

    VisualDriver->Apply(*Assembly, NeutralState);
    Test->TestTrue(TEXT("visual driver recaches donor steering base transform"),
        Steering->GetRelativeTransform().Equals(SteeringBinding.LocalTransform, 0.01f));

    Test->TestTrue(TEXT("fallback profile reapplies"),
        Pawn->ApplyVehicleVisualProfile(FPinkCabVehicleVisualProfile::Fallback()));
    Test->TestTrue(TEXT("physics chassis returns when donor exterior is removed"), Pawn->GetMesh()->IsVisible());
    Test->TestTrue(TEXT("fallback restores original steering transform"),
        Steering->GetRelativeTransform().Equals(OriginalTransform, 0.01f));
    Test->TestTrue(TEXT("fallback restores primitive steering visibility"),
        SteeringPrimitive && !SteeringPrimitive->bHiddenInGame);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabVehiclePresentationBindingRuntimeTest,
    "PinkCab.Vehicle.Visual.PresentationBindingRuntime",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabVehiclePresentationBindingRuntimeTest::RunTest(const FString& Parameters)
{
    const bool bOpened = AutomationOpenMap(TEXT("/Game/Dev/Maps/L_PinkCab_ChaosWeave"), true);
    TestTrue(TEXT("vehicle presentation runtime map opens"), bOpened);
    if (!bOpened) return false;
    ADD_LATENT_AUTOMATION_COMMAND(FPinkCabVehiclePresentationBindingCommand(this));
    return true;
}

#endif
