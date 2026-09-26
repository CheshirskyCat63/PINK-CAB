#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Vehicle/PinkCabChaosPhysicalProfile.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPhysicsBaselineProfileIdentityTest,
    "PinkCab.Vehicle.Physics.Baseline.ProfileIdentity",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabPhysicsBaselineProfileIdentityTest::RunTest(const FString& Parameters)
{
    const FPinkCabChaosPhysicalProfile Profile =
        FPinkCabChaosPhysicalProfile::ForVariant(EPinkCabCalibrationVariant::Nominal);
    const uint64 ProfileHash = Profile.GetDeterministicProfileHash();

    TestTrue(TEXT("baseline profile envelope is valid"), Profile.HasValidEnvelope());
    TestTrue(TEXT("baseline deterministic profile hash is non-zero"), ProfileHash != 0);

    const FString HashText =
        FString::Printf(TEXT("%016llX"), static_cast<unsigned long long>(ProfileHash));
    AddInfo(FString::Printf(
        TEXT("PINKCAB_PHYSICS_PROFILE ModelId=%s ProfileId=%s Schema=%d Calibration=%d Variant=%d Hash=%s"),
        *Profile.ModelId.ToString(),
        *Profile.ProfileId.ToString(),
        Profile.SchemaVersion,
        Profile.CalibrationVersion,
        static_cast<int32>(Profile.CalibrationVariant),
        *HashText));

    return true;
}

#endif
