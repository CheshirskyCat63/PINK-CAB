#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "EngineUtils.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "RHI.h"
#include "UnrealClient.h"
#include "Runtime/PinkCabChaosTatraPawn.h"

class FPinkCabTatraCaptureCommand final : public IAutomationLatentCommand
{
public:
    explicit FPinkCabTatraCaptureCommand(FAutomationTestBase* InTest) : Test(InTest) {}
    virtual bool Update() override
    {
        UWorld* World = AutomationCommon::GetAnyGameWorld();
        if (!World) return false;
        if (!Pawn.IsValid())
        {
            for (TActorIterator<APinkCabChaosTatraPawn> It(World); It; ++It)
            {
                Pawn = *It;
                break;
            }
            if (!Pawn.IsValid()) return false;
            Pawn->SetSystemMenuOpen(false);
            Started = FPlatformTime::Seconds();
        }
        APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
        if (!PC) return false;
        if (FPlatformTime::Seconds() - Started > 30.0)
        {
            Test->AddError(TEXT("fixed-view capture timed out"));
            return true;
        }
        if (FPlatformTime::Seconds() - Started < 2.0) return false;
        if (bAwaitingFile)
        {
            if (FScreenshotRequest::IsScreenshotRequested()) return false;
            Test->TestTrue(TEXT("rendered PNG exists"), IFileManager::Get().FileSize(*Output) > 0);
            Test->AddInfo(TEXT("TATRA_CAPTURE=") + Output);
            bAwaitingFile = false;
            ++View;
            ViewStarted = 0.0;
            if (View == 3)
            {
                PC->SetViewTarget(Pawn.Get());
                if (Camera.IsValid()) Camera->Destroy();
                return true;
            }
        }
        if (ViewStarted == 0.0)
        {
            if (!Camera.IsValid()) Camera = World->SpawnActor<ACameraActor>();
            if (!Camera.IsValid()) { Test->AddError(TEXT("capture camera missing")); return true; }
            const FVector Location[] = {FVector(-18,-40,112), FVector(450,-450,230), FVector(0,-650,65)};
            const FVector Target[] = {FVector(200,-40,100), FVector(0,0,65), FVector(0,0,65)};
            const FTransform Car = Pawn->GetActorTransform();
            const FVector Position = Car.TransformPosition(Location[View]);
            Camera->SetActorLocationAndRotation(Position, (Car.TransformPosition(Target[View])-Position).Rotation());
            Camera->GetCameraComponent()->SetFieldOfView(View == 0 ? 85.0f : 60.0f);
            PC->SetViewTarget(Camera.Get());
            ViewStarted = FPlatformTime::Seconds();
            return false;
        }
        if (FPlatformTime::Seconds() - ViewStarted < 1.0) return false;
        const TCHAR* Names[] = {TEXT("cockpit"), TEXT("exterior"), TEXT("side")};
        Output = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() / TEXT("VF90/Captures") / FString(Names[View]) + TEXT(".png"));
        IFileManager::Get().MakeDirectory(*FPaths::GetPath(Output), true);
        FScreenshotRequest::RequestScreenshot(Output, false, false);
        bAwaitingFile = true;
        return false;
    }
private:
    FAutomationTestBase* Test;
    TWeakObjectPtr<APinkCabChaosTatraPawn> Pawn;
    TWeakObjectPtr<ACameraActor> Camera;
    FString Output;
    double Started = 0.0;
    double ViewStarted = 0.0;
    int32 View = 0;
    bool bAwaitingFile = false;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPinkCabTatraCaptureTest,
    "PinkCab.Vehicle.Visual.FixedViewCapture", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter | EAutomationTestFlags::NonNullRHI)
bool FPinkCabTatraCaptureTest::RunTest(const FString& Parameters)
{
    if (GUsingNullRHI) { AddError(TEXT("FixedViewCapture requires a rendering RHI")); return false; }
    if (!AutomationOpenMap(TEXT("/Game/Dev/Maps/L_PinkCab_L1_EndlessStraight"), true)) return false;
    ADD_LATENT_AUTOMATION_COMMAND(FPinkCabTatraCaptureCommand(this));
    return true;
}

#endif
