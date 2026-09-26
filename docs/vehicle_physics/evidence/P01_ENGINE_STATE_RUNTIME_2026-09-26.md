# P01 Engine State / Coast / Idle Runtime Evidence — 2026-09-26

## Verdict

**Technical acceptance: GREEN. Human feel gate: PENDING.**

This evidence closes PHY-005 through PHY-008 as engineering work packages. It does not record owner acceptance of vehicle feel.

## Canonical candidate

- Git main: `eae3907ca4287a94abf9c205a8f4c89d2c99ef5a`
- Exact-main physics run: **36264255545**
- Unreal runtime: **5.8.3**
- Model: `TATRA_613`
- Profile: `PINKCAB_TATRA613_CHAOS`
- Schema / calibration: **1 / 3**
- Deterministic profile hash: `7A90D02ED12B1E93`
- Writer guard: **PASS — 72 groups / 84 executable occurrences**
- Focused physics suite: **23/23 PASS**
- Editor build / Game build / baseline collector: **PASS**

## PHY-005 — single combustion permission

One resolved actuation path gates both native Chaos throttle and the authored partial-clutch rear torque path. Off/Stalled states cannot contribute positive combustion torque. Existing accepted control grammar is unchanged.

## PHY-006 — coast is not propulsion

Runtime flat key-off proof:
- key-off speed: **300.595 cm/s**
- speed after observation: **202.344 cm/s**
- post-key-off physical travel: **195.934 cm**
- final Chaos throttle: **0**
- external rear drive torque: **0**

Runtime isolated 20° neutral slope proof:
- downhill horizontal travel: **54.168 cm**
- start/end horizontal speed: **46.494 / 39.816 cm/s**
- max abs wheel angular velocity: **1.269 rad/s**
- engine contribution: **0**

No velocity freeze, actor teleport, direct-force boost, ABS, TC, ESP, yaw rescue or hidden propulsion was added to production code.

## PHY-007 — start / stall / restart

- deterministic Off / Running / Stalled transition tests: PASS
- low-RPM coupled-overload stall test: PASS
- restart clears stale propulsion demand and does not resurrect stale throttle: PASS

## PHY-008 — warm carb idle

- warm healthy neutral idle: **925 RPM**
- neutral 55% throttle blip peak: **4429.622 RPM**
- returned to **949.435 RPM**
- measured return time: **7.600 s**
- explicit vehicle-profile rev-down calibration: `EngineRevDownRate=1800`
- calibration identity advanced to v3 and is fingerprinted

## Human-gate delivery

Fresh package workflow run: **36266076177**.

- exact candidate checkout: PASS
- fresh BuildCookRun: PASS
- Authenticode signing: PASS
- packaged endless-road smoke: PASS
- desktop shortcuts updated:
  - `PINKCAB P01 HUMAN.lnk`
  - `PINKCAB Latest.lnk`
- delivery root: `E:\CHESHIRE_DIVISION\Builds\PINKCAB\P01_HUMAN_eae3907c_RUN36266076177`
- launched process PID at delivery: **45876**
- human state: **PENDING**

## Scope integrity

No city/MetaRoad/world geometry, fare/economy logic, accepted mouse/Space/Q/W/E/RMB/LMB grammar, H-pattern layout, ABS/TC policy, or hidden driver-assist behavior was changed by P01.
