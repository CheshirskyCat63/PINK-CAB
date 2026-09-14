#pragma once

#include "Persistence/PinkCabGameSnapshot.h"
#include "Persistence/PinkCabGameSnapshotArchivePrimitives.h"
#include "Persistence/PinkCabPassengerSnapshotArchive.h"
#include "Persistence/PinkCabEconomySnapshotArchive.h"
#include "Persistence/PinkCabFareSnapshotArchive.h"
#include "Persistence/PinkCabVehicleSnapshotArchive.h"
#include "Persistence/PinkCabServiceSnapshotArchive.h"
#include "Persistence/PinkCabWorldSnapshotArchive.h"

class FPinkCabGameSnapshotArchive : protected FPinkCabGameSnapshotArchivePrimitives
{
public:
    static bool Serialize(FArchive& Ar, FPinkCabGameSnapshot& Snapshot)
    {
        Ar << Snapshot.SchemaVersion;
        SerializeCityIdentity(Ar, Snapshot.CityIdentity);
        FPinkCabPassengerSnapshotArchive::Serialize(Ar, Snapshot.Passenger);
        FPinkCabEconomySnapshotArchive::Serialize(Ar, Snapshot.Economy);
        FPinkCabFareSnapshotArchive::Serialize(Ar, Snapshot.FareRuntime);
        FPinkCabVehicleSnapshotArchive::Serialize(Ar, Snapshot.Vehicle);
        FPinkCabServiceSnapshotArchive::Serialize(Ar, Snapshot.Service);
        FPinkCabWorldSnapshotArchive::Serialize(Ar, Snapshot.CityDeltas, Snapshot.Kernel, Snapshot.Workday);
        return !Ar.IsError();
    }
};
