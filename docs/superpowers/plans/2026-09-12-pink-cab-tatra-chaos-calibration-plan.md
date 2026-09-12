# PINK CAB Tatra Native Chaos Calibration Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace scattered CD-786 vehicle seeds with one versioned, provenance-tagged native Chaos physical profile for the PINK CAB Tatra baseline.

**Architecture:** `FPinkCabChaosPhysicalProfile` is the single source for physical configuration consumed by the Chaos pawn and wheel classes. Every configured field carries `SOURCE`, `DESIGN_TARGET`, or `CALIBRATION` provenance; tuning-owned values expose Low/Nominal/High variants while locked values remain invariant.

**Tech Stack:** Unreal Engine 5.8.2, C++20, Chaos Vehicles, Unreal Automation Tests.

**Spec:** `docs/PINK_CAB_TATRA_HANDLING_E34_REFERENCE.md`

## Global Constraints

- Chaos Vehicles remains the sole road-dynamics owner.
- `StabilizeControl`, `TargetRotationControl`, and `TorqueControl` remain OFF.
- No third-party vehicle/damage dependency or purchase.
- Do not invent historical provenance: unverified runtime seeds are `CALIBRATION`.
- 1450 / 1550 / 1657 / 2107 kg mass fixtures remain product authority.
- Rear-engine identity, RWD, no ABS/ESP, 180 hp / 240 Nm / 195 km/h remain product targets.
- Historical geometry baseline stays ~2750 mm wheelbase / ~1485 mm front track / ~1400 mm rear track.
- CD-788 owns measured handling acceptance; CD-789 owns any bounded arcade assists.

---
### Task 1: Provenance-tagged physical profile and calibration matrix

**Files:**
- Create: `Source/PinkCab/Public/Vehicle/PinkCabChaosPhysicalProfile.h`
- Create: `Source/PinkCab/Private/Vehicle/PinkCabChaosPhysicalProfile.cpp`
- Create: `docs/qa/PINK_CAB_TATRA_CHAOS_CALIBRATION_MATRIX.md`
- Test: `Source/PinkCabTests/Private/Vehicle/PinkCabChaosPhysicalProfileTests.cpp`

**Interfaces:**
- Produces: `EPinkCabPhysicalParameterAuthority`, `TPinkCabPhysicalParameter<T>`, `EPinkCabCalibrationVariant`, `FPinkCabChaosPhysicalProfile::ForVariant(...)`.
- Consumes: `FPinkCabTatraProfile::Canonical()` mass fixtures and locked handling authority docs.

- [ ] **Step 1: Write failing profile tests** requiring all exposed parameters to have non-`Unspecified` authority and checking locked mass / power / geometry / steering / brake targets.
- [ ] **Step 2: Run** `PinkCab.Vehicle.ChaosCalibration.Profile` and verify RED because the profile does not exist.
- [ ] **Step 3: Implement the minimal tagged profile** with explicit fields for mass, balance, geometry, engine, transmission, steering, brakes, tires, wheels, and suspension.
- [ ] **Step 4: Add Low/Nominal/High construction** changing only fields tagged `CALIBRATION`; SOURCE and DESIGN_TARGET fields must remain byte-for-byte equal across variants.
- [ ] **Step 5: Write the markdown matrix** with columns `Parameter | Low | Nominal | High | Authority | Evidence/Reason` and no untagged row.
- [ ] **Step 6: Run profile tests GREEN and commit** `feat(CD-787): add tagged Chaos physical profile`.

### Task 2: Apply drivetrain and steering profile to the pawn

**Files:**
- Modify: `Source/PinkCab/Private/Vehicle/PinkCabChaosTatraPawn.cpp`
- Test: `Source/PinkCabTests/Private/Vehicle/PinkCabChaosPawnTests.cpp`

**Interfaces:**
- Consumes: `FPinkCabChaosPhysicalProfile::ForVariant(EPinkCabCalibrationVariant::Nominal)`.
- Produces: Pawn configuration with no drivetrain/steering numeric literals except asset/camera/input presentation values.
- [ ] **Step 1: Extend failing Pawn.Config test** to compare mass, differential, peak torque, RPM, engine brake, torque-curve keys, gear ratios/final drive, and steering setup to the nominal profile.
- [ ] **Step 2: Run** `PinkCab.Vehicle.ChaosBaseline.Pawn.Config` and verify RED against the new profile contract.
- [ ] **Step 3: Add `ApplyToMovement(...)`** in the profile implementation and replace scattered CD-786 drivetrain/steering literals in the pawn constructor.
- [ ] **Step 4: Verify** `Pawn.Config` and the existing runtime `DriveSmoke` both remain GREEN with all assists OFF.
- [ ] **Step 5: Commit** `refactor(CD-787): apply nominal Chaos drivetrain profile`.

### Task 3: Apply wheel, tire, brake, and suspension profile

**Files:**
- Modify: `Source/PinkCab/Private/Vehicle/PinkCabChaosWheelFront.cpp`
- Modify: `Source/PinkCab/Private/Vehicle/PinkCabChaosWheelRear.cpp`
- Test: `Source/PinkCabTests/Private/Vehicle/PinkCabChaosPawnTests.cpp`

**Interfaces:**
- Consumes: nominal front/rear wheel subprofiles.
- Produces: explicit Chaos wheel setup for radius/width/mass, friction/cornering/slip/load, brakes/handbrake, spring/preload/damping/travel/rollbar.

- [ ] **Step 1: Expand WheelRoles into wheel-parameter RED tests** against nominal tagged profile values.
- [ ] **Step 2: Run the wheel tests RED** before modifying production wheel classes.
- [ ] **Step 3: Replace wheel constructor literals/default reliance** with profile fields. Preserve front steering / rear drive / rear handbrake ownership.
- [ ] **Step 4: Keep ABS/TCS disabled** for the baseline; do not add hidden stability forces.
- [ ] **Step 5: Run wheel tests + DriveSmoke GREEN** and commit `refactor(CD-787): apply Tatra wheel and suspension profile`.

### Task 4: Variant invariants and baseline verification

**Files:**
- Test: `Source/PinkCabTests/Private/Vehicle/PinkCabChaosPhysicalProfileTests.cpp`
- Modify: `docs/qa/PINK_CAB_TATRA_CHAOS_CALIBRATION_MATRIX.md`

- [ ] **Step 1: Add variant invariant tests** proving SOURCE/DESIGN_TARGET fields do not change among Low/Nominal/High.
- [ ] **Step 2: Add calibration-direction tests** proving Low/Nominal/High only move named tuning fields in documented directions.
- [ ] **Step 3: Run** `PinkCab.Vehicle.ChaosCalibration` and `PinkCab.Vehicle.ChaosBaseline` with separate automation reports.
- [ ] **Step 4: Run fresh `scripts/build.ps1`** and require exit 0.
- [ ] **Step 5: Record final test/build evidence in the matrix and Jira CD-787; only then transition CD-787 DONE.**

## Exact interface contract

```cpp
enum class EPinkCabPhysicalParameterAuthority : uint8
{
    Unspecified,
    Source,
    DesignTarget,
    Calibration
};

template <typename T>
struct TPinkCabPhysicalParameter
{
    T Value{};
    EPinkCabPhysicalParameterAuthority Authority = EPinkCabPhysicalParameterAuthority::Unspecified;
};

enum class EPinkCabCalibrationVariant : uint8 { Low, Nominal, High };

struct FPinkCabChaosPhysicalProfile
{
    static FPinkCabChaosPhysicalProfile ForVariant(EPinkCabCalibrationVariant Variant);
    bool HasCompleteProvenance() const;
    void ApplyToMovement(UChaosWheeledVehicleMovementComponent& Movement) const;
};
```

Automation commands used by this plan:
`Automation RunTests PinkCab.Vehicle.ChaosCalibration; Quit`
`Automation RunTests PinkCab.Vehicle.ChaosBaseline; Quit`
