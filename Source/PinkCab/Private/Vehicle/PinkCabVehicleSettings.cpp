#include "Vehicle/PinkCabVehicleSettings.h"

#include "Misc/Parse.h"
#include "Vehicle/PinkCabVehicleDefinition.h"

FSoftObjectPath UPinkCabVehicleSettings::ResolveDefinitionPath(
    const FString& CommandLine,
    const FSoftObjectPath& DefaultPath)
{
    FString Override;
    if (FParse::Value(
            *CommandLine,
            TEXT("PinkCabVehicleDefinition="),
            Override)
        && !Override.IsEmpty())
    {
        return FSoftObjectPath(Override);
    }
    return DefaultPath;
}

const UPinkCabVehicleDefinition* UPinkCabVehicleSettings::LoadSelectedDefinition(
    const FString& CommandLine) const
{
    const FSoftObjectPath Path = ResolveDefinitionPath(
        CommandLine,
        DefaultVehicleDefinition.ToSoftObjectPath());
    if (!Path.IsValid())
    {
        return nullptr;
    }
    return Cast<UPinkCabVehicleDefinition>(Path.TryLoad());
}
