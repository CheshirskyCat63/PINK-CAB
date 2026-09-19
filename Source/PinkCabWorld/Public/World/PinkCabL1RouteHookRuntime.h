#pragma once

#include "CoreMinimal.h"
#include "World/PinkCabRouteService.h"
#include "World/PinkCabVerticalContactRegistry.h"

struct FPinkCabL1RouteHook
{
    FString HookId;
    FPinkCabLaneId LaneId;
    FString ContactId;
    EPinkCabVerticalContactKind Kind = EPinkCabVerticalContactKind::WallLeft;
    int32 RouteOrder = 0;

    bool IsValid() const
    {
        return !HookId.IsEmpty() && LaneId.IsValid()
            && !ContactId.IsEmpty() && RouteOrder >= 0;
    }
};

class FPinkCabL1RouteHookRuntime
{
public:
    explicit FPinkCabL1RouteHookRuntime(int32 InMaxHooks = 256)
        : MaxHooks(FMath::Max(1, InMaxHooks))
    {
    }
    static FString MakeHookId(const FPinkCabVerticalContactRecord& Contact, int32 RouteOrder)
    {
        if (!Contact.IsValid() || RouteOrder < 0)
        {
            return FString();
        }
        const FString Payload = FString::Printf(
            TEXT("%s|%s|%d|%d"),
            *Contact.ContactId,
            *Contact.LaneId.Serialize(),
            static_cast<int32>(Contact.Kind),
            RouteOrder);
        return PinkCabWorldId::StableToken(TEXT("l1-route-hook:"), Payload);
    }

    bool TryAddHook(
        const FPinkCabRoadGraph& Graph,
        const FPinkCabVerticalContactRegistry& Contacts,
        const FString& ContactId,
        int32 RouteOrder,
        FString& OutHookId)
    {
        FPinkCabVerticalContactRecord Contact;
        if (!Contacts.TryGet(ContactId, Contact) || RouteOrder < 0
            || !Graph.FindLane(Contact.LaneId) || Hooks.Num() >= MaxHooks)
        {
            return false;
        }
        const FString HookId = MakeHookId(Contact, RouteOrder);
        if (HookId.IsEmpty() || Hooks.Contains(HookId))
        {
            return false;
        }

        FPinkCabL1RouteHook Hook;
        Hook.HookId = HookId;
        Hook.LaneId = Contact.LaneId;
        Hook.ContactId = Contact.ContactId;
        Hook.Kind = Contact.Kind;
        Hook.RouteOrder = RouteOrder;
        Hooks.Add(HookId, Hook);
        HookIdsByLane.FindOrAdd(Hook.LaneId.Serialize()).Add(HookId);
        OutHookId = HookId;
        return true;
    }

    bool BuildOrderedHooksForRoute(
        const FPinkCabRoute& Route,
        TArray<FPinkCabL1RouteHook>& OutHooks) const
    {
        OutHooks.Reset();
        if (Route.LaneIds.IsEmpty())
        {
            return false;
        }        for (const FPinkCabLaneId& LaneId : Route.LaneIds)
        {
            const TArray<FString>* LaneHookIds = HookIdsByLane.Find(LaneId.Serialize());
            if (!LaneHookIds)
            {
                continue;
            }

            TArray<FPinkCabL1RouteHook> LaneHooks;
            LaneHooks.Reserve(LaneHookIds->Num());
            for (const FString& HookId : *LaneHookIds)
            {
                if (const FPinkCabL1RouteHook* Hook = Hooks.Find(HookId))
                {
                    LaneHooks.Add(*Hook);
                }
            }
            LaneHooks.Sort([](const FPinkCabL1RouteHook& A, const FPinkCabL1RouteHook& B)
            {
                if (A.RouteOrder != B.RouteOrder)
                {
                    return A.RouteOrder < B.RouteOrder;
                }
                return A.HookId < B.HookId;
            });
            OutHooks.Append(LaneHooks);
        }
        return !OutHooks.IsEmpty();
    }
    FString GetReconstructionSignature() const
    {
        TArray<FString> Keys;
        Hooks.GetKeys(Keys);
        Keys.Sort();
        FString Payload;
        for (const FString& Key : Keys)
        {
            const FPinkCabL1RouteHook& Hook = Hooks[Key];
            Payload += FString::Printf(
                TEXT("%s|%s|%s|%d|%d;"),
                *Hook.HookId,
                *Hook.LaneId.Serialize(),
                *Hook.ContactId,
                static_cast<int32>(Hook.Kind),
                Hook.RouteOrder);
        }
        return PinkCabWorldId::StableToken(TEXT("l1-route-hooks:"), Payload);
    }

    int32 Num() const { return Hooks.Num(); }

private:
    int32 MaxHooks = 256;
    TMap<FString, FPinkCabL1RouteHook> Hooks;
    TMap<FString, TArray<FString>> HookIdsByLane;
};