<!-- RECOVERED_VERBATIM from File Library source PINKCAB_G1_Master_Plan.md on 2026-09-20.
Current Git/Jira/Confluence authority supersedes conflicting historical planning text. -->

# Город — от нынешней карты до G1 и полного продукта

## 1. Что означают рисунки

`1.png`: схематическая сложная развязка вокруг продолжающейся продольной оси, крупный внешний контур и внутренние петли. Цвета без легенды не превращаются в новые законы полос. Из текущего контракта берутся right entry, two-lane one-way pipe, loops/oval/figure-eight, reversal и проходы через мегаблок. Это топологический референс, не готовая engineering centerline.

`1_уровень_прямая _низ.png`: вид сверху нижнего слоя. Центр — разделение потоков/зелёный пояс, затем скоростные полосы, разделительная полоса и местный доступ, micromobility, пешеходы и панельные дома. Подземные паркинги входят со стороны местного проезда. В мире съезды и двери по разным сторонам не обязаны стоять напротив — это явно сказано в `отчерк.docx`.

Сечение на направление **от фасада к центру**: frontage/тротуар и доступ → micromobility strip → legal parking/local edge → 2 local-access lane → separation/service belt → 5 express lane → median/poplar ribbon; затем противоположная половина. Точные ширины — отдельный signed profile, не извлечение из размера PNG.

## 2. Что должно быть при G1

Обязателен работающий ground corridor, выразительный панельный тоннель, физически читаемые ухабы, навигационная инфраструктура, right-side entry/exit, один parking/service node и одна представительная interchange family. Есть понятные окна/огни домов, реклама, мокрый асфальт, дешёвые тени, дальняя глубина; это уже город PINK CAB, а не набор кубов на пустой плоскости.

В G1 верхние конструкции допустимы как видимый shell. **Wallride, magnetic ceiling, freight ceiling simulation и Level2 не объявляются готовыми**. Для них есть задачи следующей половины города и совместимые graph/asset seams. Это соблюдает текущую точку STOP «пока 1 уровень низ», не удаляя канон из полного плана.

## 3. Последовательность

| Шаг | Игроко-видимый результат | Проверка |
|---|---|---|
| C0 | Одна измеренная секция с правильными полосами и габаритами | Sweep самого широкого adopted traffic/taxi по lane envelopes |
| C1 | На ней едет и качается настроенная Tatra | Физические контакты на стыках/неровностях, steering feel |
| C2 | Непрерывная дорога с подъёмами/поворотами | Seam continuity, curvature/grade limits, lane graph match |
| C3 | Справа съезд в местный проезд/паркинг и возврат | Route connectivity, signs, safe merge, vehicle clearance |
| C4 | Одна сложная снаружи, понятная внутри развязка | Reversal/loop/oval/figure-eight route tests |
| C5 | Повторяемый личный город впереди/сзади | CityCode + versions + independent deterministic seeds |
| C6 | Трафик из одной модели, сигналы/разметка и правила | Gap checks, merge, braking, no overlap, AI provisional |
| C7 | Черновая художественная плотность и звук | Presentation Zero excerpt; реальная сцена движется |
| C8 | 30 минут + 2 часа без растущего мусора/провала | Counters, async cancellation, long-range precision |
| STOP | G1 и city score 50/100 | Владелец видит и принимает билд |

## 4. Модули дороги и инфраструктуры

Проектируемый kit: straight, gentle left/right, crest/dip, bank transition, five-lane road bed, two-lane local bed, deceleration, exit throat, acceleration, merge, service pocket, parking mouth/ramp/interior/return, gutter/drain, curb/drop curb, joint/seam, rail/barrier ends, median/poplars/hedges, building fronts/entrances, pedestrian bridge, micromobility lane, sign supports, maintenance pocket.

Все sockets имеют stable ID, direction/tangent/up, LaneId connections, legal motion, exclusion envelope и content version. Стройка piece-by-piece не может оставлять невидимые ступени collision. Async collision создаётся **до** допуска машины. Decorative trim не имеет blocking collision внутри swept envelope.

### Неровности

`SurfaceId` и PatchId объединяют geometry/collision/physical material/audio/VFX. Семейства: worn asphalt, patch, crack/seam, rut, shallow pothole, broad undulation, wet/puddle, metal grate, concrete ramp, debris shoulder. Глубокие ямы размещаются с seed и warning/avoidance semantics; не возникают под колесом после стриминга. Traffic считывает тип допустимого поведения, а не всеведущий teleport dodge.

### Знаки, разметка, enforcement

Полный набор G1: направление/разрешённые полосы; exit advance; branch destination; speed zone; speed repeat/end; lane ends/merge; no entry; one way; priority/yield at local merge; parking allowed/prohibited; service/garage/fuel; parking clearance; curve/bump warning; camera type/sign. Lane lines: solid/broken, edge, separation, chevrons, directional arrows, merge arrows, stop/yield only там где выбранная road grammar требует (нет глобальных светофоров), parking rectangles/exclusion hatch, ramp edge, reflectors.

Знаки — presentation `FPinkCabRoadRuleRecord`, а не отдельный набор правил. AI и camera enforcement читают тот же record. Нельзя брать случайные российские decal из D: и объявлять их нормой вымышленного мира. Использовать читабельные black/orange/white faces и отдельно approve typography. Числа `140+20`, `70+20`, поток 135 из legacy — reference, не автоматически действующий speed tolerance. Штрафы и камера связываются с существующими owners после reconciliation.

## 5. Развязки: «паук снаружи, понятная дорога внутри»

Стартовая family `Interchange_RightEntry_Pipe_A` (PROPOSED ID): express spine сохраняется; правый decel → throat → односторонняя двухполосная pipe; graph branches обеспечивают продолжение, смену направления, loop, oval и figure-eight. Как минимум один участок проходит сквозь фасад/объём дома с видимыми лестницами/балконами; без полного интерьера здания.

Не строить mesh из невалидного случайного spline и потом «придумывать правила». Сначала typed graph template, validated cross-section/tangent/curvature, swept vehicle clearance, затем mesh/material dressing. Пересечения в плане имеют разные elevations либо legal junction, никогда случайные overlapping collisions.

Контрольные параметры DEC-04: `ApproachLengthCm,MergeLengthCm,PipeInnerWidthCm,PipeClearanceHeightCm,MaxBankDeg,MaxGradeDeg,MinCurveRadiusCm,SpiralPitchCm,BranchDecisionDistanceCm,NearMissClearanceCm`. Безопасная длина решения зависит от скорости/торможения: `d >= v*tDecision + v²/(2*aDesign) + margin`; это инженерный расчёт, не дорожный юридический норматив. Проверить wet stopping envelope.

G1 variant не обязан содержать все мыслимые пауки; он обязан доказать reusable family и перечисленные route behaviors. В tube sparse non-zero traffic, общий solver Tatra, fixed enforcement cameras в core нет по канону; normal approach enforcement и collision/damage остаются. Десять кругов одного oval не могут выгрузить текущую геометрию от ошибочного продольного индекса.

## 6. Бесконечность и стриминг

Существующий `FPinkCabCityIdentity` сохраняет CityCode/GeneratorVersion/ContentSetVersion. Граф, rules, static recipe детерминированы; динамические traffic/passenger streams имеют отдельные seeds/counters. Смена одного декоративного material не должна переставлять перекрёстки.

**Выбор технологии:** сначала развить существующий bounded materializer. World Partition отвечает за стриминг authored spatial cells, а не автоматически генерирует бесконечную дорогу; не заменять им проверенный logical generator ради названия. PCG рассматривать для декора по валидному recipe после proof, не как authority маршрутов. [Epic: World Partition](https://dev.epicgames.com/documentation/en-us/unreal-engine/world-partition-in-unreal-engine), [Epic: PCG Overview](https://dev.epicgames.com/documentation/en-us/unreal-engine/procedural-content-generation-overview).

Предлагаемая contract sequence: Requested → Generating → CollisionReady → Visible → Retiring → Pooled. Job carries GenerationToken; результат устаревшего поколения не вставляется в сцену. Recycle отменяет pending work, снимает event bindings, audio/VFX, collision, reservations и material references; не очищает persistent city deltas.

Lookahead = не фиксированное красивое число: `abs(speed)*LookAheadSeconds + brakingDistance + worstCaseGenerationMargin`, minimum floor и hard cap. Направление движения, reversing, loop branches, камеры/зеркала и return from ServiceNode учитываются через несколько pins/streaming sources. Branch prefetch заранее, но число соседей bounded. Текущий chunk и path-to-exit нельзя выгрузить по расстоянию до мировой origin.

World ID содержит logical chunk indices 64-bit; позиция near player локальная. У UE есть Large World Coordinates, но большие координаты всё равно проверять на precision физики, particles, audio и внешних floats. Выбор rebase/segment-local решения доказать тестом вдали и возвратом, не ломая saves и road IDs.

Boundedness: MaxActiveChunks, MaxJobsInFlight, MaxNearTraffic, MaxDecorActors, MaxEmitters, MaxDynamicLights, MaxVfxInstances; memory после warm-up выходит на плато. Fault injection: slow storage, delayed generation, failed asset, quick reversal, повторный enter/exit, shutdown while job pending. При нехватке geometry ahead нельзя разрешать выезд в пустоту: предсказуемая загрузочная пауза/безопасный streaming barrier как крайняя техническая защита, зафиксировать как failure normal experience.

## 7. Трафик G1

Один **проверенный по правам** sedan mesh с вариациями material/цвета, wheels/lights/brake/indicator states. Модель выбирается из inventory; имя скачанного файла не угадывать. Логический lane traffic отделён от near physical actors. Не создавать Chaos solver для каждой далёкой машинки.

Обязательные функции: stable entity ID; lane following; speed profile; safe headway; blocking occupancy; right merge/exit; obstacle braking; lane change reservation; deterministic spawn out of sight/overlap; near/far handoff без jump; stuck recovery без teleport перед игроком; recycling. No normal traffic lights и no systemic standing jam на express — действующий canon. После столкновения возможен локальный slowdown с moving bypass. AI personality «наглость/медленный левый/мотоцикл между рядов» — после G1.

## 8. Следующие 50% и продукт

План POST-G1 хранит задачи, но не разрешает их самовольно выполнять после STOP:

1. **City 75:** полноценная remaining L1 breadth: debris gameplay; seven-band wallride; freight ceiling exactly five lanes и opposite flow; magnetic contact/reacquisition; poplar crossing windows; incidents; Practice Hangar.
2. **City 100 current-release:** Level2 stack, two metro tracks per side, bus flow/stations, L1↔L2 links, сервисы/такси в обоих слоях, окончательная traffic family/AI, dense presentation budget.
3. **FIRST EURO release:** asset/license closure, final mix/art replacement, localisation/accessibility/settings, save migration/recovery, two-hour near-final soak, distribution/credits/store requirements.
4. **Post-FIRST-EURO:** Level3, full Neural feed/credentials/government, bars/malls/live social rooms, multiplayer/online vehicle sync. Никакой скрытой реализации этих систем в G1.

Верхние старые формулы/fixture masses хранятся как baseline для следующего milestone; прежде применения проверить действующую mass/vertical authority. Не копировать FGear/VDS и fixed-five-second остаток из раннего docx.
