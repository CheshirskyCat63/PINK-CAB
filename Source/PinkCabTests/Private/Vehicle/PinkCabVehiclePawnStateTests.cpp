#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Persistence/PinkCabVehicleSnapshot.h"
#include "Vehicle/PinkCabChaosTatraPawn.h"
#include "Vehicle/PinkCabTatraProfile.h"
#include "Vehicle/PinkCabVehicleVisualShellComponent.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabChaosPawnIntegratedVehicleStateTest,
    "PinkCab.Vehicle.ChaosBaseline.Pawn.IntegratedVehicleState",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabChaosPawnIntegratedVehicleStateTest::RunTest(const FString& Parameters)
{
    const APinkCabChaosTatraPawn* Pawn = GetDefault<APinkCabChaosTatraPawn>();
    TestNotNull(TEXT("replaceable visual shell component exists"), Pawn->GetVehicleVisualShell());
    TestEqual(TEXT("fallback visual profile is asset independent"),
        Pawn->GetVehicleVisualProfileId(), FName(TEXT("PinkCab.Visual.Fallback")));

    const FPinkCabTatraProfile Profile = FPinkCabTatraProfile::Canonical();
    TestTrue(TEXT("pawn owns canonical full-fuel crew load"), FMath::IsNearlyEqual(
        Pawn->GetVehicleLoadState().GetTotalMassKg(Profile), 1657.0f, 0.001f));
    TestTrue(TEXT("pawn load is applied to Chaos mass"), FMath::IsNearlyEqual(
        Pawn->GetChaosMovement()->Mass, 1657.0f, 0.001f));

    FPinkCabVehicleSnapshot Snapshot;
    TestTrue(TEXT("pawn vehicle state captures without visual asset identity"), Pawn->CaptureVehicleSnapshot(Snapshot));
    TestEqual(TEXT("snapshot schema stays vehicle-state-only"),
        Snapshot.SchemaVersion, FPinkCabVehicleSnapshot::CurrentSchemaVersion);
    return true;
}

#endif
