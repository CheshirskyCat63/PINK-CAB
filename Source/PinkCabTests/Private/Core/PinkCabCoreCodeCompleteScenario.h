#pragma once

#include "CoreMinimal.h"
#include "Core/PinkCabDeterministicSeed.h"
#include "Core/PinkCabStateKernel.h"
#include "Economy/PinkCabFareSettlementService.h"
#include "Enforcement/PinkCabEnforcementService.h"
#include "Persistence/PinkCabPersistenceService.h"
#include "Persistence/PinkCabRecoveryOrchestrator.h"
#include "Service/PinkCabServiceNode.h"
#include "Taxi/PinkCabFareLoopCoordinator.h"
#include "Taxi/PinkCabPassengerHistory.h"
#include "Vehicle/PinkCabL1TraversalState.h"
#include "World/PinkCabMetroTransitRuntime.h"
#include "World/PinkCabSuspendedBusRuntime.h"

struct FCoreCodeCompleteResult
{
    bool bTwoSuccessfulFares = false;
    bool bMeteredPath = false;
    bool bOffMeterPath = false;
    bool bPaymentBeforeExit = false;
    bool bRepeatEligible = false;
    bool bRepeatOrderOrdinary = false;
    bool bDeclineClean = false;
    bool bEvasionCommitted = false;
    bool bEvasionPaidConflict = false;
    bool bNeuralContinuity = false;
    bool bFineCommittedOnce = false;
    bool bServiceApplied = false;
    bool bMovingFuelApplied = false;
    bool bL1Completed = false;
    bool bL2BusWindow = false;
    bool bL2MetroRoundTrip = false;
    bool bSnapshotRestored = false;
    bool bReplayNoOps = false;
    bool bTerminalRecovery = false;
    bool bDamagePreserved = false;
    bool bWorkdayResetScoped = false;
    uint64 IntegratedSignature = 0;
    FString FineTransactionId;
    FString EvasionConsequenceId;
    FString PassengerIdentityId;
};

namespace PinkCabCoreCodeCompleteScenario
{
inline FPinkCabCityIdentity City()
{
    return FPinkCabCityIdentity::Create(TEXT("FIRST-EURO"), TEXT("gen-1"), TEXT("content-1"));
}
inline FPinkCabSaveHeader SaveHeader()
{
    return FPinkCabSaveHeader::Create(
        TEXT("PINK-CAB"), TEXT("core-1"), TEXT("cfg-1"), TEXT("gen-1"), TEXT("content-1"));
}

inline FPinkCabMovingFuelPolicyInputs FuelPolicy()
{
    FPinkCabMovingFuelPolicyInputs Policy;
    Policy.QueuePolicyId = TEXT("queue.code-complete");
    Policy.SettlementPolicyId = TEXT("settlement.code-complete");
    Policy.InsufficientFundsPolicyId = TEXT("funds.code-complete");
    Policy.TargetLongitudinalGapCm = 40.0f;
    Policy.GapToleranceCm = 10.0f;
    Policy.MaxConnectionSpeedKmh = 20.0f;
    return Policy;
}

inline FPinkCabPassengerRelationship RelationshipDelta()
{
    FPinkCabPassengerRelationship Delta;
    Delta.Trust = 0.2f;
    Delta.Satisfaction = 0.15f;
    Delta.RiskTolerance = 0.05f;
    return Delta;
}
inline FPinkCabServiceOwnerIdentity ServiceOwners()
{
    FPinkCabServiceOwnerIdentity Owners;
    Owners.VehicleId = FPinkCabStableId(TEXT("vehicle:code-complete"));
    Owners.BuildOwnerId = FPinkCabStableId(TEXT("build:code-complete"));
    Owners.HealthOwnerId = FPinkCabStableId(TEXT("health:code-complete"));
    Owners.InventoryOwnerId = FPinkCabStableId(TEXT("inventory:code-complete"));
    Owners.EconomyOwnerId = FPinkCabStableId(TEXT("economy:code-complete"));
    return Owners;
}

inline bool BuildRoadGraph(FPinkCabRoadGraph& OutGraph, FPinkCabLaneId& OutLane)
{
    OutLane = FPinkCabLaneId(TEXT("lane:code-complete"));
    FPinkCabLogicalLane Lane;
    Lane.LaneId = OutLane;
    Lane.FromNode = FPinkCabRoadNodeId(TEXT("node:code-complete:a"));
    Lane.ToNode = FPinkCabRoadNodeId(TEXT("node:code-complete:b"));
    Lane.LengthCm = 1000.0;
    Lane.Layer = 0;
    return OutGraph.AddLane(Lane);
}

inline FPinkCabL1TraversalInput Contact(
    EPinkCabVerticalContactKind Kind, const TCHAR* ContactId, float MassKg)
{
    FPinkCabL1TraversalInput Input;
    Input.bHasContact = true;
    Input.ContactKind = Kind;
    Input.ContactId = ContactId;
    Input.TotalMassKg = MassKg;
    Input.DeltaSeconds = 0.25f;
    return Input;
}

inline FPinkCabOrder MakeOrder(
    const TCHAR* FareId, const FPinkCabStableId& PassengerId, EPinkCabFareMode Mode)
{
    FPinkCabOrder Order;
    Order.OrderId = FPinkCabStableId(FareId);
    Order.PickupId = FPinkCabStableId(FString(FareId) + TEXT(":pickup"));
    Order.DestinationId = FPinkCabStableId(FString(FareId) + TEXT(":destination"));
    Order.PassengerIdentityIds.Add(PassengerId);
    Order.FareMode = Mode;
    return Order;
}

inline bool BuildFare(
    const TCHAR* FareId, const FPinkCabStableId& PassengerId, EPinkCabFareMode Mode,
    FPinkCabFareLoopCoordinator& OutFare)
{
    const FPinkCabOrder Order = MakeOrder(FareId, PassengerId, Mode);
    TArray<FPinkCabFarePassengerInput> Inputs = {{PassengerId, 72.0f}};
    FPinkCabFarePassengerManifest Manifest;
    if (!FPinkCabFarePassengerManifest::TryCreate(Order.OrderId, Inputs, Manifest)) return false;
    const FPinkCabFarePricingTerms Pricing = Mode == EPinkCabFareMode::Metered
        ? FPinkCabFarePricingTerms::Metered(100, 100, 10)
        : FPinkCabFarePricingTerms::OffMeter(650);
    return FPinkCabFareLoopCoordinator::TryCreate(
        Order.OrderId, Order, Pricing, Manifest, OutFare);
}

struct FFixture
{
    explicit FFixture(uint64 RootSeed)
        : Kernel(RootSeed), MovingFuel(FuelPolicy()) {}

    FPinkCabPassengerRegistry Passengers{8, 8, 8, 64, 8, 64};
    FPinkCabEconomyLedger Economy{20000, 5000, 128};
    FPinkCabFareSettlementService Settlement{64};
    FPinkCabFareLoopCoordinator Fare;
    FPinkCabVehicleHealthState Health;
    FPinkCabVehicleLoadState Load;
    FPinkCabServiceContext Service;
    FPinkCabServiceInventory Inventory{8, 32};
    FPinkCabVehicleBuild Build{32};
    FPinkCabServiceOperationRuntime Operations{64};
    FPinkCabFuelTank Fuel{60.0f, 10.0f, 64};
    FPinkCabStateKernel Kernel;
    FPinkCabMovingFuelSession MovingFuel;
    FPinkCabCityDeltaState CityDeltas{64};
    FPinkCabWorkdaySessionState Workday;

    FPinkCabGamePersistenceOwners Owners()
    {
        return FPinkCabGamePersistenceOwners{
            Passengers, Economy, Settlement, Fare, Health, Load, Service,
            Inventory, Build, Operations, Fuel, MovingFuel, CityDeltas, Kernel, Workday};
    }
};

inline bool CompletePaidFare(
    FFixture& F, const TCHAR* FareId, const FPinkCabStableId& PassengerId,
    bool& OutExitBlockedBeforePayment, int64& OutFareMinor)
{
    if (!BuildFare(FareId, PassengerId, EPinkCabFareMode::Metered, F.Fare)) return false;
    if (!F.Fare.TryAcceptPickup(true, true)) return false;
    if (!F.Fare.TryBoard(true, true, F.Load)) return false;
    if (!F.Fare.TryStartRide(true)) return false;
    F.Fare.TickFare(1.25, 75.0, false);
    if (!F.Fare.TryArrive(true, true)) return false;
    OutFareMinor = F.Fare.GetFareMinor();
    OutExitBlockedBeforePayment = !F.Fare.TryExit(true, true, F.Load);
    if (!OutExitBlockedBeforePayment) return false;
    if (!F.Fare.TryPay(25, F.Economy, F.Settlement)) return false;
    if (!F.Fare.TryExit(true, true, F.Load)) return false;
    return F.Fare.GetState() == EPinkCabFareLoopState::Complete
        && !F.Load.HasFarePassengerGroup(FPinkCabStableId(FareId));
}

inline bool RunOffMeterEvasion(
    FFixture& F, const FPinkCabStableId& PassengerId,
    FString& OutConsequenceId)
{
    const TCHAR* FareId = TEXT("fare:code-complete:evasion");
    if (!BuildFare(FareId, PassengerId, EPinkCabFareMode::OffMeter, F.Fare)) return false;
    if (!F.Fare.TryAcceptPickup(true, true)) return false;
    if (!F.Fare.TryBoard(true, true, F.Load)) return false;
    if (!F.Fare.TryStartRide(true)) return false;
    F.Fare.TickFare(0.4, 20.0, false);
    if (!F.Fare.TryArrive(true, true)) return false;
    if (!F.Fare.TryEvade(true, F.Settlement)) return false;
    const FPinkCabFareSettlementResult Replay = F.Settlement.RecordEvasion(FPinkCabStableId(FareId));
    if (Replay.Disposition != EPinkCabFareSettlementDisposition::Replayed) return false;
    OutConsequenceId = Replay.ConsequenceId.Serialize();
    const bool bPaidConflict = F.Settlement.CommitPaid(
        FPinkCabStableId(FareId), 650, 0, F.Economy).Disposition
        == EPinkCabFareSettlementDisposition::Conflict;
    if (!bPaidConflict) return false;
    if (!F.Fare.TryExit(true, true, F.Load)) return false;
    return F.Fare.GetState() == EPinkCabFareLoopState::Complete;
}

inline bool RunEnforcement(
    FFixture& F, const FPinkCabLaneId& LaneId,
    FPinkCabEnforcementLedger& Reputation,
    FString& OutFineTransactionId)
{
    FPinkCabRoadRuleProfile Rules;
    FPinkCabRoadRuleDefinition Rule;
    Rule.Kind = EPinkCabRoadRuleKind::SpeedLimit;
    Rule.TypeId = FName(TEXT("CodeCompleteSpeed"));
    Rule.SpeedLimitCmPerSec = 100.0;
    Rule.FineAmountMinor = 700;
    Rule.ReputationDelta = -3;
    Rule.BaseSeverity = 0.5f;
    if (!Rules.TrySetRule(LaneId, Rule)) return false;

    FPinkCabRoadObservation Observation;
    Observation.ObservationKey = TEXT("code-complete-speed");
    Observation.LaneId = LaneId;
    Observation.CityLocationId = FPinkCabStableId(TEXT("location:code-complete:rule"));
    Observation.VehicleId = FPinkCabStableId(TEXT("vehicle:code-complete"));
    Observation.TimestampMs = 802;
    Observation.SpeedCmPerSec = 150.0;
    TArray<FPinkCabEnforcementEvent> Events;
    if (!FPinkCabEnforcementService::Evaluate(Rules, Observation, Events)
        || Events.Num() != 1) return false;

    FPinkCabEnforcementSettlement Fine;
    if (FPinkCabEnforcementService::Settle(
            Rules, Events[0], F.Economy, Reputation, Fine)
        != EPinkCabEnforcementSettlementResult::Committed)
    {
        return false;
    }
    OutFineTransactionId = Fine.TransactionId.GetValue();
    const int64 BalanceAfterFine = F.Economy.GetBalanceMinor();
    FPinkCabEnforcementSettlement Replay;
    if (FPinkCabEnforcementService::Settle(
            Rules, Events[0], F.Economy, Reputation, Replay)
        != EPinkCabEnforcementSettlementResult::Duplicate)
    {
        return false;
    }
    return F.Economy.GetBalanceMinor() == BalanceAfterFine
        && Reputation.GetTotalEventCount() == 1;
}

inline bool RunServiceAndFuel(
    FFixture& F, const FPinkCabRoadGraph& Graph, const FPinkCabLaneId& LaneId,
    FPinkCabStableId& OutServiceOperationId, FPinkCabTransactionId& OutFuelTx)
{    FPinkCabServiceNode Node(EPinkCabServiceNodeKind::GarageTuning);
    const FPinkCabServiceOwnerIdentity Owners = ServiceOwners();
    if (!Node.BindContext(City(), TEXT("garage:code-complete"), Owners)
        || !Node.MarkEligible() || !Node.Enter() || !Node.Activate())
    {
        return false;
    }

    OutServiceOperationId = FPinkCabStableId(TEXT("op:code-complete:parking"));
    if (F.Operations.ChargeParking(F.Economy, OutServiceOperationId, 500)
        != EPinkCabServiceOperationResult::Applied)
    {
        return false;
    }
    if (!Node.BeginCommit() || !Node.Exit() || !Node.ResumeWorld()) return false;
    F.Service = Node.GetContext();

    FPinkCabVehicleTelemetry Telemetry;
    Telemetry.SpeedKmh = 10.0f;
    if (!F.MovingFuel.TryConnect(Graph, Telemetry, 40.0f, LaneId)
        || !F.MovingFuel.BeginFueling())
    {
        return false;
    }
    OutFuelTx = FPinkCabTransactionId(TEXT("tx:code-complete:fuel"));
    const FPinkCabEconomyTransaction FuelPurchase = FPinkCabEconomyTransaction::Debit(
        OutFuelTx, EPinkCabTransactionType::FuelPurchase, 600,
        EPinkCabEconomyPurpose::OrdinaryPurchase);
    const EPinkCabSettlementResult Settlement = F.Economy.Commit(FuelPurchase);
    if (Settlement != EPinkCabSettlementResult::Committed) return false;
    if (F.MovingFuel.ApplySettledFuel(F.Fuel, OutFuelTx, 6.0f, Settlement)
        != EPinkCabFuelCreditResult::Applied)
    {
        return false;
    }
    return F.MovingFuel.Complete()
        && F.MovingFuel.GetState() == EPinkCabMovingFuelState::Completed
        && FMath::IsNearlyEqual(F.Fuel.GetLiters(), 16.0f);
}

inline bool RunVertical(bool& OutL1, bool& OutBus, bool& OutMetro)
{
    FPinkCabL1TraversalState L1;
    const float Mass = 1657.0f;
    if (L1.Step(Contact(EPinkCabVerticalContactKind::WallLeft,
            TEXT("contact:code-complete:wall"), Mass)).Phase
        != EPinkCabL1TraversalPhase::WallContact)
    {
        return false;
    }
    const FPinkCabL1TraversalResult L1Done = L1.Step(Contact(
        EPinkCabVerticalContactKind::ReceivingStrip,
        TEXT("contact:code-complete:receive"), Mass));
    OutL1 = L1Done.bCompleted;

    FPinkCabSuspendedBusRuntime Bus(3, 2);
    if (!Bus.TryAddSegment(TEXT("bus:cc:0"), 100.0, false)
        || !Bus.TryAddSegment(TEXT("bus:cc:1"), 200.0, true)
        || !Bus.TryAddObstacle(TEXT("bus:cc:obstacle"), TEXT("bus:cc:1"), 25.0, 75.0)
        || !Bus.Start(TEXT("route:cc:bus"), 50.0)
        || !Bus.Advance(2.5)) return false;
    OutBus = Bus.GetCurrentSegmentId() == TEXT("bus:cc:1")
        && Bus.IsTatraContactEligible()
        && Bus.GetPhase() == EPinkCabSuspendedBusPhase::ObstacleWindow;

    FPinkCabMetroTransitRuntime Metro(3, 2);
    FString A, B, Segment;
    if (!Metro.TryAddStation(City(), TEXT("cc-a"), A)
        || !Metro.TryAddStation(City(), TEXT("cc-b"), B)
        || !Metro.TryAddSegment(City(), A, B, TEXT("cc-ab"), 10.0, 2.0, 4.0, Segment)
        || !Metro.Start(TEXT("route:cc:metro"), Segment)
        || !Metro.Advance(2.5)
        || !Metro.RefreshTatraTransitionState()
        || !Metro.EnterTransitContact()
        || !Metro.IsMetroDriveableForTatra()
        || !Metro.RequestExit()
        || !Metro.CompleteExit())
    {
        return false;
    }
    OutMetro = Metro.GetTatraState() == EPinkCabMetroTatraState::Road
        && !Metro.IsMetroDriveableForTatra();
    return OutL1 && OutBus && OutMetro;
}

inline uint64 BuildSignature(
    const FFixture& F, const FCoreCodeCompleteResult& R)
{
    const FString Payload = FString::Printf(
        TEXT("%lld|%llu|%s|%s|%s|%s|%d|%d"),
        static_cast<long long>(F.Economy.GetBalanceMinor()),
        F.Passengers.GetReconstructionSignature(),
        *R.FineTransactionId, *R.EvasionConsequenceId,
        *R.PassengerIdentityId, *F.Workday.GetWorkdayId().Serialize(),
        F.Workday.GetOrdinal(), static_cast<int32>(F.Fare.GetState()));
    return FPinkCabDeterministicSeed::FromText(Payload);
}
inline bool RunCanonicalScenarioImpl(uint64 RootSeed, FCoreCodeCompleteResult& Out)
{
    FFixture F(RootSeed);
    FCoreCodeCompleteResult R;
    const FPinkCabStableId PassengerId(TEXT("passenger:code-complete:repeat"));
    R.PassengerIdentityId = PassengerId.Serialize();
    if (!FPinkCabWorkdaySessionState::TryCreate(
            FPinkCabStableId(TEXT("workday:code-complete:1")), 1, 120.0, 16, F.Workday))
    {
        return false;
    }

    FPinkCabPassengerRecord* Passenger = nullptr;
    if (!F.Passengers.TryCreate(
            PassengerId, FPinkCabPassengerTemplate(FName(TEXT("Commuter")), 72.0f),
            TEXT("city:first-euro:code-complete"), {FName(TEXT("quiet"))}, Passenger)
        || !Passenger)
    {
        return false;
    }
    const uint64 AppearanceSeed = Passenger->AppearanceSeed;

    bool FirstExitBlocked = false;
    int64 FirstFareMinor = 0;
    if (!CompletePaidFare(F, TEXT("fare:code-complete:1"), PassengerId,
            FirstExitBlocked, FirstFareMinor)) return false;
    R.bPaymentBeforeExit = FirstExitBlocked;
    R.bMeteredPath = FirstFareMinor > 0;
    if (Passenger->RegisterPaidFareOnce(
            FPinkCabStableId(TEXT("social:code-complete:1")), RelationshipDelta())
        != EPinkCabPassengerMutationResult::Applied) return false;
    if (Passenger->AddRideMemoryOnce(
            FPinkCabStableId(TEXT("memory:code-complete:1")),
            FPinkCabStableId(TEXT("fare:code-complete:1")), FName(TEXT("paid")))
        != EPinkCabPassengerMutationResult::Applied) return false;

    Passenger = nullptr;
    Passenger = F.Passengers.Find(PassengerId);
    if (!Passenger || Passenger->AppearanceSeed != AppearanceSeed) return false;
    Passenger->SetNeuralPermission(true);
    const FPinkCabStableId MessageId(TEXT("message:code-complete:callback"));
    if (Passenger->AddMessageOnce(MessageId, TEXT("same pickup tomorrow"))
        != EPinkCabPassengerMutationResult::Applied) return false;

    bool SecondExitBlocked = false;
    int64 SecondFareMinor = 0;
    if (!CompletePaidFare(F, TEXT("fare:code-complete:2"), PassengerId,
            SecondExitBlocked, SecondFareMinor)) return false;
    if (Passenger->RegisterPaidFareOnce(
            FPinkCabStableId(TEXT("social:code-complete:2")), RelationshipDelta())
        != EPinkCabPassengerMutationResult::Applied) return false;
    R.bTwoSuccessfulFares = SecondFareMinor > 0;
    R.bRepeatEligible = Passenger->IsRepeatEligible();
    const FPinkCabOrder RepeatOrder = FPinkCabRepeatOrderFactory::Create(
        FPinkCabStableId(TEXT("fare:code-complete:repeat")),
        FPinkCabStableId(TEXT("pickup:code-complete:repeat")),
        FPinkCabStableId(TEXT("destination:code-complete:repeat")),
        *Passenger, EPinkCabFareMode::Metered);
    TArray<FPinkCabFarePassengerInput> RepeatInputs = {{PassengerId, 72.0f}};
    FPinkCabFarePassengerManifest RepeatManifest;
    FPinkCabFareLoopCoordinator RepeatFare;
    R.bRepeatOrderOrdinary = RepeatOrder.IsValid()
        && FPinkCabFarePassengerManifest::TryCreate(RepeatOrder.OrderId, RepeatInputs, RepeatManifest)
        && FPinkCabFareLoopCoordinator::TryCreate(
            RepeatOrder.OrderId, RepeatOrder,
            FPinkCabFarePricingTerms::Metered(100, 100, 10), RepeatManifest, RepeatFare)
        && RepeatFare.GetState() == EPinkCabFareLoopState::Offered;
    if (!R.bRepeatOrderOrdinary) return false;

    FPinkCabFareLoopCoordinator Declined;
    if (!BuildFare(TEXT("fare:code-complete:declined"), PassengerId,
            EPinkCabFareMode::Metered, Declined)
        || !Declined.TryDecline(true)) return false;
    R.bDeclineClean = Declined.GetState() == EPinkCabFareLoopState::Declined
        && Declined.CanAcceptNextFare();
    if (!R.bDeclineClean) return false;

    R.bOffMeterPath = RunOffMeterEvasion(F, PassengerId, R.EvasionConsequenceId);
    R.bEvasionCommitted = R.bOffMeterPath && !R.EvasionConsequenceId.IsEmpty();
    R.bEvasionPaidConflict = R.bOffMeterPath;
    if (!R.bOffMeterPath) return false;
    FPinkCabRoadGraph Graph;
    FPinkCabLaneId LaneId;
    if (!BuildRoadGraph(Graph, LaneId)) return false;
    FPinkCabEnforcementLedger Reputation(8);
    R.bFineCommittedOnce = RunEnforcement(F, LaneId, Reputation, R.FineTransactionId);
    if (!R.bFineCommittedOnce) return false;

    FPinkCabStableId ServiceOperationId;
    FPinkCabTransactionId FuelTransactionId;
    R.bServiceApplied = RunServiceAndFuel(
        F, Graph, LaneId, ServiceOperationId, FuelTransactionId);
    R.bMovingFuelApplied = R.bServiceApplied;
    if (!R.bServiceApplied) return false;

    if (!RunVertical(R.bL1Completed, R.bL2BusWindow, R.bL2MetroRoundTrip))
    {
        return false;
    }

    const uint64 PassengerSignatureBeforeSave = F.Passengers.GetReconstructionSignature();
    const int64 BalanceBeforeSave = F.Economy.GetBalanceMinor();
    const float FuelBeforeSave = F.Fuel.GetLiters();
    FPinkCabGameSnapshot Snapshot;
    auto CaptureOwners = F.Owners();
    if (!FPinkCabGamePersistenceCoordinator::Capture(City(), CaptureOwners, Snapshot))
    {
        return false;
    }
    FPinkCabPersistenceService Persistence;
    if (!Persistence.CommitSnapshot(SaveHeader(), Snapshot,
            EPinkCabPersistenceCommitReason::PeriodicCheckpoint)) return false;
    if (F.Economy.Commit(FPinkCabEconomyTransaction::Debit(
            FPinkCabTransactionId(TEXT("tx:code-complete:post-save")),
            EPinkCabTransactionType::Parking, 100,
            EPinkCabEconomyPurpose::OrdinaryPurchase))
        != EPinkCabSettlementResult::Committed) return false;
    Passenger = F.Passengers.Find(PassengerId);
    if (!Passenger || Passenger->AddMessageOnce(
            FPinkCabStableId(TEXT("message:code-complete:post-save")), TEXT("discard me"))
        != EPinkCabPassengerMutationResult::Applied) return false;
    if (!F.Health.ApplyFunctionalDamage(EPinkCabVehicleHealthChannel::Door, 0.2f))
        return false;

    FPinkCabMigrationRegistry Migrations;
    FPinkCabCityIdentity RestoredCity;
    auto RestoreOwners = F.Owners();
    if (Persistence.RecoverLastCommittedInto(
            SaveHeader(), Migrations, RestoredCity, RestoreOwners)
        != EPinkCabLoadResult::Success)
    {
        return false;
    }
    Passenger = F.Passengers.Find(PassengerId);
    R.bSnapshotRestored = RestoredCity.GetStableKey() == City().GetStableKey()
        && F.Economy.GetBalanceMinor() == BalanceBeforeSave
        && F.Passengers.GetReconstructionSignature() == PassengerSignatureBeforeSave
        && FMath::IsNearlyEqual(F.Fuel.GetLiters(), FuelBeforeSave)
        && F.Operations.IsCompleted(ServiceOperationId)
        && F.MovingFuel.GetState() == EPinkCabMovingFuelState::Completed;
    R.bNeuralContinuity = Passenger
        && Passenger->AppearanceSeed == AppearanceSeed
        && Passenger->HasNeuralPermission()
        && Passenger->GetNeuralMessages().Num() == 1
        && Passenger->GetNeuralMessages()[0].MessageId == MessageId;
    if (!R.bSnapshotRestored || !R.bNeuralContinuity) return false;

    const bool bSocialReplay = Passenger->RegisterPaidFareOnce(
        FPinkCabStableId(TEXT("social:code-complete:1")), RelationshipDelta())
        == EPinkCabPassengerMutationResult::Duplicate;
    const bool bNeuralReplay = Passenger->AddMessageOnce(
        MessageId, TEXT("same pickup tomorrow"))
        == EPinkCabPassengerMutationResult::Duplicate;
    const bool bServiceReplay = F.Operations.ChargeParking(
        F.Economy, ServiceOperationId, 500)
        == EPinkCabServiceOperationResult::Duplicate;
    const bool bFuelReplay = F.Fuel.CreditFuelOnce(FuelTransactionId, 6.0f)
        == EPinkCabFuelCreditResult::Duplicate;
    const bool bEvasionReplay = F.Settlement.RecordEvasion(
        FPinkCabStableId(TEXT("fare:code-complete:evasion"))).Disposition
        == EPinkCabFareSettlementDisposition::Replayed;
    const bool bPaidReplay = F.Settlement.CommitPaid(
        FPinkCabStableId(TEXT("fare:code-complete:1")), FirstFareMinor, 25, F.Economy).Disposition
        == EPinkCabFareSettlementDisposition::Replayed;
    const bool bFineReplay = F.Economy.Commit(FPinkCabEconomyTransaction::Debit(
        FPinkCabTransactionId(R.FineTransactionId), EPinkCabTransactionType::Fine, 700,
        EPinkCabEconomyPurpose::MandatoryDayObligation))
        == EPinkCabSettlementResult::Duplicate;
    R.bReplayNoOps = bSocialReplay && bNeuralReplay && bServiceReplay
        && bFuelReplay && bEvasionReplay && bPaidReplay && bFineReplay;
    if (!R.bReplayNoOps) return false;

    if (!F.Health.ApplyFunctionalDamage(EPinkCabVehicleHealthChannel::Brake, 0.35f))
        return false;
    const float BrakeHealthBeforeRecovery =
        F.Health.GetHealth(EPinkCabVehicleHealthChannel::Brake);
    const int64 BalanceBeforeRecovery = F.Economy.GetBalanceMinor();
    const FPinkCabStableId RecoveryFareId(TEXT("fare:code-complete:recovery"));
    if (!BuildFare(TEXT("fare:code-complete:recovery"), PassengerId,
            EPinkCabFareMode::Metered, F.Fare)
        || !F.Fare.TryAcceptPickup(true, true)
        || !F.Fare.TryBoard(true, true, F.Load)
        || !F.Fare.TryStartRide(true))
    {
        return false;
    }
    auto RecoveryOwners = F.Owners();
    if (!FPinkCabRecoveryOrchestrator::ApplyTerminalRecovery(RecoveryOwners))
        return false;
    R.bTerminalRecovery =
        F.Fare.GetState() == EPinkCabFareLoopState::Failed
        && !F.Load.HasFarePassengerGroup(RecoveryFareId)
        && F.Workday.WasTerminalRecovery();
    R.bDamagePreserved =
        FMath::IsNearlyEqual(
            F.Health.GetHealth(EPinkCabVehicleHealthChannel::Brake),
            BrakeHealthBeforeRecovery)
        && F.Economy.GetBalanceMinor() == BalanceBeforeRecovery;
    if (!R.bTerminalRecovery || !R.bDamagePreserved) return false;

    const uint64 PersistentPassengerSignature = F.Passengers.GetReconstructionSignature();
    const int64 PersistentBalance = F.Economy.GetBalanceMinor();
    const float PersistentBrakeHealth =
        F.Health.GetHealth(EPinkCabVehicleHealthChannel::Brake);
    if (!F.Workday.MarkSummaryCommitted()
        || !F.Workday.RecordHouseholdTransactionOnce(
            FPinkCabStableId(TEXT("household:code-complete:1"))))
    {
        return false;
    }

    const uint64 NextRootSeed = RootSeed ^ 0x802802ull;
    auto NextDayOwners = F.Owners();
    if (!FPinkCabRecoveryOrchestrator::AdvanceToNextWorkday(
            FPinkCabStableId(TEXT("workday:code-complete:2")),
            2, NextRootSeed, NextDayOwners))
    {
        return false;
    }
    R.bWorkdayResetScoped =
        F.Workday.GetWorkdayId() == FPinkCabStableId(TEXT("workday:code-complete:2"))
        && F.Workday.GetOrdinal() == 2
        && FMath::IsNearlyZero(F.Workday.GetElapsedGameSeconds())
        && !F.Workday.WasTerminalRecovery()
        && F.Kernel.GetRootSeed() == NextRootSeed
        && F.Passengers.GetReconstructionSignature() == PersistentPassengerSignature
        && F.Economy.GetBalanceMinor() == PersistentBalance
        && FMath::IsNearlyEqual(
            F.Health.GetHealth(EPinkCabVehicleHealthChannel::Brake),
            PersistentBrakeHealth);
    if (!R.bWorkdayResetScoped) return false;

    R.IntegratedSignature = BuildSignature(F, R);
    Out = MoveTemp(R);
    return Out.IntegratedSignature != 0ull;
}
} // namespace PinkCabCoreCodeCompleteScenario

struct FCoreCodeCompleteScenario
{
    static bool RunCanonicalScenario(uint64 RootSeed, FCoreCodeCompleteResult& Out)
    {
        return PinkCabCoreCodeCompleteScenario::RunCanonicalScenarioImpl(RootSeed, Out);
    }
};
