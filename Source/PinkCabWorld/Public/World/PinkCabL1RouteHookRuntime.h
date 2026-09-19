#pragma once

#include "CoreMinimal.h"
#include "World/PinkCabRouteService.h"
#include "World/PinkCabVerticalContactRegistry.h"

struct PINKCABWORLD_API FPinkCabL1RouteHook
{
    FString HookId;
    FPinkCabLaneId LaneId;
    FString ContactId;
    EPinkCabVerticalContactKind Kind = EPinkCabVerticalContactKind::WallLeft;
    int32 RouteOrder = 0;

    bool IsValid() const;
};

class PINKCABWORLD_API FPinkCabL1RouteHookRuntime
{
public:
    explicit FPinkCabL1RouteHookRuntime(int32 InMaxHooks = 256);
    static FString MakeHookId(const FPinkCabVerticalContactRecord& Contact, int32 RouteOrder);
    bool TryAddHook(
        const FPinkCabRoadGraph& Graph,
        const FPinkCabVerticalContactRegistry& Contacts,
        const FString& ContactId,
        int32 RouteOrder,
        FString& OutHookId);
    bool BuildOrderedHooksForRoute(
        const FPinkCabRoute& Route,
        TArray<FPinkCabL1RouteHook>& OutHooks) const;
    FString GetReconstructionSignature() const;
    int32 Num() const;

private:
    int32 MaxHooks = 256;
    TMap<FString, FPinkCabL1RouteHook> Hooks;
    TMap<FString, TArray<FString>> HookIdsByLane;
};
