#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "EngineUtils.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "ChaosVehicleWheel.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Cockpit/PinkCabCockpitAssemblyComponent.h"
#include "Cockpit/PinkCabCockpitPresentationState.h"
#include "Cockpit/PinkCabCockpitVisualBinding.h"
#include "Cockpit/PinkCabCockpitVisualDriverComponent.h"
#include "Runtime/PinkCabChaosTatraPawn.h"
#include "Runtime/PinkCabVehicleVisualProfile.h"
#include "Runtime/PinkCabVehicleVisualShellComponent.h"

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
    bool bFoundPlayableSkyRig = false;
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        if (It->ActorHasTag(FName(TEXT("PinkCab.PlayableLighting"))))
        {
            bFoundPlayableSkyRig = true;
            break;
        }
    }
    Test->TestTrue(TEXT("playable map owns Unreal sky/light rig"), bFoundPlayableSkyRig);
    if (!Pawn) return true;
    UPinkCabCockpitAssemblyComponent* Assembly = Pawn->GetCockpitAssembly();
    UPinkCabCockpitVisualDriverComponent* VisualDriver = Pawn->GetCockpitVisualDriver();
    UPinkCabVehicleVisualShellComponent* Shell = Pawn->GetVehicleVisualShell();
    Test->TestNotNull(TEXT("cockpit assembly exists"), Assembly);
    Test->TestNotNull(TEXT("cockpit visual driver exists"), VisualDriver);
    Test->TestNotNull(TEXT("vehicle visual shell exists"), Shell);
    if (!Assembly || !VisualDriver || !Shell) return true;
    Test->TestEqual(TEXT("playable map starts with Tatra 613 desktop-clean profile"),
        Pawn->GetVehicleVisualProfileId(), FName(TEXT("PinkCab.Visual.Tatra613.ScenePreserved")));
    Test->TestNotNull(TEXT("playable map starts with visible donor exterior"), Shell->GetExteriorPresentation());
    Test->TestNotNull(TEXT("playable map starts with owner-visible source scene"), Shell->GetCabinPresentation());
    Test->TestEqual(TEXT("playable map renders all 133 source meshes plus four donor wheels"),
        Shell->GetPresentationPartCount(), 137);

    const FPinkCabVehicleVisualProfile& ActiveProfile = Shell->GetProfile();
    Test->TestFalse(TEXT("active Tatra profile identifies its source steering part"),
        ActiveProfile.SteeringPresentationPartId.IsNone());
    UStaticMeshComponent* SourceSteering =
        Shell->GetPresentationPartComponent(ActiveProfile.SteeringPresentationPartId);
    Test->TestNotNull(TEXT("source t613 steering scene part is present"), SourceSteering);
    if (SourceSteering && SourceSteering->GetStaticMesh())
    {
        Test->TestEqual(TEXT("visible steering is the preserved source mesh, not a replacement"),
            SourceSteering->GetStaticMesh()->GetName(), FString(TEXT("t613_Black_material_021")));
        Test->TestFalse(TEXT("source steering stays visible"), SourceSteering->bHiddenInGame);
    }

    USceneComponent* Steering = Assembly->GetSlotComponent(EPinkCabCockpitSlot::SteeringWheel);
    const FPinkCabCockpitSlotDefinition* Definition = Assembly->GetSlotDefinition(EPinkCabCockpitSlot::SteeringWheel);
    Test->TestNotNull(TEXT("generated steering anchor still exists for fallback semantics"), Steering);
    Test->TestNotNull(TEXT("steering definition exists"), Definition);
    if (!Steering || !Definition || !SourceSteering) return true;

    const UPrimitiveComponent* SteeringPrimitive = Cast<UPrimitiveComponent>(Steering);
    Test->TestTrue(TEXT("generated/replacement steering primitive is hidden for Tatra"),
        SteeringPrimitive && SteeringPrimitive->bHiddenInGame);

    USceneComponent* SteeringPivot = VisualDriver->GetSteeringVisualComponent();
    Test->TestNotNull(TEXT("source steering is driven through an invisible authored pivot"), SteeringPivot);
    if (!SteeringPivot) return true;
    Test->TestTrue(TEXT("the actual source steering mesh is parented to the live pivot"),
        SourceSteering->GetAttachParent() == SteeringPivot);
    Test->TestTrue(TEXT("steering-column pivot lies inside the actual steering-wheel bounds"),
        SourceSteering->Bounds.GetBox().IsInsideOrOn(SteeringPivot->GetComponentLocation()));

    const FTransform OriginalPivot = SteeringPivot->GetRelativeTransform();
    const FTransform OriginalSourceWorld = SourceSteering->GetComponentTransform();
    FPinkCabCockpitPresentationState TurnState;
    TurnState.Steering = 0.25f;
    VisualDriver->Apply(*Assembly, TurnState);
    Test->TestTrue(TEXT("source steering pivot does not translate while turning"),
        SteeringPivot->GetRelativeLocation().Equals(OriginalPivot.GetLocation(), 0.01f));
    Test->TestFalse(TEXT("source steering pivot rotates with steering command"),
        SteeringPivot->GetRelativeRotation().Equals(OriginalPivot.Rotator(), 0.01f));
    Test->TestFalse(TEXT("actual source steering mesh follows the pivot rotation"),
        SourceSteering->GetComponentRotation().Equals(OriginalSourceWorld.Rotator(), 0.01f));
    VisualDriver->Apply(*Assembly, FPinkCabCockpitPresentationState{});

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
    Test->TestTrue(TEXT("fallback restores primitive steering visibility"),
        SteeringPrimitive && !SteeringPrimitive->bHiddenInGame);
    Test->TestTrue(TEXT("donor profile reapplies after fallback"),
        Pawn->ApplyVehicleVisualProfile(FPinkCabVehicleVisualProfile::Tatra613Donor()));
    Test->TestEqual(TEXT("desktop-clean profile id restored after fallback proof"),
        Pawn->GetVehicleVisualProfileId(), FName(TEXT("PinkCab.Visual.Tatra613.ScenePreserved")));
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


class FPinkCabWheelPresentationChaosSyncCommand final : public IAutomationLatentCommand
{
public:
    explicit FPinkCabWheelPresentationChaosSyncCommand(FAutomationTestBase* InTest)
        : Test(InTest) {}

    virtual bool Update() override
    {
        UWorld* World = AutomationCommon::GetAnyGameWorld();
        if (!World) return false;

        APinkCabChaosTatraPawn* Pawn = nullptr;
        for (TActorIterator<APinkCabChaosTatraPawn> It(World); It; ++It)
        {
            Pawn = *It;
            break;
        }
        if (!Pawn) return false;

        UPinkCabVehicleVisualShellComponent* Shell = Pawn->GetVehicleVisualShell();
        UChaosWheeledVehicleMovementComponent* Movement = Pawn->GetChaosMovement();
        if (!Shell || !Movement || Movement->Wheels.Num() != 4) return false;

        static const FName WheelIds[] = {
            TEXT("WheelFL"), TEXT("WheelFR"), TEXT("WheelRL"), TEXT("WheelRR")};

        if (Phase == 0)
        {
            Pawn->SetSystemMenuOpen(false);
            const FPinkCabVehicleVisualProfile& Profile = Shell->GetProfile();
            for (int32 Index = 0; Index < 4; ++Index)
            {
                UStaticMeshComponent* VisualWheel =
                    Shell->GetPresentationPartComponent(WheelIds[Index]);
                Test->TestNotNull(
                    *FString::Printf(TEXT("%s visual wheel exists"), *WheelIds[Index].ToString()),
                    VisualWheel);
                if (!VisualWheel || !VisualWheel->GetStaticMesh()) return true;
                Test->TestTrue(
                    *FString::Printf(TEXT("%s visual wheel is rendered"), *WheelIds[Index].ToString()),
                    VisualWheel->IsVisible() && !VisualWheel->bHiddenInGame);

                const FVector Extent =
                    VisualWheel->GetStaticMesh()->GetBounds().BoxExtent
                    * VisualWheel->GetRelativeScale3D().GetAbs();
                TArray<float> HalfDimensions = {
                    FMath::Abs(Extent.X), FMath::Abs(Extent.Y), FMath::Abs(Extent.Z)};
                HalfDimensions.Sort();
                Test->AddInfo(FString::Printf(
                    TEXT("WHEEL_VISUAL_BOUNDS id=%s half=(%.2f,%.2f,%.2f)"),
                    *WheelIds[Index].ToString(),
                    HalfDimensions[0], HalfDimensions[1], HalfDimensions[2]));
                Test->TestTrue(
                    *FString::Printf(TEXT("%s visual tyre width matches 205mm source"), *WheelIds[Index].ToString()),
                    FMath::IsNearlyEqual(HalfDimensions[0], 10.25f, 2.5f));
                Test->TestTrue(
                    *FString::Printf(TEXT("%s visual tyre radius matches 205/70R14 source"), *WheelIds[Index].ToString()),
                    FMath::IsNearlyEqual(HalfDimensions[2], 32.13f, 3.0f));
            }

            FrontBaseRotation =
                Shell->GetPresentationPartComponent(TEXT("WheelFL"))->GetRelativeRotation();
            RearBaseRotation =
                Shell->GetPresentationPartComponent(TEXT("WheelRL"))->GetRelativeRotation();

            FPinkCabVehicleInputFrame Frame;
            Pawn->ApplyVehicleInputFrame(Frame, 180.0f, 0.25f);
            StartSeconds = FPlatformTime::Seconds();
            Phase = 1;
            return false;
        }

        if ((FPlatformTime::Seconds() - StartSeconds) < 0.35)
        {
            return false;
        }

        UStaticMeshComponent* FrontVisual =
            Shell->GetPresentationPartComponent(TEXT("WheelFL"));
        UStaticMeshComponent* RearVisual =
            Shell->GetPresentationPartComponent(TEXT("WheelRL"));
        const UChaosVehicleWheel* FrontChaos = Movement->Wheels[0];
        const UChaosVehicleWheel* RearChaos = Movement->Wheels[2];
        Test->TestNotNull(TEXT("front Chaos wheel instance exists"), FrontChaos);
        Test->TestNotNull(TEXT("rear Chaos wheel instance exists"), RearChaos);
        if (!FrontVisual || !RearVisual || !FrontChaos || !RearChaos) return true;

        const float FrontSteer = FrontChaos->GetSteerAngle();
        Test->AddInfo(FString::Printf(
            TEXT("WHEEL_CHAOS_POSE frontSteer=%.3f frontSpin=%.3f frontSusp=%.3f rearSpin=%.3f rearSusp=%.3f"),
            FrontSteer,
            FrontChaos->GetRotationAngle(),
            FrontChaos->GetSuspensionOffset(),
            RearChaos->GetRotationAngle(),
            RearChaos->GetSuspensionOffset()));
        Test->TestTrue(TEXT("Chaos front wheel receives a real steering pose"),
            FMath::Abs(FrontSteer) > 0.25f);
        Test->TestFalse(TEXT("visible front wheel follows Chaos steering"),
            FrontVisual->GetRelativeRotation().Equals(FrontBaseRotation, 0.05f));

        const FPinkCabVehicleVisualProfile& Profile = Shell->GetProfile();
        const FPinkCabVehiclePresentationPart* FrontPart =
            Profile.PresentationParts.FindByPredicate([](const FPinkCabVehiclePresentationPart& Part)
            {
                return Part.PartId == FName(TEXT("WheelFL"));
            });
        if (FrontPart && FMath::Abs(FrontChaos->GetSuspensionOffset()) > 0.10f)
        {
            Test->TestFalse(TEXT("visible front wheel follows Chaos suspension offset"),
                FrontVisual->GetRelativeLocation().Equals(
                    FrontPart->LocalTransform.GetLocation(), 0.05f));
        }

        Test->TestTrue(TEXT("rear wheel is not falsely steered by front steering input"),
            FMath::Abs(RearChaos->GetSteerAngle()) < 0.10f);
        return true;
    }

private:
    FAutomationTestBase* Test = nullptr;
    int32 Phase = 0;
    double StartSeconds = 0.0;
    FRotator FrontBaseRotation = FRotator::ZeroRotator;
    FRotator RearBaseRotation = FRotator::ZeroRotator;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabWheelPresentationChaosSyncTest,
    "PinkCab.Vehicle.Visual.WheelPresentationChaosSync",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabWheelPresentationChaosSyncTest::RunTest(const FString& Parameters)
{
    const bool bOpened =
        AutomationOpenMap(TEXT("/Game/Dev/Maps/L_PinkCab_ChaosWeave"), true);
    TestTrue(TEXT("wheel presentation sync map opens"), bOpened);
    if (!bOpened) return false;
    ADD_LATENT_AUTOMATION_COMMAND(
        FPinkCabWheelPresentationChaosSyncCommand(this));
    return true;
}

#endif
