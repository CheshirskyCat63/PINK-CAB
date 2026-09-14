# PINK CAB CD-801 Persistence / Recovery / Workday Design

## Status

Approved architectural direction for CD-801. This spec closes the remaining Core persistence/recovery/workday ownership without expanding into post-Core Mechanics Freeze design.

## Goal

Provide versioned, bounded and atomic logical persistence for the already-approved Core owners so save/load, crash recovery, terminal recovery and workday rollover cannot duplicate or partially corrupt gameplay state.

CD-801 must persist enough state for representative active gameplay, including an active fare, payment boundary, passenger identity/Neural, economy, vehicle load/health/build, service/refuel, persistent world deltas and workday/session state.

## Non-goals

- No full Neural civic/payment/licence/fines design beyond the CD-800 substrate.
- No floor 2/3 mechanics, bar, shops, authored character expansion or new world rules.
- No event-sourcing rewrite.
- No disk/platform save UI ownership beyond the logical persistence contract required for Core.
- No free recovery/reset path that bypasses economy or vehicle consequences.
## Architecture

Use versioned subsystem snapshots plus one aggregate game snapshot.

Each authoritative owner remains responsible for its own logical state shape, validation and reconstruction. `FPinkCabGameSnapshot` composes those snapshots but does not reach through their invariants. `FPinkCabPersistenceService` owns only envelope serialization, schema/config/world compatibility, committed checkpoint history and recovery selection.

This keeps persistence from becoming a second gameplay authority.

### Aggregate owners

`FPinkCabGameSnapshot` contains:

- explicit `FPinkCabCityIdentity` (CityCode/generator/content identity) validated against the save header world versions;
- passenger registry snapshot from CD-800;
- economy ledger plus fare-settlement replay state;
- fare loop/session/taximeter/passenger-manifest state;
- vehicle health and load state;
- service snapshot for inventory/build/operations/fuel/moving-refuel;
- persistent city delta state;
- deterministic state-kernel state;
- workday/session state.

The aggregate has its own integer schema version independent from the product/header schema string.`r`n`r`nGlobal city identity is stored directly in the aggregate. It must not be inferred from an optional/current ServiceNode context.
## Atomic capture and restore

Capture succeeds only when every subsystem snapshot is valid. Restore is two-phase:

1. Validate the aggregate schema and all subsystem snapshots, then reconstruct all owners into temporary values.
2. Commit the temporary values to live owners only if every reconstruction succeeded.

Any invalid schema, capacity overflow, duplicate replay ID, invalid stable ID, impossible state transition or incompatible header fails before live state mutation. Partial restore is forbidden.

All bounded collections persist their configured capacities. Restore rejects snapshots whose stored collection sizes exceed their stored capacity or whose capacities are invalid.

## Versioning and migration boundary

`FPinkCabSaveHeader` remains authoritative for product/schema/config/generator/content compatibility. Unknown or newer incompatible schemas return an explicit incompatibility result. Registered schema boundaries return `MigrationRequired`; migration itself remains explicit rather than silently coercing data.

The aggregate snapshot also carries a numeric schema version for internal logical layout validation. Subsystem snapshots may carry their own versions where needed.

Corrupt bytes, truncated payloads and invalid logical values must fail without replacing the last committed live state.
## Subsystem snapshot contracts

### Passenger

Reuse `FPinkCabPassengerSnapshot` unchanged as the canonical passenger/social/appearance/Neural owner. CD-801 consumes it; it does not duplicate passenger serialization rules.

### Economy and fare settlement

Persist ledger balance, debt limit and committed transaction IDs. Persist resolved fare-settlement records including paid/evasion kind and consequence identity so retry after recovery returns replay/conflict rather than issuing a second credit or consequence.

Restore rejects duplicate/invalid transaction IDs and impossible negative limits.

### Active fare

Persist enough of `FPinkCabFareLoopCoordinator`, `FPinkCabFareSession`, `FPinkCabTaximeter` and `FPinkCabFarePassengerManifest` to reconstruct Offered/Pickup/Boarded/Active/AwaitingPayment/Paid/Evaded/Complete/Declined states.

Snapshot data includes fare ID, order-relevant passenger identities, pricing terms, meter distance/time/running state, manifest seats/masses/layout, session state and fare-loop state. State combinations are validated as a coherent state machine, not as arbitrary enum values.
### Vehicle

Persist functional health channels and damage serial. Persist logical load state required to resume an active fare and deterministic mass/CG inputs: fuel mass/position, crew masses, ordinary passenger loads and active fare-passenger group.

Vehicle load restore validates finite non-negative masses, bounded fare group size and valid group ID relationships.

### Service/refuel/build

Reuse `FPinkCabServiceSnapshot` as the canonical service owner: service context, inventory, installed build, service-operation replay journal, fuel tank/replay journal and moving-refuel state.

No second copy of these invariants is introduced in the aggregate layer.

### Persistent world state

Persist `FPinkCabCityDeltaState` records and capacity. Only persistent authored/runtime deltas belong here; streamed decorative/materialized actors do not.

Restore reconstructs derived lookup sets from validated delta records rather than serializing redundant caches.
### Deterministic kernel

Persist `FPinkCabStateKernel` root seed and sequence so recovery resumes deterministic sequencing rather than silently re-seeding the same workday.

Runtime reset clears only runtime sequence. Workday reset accepts a new workday root seed and clears sequence while preserving campaign/profile state owned elsewhere.

### Workday/session

Add a small canonical `FPinkCabWorkdaySessionState` with stable workday identity, ordinal/index, elapsed game seconds, summary-committed flag, household-transaction count, terminal-recovery marker and bounded commit/recovery IDs where replay protection is needed.

It must encode enough state to enforce existing `FPinkCabPersistencePolicy` gates without inventing new day-economy design.

New-workday advancement is allowed only after summary commitment and exactly one household transaction, matching the existing policy contract.
## Commit and checkpoint semantics

A persistence commit serializes one validated aggregate snapshot into immutable bytes, then replaces `LastCommittedBytes` and pushes the same bytes into the bounded rolling checkpoint ring.

No caller may mutate a committed snapshot in place. The service records the explicit commit reason (`FareSettlement`, `ServiceSettlement`, `RefuelSettlement`, `RepeatClientPromotion`, `WorkdayEnd`, `PersistentWorldChange`, `PeriodicCheckpoint` or `TerminalRecovery`).

Recovery always reads a committed checkpoint, never a partially assembled live snapshot.

The existing three-checkpoint ring remains bounded. CD-801 may add explicit recovery from a selected valid checkpoint, but must not silently skip incompatible schemas or corrupt payloads without reporting the reason.
## Replay guarantees

Recovered state preserves the authoritative replay data required for idempotent retry:

- economy committed transaction IDs;
- fare-settlement resolution records;
- passenger social and Neural replay journals;
- service inventory/install/operation journals;
- fuel transaction journal;
- bounded workday operation IDs where introduced.

Retrying the same logical operation after recovery must return the subsystem replay/duplicate result or a conflict result. It must never apply the same logical mutation twice.

Replay protection remains bounded and fail-closed when its configured journal is full.

## Terminal recovery

Terminal vehicle handling uses `FPinkCabTerminalRecoveryPolicy::Canonical()` as the policy boundary:

- active fare is failed rather than silently resumed as successful;
- the current workday ends;
- damaged vehicle state is preserved;
- there is no free reset.

The recovery orchestration layer may translate the policy into owner-specific reset/failure calls, but it must not rewrite physics, economy or fare ownership rules.

## Workday rollover semantics

A new workday performs a `Workday`-scope reset only after the existing policy gate passes: summary committed and exactly one household transaction recorded.

Workday rollover resets day/runtime state such as active fare/session, workday elapsed time and deterministic runtime sequence. It may provide a new root seed to `FPinkCabStateKernel::ResetWorkday`.

It preserves campaign/profile state: economy balance and replay history, passenger identities/social history, vehicle ownership/build, vehicle damage unless explicitly repaired, persistent city deltas and other long-lived owner state.

Profile reset is outside CD-801 acceptance.

## Recovery ownership

Add one orchestration layer that coordinates capture/restore/reset, but does not own gameplay state. It receives references to existing owners, requests their snapshots, validates an aggregate and commits reconstructed owners atomically.

The orchestrator is the only place allowed to decide the order of cross-owner reconstruction. Subsystem codecs remain independently testable.

## Acceptance scenarios

CD-801 is complete only when automation proves all of the following on a fresh UE 5.8.2 build:

1. Active metered fare save/load resumes with coherent fare/session/meter/manifest/load state.
2. Awaiting-payment save/load preserves settlement identity; retry cannot double-credit fare/tip.
3. Passenger registry/Neural/social state survives aggregate save/load unchanged.
4. Service inventory/build/fuel/moving-refuel state survives aggregate save/load; retry cannot duplicate service or fuel effects.
5. Vehicle functional damage and load state survive save/load.
6. Persistent city deltas and deterministic kernel sequence survive save/load.
7. Corrupt/truncated payload and unknown schema reject atomically without replacing live state.
8. Registered schema boundary returns `MigrationRequired` explicitly.
9. Repeated recovery of the same committed checkpoint is idempotent.
10. Terminal recovery obeys canonical policy and preserves damaged vehicle state.
11. New-workday rollover resets only workday/runtime state and preserves campaign/profile owners.
12. Rolling checkpoint history remains bounded to the existing policy count.
