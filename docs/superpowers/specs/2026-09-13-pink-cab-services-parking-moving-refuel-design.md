# PINK CAB Services / Parking / Moving Refuel Design

**Jira:** CD-799 · PC-CORE-06 · 10%

**Goal:** complete the code-only automotive service runtime without creating a second owner for money, fuel, vehicle build, health, city identity, or road state.

## Architecture

`FPinkCabServiceNode` remains a thin lifecycle boundary for Parking, Garage/Tuning, Parts and Repair/Service. It may coordinate eligibility and commit phases, but it never directly owns settlement truth.

The existing owners remain authoritative: `FPinkCabEconomyLedger` for money, `FPinkCabVehicleBuild` for installed parts, `FPinkCabVehicleHealthState` for health, `FPinkCabFuelTank` for fuel, `FPinkCabRoadGraph` for lanes and `FPinkCabVehicleTelemetry` for moving-refuel eligibility.

Moving refuel remains a separate live-road session, not a `ServiceNode`.
## Service identity and context

Each service node has a stable node id and explicit kind. Entering a node captures references/identity keys for the same owned Tatra, CityCode, VehicleBuild, VehicleHealth, inventory and economy context; exit must prove the same context resumes.

No service transition may silently replace these owners. Placeholder presentation actors may visualize state only.

## Inventory and parts

Add one bounded inventory/catalog layer with stable part ids, slot ids and deterministic compatibility constraints. Purchasing and installing are separate operations: purchase creates owned inventory only after an economy commit; install consumes an owned compatible part and updates `FPinkCabVehicleBuild` exactly once.

Invalid slot/part combinations, duplicates, capacity overflow and replayed operation ids fail deterministically without partial mutation.
## Parking, repair and service settlement

Parking/service charges and repairs use typed `FPinkCabEconomyTransaction` debits. Mutation after settlement is replay-safe: the same operation id cannot charge twice or apply a second repair/install side effect.

Repair continues to use `FPinkCabRepairService` and the same `FPinkCabVehicleHealthState`. Minimum-roadworthy repair may use the existing debt policy; discretionary tuning and ordinary purchases may not.

## Moving refuel

`FPinkCabMovingFuelSession` is extended into a bounded lifecycle with explicit connect, fueling, completed and aborted outcomes. Eligibility uses caller-supplied policy plus normalized telemetry, a valid RoadGraph service lane and longitudinal gap tolerance.

Tolerance loss, collision or explicit abort ends the live-road session without inventing fuel. A committed or duplicate economy settlement may credit the tank only through `FPinkCabFuelTank::CreditFuelOnce`.
## Snapshot boundary

CD-799 exposes logical snapshot/reconstruction records for service context, inventory/build operations and moving-refuel state so CD-801 can persist them later. CD-799 does not implement disk save/load and does not own workday recovery policy.

Reconstruction must preserve stable ids and exactly-once replay guards. Unknown or invalid snapshot state fails closed.

## Acceptance

A primitive-only integrated fixture must exercise Parking, Parts purchase/install, Repair/Service and moving refuel while preserving the same vehicle/city/build/health/economy identities.

Required cases: exactly-once purchase/install/repair/parking/fuel; deterministic invalid compatibility rejection; inventory bounds; moving-refuel success, explicit abort, tolerance loss and collision; replay after reconstruction converges without duplicate debit or duplicate fuel/part/repair effect.

Final gate: fresh UE 5.8.2 build, `PinkCab.Service`, affected Economy/Vehicle/World automation, clean diff checks and Jira evidence. Final models, materials, animations and decorative service spaces remain outside this gate.