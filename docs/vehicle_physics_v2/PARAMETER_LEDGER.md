# Vehicle Physics V2 parameter provenance

Status meanings:

- **OWNER_LOCKED / OWNER_REQUIREMENT** — may change only by explicit owner decision.
- **CURRENT_OBSERVED** — present in audited source; not automatically final tune.
- **SOURCE_PROVENANCE** — measured/source value for a specific donor/model only.
- **CALIBRATION_CANDIDATE** — trial value requiring evidence + human gate.
- **HISTORICAL_REFERENCE** — preserved for context; not current implementation authority.
- **UNKNOWN** — do not invent.

| Parameter | Value | Status | Source / rule |
|---|---:|---|---|
| engine.max_power_hp | 250 | CURRENT_OBSERVED | current `FPinkCabChaosPhysicalProfile`; old docs 180 must not overwrite blindly |
| engine.max_torque_nm | 260 | CURRENT_OBSERVED | current physical profile; old docs 240 historical until P04 decision |
| engine.max_rpm | 8500 | CURRENT_OBSERVED | current physical profile |
| engine.idle_rpm | 750 | CURRENT_OBSERVED | known current code value; does not meet new owner requirement |
| engine.warm_idle_acceptance_rpm | 900–950 | OWNER_REQUIREMENT | 2026-09-26 requirement |
| engine.warm_idle_center_rpm | 925 | CALIBRATION_CANDIDATE | first center candidate only |
| engine.brake_effect | 0.15 | CURRENT_OBSERVED | tune only with coast evidence |
| engine.rev_up_moi | 0.17 | CURRENT_OBSERVED | P04 response surface |
| transmission.final_drive | 3.2 | CURRENT_OBSERVED | old page27 4.25 is historical/reference |
| transmission.forward | 4.6 / 2.2 / 1.5 / 1.1 / 0.85 | CURRENT_OBSERVED | build actual speed/RPM table |
| transmission.reverse | 4.6 | CURRENT_OBSERVED | old page27 3.5 historical/reference |
| steering.max_road_wheel_deg | 41 | CURRENT_OBSERVED | current profile |
| steering.center_exponent | ~1.28 | CURRENT_OBSERVED | current steering controller |
| steering.stationary_response_per_s | ~2.5 | CURRENT_OBSERVED | current controller |
| steering.moving_low_response_per_s | ~10.5 | CURRENT_OBSERVED | current controller |
| steering.moving_high_response_per_s | ~6.0 | CURRENT_OBSERVED | current controller |
| steering.high_speed_target_gain | ~0.55 | CURRENT_OBSERVED | current controller |
| pedal.throttle_press_s | ~0.35 | CURRENT_OBSERVED | current pawn response surface |
| pedal.throttle_release_s | ~0.22 | CURRENT_OBSERVED | current pawn |
| pedal.brake_press_s | ~0.20 | CURRENT_OBSERVED | current pawn |
| pedal.brake_release_s | ~0.28 | CURRENT_OBSERVED | current pawn |
| pedal.clutch_press_s | ~0.16 | CURRENT_OBSERVED | current pawn |
| tire.front_nominal_friction_multiplier | 2.00 | CURRENT_OBSERVED | current profile |
| tire.rear_nominal_friction_multiplier | 0.50 | CURRENT_OBSERVED | explicitly supports easy wheelspin; must be A/B validated |
| wheel.radius_cm | 32.13 | CURRENT_OBSERVED / 613 SOURCE | never silently copy to 603/77 |
| wheel.width_cm | 20.5 | CURRENT_OBSERVED / 613 SOURCE | never silently copy to 603/77 |
| geometry.613.wheelbase_mm | 2980 | SOURCE_PROVENANCE | current Confluence donor measurements |
| geometry.613.front_track_mm | 1520 | SOURCE_PROVENANCE | 613 donor only |
| geometry.613.rear_track_mm | 1520 | SOURCE_PROVENANCE | 613 donor only |
| suspension.front_total_travel_mm | ~160 | CURRENT_OBSERVED_SEED | measure actual sag/bump/droop in P06 |
| suspension.rear_total_travel_mm | ~180 | CURRENT_OBSERVED_SEED | measure in P06 |
| ABS | false | OWNER_LOCKED | no ABS |
| ESP / yaw rescue | false | OWNER_LOCKED | no ESP/hidden correction |
| hero identity | bespoke early/Gen-1 603-family | CANON | physical profile support does not rewrite product canon |

## Profile contract

Each Tatra profile must carry:

- `model_id`, `profile_version`, `schema_version`, `profile_hash`;
- per-field provenance;
- base/fuel/occupant mass rules;
- local CoM XYZ and inertia;
- geometry/wheels;
- suspension;
- engine idle/red-zone/limiter/max/torque curve/inertia/braking;
- gearbox/final drive;
- service brake/handbrake;
- tire/surface combined-grip;
- thermal/electrical capability;
- presentation sockets that never own physics.

A candidate profile is not current authority merely because it compiles.
