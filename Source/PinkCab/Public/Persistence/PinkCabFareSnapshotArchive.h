#pragma once

#include "Persistence/PinkCabGameSnapshotArchivePrimitives.h"

class FPinkCabFareSnapshotArchive : protected FPinkCabGameSnapshotArchivePrimitives
{
public:
    static void Serialize(FArchive& Ar, FPinkCabFareRuntimeSnapshot& Snapshot)
    {
        SerializeFareRuntimeSnapshot(Ar, Snapshot);
    }

private:
    static void SerializePricing(FArchive& Ar, FPinkCabFarePricingTerms& Terms)
    {
        SerializeEnum(Ar, Terms.Mode);
        Ar << Terms.BaseMinor;
        Ar << Terms.PerKmMinor;
        Ar << Terms.PerMinuteMinor;
        Ar << Terms.AgreedMinor;
    }

    static void SerializeFareSession(FArchive& Ar, FPinkCabFareSessionSnapshot& Session)
    {
        Ar << Session.FareId;
        SerializeEnum(Ar, Session.State);
        Ar << Session.bPaymentCommitted;
        Ar << Session.bReceiptResolved;
        Ar << Session.bReceiptTaken;
        Ar << Session.bStopoverActive;
        Ar << Session.bRequiresWorkdayEnd;
        Ar << Session.bRequiresRepairRecovery;
    }

    static void SerializeTaximeter(FArchive& Ar, FPinkCabTaximeterSnapshot& Meter)
    {
        SerializePricing(Ar, Meter.Terms);
        Ar << Meter.DistanceKm;
        Ar << Meter.FareSeconds;
        Ar << Meter.bStarted;
        Ar << Meter.bRunning;
    }
    static void SerializeManifestRecord(FArchive& Ar, FPinkCabFarePassengerRecordSnapshot& Record)
    {
        Ar << Record.PassengerId;
        SerializeEnum(Ar, Record.Seat);
        Ar << Record.MassKg;
        Ar << Record.LongitudinalCm;
    }

    static void SerializeManifest(FArchive& Ar, FPinkCabFarePassengerManifestSnapshot& Manifest)
    {
        Ar << Manifest.FareId;
        SerializeArray(Ar, Manifest.Records, 5,
            [](FArchive& A, FPinkCabFarePassengerRecordSnapshot& Value)
            { SerializeManifestRecord(A, Value); });
        Ar << Manifest.TotalPassengerMassKg;
        Ar << Manifest.bBoarded;
        Ar << Manifest.bExited;
    }

    static void SerializeFareRuntimeSnapshot(FArchive& Ar, FPinkCabFareRuntimeSnapshot& Snapshot)
    {
        Ar << Snapshot.SchemaVersion;
        Ar << Snapshot.FareId;
        SerializeEnum(Ar, Snapshot.LoopState);
        Ar << Snapshot.bInitialized;
        SerializeFareSession(Ar, Snapshot.Session);
        SerializeTaximeter(Ar, Snapshot.Taximeter);
        SerializeManifest(Ar, Snapshot.Manifest);
    }
};
