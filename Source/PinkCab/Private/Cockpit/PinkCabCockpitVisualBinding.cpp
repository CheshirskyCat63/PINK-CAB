#include "Cockpit/PinkCabCockpitVisualBinding.h"

bool FPinkCabCockpitVisualBinding::ValidateUnique(
    TConstArrayView<FPinkCabCockpitVisualBinding> Bindings)
{
    TSet<uint8> Seen;
    for (const FPinkCabCockpitVisualBinding& Binding : Bindings)
    {
        const uint8 Raw = static_cast<uint8>(Binding.Slot);
        if (Binding.Slot == EPinkCabCockpitSlot::DriverCamera || Seen.Contains(Raw)
            || Binding.LocalTransform.ContainsNaN())
        {
            return false;
        }
        Seen.Add(Raw);
    }
    return true;
}
