#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "EngineUtils.h"
#include "HAL/PlatformTime.h"
#include "Interaction/PinkCabInteractionModel.h"
#include "Runtime/PinkCabChaosTatraPawn.h"
#include "Runtime/PinkCabVehicleVisualShellComponent.h"

namespace
{
APinkCabChaosTatraPawn* FindTatra(UWorld* World)
{
    if (!World) return nullptr;
    for (TActorIterator<APinkCabChaosTatraPawn> It(World); It; ++It)
    {
        return *It;
    }
    return nullptr;
}

bool ReadBone(
    UPinkCabVehicleVisualShellComponent& Shell,
    const FName Bone,
    FTransform& Out)
{
    return Shell.GetPoseableBoneTransform(Bone, Out) && !Out.ContainsNaN();
}

float AngularDistanceDegrees(const FTransform& A, const FTransform& B)
{
    return FMath::RadiansToDegrees(A.GetRotation().AngularDistance(B.GetRotation()));
}
}

class FPinkCabTatra613Rig06CabinControlsCommand final : public IAutomationLatentCommand
{
public:
    explicit FPinkCabTatra613Rig06CabinControlsCommand(FAutomationTestBase* InTest)
        : Test(InTest) {}

    virtual bool Update() override
    {
        UWorld* World = AutomationCommon::GetAnyGameWorld();
        APinkCabChaosTatraPawn* Pawn = FindTatra(World);
        if (!Pawn) return false;
        UPinkCabVehicleVisualShellComponent* Shell = Pawn->GetVehicleVisualShell();
        if (!Shell) return false;

        const double Now = FPlatformTime::Seconds();

        if (Phase == 0)
        {
            Test->TestEqual(
                TEXT("cabin parity uses RIG06"),
                Pawn->GetVehicleVisualProfileId(),
                FName(TEXT("PinkCab.Visual.Tatra613.Rig06.TexturedOpenables")));
            Pawn->SetSystemMenuOpen(false);

            // Normal physical manipulation path: +device Y releases because
            // HandbrakePull(DeviceY) = -DeviceY.
            Pawn->ApplyPhysicalControlMouseDelta(
                TEXT("Handbrake"), true, 0.0f, 500.0f, 0.1f);
            Pawn->ApplyPhysicalControlMouseDelta(
                NAME_None, false, 0.0f, 0.0f, 0.1f);
            Test->TestTrue(TEXT("full-fuel setup accepted"), Pawn->SetFuelMassKg(100.0f));
            Started = Now;
            Phase = 1;
            return false;
        }

        if (Phase == 1)
        {
            if ((Now - Started) < 0.15) return false;
            Test->TestTrue(
                TEXT("handbrake normal path reaches zero"),
                FMath::IsNearlyEqual(
                    Pawn->GetCockpitState().GetHandbrakeAmount(), 0.0f, 0.001f));
            Test->TestTrue(TEXT("handbrake zero pose readable"),
                ReadBone(*Shell, TEXT("Cabin_Handbrake"), HandbrakeZero));
            Test->TestTrue(TEXT("left stalk rest readable"),
                ReadBone(*Shell, TEXT("Cabin_Stalk_L"), StalkLRest));
            Test->TestTrue(TEXT("right stalk rest readable"),
                ReadBone(*Shell, TEXT("Cabin_Stalk_R"), StalkRRest));
            Test->TestTrue(TEXT("horn authored bone readable"),
                ReadBone(*Shell, TEXT("Cabin_Horn"), HornRest));
            Test->TestTrue(TEXT("gear lever rest readable"),
                ReadBone(*Shell, TEXT("Cabin_GearLever"), GearRest));
            Test->TestTrue(TEXT("front-right door rest readable"),
                ReadBone(*Shell, TEXT("Door_FR"), DoorFRRest));
            Test->TestTrue(TEXT("rear-right door rest readable"),
                ReadBone(*Shell, TEXT("Door_RR"), DoorRRRest));
            Test->TestTrue(TEXT("fuel needle full pose readable"),
                ReadBone(*Shell, TEXT("Cabin_FuelNeedle"), FuelFull));
            for (const FName Needle : {
                    FName(TEXT("Cabin_SpeedometerNeedle")),
                    FName(TEXT("Cabin_TachometerNeedle")),
                    FName(TEXT("Cabin_FuelNeedle")),
                    FName(TEXT("Cabin_TemperatureNeedle")) })
            {
                FTransform Pose;
                Test->TestTrue(
                    *FString::Printf(TEXT("%s authored needle bone is live"), *Needle.ToString()),
                    ReadBone(*Shell, Needle, Pose));
            }

            Pawn->ApplyPhysicalControlMouseDelta(
                TEXT("Handbrake"), true, 0.0f, -110.0f, 0.05f);
            Started = Now;
            Phase = 2;
            return false;
        }

        if (Phase == 2)
        {
            if ((Now - Started) < 0.10) return false;
            Test->TestTrue(
                TEXT("handbrake normal path preserves 50 percent lever"),
                FMath::IsNearlyEqual(
                    Pawn->GetCockpitState().GetHandbrakeAmount(), 0.5f, 0.001f));
            FTransform Half;
            Test->TestTrue(TEXT("handbrake half pose readable"),
                ReadBone(*Shell, TEXT("Cabin_Handbrake"), Half));
            Test->TestTrue(
                TEXT("authored handbrake half travel is 16 degrees"),
                FMath::IsNearlyEqual(
                    AngularDistanceDegrees(HandbrakeZero, Half), 16.0f, 0.35f));

            Pawn->ApplyPhysicalControlMouseDelta(
                TEXT("Handbrake"), true, 0.0f, -110.0f, 0.05f);
            Started = Now;
            Phase = 3;
            return false;
        }

        if (Phase == 3)
        {
            if ((Now - Started) < 0.10) return false;
            Test->TestTrue(
                TEXT("handbrake normal path reaches full"),
                FMath::IsNearlyEqual(
                    Pawn->GetCockpitState().GetHandbrakeAmount(), 1.0f, 0.001f));
            FTransform Full;
            Test->TestTrue(TEXT("handbrake full pose readable"),
                ReadBone(*Shell, TEXT("Cabin_Handbrake"), Full));
            Test->TestTrue(
                TEXT("active DEMO_ALL handbrake full travel is 32 degrees"),
                FMath::IsNearlyEqual(
                    AngularDistanceDegrees(HandbrakeZero, Full), 32.0f, 0.35f));
            Pawn->ApplyPhysicalControlMouseDelta(
                NAME_None, false, 0.0f, 0.0f, 0.05f);

            Test->TestTrue(
                TEXT("turn-signal interaction is accepted"),
                Pawn->ApplyCockpitInteraction({
                    FName(TEXT("TurnSignals")),
                    EPinkCabInteractionGesture::WheelIncrement,
                    1}));
            Started = Now;
            Phase = 4;
            return false;
        }

        if (Phase == 4)
        {
            if ((Now - Started) < 0.08) return false;
            Test->TestEqual(
                TEXT("right signal state reached cockpit"),
                Pawn->GetCockpitState().GetTurnSignalDirection(), 1);
            FTransform RightSignal;
            Test->TestTrue(TEXT("right signal stalk pose readable"),
                ReadBone(*Shell, TEXT("Cabin_Stalk_L"), RightSignal));
            Test->TestTrue(
                TEXT("DEMO_ALL signal stalk travel is 18 degrees"),
                FMath::IsNearlyEqual(
                    AngularDistanceDegrees(StalkLRest, RightSignal), 18.0f, 0.35f));
            StalkLRight = RightSignal;

            Test->TestTrue(
                TEXT("left turn-signal interaction is accepted"),
                Pawn->ApplyCockpitInteraction({
                    FName(TEXT("TurnSignals")),
                    EPinkCabInteractionGesture::WheelIncrement,
                    -1}));
            Started = Now;
            Phase = 5;
            return false;
        }

        if (Phase == 5)
        {
            if ((Now - Started) < 0.08) return false;
            Test->TestEqual(
                TEXT("left signal state reached cockpit"),
                Pawn->GetCockpitState().GetTurnSignalDirection(), -1);
            FTransform LeftSignal;
            Test->TestTrue(TEXT("left signal stalk pose readable"),
                ReadBone(*Shell, TEXT("Cabin_Stalk_L"), LeftSignal));
            Test->TestTrue(
                TEXT("left signal has same authored 18 degree travel"),
                FMath::IsNearlyEqual(
                    AngularDistanceDegrees(StalkLRest, LeftSignal), 18.0f, 0.35f));
            Test->TestFalse(
                TEXT("left and right authored signal poses are distinct"),
                LeftSignal.GetRotation().Equals(StalkLRight.GetRotation(), 0.001f));

            Test->TestTrue(
                TEXT("wiper step 1 interaction is accepted"),
                Pawn->ApplyCockpitInteraction({
                    FName(TEXT("Wipers")),
                    EPinkCabInteractionGesture::WheelIncrement,
                    1}));
            Started = Now;
            Phase = 6;
            return false;
        }

        if (Phase == 6)
        {
            if ((Now - Started) < 0.08) return false;
            Test->TestEqual(
                TEXT("wiper mode 1 reached cockpit"),
                Pawn->GetCockpitState().GetWiperMode(), 1);
            FTransform Wiper1;
            Test->TestTrue(TEXT("wiper mode 1 stalk pose readable"),
                ReadBone(*Shell, TEXT("Cabin_Stalk_R"), Wiper1));
            Test->TestTrue(
                TEXT("DEMO_ALL wiper detent 1 is 14 degrees"),
                FMath::IsNearlyEqual(
                    AngularDistanceDegrees(StalkRRest, Wiper1), 14.0f, 0.35f));

            Test->TestTrue(
                TEXT("wiper step 2 interaction is accepted"),
                Pawn->ApplyCockpitInteraction({
                    FName(TEXT("Wipers")),
                    EPinkCabInteractionGesture::WheelIncrement,
                    1}));
            Started = Now;
            Phase = 7;
            return false;
        }

        if (Phase == 7)
        {
            if ((Now - Started) < 0.08) return false;
            Test->TestEqual(
                TEXT("wiper mode 2 reached cockpit"),
                Pawn->GetCockpitState().GetWiperMode(), 2);
            FTransform Wiper2;
            Test->TestTrue(TEXT("wiper mode 2 stalk pose readable"),
                ReadBone(*Shell, TEXT("Cabin_Stalk_R"), Wiper2));
            Test->TestTrue(
                TEXT("DEMO_ALL wiper detent 2 is 26 degrees"),
                FMath::IsNearlyEqual(
                    AngularDistanceDegrees(StalkRRest, Wiper2), 26.0f, 0.35f));

            Test->TestTrue(
                TEXT("horn press reaches normal interaction router"),
                Pawn->ApplyCockpitInteraction({
                    FName(TEXT("Horn")),
                    EPinkCabInteractionGesture::PressHold,
                    1}));
            Started = Now;
            Phase = 8;
            return false;
        }

        if (Phase == 8)
        {
            if ((Now - Started) < 0.05) return false;
            Test->TestTrue(TEXT("horn state is active"), Pawn->GetCockpitState().IsHornActive());
            FTransform HornPressed;
            Test->TestTrue(TEXT("horn semantic bone remains readable"),
                ReadBone(*Shell, TEXT("Cabin_Horn"), HornPressed));
            Test->TestTrue(
                TEXT("no invented horn travel without an authored source action"),
                HornPressed.Equals(HornRest, 0.001f));
            Test->TestTrue(
                TEXT("horn release reaches normal interaction router"),
                Pawn->ApplyCockpitInteraction({
                    FName(TEXT("Horn")),
                    EPinkCabInteractionGesture::PressHold,
                    -1}));
            Test->TestTrue(
                TEXT("passenger-door interaction is accepted"),
                Pawn->ApplyCockpitInteraction({
                    FName(TEXT("PassengerDoor")),
                    EPinkCabInteractionGesture::WheelIncrement,
                    1}));
            Started = Now;
            Phase = 9;
            return false;
        }

        if (Phase == 9)
        {
            const bool bDoorsOpen =
                Pawn->GetTatraOpenable(TEXT("Door_FR")) > 0.99f
                && Pawn->GetTatraOpenable(TEXT("Door_RR")) > 0.99f;
            if (!bDoorsOpen && (Now - Started) < 1.0) return false;
            Test->TestFalse(TEXT("horn release clears state"), Pawn->GetCockpitState().IsHornActive());
            Test->TestTrue(TEXT("passenger-door state is open"), Pawn->GetCockpitState().IsPassengerDoorOpen());
            Test->TestTrue(TEXT("normal passenger-door state opens both right authored doors"), bDoorsOpen);
            FTransform DoorFR;
            FTransform DoorRR;
            Test->TestTrue(TEXT("front-right open pose readable"),
                ReadBone(*Shell, TEXT("Door_FR"), DoorFR));
            Test->TestTrue(TEXT("rear-right open pose readable"),
                ReadBone(*Shell, TEXT("Door_RR"), DoorRR));
            Test->TestFalse(TEXT("front-right authored door rotates"),
                DoorFR.GetRotation().Equals(DoorFRRest.GetRotation(), 0.001f));
            Test->TestFalse(TEXT("rear-right authored door rotates"),
                DoorRR.GetRotation().Equals(DoorRRRest.GetRotation(), 0.001f));

            Test->TestTrue(
                TEXT("passenger-door close interaction is accepted"),
                Pawn->ApplyCockpitInteraction({
                    FName(TEXT("PassengerDoor")),
                    EPinkCabInteractionGesture::WheelIncrement,
                    -1}));
            Started = Now;
            Phase = 10;
            return false;
        }

        if (Phase == 10)
        {
            const bool bDoorsClosed =
                Pawn->GetTatraOpenable(TEXT("Door_FR")) < 0.01f
                && Pawn->GetTatraOpenable(TEXT("Door_RR")) < 0.01f;
            if (!bDoorsClosed && (Now - Started) < 1.0) return false;
            Test->TestFalse(TEXT("passenger-door state closes"), Pawn->GetCockpitState().IsPassengerDoorOpen());
            Test->TestTrue(TEXT("both right authored doors return closed"), bDoorsClosed);

            Pawn->ApplyPhysicalControlMouseDelta(
                TEXT("Gearbox"), true, -640.0f, 0.0f, 0.05f);
            Pawn->ApplyPhysicalControlMouseDelta(
                TEXT("Gearbox"), true, 0.0f, 480.0f, 0.05f);
            Started = Now;
            Phase = 11;
            return false;
        }

        if (Phase == 11)
        {
            if ((Now - Started) < 0.08) return false;
            const FVector2D Cursor = Pawn->GetGearLeverVisualCursor();
            Test->TestTrue(TEXT("normal H-gate manipulation reaches left rail"), Cursor.X < -0.9f);
            Test->TestTrue(TEXT("normal H-gate manipulation reaches top row"), Cursor.Y > 0.9f);
            FTransform Gear;
            Test->TestTrue(TEXT("authored gear lever pose readable"),
                ReadBone(*Shell, TEXT("Cabin_GearLever"), Gear));
            Test->TestTrue(
                TEXT("authored H-gate rotates lever around lower shaft pivot"),
                AngularDistanceDegrees(GearRest, Gear) > 10.0f);
            Pawn->ApplyPhysicalControlMouseDelta(
                NAME_None, false, 0.0f, 0.0f, 0.05f);

            Test->TestTrue(TEXT("empty fuel setup accepted"), Pawn->SetFuelMassKg(0.0f));
            Started = Now;
            Phase = 12;
            return false;
        }

        if (Phase == 12)
        {
            if ((Now - Started) < 0.10) return false;
            Test->TestTrue(TEXT("fuel zero state reached"), FMath::IsNearlyZero(
                Pawn->GetVehicleLoadState().GetFuelMassKg(), 0.001f));
            Test->TestTrue(TEXT("fuel zero needle readable"),
                ReadBone(*Shell, TEXT("Cabin_FuelNeedle"), FuelZero));
            Test->TestTrue(
                TEXT("authored fuel needle spans 120 degrees empty-to-full"),
                FMath::IsNearlyEqual(
                    AngularDistanceDegrees(FuelZero, FuelFull), 120.0f, 0.75f));
            Test->TestTrue(TEXT("half fuel setup accepted"), Pawn->SetFuelMassKg(50.0f));
            Started = Now;
            Phase = 13;
            return false;
        }

        if ((Now - Started) < 0.10) return false;
        FTransform FuelHalf;
        Test->TestTrue(TEXT("fuel half needle readable"),
            ReadBone(*Shell, TEXT("Cabin_FuelNeedle"), FuelHalf));
        Test->TestTrue(
            TEXT("fuel 50 percent sits at half of authored gauge span"),
            FMath::IsNearlyEqual(
                AngularDistanceDegrees(FuelZero, FuelHalf), 60.0f, 0.75f));
        Test->TestTrue(TEXT("restore full fuel"), Pawn->SetFuelMassKg(100.0f));
        return true;
    }

private:
    FAutomationTestBase* Test = nullptr;
    int32 Phase = 0;
    double Started = 0.0;
    FTransform HandbrakeZero = FTransform::Identity;
    FTransform StalkLRest = FTransform::Identity;
    FTransform StalkRRest = FTransform::Identity;
    FTransform StalkLRight = FTransform::Identity;
    FTransform HornRest = FTransform::Identity;
    FTransform GearRest = FTransform::Identity;
    FTransform DoorFRRest = FTransform::Identity;
    FTransform DoorRRRest = FTransform::Identity;
    FTransform FuelFull = FTransform::Identity;
    FTransform FuelZero = FTransform::Identity;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabTatra613Rig06CabinControlsRuntimeTest,
    "PinkCab.Vehicle.Visual.Tatra613Rig06CabinControlsRuntime",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabTatra613Rig06CabinControlsRuntimeTest::RunTest(const FString& Parameters)
{
    const bool bOpened =
        AutomationOpenMap(TEXT("/Game/Dev/Maps/L_PinkCab_L1_EndlessStraight"), true);
    TestTrue(TEXT("RIG06 cabin parity map opens"), bOpened);
    if (!bOpened) return false;
    ADD_LATENT_AUTOMATION_COMMAND(
        FPinkCabTatra613Rig06CabinControlsCommand(this));
    return true;
}


class FPinkCabTatra613Rig06WindowsCommand final : public IAutomationLatentCommand
{
public:
    explicit FPinkCabTatra613Rig06WindowsCommand(FAutomationTestBase* InTest)
        : Test(InTest) {}

    virtual bool Update() override
    {
        UWorld* World = AutomationCommon::GetAnyGameWorld();
        APinkCabChaosTatraPawn* Pawn = FindTatra(World);
        if (!Pawn) return false;
        UPinkCabVehicleVisualShellComponent* Shell = Pawn->GetVehicleVisualShell();
        if (!Shell) return false;

        const double Now = FPlatformTime::Seconds();
        static const FName Windows[] = {
            TEXT("Window_FL"), TEXT("Window_FR"),
            TEXT("Window_RL"), TEXT("Window_RR")
        };

        if (Phase == 0)
        {
            Pawn->SetSystemMenuOpen(false);
            for (const FName Bone : Windows)
            {
                FTransform Pose;
                Test->TestTrue(
                    *FString::Printf(TEXT("%s closed pose readable"), *Bone.ToString()),
                    ReadBone(*Shell, Bone, Pose));
                Closed.Add(Bone, Pose);
                Test->TestTrue(
                    *FString::Printf(TEXT("%s accepts 50 percent target"), *Bone.ToString()),
                    Pawn->SetTatraOpenable(Bone, 0.5f));
            }
            Started = Now;
            Phase = 1;
            return false;
        }

        if (Phase == 1)
        {
            bool bHalf = true;
            for (const FName Bone : Windows)
            {
                bHalf &= FMath::IsNearlyEqual(Pawn->GetTatraOpenable(Bone), 0.5f, 0.01f);
            }
            if (!bHalf && (Now - Started) < 0.8) return false;
            Test->TestTrue(TEXT("all authored windows reach 50 percent"), bHalf);

            for (int32 Index = 0; Index < UE_ARRAY_COUNT(Windows); ++Index)
            {
                const FName Bone = Windows[Index];
                FTransform Pose;
                Test->TestTrue(
                    *FString::Printf(TEXT("%s half pose readable"), *Bone.ToString()),
                    ReadBone(*Shell, Bone, Pose));
                Half.Add(Bone, Pose);
                const FTransform* Rest = Closed.Find(Bone);
                if (!Rest) continue;
                const float ExpectedHalfAngle = Index < 2 ? 4.0661f : 0.6532f;
                Test->TestTrue(
                    *FString::Printf(TEXT("%s follows authored exponential half-roll"), *Bone.ToString()),
                    FMath::IsNearlyEqual(
                        AngularDistanceDegrees(*Rest, Pose), ExpectedHalfAngle, 0.30f));
                Test->TestFalse(
                    *FString::Printf(TEXT("%s travels into door at half"), *Bone.ToString()),
                    Pose.GetLocation().Equals(Rest->GetLocation(), 0.001f));
                Test->TestTrue(
                    *FString::Printf(TEXT("%s accepts full target"), *Bone.ToString()),
                    Pawn->SetTatraOpenable(Bone, 1.0f));
            }
            Started = Now;
            Phase = 2;
            return false;
        }

        if (Phase == 2)
        {
            bool bFull = true;
            for (const FName Bone : Windows)
            {
                bFull &= Pawn->GetTatraOpenable(Bone) > 0.99f;
            }
            if (!bFull && (Now - Started) < 0.8) return false;
            Test->TestTrue(TEXT("all authored windows reach full-down"), bFull);

            for (int32 Index = 0; Index < UE_ARRAY_COUNT(Windows); ++Index)
            {
                const FName Bone = Windows[Index];
                FTransform Pose;
                Test->TestTrue(
                    *FString::Printf(TEXT("%s full pose readable"), *Bone.ToString()),
                    ReadBone(*Shell, Bone, Pose));
                const FTransform* Rest = Closed.Find(Bone);
                const FTransform* HalfPose = Half.Find(Bone);
                if (!Rest || !HalfPose) continue;
                const float ExpectedFullAngle = Index < 2 ? 24.7153f : 25.1639f;
                Test->TestTrue(
                    *FString::Printf(TEXT("%s reaches exact V22 full roll"), *Bone.ToString()),
                    FMath::IsNearlyEqual(
                        AngularDistanceDegrees(*Rest, Pose), ExpectedFullAngle, 0.40f));
                Test->TestTrue(
                    *FString::Printf(TEXT("%s full travel exceeds half travel"), *Bone.ToString()),
                    FVector::Distance(Pose.GetLocation(), Rest->GetLocation())
                        > FVector::Distance(HalfPose->GetLocation(), Rest->GetLocation()));
                Pawn->SetTatraOpenable(Bone, 0.0f);
            }
            Started = Now;
            Phase = 3;
            return false;
        }

        bool bClosed = true;
        for (const FName Bone : Windows)
        {
            bClosed &= Pawn->GetTatraOpenable(Bone) < 0.01f;
        }
        if (!bClosed && (Now - Started) < 1.0) return false;
        Test->TestTrue(TEXT("all authored windows return closed"), bClosed);
        for (const FName Bone : Windows)
        {
            FTransform Pose;
            Test->TestTrue(
                *FString::Printf(TEXT("%s final closed pose readable"), *Bone.ToString()),
                ReadBone(*Shell, Bone, Pose));
            const FTransform* Rest = Closed.Find(Bone);
            if (Rest)
            {
                Test->TestTrue(
                    *FString::Printf(TEXT("%s returns to exact authored rest"), *Bone.ToString()),
                    Pose.Equals(*Rest, 0.002f));
            }
        }
        return true;
    }

private:
    FAutomationTestBase* Test = nullptr;
    int32 Phase = 0;
    double Started = 0.0;
    TMap<FName, FTransform> Closed;
    TMap<FName, FTransform> Half;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabTatra613Rig06WindowsRuntimeTest,
    "PinkCab.Vehicle.Visual.Tatra613Rig06WindowsRuntime",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabTatra613Rig06WindowsRuntimeTest::RunTest(const FString& Parameters)
{
    const bool bOpened =
        AutomationOpenMap(TEXT("/Game/Dev/Maps/L_PinkCab_L1_EndlessStraight"), true);
    TestTrue(TEXT("RIG06 window parity map opens"), bOpened);
    if (!bOpened) return false;
    ADD_LATENT_AUTOMATION_COMMAND(
        FPinkCabTatra613Rig06WindowsCommand(this));
    return true;
}

#endif
