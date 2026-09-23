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

struct PINKCABTAXI_API FPinkCabPassengerGroup
{
    static bool TryAssignSeats(
        int32 GroupSize,
        TArray<EPinkCabPassengerSeat>& OutSeats);
};

struct PINKCABTAXI_API FPinkCabPassengerTemplate
{
    FPinkCabPassengerTemplate();
    FPinkCabPassengerTemplate(FName InTemplateId, float InDefaultMassKg);

    FName TemplateId = NAME_None;
    float DefaultMassKg = 0.0f;
};
