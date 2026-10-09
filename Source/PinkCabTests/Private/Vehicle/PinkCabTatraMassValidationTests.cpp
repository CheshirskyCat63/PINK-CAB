#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Persistence/PinkCabVehicleSnapshot.h"
#include <limits>

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPinkCabTatraMassCoordinateValidationTest,
    "PinkCab.Vehicle.PhysicalFoundation.InvalidMassCoordinates",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabTatraMassCoordinateValidationTest::RunTest(const FString& Parameters)
{
    FPinkCabVehicleHealthState Health;
    FPinkCabVehicleLoadState Load;
    Load.SetFuelMassKg(100.0f);
    Load.SetCrew(58.0f, 49.0f);
    Load.AddPassenger(FPinkCabVehicleLoadItem(80.0f, FVector(-90, -40, 75)));
    FPinkCabVehicleSnapshot Good;
    if (!TestTrue(TEXT("valid reference captures"),
        FPinkCabVehicleSnapshotCodec::Capture(Health, Load, Good))) return false;

    for (int32 Field = 0; Field < 4; ++Field)
    {
        FPinkCabVehicleSnapshot Bad = Good;
        float* Coordinate = Field == 0 ? &Bad.Load.FuelLateralCm
            : Field == 1 ? &Bad.Load.FuelVerticalCm
            : Field == 2 ? &Bad.Load.Passengers[0].LateralCm
            : &Bad.Load.Passengers[0].VerticalCm;
        FPinkCabVehicleHealthState RestoredHealth = Health;
        FPinkCabVehicleLoadState RestoredLoad = Load;
        *Coordinate = std::numeric_limits<float>::quiet_NaN();
        TestFalse(*FString::Printf(TEXT("3D coordinate field %d rejects NaN on restore"), Field),
            FPinkCabVehicleSnapshotCodec::Restore(Bad, RestoredHealth, RestoredLoad));
        *Coordinate = std::numeric_limits<float>::infinity();
        TestFalse(*FString::Printf(TEXT("3D coordinate field %d rejects infinity on restore"), Field),
            FPinkCabVehicleSnapshotCodec::Restore(Bad, RestoredHealth, RestoredLoad));
        TestEqual(TEXT("rejected coordinates leave destination mass unchanged"),
            RestoredLoad.GetTotalMassKg(FPinkCabTatraProfile::Canonical()), 1737.0f);
    }
    return true;
}
#endif
