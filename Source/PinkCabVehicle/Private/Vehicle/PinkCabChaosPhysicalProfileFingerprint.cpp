#include "Vehicle/PinkCabChaosPhysicalProfile.h"

#include "Containers/StringConv.h"

namespace
{
int64 QuantizeProfileFloat(const float Value)
{
    return FMath::RoundToInt64(static_cast<double>(Value) * 1000000.0);
}

void AppendNameToken(FString& Out, const TCHAR* Key, const FName Value)
{
    Out.Appendf(TEXT("|%s=%s"), Key, *Value.ToString());
}

void AppendIntToken(FString& Out, const TCHAR* Key, const int32 Value)
{
    Out.Appendf(TEXT("|%s=%d"), Key, Value);
}

void AppendFloatParameter(
    FString& Out,
    const TCHAR* Key,
    const TPinkCabPhysicalParameter<float>& Parameter)
{
    Out.Appendf(
        TEXT("|%s=%lld:%u"),
        Key,
        QuantizeProfileFloat(Parameter.Value),
        static_cast<uint8>(Parameter.Authority));
}

void AppendBoolParameter(
    FString& Out,
    const TCHAR* Key,
    const TPinkCabPhysicalParameter<bool>& Parameter)
{
    Out.Appendf(
        TEXT("|%s=%d:%u"),
        Key,
        Parameter.Value ? 1 : 0,
        static_cast<uint8>(Parameter.Authority));
}

void AppendFloatArrayParameter(
    FString& Out,
    const TCHAR* Key,
    const TPinkCabPhysicalParameter<TArray<float>>& Parameter)
{
    Out.Appendf(
        TEXT("|%s#a=%u#n=%d"),
        Key,
        static_cast<uint8>(Parameter.Authority),
        Parameter.Value.Num());
    for (const float Value : Parameter.Value)
    {
        Out.Appendf(TEXT(":%lld"), QuantizeProfileFloat(Value));
    }
}

void AppendVectorArrayParameter(
    FString& Out,
    const TCHAR* Key,
    const TPinkCabPhysicalParameter<TArray<FVector2D>>& Parameter)
{
    Out.Appendf(
        TEXT("|%s#a=%u#n=%d"),
        Key,
        static_cast<uint8>(Parameter.Authority),
        Parameter.Value.Num());
    for (const FVector2D& Value : Parameter.Value)
    {
        Out.Appendf(
            TEXT(":%lld,%lld"),
            QuantizeProfileFloat(static_cast<float>(Value.X)),
            QuantizeProfileFloat(static_cast<float>(Value.Y)));
    }
}

void AppendWheelFingerprint(
    FString& Out,
    const TCHAR* Prefix,
    const FPinkCabChaosWheelPhysicalProfile& W)
{
    const auto Key = [Prefix](const TCHAR* S)
    {
        return FString::Printf(TEXT("%s.%s"), Prefix, S);
    };
    AppendFloatParameter(Out, *Key(TEXT("WheelRadiusCm")), W.WheelRadiusCm);
    AppendFloatParameter(Out, *Key(TEXT("WheelWidthCm")), W.WheelWidthCm);
    AppendFloatParameter(Out, *Key(TEXT("WheelMassKg")), W.WheelMassKg);
    AppendFloatParameter(Out, *Key(TEXT("CorneringStiffness")), W.CorneringStiffness);
    AppendFloatParameter(Out, *Key(TEXT("FrictionForceMultiplier")), W.FrictionForceMultiplier);
    AppendFloatParameter(Out, *Key(TEXT("SideSlipModifier")), W.SideSlipModifier);
    AppendFloatParameter(Out, *Key(TEXT("SlipThreshold")), W.SlipThreshold);
    AppendFloatParameter(Out, *Key(TEXT("SkidThreshold")), W.SkidThreshold);
    AppendFloatParameter(Out, *Key(TEXT("MaxSteerAngleDeg")), W.MaxSteerAngleDeg);
    AppendFloatParameter(Out, *Key(TEXT("MaxBrakeTorqueNm")), W.MaxBrakeTorqueNm);
    AppendFloatParameter(Out, *Key(TEXT("MaxHandBrakeTorqueNm")), W.MaxHandBrakeTorqueNm);
    AppendFloatParameter(Out, *Key(TEXT("SpringRate")), W.SpringRate);
    AppendFloatParameter(Out, *Key(TEXT("SpringPreload")), W.SpringPreload);
    AppendFloatParameter(Out, *Key(TEXT("SuspensionMaxRaiseCm")), W.SuspensionMaxRaiseCm);
    AppendFloatParameter(Out, *Key(TEXT("SuspensionMaxDropCm")), W.SuspensionMaxDropCm);
    AppendFloatParameter(Out, *Key(TEXT("SuspensionDampingRatio")), W.SuspensionDampingRatio);
    AppendFloatParameter(Out, *Key(TEXT("WheelLoadRatio")), W.WheelLoadRatio);
    AppendFloatParameter(Out, *Key(TEXT("RollbarScaling")), W.RollbarScaling);
    AppendBoolParameter(Out, *Key(TEXT("bABSEnabled")), W.bABSEnabled);
    AppendBoolParameter(Out, *Key(TEXT("bTractionControlEnabled")), W.bTractionControlEnabled);
    AppendBoolParameter(Out, *Key(TEXT("bAffectedBySteering")), W.bAffectedBySteering);
    AppendBoolParameter(Out, *Key(TEXT("bAffectedByEngine")), W.bAffectedByEngine);
    AppendBoolParameter(Out, *Key(TEXT("bAffectedByBrake")), W.bAffectedByBrake);
    AppendBoolParameter(Out, *Key(TEXT("bAffectedByHandbrake")), W.bAffectedByHandbrake);
}

FString BuildProfileFingerprint(const FPinkCabChaosPhysicalProfile& P)
{
    FString Out;
    Out.Reserve(2048);
    AppendNameToken(Out, TEXT("ModelId"), P.ModelId);
    AppendNameToken(Out, TEXT("ProfileId"), P.ProfileId);
    AppendIntToken(Out, TEXT("SchemaVersion"), P.SchemaVersion);
    AppendIntToken(Out, TEXT("CalibrationVersion"), P.CalibrationVersion);
    AppendNameToken(Out, TEXT("UnitSystemId"), P.UnitSystemId);
    AppendNameToken(Out, TEXT("ProvenanceSetId"), P.ProvenanceSetId);
    AppendNameToken(Out, TEXT("CompatibilityId"), P.CompatibilityId);
    AppendNameToken(Out, TEXT("MigrationId"), P.MigrationId);
    AppendIntToken(Out, TEXT("CalibrationVariant"), static_cast<int32>(P.CalibrationVariant));
    AppendFloatParameter(Out, TEXT("ReferenceMassKg"), P.ReferenceMassKg);
    AppendFloatParameter(Out, TEXT("WheelbaseMm"), P.WheelbaseMm);
    AppendFloatParameter(Out, TEXT("FrontTrackMm"), P.FrontTrackMm);
    AppendFloatParameter(Out, TEXT("RearTrackMm"), P.RearTrackMm);
    AppendFloatParameter(Out, TEXT("MaxPowerHp"), P.MaxPowerHp);
    AppendFloatParameter(Out, TEXT("MaxTorqueNm"), P.MaxTorqueNm);
    AppendFloatParameter(Out, TEXT("TerminalTargetKmh"), P.TerminalTargetKmh);
    AppendBoolParameter(Out, TEXT("bRearWheelDrive"), P.bRearWheelDrive);
    AppendFloatParameter(Out, TEXT("EngineMaxRpm"), P.EngineMaxRpm);
    AppendFloatParameter(Out, TEXT("EngineIdleRpm"), P.EngineIdleRpm);
    AppendFloatParameter(Out, TEXT("EngineBrakeEffect"), P.EngineBrakeEffect);
    AppendFloatParameter(Out, TEXT("EngineRevUpMOI"), P.EngineRevUpMOI);
    AppendFloatParameter(Out, TEXT("EngineRevDownRate"), P.EngineRevDownRate);
    AppendVectorArrayParameter(Out, TEXT("NormalizedTorqueCurve"), P.NormalizedTorqueCurve);
    AppendBoolParameter(Out, TEXT("bUseAutomaticGears"), P.bUseAutomaticGears);
    AppendBoolParameter(Out, TEXT("bUseAutoReverse"), P.bUseAutoReverse);
    AppendFloatParameter(Out, TEXT("FinalDriveRatio"), P.FinalDriveRatio);
    AppendFloatArrayParameter(Out, TEXT("ForwardGearRatios"), P.ForwardGearRatios);
    AppendFloatArrayParameter(Out, TEXT("ReverseGearRatios"), P.ReverseGearRatios);
    AppendFloatParameter(Out, TEXT("SteeringAngleRatio"), P.SteeringAngleRatio);
    AppendWheelFingerprint(Out, TEXT("Front"), P.FrontWheel);
    AppendWheelFingerprint(Out, TEXT("Rear"), P.RearWheel);
    return Out;
}

uint64 HashProfileFingerprint(const FString& Fingerprint)
{
    constexpr uint64 OffsetBasis = 14695981039346656037ull;
    constexpr uint64 Prime = 1099511628211ull;
    uint64 Hash = OffsetBasis;
    const FTCHARToUTF8 Utf8(*Fingerprint);
    const uint8* Bytes = reinterpret_cast<const uint8*>(Utf8.Get());
    for (int32 Index = 0; Index < Utf8.Length(); ++Index)
    {
        Hash ^= Bytes[Index];
        Hash *= Prime;
    }
    return Hash;
}
} // namespace

bool FPinkCabChaosPhysicalProfile::HasValidEnvelope() const
{
    return !ModelId.IsNone()
        && !ProfileId.IsNone()
        && SchemaVersion > 0
        && CalibrationVersion > 0
        && !UnitSystemId.IsNone()
        && !ProvenanceSetId.IsNone()
        && !CompatibilityId.IsNone()
        && !MigrationId.IsNone();
}

uint64 FPinkCabChaosPhysicalProfile::GetDeterministicProfileHash() const
{
    return HashProfileFingerprint(BuildProfileFingerprint(*this));
}
