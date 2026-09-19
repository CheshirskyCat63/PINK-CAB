#include "Taxi/PinkCabNeuralContactState.h"

FPinkCabNeuralContactState::FPinkCabNeuralContactState() = default;

FPinkCabNeuralContactState::FPinkCabNeuralContactState(
    const FPinkCabStableId& InIdentityId,
    int32 InMaxMessages)
    : IdentityId(InIdentityId)
    , MaxMessages(FMath::Max(1, InMaxMessages))
    , bPermissionGranted(true)
{
}

void FPinkCabNeuralContactState::SetPermission(bool bGranted)
{
    bPermissionGranted = bGranted;
}

bool FPinkCabNeuralContactState::HasPermission() const
{
    return bPermissionGranted;
}

void FPinkCabNeuralContactState::SetBlocked(bool bInBlocked)
{
    bBlocked = bInBlocked;
}

bool FPinkCabNeuralContactState::IsBlocked() const
{
    return bBlocked;
}

bool FPinkCabNeuralContactState::AddSimpleMessage(const FString& Message)
{
    const FString Normalized = Message.TrimStartAndEnd();
    if (!IdentityId.IsValid()
        || !bPermissionGranted
        || bBlocked
        || Normalized.IsEmpty())
    {
        return false;
    }

    if (Messages.Num() >= MaxMessages)
    {
        Messages.RemoveAt(0);
    }
    Messages.Add(Normalized);
    return true;
}

const TArray<FString>& FPinkCabNeuralContactState::GetMessages() const
{
    return Messages;
}
