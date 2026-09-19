#include "Runtime/PinkCabChaosTatraPawn.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Vehicle/PinkCabChaosLoadBridge.h"
#include "Vehicle/PinkCabVehicleStateSnapshot.h"

bool APinkCabChaosTatraPawn::SyncLoadToChaos()
{
    UChaosWheeledVehicleMovementComponent* Movement = GetChaosMovement();
    return Movement && FPinkCabChaosLoadBridge::Apply(VehicleLoadState, TatraProfile, *Movement);
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

bool APinkCabChaosTatraPawn::CaptureVehicleSnapshot(FPinkCabVehicleStateSnapshot& OutSnapshot) const
{
    return FPinkCabVehicleStateSnapshotCodec::Capture(GetVehicleHealthState(), VehicleLoadState, OutSnapshot);
}

bool APinkCabChaosTatraPawn::RestoreVehicleSnapshot(const FPinkCabVehicleStateSnapshot& Snapshot)
{
    FPinkCabVehicleHealthState RestoredHealth;
    FPinkCabVehicleLoadState RestoredLoad;
    if (!FPinkCabVehicleStateSnapshotCodec::Restore(Snapshot, RestoredHealth, RestoredLoad)) return false;

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
    SyncCockpitToChaos();
    return true;
}
