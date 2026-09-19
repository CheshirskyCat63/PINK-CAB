#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "EngineUtils.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Vehicle/PinkCabVehicleStateSnapshot.h"
#include "Runtime/PinkCabChaosTatraPawn.h"
#include "Vehicle/PinkCabVehicleDamageProfile.h"
#include "Vehicle/PinkCabVehicleVisualProfile.h"

class FPinkCabVehicleLivePawnRoundTripCommand final : public IAutomationLatentCommand
{
public:
    explicit FPinkCabVehicleLivePawnRoundTripCommand(FAutomationTestBase* InTest) : Test(InTest) {}
    virtual bool Update() override;

private:
    FAutomationTestBase* Test = nullptr;
};

bool FPinkCabVehicleLivePawnRoundTripCommand::Update()
{
    UWorld* World = AutomationCommon::GetAnyGameWorld();
    if (!World) return false;

    APinkCabChaosTatraPawn* Pawn = nullptr;
    for (TActorIterator<APinkCabChaosTatraPawn> It(World); It; ++It)
    {
        Pawn = *It;
        break;
    }
    Test->TestNotNull(TEXT("playable map owns live Tatra pawn"), Pawn);
    if (!Pawn) return true;
    Pawn->SetActorTickEnabled(false);
    const FPinkCabTatraProfile Profile = FPinkCabTatraProfile::Canonical();
    UChaosWheeledVehicleMovementComponent* Movement = Pawn->GetChaosMovement();
    Test->TestNotNull(TEXT("live pawn owns Chaos movement"), Movement);
    if (!Movement) return true;

    Test->TestTrue(TEXT("initial canonical mass is live"),
        FMath::IsNearlyEqual(Movement->Mass, 1657.0f, 0.001f));
    USkeletalMesh* PhysicsMeshBefore = Pawn->GetMesh()->GetSkeletalMeshAsset();
    Test->TestNotNull(TEXT("physics chassis mesh remains assigned"), PhysicsMeshBefore);

    FPinkCabVehicleDamageProfile DamageProfile(TEXT("PinkCab.Damage.CD855Runtime"));
    Test->TestTrue(TEXT("runtime authored damage zone registers"), DamageProfile.TryAddZone(
        TEXT("BrakeHydraulic"), EPinkCabVehicleHealthChannel::BrakeHydraulic, 0.40f));
    Test->TestTrue(TEXT("live pawn accepts mesh-independent damage profile"),
        Pawn->SetVehicleDamageProfile(DamageProfile));
    Test->TestEqual(TEXT("live pawn exposes damage profile id"),
        Pawn->GetVehicleDamageProfileId(), DamageProfile.GetProfileId());
    Test->TestFalse(TEXT("below authored threshold does not mutate vehicle health"),
        Pawn->ApplyAuthoredVehicleHit(TEXT("BrakeHydraulic"), 0.40f));
    Test->TestEqual(TEXT("below threshold preserves hydraulic health"),
        Pawn->GetVehicleHealthState().GetHealth(EPinkCabVehicleHealthChannel::BrakeHydraulic), 1.0f);
    Test->TestTrue(TEXT("above authored threshold mutates live vehicle health"),
        Pawn->ApplyAuthoredVehicleHit(TEXT("BrakeHydraulic"), 0.50f));
    Test->TestEqual(TEXT("authored hydraulic consequence is live"),
        Pawn->GetVehicleHealthState().GetHealth(EPinkCabVehicleHealthChannel::BrakeHydraulic), 0.50f);

    Test->TestTrue(TEXT("fuel mutation applies to live Chaos"), Pawn->SetFuelMassKg(40.0f, -10.0f));
    const FPinkCabStableId FareId(TEXT("fare:cd855:live"));
    const TArray<FPinkCabVehicleLoadItem> FareItems = {
        FPinkCabVehicleLoadItem(65.0f, -35.0f),
        FPinkCabVehicleLoadItem(80.0f, -45.0f)};
    Test->TestTrue(TEXT("fare group boards exactly once"),
        Pawn->TrySetFarePassengerGroup(FareId, FareItems));
    Test->TestTrue(TEXT("live loaded mass reaches expected fixture"),
        FMath::IsNearlyEqual(Movement->Mass, 1742.0f, 0.001f));
    Test->TestTrue(TEXT("rear load shifts live COM rearward"), Movement->CenterOfMassOverride.X < 0.0f);

    Test->TestTrue(TEXT("authored brake damage applies"), Pawn->ApplyVehicleHit(
        FPinkCabVehicleHitEvent(EPinkCabVehicleHealthChannel::Brake, 0.25f, false)));
    Test->TestEqual(TEXT("brake health is persistent state"),
        Pawn->GetVehicleHealthState().GetHealth(EPinkCabVehicleHealthChannel::Brake), 0.75f);

    FPinkCabVehicleStateSnapshot Snapshot;
    Test->TestTrue(TEXT("live vehicle snapshot captures"), Pawn->CaptureVehicleSnapshot(Snapshot));

    FPinkCabVehicleVisualProfile VisualSwap = FPinkCabVehicleVisualProfile::Fallback();
    VisualSwap.ProfileId = TEXT("PinkCab.Visual.CD855SwapProof");
    VisualSwap.DriverHeadTransform.SetLocation(FVector(-12.0f, -36.0f, 126.0f));
    Test->TestTrue(TEXT("visual profile swaps without physics rewrite"),
        Pawn->ApplyVehicleVisualProfile(VisualSwap));
    Test->TestEqual(TEXT("visual profile id changes independently"), Pawn->GetVehicleVisualProfileId(), VisualSwap.ProfileId);
    Test->TestEqual(TEXT("visual swap preserves physics chassis mesh"),
        Pawn->GetMesh()->GetSkeletalMeshAsset(), PhysicsMeshBefore);

    Test->TestTrue(TEXT("post-snapshot fuel mutation applies"), Pawn->SetFuelMassKg(0.0f));
    Test->TestTrue(TEXT("post-snapshot fare group removes"), Pawn->RemoveFarePassengerGroup(FareId));
    Test->TestTrue(TEXT("terminal brake damage applies"), Pawn->ApplyVehicleHit(
        FPinkCabVehicleHitEvent(EPinkCabVehicleHealthChannel::Brake, 1.0f, false)));
    Test->TestTrue(TEXT("terminal capability state is observable"), Pawn->IsVehicleTerminal());

    Test->TestTrue(TEXT("snapshot restores live pawn atomically"), Pawn->RestoreVehicleSnapshot(Snapshot));
    Test->TestFalse(TEXT("restored nonterminal health is live"), Pawn->IsVehicleTerminal());
    Test->TestEqual(TEXT("brake health restored"),
        Pawn->GetVehicleHealthState().GetHealth(EPinkCabVehicleHealthChannel::Brake), 0.75f);
    Test->TestTrue(TEXT("fare group restored"), Pawn->GetVehicleLoadState().HasFarePassengerGroup(FareId));
    Test->TestTrue(TEXT("restored load reapplies to Chaos"),
        FMath::IsNearlyEqual(Movement->Mass, 1742.0f, 0.001f));
    Test->TestEqual(TEXT("snapshot restore never rewrites visual profile"),
        Pawn->GetVehicleVisualProfileId(), VisualSwap.ProfileId);
    Test->TestEqual(TEXT("snapshot restore preserves physics chassis mesh"),
        Pawn->GetMesh()->GetSkeletalMeshAsset(), PhysicsMeshBefore);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabVehicleLivePawnRoundTripTest,
    "PinkCab.Vehicle.LiveState.RuntimeRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabVehicleLivePawnRoundTripTest::RunTest(const FString& Parameters)
{
    const bool bOpened = AutomationOpenMap(TEXT("/Game/Dev/Maps/L_PinkCab_ChaosWeave"), true);
    TestTrue(TEXT("vehicle runtime map opens"), bOpened);
    if (!bOpened) return false;
    ADD_LATENT_AUTOMATION_COMMAND(FPinkCabVehicleLivePawnRoundTripCommand(this));
    return true;
}

#endif