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

struct PINKCABTAXI_API FPinkCabPassengerIdentity
{
    FPinkCabPassengerIdentity();
    FPinkCabPassengerIdentity(
        const FPinkCabStableId& InIdentityId,
        FName InTemplateId);

    bool RegisterPaidFare();
    void RegisterAuthoredRelationshipEvent();
    bool IsRepeatEligible() const;
    bool AddLocalReview(int32 Stars, const FString& Text);
    FPinkCabPassengerPublicProfile BuildPublicProfile() const;

    FPinkCabStableId IdentityId;
    FName TemplateId = NAME_None;
    FPinkCabPassengerRelationship Relationship;
    TArray<FPinkCabPassengerReview> LocalReviews;

private:
    int32 SuccessfullyPaidFares = 0;
    bool bRepeatEligible = false;
};
