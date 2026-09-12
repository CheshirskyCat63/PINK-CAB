#pragma once

#include "CoreMinimal.h"
#include "Core/PinkCabStableId.h"

struct FPinkCabPassengerRelationship
{
    float Trust = 0.0f;
    float Satisfaction = 0.0f;
    float RiskTolerance = 0.0f;
};


struct FPinkCabPassengerReview
{
    int32 Stars = 0;
    FString Text;
};

struct FPinkCabPassengerPublicProfile
{
    FPinkCabStableId IdentityId;
    FName TemplateId = NAME_None;
    TArray<FPinkCabPassengerReview> LocalReviews;
};

struct FPinkCabPassengerIdentity
{
    FPinkCabPassengerIdentity() = default;
    FPinkCabPassengerIdentity(const FPinkCabStableId& InIdentityId, const FName InTemplateId)
        : IdentityId(InIdentityId), TemplateId(InTemplateId) {}

    bool RegisterPaidFare()
    {
        ++SuccessfullyPaidFares;
        if (SuccessfullyPaidFares >= 2) bRepeatEligible = true;
        return bRepeatEligible;
    }

    void RegisterAuthoredRelationshipEvent() { bRepeatEligible = true; }
    bool IsRepeatEligible() const { return bRepeatEligible; }

    bool AddLocalReview(const int32 Stars, const FString& Text)
    {
        const FString Normalized = Text.TrimStartAndEnd();
        if (Stars < 1 || Stars > 5 || Normalized.IsEmpty()) return false;
        LocalReviews.Add({Stars, Normalized});
        return true;
    }

    FPinkCabPassengerPublicProfile BuildPublicProfile() const
    {
        return {IdentityId, TemplateId, LocalReviews};
    }

    FPinkCabStableId IdentityId;
    FName TemplateId = NAME_None;
    FPinkCabPassengerRelationship Relationship;
    TArray<FPinkCabPassengerReview> LocalReviews;

private:
    int32 SuccessfullyPaidFares = 0;
    bool bRepeatEligible = false;
};
