#include "CoreMinimal.h"
#include "CoreGlobals.h"

#if !UE_BUILD_SHIPPING

#include "Camera/CameraComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Containers/Ticker.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/SpringArmComponent.h"
#include "HAL/FileManager.h"
#include "Misc/CommandLine.h"
#include "Misc/FileHelper.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Runtime/PinkCabChaosTatraPawn.h"
#include "Runtime/PinkCabVehicleArticulationComponent.h"
#include "Runtime/PinkCabVehicleVisualProfile.h"
#include "Runtime/PinkCabVehicleVisualShellComponent.h"
#include "TimerManager.h"
#include "UnrealClient.h"

namespace
{
constexpr int32 GHeroStageCount = 7;
const FName GHeroPanelIds[] = {
    TEXT("DoorFL"), TEXT("DoorFR"), TEXT("DoorRL"),
    TEXT("DoorRR"), TEXT("FrontLid"), TEXT("RearLid")
};
constexpr uint8 GHeroStageOpenMasks[GHeroStageCount] = {
    0u, 0x03u, 0x0Fu, 0x10u, 0x20u, 0x3Fu, 0u
};
constexpr float GHeroStageViewYaw[GHeroStageCount] = {
    135.0f, 90.0f, 90.0f, 180.0f, 0.0f, 135.0f, 135.0f
};

struct FPinkCabTatraHeroAudit : TSharedFromThis<FPinkCabTatraHeroAudit>
{
    TWeakObjectPtr<UWorld> World;
    TWeakObjectPtr<APinkCabChaosTatraPawn> Pawn;
    FString OutputDir;
    int32 Stage = 0;
    int32 Captures = 0;
    int32 Polls = 0;
    FString PendingCapture;
    bool bFinishing = false;

    void Finish(const bool bPass, const TCHAR* Reason)
    {
        if (bFinishing)
        {
            return;
        }
        bFinishing = true;
        UE_LOG(LogTemp, Display,
            TEXT("CD855_TATRA_HERO_AUDIT_COMPLETE Result=%s Reason=%s Stages=%d Captures=%d Human=PENDING"),
            bPass ? TEXT("PASS") : TEXT("FAIL"),
            Reason,
            Stage,
            Captures);
        RequestEngineExit(TEXT("CD-855 bounded Tatra hero presentation audit complete"));
    }

    bool SetPanel(const FName Id, const bool bOpen)
    {
        UPinkCabVehicleArticulationComponent* Articulation =
            Pawn.IsValid() ? Pawn->GetVehicleArticulation() : nullptr;
        return Articulation && Articulation->SetPanelOpen(Id, bOpen);
    }

    void CloseAll()
    {
        if (Pawn.IsValid() && Pawn->GetVehicleArticulation())
        {
            Pawn->GetVehicleArticulation()->CloseAll();
        }
    }

    bool ApplyStage()
    {
        if (!Pawn.IsValid() || Stage < 0 || Stage >= GHeroStageCount)
        {
            return false;
        }
        CloseAll();
        const uint8 Mask = GHeroStageOpenMasks[Stage];
        for (int32 Index = 0; Index < UE_ARRAY_COUNT(GHeroPanelIds); ++Index)
        {
            if ((Mask & (1u << Index)) != 0u
                && !SetPanel(GHeroPanelIds[Index], true))
            {
                return false;
            }
        }
        return true;
    }

    bool VerifyStage() const
    {
        const UPinkCabVehicleArticulationComponent* Articulation =
            Pawn.IsValid() ? Pawn->GetVehicleArticulation() : nullptr;
        if (!Articulation || Stage < 0 || Stage >= GHeroStageCount)
        {
            return false;
        }

        const uint8 Mask = GHeroStageOpenMasks[Stage];
        for (int32 Index = 0; Index < UE_ARRAY_COUNT(GHeroPanelIds); ++Index)
        {
            const float Fraction =
                Articulation->GetPanelFraction(GHeroPanelIds[Index]);
            const bool bExpectedOpen = (Mask & (1u << Index)) != 0u;
            if ((bExpectedOpen && Fraction < 0.96f)
                || (!bExpectedOpen && Fraction > 0.04f))
            {
                return false;
            }
        }
        return true;
    }

    void PollCapture()
    {
        if (bFinishing)
        {
            return;
        }
        if (IFileManager::Get().FileSize(*PendingCapture) > 4096)
        {
            ++Captures;
            ++Stage;
            if (Stage >= GHeroStageCount)
            {
                Finish(Captures == GHeroStageCount, TEXT("MATRIX_COMPLETE"));
                return;
            }
            ScheduleStage();
            return;
        }
        if (++Polls > 80)
        {
            Finish(false, TEXT("CAPTURE_TIMEOUT"));
            return;
        }
        const auto Self = AsShared();
        FTimerHandle Timer;
        World->GetTimerManager().SetTimer(
            Timer,
            FTimerDelegate::CreateLambda([Self]() { Self->PollCapture(); }),
            0.10f,
            false);
    }

    void CaptureStage()
    {
        if (!VerifyStage())
        {
            Finish(false, TEXT("ARTICULATION_READBACK"));
            return;
        }

        const UPinkCabVehicleVisualShellComponent* Shell =
            Pawn->GetVehicleVisualShell();
        const UPoseableMeshComponent* Driver =
            Pawn->GetPrototypeDriverVisual();
        if (!Shell || !Shell->GetProfile().IsValid()
            || !Driver || !Driver->GetSkinnedAsset()
            || Driver->GetBoneIndex(TEXT("pelvis")) == INDEX_NONE)
        {
            Finish(false, TEXT("VISUAL_PROFILE_OR_DRIVER"));
            return;
        }

        int32 MaterialOverrides = 0;
        for (const FPinkCabVehiclePresentationPart& Part
            : Shell->GetProfile().PresentationParts)
        {
            MaterialOverrides += Part.MaterialOverride.IsNull() ? 0 : 1;
        }
        if (MaterialOverrides < 80)
        {
            Finish(false, TEXT("MATERIAL_OVERRIDE_COVERAGE"));
            return;
        }

        const FString Name = FString::Printf(
            TEXT("stage_%02d.png"), Stage);
        PendingCapture = FPaths::Combine(OutputDir, Name);
        IFileManager::Get().Delete(*PendingCapture, false, true);
        Polls = 0;

        UE_LOG(LogTemp, Display,
            TEXT("CD855_TATRA_HERO_STAGE Stage=%d MaterialOverrides=%d DriverVisible=%d DoorFL=%.3f DoorFR=%.3f DoorRL=%.3f DoorRR=%.3f FrontLid=%.3f RearLid=%.3f"),
            Stage,
            MaterialOverrides,
            Driver->IsVisible() ? 1 : 0,
            Pawn->GetVehicleArticulation()->GetPanelFraction(TEXT("DoorFL")),
            Pawn->GetVehicleArticulation()->GetPanelFraction(TEXT("DoorFR")),
            Pawn->GetVehicleArticulation()->GetPanelFraction(TEXT("DoorRL")),
            Pawn->GetVehicleArticulation()->GetPanelFraction(TEXT("DoorRR")),
            Pawn->GetVehicleArticulation()->GetPanelFraction(TEXT("FrontLid")),
            Pawn->GetVehicleArticulation()->GetPanelFraction(TEXT("RearLid")));

        FScreenshotRequest::RequestScreenshot(
            PendingCapture,
            false,
            false);
        PollCapture();
    }

    bool ConfigureStageView()
    {
        if (!Pawn.IsValid() || Stage < 0 || Stage >= GHeroStageCount)
        {
            return false;
        }
        USpringArmComponent* Boom = Pawn->FindComponentByClass<USpringArmComponent>();
        if (!Boom)
        {
            return false;
        }
        Boom->TargetArmLength = 620.0f;
        Boom->SocketOffset = FVector(0.0f, 0.0f, 135.0f);
        Boom->SetRelativeRotation(
            FRotator(-12.0f, GHeroStageViewYaw[Stage], 0.0f));
        return true;
    }

    void ScheduleStage()
    {
        if (!ApplyStage() || !ConfigureStageView())
        {
            Finish(false, TEXT("STAGE_ASSIGNMENT_OR_VIEW"));
            return;
        }
        UE_LOG(LogTemp, Display,
            TEXT("CD855_TATRA_HERO_STAGE_BEGIN Stage=%d"), Stage);
        const auto Self = AsShared();
        FTimerHandle Timer;
        World->GetTimerManager().SetTimer(
            Timer,
            FTimerDelegate::CreateLambda([Self]() { Self->CaptureStage(); }),
            1.35f,
            false);
    }

    void Start()
    {
        if (!World.IsValid())
        {
            Finish(false, TEXT("WORLD"));
            return;
        }

        APinkCabChaosTatraPawn* Found = nullptr;
        int32 Count = 0;
        for (TActorIterator<APinkCabChaosTatraPawn> It(World.Get()); It; ++It)
        {
            Found = *It;
            ++Count;
        }
        if (Count != 1 || !Found)
        {
            Finish(false, TEXT("PAWN_COUNT"));
            return;
        }
        Pawn = Found;
        Found->SetSystemMenuOpen(false);
        Found->SetVehiclePresentationCamera(true);

        if (UPoseableMeshComponent* Driver = Found->GetPrototypeDriverVisual())
        {
            Driver->SetOwnerNoSee(false);
            Driver->SetVisibility(true, true);
        }

        if (!IFileManager::Get().MakeDirectory(*OutputDir, true))
        {
            Finish(false, TEXT("OUTPUT"));
            return;
        }

        UE_LOG(LogTemp, Display,
            TEXT("CD855_TATRA_HERO_AUDIT_BEGIN Profile=%s VehicleVisual=%s"),
            *Found->GetPrototypeVisualProfileId().ToString(),
            *Found->GetVehicleVisualProfileId().ToString());
        ScheduleStage();
    }
};

void BindTatraHeroAudit(UWorld* World, const UWorld::InitializationValues Values)
{
    (void)Values;
    if (!World || !World->IsGameWorld()
        || !FParse::Param(FCommandLine::Get(), TEXT("PinkCabTatraHeroAudit")))
    {
        return;
    }

    FString OutputDir;
    if (!FParse::Value(
        FCommandLine::Get(),
        TEXT("PinkCabTatraHeroCaptureDir="),
        OutputDir))
    {
        UE_LOG(LogTemp, Error,
            TEXT("CD855_TATRA_HERO_AUDIT_COMPLETE Result=FAIL Reason=MISSING_CAPTURE_DIR"));
        RequestEngineExit(TEXT("CD855 missing capture directory"));
        return;
    }

    const auto State = MakeShared<FPinkCabTatraHeroAudit>();
    State->World = World;
    State->OutputDir = OutputDir;
    const TWeakObjectPtr<UWorld> WeakWorld(World);

    World->OnWorldBeginPlay.AddLambda([State, WeakWorld]()
    {
        if (!WeakWorld.IsValid())
        {
            State->Finish(false, TEXT("WORLD_LOST"));
            return;
        }
        FTSTicker::GetCoreTicker().AddTicker(
            FTickerDelegate::CreateLambda([State](float)
            {
                State->Start();
                return false;
            }),
            4.0f);
    });
}

struct FPinkCabTatraHeroAuditRegistrar
{
    FPinkCabTatraHeroAuditRegistrar()
    {
        FWorldDelegates::OnPostWorldInitialization.AddStatic(&BindTatraHeroAudit);
    }
} GTatraHeroAuditRegistrar;
}

#endif
