#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "EngineUtils.h"
#include "ChaosVehicleWheel.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkinnedAsset.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Runtime/PinkCabChaosTatraPawn.h"
#include "Runtime/PinkCabVehicleVisualShellComponent.h"

class FPinkCabTatraWheelContactCommand final : public IAutomationLatentCommand
{
public:
    explicit FPinkCabTatraWheelContactCommand(FAutomationTestBase* InTest)
        : Test(InTest), Started(FPlatformTime::Seconds()) {}

    virtual bool Update() override
    {
        if (FPlatformTime::Seconds() - Started > 15.0)
        {
            Test->AddError(TEXT("Tatra contact fixture timed out"));
            return true;
        }
        UWorld* World = AutomationCommon::GetAnyGameWorld();
        if (!World) return false;
        APinkCabChaosTatraPawn* Pawn = nullptr;
        for (TActorIterator<APinkCabChaosTatraPawn> It(World); It; ++It) { Pawn = *It; break; }
        if (!Pawn) return false;
        Pawn->SetSystemMenuOpen(false);
        if (SettlingStarted == 0.0) SettlingStarted = FPlatformTime::Seconds();
        if (FPlatformTime::Seconds() - SettlingStarted < 2.0) return false;
        const UPinkCabVehicleVisualShellComponent* Shell = Pawn->GetVehicleVisualShell();
        UChaosWheeledVehicleMovementComponent* Movement = Pawn->GetChaosMovement();
        if (!Test->TestNotNull(TEXT("live visual shell"), Shell)
            || !Test->TestNotNull(TEXT("live Chaos movement"), Movement)) return true;
        const UPoseableMeshComponent* Exterior = Shell->GetExteriorPoseablePresentation();
        if (!Test->TestNotNull(TEXT("live authored exterior"), Exterior)) return true;
        if (!Test->TestEqual(TEXT("four physics wheels"), Movement->Wheels.Num(), 4)) return true;
        if (const FBodyInstance* Body = Pawn->GetMesh()->GetBodyInstance())
        {
            Test->AddInfo(FString::Printf(TEXT("TATRA_BODY requested_mass_kg=%.3f simulated_body_mass_kg=%.3f com_world=(%s)"),
                Movement->Mass, Body->GetBodyMass(), *Body->GetCOMPosition().ToString()));
            Test->TestTrue(TEXT("simulated chassis mass matches the declared live load"),
                FMath::IsNearlyEqual(Body->GetBodyMass(), Movement->Mass, 0.1f));
        }
        const FName Bones[] = {TEXT("Phys_Wheel_FL"), TEXT("Phys_Wheel_FR"), TEXT("Phys_Wheel_BL"), TEXT("Phys_Wheel_BR")};
        for (int32 Index = 0; Index < 4; ++Index)
        {
            UChaosVehicleWheel* Wheel = Movement->Wheels[Index];
            if (!Test->TestNotNull(TEXT("physics wheel initialized"), Wheel)) continue;
            FTransform Pose;
            if (!Test->TestTrue(TEXT("visible wheel pose readable"), Shell->GetPoseableBoneTransform(Bones[Index], Pose))) continue;
            const FVector Visible = Exterior->GetComponentTransform().TransformPosition(Pose.GetLocation());
            const FWheelStatus& State = Movement->GetWheelState(Index);
            const FVector Bottom = Visible - Pawn->GetActorUpVector() * 32.13f;
            const FVector Error = Bottom - State.ContactPoint;
            const FVector NativeRest = Pawn->GetMesh()->GetSkinnedAsset()->GetComposedRefPoseMatrix(
                Movement->WheelSetups[Index].BoneName).GetOrigin();
            Test->AddInfo(FString::Printf(
                TEXT("TATRA_CONTACT wheel=%s visible=(%s) physics=(%s) contact=(%s) error=(%s) radius=%.3f suspension=%.3f force=%.3f grounded=%d"),
                *Bones[Index].ToString(), *Visible.ToString(), *Wheel->Location.ToString(),
                *State.ContactPoint.ToString(), *Error.ToString(), Wheel->WheelRadius,
                Wheel->GetSuspensionOffset(), State.SpringForce, State.bInContact));
            Test->AddInfo(FString::Printf(TEXT("TATRA_REST wheel=%d native=(%s) chassis=(%s) mass_kg=%.3f"),
                Index, *NativeRest.ToString(), *Pawn->GetMesh()->GetComponentTransform().ToString(), Pawn->GetMesh()->GetMass()));
            Test->TestTrue(TEXT("settled wheel has a road contact"), State.bInContact);
            Test->TestTrue(TEXT("visible tyre meets its physics contact within 2 cm"), Error.Size() < 2.0f);
        }
        return true;
    }
private:
    FAutomationTestBase* Test;
    double Started;
    double SettlingStarted = 0.0;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPinkCabTatraWheelContactRuntimeTest,
    "PinkCab.Vehicle.Visual.TatraWheelContacts", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabTatraWheelContactRuntimeTest::RunTest(const FString& Parameters)
{
    if (!AutomationOpenMap(TEXT("/Game/Dev/Maps/L_PinkCab_L1_EndlessStraight"), true)) return false;
    ADD_LATENT_AUTOMATION_COMMAND(FPinkCabTatraWheelContactCommand(this));
    return true;
}

#endif
