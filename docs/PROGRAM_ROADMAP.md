# PINK CAB · FIRST EURO Execution Roadmap

**Status:** CURRENT EXECUTION MIRROR  
**Product root:** `CD-519`  
**Mechanics Freeze owner:** `CD-848`  
**Scope owner:** `CD-753`  
**Canonical branch:** `main`

## Execution rule

PINK-CAB is no longer in recovery/bootstrap mode. New implementation follows:

`main → one short-lived branch → PR → verify.yml → merge → optional exact-head deliver.yml → owner verdict`

No parallel recovery lane, second vehicle solver, or feature-specific CI ecosystem is allowed.

## Current execution checkpoint

The simplified stock-Chaos P4 baseline is complete. The owner accepted the P4 + RIG06 playable build on 2026-10-07; immutable fallback tag: `accepted/p4-rig06-20261007` at `6edea7747d3a8433188c9fb394b98ae9c320d49b`.

The immediate execution lane is **Vehicle Feel 90**. It calibrates the already accepted hero-car stack toward roughly 80–90% of intended FIRST EURO driving behavior without mixing in world, taxi, traffic, economy, Neural or ServiceNode work. P4 acceptance remains closed history; failed feel candidates roll back to the accepted tag rather than accumulating compensating hacks.

Vehicle Feel 90 closes in this order: measurements/evidence → steering/input → engine/clutch/gearbox → mass/CoM/inertia → suspension/body control → dry tyres/brakes → wet/storm → load envelope → runtime robustness → one exact-head owner acceptance. The executable contract is mirrored in `docs/PINK_CAB_VEHICLE_FEEL_90.md`.

Only after Vehicle Feel 90 owner acceptance does the broader FIRST EURO queue below resume at R01.

Active GitHub control is only `verify.yml` and `deliver.yml`. Historical CD-648/CD-869 workflows and custom driveline research remain archived evidence, not parallel production architecture.

## Production technology

- Unreal Engine 5.8 / current installed 5.8.3 line.
- Native Chaos Vehicles behind the PINK-CAB dynamics provider is the sole hero-car road-dynamics owner.
- Vehicle Health + bounded authored/native damage owns functional damage consequences.
- FGear/VDS material is archived research only.
- Presentation/model geometry cannot own physics, control, persistence or economy state.

## Broader finite Mechanics Freeze queue (not the immediate next physics task)

### R01 · CD-869 — L1 → L2 → L1 route + streaming closure
One representative forward/return route; deterministic CityCode reconstruction; bounded active/recent chunks; no voids, duplicate persistent deltas or forced reset.

### R02 · CD-870 — Pickup/dropoff + parking acceptance
Close official-stop/curb/full-stop eligibility, physical passenger exchange and parking acceptance as one player-visible loop.

### R03 · CD-871 — Garage / parts / repair transaction closure
One consistent automotive ServiceNode transaction path using existing Economy, Vehicle Health and persistence owners.

### R04 · CD-872 — Moving refuel loop closure
Bounded moving-refuel session over shared FuelTank/Economy/VehicleTelemetry/RoadGraph with exactly-once settlement.

### R05 · CD-873 — Wet-weather driving contract closure
Verify wet/storm surface behavior remains recoverable, manual and consistent with the accepted no-assist steering/Chaos contract.

### R06 · CD-874 — Bounded traffic + incident population closure
Bound logical/physical traffic materialization, incidents and persistence without unbounded simulation or standing-jam assumptions.

### R07 · CD-875 — Repeat-client identity + persistence closure
Stable PassengerIdentity, history/contact eligibility and repeat-order continuity through save/reload.

### R08 · CD-876 — Daughter FIRST EURO bounded role closure
Close only the first-year daughter role required by current product scope; do not expand into post-year lifestyle/social systems.

### R09 · CD-877 — Minimal Neural profile/contact/thread closure
First-year bounded Neural: persistent profile/contact/message/repeat-client surface without broad social-sim ownership.

### R10 · CD-878 — Payment receipt + fines projection closure
Close payment/receipt/fines player-facing projection on top of existing exactly-once transaction truth.

### R11 · CD-879 — terminal FIRST EURO Mechanics Freeze evidence audit
Reconcile all included/excluded rows, run full exact-candidate automation/package/runtime evidence, perform one consolidated human pass, and close `CD-848` only if no included row remains open.

## Vehicle/model lane

`CD-856` pre-model handoff is DONE. CD-855 placeholder polish/replacement is deferred until vehicle calibration; only test-blocking placeholder defects justify immediate work. Import and physics/presentation ownership contracts remain binding.

The donor Tatra commercial modification/redistribution permission is not proven. It is an asset-release blocker only; internal engineering may continue with the accepted working presentation until CD-855 records permission or replaces the donor.

## Delivery / verification lanes

Normal verification is `.github/workflows/verify.yml`: hosted static contracts first, then an exact-head `PinkCabEditor` build and the focused playable runtime suite on the PINKCAB self-hosted runner.

Human delivery is `.github/workflows/deliver.yml`. Its single entry point is `scripts/deliver.ps1`, which requires a clean exact HEAD, builds the editor, runs the focused runtime suite, packages with BuildCookRun, smoke-tests the package, and only then atomically publishes `PINCKCAB_BUILD` plus the `PINCKCAB.lnk` shortcut.

The old CD-648/CD-869/P00–P04 workflow graph, causality evidence programs, calibration fixtures and road-authoring gates are historical evidence only. They are not active build, verification or delivery authority.

Documentation-only administration does not require replacing the runtime package. PINK CAB and KUKURUZA registrations share one physical host; coordinate heavy Unreal work without stopping another project's processes.

## Post-FIRST-EURO

Explicitly outside this queue: multiplayer/coop/common rooms, Level 3 gameplay, lifestyle/social ServiceNodes, full Taxi Regulator, daily insurance purchase/claim system and broad online/social expansion.
