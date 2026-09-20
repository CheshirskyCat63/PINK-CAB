<!-- RECOVERED_VERBATIM from File Library source PINKCAB_G1_Master_Plan.md on 2026-09-20.
Current Git/Jira/Confluence authority supersedes conflicting historical planning text. -->

# Гейты, проценты и доказательная приёмка

## 1. Статусы

`NOT_STARTED → IN_PROGRESS → IMPLEMENTED → AUTO_VERIFIED → REVIEWED → HUMAN_ACCEPTED`.
Возможны `BLOCKED`, `FAILED`, `SUPERSEDED`. Только G1 owner принимает G1; Sol не имеет права поставить HUMAN_ACCEPTED. Сейчас все задачи пакета — PLANNED; отдельные старые системы могут быть implemented, но их reuse acceptance проходит заново.

Код 100% = все обязательные функциональные критерии реализованы и проверены; а не «ошибок компиляции нет». Салон 100% = все adopted G1 ControlId + animation/feedback/state/persistence rows accepted. Sound 100% = все adopted events have admitted audio + runtime mix accepted. При новой обязательной строке denominator увеличивается с явной revision, не переписывается история.

## 2. Gate ladder

| Gate | Вход | Выход / блокирующие условия |
|---|---|---|
| T-BASE | ADM-01..04 | Точный workspace/build provenance, принятый scope, конфликты помечены; нет присвоенной готовности |
| T-MOVE | VEH-01..03 | Авто на физических колёсах, движется вперед/назад, нет spawn fall; существующие blockers воспроизведены/закрыты |
| H-INP | INP-01..07 | Typed controls показаны на lever/button/rotary; owner принимает только действительно открытые semantics |
| H-VEH | VEH-04..09 | Steering standstill→rolling, масса/подвеска/трансмиссия приняты человеком; no assistance invariants |
| H-CAB | CAB-01..09 + CAM-01..03 | Весь adopted inventory, руки, приборы, glass, rear-seat intro и focus/pause |
| H-AUD | AUD-01..05 | Функциональный звук и mix 100% G1, rights 100% admitted |
| H-ROAD | CITY-01..07 | Сечение, неровности, right-entry, парковка, труба через дом приняты на видео/билде |
| T-WORLD | STR-01..07 + TRA-01..06 + CITY-08..10 | Бесконечность, возврат, traffic, signs/rules, bounded counters |
| H-PERF | QA-02..05 | Hardware/budget accepted, stable packaged performance, 30min + 2h evidence |
| T-RC | SYS, ASSET, ADM release tasks | Reproducible candidate, save/migration/license/credits, no blockers, fresh independent review |
| **H-G1** | Все mandatory G1 задачи и предыдущие gates | **Owner accepts shareholder build → STOP перед следующей волной asset replacement** |

H-INP/H-ROAD можно включить в общую сессию владельца; не требовать 10 отдельных approval-сообщений. Но каждое решение записывается с результатом. Если owner уже утвердил конкретный scope/gesture, повторное разрешение не требуется.

## 3. Город 50/100 — фиксированный знаменатель

Это **предложенная метрика milestone**, не измеренная нынешняя готовность. G1 требует 50/50 всех строк первой группы. Прочие системы не заменяют отсутствующие базовые: extra artwork не компенсирует провал streaming.

| Направление | Баллы | При G1 обязательно |
|---|---:|---|
| Ground cross-section, collision, seams, curves/grades | 10 | Да |
| Physical road surfaces / bumps / wet response | 6 | Да |
| Infinite deterministic streaming + reverse + saves | 10 | Да |
| Signs/markings/rules/enforcement infrastructure | 6 | Да |
| One complete interchange family + parking/service return | 8 | Да |
| One-model traffic integration, bounded provisional AI | 5 | Да |
| Rough but coherent facade/ads/light/ambient presentation | 5 | Да |
| L1 debris/wallride/magnet/ceiling/freight/poplar windows | 15 | После G1 |
| L2 metro/bus/stations/vertical routes | 15 | После G1 |
| Final traffic roster/AI + incidents/crowd breadth | 8 | После G1 |
| Full city art/material/lighting/audio variation and final budgets | 8 | После G1 |
| Full-release services/navigation/persistence edge completion | 4 | После G1 |
| **Итого** | **100** | **50 mandatory G1** |

Для задач: completion percent = accepted required criteria / frozen required criteria *100. AUTO_VERIFIED и HUMAN_ACCEPTED выводить двумя колонками. BLOCKED/NOT_RUN не являются 50%-выполненными. Общий процент milestone не может скрыть P0/P1 blocker.

## 4. Acceptance scenarios

| Test ID | Setup / действие | Pass / Fail oracle |
|---|---|---|
| QA-G1-001 | Cold launch 20 раз; menu 30 sec; Start | Rear-seat view, wheels visible, no underside camera, no falling/overlap |
| QA-G1-002 | Flat road, healthy profile, N→1, redose, clutch release | Positive wheel torque и движение; снимок всех 4 контактов |
| QA-G1-003 | Та же машина R, умеренный газ, straight/steer | Отрицательная longitudinal скорость, correct steering geometry, no auto forward gear |
| QA-G1-004 | Idle/E+wheel до 100% в N, потом под нагрузкой | RPM dynamic, limiter ~accepted 8500, power curve consistent; не throttle display fake |
| QA-G1-005 | 5th at 30 km/h, сцепление отпущено | Lugging/RPM падение/возможный stall; corrected downshift возвращает езду |
| QA-G1-006 | Steering 0→1→3→5→20 km/h | Ощутимо освобождается, sign верный, нет discontinuity |
| QA-G1-007 | Recorded mouse counts at 30/60/120 FPS | Similar command/actuator traces within predeclared tolerance; no dt² |
| QA-G1-008 | Gaze/grip/release после быстрого mouse motion | Один owner, нет скачка руля/взгляда или residual lever delta |
| QA-G1-009 | 20 launches; stop jitter 0..threshold | Один reset/LaunchSerial; каждый новый launch требует wheel |
| QA-G1-010 | Q/W/E/Space/RMB/LMB combinations + wheel | Single recipient, E>W>Q, rest unchanged; radio не регулирует газ одновременно |
| QA-G1-011 | Q edge, Q hold, 3, radio grip+Q, handbrake+Q | Staged gearbox without grip; no steal active target; steering survives Q |
| QA-G1-012 | 50 H-gate gestures all positions; fast diagonals | Correct path through N, no neighbor auto-selection, 0 inversion |
| QA-G1-013 | Alt-tab/menu в середине shift | No delayed gear; held inputs cleared; actual gear preserved legally |
| QA-G1-014 | 25/50/100% handbrake at stop and moving | Distinct torque; parking latch vs hydraulic return; heat causality |
| QA-G1-015 | Low-gas stall, restart via physical ignition | Same car/fare/save; no free repair/teleport |
| QA-G1-016 | Same bump at fixed speeds/load, dry/wet | Wheel/chassis response causal; visual/collision patch aligned |
| QA-G1-017 | Every G1 control min/mid/max/default/cancel | All inventory rows have independent evidence; no dead controls |
| QA-G1-018 | Radio gaze wheel; LMB+wheel; focus loss | Correct volume/tune, no accidental power toggle, no stuck sound |
| QA-G1-019 | Every gauge sweep 0/25/50/75/100%, real driving | Semantic value and needle agree; oil vs coolant label correct |
| QA-G1-020 | Glass/mirrors/wipers night/rain | Readable road/cockpit; no opaque glass or reversed steering logo |
| QA-G1-021 | Menu Start twice, Escape midblend, pause/load | One camera owner; correct endpoint; no underbody return |
| QA-G1-022 | Audio full vehicle drive with capture | Loop continuity, RPM/load distinction, bounded voices, no digital clipping |
| QA-G1-023 | Reverse through >10 regenerated chunks same city | Graph/rules/entrances stable; hashes match; no persistent reroll |
| QA-G1-024 | Right entry both directions, loop/reversal/oval/8 | All route cases reachable; no wrong-way ordinary entry or dead end |
| QA-G1-025 | 10 loops around one interchange | Active path never unloaded; no collision seam; bounded neighbors |
| QA-G1-026 | Parking enter/save/reload/exit/retry | Same city return, vehicle/fare/payment unchanged or valid transitioned once |
| QA-G1-027 | Traffic seed replay merge/lane-change/obstacle | No overlap, no teleport in view, reservations clear, AI stays provisional |
| QA-G1-028 | Spawn sign zone + pass enforcement twice + load | Sign/rule/AI same value; duplicate event cannot double fine |
| QA-G1-029 | 30 min route + several turns/reverses/services | Resource counters plateau after warm-up, no residual orphan growth |
| QA-G1-030 | 2h packaged candidate long-range drive | No crash/critical ensure, memory budget held, precision/streaming valid |
| QA-G1-031 | Save all stable controls, damage, active fare | Legal restore, no held key/grip, no free repair/credit duplication |
| QA-G1-032 | Model profile swap A→B→A | Input, physics ownership, saves stay valid; pivots/material/glass pass |
| QA-G1-033 | Delayed generation and failed asset injection | No stale job attach/use-after-release; clear diagnostic; no drive into void |
| QA-G1-034 | New build hash; all tests; launch exact artifact | Evidence hash matches tested files; shortcut matches accepted candidate |

## 5. Evidence schema

Каждый результат: `TaskId,RequirementIds,TestId,SourceSha,DirtyPatchHash,BuildId,ExeSha256,CookedManifestHash,EngineVersion,Platform,Configuration,ProfileId,ProfileVersion,CityCode,GeneratorVersion,ContentSetVersion,TrafficSeed,SaveSchemaVersion,Command,StartUtc,DurationSeconds,Expected,Observed,Status,LogPaths,VideoPaths,TelemetryPaths,Reviewer,HumanDecision`.

Для final candidate DirtyPatchHash пуст только при clean tracked tree; untracked build inputs учитываются manifest. Поле HumanDecision не автозаполняется. Error budget заранее фиксируется: crash/fatal/assert/critical ensure 0; known benign startup DLL notices не скрывают новые warnings. Автоматическая классификация хранит raw log и list of allowed known messages, не стирает строки.

## 6. Какие проверки когда

На микрошаге: focused regression + affected domain + code-health + diff check. На integration boundary: full PinkCab automation, relevant non-NullRHI/PIE. Перед candidate: build+cook+package, actual D3D launch, real controls/audio/collision, smoke, 30min/2h. NullRHI не проверяет стекло, камеру, actual audio или perceptual feel.

Нужные существующие команды:

```powershell
# Запускать позже, из VERIFIED repo root, после разрешения реализации.
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\Scripts\build.ps1
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\Scripts\code-health.ps1
python .\Scripts\code_health.py --check --require-zero-debt
python -m unittest discover -s .\Scripts\tests -p 'test_code_health.py'
git diff --check
$PinkCabEditorCmd = 'C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
& $PinkCabEditorCmd "$PWD\PinkCab.uproject" -Multiprocess -unattended -NullRHI -nosplash -nopause -NoSound -stdout '-ExecCmds=Automation RunTests PinkCab' '-TestExit=Automation Test Queue Empty'
```

Проверить актуальный CLI `--help` анализатора до исполнения; exact filters из ENGINEERING_START_HERE. Exit code 0 без найденных/завершённых тестов не PASS. Старые 354/354 и 310/310 не переносятся на новый SHA.
