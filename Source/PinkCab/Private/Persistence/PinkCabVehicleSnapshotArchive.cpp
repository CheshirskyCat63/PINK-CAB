#include "Persistence/PinkCabVehicleSnapshotArchive.h"

void FPinkCabVehicleSnapshotArchive::Serialize(
    FArchive& Ar,
    FPinkCabVehicleSnapshot& Snapshot)
{
    SerializeVehicleSnapshot(Ar, Snapshot);
}

void FPinkCabVehicleSnapshotArchive::SerializeLoadItem(
    FArchive& Ar,
    FPinkCabVehicleLoadItemSnapshot& Item, bool bCoordinates3D)
{
    Ar << Item.MassKg;
    Ar << Item.LongitudinalCm;
    if (bCoordinates3D)
    {
        Ar << Item.LateralCm;
        Ar << Item.VerticalCm;
    }
    else if (Ar.IsLoading())
    {
        Item.LateralCm = 0.0f;
        Item.VerticalCm = FPinkCabTatraProfile::DefaultPassengerHeightCm;
    }
}

void FPinkCabVehicleSnapshotArchive::SerializeVehicleSnapshot(
    FArchive& Ar,
    FPinkCabVehicleSnapshot& Snapshot)
{
    Ar << Snapshot.SchemaVersion;
    const bool bCoordinates3D = Snapshot.SchemaVersion >= FPinkCabVehicleSnapshot::CurrentSchemaVersion;
    SerializeArray(
        Ar,
        Snapshot.Health.ChannelHealth,
        64,
        [](FArchive& A, float& Value)
        {
            A << Value;
        });
    Ar << Snapshot.Health.FunctionalDamageSerial;

    bool bSerializeThermals =
        Snapshot.SchemaVersion
        >= FPinkCabVehicleSnapshot::LongitudinalOnlySchemaVersion;
    if (Snapshot.SchemaVersion
        == FPinkCabVehicleSnapshot::DivergentSchemaVersion)
    {
        bSerializeThermals =
            Snapshot.Health.ChannelHealth.Num()
            == FPinkCabVehicleSnapshot::RuntimeSchema2HealthChannelCount;
    }

    if (bSerializeThermals)
    {
        Ar << Snapshot.Health.ClutchTemperature01;
        Ar << Snapshot.Health.BrakeTemperature01;
    }
    else if (Ar.IsLoading())
    {
        Snapshot.Health.ClutchTemperature01 = 0.0f;
        Snapshot.Health.BrakeTemperature01 = 0.0f;
    }

    Ar << Snapshot.Load.FuelMassKg;
    Ar << Snapshot.Load.FuelLongitudinalCm;
    if (bCoordinates3D)
    {
        Ar << Snapshot.Load.FuelLateralCm;
        Ar << Snapshot.Load.FuelVerticalCm;
    }
    else if (Ar.IsLoading())
    {
        Snapshot.Load.FuelLateralCm = 0.0f;
        Snapshot.Load.FuelVerticalCm = FPinkCabTatraProfile::DefaultFuelHeightCm;
    }
    Ar << Snapshot.Load.HeroineMassKg;
    Ar << Snapshot.Load.DaughterMassKg;
    SerializeArray(
        Ar,
        Snapshot.Load.Passengers,
        64,
        [bCoordinates3D](FArchive& A, FPinkCabVehicleLoadItemSnapshot& Value)
        {
            SerializeLoadItem(A, Value, bCoordinates3D);
        });
    SerializeArray(
        Ar,
        Snapshot.Load.FarePassengers,
        5,
        [bCoordinates3D](FArchive& A, FPinkCabVehicleLoadItemSnapshot& Value)
        {
            SerializeLoadItem(A, Value, bCoordinates3D);
        });
    Ar << Snapshot.Load.FarePassengerGroupId;
    Ar << Snapshot.Load.bFarePassengerGroupActive;
}
