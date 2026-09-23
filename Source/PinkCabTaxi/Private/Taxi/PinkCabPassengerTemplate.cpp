#include "Taxi/PinkCabPassengerTemplate.h"

bool FPinkCabPassengerGroup::TryAssignSeats(
    int32 GroupSize,
    TArray<EPinkCabPassengerSeat>& OutSeats)
{
    OutSeats.Reset();
    if (GroupSize < 1 || GroupSize > 5)
    {
        return false;
    }

    static const EPinkCabPassengerSeat Order[] = {
        EPinkCabPassengerSeat::Rear1,
        EPinkCabPassengerSeat::Rear2,
        EPinkCabPassengerSeat::Rear3,
        EPinkCabPassengerSeat::Front1,
        EPinkCabPassengerSeat::Front2};

    for (int32 Index = 0; Index < GroupSize; ++Index)
    {
        OutSeats.Add(Order[Index]);
    }
    return true;
}

FPinkCabPassengerTemplate::FPinkCabPassengerTemplate() = default;

FPinkCabPassengerTemplate::FPinkCabPassengerTemplate(
    FName InTemplateId,
    float InDefaultMassKg)
    : TemplateId(InTemplateId)
    , DefaultMassKg(FMath::Max(0.0f, InDefaultMassKg))
{
}
