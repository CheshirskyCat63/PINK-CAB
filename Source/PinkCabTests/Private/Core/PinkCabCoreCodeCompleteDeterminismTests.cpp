#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Core/PinkCabCoreCodeCompleteScenario.h"
#include "Traffic/PinkCabTrafficFlow.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabCoreCodeCompleteDeterministicReplayTest,
    "PinkCab.Core.CodeComplete.Determinism.SeedReplay",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabCoreCodeCompleteDeterministicReplayTest::RunTest(const FString& Parameters)
{
    FCoreCodeCompleteResult A, B, Different;
    TestTrue(TEXT("first canonical run"),
        FCoreCodeCompleteScenario::RunCanonicalScenario(0xC0802ull, A));
    TestTrue(TEXT("second canonical run"),
        FCoreCodeCompleteScenario::RunCanonicalScenario(0xC0802ull, B));
    TestTrue(TEXT("different-seed run"),
        FCoreCodeCompleteScenario::RunCanonicalScenario(0xC0803ull, Different));

    TestEqual(TEXT("same seed gives same signature"), A.IntegratedSignature, B.IntegratedSignature);
    TestEqual(TEXT("same seed gives same fine id"), A.FineTransactionId, B.FineTransactionId);
    TestEqual(TEXT("same seed gives same evasion id"), A.EvasionConsequenceId, B.EvasionConsequenceId);
    TestEqual(TEXT("same seed gives same passenger id"), A.PassengerIdentityId, B.PassengerIdentityId);
    TestNotEqual(TEXT("different seed changes integrated signature"),
        A.IntegratedSignature, Different.IntegratedSignature);
    return true;
}
struct FCoreCodeCompleteEnduranceResult
{
    int32 MaxPassengerRecordsSeen = 0;
    int32 MaxTrafficEntitiesSeen = 0;
    int32 MaxResolvedFaresSeen = 0;
    int32 MaxCheckpointsSeen = 0;
    int32 MaxNeuralMessagesSeen = 0;
    bool bCapacityFailuresNoMutation = false;
    bool bFinishedFixedBudget = false;
};

struct FCoreCodeCompleteEnduranceProbe
{
    static bool Run(int32 IterationBudget, FCoreCodeCompleteEnduranceResult& Out)
    {
        if (IterationBudget <= 0) return false;

        FPinkCabPassengerRegistry Passengers(2, 2, 2, 4, 1, 4);
        const FPinkCabPassengerTemplate Template(FName(TEXT("endurance")), 72.0f);
        FPinkCabPassengerRecord* Primary = nullptr;
        if (!Passengers.TryCreate(FPinkCabStableId(TEXT("passenger:endurance:0")),
                Template, TEXT("endurance-city"), {}, Primary) || !Primary)
            return false;
        FPinkCabPassengerRecord* Secondary = nullptr;
        if (!Passengers.TryCreate(FPinkCabStableId(TEXT("passenger:endurance:1")),
                Template, TEXT("endurance-city"), {}, Secondary) || !Secondary)
            return false;
        FPinkCabEconomyLedger Economy(0, 0, 4);
        FPinkCabFareSettlementService Settlement(4);
        FPinkCabTrafficFlowConstraints TrafficLimits;
        TrafficLimits.MaxLogicalEntities = 3;
        TrafficLimits.MinPlayableGapCm = 100.0;
        FPinkCabTrafficFlow Traffic(TrafficLimits);
        const FPinkCabLaneId Lane(TEXT("lane:endurance"));
        for (int32 Index = 0; Index < 3; ++Index)
        {
            if (!Traffic.TryAddOrdinary(FPinkCabTrafficEntity(
                    FString::Printf(TEXT("traffic:endurance:%d"), Index), Lane,
                    static_cast<double>(Index) * 200.0, 100.0)))
                return false;
        }

        FPinkCabCheckpointRing Checkpoints;
        bool bNoMutation = true;
        int32 Executed = 0;
        for (int32 Iteration = 0; Iteration < IterationBudget; ++Iteration)
        {
            ++Executed;
            Primary->SetNeuralPermission(true);
            const int32 MessagesBefore = Primary->GetNeuralMessages().Num();
            const EPinkCabPassengerMutationResult MessageResult = Primary->AddMessageOnce(
                FPinkCabStableId(FString::Printf(TEXT("message:endurance:%d"), Iteration)),
                TEXT("bounded"));
            if (Iteration >= 4)
            {
                bNoMutation &= MessageResult == EPinkCabPassengerMutationResult::CapacityExceeded
                    && Primary->GetNeuralMessages().Num() == MessagesBefore;
            }

            if (Iteration < 4)
            {
                const int64 BalanceBefore = Economy.GetBalanceMinor();
                const FPinkCabFareSettlementResult FareResult = Settlement.CommitPaid(
                    FPinkCabStableId(FString::Printf(TEXT("fare:endurance:%d"), Iteration)),
                    100, 0, Economy);
                if (FareResult.Disposition != EPinkCabFareSettlementDisposition::Committed
                    || Economy.GetBalanceMinor() != BalanceBefore + 100)
                    return false;
            }
            else
            {
                const int64 BalanceBefore = Economy.GetBalanceMinor();
                const int32 ResolvedBefore = Settlement.NumResolvedFares();
                const FPinkCabFareSettlementResult FareResult = Settlement.CommitPaid(
                    FPinkCabStableId(FString::Printf(TEXT("fare:endurance:%d"), Iteration)),
                    100, 0, Economy);
                bNoMutation &= FareResult.Disposition == EPinkCabFareSettlementDisposition::CapacityExceeded
                    && Economy.GetBalanceMinor() == BalanceBefore
                    && Settlement.NumResolvedFares() == ResolvedBefore;
            }
            TArray<uint8> Bytes;
            Bytes.Add(static_cast<uint8>(Iteration & 0xff));
            Checkpoints.Push(Bytes);

            Out.MaxPassengerRecordsSeen = FMath::Max(
                Out.MaxPassengerRecordsSeen, Passengers.Num());
            Out.MaxTrafficEntitiesSeen = FMath::Max(
                Out.MaxTrafficEntitiesSeen, Traffic.Num());
            Out.MaxResolvedFaresSeen = FMath::Max(
                Out.MaxResolvedFaresSeen, Settlement.NumResolvedFares());
            Out.MaxCheckpointsSeen = FMath::Max(
                Out.MaxCheckpointsSeen, Checkpoints.Num());
            Out.MaxNeuralMessagesSeen = FMath::Max(
                Out.MaxNeuralMessagesSeen, Primary->GetNeuralMessages().Num());
        }

        const int32 PassengerCountBefore = Passengers.Num();
        FPinkCabPassengerRecord* OverflowPassenger = nullptr;
        bNoMutation &= !Passengers.TryCreate(
            FPinkCabStableId(TEXT("passenger:endurance:overflow")),
            Template, TEXT("endurance-city"), {}, OverflowPassenger)
            && Passengers.Num() == PassengerCountBefore
            && OverflowPassenger == nullptr;

        const int32 TrafficCountBefore = Traffic.Num();
        bNoMutation &= !Traffic.TryAddOrdinary(FPinkCabTrafficEntity(
            TEXT("traffic:endurance:overflow"), Lane, 800.0, 100.0))
            && Traffic.Num() == TrafficCountBefore;
        Out.bCapacityFailuresNoMutation = bNoMutation;
        Out.bFinishedFixedBudget = Executed == IterationBudget;
        return Out.bCapacityFailuresNoMutation && Out.bFinishedFixedBudget;
    }
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabCoreCodeCompleteBoundedEnduranceTest,
    "PinkCab.Core.CodeComplete.Endurance.BoundedPopulations",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabCoreCodeCompleteBoundedEnduranceTest::RunTest(const FString& Parameters)
{
    FCoreCodeCompleteEnduranceResult Result;
    TestTrue(TEXT("fixed-budget endurance executes"),
        FCoreCodeCompleteEnduranceProbe::Run(32, Result));
    TestTrue(TEXT("endurance finishes exact budget"), Result.bFinishedFixedBudget);
    TestTrue(TEXT("passenger registry bounded"), Result.MaxPassengerRecordsSeen <= 2);
    TestTrue(TEXT("traffic flow bounded"), Result.MaxTrafficEntitiesSeen <= 3);
    TestTrue(TEXT("fare settlement bounded"), Result.MaxResolvedFaresSeen <= 4);
    TestTrue(TEXT("checkpoint ring bounded"),
        Result.MaxCheckpointsSeen <= FPinkCabPersistencePolicy::RollingCheckpointCount);
    TestTrue(TEXT("neural ring bounded"), Result.MaxNeuralMessagesSeen <= 1);
    TestTrue(TEXT("capacity failures are no-op"), Result.bCapacityFailuresNoMutation);
    return true;
}

#endif
