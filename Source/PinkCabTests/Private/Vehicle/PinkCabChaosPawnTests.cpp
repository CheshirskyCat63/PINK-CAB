#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Vehicle/PinkCabChaosTatraPawn.h"
#include "Vehicle/PinkCabChaosWheelFront.h"
#include "Vehicle/PinkCabChaosWheelRear.h"
#include "Vehicle/PinkCabTatraProfile.h"

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

#endif
