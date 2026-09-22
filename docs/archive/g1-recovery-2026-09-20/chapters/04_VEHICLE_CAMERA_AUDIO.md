<!-- RECOVERED_VERBATIM from File Library source PINKCAB_G1_Master_Plan.md on 2026-09-20.
Current Git/Jira/Confluence authority supersedes conflicting historical planning text. -->

# Автомобиль, камера, анимация и звук — план поведения

> Для последующей реализации: `superpowers:executing-plans`, debugging до fixes, TDD для проверяемой логики. Задачи и зависимости — 09. Здесь не исполнялись изменения.

## 1. Сначала воспроизвести «не едет»

Это P0. Не лечить увеличением MaxTorque или отключением collision. Проверка выполняется на packaged candidate и диагностическом fixture одного и того же source tree.

| Шаг | Что записать | Что отделяет |
|---|---|---|
| 1 | SHA, executable hash, command line, save/profile, map, spawn pose | Ошибочный EXE/старый save/не та карта |
| 2 | Menu phase, possession, input capture, bSimulatePhysics, sleeping, physics enabled | Авто ещё удерживается меню или не проснулось |
| 3 | Target/actual throttle, brake, clutch, handbrake, parking latch | Нажатие не доходит или ручник остался включён |
| 4 | Requested/engaged gear, coupling, EngineRPM, stall state | Передача не включилась; сцепление разомкнуто; двигатель заглох |
| 5 | FL/FR/RL/RR wheel centers, radius, bone/anchor, trace start/end, hit component/material | Физические колёса не совпали с видимыми или hit идёт в собственный кузов |
| 6 | Wheel load/compression, chassis overlap, normals, contact count | Кузов опирается на землю/подвеска за пределами хода |
| 7 | DriveTorque и BrakeTorque per wheel, AngularSpeed, linear chassis speed | Torque не попал на заднюю ось; тормоза блокируют; колёса крутятся без контакта |
| 8 | Forward/Reverse fixture на плоскости; затем текущая карта | Ошибка машины либо поверхности/спавна |
| 9 | Fresh save и copied existing save | Сохранённое повреждение/ручник/неверная трансформация |
| 10 | Полный input trace через фактический PlayerController | Unit tests обходят реальный сломанный ввод |

Диагностический overlay в Development-only: `InputOwner`, `WheelRecipient`, `ThrottleTarget/Actual`, `BrakeTarget/Actual`, `ClutchPedal/Coupling`, `HandbrakeAmount/Latched`, `RequestedGear/EngagedGear`, `EngineRpm`, `SpeedKmh`, `ContactCount`, `WheelLoads`, `ChassisOverlap`, `SpawnPhase`. Не включать эту таблицу в финальный игровой HUD.

Официальные debug commands: `p.chaos.debugdraw.enabled 1`, `p.Vehicle.ShowSuspensionRaycasts 1`, `p.Vehicle.ShowWheelCollisionNormal 1`, `p.Vehicle.ShowRaycastComponent 1`, `p.Vehicle.ShowWheelForces 1`, `p.Vehicle.ShowCOM 1`, `stat ChaosVehicle`. Сверить регистрацию на установленной 5.8.2. Использовать в отдельном диагностическом запуске, не сохранять overrides в production ini.

**Доказательство причины:** один воспроизводимый случай, минимальная изменённая переменная, одинаковый input sequence до/после, contact/torque telemetry и движение кузова. Скрин видимых шин не доказывает работоспособность колёс.

## 2. Spawn lifecycle

Предлагаемая последовательность: `MapReady → RoadCollisionReady → SpawnValidated → PhysicsSettling → MenuReady → DriverBlend → Driving`. Использовать readiness events и bounded settle window; не таймер «через 2 секунды наверное готово».

- Источник геометрии спавна — physical wheel centers/radii и collision sweep кузова, а не min bounds произвольного render mesh.
- Четыре контакта измеряются с дорогой, собственный actor исключён; фильтр road collision отделён от декоративного Visibility.
- Если spawn overlap/road not ready: старт недоступен, есть конкретный failure reason; выбрать authored recovery/spawn fixture. Не снимать collision и не выставлять бесконечную силу вверх.
- Physics settles до показа main menu; автомобиль не падает с высоты при Start. Если после settle используется freeze, release не меняет transform/penetration и не добавляет импульс.
- У initial menu и pause разные пути. Pause не переспавнивает машину, не двигает её на землю заново, не чинит повреждения.
- Root scale положительный и единичный; visual offset допустим только в отдельном profile; не меняет радиус шин, track или chassis mass.
- Donor wheels сохраняются до утверждённой замены. Render FL/FR/RL/RR явно привязаны к физическим колёсам, вращаются и ходят с подвеской.

Acceptance: 20 cold starts + 20 menu/resume, flat/camber/ramp, empty/full adopted load. Нет заметного падения, проникновения, скрытых шин, непреднамеренного движения и изменения wheel ownership. Численный допуск settling/pose записать в fixture после измерения, не «на глаз норм».

## 3. Руль: тяжесть без запаздывающей каши

Мышь не создаёт физический force feedback. Ощущение тяжести формируется ограничением скорости рулевого механизма, прогрессивной чувствительностью, видимым поворотом руля/рук, трением шин/звуком и передачей нагрузок. Нельзя обещать пользователю механическое сопротивление обычной мыши.

Предлагаемое развитие `FPinkCabSteeringController`: сохранить virtual cursor и sign convention, сделать переход stationary→rolling зависимым от малой **фактической скорости качения**, не от газа. На 0 тяжело; на 1–5 km/h должно отчётливо легче; на больших скоростях предсказуемая точность без резкого роста угла/раскачки.

Новые calibration поля, если существующих недостаточно: `RollingAssistStartKmh`, `RollingAssistFullKmh`, `MaxSteerRateStationaryDegPerSec`, `MaxSteerRateRollingDegPerSec`, `MaxSteerRateHighSpeedDegPerSec`. Это rate map реального actuator, не скрытый steering assist. Измеренная input latency и settling time ограничены; при смене направления не остаётся очередь mouse delta.

Форма интерполяции (proposal): `alpha = 1 - exp(-ResponsePerSecond * dt)`; команда и rate limit учитываются один раз. Delta counts не умножать на dt второй раз. Если накопитель достигает края, лишние counts не превращаются в будущий довод после возврата мыши.

Матрица ручной оценки: скорость 0/1/3/5/20/60/120/160 km/h; dry/wet; normal/reverse; 30/60/120 FPS; короткий рывок, длинный sweep, быстрое контрруление, остановка мыши. Одинаковый input replay сравнивает target/actual traces с заранее определённым допуском. 5 cm на столе зависит от DPI: вводить calibration «сколько counts даёт комфортный ход», а не магическое обещание физических сантиметров.

Проводить A/B/C: current / heavy-standstill-fast-rolling / менее резкий high-speed. Менять не более 2–3 связанных параметров за эксперимент. Владелец выбирает по ощущениям; no ABS/ESP/yaw rescue/auto-countersteer сохраняются.

## 4. Двигатель и трансмиссия

- 8500 RPM — доступный предел высокооборотистого принятого профиля при нужных условиях, а не команда RPM=8500 при любой нагрузке и 100% педали.
- Обороты в N растут по инерции двигателя; на передаче связаны с torque/load/slip; в высокой передаче с малой скоростью возможны lugging/RPM drop/stall.
- `P(kW)=T(Nm)*RPM/9549.2966`; финальный dyno table содержит 500-RPM samples, peak torque, peak power, limiter, idle, units. `MaxPowerHp` сам ничего не ограничивает в Chaos.
- Уточнить открытое противоречие: более новый engine target 250 hp/8500 соседствует с legacy `TerminalTargetKmh=195`. Не вводить невидимый speed cap. Согласовать gearing/aero/terminal target в VEH-04 и H-VEH.
- Gearing: wheelRPM = speedMps/(2πrM)*60; coupledRPM = wheelRPM*abs(gearRatio)*finalDrive. Проверить метр/сантиметр, RPM/rad/s, знак R, actual radius.
- Partial clutch torque и stock fully coupled driveline не должны складываться дважды. Один authoritative torque path на каждом состоянии, continuity на границе coupling.
- Сцепление 0..1, slip heat `Pslip≈abs(Tclutch*DeltaOmega)`, температурная/износная модель bounded. Нет нагрева заднего тормоза от RPM, пока колесо неподвижно и сцепление разомкнуто.
- Доступны slow reverse на малом газе и мощный wheelspin при достаточном газе; проценты 25/50/100 проверяются в fixture при известном grip, load, clutch и brake. Не скриптовать drift от `Throttle>.5`.
- Испытания старта, переключения, R, торможения и restart выполняются после исправления контактов. Иначе калибруется ошибка collision.

## 5. Подвеска, масса, поверхности

Разделить physical chassis и presentation expression. Неровность дороги меняет wheel contact и подвеску. Camera shake/звук лишь выражают это. Для мелких швов допустима presentation-only вибрация с явным SurfaceId; её не выдавать за физический ухаб.

Трасса калибровки: гладкий разгон; широкая волна; один bump на обе оси; левый wheel-only bump; patch edge; shallow rut; camber; мокрый участок; controlled curb; торможение на split surface. Геометрия и скорости версионированы. Замерить compression peaks, rebound settling, wheel unload, yaw disturbance, stopping distance. Обычная трещина не бросает машину в случайную сторону; высокая скорость усиливает физические последствия, не случайный input noise.

## 6. Камера меню

В visual profile измерить `RearSeatLocalTransform` по заднему пассажирскому месту и `DriverSeatLocalTransform` по существующему driver head. Локальная траектория внутри салона, выбранный взгляд вперёд, отсутствие прохода через headrest/крышу/руль. Quaternion rotation interpolation, плавное ускорение/замедление, никакой привязки к нижнему chassis origin.

`BlendDurationSeconds=2.0` допустимо только как EXPERIMENTAL seed. Нужен watchdog завершения и обработка repeated Start, Escape, pause, focus loss, low FPS. Во время blend driving inputs либо явно блокированы до готовности, либо согласованно разрешены; для G1 proposal — выдавать управление после blend. Idle physics already settled. При Continue из pause камера возвращается из pause flow без вступительной поездки с заднего места.

## 7. Звук автомобиля — 100% G1

Один `UPinkCabVehicleAudioComponent` в runtime/composition либо расширение найденного аналога. Он читает `FPinkCabVehicleAudioFrame`, не нажимает газ и не считает второй двигатель. Звуковые samples подбираются по RPM **и нагрузке**. Простое повышение pitch холостого loop до 8500 не проходит sound gate.

| Слой | Входы | Поведение |
|---|---|---|
| Starter/ignition/stall | Engine state events | One-shot, release, failed crank, hot restart без дублей |
| Engine on-load/off-load | RPM, load, throttle derivative | Crossfade диапазонов idle/low/mid/high/redline; no gap/click |
| Air-cooled fan / compressor / intake / exhaust | RPM/load | Раздельная читаемость без клиппинга; компрессор соответствует принятому образу |
| Transmission / gear conflict | actual engagement, mismatch, event serial | Click/clunk/grind только по причинам, не на каждом Tick |
| Clutch / lugging | slip power, load, engine health | Subtle load cues; thermal warning отдельно |
| Tires | contact, speed, slip, wetness, surface | Rolling dry/wet; skid progression; landing/impact, отсутствие squeal в воздухе |
| Suspension / body / cabin | contact impulse, travel, chassis accel | Bounded rattles/thumps, без одинакового треска каждый кадр |
| Controls | control event serial/value | Rotary detents, key, doors, handbrake latch, shift, windows, wipers, switches |
| Radio | power, volume, frequency, band | Static/capture crossfade; approved local stations, no external stream dependency |
| Interior/exterior mix | camera, glass/windows/doors | Обоснованная фильтрация дороги/двигателя, плавные переходы |
| Warnings | authoritative faults | Distinct warning events, silence/reset rules, не постоянная какофония |

При audio gate нужен ненулевой лицензированный cue для каждого активного события; procedural noise/click допустимы, если приняты слухом. Реестр хранит `CueId,SourceAsset,LicenseId,LoopPoints,BaseRpm,Layer,Bus,ConcurrencyGroup,Priority,MaxInstances,ExpectedTrigger`. WAV PCM 48kHz/24bit — предлагаемый формат исходников, mono для spatial loops, stereo для music/bed. Измерять streaming decode/memory, не загружать все станции в PCM одновременно.

Проверка: 10 минут реальной записи interior+exterior, idle→limiter→coast→shift→stall/restart→reverse→brake→radio; clipping 0 в выходном capture, щелчки loops 0, active voices/emitter counts выходят на plateau. При master mute телеметрия и input не меняются. Sound 100% означает готовый функциональный/слуховой набор G1; расширение музыкального контента позже — отдельный scope.
