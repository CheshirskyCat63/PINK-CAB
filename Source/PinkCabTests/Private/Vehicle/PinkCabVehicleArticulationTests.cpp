#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Runtime/PinkCabVehicleArticulationLogic.h"
#include <limits>

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabVehicleArticulationStateTest,
    "PinkCab.Vehicle.Presentation.Articulation.State",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabVehicleArticulationStateTest::RunTest(const FString& Parameters)
{
    FPinkCabVehicleArticulationState State;
    TestEqual(TEXT("starts closed"), State.Current, 0.0f);
    TestTrue(TEXT("toggle opens target"), State.Toggle());
    TestEqual(TEXT("open target"), State.Target, 1.0f);
    TestTrue(TEXT("advance is valid"), State.Advance(0.25f, 1.0f));
    TestEqual(TEXT("quarter travel"), State.Current, 0.25f);
    TestTrue(TEXT("toggle closes target"), !State.Toggle());
    TestTrue(TEXT("returns toward closed"), State.Advance(0.10f, 1.0f));
    TestEqual(TEXT("closed travel"), State.Current, 0.15f);
    TestFalse(TEXT("negative delta rejected"), State.Advance(-0.1f, 1.0f));
    TestFalse(TEXT("zero travel rejected"), State.Advance(0.1f, 0.0f));
    TestFalse(TEXT("nan target rejected"),
        State.SetTarget(std::numeric_limits<float>::quiet_NaN()));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabVehicleArticulationRotationTest,
    "PinkCab.Vehicle.Presentation.Articulation.Rotation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabVehicleArticulationRotationTest::RunTest(const FString& Parameters)
{
    const FQuat Closed = PinkCabArticulationRotation(FVector::UpVector, 70.0f, 0.0f);
    TestTrue(TEXT("closed identity"), Closed.Equals(FQuat::Identity, KINDA_SMALL_NUMBER));

    const FQuat Open = PinkCabArticulationRotation(FVector::UpVector, 70.0f, 1.0f);
    const FVector Turned = Open.RotateVector(FVector::ForwardVector);
    TestTrue(TEXT("open rotates forward vector"), Turned.Y > 0.9f);

    TestTrue(TEXT("bad axis fails closed"),
        PinkCabArticulationRotation(FVector::ZeroVector, 70.0f, 1.0f)
            .Equals(FQuat::Identity, KINDA_SMALL_NUMBER));
    return true;
}

#endif
