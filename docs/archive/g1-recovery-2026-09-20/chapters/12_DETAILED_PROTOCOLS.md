<!-- RECOVERED_VERBATIM from File Library source PINKCAB_G1_Master_Plan.md on 2026-09-20.
Current Git/Jira/Confluence authority supersedes conflicting historical planning text. -->

# Приложение — протоколы реализации и проверки

## A. Контракт события ввода

Предлагаемая запись диагностического события:

```json
{
  "frame": 420,
  "input_serial": 19,
  "source": "MouseWheel",
  "signed_steps": 1,
  "held": ["Q", "E", "Space"],
  "focused_control": "CKP-RAD-PWRVOL",
  "captured_control": null,
  "wheel_recipient": "Throttle",
  "reason": "PedalPriority_E_W_Q",
  "consumed_count": 1
}
```

Oracle: ровно одно изменение target, clutch release time/radio volume unchanged. В следующем кадре release E+Q при всё ещё удерживаемом Space переводит wheel к radio, только если knob валиден и supports wheel-in-gaze. Отсутствие wheel event не повторяет предыдущий detent.

Input state transition table в implementation: для каждой пары `CurrentContext × Event` определить `NextContext,ConsumedChannels,CancelPendingGesture,ClearDelta,StateEffects`. Перечень events: gaze down/up; quick recall down/up; Q down/up; RMB down/up; LMB down/up; wheel; focus lost/gained; menu opened/closed; target invalidated; vehicle unpossessed; load begun/ended. Не допускать неявного default, который передаёт input сразу двум owners.

## B. Steering replay

Весь time series задаётся в device counts и timestamp. За 1 секунду 600 counts вправо, 0.2s hold, 1 секунду 1200 влево, 0.2s hold, 0.5s 600 вправо. Ресэмплинг на 30/60/120 FPS сохраняет интегральные counts и event order. Сравнивать cursor/target/steering/end angle/settling delay, отдельно nonlinear rate response.

Предлагаемые диагностические допуски: cursor endpoint ≤0.01 normalized; steering endpoint ≤0.02 после settling; qualitative result не меняется; direction sign errors=0. Это **EXPERIMENTAL test tolerances**, после оценки цифрового quantization подтвердить в INP/VEH fixture до принятия. Не увеличивать допуск после неудачи без причины.

Тест stationary и rolling с одинаковыми counts: направление одинаковое, скорость actuator выше при rolling. Тест медленного движения в R использует abs(speed) для feel, но реальную геометрию руления и задний знак движения, не меняет input sign в reverse.

## C. Contact report

По каждому wheel снимать: semantic ID/index, bone/source anchor, local center, physical radius, visual measured radius, suspension axis/start/end, hit actor/component, surface ID, compression, normal load, tire angular speed, torque. Дополнительно chassis overlap sweep и body bounds. Debug screenshot должен иметь подписи wheel IDs, не только четыре линии.

Нельзя заключать «контакт правильный» только из ContactCount=4: все raycasts могут бить по собственному body либо не той surface. Проверять actor ownership и material. Render wheels не могут служить authority spawn при отличающихся dynamic poses.

## D. Torque consistency

Конкретный fixture из текущего source profile: radius .3213m, final drive 3.2, 5th .85. При 30 km/h wheelRPM≈247.7, coupledRPM≈673.7. Это объясняет необходимость lugging при полностью замкнутом сцеплении в высокой передаче; не обещает точное поведение без load/friction/idle torque. First 4.6 при той же скорости даёт примерно 3646 RPM.

В reverse используй отдельный sign/direction и ratio magnitude; wheelRPM от signed longitudinal speed не превращает engine RPM в отрицательные значения. Проверить typed unit functions `KmhToMps`, `WheelRadPerSecToRpm` либо существующие equivalents, без произвольных множителей в UI.

На 8500 RPM normalized torque .81×260=210.6 Nm. `187.5kW ≈251.4hp ≈254.9PS`; эти величины не равны одной единице. В profile metadata явно выбрать convention. Torque curve table также проверить на пик выше declared 250: поле MaxPowerHp не применяется как ограничитель в прочитанном ApplyToMovement.

## E. Cabin per-control receipt

```json
{
  "control_id": "CKP-RAD-PWRVOL",
  "scope": "G1_ADOPTED",
  "kind": "Rotary",
  "part_id": "RadioVolumeKnob",
  "socket_provenance": "measured in admitted model",
  "state_owner": "RadioRuntime",
  "wheel_in_gaze": true,
  "grip_actuates": false,
  "min": 0.0,
  "max": 1.0,
  "default": 0.5,
  "save_stable_value": true,
  "save_held_inputs": false,
  "evidence_state": "NOT_RUN"
}
```

`RadioVolumeKnob/RadioRuntime/default .5` — предложенные имена/значение, не якобы найденные source symbols. ASSET/CAB task связывает с реальными parts/owner. Дефолт UI и saved restore проверять независимо. Отсутствие fitted part не разрешает тихо исключить control без DEC-02.

## F. Streaming proof fixtures

Три проверочных CityCode (предлагаемые для QA, не существующие сохранения): `G1-STRAIGHT-A`, `G1-PIPE-LOOP-B`, `G1-PARK-RETURN-C`. Отдельные recorded seeds для traffic/decor/weather. Не использовать elapsed wall-clock как seed.

1. Generate logical chunks -2..+20 в прямом порядке; hash normalized graph/rules/static recipes.
2. Retire все, кроме current/pinned; восстановить +20..-2 в обратном порядке; hashes совпадают.
3. Сменить traffic seed при той же CityIdentity; статические hashes не меняются.
4. Изменить material-only style profile; graph/rules/entrance hashes не меняются.
5. Simulate chunk generation completion после его cancel; результат не создаёт actor/collision.
6. На развязке 10 кругов; graph-path pins защищают текущую road mesh независимо от longitudinal progress.
7. Войти в ServiceNode, выгрузить corridor visual, выйти; восстановить same route ID и accepted save state.
8. Перейти к далёкому logical index; precision camera/contact/traffic не ухудшается сверх принятого budget.
9. Нехватка asset/collision-ready оформлена fail-state; no silent fallback road reroll.

Сборка не обещает побитно детерминированную Chaos simulation между разными CPU. Детерминизм city recipe/IDs/rules и воспроизводимые traffic сценарии отделены от floating-point physics tolerances.

## G. Производственная деградация под бюджет

Порядок снижения стоимости при сохранении функций: дальние decorative silhouettes/LODs → texture/mip budget → decorative light distance/concurrency → aggregate audio → pooled cosmetic VFX → visual population density. Collision перед игроком, route signs, wheel contacts, interaction feedback и текущий control target не отключать ради FPS.

Mirrors могут иметь отдельные update rates/quality tiers после измерений, но должны показывать полезные угрозы. Rain/fog не должны скрывать заготовку дороги ближе принятой stopping/decision distance. Profile change в меню не reroll city и не reset engine health.

## H. Более подробная последовательность после G1

| Task | Подэтапы, которые нужно конкретизировать по новому scope |
|---|---|
| POST-01 Asset wave | admission manifest → authoring units → semantic slots → import sandbox → visual profile A/B → collision envelope → material remap → LOD/perf → save/input regression → owner look gate |
| POST-02 Wallride | current authority audit → floor/debris transition → seven band IDs/contact sampling → load/instability state → front/rear loss classification → recoverable/terminal states → camera/audio cues → normal road handoff → no-assist audit → matched-seed QA |
| POST-03 Ceiling | resolve M05 → five lane graph above ground → opposite freight direction → single/3–5 convoy occupancy → docking non-player apertures → magnetic receiving strip → mass-dependent residual → gap/reacquisition → poplar crossing envelope → opposite landing → save/recovery → bounded soak |
| POST-04 Life/hangar | node ID/return contract → training geometry → safe reset only in approved training context → incidents with moving bypass → deterministic selected persistence → motorcycles/sidecars → micromobility vignette slots → crowd near/mid/far → cleanup counters → owner city-life gate |
| POST-05 L2 | verify CD-798 existing implementation → road/rail/ceiling cross-section → two metro tracks per side → station structure/sightlines → bus-lane count decision → deterministic schedules/occupancy → vertical connections → signs/work routing → tier streaming pins → collision safety → service/fare tests → art/perf gate |
| POST-06 Final AI/art | traffic roster permission → vehicle classes envelopes → speed/headway/personality matrix → owner driving targets → aggressive merge bounded rules → incident responses → final ads/frontage/vignettes → dusk/night visual lock according to current canon → audio mix → 30min dense worst-case run |
| POST-07 Release | all current offline product AC reconciliation → supported save versions → settings/accessibility/localisation → install/prerequisites → packaged shipping smoke → credits/licenses/privacy only if needed → store content truthful → regression/2h soak → independent review → final owner gate → tagged release |
| POST-08 Future | standalone approved specs for L3, Neural/social/government, character roster, common rooms, multiplayer authority/netcode; не рисовать точные implementation fields до принятой механики |

Эти этапы конкретны по порядку и границам, но кодовые параметры ещё не утверждённых верхних систем нельзя честно заморозить сегодня. Текущее поручение просит first gate; никакая из будущих задач не считается выполненной от наличия таблицы.
