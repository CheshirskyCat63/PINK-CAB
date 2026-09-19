#pragma once

#include "CoreMinimal.h"
#include "Core/PinkCabStableId.h"
#include "Taxi/PinkCabOrder.h"
#include "Taxi/PinkCabPassengerIdentity.h"
#include "Taxi/PinkCabPassengerRecord.h"

struct PINKCABTAXI_API FPinkCabPassengerHistory
{
    FPinkCabPassengerHistory();
    explicit FPinkCabPassengerHistory(const FPinkCabStableId& InIdentityId);

    bool RegisterPaidFare();
    void RegisterAuthoredRelationshipEvent();
    int32 GetPaidFareCount() const;
    int32 GetAuthoredRelationshipEventCount() const;
    bool IsRepeatEligible() const;

    FPinkCabStableId IdentityId;
    FPinkCabPassengerRelationship Relationship;

private:
    int32 PaidFareCount = 0;
    int32 AuthoredRelationshipEventCount = 0;
    bool bRepeatEligible = false;
    friend class FPinkCabPassengerHistoryCodec;
};

class PINKCABTAXI_API FPinkCabPassengerHistoryCodec
{
public:
    static bool Serialize(
        const FPinkCabPassengerHistory& History,
        TArray<uint8>& OutBytes);
    static bool Deserialize(
        const TArray<uint8>& Bytes,
        FPinkCabPassengerHistory& OutHistory);

private:
    static constexpr uint32 PayloadMagic = 0x50434849u;
};

struct PINKCABTAXI_API FPinkCabRepeatOrderFactory
{
    static FPinkCabOrder Create(
        const FPinkCabStableId& OrderId,
        const FPinkCabStableId& PickupId,
        const FPinkCabStableId& DestinationId,
        const FPinkCabPassengerRecord& Record,
        EPinkCabFareMode FareMode);
    static FPinkCabOrder Create(
        const FPinkCabStableId& OrderId,
        const FPinkCabStableId& PickupId,
        const FPinkCabStableId& DestinationId,
        const FPinkCabPassengerHistory& History,
        EPinkCabFareMode FareMode);

private:
    static FPinkCabOrder CreateForIdentity(
        const FPinkCabStableId& OrderId,
        const FPinkCabStableId& PickupId,
        const FPinkCabStableId& DestinationId,
        const FPinkCabStableId& PassengerId,
        bool bRepeatEligible,
        EPinkCabFareMode FareMode);
};
