# PINK CAB · FIRST EURO Execution Roadmap

**Status:** CURRENT EXECUTION MIRROR  
**Product root:** `CD-519`  
**Mechanics Freeze owner:** `CD-848`  
**Scope owner:** `CD-753`  
**Canonical branch:** `main`

## Execution rule

PINK-CAB is no longer in recovery/bootstrap mode. New implementation follows:

`main → one Jira-keyed task branch → focused FAST verification → PR/review → merge → human gate only where required`

No parallel recovery lane, second vehicle solver, or feature-specific CI ecosystem is allowed.

## Production technology

- Unreal Engine 5.8 / current installed 5.8.2 line.
- Native Chaos Vehicles behind the PINK-CAB dynamics provider is the sole hero-car road-dynamics owner.
- Vehicle Health + bounded authored/native damage owns functional damage consequences.
- FGear/VDS material is archived research only.
- Presentation/model geometry cannot own physics, control, persistence or economy state.

## Current finite Mechanics Freeze queue

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

`CD-856` pre-model handoff is DONE. `CD-855` is a separate presentation/provenance consumer and may proceed without reopening vehicle physics or input authority.

The donor Tatra commercial modification/redistribution permission is not proven. It is an asset-release blocker only; internal engineering may continue with the accepted working presentation until CD-855 records permission or replaces the donor.

## Delivery / verification lanes

- **fast**: everyday engineering gate; no full recook.
- **human_gate**: lightweight test delivery to `PINKCAB Latest.lnk`; technical PASS never implies owner acceptance.
- **release_gate**: full automation + fresh cook/package + packaged runtime; release-host signing/trust belongs here.

Build success never equals gameplay verification. Documentation never equals implementation.

## Post-FIRST-EURO

Explicitly outside this queue: multiplayer/coop/common rooms, Level 3 gameplay, lifestyle/social ServiceNodes, full Taxi Regulator, daily insurance purchase/claim system and broad online/social expansion.
