#pragma once

#include "CoreMinimal.h"

enum class EPinkCabPassengerSeat : uint8
{
    Rear1,
    Rear2,
    Rear3,
    Front1,
    Front2
};

struct FPinkCabPassengerGroup
{
    static bool TryAssignSeats(const int32 GroupSize, TArray<EPinkCabPassengerSeat>& OutSeats)
    {
        OutSeats.Reset();
        if (GroupSize < 1 || GroupSize > 5)
        {
            return false;
        }
        static const EPinkCabPassengerSeat Order[] = {
            EPinkCabPassengerSeat::Rear1, EPinkCabPassengerSeat::Rear2,
            EPinkCabPassengerSeat::Rear3, EPinkCabPassengerSeat::Front1,
            EPinkCabPassengerSeat::Front2};
        for (int32 Index = 0; Index < GroupSize; ++Index) OutSeats.Add(Order[Index]);
        return true;
    }
};

struct FPinkCabPassengerTemplate
{
    FPinkCabPassengerTemplate() = default;
    FPinkCabPassengerTemplate(const FName InTemplateId, const float InDefaultMassKg)
        : TemplateId(InTemplateId), DefaultMassKg(FMath::Max(0.0f, InDefaultMassKg)) {}

    FName TemplateId = NAME_None;
    float DefaultMassKg = 0.0f;
};
