#pragma once

#include "CoreMinimal.h"
#include "Core/PinkCabStableId.h"

struct FPinkCabNeuralContactState
{
    FPinkCabNeuralContactState() = default;
    explicit FPinkCabNeuralContactState(
        const FPinkCabStableId& InIdentityId,
        int32 InMaxMessages = 64)
        : IdentityId(InIdentityId)
        , MaxMessages(FMath::Max(1, InMaxMessages))
        , bPermissionGranted(true)
    {
    }

    void SetPermission(bool bGranted) { bPermissionGranted = bGranted; }
    bool HasPermission() const { return bPermissionGranted; }
    void SetBlocked(bool bInBlocked) { bBlocked = bInBlocked; }
    bool IsBlocked() const { return bBlocked; }

    bool AddSimpleMessage(const FString& Message)
    {
        const FString Normalized = Message.TrimStartAndEnd();
        if (!IdentityId.IsValid() || !bPermissionGranted || bBlocked || Normalized.IsEmpty())
            return false;
        if (Messages.Num() >= MaxMessages) Messages.RemoveAt(0);
        Messages.Add(Normalized);
        return true;
    }

    const TArray<FString>& GetMessages() const { return Messages; }

    FPinkCabStableId IdentityId;

private:
    int32 MaxMessages = 64;
    bool bPermissionGranted = false;
    bool bBlocked = false;
    TArray<FString> Messages;
};
