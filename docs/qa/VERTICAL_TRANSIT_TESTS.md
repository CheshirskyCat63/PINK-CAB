# PINK CAB · Vertical Transit & Auto-Parkour QA / Evidence Contract

**Status:** FIRST-EURO PLANNED VERIFICATION AUTHORITY / NOT RUNTIME VERIFIED
Canon/spec: `docs/PINK_CAB_VERTICAL_TRANSIT_PARKOUR.md`
Vehicle stack: `docs/PINK_CAB_VEHICLE_TECH_STACK_FGEAR_VDS.md`
Mass/load: `docs/PINK_CAB_VEHICLE_MASS_LOAD_DYNAMICS.md`
Confluence: `8388609`
Jira: gate `CD-547`; load/vertical QA `CD-701`; FGear `CD-732/CD-738`; BASE-100 vehicle `CD-748`.

FIRST EURO fully implements Level1 + Level2. Level3 gameplay is POST-FIRST-EURO and has only a compatible route/schema extension boundary.

## Evidence conventions

Every VERIFIED result records exact build/commit, UE/FGear/VDS/profile/content/generator/schema versions, CityCode/seeds/module IDs, vehicle position/velocity/yaw/slip, contact/band/track state, fuel/crew/passenger/total mass, expected/observed magnet timeout, physical vs Expression channels, traffic IDs, failure/recovery reason, bounded-runtime counters and artifact paths.

## Locked mass / residual function

Base 1450 kg; full-fuel 1550 kg; heroine 58 kg; daughter 49 kg; actual passenger masses; standard fixture 1657 kg; max declared fixture 2107 kg; no ABS/ESP.

Level1 residual magnetism is **LOCKED LINEAR** by authoritative total mass:

- `m <= 1657 kg` → **5.0 s** cap;
- `1657 < m < 2107 kg` → `timeout_s = 5.0 - (m - 1657) / 450`;
- `m = 2107 kg` → **4.0 s**;
- `m = 1882 kg` → **4.5 s**.

Old fixed-five-second and `interpolation OPEN` wording is SUPERSEDED. Force/contact/reacquisition constants remain `CD-592` calibration; exact reacquisition reset semantics remain owner-pack item M05.

## Vehicle preconditions

### `PC-T-MASS-001` · Exact runtime mass

Prove `total_mass = 1450 + current_fuel + 58 + 49 + sum(boarded_passenger_mass)`. Boarding/exiting and save/reload may not duplicate or reroll mass.

### `PC-T-ELEC-001` · No ABS / no ESP

Hard-brake/yaw fixtures show no ABS pulsing or ESP individual-wheel intervention. Any bounded non-diegetic assistance is separately logged and cannot become hidden ESP.

### `PC-T-LOAD-001/002` · Load trend / body coherence

Matched FGear fixtures show modest acceleration/braking degradation and increased roll/pitch/settling with load. Expression remains downstream of physical state.

## Level1 · wallride / freight ceiling

### `PC-T-VERT-T1-001` · Ceiling entry

Road→wallride→ceiling enters stable contact with no teleport, mission-lane override or stuck control state.

### `PC-T-VERT-T1-002` · Freight-gap traversal

Representative live five-lane opposite-flow freight section provides a skill-readable bounded obstacle field; no guaranteed scripted success is required.

### `PC-T-WALL-LOAD-001` · Load direction

At matched speed/yaw/band/geometry, heavier fixtures follow the locked modest contact-retention direction while roll/pitch/settling penalty remains.

### `PC-T-MAG-MASS-001` · Reference endpoint

1657 kg effective residual endpoint = **5.0 s**.

### `PC-T-MAG-MASS-002` · Max endpoint

2107 kg effective residual endpoint = **4.0 s**.

### `PC-T-MAG-MASS-003` · Linear intermediate fixtures

Use at least three masses between endpoints, including 1882 kg = **4.5 s**. Observed values must match `5.0 - (m - 1657)/450` within final timer precision; no hidden seat-count preset/nonlinear curve.

### `PC-T-MAG-MASS-004` · Lighter cap

Legal mass below 1657 kg never receives more than **5.0 s**.

### `PC-T-MAG-MASS-005` · Above-current-authority guard

Artificial mass above 2107 kg cannot silently create an unowned production extrapolation; behavior must be explicit/logged until future authority exists.

### `PC-T-MASS-SAVE-001` · Persistence

Passenger/fuel/total mass and derived timeout reconstruct identically after save/reload/streaming.

### `PC-T-VERT-T1-003/004/005` · Side swap guards

Approved ceiling/poplar window supports legal cross-carriageway side swap using current contact/load state. Ground road has no equivalent instant side-swap. Static tree/support obstruction invalidates legal jump marker deterministically.

### `PC-T-VERT-T1-006` · Failure / recovery

Too slow, excess yaw/lateral error, tree strike, freight collision, bad landing/contact loss/timeout each resolve through normal FGear/VDS/Vehicle Health result with no stuck magnet/input/camera/traffic state.

If terminal/immobilized, emit the common FIRST EURO recovery event to `CD-598/CD-750`. **No insurance-specific branch exists in year one.** Future insurance attaches through `RecoveryPolicy/RecoveryHook` after physical classification and cannot change the failed-jump physics.

### `PC-T-VERT-T1-007` · Reverse reconstruction

Forward/reverse travel reconstructs the same static vertical topology for the same CityCode/content version; passenger load changes dynamics only.

## Level2 · bus layer

### `PC-T-VERT-T2-001` · Bus-layer entry

Enter the legal Level2 transition and reach stable contact using the same hero-Tatra FGear owner. Record total mass; Level2 may use a distinct calibrated contact profile.

### `PC-T-VERT-T2-002` · Bus obstacle / relative speed

Moving bus traffic provides readable clearance without teleport/scripted avoidance.

### `PC-T-VERT-T2-003` · Partial / knife-edge state

Approved partial-contact state remains bounded and recoverable; no free-flight/stuck exploit.

### `PC-T-VERT-T2-004` · 360° body-language cycle

Ascent and descent follow the final direction/orientation authority; one complete cycle meets final tolerance.

### `PC-T-VERT-T2-005` · Lane transfer

Runs after final bus-lane/profile lock and respects actual clearance/traffic.

### `PC-T-VERT-T2-006` · Station-zone traversal

Station/vokzal region preserves route/road/bus/metro boundaries and follows the final station gameplay contract from owner Pack N05.

### `PC-T-VERT-T2-007` · Abort / collision / restart

Representative failures return to clean state. Terminal damage emits the common recovery event; no first-year insurance state is consulted.

## Level2 · metro / station

- `PC-T-METRO-001`: two metro tracks per side reconstruct without clearance violation;
- `PC-T-METRO-002`: trains remain readable from required Level1 viewpoints within final performance budgets;
- `PC-T-METRO-003`: versioned direction/headway/offset follows final calibrated/owner rule;
- `PC-T-METRO-004`: station preserves track/platform/road continuity and deterministic CityCode identity;
- `PC-T-METRO-005`: reverse travel reconstructs the same station/track topology;
- `PC-T-METRO-006`: metro roof/track is not a player taxi driving surface unless separately reopened.

## Performance / streaming

- `PC-T-VERT-PERF-001`: heavy Level1 ground+freight+vertical scene meets final bounded budgets;
- `PC-T-VERT-PERF-002`: heavy Level2 bus+metro+station scene meets final bounded budgets;
- `PC-T-VERT-STREAM-001`: 30-minute longitudinal session has no monotonic Actor/memory/contact/passenger-mass leak;
- `PC-T-VERT-SAVE-001`: save/reload near a vertical route reconstructs static topology, exact load and linear timeout;
- `PC-T-VERT-VERSION-001`: generator/content changes migrate/preserve/fail explicitly rather than silently reshuffling persisted identity.

## Level3 post-year guard

`PC-T-VERT-T3-001`: FIRST EURO code may expose compatible tier/route schema fields but may not copy Level1 freight or Level2 bus/metro mechanics into a canonical Level3 gameplay system. Level3 gameplay/tests are POST-FIRST-EURO and do not block year-one gates.

## Maturity

These tests are SPECIFIED only. Runtime remains **NOT VERIFIED** until exact-build executable evidence passes the applicable FIRST EURO rows.
