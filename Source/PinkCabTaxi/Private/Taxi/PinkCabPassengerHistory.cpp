#include "Taxi/PinkCabPassengerHistory.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"

FPinkCabPassengerHistory::FPinkCabPassengerHistory() = default;

FPinkCabPassengerHistory::FPinkCabPassengerHistory(
    const FPinkCabStableId& InIdentityId)
    : IdentityId(InIdentityId)
{
}

bool FPinkCabPassengerHistory::RegisterPaidFare()
{
    ++PaidFareCount;
    if (PaidFareCount >= 2)
    {
        bRepeatEligible = true;
    }
    return bRepeatEligible;
}

void FPinkCabPassengerHistory::RegisterAuthoredRelationshipEvent()
{
    ++AuthoredRelationshipEventCount;
    bRepeatEligible = true;
}

int32 FPinkCabPassengerHistory::GetPaidFareCount() const { return PaidFareCount; }
int32 FPinkCabPassengerHistory::GetAuthoredRelationshipEventCount() const
{
    return AuthoredRelationshipEventCount;
}
bool FPinkCabPassengerHistory::IsRepeatEligible() const { return bRepeatEligible; }

bool FPinkCabPassengerHistoryCodec::Serialize(
    const FPinkCabPassengerHistory& History,
    TArray<uint8>& OutBytes)
{
    if (!History.IdentityId.IsValid()) return false;

    OutBytes.Reset();
    FMemoryWriter Writer(OutBytes, true);
    uint32 Magic = PayloadMagic;
    Writer << Magic;
    FString Identity = History.IdentityId.Serialize();
    int32 Paid = History.PaidFareCount;
    int32 Authored = History.AuthoredRelationshipEventCount;
    uint8 Repeat = History.bRepeatEligible ? 1u : 0u;
    float Trust = History.Relationship.Trust;
    float Satisfaction = History.Relationship.Satisfaction;
    float RiskTolerance = History.Relationship.RiskTolerance;
    Writer << Identity << Paid << Authored << Repeat;
    Writer << Trust << Satisfaction << RiskTolerance;
    Writer.Close();
    return !Writer.IsError() && OutBytes.Num() > 0;
}

bool FPinkCabPassengerHistoryCodec::Deserialize(
    const TArray<uint8>& Bytes,
    FPinkCabPassengerHistory& OutHistory)
{
    if (Bytes.Num() == 0) return false;

    FMemoryReader Reader(Bytes, true);
    uint32 Magic = 0;
    Reader << Magic;
    if (Magic != PayloadMagic) return false;

    FString Identity;
    int32 Paid = 0;
    int32 Authored = 0;
    uint8 Repeat = 0;
    float Trust = 0.0f;
    float Satisfaction = 0.0f;
    float RiskTolerance = 0.0f;
    Reader << Identity << Paid << Authored << Repeat;
    Reader << Trust << Satisfaction << RiskTolerance;
    if (Reader.IsError() || Paid < 0 || Authored < 0) return false;

    FPinkCabPassengerHistory Restored;
    if (!FPinkCabStableId::TryParse(Identity, Restored.IdentityId)) return false;

    Restored.PaidFareCount = Paid;
    Restored.AuthoredRelationshipEventCount = Authored;
    Restored.bRepeatEligible = Repeat != 0;
    Restored.Relationship.Trust = Trust;
    Restored.Relationship.Satisfaction = Satisfaction;
    Restored.Relationship.RiskTolerance = RiskTolerance;
    OutHistory = MoveTemp(Restored);
    return true;
}

FPinkCabOrder FPinkCabRepeatOrderFactory::Create(
    const FPinkCabStableId& OrderId,
    const FPinkCabStableId& PickupId,
    const FPinkCabStableId& DestinationId,
    const FPinkCabPassengerRecord& Record,
    EPinkCabFareMode FareMode)
{
    return CreateForIdentity(
        OrderId,
        PickupId,
        DestinationId,
        Record.IdentityId,
        Record.IsRepeatEligible(),
        FareMode);
}

FPinkCabOrder FPinkCabRepeatOrderFactory::Create(
    const FPinkCabStableId& OrderId,
    const FPinkCabStableId& PickupId,
    const FPinkCabStableId& DestinationId,
    const FPinkCabPassengerHistory& History,
    EPinkCabFareMode FareMode)
{
    return CreateForIdentity(
        OrderId,
        PickupId,
        DestinationId,
        History.IdentityId,
        History.IsRepeatEligible(),
        FareMode);
}

FPinkCabOrder FPinkCabRepeatOrderFactory::CreateForIdentity(
    const FPinkCabStableId& OrderId,
    const FPinkCabStableId& PickupId,
    const FPinkCabStableId& DestinationId,
    const FPinkCabStableId& PassengerId,
    bool bRepeatEligible,
    EPinkCabFareMode FareMode)
{
    FPinkCabOrder Order;
    if (!bRepeatEligible || !PassengerId.IsValid()) return Order;

    Order.OrderId = OrderId;
    Order.PickupId = PickupId;
    Order.DestinationId = DestinationId;
    Order.PassengerIdentityIds.Add(PassengerId);
    Order.FareMode = FareMode;
    return Order;
}
