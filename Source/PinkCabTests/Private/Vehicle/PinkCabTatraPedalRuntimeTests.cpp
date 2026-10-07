#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "InputKeyEventArgs.h"
#include "Runtime/PinkCabChaosTatraPawn.h"
#include "Runtime/PinkCabVehicleVisualShellComponent.h"

class FPinkCabTatraPedalCommand final : public IAutomationLatentCommand
{
public:
    FPinkCabTatraPedalCommand(FAutomationTestBase* InTest, FName InBone, FKey InKey, float InTravel)
        : Test(InTest), Bone(InBone), Key(InKey), Travel(InTravel) {}

    virtual bool Update() override
    {
        if (!Pawn.IsValid())
        {
            UWorld* World = AutomationCommon::GetAnyGameWorld();
            if (World)
            {
                for (TActorIterator<APinkCabChaosTatraPawn> It(World); It; ++It)
                {
                    Pawn = *It;
                    break;
                }
            }
            if (!Pawn.IsValid()) return false;
        }
        APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
        UPinkCabVehicleVisualShellComponent* Shell = Pawn->GetVehicleVisualShell();
        if (!PC || !Shell) return false;

        if (Phase == 0)
        {
            Test->TestEqual(TEXT("pedal test uses authored RIG06"), Pawn->GetVehicleVisualProfileId(),
                FName(TEXT("PinkCab.Visual.Tatra613.Rig06.TexturedOpenables")));
            if (!Test->TestTrue(TEXT("authored pedal exists"), Shell->GetPoseableBoneTransform(Bone, Rest))) return true;
            Pawn->SetSystemMenuOpen(false);
            SendKey(*PC, Key, IE_Pressed, 1.0f);
            Phase = 1;
            Started = FPlatformTime::Seconds();
            return false;
        }
        if (FPlatformTime::Seconds() - Started > 8.0)
        {
            SendKey(*PC, Key, IE_Released, 0.0f);
            SendKey(*PC, EKeys::MouseWheelAxis, IE_Axis, 0.0f);
            Test->AddError(TEXT("pedal input proof timed out"));
            return true;
        }

        if (Phase == 1)
        {
            // Brake/throttle dosing uses the actual controller wheel path.
            if (Key != EKeys::Q && Pulses < 20)
            {
                SendKey(*PC, EKeys::MouseWheelAxis, IE_Axis, bPulseHeld ? 0.0f : 1.0f);
                bPulseHeld = !bPulseHeld;
                if (!bPulseHeld) ++Pulses;
                return false;
            }
            if (++SettlingFrames < 8) return false;
            FTransform Pressed;
            Test->TestTrue(TEXT("pressed pedal pose readable"), Shell->GetPoseableBoneTransform(Bone, Pressed));
            const float Degrees = FMath::RadiansToDegrees(Rest.GetRotation().AngularDistance(Pressed.GetRotation()));
            Test->AddInfo(FString::Printf(TEXT("TATRA_PEDAL bone=%s pressed_deg=%.3f expected=%.1f"), *Bone.ToString(), Degrees, Travel));
            Test->TestTrue(TEXT("full real key input reaches authored pedal travel"),
                FMath::IsNearlyEqual(Degrees, Travel, 1.5f));
            Test->TestTrue(TEXT("pedal hinge does not translate"), Rest.GetLocation().Equals(Pressed.GetLocation(), 0.0001f));
            const FVector FootRest(-0.10f, 0.0f, -0.10f);
            const FVector FootPressed = (Pressed.GetRotation() * Rest.GetRotation().Inverse()).RotateVector(FootRest);
            Test->TestTrue(TEXT("press moves the authored footpad forward and down"),
                FootPressed.X > FootRest.X && FootPressed.Z < FootRest.Z);
            SendKey(*PC, Key, IE_Released, 0.0f);
            SendKey(*PC, EKeys::MouseWheelAxis, IE_Axis, 0.0f);
            Phase = 2;
            ReleaseStarted = FPlatformTime::Seconds();
            return false;
        }
        FTransform Released;
        Test->TestTrue(TEXT("released pedal pose readable"), Shell->GetPoseableBoneTransform(Bone, Released));
        const float Remaining = FMath::RadiansToDegrees(Rest.GetRotation().AngularDistance(Released.GetRotation()));
        bSawPartialRelease |= Remaining > 1.0f && Remaining < Travel - 1.0f;
        // The input contract has a 0.70 s clutch release, 0.28 s brake release
        // and 0.22 s throttle release. Eight render frames are not that contract.
        if (Remaining > 0.1f && FPlatformTime::Seconds() - ReleaseStarted < 1.5) return false;
        Test->TestTrue(TEXT("analog release is visible at intermediate travel"), bSawPartialRelease);
        Test->TestTrue(TEXT("key release returns authored pedal to rest"),
            Rest.GetRotation().Equals(Released.GetRotation(), 0.001f));
        return true;
    }

private:
    static void SendKey(APlayerController& PC, const FKey& Input, EInputEvent Event, float Amount)
    {
        PC.InputKey(FInputKeyEventArgs(nullptr, IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(),
            Input, Event, Amount, false, FPlatformTime::Cycles64()));
    }
    FAutomationTestBase* Test;
    FName Bone;
    FKey Key;
    float Travel;
    TWeakObjectPtr<APinkCabChaosTatraPawn> Pawn;
    FTransform Rest;
    int32 Phase = 0;
    int32 Pulses = 0;
    int32 SettlingFrames = 0;
    bool bPulseHeld = false;
    double Started = 0.0;
    double ReleaseStarted = 0.0;
    bool bSawPartialRelease = false;
};

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FPinkCabTatraPedalRuntimeTest,
    "PinkCab.Vehicle.Visual.TatraPedals",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

void FPinkCabTatraPedalRuntimeTest::GetTests(TArray<FString>& Names, TArray<FString>& Commands) const
{
    for (const TCHAR* Name : {TEXT("Clutch"), TEXT("Brake"), TEXT("Throttle")})
    {
        Names.Add(Name);
        Commands.Add(Name);
    }
}

bool FPinkCabTatraPedalRuntimeTest::RunTest(const FString& Parameters)
{
    if (!AutomationOpenMap(TEXT("/Game/Dev/Maps/L_PinkCab_L1_EndlessStraight"), true)) return false;
    const bool bClutch = Parameters == TEXT("Clutch");
    const bool bBrake = Parameters == TEXT("Brake");
    const FName Bone = bClutch ? TEXT("Cabin_ClutchPedal") : bBrake ? TEXT("Cabin_BrakePedal") : TEXT("Cabin_ThrottlePedal");
    ADD_LATENT_AUTOMATION_COMMAND(FPinkCabTatraPedalCommand(this, Bone,
        bClutch ? EKeys::Q : bBrake ? EKeys::W : EKeys::E, bClutch ? 24.0f : bBrake ? 20.0f : 28.0f));
    return true;
}

#endif
