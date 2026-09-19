#include "Persistence/PinkCabPersistenceBinaryCodec.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"

bool FPinkCabPersistenceBinaryCodec::Serialize(
    const FPinkCabSaveHeader& Header,
    const FPinkCabLogicalSaveState& State,
    TArray<uint8>& OutBytes)
{
    OutBytes.Reset();
    FMemoryWriter Writer(OutBytes, true);
    uint32 Magic = PayloadMagic;
    Writer << Magic;

    FPinkCabSaveHeader MutableHeader = Header;
    Writer << MutableHeader.ProductName;
    Writer << MutableHeader.SchemaVersion;
    Writer << MutableHeader.ConfigVersion;
    Writer << MutableHeader.GeneratorVersion;
    Writer << MutableHeader.ContentSetVersion;

    FPinkCabLogicalSaveState MutableState = State;
    Writer << MutableState.VehicleHealth.ChannelHealth;
    Writer << MutableState.VehicleHealth.FunctionalDamageSerial;
    Writer << MutableState.Passenger.IdentityId;
    Writer << MutableState.Passenger.TemplateId;
    Writer << MutableState.Passenger.IdentitySeed;
    Writer << MutableState.Passenger.AppearanceSeed;
    Writer << MutableState.Passenger.AppearanceProfileId;
    Writer << MutableState.Passenger.PaidFareCount;
    Writer << MutableState.Passenger.AuthoredEventCount;
    Writer << MutableState.Passenger.bRepeatEligible;
    Writer << MutableState.Passenger.bNeuralPermissionGranted;
    Writer << MutableState.Passenger.bNeuralBlocked;
    Writer << MutableState.Passenger.Trust;
    Writer << MutableState.Passenger.Satisfaction;
    Writer << MutableState.Passenger.RiskTolerance;
    Writer << MutableState.Passenger.ReviewStars;
    Writer << MutableState.Passenger.ReviewTexts;
    Writer << MutableState.Economy.BalanceMinor;
    Writer << MutableState.Economy.DebtLimitMinor;
    Writer << MutableState.Economy.CommittedTransactionIds;
    Writer.Close();
    return !Writer.IsError() && OutBytes.Num() > 0;
}

EPinkCabLoadResult FPinkCabPersistenceBinaryCodec::Deserialize(
    const TArray<uint8>& Bytes,
    const FPinkCabSaveHeader& ExpectedHeader,
    const FPinkCabMigrationRegistry& Registry,
    FPinkCabLogicalSaveState& OutState)
{
    if (Bytes.Num() == 0)
    {
        return EPinkCabLoadResult::CorruptPayload;
    }

    FMemoryReader Reader(Bytes, true);
    uint32 Magic = 0;
    Reader << Magic;
    if (Magic != PayloadMagic)
    {
        return EPinkCabLoadResult::CorruptPayload;
    }

    FPinkCabSaveHeader SavedHeader;
    Reader << SavedHeader.ProductName;
    Reader << SavedHeader.SchemaVersion;
    Reader << SavedHeader.ConfigVersion;
    Reader << SavedHeader.GeneratorVersion;
    Reader << SavedHeader.ContentSetVersion;

    const EPinkCabLoadResult Compatibility =
        CheckCompatibility(
            SavedHeader,
            ExpectedHeader,
            Registry);
    if (Compatibility != EPinkCabLoadResult::Success)
    {
        return Compatibility;
    }

    FPinkCabLogicalSaveState Restored;
    Reader << Restored.VehicleHealth.ChannelHealth;
    Reader << Restored.VehicleHealth.FunctionalDamageSerial;
    Reader << Restored.Passenger.IdentityId;
    Reader << Restored.Passenger.TemplateId;
    Reader << Restored.Passenger.IdentitySeed;
    Reader << Restored.Passenger.AppearanceSeed;
    Reader << Restored.Passenger.AppearanceProfileId;
    Reader << Restored.Passenger.PaidFareCount;
    Reader << Restored.Passenger.AuthoredEventCount;
    Reader << Restored.Passenger.bRepeatEligible;
    Reader << Restored.Passenger.bNeuralPermissionGranted;
    Reader << Restored.Passenger.bNeuralBlocked;
    Reader << Restored.Passenger.Trust;
    Reader << Restored.Passenger.Satisfaction;
    Reader << Restored.Passenger.RiskTolerance;
    Reader << Restored.Passenger.ReviewStars;
    Reader << Restored.Passenger.ReviewTexts;
    Reader << Restored.Economy.BalanceMinor;
    Reader << Restored.Economy.DebtLimitMinor;
    Reader << Restored.Economy.CommittedTransactionIds;

    if (Reader.IsError())
    {
        return EPinkCabLoadResult::CorruptPayload;
    }

    OutState = MoveTemp(Restored);
    return EPinkCabLoadResult::Success;
}
