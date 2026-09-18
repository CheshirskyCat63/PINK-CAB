#pragma once

#include "Persistence/PinkCabGameSnapshotArchivePrimitives.h"

class FPinkCabVehicleSnapshotArchive : protected FPinkCabGameSnapshotArchivePrimitives
{
public:
    static void Serialize(FArchive& Ar, FPinkCabVehicleSnapshot& Snapshot)
    {
        SerializeVehicleSnapshot(Ar, Snapshot);
    }

private:
    static void SerializeLoadItem(FArchive& Ar, FPinkCabVehicleLoadItemSnapshot& Item)
    {
        Ar << Item.MassKg;
        Ar << Item.LongitudinalCm;
    }

    static void SerializeVehicleSnapshot(FArchive& Ar, FPinkCabVehicleSnapshot& Snapshot)
    {
        Ar << Snapshot.SchemaVersion;
        SerializeArray(Ar, Snapshot.Health.ChannelHealth, 64,
            [](FArchive& A, float& Value) { A << Value; });
        Ar << Snapshot.Health.FunctionalDamageSerial;
        if (Snapshot.SchemaVersion >= 2)
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
        Ar << Snapshot.Load.HeroineMassKg;
        Ar << Snapshot.Load.DaughterMassKg;
        SerializeArray(Ar, Snapshot.Load.Passengers, 64,
            [](FArchive& A, FPinkCabVehicleLoadItemSnapshot& Value) { SerializeLoadItem(A, Value); });
        SerializeArray(Ar, Snapshot.Load.FarePassengers, 5,
            [](FArchive& A, FPinkCabVehicleLoadItemSnapshot& Value) { SerializeLoadItem(A, Value); });
        Ar << Snapshot.Load.FarePassengerGroupId;
        Ar << Snapshot.Load.bFarePassengerGroupActive;
    }
};
