#include "Vehicle/PinkCabTatraPhysicsAssetCommandlet.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Dom/JsonObject.h"
#include "Engine/SkeletalMesh.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/SkeletalBodySetup.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"

namespace
{
const TCHAR* PhysicalMeshPath = TEXT("/Game/Dev/Vehicles/Tatra613Physics/V1/SK_Tatra613_Physical.SK_Tatra613_Physical");
const TCHAR* PhysicsPackage = TEXT("/Game/Dev/Vehicles/Tatra613Physics/V1/PA_Tatra613_Physical");

bool ReadVector(const TSharedPtr<FJsonValue>& Value, FVector& Out)
{
    const TArray<TSharedPtr<FJsonValue>>* Values = nullptr;
    if (!Value.IsValid() || !Value->TryGetArray(Values) || Values->Num() != 3) return false;
    double X, Y, Z;
    if (!(*Values)[0]->TryGetNumber(X) || !(*Values)[1]->TryGetNumber(Y)
        || !(*Values)[2]->TryGetNumber(Z)) return false;
    Out = FVector(X, Y, Z);
    return !Out.ContainsNaN() && Out.GetAbsMax() < 500.0;
}

bool ValidateRig(USkeletalMesh& Mesh, const FJsonObject& Recipe)
{
    const TSharedPtr<FJsonObject>* Wheels = nullptr;
    if (!Recipe.TryGetObjectField(TEXT("wheel_bones_cm"), Wheels)
        || (*Wheels)->Values.Num() != 4 || Mesh.GetRefSkeleton().GetNum() != 5) return false;
    bool bValid = true;
    for (const auto& Pair : (*Wheels)->Values)
    {
        FVector Expected;
        const FName Bone(*Pair.Key);
        if (!ReadVector(Pair.Value, Expected) || Mesh.GetRefSkeleton().FindBoneIndex(Bone) == INDEX_NONE) return false;
        const FVector Actual = Mesh.GetComposedRefPoseMatrix(Bone).GetOrigin();
        UE_LOG(LogTemp, Display, TEXT("T5_AUTHORED_BONE name=%s actual=%s expected=%s"),
            *Pair.Key, *Actual.ToString(), *Expected.ToString());
        bValid &= Actual.Equals(Expected, 0.1);
    }
    UE_LOG(LogTemp, Display, TEXT("T5_AUTHORED_ROOT %s"),
        *Mesh.GetRefSkeleton().GetRefBonePose()[0].ToString());
    return bValid && Mesh.GetRefSkeleton().GetBoneName(0) == FName(TEXT("Root"))
        && Mesh.GetRefSkeleton().GetRefBonePose()[0].Equals(FTransform::Identity, 0.001);
}

bool AddHulls(USkeletalBodySetup& Body, const FJsonObject& Recipe)
{
    const TArray<TSharedPtr<FJsonValue>>* Hulls = nullptr;
    if (!Recipe.TryGetArrayField(TEXT("hulls"), Hulls) || Hulls->Num() != 2) return false;
    for (const TSharedPtr<FJsonValue>& HullValue : *Hulls)
    {
        const TSharedPtr<FJsonObject>* Hull = nullptr;
        const TArray<TSharedPtr<FJsonValue>>* Vertices = nullptr;
        if (!HullValue->TryGetObject(Hull) || !(*Hull)->TryGetArrayField(TEXT("vertices_cm"), Vertices)
            || Vertices->Num() < 4 || Vertices->Num() > 1024) return false;
        FKConvexElem Convex;
        for (const TSharedPtr<FJsonValue>& Vertex : *Vertices)
        {
            FVector Point;
            if (!ReadVector(Vertex, Point)) return false;
            Convex.VertexData.Add(Point);
        }
        Convex.UpdateElemBox();
        Body.AggGeom.ConvexElems.Add(MoveTemp(Convex));
    }
    return true;
}

bool SaveAsset(UObject& Asset)
{
    UPackage* Package = Asset.GetOutermost();
    const FString Filename = FPackageName::LongPackageNameToFilename(
        Package->GetName(), FPackageName::GetAssetPackageExtension());
    IFileManager::Get().MakeDirectory(*FPaths::GetPath(Filename), true);
    FSavePackageArgs Args;
    Args.TopLevelFlags = RF_Public | RF_Standalone;
    Args.SaveFlags = SAVE_NoError;
    return UPackage::SavePackage(Package, &Asset, *Filename, Args);
}
}

UPinkCabTatraPhysicsAssetCommandlet::UPinkCabTatraPhysicsAssetCommandlet()
{
    IsClient = false;
    IsServer = false;
    IsEditor = true;
    LogToConsole = true;
}

int32 UPinkCabTatraPhysicsAssetCommandlet::Main(const FString& Params)
{
    FString RecipePath, Json;
    TSharedPtr<FJsonObject> Recipe;
    if (!FParse::Value(*Params, TEXT("Recipe="), RecipePath)
        || !FFileHelper::LoadFileToString(Json, *RecipePath)
        || !FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), Recipe)
        || !Recipe.IsValid()) return 1;
    double Schema;
    FString Frame;
    if (!Recipe->TryGetNumberField(TEXT("schema"), Schema) || Schema != 1.0
        || !Recipe->TryGetStringField(TEXT("coordinate_frame"), Frame)
        || Frame != TEXT("accepted_chassis_cm")) return 2;
    if (FPackageName::DoesPackageExist(PhysicsPackage))
    {
        UE_LOG(LogTemp, Error, TEXT("Physical asset exists; authoring refuses silent overwrite"));
        return 3;
    }
    USkeletalMesh* Mesh = LoadObject<USkeletalMesh>(nullptr, PhysicalMeshPath);
    if (!Mesh || !ValidateRig(*Mesh, *Recipe))
    {
        UE_LOG(LogTemp, Error, TEXT("Physical rig axes/units/rest positions disagree with recipe"));
        return 4;
    }
    UPackage* Package = CreatePackage(PhysicsPackage);
    UPhysicsAsset* Asset = NewObject<UPhysicsAsset>(Package, TEXT("PA_Tatra613_Physical"), RF_Public | RF_Standalone);
    USkeletalBodySetup* Body = NewObject<USkeletalBodySetup>(Asset, TEXT("Tatra613Root"), RF_Transactional);
    Body->BoneName = TEXT("Root");
    Body->PhysicsType = PhysType_Simulated;
    Body->bConsiderForBounds = true;
    Body->CollisionTraceFlag = CTF_UseSimpleAsComplex;
    Body->DefaultInstance.SetCollisionProfileName(TEXT("Vehicle"));
    Body->DefaultInstance.SetMassOverride(1450.0f, true);
    if (!AddHulls(*Body, *Recipe)) return 5;
    Body->InvalidatePhysicsData();
    Body->CreatePhysicsMeshes();
    if (Body->bFailedToCreatePhysicsMeshes) return 6;
    Asset->SkeletalBodySetups.Add(Body);
    Asset->PreviewSkeletalMesh = Mesh;
    Asset->UpdateBodySetupIndexMap();
    Asset->UpdateBoundsBodiesArray();
    FAssetRegistryModule::AssetCreated(Asset);
    Asset->MarkPackageDirty();
    Mesh->SetPhysicsAsset(Asset);
    Mesh->MarkPackageDirty();
    if (!SaveAsset(*Asset) || !SaveAsset(*Mesh)) return 7;
    UE_LOG(LogTemp, Display, TEXT("TATRA_PHYSICAL_ASSET_AUTHORED=PASS bodies=%d hulls=%d constraints=%d"),
        Asset->SkeletalBodySetups.Num(), Body->AggGeom.ConvexElems.Num(), Asset->ConstraintSetup.Num());
    return 0;
}
