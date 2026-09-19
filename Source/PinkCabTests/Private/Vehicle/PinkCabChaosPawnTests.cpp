#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/SkinnedAsset.h"
#include "Vehicle/PinkCabChaosTatraPawn.h"
#include "Vehicle/PinkCabChaosCockpitBridge.h"
#include "Vehicle/PinkCabCockpitState.h"
#include "Vehicle/PinkCabChaosWheelFront.h"
#include "Vehicle/PinkCabChaosWheelRear.h"
#include "Vehicle/PinkCabTatraProfile.h"
#include "Vehicle/PinkCabSteeringController.h"
#include "Vehicle/PinkCabChaosPhysicalProfile.h"
#include "Vehicle/PinkCabVehicleVisualProfile.h"
#include "PhysicsEngine/BodyInstance.h"
#include "PhysicsEngine/BodySetup.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabChaosPawnBaselineConfigTest,
    "PinkCab.Vehicle.ChaosBaseline.Pawn.Config",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabChaosPawnBaselineConfigTest::RunTest(const FString& Parameters)
{
    const APinkCabChaosTatraPawn* Pawn = GetDefault<APinkCabChaosTatraPawn>();
    const UChaosWheeledVehicleMovementComponent* Movement = Pawn->GetChaosMovement();
    TestNotNull(TEXT("pawn owns Chaos movement"), Movement);
    if (!Movement)
    {
        return false;
    }

    const FPinkCabTatraProfile Profile = FPinkCabTatraProfile::Canonical();
    TestEqual(TEXT("reference fixture mass"), Movement->Mass, Profile.GetReferenceCrewMassKg());
    TestEqual(TEXT("four wheel setups"), Movement->WheelSetups.Num(), 4);
    TestEqual(TEXT("rear wheel drive"), Movement->DifferentialSetup.DifferentialType, EVehicleDifferential::RearWheelDrive);
    TestFalse(TEXT("torque arcade control off"), Movement->TorqueControl.Enabled);
    TestFalse(TEXT("target rotation arcade control off"), Movement->TargetRotationControl.Enabled);
    TestFalse(TEXT("stabilize arcade control off"), Movement->StabilizeControl.Enabled);
    float TorqueMin = 0.0f;
    float TorqueMax = 0.0f;
    Movement->EngineSetup.TorqueCurve.GetRichCurveConst()->GetValueRange(TorqueMin, TorqueMax);
    TestTrue(TEXT("engine torque curve has positive output"), TorqueMax > 0.0f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabChaosWheelRolesTest,
    "PinkCab.Vehicle.ChaosBaseline.Pawn.WheelRoles",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabChaosWheelRolesTest::RunTest(const FString& Parameters)
{
    const UPinkCabChaosWheelFront* Front = GetDefault<UPinkCabChaosWheelFront>();
    const UPinkCabChaosWheelRear* Rear = GetDefault<UPinkCabChaosWheelRear>();

    TestTrue(TEXT("front wheels steer"), Front->bAffectedBySteering);
    TestFalse(TEXT("front wheels do not drive"), Front->bAffectedByEngine);
    TestFalse(TEXT("front wheels do not handbrake"), Front->bAffectedByHandbrake);

    TestFalse(TEXT("rear wheels do not steer"), Rear->bAffectedBySteering);
    TestTrue(TEXT("rear wheels drive"), Rear->bAffectedByEngine);
    TestTrue(TEXT("rear wheels handbrake"), Rear->bAffectedByHandbrake);
    return true;
}

namespace
{
FVector ResolveChaosWheelRestPosition(
    const APinkCabChaosTatraPawn& Pawn,
    const FChaosWheelSetup& Setup)
{
    const USkeletalMeshComponent* Mesh = Pawn.GetMesh();
    const USkinnedAsset* Asset = Mesh ? Mesh->GetSkinnedAsset() : nullptr;
    FVector Offset = Setup.WheelClass.GetDefaultObject()->Offset + Setup.AdditionalOffset;
    if (!Mesh || !Asset || Setup.BoneName.IsNone())
    {
        return Offset;
    }

    const FVector BonePosition =
        Asset->GetComposedRefPoseMatrix(Setup.BoneName).GetOrigin() * Mesh->GetRelativeScale3D();
    FMatrix RootBodyMatrix = FMatrix::Identity;
    if (const FBodyInstance* BodyInstance = Mesh->GetBodyInstance())
    {
        if (BodyInstance->BodySetup.IsValid())
        {
            RootBodyMatrix = Asset->GetComposedRefPoseMatrix(BodyInstance->BodySetup->BoneName);
        }
    }
    return Offset + RootBodyMatrix.InverseTransformPosition(BonePosition);
}

const FPinkCabVehiclePresentationPart* FindPresentationPart(
    const FPinkCabVehicleVisualProfile& Profile,
    const FName PartId)
{
    return Profile.PresentationParts.FindByPredicate([PartId](const FPinkCabVehiclePresentationPart& Part)
    {
        return Part.PartId == PartId;
    });
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabChaosTatraWheelGeometryBindingTest,
    "PinkCab.Vehicle.ChaosBaseline.Pawn.TatraWheelGeometryBinding",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabChaosTatraWheelGeometryBindingTest::RunTest(const FString& Parameters)
{
    const APinkCabChaosTatraPawn* Pawn = GetDefault<APinkCabChaosTatraPawn>();
    const UChaosWheeledVehicleMovementComponent* Movement = Pawn->GetChaosMovement();
    TestNotNull(TEXT("Tatra pawn owns Chaos movement"), Movement);
    if (!Movement || Movement->WheelSetups.Num() != 4)
    {
        return false;
    }

    const FPinkCabVehicleVisualProfile Tatra = FPinkCabVehicleVisualProfile::Tatra613Donor();
    const FName PartIds[4] = { TEXT("WheelFL"), TEXT("WheelFR"), TEXT("WheelRL"), TEXT("WheelRR") };
    for (int32 Index = 0; Index < 4; ++Index)
    {
        const FPinkCabVehiclePresentationPart* Part = FindPresentationPart(Tatra, PartIds[Index]);
        TestNotNull(*FString::Printf(TEXT("%s target wheel contract exists"), *PartIds[Index].ToString()), Part);
        if (!Part)
        {
            return false;
        }
        const FVector Expected = Part->LocalTransform.GetLocation();
        const FVector Actual = ResolveChaosWheelRestPosition(*Pawn, Movement->WheelSetups[Index]);
        TestTrue(
            *FString::Printf(TEXT("%s Chaos resting position follows Tatra geometry contract"), *PartIds[Index].ToString()),
            Actual.Equals(Expected, 0.01f));
    }
    return true;
}

#endif

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabChaosMouseSteeringContractTest,
    "PinkCab.Vehicle.ChaosBaseline.Input.MouseSteering",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabChaosMouseSteeringContractTest::RunTest(const FString& Parameters)
{
    FPinkCabSteeringControllerConfig Config;
    Config.MouseCountsForFullScale = 100.0f;

    FPinkCabSteeringController Steering(Config);
    const float First = Steering.Step(
        50.0f, false, 0.0f, EPinkCabVehicleMotionMode::Stationary, 1.0f);
    TestTrue(TEXT("mouse right stays semantic right-positive before Chaos adaptation"), First > 0.0f);

    const float CursorBeforeTiny = Steering.GetVirtualCursor();
    Steering.Step(-0.1f, false, 0.0f, EPinkCabVehicleMotionMode::Stationary, 1.0f);
    TestTrue(TEXT("no center dead-zone swallows tiny mouse delta"),
        Steering.GetVirtualCursor() < CursorBeforeTiny);

    const float BeforeGaze = Steering.GetSteering();
    const float GazeHeld = Steering.Step(
        50.0f, true, 120.0f, EPinkCabVehicleMotionMode::Moving, 1.0f);
    TestEqual(TEXT("Space gaze hold preserves steering command"), GazeHeld, BeforeGaze);

    FPinkCabSteeringController Slow(Config);
    FPinkCabSteeringController Fast(Config);
    const float SlowResult = Slow.Step(
        100.0f, false, 0.0f, EPinkCabVehicleMotionMode::Stationary, 0.1f);
    const float FastResult = Fast.Step(
        100.0f, false, 160.0f, EPinkCabVehicleMotionMode::Moving, 0.1f);
    TestTrue(TEXT("stationary steering is heavier than moving high-speed response"),
        FMath::Abs(SlowResult) < FMath::Abs(FastResult));
    TestTrue(TEXT("steering remains bounded"), FMath::Abs(FastResult) <= 1.0f);
    return true;
}

#endif

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabChaosTemplateChassisAssetTest,
    "PinkCab.Vehicle.ChaosBaseline.Assets.TemplateChassis",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabChaosTemplateChassisAssetTest::RunTest(const FString& Parameters)
{
    const APinkCabChaosTatraPawn* Pawn = GetDefault<APinkCabChaosTatraPawn>();
    const USkeletalMeshComponent* Mesh = Pawn->GetMesh();
    USkeletalMesh* SkeletalMesh = Mesh->GetSkeletalMeshAsset();
    TestNotNull(TEXT("temporary UE template skeletal mesh is assigned"), SkeletalMesh);
    TestNotNull(TEXT("temporary UE template physics asset is available"), Mesh->GetPhysicsAsset());
    if (!SkeletalMesh)
    {
        return false;
    }

    TestNotNull(TEXT("template skeleton reference resolves"), SkeletalMesh->GetSkeleton());
    const FReferenceSkeleton& RefSkeleton = SkeletalMesh->GetRefSkeleton();
    TestTrue(TEXT("front-left Chaos wheel bone exists"), RefSkeleton.FindBoneIndex(TEXT("Phys_Wheel_FL")) != INDEX_NONE);
    TestTrue(TEXT("rear-right Chaos wheel bone exists"), RefSkeleton.FindBoneIndex(TEXT("Phys_Wheel_BR")) != INDEX_NONE);
    return SkeletalMesh->GetSkeleton() != nullptr && Mesh->GetPhysicsAsset() != nullptr;
}

#endif

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabChaosCockpitBridgeTest,
    "PinkCab.Vehicle.ChaosBaseline.Pawn.CockpitBridge",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabChaosCockpitBridgeTest::RunTest(const FString& Parameters)
{
    UChaosWheeledVehicleMovementComponent* Movement = NewObject<UChaosWheeledVehicleMovementComponent>();
    FPinkCabChaosPhysicalProfile::ForVariant(EPinkCabCalibrationVariant::Nominal)
        .ApplyToMovement(*Movement);
    FPinkCabChaosVehicleDynamicsProvider Provider(Movement);
    FPinkCabVehicleControlState Controls;
    FPinkCabCockpitState Cockpit;

    Controls.SetSteering(0.40f);
    TestTrue(TEXT("semantic right-positive steering reaches Chaos adapter"),
        Provider.ApplyControls(Controls));
    TestTrue(TEXT("Chaos vehicle-space steering is inverted exactly once from player-facing right-positive"),
        FMath::IsNearlyEqual(Movement->GetSteeringInput(), -0.40f, 1.e-4f));

    Controls.SetHandbrake(0.37f);
    TestTrue(TEXT("default cockpit applies to Chaos"),
        FPinkCabChaosCockpitBridge::Apply(Cockpit, *Movement, Controls, Provider));
    TestFalse(TEXT("engine off disables mechanical simulation"), Movement->bMechanicalSimEnabled);
    TestFalse(TEXT("legacy bool handbrake path stays disabled"), Movement->GetHandbrakeInput());
    TestEqual(TEXT("analog handbrake command remains continuous through provider"),
        Provider.GetLastControls().Handbrake, 0.37f);

    Cockpit.StartEngine();
    Cockpit.ShiftBy(1);
    Controls.SetThrottle(0.6f);
    Controls.SetClutch(0.0f);
    Controls.SetDriveline(1, 1, 1.0f);
    TestTrue(TEXT("running cockpit reapplies to Chaos"),
        FPinkCabChaosCockpitBridge::Apply(Cockpit, *Movement, Controls, Provider));
    TestTrue(TEXT("running ignition enables mechanical simulation"), Movement->bMechanicalSimEnabled);
    TestFalse(TEXT("bool handbrake remains disabled after reapply"), Movement->GetHandbrakeInput());
    TestFalse(TEXT("cockpit gearbox disables automatic shifting"), Movement->GetUseAutoGears());
    TestFalse(TEXT("manual H-gate disables Chaos arcade reverse-as-brake override"),
        Movement->bReverseAsBrake);
    TestFalse(TEXT("manual H-gate disables throttle-as-brake companion behavior"),
        Movement->bThrottleAsBrake);
    TestEqual(TEXT("fully coupled engaged first reaches Chaos"), Movement->GetTargetGear(), 1);

    Controls.SetClutch(0.5f);
    Controls.SetDriveline(1, 1, 0.5f);
    FPinkCabChaosCockpitBridge::Apply(Cockpit, *Movement, Controls, Provider);
    TestEqual(TEXT("partial coupling keeps Chaos transmission neutral"), Movement->GetTargetGear(), 0);
    const float PoweredPartialTorque =
        FMath::Abs(Provider.GetLastControls().ExternalRearDriveTorquePerWheelNm);
    TestTrue(TEXT("partial coupling produces continuous external rear torque"),
        PoweredPartialTorque > 0.0f);

    // A real idling engine still has anti-stall/idle torque at the clutch bite point.
    // This is what lets a careful no-throttle launch creep while a fast clutch dump
    // can still hit the drivetrain stall gate.
    Controls.SetThrottle(0.0f);
    FPinkCabChaosCockpitBridge::Apply(Cockpit, *Movement, Controls, Provider);
    const float IdleBiteTorque =
        FMath::Abs(Provider.GetLastControls().ExternalRearDriveTorquePerWheelNm);
    TestTrue(TEXT("partial clutch carries idle bite torque with zero throttle"),
        IdleBiteTorque > 0.0f);
    TestTrue(TEXT("idle bite remains below powered partial-clutch torque"),
        IdleBiteTorque < PoweredPartialTorque);

    Controls.SetClutch(1.0f);
    Controls.SetDriveline(1, 1, 0.0f);
    FPinkCabChaosCockpitBridge::Apply(Cockpit, *Movement, Controls, Provider);
    TestEqual(TEXT("fully pressed clutch carries zero transmission gear"), Movement->GetTargetGear(), 0);
    return true;
}

#endif

#if WITH_DEV_AUTOMATION_TESTS

#include "Camera/CameraComponent.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabDriverCockpitCameraContractTest,
    "PinkCab.Cockpit.Contract.DriverCamera",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabDriverCockpitCameraContractTest::RunTest(const FString& Parameters)
{
    const APinkCabChaosTatraPawn* Pawn = GetDefault<APinkCabChaosTatraPawn>();
    TInlineComponentArray<UCameraComponent*> Cameras(Pawn);
    const UCameraComponent* DriverCamera = nullptr;
    for (const UCameraComponent* Camera : Cameras)
    {
        if (Camera && Camera->GetFName() == FName(TEXT("DriverCamera")))
        {
            DriverCamera = Camera;
            break;
        }
    }

    TestNotNull(TEXT("Tatra pawn exposes a dedicated DriverCamera component"), DriverCamera);
    return DriverCamera != nullptr;
}

#endif
