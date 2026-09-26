#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "EngineUtils.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "HAL/PlatformTime.h"
#include "Interaction/PinkCabInteractionModel.h"
#include "Runtime/PinkCabChaosTatraPawn.h"

class FPinkCabEngineOffCoastCommand final : public IAutomationLatentCommand
{
public:
    explicit FPinkCabEngineOffCoastCommand(FAutomationTestBase* InTest)
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

        UChaosWheeledVehicleMovementComponent* Movement =
            Pawn->GetChaosMovement();
        USkeletalMeshComponent* Mesh = Pawn->GetMesh();
        Test->TestNotNull(TEXT("live Chaos movement exists"), Movement);
        Test->TestNotNull(TEXT("live vehicle mesh exists"), Mesh);
        if (!Movement || !Mesh)
        {
            return true;
        }

        if (!bInitialized)
        {
            if (!Movement->bMechanicalSimEnabled)
            {
                Test->TestTrue(TEXT("ignition starts live engine before coast fixture"),
                    Pawn->ApplyCockpitInteraction(
                        {FName(TEXT("Ignition")),
                         EPinkCabInteractionGesture::PressHold,
                         1}));
            }
            Test->TestTrue(TEXT("precondition mechanical sim is Running"),
                Movement->bMechanicalSimEnabled);

            const FVector FixtureVelocity =
                Pawn->GetActorForwardVector() * 800.0f;
            Mesh->SetPhysicsLinearVelocity(FixtureVelocity);
            InitialSpeedCmPerSec =
                Mesh->GetPhysicsLinearVelocity().Size();
            Test->TestTrue(TEXT("coast fixture has meaningful initial speed"),
                InitialSpeedCmPerSec > 500.0f);

            Test->TestTrue(TEXT("key-off interaction applies"),
                Pawn->ApplyCockpitInteraction(
                    {FName(TEXT("Ignition")),
                     EPinkCabInteractionGesture::PressHold,
                     1}));
            Test->TestFalse(TEXT("key-off disables mechanical engine sim"),
                Movement->bMechanicalSimEnabled);
            Test->TestEqual(TEXT("key-off sends zero Chaos throttle"),
                Movement->GetThrottleInput(), 0.0f);

            const float ImmediateSpeed =
                Mesh->GetPhysicsLinearVelocity().Size();
            Test->TestTrue(TEXT("key-off preserves chassis inertia immediately"),
                ImmediateSpeed > InitialSpeedCmPerSec * 0.80f);

            ObserveStartSeconds = FPlatformTime::Seconds();
            bInitialized = true;
            return false;
        }

        if (FPlatformTime::Seconds() - ObserveStartSeconds < 0.20)
        {
            return false;
        }

        const float CoastSpeed =
            Mesh->GetPhysicsLinearVelocity().Size();
        Test->TestTrue(TEXT("engine-off body is still physically free to coast"),
            CoastSpeed > 10.0f);
        Test->TestEqual(TEXT("engine-off coast still has zero Chaos throttle"),
            Movement->GetThrottleInput(), 0.0f);
        return true;
    }

private:
    FAutomationTestBase* Test = nullptr;
    bool bInitialized = false;
    double ObserveStartSeconds = 0.0;
    float InitialSpeedCmPerSec = 0.0f;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabVehicleEngineOffCoastRuntimeTest,
    "PinkCab.Vehicle.Physics.EngineState.LiveEngineOffCoast",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabVehicleEngineOffCoastRuntimeTest::RunTest(const FString& Parameters)
{
    const bool bOpened = AutomationOpenMap(
        TEXT("/Game/Dev/Maps/L_PinkCab_ChaosWeave"),
        true);
    TestTrue(TEXT("vehicle runtime map opens"), bOpened);
    if (!bOpened)
    {
        return false;
    }

    ADD_LATENT_AUTOMATION_COMMAND(
        FPinkCabEngineOffCoastCommand(this));
    return true;
}

#endif
