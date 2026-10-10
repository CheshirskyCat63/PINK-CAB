#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "EngineUtils.h"
#include "Runtime/PinkCabChaosTatraPawn.h"
#include "Vehicle/PinkCabChaosVehicleMovementComponent.h"

namespace
{
class FClutchEnvelopeCommand final : public IAutomationLatentCommand
{
public:
    explicit FClutchEnvelopeCommand(FAutomationTestBase* InTest) : Test(InTest), Started(FPlatformTime::Seconds()) {}
    bool Update() override
    {
        if (FPlatformTime::Seconds() - Started > 40.0) { Test->AddError(TEXT("clutch envelope fixture timed out")); return true; }
        UWorld* World = AutomationCommon::GetAnyGameWorld();
        if (!World) return false;
        APinkCabChaosTatraPawn* Pawn = nullptr;
        for (TActorIterator<APinkCabChaosTatraPawn> It(World); It; ++It) { Pawn = *It; break; }
        if (!Pawn) return false;
        auto* Movement = Cast<UPinkCabChaosVehicleMovementComponent>(Pawn->GetChaosMovement());
        if (!Movement || Movement->Wheels.Num() != 4) return false;
        if (StageStart < 0.0)
        {
            Pawn->SetSystemMenuOpen(false);
            Pawn->SetActorTickEnabled(false);
            const bool Settling = Case < 0;
            Gear = Settling || Case < 14 ? 1 : -1;
            Coupling = Settling ? 0.0f : Values[Case % 7];
            Health = Settling || (Case / 7) % 2 == 0 ? 1.0f : 0.4f;
            FPinkCabVehicleControlState Controls;
            Controls.SetDriveline(Gear, Gear, Coupling);
            Controls.SetClutch(1.0f - Coupling);
            Controls.SetDrivetrainTorqueCapacity(Health);
            Controls.SetSteering(Gear > 0 ? 0.07f : -0.09f);
            Controls.SetThrottle(0.6f);
            Controls.SetBrake(1.0f);
            Controls.SetHandbrake(0.15f);
            Controls.SetResolvedEngineActuation(true, 0.6f, 0.6f,
                Movement->EngineSetup.MaxTorque, Movement->EngineSetup.MaxTorque * 0.6f);
            Pawn->GetPinkCabDynamicsProvider().ApplyControls(Controls);
            StageStart = World->GetTimeSeconds();
            Samples = 0;
            LargestTorque = 0.0;
            return false;
        }
        const double Time = World->GetTimeSeconds() - StageStart;
        const auto Step = Movement->GetPinkCabDrivelineStepTelemetry();
        if (Case >= 0 && Time >= 0.06 && Step.PhysicsStep != LastStep)
        {
            LastStep = Step.PhysicsStep;
            ++Samples;
            Test->TestTrue(TEXT("all physical controls use one complete prepared frame"), Step.CommandSequence > 0 && Step.bCommandMatchesPreparedFrame);
            Test->TestTrue(TEXT("actual selected gear and continuous pressure reach same physical step"),
                Step.Gear == Gear && Step.NativeGear == Gear && FMath::IsNearlyEqual(Step.Coupling, Coupling, 0.00001f));
            const double ExpectedCapacity = Movement->GetPinkCabClutchConfig().MaxClutchTorqueNm * Coupling * Health;
            Test->TestTrue(TEXT("live torque capacity follows pedal and health without the old0.95 threshold"),
                FMath::Abs(Step.CapacityNm - ExpectedCapacity) <= 0.002 && Step.bNativeJointApplied);
            Test->TestTrue(TEXT("actual native torque stays within continuous capacity"), FMath::Abs(Step.TransferredTorqueNm) <= ExpectedCapacity + 0.002);
            Test->TestTrue(TEXT("integrated connection is passive outside combustion"), Step.ConnectionEnergyDeltaJ <= 0.02 && Step.GearLossJ >= -0.02);
            Test->TestTrue(TEXT("physical front axle is never a propulsion owner"), Step.NativeWheelCount == 4
                && FMath::Abs(Step.NativeWheelDriveNm[0]) < 0.01 && FMath::Abs(Step.NativeWheelDriveNm[1]) < 0.01);
            LargestTorque = FMath::Max(LargestTorque, FMath::Abs(Step.NativeWheelDriveNm[2]));
            if (Coupling == 0.0f)
                Test->TestTrue(TEXT("open clutch clears native rear-wheel torque while gear remains selected"),
                    FMath::Abs(Step.NativeWheelDriveNm[2]) < 0.01 && FMath::Abs(Step.NativeWheelDriveNm[3]) < 0.01);
        }
        if (Time < (Case < 0 ? 2.0 : 0.25)) return false;
        if (Case >= 0)
        {
            Test->TestTrue(TEXT("each pressure/health/gear fixture contains multiple physical outputs"), Samples >= 2);
            if (Coupling > 0.0f) Test->TestTrue(TEXT("nonzero partial pressure reaches native driven wheels"), LargestTorque > 1.0);
            Test->AddInfo(FString::Printf(TEXT("T6_CLUTCH_ENVELOPE case=%d gear=%d c=%.3f health=%.1f max_rear_nm=%.4f samples=%d"),
                Case, Gear, Coupling, Health, LargestTorque, Samples));
        }
        if (++Case < 28) { StageStart = -1.0; return false; }
        Pawn->SetActorTickEnabled(true);
        return true;
    }
private:
    FAutomationTestBase* Test;
    double Started;
    double StageStart = -1.0;
    int32 Case = -1;
    int32 Gear = 0;
    float Coupling = 0.0f;
    float Health = 1.0f;
    int32 Samples = 0;
    uint64 LastStep = 0;
    double LargestTorque = 0.0;
    const float Values[7] = {0.0f, 0.25f, 0.5f, 0.949f, 0.95f, 0.999f, 1.0f};
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPinkCabClutchEnvelopeRuntimeTest,
    "PinkCab.Vehicle.Actuation.ClutchEnvelopeRuntime",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabClutchEnvelopeRuntimeTest::RunTest(const FString& Parameters)
{
    if (!AutomationOpenMap(TEXT("/Game/Dev/Maps/L_PinkCab_L1_EndlessStraight"), true)) return false;
    ADD_LATENT_AUTOMATION_COMMAND(FClutchEnvelopeCommand(this));
    return true;
}
#endif
