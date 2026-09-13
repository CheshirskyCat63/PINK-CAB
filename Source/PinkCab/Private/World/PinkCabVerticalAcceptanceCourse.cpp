#include "World/PinkCabVerticalAcceptanceCourse.h"

#include "Components/DirectionalLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "World/PinkCabChunkId.h"
#include "World/PinkCabGreyboxAuthorityTag.h"

namespace PinkCabVerticalAcceptancePrivate
{
    EPinkCabVerticalContactKind KindAt(const int32 Index)
    {
        switch (Index)
        {
        case 0: return EPinkCabVerticalContactKind::WallLeft;
        case 1: return EPinkCabVerticalContactKind::FreightCeiling;
        case 2: return EPinkCabVerticalContactKind::PoplarGapHook;
        default: return EPinkCabVerticalContactKind::ReceivingStrip;
        }
    }

    const TCHAR* SemanticAt(const int32 Index)
    {
        static const TCHAR* Keys[] = {TEXT("wall"), TEXT("freight"), TEXT("gap"), TEXT("receive")};
        return Keys[FMath::Clamp(Index, 0, 3)];
    }
}

APinkCabVerticalAcceptanceCourse::APinkCabVerticalAcceptanceCourse()
{
    PrimaryActorTick.bCanEverTick = false;
    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(SceneRoot);
    Tags.Add(FName(TEXT("NON_AUTHORITATIVE_GEOMETRY")));
    Tags.Add(FName(TEXT("PINKCAB_VERTICAL_ACCEPTANCE")));

    KeyLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("KeyLight"));
    KeyLight->SetupAttachment(SceneRoot);
    KeyLight->SetRelativeRotation(FRotator(-50.0f, -30.0f, 0.0f));
    KeyLight->SetIntensity(4.0f);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
    UStaticMesh* Cube = CubeFinder.Succeeded() ? CubeFinder.Object : nullptr;

    const FVector L1Locations[] = {
        FVector(-3000.0f, -900.0f, 300.0f),
        FVector(-1000.0f, -900.0f, 700.0f),
        FVector(1000.0f, -900.0f, 250.0f),
        FVector(3000.0f, -900.0f, 50.0f)};
    const FVector L1Scales[] = {
        FVector(12.0f, 0.3f, 6.0f),
        FVector(12.0f, 5.0f, 0.3f),
        FVector(2.0f, 2.0f, 5.0f),
        FVector(4.0f, 5.0f, 0.2f)};

    for (int32 Index = 0; Index < 4; ++Index)
    {
        const FName Name(*FString::Printf(TEXT("L1_Primitive_%d"), Index));
        UStaticMeshComponent* Primitive = CreateDefaultSubobject<UStaticMeshComponent>(Name);
        Primitive->SetupAttachment(SceneRoot);
        Primitive->SetStaticMesh(Cube);
        Primitive->SetRelativeLocation(L1Locations[Index]);
        Primitive->SetRelativeScale3D(L1Scales[Index]);
        Primitive->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        Primitive->SetCastShadow(false);
        Primitive->ComponentTags.Add(FName(*GetPresentationLaneId(Index).Serialize()));
        Primitive->ComponentTags.Add(FName(*GetPresentationContactId(Index)));
        L1Primitives.Add(Primitive);
    }

    const FVector L2Locations[] = {
        FVector(-2500.0f, 1400.0f, 450.0f),
        FVector(-500.0f, 1400.0f, 650.0f),
        FVector(1500.0f, 1400.0f, 250.0f),
        FVector(3500.0f, 1400.0f, 150.0f)};
    const FVector L2Scales[] = {
        FVector(15.0f, 3.0f, 0.3f),
        FVector(2.0f, 3.0f, 3.0f),
        FVector(15.0f, 2.0f, 0.2f),
        FVector(4.0f, 4.0f, 0.3f)};

    static const TCHAR* L2Tags[] = {
        TEXT("L2_BUS_ROUTE"), TEXT("L2_BUS_CONTACT"),
        TEXT("L2_METRO_ROUTE"), TEXT("L2_METRO_STATION")};
    for (int32 Index = 0; Index < 4; ++Index)
    {
        const FName Name(*FString::Printf(TEXT("L2_Primitive_%d"), Index));
        UStaticMeshComponent* Primitive = CreateDefaultSubobject<UStaticMeshComponent>(Name);
        Primitive->SetupAttachment(SceneRoot);
        Primitive->SetStaticMesh(Cube);
        Primitive->SetRelativeLocation(L2Locations[Index]);
        Primitive->SetRelativeScale3D(L2Scales[Index]);
        Primitive->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        Primitive->SetCastShadow(false);
        Primitive->ComponentTags.Add(FName(L2Tags[Index]));
        L2Primitives.Add(Primitive);
    }
}

FPinkCabCityIdentity APinkCabVerticalAcceptanceCourse::GetPresentationCityIdentity()
{
    return FPinkCabCityIdentity::Create(
        TEXT("VERTICAL-ACCEPTANCE"), TEXT("dev-vertical-gen-v1"), TEXT("builtin-primitives-v1"));
}

FPinkCabLaneId APinkCabVerticalAcceptanceCourse::GetPresentationLaneId(const int32 Index)
{
    if (Index < 0 || Index >= 4)
    {
        return FPinkCabLaneId();
    }
    const FPinkCabCityIdentity City = GetPresentationCityIdentity();
    const FPinkCabChunkId Chunk = FPinkCabChunkId::From(City, {0, 0, 1});
    return FPinkCabRoadGraph::MakeLaneId(City, Chunk, 0, Index);
}

FString APinkCabVerticalAcceptanceCourse::GetPresentationContactId(const int32 Index)
{
    const FPinkCabLaneId LaneId = GetPresentationLaneId(Index);
    if (!LaneId.IsValid())
    {
        return FString();
    }
    return FPinkCabVerticalContactRegistry::MakeContactId(
        GetPresentationCityIdentity(), LaneId,
        PinkCabVerticalAcceptancePrivate::KindAt(Index),
        PinkCabVerticalAcceptancePrivate::SemanticAt(Index));
}

FString APinkCabVerticalAcceptanceCourse::GetPresentedContactId(const int32 Index) const
{
    return GetPresentationContactId(Index);
}

bool APinkCabVerticalAcceptanceCourse::HasPrimitiveForContact(const FString& ContactId) const
{
    for (int32 Index = 0; Index < L1Primitives.Num(); ++Index)
    {
        if (L1Primitives[Index] && GetPresentationContactId(Index) == ContactId)
        {
            return true;
        }
    }
    return false;
}

FString APinkCabVerticalAcceptanceCourse::GetAuthorityMarker() const
{
    return FPinkCabGreyboxAuthorityTag::Marker();
}
