#pragma once

#include "CoreMinimal.h"
#include "Core/PinkCabStableId.h"

struct FPinkCabNeuralContactState
{
    FPinkCabNeuralContactState() = default;
    explicit FPinkCabNeuralContactState(const FPinkCabStableId& InIdentityId)
        : IdentityId(InIdentityId)
    {
    }

    bool AddSimpleMessage(const FString& Message)
    {
        const FString Normalized = Message.TrimStartAndEnd();
        if (!IdentityId.IsValid() || Normalized.IsEmpty()) return false;
        Messages.Add(Normalized);
        return true;
    }

    const TArray<FString>& GetMessages() const { return Messages; }

    FPinkCabStableId IdentityId;

private:
    TArray<FString> Messages;
};
