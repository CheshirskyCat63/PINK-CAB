#include "Taxi/PinkCabPassengerIdentity.h"

FPinkCabPassengerIdentity::FPinkCabPassengerIdentity() = default;

FPinkCabPassengerIdentity::FPinkCabPassengerIdentity(
    const FPinkCabStableId& InIdentityId,
    FName InTemplateId)
    : IdentityId(InIdentityId)
    , TemplateId(InTemplateId)
{
}

bool FPinkCabPassengerIdentity::RegisterPaidFare()
{
    ++SuccessfullyPaidFares;
    if (SuccessfullyPaidFares >= 2)
    {
        bRepeatEligible = true;
    }
    return bRepeatEligible;
}

void FPinkCabPassengerIdentity::RegisterAuthoredRelationshipEvent()
{
    bRepeatEligible = true;
}

bool FPinkCabPassengerIdentity::IsRepeatEligible() const
{
    return bRepeatEligible;
}

bool FPinkCabPassengerIdentity::AddLocalReview(
    int32 Stars,
    const FString& Text)
{
    const FString Normalized = Text.TrimStartAndEnd();
    if (Stars < 1 || Stars > 5 || Normalized.IsEmpty())
    {
        return false;
    }
    LocalReviews.Add({Stars, Normalized});
    return true;
}

FPinkCabPassengerPublicProfile FPinkCabPassengerIdentity::BuildPublicProfile() const
{
    return {IdentityId, TemplateId, LocalReviews};
}
