#include "World/PinkCabChaosWeaveCourse.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "UObject/ConstructorHelpers.h"

namespace PinkCabChaosWeavePrivate
{
    constexpr float FirstObstacleX = -7000.0f;
    constexpr float ObstacleSideOffsetY = 250.0f;
    const FVector ObstacleSizeCm(480.0f, 380.0f, 150.0f);
}

APinkCabChaosWeaveCourse::APinkCabChaosWeaveCourse()
{
    PrimaryActorTick.bCanEverTick = false;
    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(SceneRoot);

    KeyLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("KeyLight"));
    KeyLight->SetupAttachment(SceneRoot);
    KeyLight->SetRelativeRotation(FRotator(-45.0f, -35.0f, 0.0f));
    KeyLight->SetIntensity(5.0f);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(
        TEXT("/Engine/BasicShapes/Cube.Cube"));
    UStaticMesh* Cube = CubeFinder.Succeeded() ? CubeFinder.Object : nullptr;

    RoadSurface = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RoadSurface"));
    RoadSurface->SetupAttachment(SceneRoot);
    RoadSurface->SetStaticMesh(Cube);
    const FVector CourseSize = GetCourseSizeCm();
    RoadSurface->SetRelativeLocation(FVector(0.0f, 0.0f, -CourseSize.Z * 0.5f));
    RoadSurface->SetRelativeScale3D(CourseSize / 100.0f);
    RoadSurface->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    RoadSurface->SetCastShadow(false);

    for (int32 Index = 0; Index < GetObstacleCount(); ++Index)
    {
        const FName Name(*FString::Printf(TEXT("VehicleBlock_%02d"), Index));
        UStaticMeshComponent* Block = CreateDefaultSubobject<UStaticMeshComponent>(Name);
        Block->SetupAttachment(SceneRoot);
        Block->SetStaticMesh(Cube);
        Block->SetRelativeLocation(GetObstacleLocation(Index));
        Block->SetRelativeScale3D(PinkCabChaosWeavePrivate::ObstacleSizeCm / 100.0f);
        Block->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        Block->SetCastShadow(false);
        ObstacleBlocks.Add(Block);
    }
}

FVector APinkCabChaosWeaveCourse::GetObstacleLocation(const int32 Index)
{
    const int32 SafeIndex = FMath::Clamp(Index, 0, GetObstacleCount() - 1);
    const float X = PinkCabChaosWeavePrivate::FirstObstacleX
        + SafeIndex * GetLongitudinalGapCm();
    const float Side = (SafeIndex % 2 == 0) ? -1.0f : 1.0f;
    return FVector(
        X,
        Side * PinkCabChaosWeavePrivate::ObstacleSideOffsetY,
        PinkCabChaosWeavePrivate::ObstacleSizeCm.Z * 0.5f);
}

FTransform APinkCabChaosWeaveCourse::GetPawnSpawnTransform()
{
    return FTransform(FRotator::ZeroRotator, FVector(-13000.0f, 0.0f, 180.0f));
}
