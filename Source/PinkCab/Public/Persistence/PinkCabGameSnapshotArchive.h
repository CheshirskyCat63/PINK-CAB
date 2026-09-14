#pragma once

#include "CoreMinimal.h"
#include "Persistence/PinkCabGameSnapshot.h"

class FPinkCabGameSnapshotArchive
{
public:
    static bool Serialize(FArchive& Ar, FPinkCabGameSnapshot& Snapshot)
    {
        Ar << Snapshot.SchemaVersion;
        SerializeCityIdentity(Ar, Snapshot.CityIdentity);
        SerializePassengerSnapshot(Ar, Snapshot.Passenger);
        SerializeEconomySnapshot(Ar, Snapshot.Economy);
        SerializeFareRuntimeSnapshot(Ar, Snapshot.FareRuntime);
        SerializeVehicleSnapshot(Ar, Snapshot.Vehicle);
        SerializeServiceSnapshot(Ar, Snapshot.Service);
        SerializeWorldSnapshots(Ar, Snapshot.CityDeltas, Snapshot.Kernel, Snapshot.Workday);
        return !Ar.IsError();
    }

private:
    static constexpr int32 MaxArrayElements = 65536;

    template <typename TEnum>
    static void SerializeEnum(FArchive& Ar, TEnum& Value)
    {
        uint8 Raw = static_cast<uint8>(Value);
        Ar << Raw;
        if (Ar.IsLoading()) Value = static_cast<TEnum>(Raw);
    }

    template <typename TItem, typename TSerializer>
    static void SerializeArray(
        FArchive& Ar, TArray<TItem>& Items, int32 MaxCount, TSerializer Serializer)
    {
        int32 Count = Items.Num();
        Ar << Count;
        if (Count < 0 || Count > MaxCount)
        {
            Ar.SetError();
            return;
        }
        if (Ar.IsLoading()) Items.SetNum(Count);
        for (int32 Index = 0; Index < Count && !Ar.IsError(); ++Index)
        {
            Serializer(Ar, Items[Index]);
        }
    }

    static bool IsCapacityReasonable(int32 Value)
    {
        return Value >= 0 && Value <= MaxArrayElements;
    }
    static void SerializeStableId(FArchive& Ar, FPinkCabStableId& Id)
    {
        FString Value = Id.Serialize();
        Ar << Value;
        if (Ar.IsLoading()) Id = FPinkCabStableId(Value);
    }

    static void SerializeCityIdentity(FArchive& Ar, FPinkCabCityIdentity& City)
    {
        FString CityCode = City.GetCityCode();
        FString Generator = City.GetGeneratorVersion();
        FString Content = City.GetContentSetVersion();
        Ar << CityCode;
        Ar << Generator;
        Ar << Content;
        if (Ar.IsLoading())
        {
            City = FPinkCabCityIdentity::Create(CityCode, Generator, Content);
        }
    }

    static void SerializeRelationship(FArchive& Ar, FPinkCabPassengerRelationship& Relationship)
    {
        Ar << Relationship.Trust;
        Ar << Relationship.Satisfaction;
        Ar << Relationship.RiskTolerance;
    }
    static void SerializeRideMemory(FArchive& Ar, FPinkCabPassengerRideMemory& Memory)
    {
        SerializeStableId(Ar, Memory.MemoryId);
        SerializeStableId(Ar, Memory.FareId);
        Ar << Memory.OutcomeTag;
    }

    static void SerializeNeuralMessage(FArchive& Ar, FPinkCabPassengerNeuralMessage& Message)
    {
        SerializeStableId(Ar, Message.MessageId);
        Ar << Message.Text;
    }

    static void SerializePassengerRecord(FArchive& Ar, FPinkCabPassengerRecord& Record)
    {
        SerializeStableId(Ar, Record.IdentityId);
        Ar << Record.TemplateId;
        Ar << Record.ContextKey;
        Ar << Record.IdentitySeed;
        Ar << Record.AppearanceSeed;
        Ar << Record.AppearanceProfileId;
        SerializeArray(Ar, Record.AppearanceTraitIds, 64,
            [](FArchive& A, FName& Value) { A << Value; });
        Ar << Record.ResolvedMassKg;
        SerializeArray(Ar, Record.PreferenceTags, 256,
            [](FArchive& A, FName& Value) { A << Value; });
        SerializeRelationship(Ar, Record.Relationship);
        Ar << Record.MaxRideMemories;
        Ar << Record.MaxReplayJournalEntries;
        Ar << Record.PaidFareCount;
        Ar << Record.AuthoredEventCount;
        Ar << Record.bRepeatEligible;
        SerializeArray(Ar, Record.RideMemories, MaxArrayElements,
            [](FArchive& A, FPinkCabPassengerRideMemory& Value) { SerializeRideMemory(A, Value); });
        SerializeArray(Ar, Record.AppliedSocialEventIds, MaxArrayElements,
            [](FArchive& A, FString& Value) { A << Value; });
        Ar << Record.MaxNeuralMessages;
        Ar << Record.MaxNeuralReplayJournalEntries;
        Ar << Record.bNeuralPermissionGranted;
        Ar << Record.bNeuralBlocked;
        SerializeArray(Ar, Record.NeuralMessages, MaxArrayElements,
            [](FArchive& A, FPinkCabPassengerNeuralMessage& Value) { SerializeNeuralMessage(A, Value); });
        SerializeArray(Ar, Record.AppliedNeuralMessageIds, MaxArrayElements,
            [](FArchive& A, FString& Value) { A << Value; });
        if (!IsCapacityReasonable(Record.MaxRideMemories)
            || !IsCapacityReasonable(Record.MaxReplayJournalEntries)
            || !IsCapacityReasonable(Record.MaxNeuralMessages)
            || !IsCapacityReasonable(Record.MaxNeuralReplayJournalEntries))
        {
            Ar.SetError();
        }
    }
    static void SerializePassengerSnapshot(FArchive& Ar, FPinkCabPassengerSnapshot& Snapshot)
    {
        Ar << Snapshot.SchemaVersion;
        Ar << Snapshot.MaxRecords;
        Ar << Snapshot.MaxPreferences;
        Ar << Snapshot.MaxRideMemories;
        Ar << Snapshot.MaxReplayJournalEntries;
        Ar << Snapshot.MaxNeuralMessages;
        Ar << Snapshot.MaxNeuralReplayJournalEntries;
        if (!IsCapacityReasonable(Snapshot.MaxRecords)
            || !IsCapacityReasonable(Snapshot.MaxPreferences)
            || !IsCapacityReasonable(Snapshot.MaxRideMemories)
            || !IsCapacityReasonable(Snapshot.MaxReplayJournalEntries)
            || !IsCapacityReasonable(Snapshot.MaxNeuralMessages)
            || !IsCapacityReasonable(Snapshot.MaxNeuralReplayJournalEntries))
        {
            Ar.SetError();
            return;
        }
        SerializeArray(Ar, Snapshot.Records, Snapshot.MaxRecords,
            [](FArchive& A, FPinkCabPassengerRecord& Value) { SerializePassengerRecord(A, Value); });
    }

    static void SerializeSettlementRecord(FArchive& Ar, FPinkCabFareSettlementSnapshotRecord& Record)
    {
        Ar << Record.FareId;
        SerializeEnum(Ar, Record.Kind);
        SerializeEnum(Ar, Record.FareResult);
        SerializeEnum(Ar, Record.TipResult);
        Ar << Record.ConsequenceId;
    }

    static void SerializeEconomySnapshot(FArchive& Ar, FPinkCabEconomySnapshot& Snapshot)
    {
        Ar << Snapshot.SchemaVersion;
        Ar << Snapshot.BalanceMinor;
        Ar << Snapshot.DebtLimitMinor;
        Ar << Snapshot.MaxCommittedTransactionIds;
        Ar << Snapshot.MaxResolvedFares;
        if (!IsCapacityReasonable(Snapshot.MaxCommittedTransactionIds)
            || !IsCapacityReasonable(Snapshot.MaxResolvedFares))
        {
            Ar.SetError();
            return;
        }
        SerializeArray(Ar, Snapshot.CommittedTransactionIds, Snapshot.MaxCommittedTransactionIds,
            [](FArchive& A, FString& Value) { A << Value; });
        SerializeArray(Ar, Snapshot.FareSettlements, Snapshot.MaxResolvedFares,
            [](FArchive& A, FPinkCabFareSettlementSnapshotRecord& Value)
            { SerializeSettlementRecord(A, Value); });
    }

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

    static void SerializeOwnerIdentity(FArchive& Ar, FPinkCabServiceOwnerIdentity& Owners)
    {
        SerializeStableId(Ar, Owners.VehicleId);
        SerializeStableId(Ar, Owners.BuildOwnerId);
        SerializeStableId(Ar, Owners.HealthOwnerId);
        SerializeStableId(Ar, Owners.InventoryOwnerId);
        SerializeStableId(Ar, Owners.EconomyOwnerId);
    }

    static void SerializeServiceContext(FArchive& Ar, FPinkCabServiceContext& Context)
    {
        SerializeCityIdentity(Ar, Context.City);
        Ar << Context.SemanticKey;
        Ar << Context.ServiceNodeId;
        SerializeOwnerIdentity(Ar, Context.Owners);
    }

    static void SerializeOwnedPart(FArchive& Ar, FPinkCabOwnedPartSnapshot& Part)
    {
        Ar << Part.PartId;
        Ar << Part.Quantity;
    }

    static void SerializeInstalledPart(FArchive& Ar, FPinkCabInstalledPartSnapshot& Part)
    {
        Ar << Part.SlotId;
        Ar << Part.PartId;
    }
    static void SerializeMovingFuelPolicy(FArchive& Ar, FPinkCabMovingFuelPolicyInputs& Policy)
    {
        Ar << Policy.QueuePolicyId;
        Ar << Policy.SettlementPolicyId;
        Ar << Policy.InsufficientFundsPolicyId;
        Ar << Policy.TargetLongitudinalGapCm;
        Ar << Policy.GapToleranceCm;
        Ar << Policy.MaxConnectionSpeedKmh;
    }

    static void SerializeServiceSnapshot(FArchive& Ar, FPinkCabServiceSnapshot& Snapshot)
    {
        Ar << Snapshot.SchemaVersion;
        SerializeServiceContext(Ar, Snapshot.Context);
        Ar << Snapshot.Inventory.MaxItems;
        Ar << Snapshot.Inventory.MaxReplayJournalEntries;
        SerializeArray(Ar, Snapshot.Inventory.Parts, MaxArrayElements,
            [](FArchive& A, FPinkCabOwnedPartSnapshot& Value) { SerializeOwnedPart(A, Value); });
        SerializeArray(Ar, Snapshot.Inventory.AppliedOperationIds, MaxArrayElements,
            [](FArchive& A, FString& Value) { A << Value; });
        Ar << Snapshot.Build.SchemaVersion;
        Ar << Snapshot.Build.MaxReplayJournalEntries;
        SerializeArray(Ar, Snapshot.Build.InstalledParts, MaxArrayElements,
            [](FArchive& A, FPinkCabInstalledPartSnapshot& Value) { SerializeInstalledPart(A, Value); });
        SerializeArray(Ar, Snapshot.Build.AppliedInstallOperationIds, MaxArrayElements,
            [](FArchive& A, FString& Value) { A << Value; });
        Ar << Snapshot.Operations.MaxReplayJournalEntries;
        SerializeArray(Ar, Snapshot.Operations.CompletedOperationIds, MaxArrayElements,
            [](FArchive& A, FString& Value) { A << Value; });
        Ar << Snapshot.FuelTank.CapacityLiters;
        Ar << Snapshot.FuelTank.CurrentLiters;
        Ar << Snapshot.FuelTank.MaxReplayJournalEntries;
        SerializeArray(Ar, Snapshot.FuelTank.CreditedTransactionIds, MaxArrayElements,
            [](FArchive& A, FString& Value) { A << Value; });
        SerializeMovingFuelPolicy(Ar, Snapshot.MovingFuel.Policy);
        SerializeEnum(Ar, Snapshot.MovingFuel.State);
        SerializeEnum(Ar, Snapshot.MovingFuel.AbortReason);
        Ar << Snapshot.MovingFuel.ServiceLaneId;

        if (!IsCapacityReasonable(Snapshot.Inventory.MaxItems)
            || !IsCapacityReasonable(Snapshot.Inventory.MaxReplayJournalEntries)
            || !IsCapacityReasonable(Snapshot.Build.MaxReplayJournalEntries)
            || !IsCapacityReasonable(Snapshot.Operations.MaxReplayJournalEntries)
            || !IsCapacityReasonable(Snapshot.FuelTank.MaxReplayJournalEntries))
        {
            Ar.SetError();
        }
    }
    static void SerializeCityDeltaRecord(FArchive& Ar, FPinkCabCityDeltaSnapshotRecord& Record)
    {
        Ar << Record.DeltaId;
        SerializeEnum(Ar, Record.Kind);
        Ar << Record.SubjectStableId;
        Ar << Record.OperationKey;
    }

    static void SerializeWorldSnapshots(
        FArchive& Ar,
        FPinkCabCityDeltaSnapshot& CityDeltas,
        FPinkCabStateKernelSnapshot& Kernel,
        FPinkCabWorkdaySessionSnapshot& Workday)
    {
        Ar << CityDeltas.SchemaVersion;
        Ar << CityDeltas.MaxDeltas;
        if (!IsCapacityReasonable(CityDeltas.MaxDeltas))
        {
            Ar.SetError();
            return;
        }
        SerializeArray(Ar, CityDeltas.Records, CityDeltas.MaxDeltas,
            [](FArchive& A, FPinkCabCityDeltaSnapshotRecord& Value)
            { SerializeCityDeltaRecord(A, Value); });
        Ar << Kernel.SchemaVersion;
        Ar << Kernel.RootSeed;
        Ar << Kernel.Sequence;
        Ar << Workday.SchemaVersion;
        Ar << Workday.WorkdayId;
        Ar << Workday.Ordinal;
        Ar << Workday.ElapsedGameSeconds;
        Ar << Workday.MaxReplayJournalEntries;
        Ar << Workday.bSummaryCommitted;
        Ar << Workday.HouseholdTransactionCount;
        Ar << Workday.bTerminalRecovery;
        if (!IsCapacityReasonable(Workday.MaxReplayJournalEntries))
        {
            Ar.SetError();
            return;
        }
        SerializeArray(Ar, Workday.AppliedHouseholdOperationIds, Workday.MaxReplayJournalEntries,
            [](FArchive& A, FString& Value) { A << Value; });
    }
};
