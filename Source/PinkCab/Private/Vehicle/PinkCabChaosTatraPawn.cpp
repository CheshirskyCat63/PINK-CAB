#include "Vehicle/PinkCabChaosTatraPawn.h"

#include "Camera/CameraComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Cockpit/PinkCabCockpitAssemblyComponent.h"
#include "Cockpit/PinkCabCockpitInteractionComponent.h"
#include "Cockpit/PinkCabCockpitPresentationState.h"
#include "Cockpit/PinkCabCockpitServiceBridge.h"
#include "Cockpit/PinkCabCockpitVisualDriverComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/SkinnedAsset.h"
#include "EngineUtils.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerInput.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Misc/CoreDelegates.h"
#include "Math/RotationMatrix.h"
#include "Interaction/PinkCabInteractionModel.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/BodyInstance.h"
#include "PhysicsEngine/BodySetup.h"
#include "Persistence/PinkCabVehicleSnapshot.h"
#include "Vehicle/PinkCabChaosLoadBridge.h"
#include "Vehicle/PinkCabVehicleVisualShellComponent.h"
#include "Vehicle/PinkCabVehicleVisualProfile.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"
#include "UObject/ConstructorHelpers.h"
#include "Vehicle/PinkCabChaosWheelFront.h"
#include "Vehicle/PinkCabChaosWheelRear.h"
#include "Vehicle/PinkCabChaosPhysicalProfile.h"
#include "Vehicle/PinkCabChaosCockpitBridge.h"
#include "Vehicle/PinkCabCockpitInteractionRouter.h"
#include "Vehicle/PinkCabVehicleInputFrame.h"
#include "Vehicle/PinkCabVehicleInputResponse.h"

namespace
{
FVector ResolveWheelBoneLocalPosition(
    const USkeletalMeshComponent& Mesh,
    const FName BoneName)
{
    const USkinnedAsset* Asset = Mesh.GetSkinnedAsset();
    if (!Asset || BoneName.IsNone())
    {
        return FVector::ZeroVector;
    }

    const FVector BonePosition =
        Asset->GetComposedRefPoseMatrix(BoneName).GetOrigin() * Mesh.GetRelativeScale3D();
    FMatrix RootBodyMatrix = FMatrix::Identity;
    if (const FBodyInstance* BodyInstance = Mesh.GetBodyInstance())
    {
        if (BodyInstance->BodySetup.IsValid())
        {
            RootBodyMatrix = Asset->GetComposedRefPoseMatrix(BodyInstance->BodySetup->BoneName);
        }
    }
    return RootBodyMatrix.InverseTransformPosition(BonePosition);
}

const FPinkCabVehiclePresentationPart* FindPresentationPart(
    const FPinkCabVehicleVisualProfile& Profile,
    const FName PartId)
{
    return Profile.PresentationParts.FindByPredicate([PartId](const FPinkCabVehiclePresentationPart& Part)
    {
        return Part.PartId == PartId;
    });
}

bool BindChaosWheelsToTatraGeometry(
    UChaosWheeledVehicleMovementComponent& Movement,
    const USkeletalMeshComponent& Mesh,
    const FPinkCabVehicleVisualProfile& TatraVisual)
{
    if (Movement.WheelSetups.Num() != 4)
    {
        return false;
    }

    const FName PartIds[4] = { TEXT("WheelFL"), TEXT("WheelFR"), TEXT("WheelRL"), TEXT("WheelRR") };
    for (int32 Index = 0; Index < 4; ++Index)
    {
        FChaosWheelSetup& Setup = Movement.WheelSetups[Index];
        const FPinkCabVehiclePresentationPart* Part = FindPresentationPart(TatraVisual, PartIds[Index]);
        if (!Part || !Setup.WheelClass)
        {
            return false;
        }

        const UChaosVehicleWheel* WheelDefaults = Setup.WheelClass.GetDefaultObject();
        if (!WheelDefaults)
        {
            return false;
        }

        const FVector BaseRestPosition =
            ResolveWheelBoneLocalPosition(Mesh, Setup.BoneName) + WheelDefaults->Offset;
        Setup.AdditionalOffset = Part->LocalTransform.GetLocation() - BaseRestPosition;
    }
    return true;
}

FString GearLabel(const int32 Gear)
{
    return Gear < 0 ? TEXT("R")
        : (Gear == 0 ? TEXT("N") : FString::FromInt(Gear));
}

const TCHAR* MotionLabel(const EPinkCabVehicleMotionMode Mode)
{
    return Mode == EPinkCabVehicleMotionMode::Moving ? TEXT("MOVING") : TEXT("STATIONARY");
}

const TCHAR* EngagementLabel(const EPinkCabGearEngagementResult Result)
{
    switch (Result)
    {
    case EPinkCabGearEngagementResult::Neutral: return TEXT("NEUTRAL");
    case EPinkCabGearEngagementResult::ClutchDisengagedAccepted: return TEXT("CLUTCH");
    case EPinkCabGearEngagementResult::MatchedClutchlessAccepted: return TEXT("MATCH");
    case EPinkCabGearEngagementResult::GrindRefused: return TEXT("GRIND");
    case EPinkCabGearEngagementResult::ReverseLockout: return TEXT("R-LOCK");
    case EPinkCabGearEngagementResult::DangerousOverrev: return TEXT("OVERREV");
    default: return TEXT("NONE");
    }
}

FString GaugeBar(const float Value, const int32 Segments = 10)
{
    const float Clamped = FMath::Clamp(Value, 0.0f, 1.0f);
    const int32 Filled = FMath::Clamp(FMath::RoundToInt(Clamped * Segments), 0, Segments);
    FString Result;
    Result.Reserve(Segments + 2);
    Result.AppendChar(TEXT('['));
    for (int32 Index = 0; Index < Segments; ++Index)
    {
        Result.AppendChar(Index < Filled ? TEXT('#') : TEXT('-'));
    }
    Result.AppendChar(TEXT(']'));
    return Result;
}
}

APinkCabChaosTatraPawn::APinkCabChaosTatraPawn()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bTickEvenWhenPaused = true;
    AutoPossessPlayer = EAutoReceiveInput::Player0;

    USkeletalMeshComponent* VehicleMesh = GetMesh();
    if (USkeletalMesh* VehicleAsset = Cast<USkeletalMesh>(PrototypeVisualProfile.VehicleMeshPath.TryLoad()))
    {
        VehicleMesh->SetSkeletalMesh(VehicleAsset);
    }
    if (UPhysicsAsset* PhysicsAsset = Cast<UPhysicsAsset>(PrototypeVisualProfile.PhysicsAssetPath.TryLoad()))
    {
        VehicleMesh->SetPhysicsAsset(PhysicsAsset, false);
    }

    VehicleMesh->SetCollisionProfileName(TEXT("Vehicle"));
    VehicleMesh->SetSimulatePhysics(true);
    VehicleMesh->SetOwnerNoSee(true);

    VehicleVisualShell = CreateDefaultSubobject<UPinkCabVehicleVisualShellComponent>(TEXT("VehicleVisualShell"));
    VehicleVisualShell->SetupAttachment(VehicleMesh);

    CockpitAssembly = CreateDefaultSubobject<UPinkCabCockpitAssemblyComponent>(TEXT("CockpitAssembly"));
    CockpitAssembly->SetupAttachment(VehicleMesh);
    CockpitAssembly->SetRelativeTransform(PrototypeVisualProfile.CockpitRootTransform);

    CockpitInteraction = CreateDefaultSubobject<UPinkCabCockpitInteractionComponent>(TEXT("CockpitInteraction"));
    CockpitVisualDriver = CreateDefaultSubobject<UPinkCabCockpitVisualDriverComponent>(TEXT("CockpitVisualDriver"));

    PrototypeDriverVisual = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PrototypeDriverVisual"));
    PrototypeDriverVisual->SetupAttachment(VehicleMesh);
    PrototypeDriverVisual->SetRelativeTransform(PrototypeVisualProfile.DriverTransform);
    PrototypeDriverVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    PrototypeDriverVisual->SetGenerateOverlapEvents(false);
    PrototypeDriverVisual->SetCastShadow(false);
    PrototypeDriverVisual->SetOwnerNoSee(true);
    if (USkeletalMesh* DriverAsset = Cast<USkeletalMesh>(PrototypeVisualProfile.DriverMeshPath.TryLoad()))
    {
        PrototypeDriverVisual->SetSkeletalMesh(DriverAsset);
    }

    DriverHeadRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DriverHeadRoot"));
    DriverHeadRoot->SetupAttachment(CockpitAssembly);
    DriverHeadRoot->SetRelativeLocation(FVector(-15.0f, -38.0f, 128.0f));

    DriverCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("DriverCamera"));
    DriverCamera->SetupAttachment(DriverHeadRoot);
    DriverCamera->SetFieldOfView(86.0f);
    DriverCamera->SetRelativeRotation(FRotator(-4.0f, 0.0f, 0.0f));
    DriverCamera->bUsePawnControlRotation = false;
    DriverCamera->SetAutoActivate(true);
    CockpitAssembly->RegisterExternalSlot(EPinkCabCockpitSlot::DriverCamera, DriverCamera);

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(VehicleMesh);
    CameraBoom->TargetArmLength = 520.0f;
    CameraBoom->SocketOffset = FVector(0.0f, 0.0f, 150.0f);
    CameraBoom->bDoCollisionTest = false;
    CameraBoom->bInheritPitch = false;
    CameraBoom->bInheritRoll = false;

    ChaseCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ChaseCamera"));
    ChaseCamera->SetupAttachment(CameraBoom);
    ChaseCamera->SetAutoActivate(false);

    UChaosWheeledVehicleMovementComponent* Movement = GetChaosMovement();
    check(Movement);
    DynamicsProvider = FPinkCabChaosVehicleDynamicsProvider(Movement);

    const FPinkCabChaosPhysicalProfile Profile =
        FPinkCabChaosPhysicalProfile::ForVariant(EPinkCabCalibrationVariant::Nominal);
    Profile.ApplyToMovement(*Movement);

    VehicleLoadState.SetFuelMassKg(TatraProfile.FullFuelMassKg);
    VehicleLoadState.SetCrew(TatraProfile.HeroineMassKg, TatraProfile.DaughterMassKg);
    SyncLoadToChaos();

    Movement->WheelSetups.SetNum(4);
    Movement->WheelSetups[0].WheelClass = UPinkCabChaosWheelFront::StaticClass();
    Movement->WheelSetups[0].BoneName = PrototypeVisualProfile.WheelBones[0];
    Movement->WheelSetups[1].WheelClass = UPinkCabChaosWheelFront::StaticClass();
    Movement->WheelSetups[1].BoneName = PrototypeVisualProfile.WheelBones[1];
    Movement->WheelSetups[2].WheelClass = UPinkCabChaosWheelRear::StaticClass();
    Movement->WheelSetups[2].BoneName = PrototypeVisualProfile.WheelBones[2];
    Movement->WheelSetups[3].WheelClass = UPinkCabChaosWheelRear::StaticClass();
    Movement->WheelSetups[3].BoneName = PrototypeVisualProfile.WheelBones[3];

    const FPinkCabVehicleVisualProfile TatraVisual = FPinkCabVehicleVisualProfile::Tatra613Donor();
    if (!BindChaosWheelsToTatraGeometry(*Movement, *VehicleMesh, TatraVisual))
    {
        UE_LOG(LogTemp, Error, TEXT("PinkCab Tatra wheel geometry contract could not be bound; disabling Chaos wheel setups."));
        Movement->WheelSetups.Reset();
    }
}

void APinkCabChaosTatraPawn::BeginPlay()
{
    Super::BeginPlay();
    DynamicsProvider = FPinkCabChaosVehicleDynamicsProvider(GetChaosMovement());
    EnsurePlayableLighting();
    ApplyVehicleVisualProfile(FPinkCabVehicleVisualProfile::Tatra613Donor());
    DriverCamera->SetActive(true);
    ChaseCamera->SetActive(false);
    SyncLoadToChaos();
    ControlState.SetHandbrake(HandbrakeActuator.GetBrakeCommand());
    ControlState.SetDriveline(0, 0, 0.0f);
    SyncCockpitToChaos();

    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        ApplyGameplayInputMode(PC);
    }
    MountPlayableHud();
    SetSystemMenuOpen(true);
    ApplicationWillDeactivateHandle = FCoreDelegates::ApplicationWillDeactivateDelegate.AddUObject(
        this, &APinkCabChaosTatraPawn::HandleApplicationWillDeactivate);
}

void APinkCabChaosTatraPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (ApplicationWillDeactivateHandle.IsValid())
    {
        FCoreDelegates::ApplicationWillDeactivateDelegate.Remove(ApplicationWillDeactivateHandle);
        ApplicationWillDeactivateHandle.Reset();
    }
    ResetTransientCockpitInput();
    UnmountSystemMenu();
    UnmountPlayableHud();
    Super::EndPlay(EndPlayReason);
}

void APinkCabChaosTatraPawn::EnsurePlayableLighting()
{
    UWorld* World = GetWorld();
    if (!World) return;
    const FName RigTag(TEXT("PinkCab.PlayableLighting"));
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        if (It->ActorHasTag(RigTag)) return;
    }

    AActor* Rig = World->SpawnActor<AActor>();
    if (!Rig) return;
    Rig->Tags.Add(RigTag);

    USceneComponent* Root = NewObject<USceneComponent>(Rig, TEXT("SkyRigRoot"));
    Rig->AddInstanceComponent(Root);
    Rig->SetRootComponent(Root);
    Root->RegisterComponent();

    USkyAtmosphereComponent* Atmosphere = NewObject<USkyAtmosphereComponent>(Rig, TEXT("PlayableSkyAtmosphere"));
    Rig->AddInstanceComponent(Atmosphere);
    Atmosphere->SetupAttachment(Root);
    Atmosphere->RegisterComponent();

    UDirectionalLightComponent* Sun = NewObject<UDirectionalLightComponent>(Rig, TEXT("PlayableSun"));
    Rig->AddInstanceComponent(Sun);
    Sun->SetupAttachment(Root);
    Sun->SetMobility(EComponentMobility::Movable);
    Sun->SetIntensity(4.0f);
    Sun->SetAtmosphereSunLight(true);
    Sun->SetRelativeRotation(FRotator(-32.0f, -28.0f, 0.0f));
    Sun->RegisterComponent();

    USkyLightComponent* SkyLight = NewObject<USkyLightComponent>(Rig, TEXT("PlayableSkyLight"));
    Rig->AddInstanceComponent(SkyLight);
    SkyLight->SetupAttachment(Root);
    SkyLight->SetMobility(EComponentMobility::Movable);
    SkyLight->SetIntensity(0.8f);
    SkyLight->SetRealTimeCapture(true);
    SkyLight->RegisterComponent();
}

void APinkCabChaosTatraPawn::MountPlayableHud()
{
    if (PlayableHudOverlay.IsValid() || !GEngine || !GEngine->GameViewport) return;
    TWeakObjectPtr<APinkCabChaosTatraPawn> WeakThis(this);
    PlayableHudOverlay = SNew(SOverlay).Visibility(EVisibility::HitTestInvisible)
        + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
        [ SNew(STextBlock).Text(FText::FromString(FString::Chr(0x2022))).Font(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), 22)).ColorAndOpacity(FLinearColor::White) ]
        + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center).Padding(FMargin(0,34,0,0))
        [ SNew(STextBlock).Font(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 10)).ColorAndOpacity(FLinearColor(1,1,1,0.82f)).Text_Lambda([WeakThis]() {
            if (!WeakThis.IsValid() || !WeakThis->CockpitInteraction) return FText::GetEmpty();
            if (WeakThis->bGearLeverDragging)
            {
                const int32 PreviewGear = UPinkCabCockpitVisualDriverComponent::GearForCursor(WeakThis->GearLeverCursor);
                const FString GearText = PreviewGear < 0 ? TEXT("R") : (PreviewGear == 0 ? TEXT("N") : FString::FromInt(PreviewGear));
                return FText::FromString(FString::Printf(TEXT("GEAR -> %s"), *GearText));
            }
            const FName Target = WeakThis->CockpitInteraction->GetCurrentTargetId();
            return Target.IsNone() ? FText::GetEmpty() : FText::FromName(Target);
        }) ]
        + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Bottom).Padding(FMargin(20,0,20,28))
        [ SNew(SBorder).Padding(FMargin(16,12)).BorderBackgroundColor(FLinearColor(0.015f,0.018f,0.022f,0.88f))
          [ SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()
            [ SNew(SHorizontalBox)
              + SHorizontalBox::Slot().AutoWidth().Padding(0,0,28,0)
              [ SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight()[ SNew(STextBlock).Font(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"),9)).ColorAndOpacity(FLinearColor(1,1,1,0.62f)).Text(FText::FromString(TEXT("SPEED"))) ]
                + SVerticalBox::Slot().AutoHeight()[ SNew(STextBlock).Font(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"),30)).ColorAndOpacity(FLinearColor::White).Text_Lambda([WeakThis]() {
                    return WeakThis.IsValid()
                        ? FText::FromString(FString::Printf(TEXT("%03d  km/h"), FMath::RoundToInt(FMath::Abs(WeakThis->LastSpeedKmh))))
                        : FText::GetEmpty();
                }) ]
              ]
              + SHorizontalBox::Slot().AutoWidth().Padding(0,0,28,0)
              [ SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight()[ SNew(STextBlock).Font(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"),9)).ColorAndOpacity(FLinearColor(1,1,1,0.62f)).Text(FText::FromString(TEXT("GEAR"))) ]
                + SVerticalBox::Slot().AutoHeight()[ SNew(STextBlock).Font(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"),30)).ColorAndOpacity(FLinearColor(0.92f,0.95f,1.0f,1.0f)).Text_Lambda([WeakThis]() {
                    return WeakThis.IsValid() ? FText::FromString(GearLabel(WeakThis->GearboxController.GetEngagedGear())) : FText::GetEmpty();
                }) ]
              ]
              + SHorizontalBox::Slot().AutoWidth()
              [ SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight()[ SNew(STextBlock).Font(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"),9)).ColorAndOpacity(FLinearColor(1,1,1,0.62f)).Text(FText::FromString(TEXT("ENGINE RPM"))) ]
                + SVerticalBox::Slot().AutoHeight()[ SNew(STextBlock).Font(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"),24)).ColorAndOpacity(FLinearColor::White).Text_Lambda([WeakThis]() {
                    return WeakThis.IsValid()
                        ? FText::FromString(FString::Printf(TEXT("%04d"), FMath::RoundToInt(FMath::Max(WeakThis->LastEngineRpm, 0.0f))))
                        : FText::GetEmpty();
                }) ]
              ]
            ]
            + SVerticalBox::Slot().AutoHeight().Padding(0,6,0,0)
            [ SNew(STextBlock).Font(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"),11)).ColorAndOpacity(FLinearColor(0.88f,0.92f,0.96f,1.0f)).Text_Lambda([WeakThis]() {
                if (!WeakThis.IsValid()) return FText::GetEmpty();
                const EPinkCabIgnitionState Ignition = WeakThis->CockpitState.GetIgnitionState();
                const TCHAR* Engine = Ignition == EPinkCabIgnitionState::Running ? TEXT("ENGINE ON")
                    : (Ignition == EPinkCabIgnitionState::Stalled ? TEXT("ENGINE STALLED") : TEXT("ENGINE OFF"));
                const float Fuel01 = WeakThis->TatraProfile.FullFuelMassKg > KINDA_SMALL_NUMBER
                    ? FMath::Clamp(WeakThis->VehicleLoadState.GetFuelMassKg() / WeakThis->TatraProfile.FullFuelMassKg, 0.0f, 1.0f)
                    : 0.0f;
                return FText::FromString(FString::Printf(
                    TEXT("%s   |   %s   |   FUEL %3d%%   |   TEMP %3d%%"),
                    Engine,
                    MotionLabel(WeakThis->GetMotionMode()),
                    FMath::RoundToInt(Fuel01 * 100.0f),
                    FMath::RoundToInt(FMath::Clamp(WeakThis->EngineTemperature01, 0.0f, 1.0f) * 100.0f)));
            }) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0,5,0,0)
            [ SNew(STextBlock).Font(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"),11)).ColorAndOpacity(FLinearColor(0.82f,0.9f,1.0f,1.0f)).Text_Lambda([WeakThis]() {
                if (!WeakThis.IsValid()) return FText::GetEmpty();
                return FText::FromString(FString::Printf(
                    TEXT("CLUTCH %s %3d%%    GAS %s %3d%%"),
                    *GaugeBar(WeakThis->ControlState.Clutch),
                    FMath::RoundToInt(WeakThis->ControlState.Clutch * 100.0f),
                    *GaugeBar(WeakThis->ControlState.Throttle),
                    FMath::RoundToInt(WeakThis->ControlState.Throttle * 100.0f)));
            }) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0,3,0,0)
            [ SNew(STextBlock).Font(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"),11)).ColorAndOpacity(FLinearColor(0.82f,0.9f,1.0f,1.0f)).Text_Lambda([WeakThis]() {
                if (!WeakThis.IsValid()) return FText::GetEmpty();
                const float Handbrake = WeakThis->HandbrakeActuator.GetBrakeCommand();
                return FText::FromString(FString::Printf(
                    TEXT("BRAKE  %s %3d%%    H-BRAKE %s %3d%%"),
                    *GaugeBar(WeakThis->ControlState.Brake),
                    FMath::RoundToInt(WeakThis->ControlState.Brake * 100.0f),
                    *GaugeBar(Handbrake),
                    FMath::RoundToInt(Handbrake * 100.0f)));
            }) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0,5,0,0)
            [ SNew(STextBlock).Font(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"),10)).ColorAndOpacity(FLinearColor(1.0f,0.78f,0.30f,1.0f)).Text_Lambda([WeakThis]() {
                if (!WeakThis.IsValid()) return FText::GetEmpty();
                TArray<FString> Warnings;
                const float Steering = WeakThis->ControlState.Steering;
                const FString SteeringText = FMath::Abs(Steering) < 0.02f
                    ? TEXT("STEER CENTER")
                    : FString::Printf(
                        TEXT("STEER %s %d%%"),
                        Steering > 0.0f ? TEXT("R") : TEXT("L"),
                        FMath::RoundToInt(FMath::Abs(Steering) * 100.0f));
                Warnings.Add(SteeringText);
                Warnings.Add(FString::Printf(
                    TEXT("REQ %s / ENG %s"),
                    *GearLabel(WeakThis->GearboxController.GetRequestedGear()),
                    *GearLabel(WeakThis->GearboxController.GetEngagedGear())));
                if (WeakThis->CockpitState.GetIgnitionState() == EPinkCabIgnitionState::Stalled) Warnings.Add(TEXT("STALL"));
                if (WeakThis->LaunchController.RequiresThrottleDose()) Warnings.Add(TEXT("SET THROTTLE"));
                if (WeakThis->HandbrakeActuator.GetBrakeCommand() > 0.02f) Warnings.Add(TEXT("HANDBRAKE"));
                if (WeakThis->GetVehicleHealthState().GetBrakeTemperature01() > 0.75f) Warnings.Add(TEXT("BRAKES HOT"));
                if (WeakThis->GetVehicleHealthState().GetClutchTemperature01() > 0.75f) Warnings.Add(TEXT("CLUTCH HOT"));
                const EPinkCabGearEngagementResult GearResult = WeakThis->GearboxController.GetLastResult();
                if (GearResult == EPinkCabGearEngagementResult::GrindRefused
                    || GearResult == EPinkCabGearEngagementResult::ReverseLockout
                    || GearResult == EPinkCabGearEngagementResult::DangerousOverrev)
                {
                    Warnings.Add(EngagementLabel(GearResult));
                }
                if (WeakThis->HandbrakeActuator.IsParkingLatched()) Warnings.Add(TEXT("PARK LATCH"));
                return FText::FromString(FString::Join(Warnings, TEXT("   |   ")));
            }) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0,7,0,0)
            [ SNew(STextBlock).Font(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"),9)).ColorAndOpacity(FLinearColor(1,1,1,0.58f))
              .Text(FText::FromString(TEXT("MOUSE STEER   Q CLUTCH   W BRAKE   E GAS   3 GEARBOX   4 HANDBRAKE   SPACE LOOK"))) ]
          ] ];
    GEngine->GameViewport->AddViewportWidgetContent(PlayableHudOverlay.ToSharedRef(),1000);
}

void APinkCabChaosTatraPawn::UnmountPlayableHud()
{
    if (!PlayableHudOverlay.IsValid()) return;
    if (GEngine && GEngine->GameViewport) GEngine->GameViewport->RemoveViewportWidgetContent(PlayableHudOverlay.ToSharedRef());
    PlayableHudOverlay.Reset();
}

void APinkCabChaosTatraPawn::ApplyGameplayInputMode(APlayerController* PC)
{
    if (!PC) return;
    FInputModeGameOnly InputMode;
    InputMode.SetConsumeCaptureMouseDown(false);
    PC->SetInputMode(InputMode);
    PC->bShowMouseCursor = false;
    if (GEngine && GEngine->GameViewport)
    {
        GEngine->GameViewport->SetMouseCaptureMode(EMouseCaptureMode::CapturePermanently_IncludingInitialMouseDown);
        GEngine->GameViewport->SetMouseLockMode(EMouseLockMode::LockOnCapture);
        GEngine->GameViewport->SetHideCursorDuringCapture(true);
    }
}

void APinkCabChaosTatraPawn::ApplySystemMenuInputMode(APlayerController* PC)
{
    if (!PC) return;
    FInputModeGameAndUI InputMode;
    InputMode.SetHideCursorDuringCapture(false);
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    if (SystemMenuOverlay.IsValid())
    {
        InputMode.SetWidgetToFocus(SystemMenuOverlay);
    }
    PC->SetInputMode(InputMode);
    PC->bShowMouseCursor = true;
    if (GEngine && GEngine->GameViewport)
    {
        GEngine->GameViewport->SetMouseCaptureMode(EMouseCaptureMode::NoCapture);
        GEngine->GameViewport->SetMouseLockMode(EMouseLockMode::DoNotLock);
        GEngine->GameViewport->SetHideCursorDuringCapture(false);
    }
}

void APinkCabChaosTatraPawn::MountSystemMenu()
{
    if (SystemMenuOverlay.IsValid() || !GEngine || !GEngine->GameViewport) return;

    TWeakObjectPtr<APinkCabChaosTatraPawn> WeakThis(this);
    const FLinearColor Pink(1.0f, 0.05f, 0.42f, 1.0f);
    SystemMenuOverlay = SNew(SOverlay)
        + SOverlay::Slot()
        [
            SNew(SBorder)
            .BorderBackgroundColor(FLinearColor(0.01f, 0.01f, 0.015f, 0.90f))
        ]
        + SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Center).Padding(FMargin(96.0f, 0.0f, 0.0f, 0.0f))
        [
            SNew(SBorder)
            .Padding(FMargin(30.0f, 26.0f))
            .BorderBackgroundColor(FLinearColor(0.025f, 0.025f, 0.035f, 0.97f))
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight()
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(TEXT("PINK CAB")))
                    .Font(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), 42))
                    .ColorAndOpacity(Pink)
                ]
                + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 24)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(TEXT("SYSTEM MENU")))
                    .Font(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 12))
                    .ColorAndOpacity(FLinearColor(1, 1, 1, 0.62f))
                ]
                + SVerticalBox::Slot().AutoHeight().Padding(0, 4)
                [
                    SNew(SButton)
                    .ContentPadding(FMargin(22, 10))
                    .ButtonColorAndOpacity(Pink)
                    .OnClicked_Lambda([WeakThis]()
                    {
                        if (WeakThis.IsValid()) WeakThis->SetSystemMenuOpen(false);
                        return FReply::Handled();
                    })
                    [
                        SNew(STextBlock).Text(FText::FromString(TEXT("DRIVE")))
                        .Font(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), 18))
                        .ColorAndOpacity(FLinearColor::White)
                    ]
                ]
                + SVerticalBox::Slot().AutoHeight().Padding(0, 4)
                [
                    SNew(SButton)
                    .ContentPadding(FMargin(22, 10))
                    .OnClicked_Lambda([WeakThis]()
                    {
                        if (WeakThis.IsValid()) WeakThis->bSystemMenuSettingsOpen = !WeakThis->bSystemMenuSettingsOpen;
                        return FReply::Handled();
                    })
                    [
                        SNew(STextBlock).Text(FText::FromString(TEXT("SETTINGS")))
                        .Font(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), 18))
                    ]
                ]
                + SVerticalBox::Slot().AutoHeight().Padding(0, 4)
                [
                    SNew(SButton)
                    .ContentPadding(FMargin(22, 10))
                    .OnClicked_Lambda([WeakThis]()
                    {
                        if (WeakThis.IsValid())
                        {
                            if (APlayerController* PC = Cast<APlayerController>(WeakThis->GetController()))
                            {
                                UKismetSystemLibrary::QuitGame(WeakThis.Get(), PC, EQuitPreference::Quit, false);
                            }
                        }
                        return FReply::Handled();
                    })
                    [
                        SNew(STextBlock).Text(FText::FromString(TEXT("QUIT")))
                        .Font(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), 18))
                    ]
                ]
                + SVerticalBox::Slot().AutoHeight().Padding(0, 18, 0, 0)
                [
                    SNew(SBorder)
                    .Padding(FMargin(14, 12))
                    .BorderBackgroundColor(FLinearColor(1, 1, 1, 0.055f))
                    .Visibility_Lambda([WeakThis]()
                    {
                        return WeakThis.IsValid() && WeakThis->bSystemMenuSettingsOpen
                            ? EVisibility::Visible : EVisibility::Collapsed;
                    })
                    [
                        SNew(SVerticalBox)
                        + SVerticalBox::Slot().AutoHeight()
                        [
                            SNew(STextBlock).Text(FText::FromString(TEXT("SETTINGS / CONTROLS")))
                            .Font(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), 13))
                            .ColorAndOpacity(Pink)
                        ]
                        + SVerticalBox::Slot().AutoHeight().Padding(0, 6, 0, 0)
                        [
                            SNew(STextBlock)
                            .Text(FText::FromString(TEXT("ESC  menu   MOUSE  steer   SPACE  look\nQ clutch   W brake   E throttle\nFull settings panel comes after the playable foundation.")))
                            .Font(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 11))
                            .ColorAndOpacity(FLinearColor(1, 1, 1, 0.78f))
                        ]
                    ]
                ]
                + SVerticalBox::Slot().AutoHeight().Padding(0, 20, 0, 0)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(TEXT("ESC TO RESUME")))
                    .Font(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 10))
                    .ColorAndOpacity(FLinearColor(1, 1, 1, 0.45f))
                ]
            ]
        ];

    GEngine->GameViewport->AddViewportWidgetContent(SystemMenuOverlay.ToSharedRef(), 2000);
}

void APinkCabChaosTatraPawn::UnmountSystemMenu()
{
    if (!SystemMenuOverlay.IsValid()) return;
    if (GEngine && GEngine->GameViewport)
    {
        GEngine->GameViewport->RemoveViewportWidgetContent(SystemMenuOverlay.ToSharedRef());
    }
    SystemMenuOverlay.Reset();
}

void APinkCabChaosTatraPawn::SetSystemMenuOpen(const bool bOpen)
{
    if (bSystemMenuOpen == bOpen && SystemMenuOverlay.IsValid() == bOpen) return;

    ResetTransientCockpitInput();
    bSystemMenuOpen = bOpen;
    bSystemMenuSettingsOpen = false;

    APlayerController* PC = Cast<APlayerController>(GetController());
    if (bOpen)
    {
        MountSystemMenu();
        UGameplayStatics::SetGamePaused(this, true);
        ApplySystemMenuInputMode(PC);
        return;
    }

    UnmountSystemMenu();
    UGameplayStatics::SetGamePaused(this, false);
    ApplyGameplayInputMode(PC);
}

void APinkCabChaosTatraPawn::SetGearboxPointerCapture(APlayerController* PC, const bool bCaptured)
{
    if (!PC || bGearboxPointerCaptured == bCaptured) return;
    bGearboxPointerCaptured = bCaptured;

    FInputModeGameOnly InputMode;
    InputMode.SetConsumeCaptureMouseDown(false);
    PC->SetInputMode(InputMode);
    PC->bShowMouseCursor = false;
    if (GEngine && GEngine->GameViewport)
    {
        GEngine->GameViewport->SetMouseCaptureMode(EMouseCaptureMode::CapturePermanently_IncludingInitialMouseDown);
        GEngine->GameViewport->SetMouseLockMode(bCaptured ? EMouseLockMode::LockAlways : EMouseLockMode::LockOnCapture);
        GEngine->GameViewport->SetHideCursorDuringCapture(true);
    }
}

void APinkCabChaosTatraPawn::HandleApplicationWillDeactivate()
{
    ResetTransientCockpitInput();
}

void APinkCabChaosTatraPawn::ResetTransientCockpitInput()
{
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        SetGearboxPointerCapture(PC, false);
    }
    bThrottleHeldLastFrame = false;
    bGearLeverDragging = false;
    GearLeverCursor = UPinkCabCockpitVisualDriverComponent::GearCursorForGear(CockpitState.GetSelectedGear());
    if (!CockpitInteraction)
    {
        return;
    }
    TArray<FPinkCabInteractionEvent> ReleaseEvents;
    CockpitInteraction->ResetTransientInputState(&ReleaseEvents);
    for (const FPinkCabInteractionEvent& ReleaseEvent : ReleaseEvents)
    {
        ApplyCockpitInteraction(ReleaseEvent);
    }

    GearboxController.CancelPendingRequest();
    CockpitState.SetSelectedGear(GearboxController.GetEngagedGear());

    if (MotionClassifier.GetMode() == EPinkCabVehicleMotionMode::Moving)
    {
        HandbrakeActuator.Reset(0.0f, false);
        CockpitState.SetHandbrakeAmount(0.0f);
        ControlState.SetHandbrake(0.0f);
        SyncCockpitToChaos();
    }
}

bool APinkCabChaosTatraPawn::ApplyCockpitInteraction(const FPinkCabInteractionEvent& Event)
{
    if (!FPinkCabCockpitInteractionRouter::Apply(Event, CockpitState))
    {
        return false;
    }
    SyncCockpitToChaos();
    return true;
}

void APinkCabChaosTatraPawn::SyncCockpitToChaos()
{
    if (UChaosWheeledVehicleMovementComponent* Movement = GetChaosMovement())
    {
        ApplyHealthToControls();
        FPinkCabChaosCockpitBridge::Apply(CockpitState, *Movement, ControlState, DynamicsProvider);
        if (!VehicleHealthService.HasCapability(GetVehicleHealthState(), EPinkCabVehicleCapability::RunEngine))
        {
            Movement->EnableMechanicalSim(false);
        }
    }
}

FName APinkCabChaosTatraPawn::GetVehicleVisualProfileId() const
{
    return VehicleVisualShell ? VehicleVisualShell->GetProfileId() : NAME_None;
}

bool APinkCabChaosTatraPawn::ConfigureSourceSteeringVisual(
    const FPinkCabVehicleVisualProfile& Profile)
{
    if (!VehicleVisualShell || !CockpitVisualDriver) return false;

    CockpitVisualDriver->SetSteeringVisualComponent(nullptr);
    if (SourceSteeringPivot)
    {
        SourceSteeringPivot->DestroyComponent();
        SourceSteeringPivot = nullptr;
    }

    if (Profile.SteeringPresentationPartId.IsNone())
    {
        return true;
    }

    UStaticMeshComponent* SourceSteering =
        VehicleVisualShell->GetPresentationPartComponent(Profile.SteeringPresentationPartId);
    const FVector Axis = Profile.SteeringPresentationAxis.GetSafeNormal();
    if (!SourceSteering || Axis.IsNearlyZero())
    {
        return false;
    }

    USceneComponent* Pivot = NewObject<USceneComponent>(this);
    if (!Pivot) return false;
    AddInstanceComponent(Pivot);
    Pivot->SetupAttachment(VehicleVisualShell);
    Pivot->SetMobility(EComponentMobility::Movable);
    Pivot->SetRelativeLocation(Profile.SteeringPresentationPivot);
    Pivot->SetRelativeRotation(FRotationMatrix::MakeFromX(Axis).Rotator());
    Pivot->RegisterComponent();

    // Preserve the exact authored steering wheel placement. Only its parent
    // changes; the pivot itself sits on the source t613_steer origin.
    SourceSteering->AttachToComponent(Pivot, FAttachmentTransformRules::KeepWorldTransform);
    SourceSteeringPivot = Pivot;
    CockpitVisualDriver->SetSteeringVisualComponent(Pivot);
    return true;
}

bool APinkCabChaosTatraPawn::ApplyVehicleVisualProfile(const FPinkCabVehicleVisualProfile& Profile)
{
    if (!VehicleVisualShell || !CockpitAssembly || !CockpitVisualDriver || !Profile.IsValid()) return false;
    const FPinkCabVehicleVisualProfile Previous = VehicleVisualShell->GetProfile();

    CockpitVisualDriver->SetSteeringVisualComponent(nullptr);
    if (SourceSteeringPivot)
    {
        SourceSteeringPivot->DestroyComponent();
        SourceSteeringPivot = nullptr;
    }

    if (!VehicleVisualShell->ApplyProfile(Profile)) return false;
    if (!CockpitAssembly->ApplyVisualBindings(Profile.CockpitBindings))
    {
        VehicleVisualShell->ApplyProfile(Previous);
        ConfigureSourceSteeringVisual(Previous);
        return false;
    }
    CockpitAssembly->SetGeneratedVisualMode(!Profile.HasVisualAsset(), Profile.CockpitBindings);

    if (!ConfigureSourceSteeringVisual(Profile))
    {
        VehicleVisualShell->ApplyProfile(Previous);
        CockpitAssembly->ApplyVisualBindings(Previous.CockpitBindings);
        CockpitAssembly->SetGeneratedVisualMode(!Previous.HasVisualAsset(), Previous.CockpitBindings);
        ConfigureSourceSteeringVisual(Previous);
        return false;
    }

    CockpitAssembly->SetRelativeTransform(Profile.CockpitRootTransform);
    DriverHeadRoot->SetRelativeTransform(Profile.DriverHeadTransform);
    CockpitVisualDriver->InvalidateBaseTransforms();
    if (USkeletalMeshComponent* VehicleMesh = GetMesh())
    {
        VehicleMesh->SetVisibility(!(Profile.HasVisualAsset() && Profile.bHidePhysicsChassisWhenExteriorPresent), false);
    }
    return true;
}

bool APinkCabChaosTatraPawn::SyncLoadToChaos()
{
    UChaosWheeledVehicleMovementComponent* Movement = GetChaosMovement();
    return Movement && FPinkCabChaosLoadBridge::Apply(VehicleLoadState, TatraProfile, *Movement);
}

void APinkCabChaosTatraPawn::ApplyHealthToControls()
{
    VehicleHealthService.ApplyCapabilitiesToControls(GetVehicleHealthState(), ControlState);
}

bool APinkCabChaosTatraPawn::SetFuelMassKg(const float MassKg, const float LongitudinalCm)
{
    if (!FMath::IsFinite(MassKg) || !FMath::IsFinite(LongitudinalCm) || MassKg < 0.0f)
    {
        return false;
    }
    const FPinkCabVehicleLoadState Before = VehicleLoadState;
    VehicleLoadState.SetFuelMassKg(MassKg, LongitudinalCm);
    if (SyncLoadToChaos()) return true;
    VehicleLoadState = Before;
    SyncLoadToChaos();
    return false;
}

bool APinkCabChaosTatraPawn::TrySetFarePassengerGroup(
    const FPinkCabStableId& GroupId,
    TConstArrayView<FPinkCabVehicleLoadItem> Items)
{
    const FPinkCabVehicleLoadState Before = VehicleLoadState;
    if (!VehicleLoadState.TrySetFarePassengerGroup(GroupId, Items)) return false;
    if (SyncLoadToChaos()) return true;
    VehicleLoadState = Before;
    SyncLoadToChaos();
    return false;
}

bool APinkCabChaosTatraPawn::RemoveFarePassengerGroup(const FPinkCabStableId& GroupId)
{
    const FPinkCabVehicleLoadState Before = VehicleLoadState;
    if (!VehicleLoadState.RemoveFarePassengerGroup(GroupId)) return false;
    if (SyncLoadToChaos()) return true;
    VehicleLoadState = Before;
    SyncLoadToChaos();
    return false;
}

bool APinkCabChaosTatraPawn::SetVehicleDamageProfile(const FPinkCabVehicleDamageProfile& Profile)
{
    if (Profile.GetProfileId().IsNone()) return false;
    VehicleDamageProfile = Profile;
    return true;
}

bool APinkCabChaosTatraPawn::ApplyAuthoredVehicleHit(FName ZoneId, float CollisionSeverity)
{
    FPinkCabVehicleHitEvent Event;
    if (!VehicleDamageProfile.ResolveFunctionalHit(ZoneId, CollisionSeverity, Event)) return false;
    return ApplyVehicleHit(Event);
}

bool APinkCabChaosTatraPawn::ApplyVehicleHit(const FPinkCabVehicleHitEvent& Event)
{
    if (!VehicleHealthService.ApplyHit(GetMutableVehicleHealthState(), Event)) return false;
    ApplyHealthToControls();
    SyncCockpitToChaos();
    if (!VehicleHealthService.HasCapability(GetVehicleHealthState(), EPinkCabVehicleCapability::RunEngine))
    {
        if (UChaosWheeledVehicleMovementComponent* Movement = GetChaosMovement())
        {
            Movement->EnableMechanicalSim(false);
        }
    }
    return true;
}

bool APinkCabChaosTatraPawn::CaptureVehicleSnapshot(FPinkCabVehicleSnapshot& OutSnapshot) const
{
    return FPinkCabVehicleSnapshotCodec::Capture(GetVehicleHealthState(), VehicleLoadState, OutSnapshot);
}

bool APinkCabChaosTatraPawn::RestoreVehicleSnapshot(const FPinkCabVehicleSnapshot& Snapshot)
{
    FPinkCabVehicleHealthState RestoredHealth;
    FPinkCabVehicleLoadState RestoredLoad;
    if (!FPinkCabVehicleSnapshotCodec::Restore(Snapshot, RestoredHealth, RestoredLoad)) return false;

    FPinkCabVehicleHealthState& MutableHealth = GetMutableVehicleHealthState();
    const FPinkCabVehicleHealthState BeforeHealth = MutableHealth;
    const FPinkCabVehicleLoadState BeforeLoad = VehicleLoadState;
    MutableHealth = RestoredHealth;
    VehicleLoadState = RestoredLoad;
    if (!SyncLoadToChaos())
    {
        MutableHealth = BeforeHealth;
        VehicleLoadState = BeforeLoad;
        SyncLoadToChaos();
        return false;
    }
    ApplyHealthToControls();
    SyncCockpitToChaos();
    return true;
}

void APinkCabChaosTatraPawn::ApplyMouseSteeringDelta(
    const float DeltaX,
    const bool bGazeHeld,
    const float DeltaSeconds)
{
    SteeringCommand = SteeringController.Step(
        DeltaX,
        bGazeHeld,
        LastSpeedKmh,
        MotionClassifier.GetMode(),
        DeltaSeconds);
    ControlState.SetSteering(SteeringCommand);
}


void APinkCabChaosTatraPawn::ApplyPhysicalControlMouseDelta(
    const FName TargetId,
    const bool bGripHeld,
    const float MouseDeltaX,
    const float MouseDeltaY,
    const float DeltaSeconds)
{
    if (TargetId == FName(TEXT("Gearbox")) && bGripHeld)
    {
        GearboxController.ApplyLeverMouseDelta(MouseDeltaX, MouseDeltaY);
        CockpitState.SetSelectedGear(GearboxController.GetRequestedGear());
        GearLeverCursor = FVector2D(
            GearboxController.GetLeverX(),
            GearboxController.GetLeverY());
    }

    const bool bHandbrakeGrip =
        TargetId == FName(TEXT("Handbrake")) && bGripHeld;
    HandbrakeActuator.Step(
        MotionClassifier.GetMode(),
        bHandbrakeGrip,
        bHandbrakeGrip ? MouseDeltaY : 0.0f,
        DeltaSeconds);
    CockpitState.SetHandbrakeAmount(HandbrakeActuator.GetLeverPosition());
    ControlState.SetHandbrake(HandbrakeActuator.GetBrakeCommand());
    SyncCockpitToChaos();
}

void APinkCabChaosTatraPawn::ApplyVehicleInputFrame(
    const FPinkCabVehicleInputFrame& InputFrame,
    const float MouseDeltaX,
    const float DeltaSeconds)
{
    FPinkCabVehicleInputFrame EffectiveInput = InputFrame;

    FPinkCabGearEngagementContext GearContext;
    GearContext.ClutchPedal = EffectiveInput.Clutch;
    GearContext.EngineRpm = LastEngineRpm;
    GearContext.SpeedKmh = LastSpeedKmh;
    GearContext.Throttle = EffectiveInput.Throttle;
    GearContext.Brake = EffectiveInput.Brake;
    FPinkCabVehicleHealthState& Health = GetMutableVehicleHealthState();
    GearContext.GearboxHealth =
        Health.GetHealth(EPinkCabVehicleHealthChannel::Gearbox);

    if (CockpitState.GetSelectedGear() != GearboxController.GetRequestedGear())
    {
        GearboxController.RequestGear(CockpitState.GetSelectedGear(), GearContext);
    }
    else
    {
        GearboxController.EvaluateCurrentEngagement(GearContext);
    }

    if (GearboxController.GetEventSerial() != LastProcessedGearEventSerial)
    {
        DrivetrainCondition.RecordGearEvent(
            GearboxController.GetLastResult(),
            Health);
        LastProcessedGearEventSerial = GearboxController.GetEventSerial();
    }

    FPinkCabDrivetrainConditionInput ConditionInput;
    ConditionInput.DeltaSeconds = DeltaSeconds;
    ConditionInput.bEngineRunning =
        CockpitState.GetIgnitionState() == EPinkCabIgnitionState::Running;
    ConditionInput.EngineRpm = LastEngineRpm;
    ConditionInput.ExpectedCoupledRpm = GearboxController.GetExpectedCoupledRpm();
    ConditionInput.SpeedKmh = LastSpeedKmh;
    ConditionInput.Throttle = EffectiveInput.Throttle;
    ConditionInput.Brake = EffectiveInput.Brake;
    ConditionInput.Handbrake = HandbrakeActuator.GetBrakeCommand();
    ConditionInput.ClutchCoupling =
        GearboxController.ComputeClutchCoupling(EffectiveInput.Clutch);
    ConditionInput.EngagedGear = GearboxController.GetEngagedGear();
    const FPinkCabDrivetrainConditionOutput ConditionOutput =
        DrivetrainCondition.Step(ConditionInput, Health);

    EffectiveInput.Brake *= ConditionOutput.BrakeEffectiveness;
    const float EffectiveHandbrake =
        HandbrakeActuator.GetBrakeCommand()
        * (MotionClassifier.GetMode() == EPinkCabVehicleMotionMode::Moving
            ? ConditionOutput.HandbrakeEffectiveness
            : 1.0f);
    DrivetrainTorqueCapacity = ConditionOutput.DrivetrainTorqueCapacity;
    if (ConditionOutput.bShouldStall)
    {
        CockpitState.StallEngine();
    }

    if (CockpitInteraction)
    {
        CockpitInteraction->SetGazeHeld(EffectiveInput.bGazeHeld);
    }
    ApplyMouseSteeringDelta(MouseDeltaX, EffectiveInput.bGazeHeld, DeltaSeconds);
    ControlState = EffectiveInput.ToControlState(
        SteeringCommand,
        EffectiveHandbrake);
    ControlState.SetDriveline(
        GearboxController.GetRequestedGear(),
        GearboxController.GetEngagedGear(),
        GearboxController.ComputeClutchCoupling(EffectiveInput.Clutch)
            * DrivetrainTorqueCapacity);
    SyncCockpitToChaos();
}

void APinkCabChaosTatraPawn::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    APlayerController* PC = Cast<APlayerController>(GetController());
    if (PC && !PlayableHudOverlay.IsValid())
    {
        MountPlayableHud();
    }
    if (PC && PC->WasInputKeyJustPressed(EKeys::Escape))
    {
        SetSystemMenuOpen(!bSystemMenuOpen);
    }
    if (!PC || UGameplayStatics::IsGamePaused(this))
    {
        ResetTransientCockpitInput();
        return;
    }

    float MouseX = 0.0f;
    float MouseY = 0.0f;
    PC->GetInputMouseDelta(MouseX, MouseY);

    // GetInputMouseDelta is already multiplied by the project's 0.07 mouse
    // sensitivity. That is appropriate for free-look, but it destroys the
    // physical ±5 cm steering workspace. Feed steering the raw mouse delta
    // while keeping gaze/cockpit look on the authored processed sensitivity.
    float SteeringMouseX = MouseX;
    if (PC->PlayerInput)
    {
        SteeringMouseX = FPinkCabSteeringController::ResolveHorizontalMouseDelta(
            MouseX,
            PC->PlayerInput->GetRawKeyValue(EKeys::MouseX));
    }

    FPinkCabVehicleInputFrame InputFrame = FPinkCabVehicleInputFrame::FromRouter(
        InputRouter,
        [PC](const FKey& Key) { return PC->IsInputKeyDown(Key); });

    const EPinkCabVehicleMotionMode PreviousMotionMode = MotionClassifier.GetMode();
    FPinkCabVehicleTelemetry MotionTelemetry;
    if (DynamicsProvider.ReadTelemetry(MotionTelemetry))
    {
        LastSpeedKmh = MotionTelemetry.SpeedKmh;
        LastEngineRpm = MotionTelemetry.EngineRpm;
        MotionClassifier.Update(LastSpeedKmh, DeltaSeconds);
    }
    const EPinkCabVehicleMotionMode MotionMode = MotionClassifier.GetMode();
    LaunchController.NotifyMotionMode(MotionMode);

    const bool bClutchHeld = InputFrame.Clutch > 0.5f;
    const bool bBrakeHeld = InputFrame.Brake > 0.5f;
    const bool bThrottleHeld = InputFrame.Throttle > 0.5f;
    const bool bReturnedToStationary =
        PreviousMotionMode == EPinkCabVehicleMotionMode::Moving
        && MotionMode == EPinkCabVehicleMotionMode::Stationary;
    if (MotionMode == EPinkCabVehicleMotionMode::Stationary
        && ((bThrottleHeld && !bThrottleHeldLastFrame)
            || (bReturnedToStationary && bThrottleHeld)))
    {
        if (LaunchController.BeginLaunchAttempt())
        {
            SmoothedThrottle = 0.0f;
        }
    }

    const auto IsActionHeld = [this, PC](const EPinkCabSemanticAction Action)
    {
        const FKey Key = InputRouter.GetKeyForAction(Action);
        return Key.IsValid() && PC->IsInputKeyDown(Key);
    };

    const FKey WheelKey = InputRouter.GetKeyForAction(EPinkCabSemanticAction::Wheel);
    const float WheelAxis = WheelKey.IsValid() ? PC->GetInputAnalogKeyState(WheelKey) : 0.0f;
    const int32 WheelSteps = WheelAxis > 0.0f ? 1 : (WheelAxis < 0.0f ? -1 : 0);
    const EPinkCabPedalWheelRecipient WheelRecipient =
        PedalDosingController.ApplyWheelSteps(
            bClutchHeld,
            bBrakeHeld,
            bThrottleHeld,
            WheelSteps,
            LaunchController,
            CockpitState);

    const FPinkCabPedalTargets PedalTargets =
        PedalDosingController.ResolveTargets(
            bBrakeHeld,
            bThrottleHeld,
            LaunchController.GetThrottleTarget());
    InputFrame.Brake = PedalTargets.Brake;
    InputFrame.Throttle = PedalTargets.Throttle;

    SmoothedClutch = FPinkCabVehicleInputResponse::StepAxis(
        SmoothedClutch,
        InputFrame.Clutch,
        DeltaSeconds,
        ClutchPressSeconds,
        CockpitState.GetClutchReleaseSeconds());
    SmoothedBrake = FPinkCabVehicleInputResponse::StepAxis(
        SmoothedBrake,
        InputFrame.Brake,
        DeltaSeconds,
        BrakePressSeconds,
        BrakeReleaseSeconds);
    SmoothedThrottle = FPinkCabVehicleInputResponse::StepAxis(
        SmoothedThrottle,
        InputFrame.Throttle,
        DeltaSeconds,
        ThrottlePressSeconds,
        ThrottleReleaseSeconds);
    InputFrame.Clutch = SmoothedClutch;
    InputFrame.Brake = SmoothedBrake;
    InputFrame.Throttle = SmoothedThrottle;

    FPinkCabCockpitInteractionFrame InteractionFrame;
    InteractionFrame.bGazeHeld = InputFrame.bGazeHeld;
    InteractionFrame.bQuickRecall1Held = IsActionHeld(EPinkCabSemanticAction::QuickRecall1);
    InteractionFrame.bQuickRecall2Held = IsActionHeld(EPinkCabSemanticAction::QuickRecall2);
    InteractionFrame.bQuickRecall3Held = IsActionHeld(EPinkCabSemanticAction::QuickRecall3);
    InteractionFrame.bQuickRecall4Held = IsActionHeld(EPinkCabSemanticAction::QuickRecall4);
    InteractionFrame.bGripHeld = IsActionHeld(EPinkCabSemanticAction::Grip);
    InteractionFrame.bMomentaryHeld = IsActionHeld(EPinkCabSemanticAction::MomentaryPress);
    InteractionFrame.WheelSteps =
        WheelRecipient == EPinkCabPedalWheelRecipient::None ? WheelSteps : 0;
    InteractionFrame.NowSeconds = FPlatformTime::Seconds();
    if (DriverCamera)
    {
        InteractionFrame.GazeOrigin = DriverCamera->GetComponentLocation();
        InteractionFrame.GazeForward = DriverCamera->GetForwardVector();
    }

    TArray<FPinkCabInteractionEvent> InteractionEvents;
    CockpitInteraction->ProcessFrame(InteractionFrame, CockpitAssembly, InteractionEvents);
    for (const FPinkCabInteractionEvent& InteractionEvent : InteractionEvents)
    {
        ApplyCockpitInteraction(InteractionEvent);
    }

    const FName ActiveGripTarget = CockpitInteraction->GetActiveGripTargetId();
    const bool bHandbrakeGripActive =
        CockpitInteraction->IsGripActive()
        && ActiveGripTarget == FName(TEXT("Handbrake"));
    const bool bGearboxGripActive =
        CockpitInteraction->IsGripActive()
        && ActiveGripTarget == FName(TEXT("Gearbox"));
    const bool bPhysicalGripActive = bHandbrakeGripActive || bGearboxGripActive;

    SetGearboxPointerCapture(PC, bPhysicalGripActive);
    ApplyPhysicalControlMouseDelta(
        ActiveGripTarget,
        CockpitInteraction->IsGripActive(),
        MouseX,
        MouseY,
        DeltaSeconds);

    bGearLeverDragging = bGearboxGripActive;
    GearLeverCursor = bGearLeverDragging
        ? FVector2D(GearboxController.GetLeverX(), GearboxController.GetLeverY())
        : UPinkCabCockpitVisualDriverComponent::GearCursorForGear(
            CockpitState.GetSelectedGear());

    ApplyVehicleInputFrame(
        InputFrame,
        bPhysicalGripActive ? 0.0f : SteeringMouseX,
        DeltaSeconds);
    bThrottleHeldLastFrame = bThrottleHeld;

    const bool bGazeHeld =
        CockpitInteraction->IsGazeHeld() && !bPhysicalGripActive;
    SmoothedLookMouseX = FMath::FInterpTo(
        SmoothedLookMouseX,
        bGazeHeld ? MouseX : 0.0f,
        DeltaSeconds,
        12.0f);
    SmoothedLookMouseY = FMath::FInterpTo(
        SmoothedLookMouseY,
        bGazeHeld ? MouseY : 0.0f,
        DeltaSeconds,
        12.0f);
    if (bGazeHeld)
    {
        LookYaw = FMath::Clamp(
            LookYaw + SmoothedLookMouseX * 0.58f,
            -130.0f,
            130.0f);
        LookPitch = FMath::Clamp(
            LookPitch + SmoothedLookMouseY * 0.46f,
            -58.0f,
            48.0f);
    }
    else
    {
        LookYaw = FMath::FInterpTo(LookYaw, 0.0f, DeltaSeconds, 3.2f);
        LookPitch = FMath::FInterpTo(LookPitch, 0.0f, DeltaSeconds, 3.2f);
    }
    DriverHeadRoot->SetRelativeRotation(FRotator(LookPitch, LookYaw, 0.0f));

    FPinkCabCockpitPresentationState Presentation;
    // The steering controller is already filtered for road feel. The visible
    // wheel must mirror that command exactly so its direction is immediately readable.
    VisualSteering = ControlState.Steering;
    Presentation.Steering = VisualSteering;
    Presentation.Clutch = ControlState.Clutch;
    Presentation.Brake = ControlState.Brake;
    Presentation.Throttle = ControlState.Throttle;
    Presentation.SelectedGear = CockpitState.GetSelectedGear();
    Presentation.bGearLeverDragging = bGearLeverDragging;
    Presentation.GearLeverCursor = GearLeverCursor;
    Presentation.bIgnitionRunning =
        CockpitState.GetIgnitionState() == EPinkCabIgnitionState::Running;
    Presentation.Handbrake = CockpitState.GetHandbrakeAmount();
    Presentation.bHandbrakeEngaged = CockpitState.IsHandbrakeEngaged();

    FPinkCabCockpitServiceSources ServiceSources;
    ServiceSources.Taximeter = CockpitTaximeterSource;
    ServiceSources.CockpitState = &CockpitState;
    ServiceSources.RouteProgress01 = CockpitRouteProgress01;
    ServiceSources.bRadioAvailable = bCockpitRadioAvailable;
    ServiceSources.bMirrorsAvailable = bCockpitMirrorsAvailable;
    FPinkCabCockpitServiceBridge::ApplyToPresentation(
        FPinkCabCockpitServiceBridge::Read(ServiceSources),
        Presentation);
    Presentation.bTurnSignalLeft = CockpitState.GetTurnSignalDirection() < 0;
    Presentation.bTurnSignalRight = CockpitState.GetTurnSignalDirection() > 0;
    Presentation.bHornActive = CockpitState.IsHornActive();
    Presentation.bLightsOn = CockpitState.GetLightMode() > 0;
    Presentation.bWipersOn = CockpitState.GetWiperMode() > 0;
    Presentation.bWasherActive = CockpitState.IsWasherActive();

    FPinkCabVehicleTelemetry Telemetry;
    if (DynamicsProvider.ReadTelemetry(Telemetry))
    {
        Presentation.SpeedKmh = Telemetry.SpeedKmh;
        Presentation.EngineRpm = Telemetry.EngineRpm;
    }

    const float Rpm01 =
        FMath::Clamp(Presentation.EngineRpm / 7000.0f, 0.0f, 1.0f);
    const float TemperatureTarget = Presentation.bIgnitionRunning
        ? FMath::Lerp(0.62f, 0.90f, Rpm01)
        : 0.12f;
    EngineTemperature01 = FMath::FInterpTo(
        EngineTemperature01,
        TemperatureTarget,
        DeltaSeconds,
        Presentation.bIgnitionRunning ? 0.08f : 0.03f);
    Presentation.EngineTemperature01 = EngineTemperature01;
    Presentation.Fuel01 = TatraProfile.FullFuelMassKg > KINDA_SMALL_NUMBER
        ? FMath::Clamp(
            VehicleLoadState.GetFuelMassKg() / TatraProfile.FullFuelMassKg,
            0.0f,
            1.0f)
        : 0.0f;

    CockpitVisualDriver->Apply(*CockpitAssembly, Presentation);
}

UChaosWheeledVehicleMovementComponent* APinkCabChaosTatraPawn::GetChaosMovement() const
{
    return Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent());
}
