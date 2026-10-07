#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "EngineUtils.h"
#include "HAL/PlatformTime.h"
#include "ChaosVehicleWheel.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Cockpit/PinkCabCockpitVisualDriverComponent.h"
#include "Runtime/PinkCabChaosTatraPawn.h"
#include "Runtime/PinkCabVehicleVisualShellComponent.h"
#include "Vehicle/PinkCabVehicleInputFrame.h"

class FPinkCabTatra613Rig06SteeringVisualCommand final : public IAutomationLatentCommand
{
public:
    explicit FPinkCabTatra613Rig06SteeringVisualCommand(FAutomationTestBase* InTest)
        : Test(InTest) {}

    virtual bool Update() override
    {
        UWorld* World = AutomationCommon::GetAnyGameWorld();
        if (!World)
        {
            return false;
        }

        APinkCabChaosTatraPawn* Pawn = nullptr;
        for (TActorIterator<APinkCabChaosTatraPawn> It(World); It; ++It)
        {
            Pawn = *It;
            break;
        }
        if (!Pawn)
        {
            return false;
        }

        UPinkCabVehicleVisualShellComponent* Shell = Pawn->GetVehicleVisualShell();
        UChaosWheeledVehicleMovementComponent* Movement = Pawn->GetChaosMovement();
        Test->TestNotNull(TEXT("RIG06 steering visual has shell"), Shell);
        Test->TestNotNull(TEXT("RIG06 steering visual has Chaos movement"), Movement);
        if (!Shell || !Movement || Movement->Wheels.Num() < 2)
        {
            return true;
        }

        if (Phase == 0)
        {
            Test->TestEqual(
                TEXT("steering visual test uses RIG06 profile"),
                Pawn->GetVehicleVisualProfileId(),
                FName(TEXT("PinkCab.Visual.Tatra613.Rig06.TexturedOpenables")));
            Test->TestTrue(
                TEXT("steering wheel rest pose is readable"),
                Shell->GetPoseableBoneTransform(TEXT("Steering_Wheel"), SteeringRest));
            Test->TestTrue(
                TEXT("front-left wheel rest pose is readable"),
                Shell->GetPoseableBoneTransform(TEXT("Phys_Wheel_FL"), FrontLeftRest));
            Test->TestTrue(
                TEXT("front-right wheel rest pose is readable"),
                Shell->GetPoseableBoneTransform(TEXT("Phys_Wheel_FR"), FrontRightRest));
            Pawn->SetSystemMenuOpen(false);
            FPinkCabVehicleInputFrame Frame;
            Pawn->ApplyVehicleInputFrame(Frame, 2000.0f, 0.10f);
            StartedSeconds = FPlatformTime::Seconds();
            Phase = 1;
            return false;
        }

        if ((FPlatformTime::Seconds() - StartedSeconds) < 0.10)
        {
            return false;
        }

        const float SemanticSteering = Pawn->GetSteeringCommand();
        UChaosVehicleWheel* FrontLeft = Movement->Wheels[0];
        UChaosVehicleWheel* FrontRight = Movement->Wheels[1];
        Test->TestNotNull(TEXT("front-left Chaos wheel exists"), FrontLeft);
        Test->TestNotNull(TEXT("front-right Chaos wheel exists"), FrontRight);
        if (!FrontLeft || !FrontRight)
        {
            return true;
        }

        const float FrontLeftSteerDeg = FrontLeft->GetSteerAngle();
        const float FrontRightSteerDeg = FrontRight->GetSteerAngle();
        Test->TestTrue(TEXT("semantic right remains positive at live pawn"),
            SemanticSteering > 0.0f);
        Test->TestTrue(TEXT("front physical wheels steer right for semantic right"),
            FrontLeftSteerDeg > 0.0f && FrontRightSteerDeg > 0.0f);

        Test->TestTrue(TEXT("live wheel presentation sync succeeds"),
            Pawn->SyncWheelPresentationFromChaos());
        Shell->RefreshPoseableBoneTransforms();

        FTransform FrontLeftVisual;
        FTransform FrontRightVisual;
        FTransform SteeringVisual;
        Test->TestTrue(TEXT("front-left visual pose is readable"),
            Shell->GetPoseableBoneTransform(TEXT("Phys_Wheel_FL"), FrontLeftVisual));
        Test->TestTrue(TEXT("front-right visual pose is readable"),
            Shell->GetPoseableBoneTransform(TEXT("Phys_Wheel_FR"), FrontRightVisual));
        Test->TestTrue(TEXT("steering wheel visual pose is readable"),
            Shell->GetPoseableBoneTransform(TEXT("Steering_Wheel"), SteeringVisual));
        const FQuat FrontLeftExpectedSteer(
            FVector::UpVector,
            FMath::DegreesToRadians(FrontLeftSteerDeg));
        const FQuat FrontLeftExpectedSpin(
            FVector::YAxisVector,
            FMath::DegreesToRadians(FrontLeft->GetRotationAngle()));
        const FQuat FrontLeftExpected =
            (FrontLeftExpectedSteer
                * FrontLeftRest.GetRotation()
                * FrontLeftExpectedSpin).GetNormalized();

        const FQuat FrontRightExpectedSteer(
            FVector::UpVector,
            FMath::DegreesToRadians(FrontRightSteerDeg));
        const FQuat FrontRightExpectedSpin(
            FVector::YAxisVector,
            FMath::DegreesToRadians(FrontRight->GetRotationAngle()));
        const FQuat FrontRightExpected =
            (FrontRightExpectedSteer
                * FrontRightRest.GetRotation()
                * FrontRightExpectedSpin).GetNormalized();

        Test->TestTrue(TEXT("front-left poseable wheel follows live Chaos steer/spin"),
            FrontLeftVisual.GetRotation().Equals(FrontLeftExpected, 0.01f));
        Test->TestTrue(TEXT("front-right poseable wheel follows live Chaos steer/spin"),
            FrontRightVisual.GetRotation().Equals(FrontRightExpected, 0.01f));

        const float SteeringWheelAngleDeg =
            FMath::Clamp(SemanticSteering, -1.0f, 1.0f) * 450.0f;
        const FQuat SteeringWheelLocalTurn(
            FVector::YAxisVector,
            FMath::DegreesToRadians(SteeringWheelAngleDeg));
        const FQuat SteeringWheelExpected =
            (SteeringRest.GetRotation() * SteeringWheelLocalTurn).GetNormalized();

        Test->TestTrue(TEXT("semantic right turns the RIG06 steering wheel visually right"),
            SteeringWheelAngleDeg > 0.0f);
        Test->TestTrue(TEXT("RIG06 steering wheel pose matches semantic adapter"),
            SteeringVisual.GetRotation().Equals(SteeringWheelExpected, 0.01f));

        Test->AddInfo(FString::Printf(
            TEXT("RIG06 steering visual: semantic=%.4f physicalFront=(%.3f,%.3f) steeringWheel=%.2fdeg"),
            SemanticSteering,
            FrontLeftSteerDeg,
            FrontRightSteerDeg,
            SteeringWheelAngleDeg));
        return true;
    }

private:
    FAutomationTestBase* Test = nullptr;
    int32 Phase = 0;
    double StartedSeconds = 0.0;
    FTransform SteeringRest = FTransform::Identity;
    FTransform FrontLeftRest = FTransform::Identity;
    FTransform FrontRightRest = FTransform::Identity;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabTatra613Rig06SteeringVisualRuntimeTest,
    "PinkCab.Vehicle.Visual.Tatra613Rig06SteeringRuntime",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabTatra613Rig06SteeringVisualRuntimeTest::RunTest(const FString& Parameters)
{
    const bool bOpened =
        AutomationOpenMap(TEXT("/Game/Dev/Maps/L_PinkCab_L1_EndlessStraight"), true);
    TestTrue(TEXT("RIG06 steering runtime map opens"), bOpened);
    if (!bOpened)
    {
        return false;
    }

    ADD_LATENT_AUTOMATION_COMMAND(
        FPinkCabTatra613Rig06SteeringVisualCommand(this));
    return true;
}

#endif
