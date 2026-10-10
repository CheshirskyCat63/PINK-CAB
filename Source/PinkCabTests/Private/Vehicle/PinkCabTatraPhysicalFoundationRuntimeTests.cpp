#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "EngineUtils.h"
#include "ChaosVehicleWheel.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/SkeletalBodySetup.h"
#include "PhysicsEngine/BodySetup.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Runtime/PinkCabChaosTatraPawn.h"

class FPinkCabTatraPhysicalFoundationAuditCommand final : public IAutomationLatentCommand
{
public:
    explicit FPinkCabTatraPhysicalFoundationAuditCommand(FAutomationTestBase* InTest, bool bInNativeSleepOnly = false)
        : Test(InTest), Started(FPlatformTime::Seconds()), bNativeSleepOnly(bInNativeSleepOnly) {}

    virtual bool Update() override
    {
        if (FPlatformTime::Seconds() - Started > 15.0)
        {
            Test->AddError(TEXT("physical-foundation audit timed out"));
            return true;
        }

        UWorld* World = AutomationCommon::GetAnyGameWorld();
        if (!World) return false;

        APinkCabChaosTatraPawn* Pawn = nullptr;
        for (TActorIterator<APinkCabChaosTatraPawn> It(World); It; ++It)
        {
            Pawn = *It;
            break;
        }
        if (!Pawn || !Pawn->GetMesh() || !Pawn->GetChaosMovement()) return false;

        Pawn->SetSystemMenuOpen(false);
        if (SettlingStarted < 0.0)
        {
            SavedSleepThreshold = Pawn->GetChaosMovement()->SleepThreshold;
            if (bNativeSleepOnly) Pawn->GetChaosMovement()->SleepThreshold = 0.0f;
            Test->AddInfo(FString::Printf(TEXT("T6_SETTLEMENT sleep_threshold_before=%.3f diagnostic_native_sleep_only=%d"),
                SavedSleepThreshold, bNativeSleepOnly));
            SettlingStarted = World->GetTimeSeconds();
            return false;
        }
        // Static axle balance is a settled interval, not one launch-transient frame.
        const double SettledSeconds = World->GetTimeSeconds() - SettlingStarted;
        if (SettledSeconds < 3.0) return false;
        UChaosWheeledVehicleMovementComponent* SampleMovement = Pawn->GetChaosMovement();
        for (int32 Index = 0; Index < SampleMovement->Wheels.Num(); ++Index)
        {
            const FWheelStatus& Sample = SampleMovement->GetWheelState(Index);
            if (Index < 2) FrontSamples += Sample.SpringForce;
            else RearSamples += Sample.SpringForce;
            bAllSamplesInContact &= Sample.bInContact;
        }
        ++SampleCount;
        if (SettledSeconds < 4.0) return false;
        Test->TestTrue(TEXT("static load sample covers an interval with four contacts"),
            SampleCount >= 10 && bAllSamplesInContact && SampleMovement->Wheels.Num() == 4);

        USkeletalMeshComponent* Mesh = Pawn->GetMesh();
        UChaosWheeledVehicleMovementComponent* Movement = Pawn->GetChaosMovement();
        UPhysicsAsset* PhysicsAsset = Mesh->GetPhysicsAsset();
        Test->TestNotNull(TEXT("physics chassis asset is assigned"), PhysicsAsset);
        if (!PhysicsAsset) return true;

        FBodyInstance* RootBody = Mesh->GetBodyInstance();
        Test->TestNotNull(TEXT("root physics body exists"), RootBody);
        if (!RootBody) return true;
        Test->AddInfo(FString::Printf(TEXT("T6_SETTLEMENT final_awake=%d velocity=%s angular=%s"),
            RootBody->IsInstanceAwake(), *Pawn->GetVelocity().ToString(),
            *Mesh->GetPhysicsAngularVelocityInRadians().ToString()));

        Test->AddInfo(FString::Printf(
            TEXT("T5_CHASSIS mesh=%s physics_asset=%s bodies=%d constraints=%d movement_mass=%.3f aggregate_mesh_mass=%.3f root_mass=%.3f"),
            Mesh->GetSkeletalMeshAsset() ? *Mesh->GetSkeletalMeshAsset()->GetPathName() : TEXT("<none>"),
            *PhysicsAsset->GetPathName(),
            PhysicsAsset->SkeletalBodySetups.Num(),
            PhysicsAsset->ConstraintSetup.Num(),
            Movement->Mass,
            Mesh->GetMass(),
            RootBody->GetBodyMass()));

        const FVector ComWorld = RootBody->GetCOMPosition();
        const FVector ComComponent =
            Mesh->GetComponentTransform().InverseTransformPosition(ComWorld);
        const FVector Inertia = RootBody->GetBodyInertiaTensor();
        Test->AddInfo(FString::Printf(
            TEXT("T5_BODY root_bone=%s com_component=(%s) com_override_enabled=%d com_override=(%s) inertia=(%s)"),
            RootBody->BodySetup.IsValid()
                ? *RootBody->BodySetup->BoneName.ToString()
                : TEXT("<none>"),
            *ComComponent.ToString(),
            Movement->bEnableCenterOfMassOverride,
            *Movement->CenterOfMassOverride.ToString(),
            *Inertia.ToString()));

        for (int32 Index = 0; Index < PhysicsAsset->SkeletalBodySetups.Num(); ++Index)
        {
            const USkeletalBodySetup* Setup = PhysicsAsset->SkeletalBodySetups[Index];
            if (!Setup) continue;
            Test->AddInfo(FString::Printf(
                TEXT("T5_SHAPE body=%d bone=%s physics_type=%d spheres=%d boxes=%d capsules=%d convex=%d mass_scale=%.3f com_nudge=(%s) inertia_scale=(%s)"),
                Index,
                *Setup->BoneName.ToString(),
                static_cast<int32>(Setup->PhysicsType),
                Setup->AggGeom.SphereElems.Num(),
                Setup->AggGeom.BoxElems.Num(),
                Setup->AggGeom.SphylElems.Num(),
                Setup->AggGeom.ConvexElems.Num(),
                Setup->DefaultInstance.MassScale,
                *Setup->DefaultInstance.COMNudge.ToString(),
                *Setup->DefaultInstance.InertiaTensorScale.ToString()));
        }

        float FrontSpringRaw = FrontSamples / FMath::Max(SampleCount, 1);
        float RearSpringRaw = RearSamples / FMath::Max(SampleCount, 1);
        for (int32 Index = 0; Index < Movement->Wheels.Num(); ++Index)
        {
            const FWheelStatus& State = Movement->GetWheelState(Index);
            const UChaosVehicleWheel* Wheel = Movement->Wheels[Index];
            const float ForceRaw = State.SpringForce;
            Test->AddInfo(FString::Printf(
                TEXT("T5_CORNER wheel=%d contact=%d spring_force=%.3f suspension_offset=%.3f radius=%.3f"),
                Index, State.bInContact, ForceRaw,
                Wheel ? Wheel->GetSuspensionOffset() : 0.0f,
                Wheel ? Wheel->WheelRadius : 0.0f));
            Test->TestTrue(
                *FString::Printf(TEXT("wheel %d has static road contact"), Index),
                State.bInContact);
        }

        const float TotalSpringRaw = FrontSpringRaw + RearSpringRaw;
        Test->AddInfo(FString::Printf(
            TEXT("T5_AXLE front_force=%.3f rear_force=%.3f rear_fraction=%.6f total_force=%.3f"),
            FrontSpringRaw, RearSpringRaw,
            TotalSpringRaw > KINDA_SMALL_NUMBER ? RearSpringRaw / TotalSpringRaw : 0.0f,
            TotalSpringRaw));

        // Acceptance RED gate: a finite number is not physical Tatra evidence.
        const FString BodyMeshPath = Mesh->GetSkeletalMeshAsset()
            ? Mesh->GetSkeletalMeshAsset()->GetPathName() : FString();
        Test->TestFalse(TEXT("physical chassis cannot be the stock SportsCar mesh"),
            BodyMeshPath.Contains(TEXT("/Vehicles/SportsCar/")));
        Test->TestFalse(TEXT("physical collision cannot use PA_SportsCar"),
            PhysicsAsset->GetPathName().Contains(TEXT("/Vehicles/SportsCar/")));
        Test->TestTrue(TEXT("runtime root mass agrees with authoritative total"),
            FMath::IsNearlyEqual(RootBody->GetBodyMass(), Movement->Mass, 0.5f));
        Test->TestTrue(TEXT("reference engine-behind-axle COM is rear of chassis origin"),
            ComComponent.X < -1.0f);
        Test->TestTrue(TEXT("four native wheel instances are active"), Movement->Wheels.Num() == 4);
        const float RearFraction =
            TotalSpringRaw > KINDA_SMALL_NUMBER ? RearSpringRaw / TotalSpringRaw : 0.0f;
        // Confluence 33 reference static balance = 45% front / 55% rear at 1657 kg.
        // This is a physical measurement gate; never make it pass by a tire shortcut.
        Test->TestTrue(TEXT("reference axle loads are rear-heavy (55% rear +/- 2%)"),
            RearFraction >= 0.53f && RearFraction <= 0.57f);
        Test->TestTrue(TEXT("physical root inertia remains strictly positive"),
            Inertia.X > 0.0f && Inertia.Y > 0.0f && Inertia.Z > 0.0f);

        Test->TestTrue(TEXT("movement mass is finite"), FMath::IsFinite(Movement->Mass));
        Test->TestTrue(TEXT("root mass is finite"), FMath::IsFinite(RootBody->GetBodyMass()));
        Test->TestTrue(TEXT("CoM is finite"), !ComComponent.ContainsNaN());
        Test->TestTrue(TEXT("inertia is finite"), !Inertia.ContainsNaN());
        if (bNativeSleepOnly) Movement->SleepThreshold = SavedSleepThreshold;
        return true;
    }

private:
    FAutomationTestBase* Test = nullptr;
    double Started = 0.0;
    double SettlingStarted = -1.0;
    double FrontSamples = 0.0;
    double RearSamples = 0.0;
    int32 SampleCount = 0;
    bool bAllSamplesInContact = true;
    bool bNativeSleepOnly = false;
    float SavedSleepThreshold = 0.0f;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabTatraPhysicalFoundationBaselineAuditTest,
    "PinkCab.Vehicle.PhysicalFoundation.BaselineAudit",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabTatraPhysicalFoundationBaselineAuditTest::RunTest(const FString& Parameters)
{
    const bool bOpened =
        AutomationOpenMap(TEXT("/Game/Dev/Maps/L_PinkCab_L1_EndlessStraight"), true);
    TestTrue(TEXT("physical-foundation audit map opens"), bOpened);
    if (!bOpened) return false;
    ADD_LATENT_AUTOMATION_COMMAND(
        FPinkCabTatraPhysicalFoundationAuditCommand(this));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPinkCabNativeSettlementDiagnostic,
    "PinkCab.Vehicle.PhysicalFoundation.NativeSleepSettlementDiagnostic",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabNativeSettlementDiagnostic::RunTest(const FString& Parameters)
{
    if (!AutomationOpenMap(TEXT("/Game/Dev/Maps/L_PinkCab_L1_EndlessStraight"), true)) return false;
    ADD_LATENT_AUTOMATION_COMMAND(FPinkCabTatraPhysicalFoundationAuditCommand(this, true));
    return true;
}
#endif
