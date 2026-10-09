#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "EngineUtils.h"
#include "ChaosVehicleWheel.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Runtime/PinkCabChaosTatraPawn.h"
#include "Vehicle/PinkCabChaosVehicleMovementComponent.h"
#include "Vehicle/PinkCabVehicleStateSnapshot.h"

namespace
{
FPinkCabVehicleStateSnapshot LoadFixture(const FPinkCabVehicleStateSnapshot& Reference, int32 Index)
{
    FPinkCabVehicleStateSnapshot Result = Reference;
    Result.Load.Passengers.Reset();
    Result.Load.FarePassengers.Reset();
    Result.Load.FarePassengerGroupId.Reset();
    Result.Load.bFarePassengerGroupActive = false;
    Result.Load.FuelMassKg = Index == 0 ? 0.0f : 100.0f;
    Result.Load.HeroineMassKg = Index == 0 ? 0.0f : 58.0f;
    Result.Load.DaughterMassKg = Index == 0 ? 0.0f : 49.0f;
    if (Index == 2)
    {
        // Explicit symmetric test load; these are not new production seat sockets.
        for (const FVector& Position : {FVector(-55,-45,73.5), FVector(-55,45,73.5),
            FVector(-125,-45,73.5), FVector(-125,45,73.5), FVector(-125,0,73.5)})
        {
            Result.Load.Passengers.Add({90.0f, float(Position.X), float(Position.Y), float(Position.Z)});
        }
    }
    return Result;
}

class FTatraLoadEnvelopeCommand final : public IAutomationLatentCommand
{
public:
    explicit FTatraLoadEnvelopeCommand(FAutomationTestBase* InTest)
        : Test(InTest), Started(FPlatformTime::Seconds()) {}

    bool Update() override
    {
        if (FPlatformTime::Seconds() - Started > 45.0)
        {
            Test->AddError(TEXT("load envelope runtime timed out"));
            return true;
        }
        UWorld* World = AutomationCommon::GetAnyGameWorld();
        if (!World) return false;
        APinkCabChaosTatraPawn* Pawn = nullptr;
        for (TActorIterator<APinkCabChaosTatraPawn> It(World); It; ++It) { Pawn = *It; break; }
        if (!Pawn || !Pawn->GetMesh()) return false;
        Pawn->SetSystemMenuOpen(false);
        auto* Movement = Cast<UPinkCabChaosVehicleMovementComponent>(Pawn->GetChaosMovement());
        FBodyInstance* Body = Pawn->GetMesh()->GetBodyInstance();
        if (!Movement || !Body || !Body->IsValidBodyInstance()) return false;
        if (StageStarted < 0.0)
        {
            if (!BeginStage(*Pawn, *Body)) return true;
            StageStarted = World->GetTimeSeconds();
            return false;
        }
        const double Elapsed = World->GetTimeSeconds() - StageStarted;
        if (Elapsed < 3.0) return false;
        Sample(*Movement);
        if (Elapsed < 4.0) return false;
        VerifyStage(*Pawn, *Movement, *Body, *World);
        ++Stage;
        StageStarted = -1.0;
        Samples = 0;
        FrontForce = RearForce = SagSum = 0.0;
        bContacts = bTravelReserve = true;
        return Stage == 4;
    }

private:
    bool BeginStage(APinkCabChaosTatraPawn& Pawn, FBodyInstance& Body)
    {
        if (Stage == 0 && !Test->TestTrue(TEXT("reference state captures"),
            Pawn.CaptureVehicleSnapshot(Reference))) return false;
        const auto Fixture = LoadFixture(Reference, Stage);
        if (!Test->TestTrue(TEXT("load fixture restores"), Pawn.RestoreVehicleSnapshot(Fixture))) return false;
        if (!Test->TestTrue(TEXT("repeated restore is idempotent"), Pawn.RestoreVehicleSnapshot(Fixture))) return false;
        // Exercise the native rebuild callback instead of only stored component fields.
        Body.UpdateMassProperties();
        Body.WakeInstance();
        return true;
    }

    void Sample(UChaosWheeledVehicleMovementComponent& Movement)
    {
        ++Samples;
        bContacts &= Movement.Wheels.Num() == 4;
        for (int32 Index = 0; Index < Movement.Wheels.Num(); ++Index)
        {
            const auto& State = Movement.GetWheelState(Index);
            const UChaosVehicleWheel* Wheel = Movement.Wheels[Index];
            bContacts &= State.bInContact && Wheel != nullptr;
            if (Index < 2) FrontForce += State.SpringForce;
            else RearForce += State.SpringForce;
            if (!Wheel) { bTravelReserve = false; continue; }
            const double Offset = Wheel->GetSuspensionOffset();
            SagSum += Offset / 4.0;
            bTravelReserve &= Offset < Wheel->SuspensionMaxRaise - 0.01
                && Offset > -Wheel->SuspensionMaxDrop + 0.01;
        }
    }

    void VerifyStage(APinkCabChaosTatraPawn& Pawn, UPinkCabChaosVehicleMovementComponent& Movement,
        FBodyInstance& Body, UWorld& World)
    {
        FPinkCabVehicleMassProperties Expected;
        const auto Profile = FPinkCabTatraProfile::Canonical();
        if (!Test->TestTrue(TEXT("fixture mass properties resolve"),
            Pawn.GetVehicleLoadState().TryGetMassProperties(Profile, Expected))) return;
        const double TargetMass = Stage == 0 ? 1450.0 : Stage == 2 ? 2107.0 : 1657.0;
        Test->AddInfo(FString::Printf(TEXT("T6_LOAD_SLEEP stage=%d awake=%d threshold=%.3f"),
            Stage, Body.IsInstanceAwake(), Movement.SleepThreshold));
        const FTransform ActualFrame = Body.GetMassSpaceLocal();
        const FVector ActualInertia = Body.GetBodyInertiaTensor();
        const double TotalForce = FrontForce + RearForce;
        const double RearShare = TotalForce > 0.0 ? RearForce / TotalForce : 0.0;
        const double PredictedShare = (135.0 - Expected.CenterCm.X) / 298.0;
        const double MeanSag = Samples > 0 ? SagSum / Samples : 0.0;
        Test->TestTrue(TEXT("physical body uses exact empty/reference/max mass"),
            FMath::IsNearlyEqual(double(Body.GetBodyMass()), TargetMass, 0.05));
        Test->TestTrue(TEXT("physical mass frame includes all three coordinates after rebuild"),
            ActualFrame.GetLocation().Equals(Expected.CenterCm, 0.01));
        Test->TestTrue(TEXT("physical principal axes survive native mass recalculation"),
            ActualFrame.GetRotation().Equals(Expected.PrincipalRotation, 0.0001));
        Test->TestTrue(TEXT("physical inertia equals native combined load inertia"),
            ActualInertia.Equals(Expected.PrincipalInertiaKgCm2, 4.0));
        Test->TestTrue(TEXT("four contacts remain throughout settled sample"), bContacts && Samples >= 10);
        Test->TestTrue(TEXT("empty/reference/max keep suspension travel reserve"), bTravelReserve);
        Test->TestTrue(TEXT("settled rear spring share follows declared load moments"),
            FMath::Abs(RearShare - PredictedShare) <= 0.02);
        Test->TestTrue(TEXT("chassis settles vertically without a velocity reset"),
            FMath::Abs(Pawn.GetVelocity().Z) < 5.0);
        if (Stage == 1) ReferenceSag = MeanSag;
        if (Stage == 2) Test->TestTrue(TEXT("added mass increases actual suspension compression"), MeanSag > ReferenceSag + 0.1);
        if (Stage == 3) Test->TestTrue(TEXT("unload and repeated restore return reference sag"),
            FMath::Abs(MeanSag - ReferenceSag) < 0.5);
        Test->AddInfo(FString::Printf(TEXT("T5_LOAD stage=%d samples=%d mass=%.3f com=%s inertia=%s rear_share=%.6f predicted=%.6f sag_cm=%.4f spring_raw=%.3f gravity_cm_s2=%.3f vz=%.4f"),
            Stage, Samples, Body.GetBodyMass(), *ActualFrame.GetLocation().ToString(),
            *ActualInertia.ToString(), RearShare, PredictedShare, MeanSag,
            TotalForce / FMath::Max(Samples, 1), World.GetGravityZ(), Pawn.GetVelocity().Z));
    }

    FAutomationTestBase* Test;
    double Started;
    double StageStarted = -1.0;
    FPinkCabVehicleStateSnapshot Reference;
    int32 Stage = 0;
    int32 Samples = 0;
    double FrontForce = 0.0;
    double RearForce = 0.0;
    double SagSum = 0.0;
    double ReferenceSag = 0.0;
    bool bContacts = true;
    bool bTravelReserve = true;
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPinkCabTatraLoadEnvelopeRuntimeTest,
    "PinkCab.Vehicle.PhysicalFoundation.LiveLoadEnvelope",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabTatraLoadEnvelopeRuntimeTest::RunTest(const FString& Parameters)
{
    if (!TestTrue(TEXT("load envelope map opens"),
        AutomationOpenMap(TEXT("/Game/Dev/Maps/L_PinkCab_L1_EndlessStraight"), true))) return false;
    ADD_LATENT_AUTOMATION_COMMAND(FTatraLoadEnvelopeCommand(this));
    return true;
}
#endif
