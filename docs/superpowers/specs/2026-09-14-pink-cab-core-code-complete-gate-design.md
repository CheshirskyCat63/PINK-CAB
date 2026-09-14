# CD-802 · Primitive-only integrated code-complete gate

Date: 2026-09-14
Status: approved design, written-spec review gate
Owner: PINK CAB Core Gameplay
Base: `2b09d29 feat(CD-801): close persistence recovery acceptance`

## Purpose

CD-802 is the final Core code-complete gate. It does not add a new gameplay subsystem.
It proves that the already-authoritative Core owners can execute one deterministic primitive-only gameplay chain on the exact integrated head without duplicating authority inside tests.

Passing CD-802 raises Core from 95% to 100%. It does **not** imply Mechanics Freeze.
After Core 100%, the next project gate remains Administrative / Design Reconciliation, followed later by Mechanics Freeze.

## Primary rule

The integrated test layer is orchestration-only. It may call public production APIs, construct deterministic fixtures, collect signatures, and assert results.
It must not reimplement fare pricing, passenger promotion, enforcement settlement, service economics, fuel credit, traversal rules, persistence validation, recovery policy, steering, or physics.

Production code is unchanged by default. A production change is allowed only when a RED acceptance test exposes a real missing integration boundary or invariant.
## Existing authoritative owners reused

- Fare/order lifecycle: `FPinkCabFareLoopCoordinator`, `FPinkCabFareSession`, `FPinkCabTaximeter`, passenger manifest/load.
- Economy/replay: `FPinkCabEconomyLedger`, `FPinkCabFareSettlementService`.
- Passenger/social: `FPinkCabPassengerRegistry` and canonical passenger record including repeat eligibility and bounded Neural state.
- Enforcement: road-rule evaluation, stable enforcement event identity, fine/economy/reputation settlement.
- Service: `FPinkCabServiceNode`, inventory/build/repair operation runtime, moving fuel session and fuel tank.
- World: deterministic city identity, road graph, city deltas, route/materialization policies and bounded logical traffic.
- Vertical: L1 traversal state/hooks plus suspended-bus and metro L2 runtimes.
- Persistence/recovery: aggregate game snapshot, binary envelope, atomic coordinator, checkpoint ring, terminal recovery and workday reset.
- Vehicle runtime: existing native Chaos Tatra pawn/dynamics provider and existing PIE automation pattern.

## Test-only structure

Create a test-only `FCoreCodeCompleteScenario` fixture/driver under `Source/PinkCabTests`.
It owns only fixture instances and stable IDs. Every state mutation is delegated to the production owner listed above.

The driver exposes scenario steps such as `RunFirstFare`, `RunRepeatFare`, `RunDecline`, `RunEvasion`, `RunEnforcement`, `RunService`, `RunVertical`, `CheckpointAndRestore`, and `RunTerminalRecovery`.
These methods are orchestration helpers, not alternate gameplay logic.

A deterministic result record captures stable observable outputs: balances, fare dispositions, passenger reconstruction signature, Neural/message counts, enforcement transaction ID, service/build/fuel state, world/kernel signatures, workday identity, and relevant traversal end states.
## Canonical integrated scenario

The scenario uses one fixed `CityIdentity`, fixed root seed, stable vehicle ID and stable passenger IDs.

1. Create first metered fare and board passengers through the ordinary manifest/load path.
2. Run metered travel and arrive at destination.
3. Commit payment before door opening; verify passenger exit cannot precede payment and economy credits exactly once.
4. Apply canonical passenger paid-fare/social memory mutation after successful fare completion.
5. Reacquire the same canonical passenger record after simulated actor stream-out; preserve visual identity data and logical social state.
6. Enable Neural permission and add a stable message/callback.
7. Complete a second paid fare for the same passenger, promoting repeat eligibility.
8. Create repeat-client order through the ordinary order/fare runtime, not a repeat-specific runtime.
9. Create a separate offered fare and decline it at the legal boundary; verify next-fare eligibility.
10. Run an off-meter fare to `AwaitingPayment`, then take the eligible evasion path and persist the stable consequence/replay state.
11. Generate one deterministic enforcement event, settle one fine/reputation consequence, then prove replay is duplicate/no-op.
12. Enter ServiceNode, perform a bounded service operation, then execute moving refuel through the existing production flow.
13. Execute canonical L1 vertical traversal and both L2 transit paths using production traversal runtimes.
14. Capture and commit aggregate game snapshot/checkpoint.
15. Intentionally mutate multiple live owners, then restore committed bytes directly into live owners through the atomic persistence coordinator.
16. Replay previously committed fare/passenger/fine/service/fuel IDs and prove no duplicate economic or logical effects.
17. Enter a representative active/awaiting fare state and execute terminal recovery; fare becomes failed, fare passenger load is removed, vehicle damage remains.
18. Start the next workday and verify only workday-scoped state resets/reseeds while persistent campaign/profile owners remain.
## Determinism gate

The complete logical scenario runs twice from fresh fixtures with the same seed and stable IDs.
Both runs must produce the same scenario signature and the same stable consequence/replay identifiers.

The signature must derive only from authoritative observable state, never pointer addresses, iteration-order accidents, wall-clock time, random device state, or presentation actors.
A different explicit root seed must be allowed to produce a different deterministic sequence without violating invariants.

## Bounded endurance gate

A bounded endurance test repeatedly performs representative short fare/passenger/traffic/checkpoint operations for a fixed iteration count.
It verifies at every interval that registry sizes, replay journals, logical traffic, snapshot checkpoint rings and retained message/memory collections remain at or below configured capacities.

The endurance gate is a deterministic stress loop, not a performance benchmark. It must terminate on its fixed iteration budget and must not depend on elapsed wall-clock duration.
Any capacity exhaustion must fail closed through the existing production result enum, with no partial money/state mutation.

## Runtime / PIE gate

Logical orchestration alone cannot close CD-802. A separate latent automation test must execute a real UE runtime smoke using the existing native Chaos/PIE pattern.
It loads the primitive-only acceptance presentation map or the minimal existing Chaos/vertical maps required by the exact-head runtime, acquires the native Tatra pawn/dynamics provider, applies controls, and verifies telemetry changes from the live movement component.

Presentation geometry remains explicitly non-authoritative. The runtime test must assert the vertical acceptance course marker remains `NON_AUTHORITATIVE_GEOMETRY` and that gameplay authority continues to live in logical owners.
## Required acceptance assertions

The CD-802 suite must explicitly prove all of the following on the same branch head:

- At least two consecutive successful fares execute through ordinary fare runtime.
- Metered and off-meter pricing paths are both exercised.
- One legal decline path completes without leaking active fare/load state.
- Payment occurs before passenger exit/door completion and credits exactly once.
- One eligible evasion produces a stable consequence and cannot later be converted into a paid settlement for the same fare.
- Repeat-client eligibility is produced by canonical passenger history and the repeat order enters the ordinary fare runtime.
- Neural contact/message state survives actor absence and persistence restore.
- Enforcement/fine settlement changes economy/reputation once and replay is a no-op.
- ServiceNode operations and moving refuel retain owner/replay continuity across snapshot restore.
- L1 traversal reaches completion through logical contacts.
- L2 suspended-bus and metro transit runtimes execute their legal transition windows.
- Save/reload restores all covered owners atomically and preserves deterministic identity.
- Corrupt/incompatible payloads remain covered by CD-801 and are rerun in the final matrix; CD-802 must not bypass that validation.
- Terminal recovery removes fare load, fails the active fare, preserves vehicle damage, and does not grant free economic reset.
- Next-workday reset changes only workday-scoped state and deterministic workday seed/sequence.
- Same seed + same inputs yields the same integrated scenario signature.
- Bounded populations/replay journals/checkpoints remain within configured limits under endurance iterations.
- Real PIE/native Chaos runtime is executable on exact head and emits live telemetry under control input.
## Stop criteria

CD-802 does not pass if a required assertion depends on test-only gameplay policy, replay duplicates economic or logical state, restore partially mutates owners, a bounded runtime collection exceeds its configured capacity, or PIE cannot exercise native vehicle runtime on the exact branch head.

A failing integration assertion is treated as a RED signal. First identify the missing or incoherent owner boundary. Production code may change only at that boundary, using the smallest API or invariant required to compose existing owners.

Out of scope: final art, animation, final environment assets, character content, bar/shop design, second/third-floor design expansion, and full Neural product design.

## Verification matrix

After the integrated suite is GREEN, run a forced clean UE 5.8.2 editor build and rerun Core, Taxi, Persistence, Vehicle, Service, World, Vertical, enforcement tests, the dedicated CodeComplete acceptance suite, PIE/runtime smoke, and bounded endurance/determinism tests.

Authoritative automation evidence comes from Saved/Logs/PinkCab.log: discovery count plus each Test Completed success result. Process exit code alone is not sufficient.

## Closure procedure

The final acceptance commit is integrated into local `main` only after a clean branch verification and merge-base check. Integration uses fast-forward only. The clean build and final matrix are then repeated on merged `main` before temporary branch cleanup.

The closure record must contain exact commit SHA, build result, discovered/passed counts for required suites, runtime smoke result, determinism/endurance result, and zero-failure confirmation.

When this gate passes, CD-802 becomes complete and Core reaches 100%. Mechanics remain unfrozen until the later project gate.