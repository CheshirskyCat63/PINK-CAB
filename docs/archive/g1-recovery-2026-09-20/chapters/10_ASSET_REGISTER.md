<!-- RECOVERED_VERBATIM / structurally stitched from exact File Library chunks of PINKCAB_G1_Master_Plan.md on 2026-09-20.
Current Git/Jira/Confluence authority supersedes conflicting historical planning text. -->

# Полный реестр потребностей в ассетах

Это список для инвентаризации и поставки, не список уже лицензированных файлов. Counts — минимальные proposed варианты контента, **не** число runtime actors. Сначала ASSET-01 сверяет существующие папки; владелец предоставляет только действительно недостающее. Общие reusable textures закрывают несколько строк.

Статус каждой строки до inventory: NOT_MATCHED_YET; права требуют evidence. Условные adopted детали из cabin matrix могут стать N/A только по явному DEC-02; удаление строки без решения запрещено. Полный будущий L3/online контент определяется отдельно в POST-08, поскольку механики ещё не приняты.

Реестр: **159 семейств/позиций**, из них **138 G1**.

## Обязательные метаданные каждой поставки

AssetId; source URL/file+hash; author/license/allowed use; original format/units/axis; semantic slots; triangle/material/texture counts; near/mid/far strategy; collisions; animation/audio event contract; consuming TaskId; validation evidence. Для source без прав — blocked, не импортировать в distributable.

## MOD

| AssetId | Что нужно | Минимум вариантов | Этап | Назначение / приёмка | Task |
|---|---|---|---|---|---|
| MOD-CAR-01 | Hero/donor body shell | 1 | G1 | Законченная внешняя оболочка Tatra с заменяемым visual profile. Силуэт, масштаб, positive transforms, admitted license | ASSET-03 |
| MOD-CAR-02 | Полный салон | 1 | G1 | Dashboard, пол, потолок, стойки, двери, задние места. Нет дыр из rear-seat/driver cameras; authored positions сохранены | CAB-01 |
| MOD-CAR-03 | Donor road wheels | 4 placements/1 set | G1 | Шина+диск FL/FR/RL/RR. Wheel centers/rotating axis/radius match profile; шины видимы | VEH-02 |
| MOD-CAR-04 | Steering wheel/column | 1 | G1 | Отдельный руль с hub pivot. Logo readable, correct axis, no mirrored geometry | CAB-02 |
| MOD-CAR-05 | Gear lever/gaiter | 1 | G1 | Рычаг с authored pivot/neutral/rest. Reach всех 6 позиций и N без переноса в салоне | CAB-02 |
| MOD-CAR-06 | Handbrake lever/link | 1 | G1 | Короткий ход отдельного ручника. Actual travel/analog state, no second fake lever | CAB-02 |
| MOD-CAR-07 | Pedals | 3 | G1 | Q clutch/W brake/E throttle meshes. Отдельные axes и rest/max poses | CAB-02 |
| MOD-CAR-08 | Gauge needles and faces | speed/tach/fuel/oil-temp + fitted | G1 | Индивидуальные needles, шкалы, tells. Диапазоны соответствуют 8500 и game telemetry | CAB-04 |
| MOD-CAR-09 | Doors locks handles | все adopted doors | G1 | Moving doors, handles, pins, hinges. Stop pickup interlock и clearance, door collision policy | CAB-08 |
| MOD-CAR-10 | Glass and winders | windshield/rear/side + 2 winders | G1 | Раздельное стекло и ход окон. Transparent two-sided review, sane specular, no opaque cabin | CAB-06 |
| MOD-CAR-11 | Wipers washer | 1 pair + nozzles | G1 | Щётки/рычаги/nozzle anchors. Wipe arc/park, не пересекают стекло | CAB-06 |
| MOD-CAR-12 | Mirrors | rearview + L/R | G1 | Корпус/отражающая плоскость. Useful rear sightlines, bounded capture cost | CAB-07 |
| MOD-CAR-13 | Dashboard rotary knobs | volume/tune/dimmer/HVAC | G1 | Отдельные grip knobs. Axes/detents и collision hit targets | CAB-08 |
| MOD-CAR-14 | Switches and buttons | по adopted ControlId | G1 | Ключ, свет, hazard, horn, wash, taxi. Каждый visible control связан с effect и звуком | CAB-01 |
| MOD-CAR-15 | HVAC trim/vents | 1 kit | G1 | Воздуховоды/заслонки/lever trim. Fitted semantics согласованы, нет выдуманного water radiator | CAB-08 |
| MOD-CAR-16 | Storage/accessories | glovebox/ashtray + adopted | G1 | Крышки, latch, slider, прикуриватель при принятии. Механический ход/persistence определён | CAB-08 |
| MOD-CAR-17 | Seats rear cabin | front pair + rear | G1 | Сиденья и переход камеры. Не закрывают driver view и reach; заднее место пригодно для меню | CAM-01 |
| MOD-CAR-18 | Hands arms rig | 1 character rig | G1 | Обе руки, fingers, correct skeleton. Reach/Q staging/RMB grip без body clipping | CAB-03 |
| MOD-CAR-19 | Exterior lamp optics | head/center if adopted/tail/brake/reverse/turn | G1 | Lenses and emissive submeshes. Состояния разделены; центр follows actual steering when adopted | CAB-05 |
| MOD-CAR-20 | Taxi equipment | meter/nav/payment/receipt | G1 | Diegetic physical work devices. Устройства принадлежат existing fare/economy states | CAB-08 |
| MOD-CAR-21 | Collision hull and wheel anchors | 1 versioned set | G1 | Physics-only hulls, anchors, debug representation. No chassis penetration/self wheel hits, measured offsets | VEH-02 |
| MOD-CAR-22 | Damage overlays/parts | по adopted health channels | G1 | Glass/lamp/tire/door readable damage presentation. Не обещать full deformation; state→symptom присутствует | VEH-09 |
| MOD-CAR-23 | Hero final 603 body/cabin | 1 | POST_G1 | Окончательная approved bespoke 603 geometry. Marathon только permission; physics отделены | POST-01 |

**Формат:** Blender+FBX/glTF; authored pivots, LOD/collision notes.

## ANIM

| AssetId | Что нужно | Минимум вариантов | Этап | Назначение / приёмка | Task |
|---|---|---|---|---|---|
| ANIM-01 | Hand steering poses | left/right/full range | G1 | Руки держат actual rotating wheel. Нет auto-steer, clipping, single update owner | CAB-03 |
| ANIM-02 | Hand reach transitions | каждый adopted reach group | G1 | Reach→grip→operate→release→wheel. Interrupted transitions smooth, no hand teleport | CAB-03 |
| ANIM-03 | Q gearbox staging | idle/stage/cancel | G1 | Подготовка правой руки до grip. Q не отключает steering | INP-04 |
| ANIM-04 | Gear shifting gestures | 1..5/N/R | G1 | Позиции рычага/кисти. Actual gate path and neutral corridor | INP-06 |
| ANIM-05 | Handbrake gesture | pull/hold/release | G1 | Короткий continuous ход. Состояние latch/return совпадает | VEH-07 |
| ANIM-06 | Control actuator library | press/toggle/rotary/slider/hinge | G1 | Параметрические local animations. 100 cycles no drift, rest transform stable | CAB-02 |
| ANIM-07 | Doors/windows/wipers | по adopted parts | G1 | Open fraction, wiper phase. Совпадает с physics/effect, interrupt rules | CAB-06 |
| ANIM-08 | Gauge animation profiles | по scale faces | G1 | Needle curve mapping. 5-point sweeps and actual value | CAB-04 |
| ANIM-09 | Traffic wheel/light motion | 1 family | G1 | Spin/steer, lamps state. Correct travel direction, no hidden chassis teleport | TRA-01 |
| ANIM-10 | Crowd/courier vignettes | 6 base loops | POST_G1 | Курьер/двор/скамья/ремонт/балкон/мост. Bounded shared animation, no full NPC schedules | POST-04 |

**Формат:** Rig/source clips or versioned parametric curves.

## ROAD

| AssetId | Что нужно | Минимум вариантов | Этап | Назначение / приёмка | Task |
|---|---|---|---|---|---|
| ROAD-01 | Express straight module | 1 parametric | G1 | 5 полос на направление. Graph/collision/marking sockets agree | CITY-02 |
| ROAD-02 | Local access road | 1 parametric | G1 | 2 полосы на направление. Service belt и legal parking preserved | CITY-02 |
| ROAD-03 | Gentle turns | left/right | G1 | Плавные кривые магистрали. Minimum radius/tangent seam validated | CITY-02 |
| ROAD-04 | Vertical profile modules | crest/dip/rise/fall | G1 | Плавная волнистость дороги. Grade, bottoming-out, contact continuity | CITY-02 |
| ROAD-05 | Bank transitions | flat→bank→flat | G1 | Повороты и tube connection. Roll continuity and swept clearances | CITY-02 |
| ROAD-06 | Deceleration right entry | 1 per road class | G1 | Advance sign, lane transition, throat. Enough decision distance; no left ordinary entry | CITY-05 |
| ROAD-07 | Acceleration merge | 1 per road class | G1 | Return lane and joining. Visibility/gap reservation and lane rules | CITY-05 |
| ROAD-08 | Parking mouth/ramp | 1 family | G1 | Подземный въезд/спуск/стены. Taxi clearance, signs, curb seam | CITY-05 |
| ROAD-09 | Parking interior/return | 1 bounded room | G1 | Остановка/service/return route. Same-city return and free safe exit | SYS-04 |
| ROAD-10 | Pipe straight/banked curves | 1 modular kit | G1 | Двухполосная односторонняя труба. Inner width/height, traffic overtaking envelope | CITY-06 |
| ROAD-11 | Pipe branch/join | 2 junction types | G1 | Loop/reversal/oval/8 connectivity. No invalid collision at graph crossing | CITY-06 |
| ROAD-12 | Spiral/tier stub | 1 visual capped stub | G1 | Future transfer seam within interchange. Недоступный stub подписан/закрыт; no fake L2 readiness | CITY-06 |
| ROAD-13 | Building penetration sleeve | 1 family | G1 | Проезд через мегаблок. Trim/collision excludes legal driving volume | CITY-07 |
| ROAD-14 | Curb/drop curb/gutter | 3 modules | G1 | Edges, drainage, service entry. Wheels/impact readable, no invisible curb at seam | CITY-02 |
| ROAD-15 | Drains/grates/manholes | 3 mesh faces | G1 | Utility road details. Surface event sane, no enormous triangle/material cost | CITY-03 |
| ROAD-16 | Joints/patch lips | 3 physical profiles | G1 | Measured small defects. Collision shape matches visible feature | CITY-03 |
| ROAD-17 | Rut strips | 2 controlled profiles | G1 | Колейность и smooth transition. Predictable influence, no random steering input | CITY-03 |
| ROAD-18 | Pothole/broad bump | 2 families x shallow/deep | G1 | Physical roughness. Speed/load response validated; safe noncritical route remains | CITY-03 |
| ROAD-19 | Barrier ends/connectors | straight/end/corner/repair | G1 | P-6V-like concrete road edge. Visual PO-2 candidate audited separately | CITY-08 |
| ROAD-20 | Reflectors/bollards | 2 families | G1 | Edges/conflict zones. No spawn inside drive envelope; readable at night | CITY-08 |
| ROAD-21 | Pedestrian bridge | 1 modular | G1 | Над parking ramp. Headroom and pedestrian visual support | CITY-05 |
| ROAD-22 | Micromobility strip | 1 modular | G1 | Outer separate lane. Не ошибочно принята за авто-полосу | CITY-01 |
| ROAD-23 | Chain-link wallride shell | panel/rib/curve/connectors | POST_G1 | 7-band visual family. Cheap collision proxy; wire not gameplay cloth | POST-02 |
| ROAD-24 | Freight ceiling road/strips | 5 aligned lanes | POST_G1 | Opposing upper layer. Stable IDs, receiving strip collision | POST-03 |
| ROAD-25 | Freight docks/service gaps | 3 modules | POST_G1 | Non-player dock apertures. No accidental drivable route; clear collision semantics | POST-03 |
| ROAD-26 | Poplar crossing aperture | 2 mirrored modules | POST_G1 | Ceiling side-swap landing. Approach/flight/landing all validated | POST-03 |
| ROAD-27 | Practice Hangar | 1 modular room | POST_G1 | Floor/wall/ceiling training. Return same vehicle/city, magnet semantics accepted | POST-04 |

**Формат:** Modular mesh/source+collision+socket metadata.

## SIGN

| AssetId | Что нужно | Минимум вариантов | Этап | Назначение / приёмка | Task |
|---|---|---|---|---|---|
| SIGN-01 | Speed zone faces | limit/repeat/end | G1 | Profile-generated speed values. SignFaceId and RuleRecord match | CITY-04 |
| SIGN-02 | Lane direction faces | straight/right/exit/merge | G1 | Ordinary navigation. Correct mirroring each direction | CITY-04 |
| SIGN-03 | Entry/priority faces | one-way/no-entry/yield | G1 | Local roads/pipe/parking. Не создают traffic light system | CITY-04 |
| SIGN-04 | Warning faces | curve/bump/clearance/lane-end | G1 | Предупредить до decision distance. Typography readable from driver at speed | CITY-04 |
| SIGN-05 | Destination faces | parking/service/garage/fuel/route | G1 | Work navigation. Stable place ID, localisation layout | CITY-04 |
| SIGN-06 | Stopping/parking faces | allowed/prohibited/zone end | G1 | Legal parking edge. Parked spawns respect rule zones/exclusions | CITY-04 |
| SIGN-07 | Camera/enforcement faces | speed/lane/type marker | G1 | Learnable camera rules. No fixed core pipe cameras | TRA-05 |
| SIGN-08 | Posts/gantries/brackets | small/overhead/wall | G1 | Reusable sign supports. Collision below clearance only if valid obstruction | CITY-04 |
| SIGN-09 | Camera units | 1 + support | G1 | Visible enforcement source. Rule event and face consistent | TRA-05 |

**Формат:** Vector/PNG atlas+mesh support+RuleId mapping.

## MARK

| AssetId | Что нужно | Минимум вариантов | Этап | Назначение / приёмка | Task |
|---|---|---|---|---|---|
| MARK-01 | Lane lines | solid/broken/double/edge | G1 | Road legal language. High contrast over rough asphalt, distance readability | CITY-04 |
| MARK-02 | Arrows/chevrons | straight/right/merge/split | G1 | Lane decision language. Road graph direction matches painted direction | CITY-04 |
| MARK-03 | Parking/exclusion marks | bay/hatch/clearance | G1 | Pockets and no-parking areas. AI parking placement aligns | CITY-05 |
| MARK-04 | Ramp/pipe edge treatment | 2 patterns | G1 | Near-miss readability. No z-fighting/floating decal | CITY-07 |

**Формат:** Vector/PNG/decal+RoadRule mapping.

## WORLD

| AssetId | Что нужно | Минимум вариантов | Этап | Назначение / приёмка | Task |
|---|---|---|---|---|---|
| WORLD-01 | Panel megablock facade | 3 reusable bay variants | G1 | Continuous city wall. Silhouette coherent, thin frontage, no full city interiors | CITY-08 |
| WORLD-02 | Building corners/seams/roof | 1 modular kit | G1 | Connection and skyline. HLOD silhouette doesn't pop visibly | CITY-08 |
| WORLD-03 | Entrance/canopy/door | 3 variants | G1 | Residential rhythm. Deterministic sockets, no deep NPC schedules | CITY-08 |
| WORLD-04 | Service/civic frontage | school/maintenance/utility slices | G1 | Busy thin edge. No demand to build full institutions | CITY-08 |
| WORLD-05 | Balconies/fire stairs/galleries | 1 kit each | G1 | Pipe through houses visual identity. NearMissClearance preserved | CITY-07 |
| WORLD-06 | Utility cabinets/vents/pipes | 4 reusable pieces | G1 | Infrastructure detail. Instanced/grouped, no per-object tick | CITY-08 |
| WORLD-07 | Benches/bins/playground fragment | 1 each + variants | G1 | Human scale. Cheap clusters, outside drive envelope | CITY-08 |
| WORLD-08 | Giant leafless poplars | 2–3 close silhouettes | G1 | Monumental repeated median. Near-identical canonical rhythm; not random forest | CITY-08 |
| WORLD-09 | Hedge segments | straight/end/junction | G1 | Clipped median. Instanced, bounded materials | CITY-08 |
| WORLD-10 | Artificial grass/dandelions | 2 card atlases | G1 | Bright median accents. Controlled masked overdraw and LOD | CITY-08 |
| WORLD-11 | Debris clusters | bags/bottles/cans/paper/boxes | G1 | Rich shoulder dressing. Static clusters/instances; no hundreds rigid bodies | CITY-08 |
| WORLD-12 | Debris gameplay representation | transition volume + feedback | POST_G1 | Road↔wallride handoff. No random forces from individual visual trash | POST-02 |
| WORLD-13 | Projector/prism ad hardware | 2 mount types | G1 | Светящаяся реклама на барьерах/домах. Emissive visual and actual light budgets separate | CITY-09 |
| WORLD-14 | Ad creative sheets | 6 original loop families | G1 | Credit/devices/food/service/entertainment. Own or licensed content; readable sequential frames | CITY-09 |
| WORLD-15 | Crowd silhouette cards | 3 grouped silhouettes | G1 | Pedestrian bridge/frontage life. Cheap near/mid/far and repeat cooldown | CITY-10 |
| WORLD-16 | Scooters/e-bikes/courier bags | 2–3 per vehicle family | POST_G1 | Micromobility life. Bounded simple actors/animations | POST-04 |
| WORLD-17 | Interchange vignettes | smoke/cook/argue/laundry/TV/repair | POST_G1 | Building penetration life. Slots deterministic, no props in collision envelope | POST-04 |

**Формат:** Low-poly source+atlas/LOD/instancing metadata.

## TRAFFIC

| AssetId | Что нужно | Минимум вариантов | Этап | Назначение / приёмка | Task |
|---|---|---|---|---|---|
| TRAFFIC-01 | Traffic sedan | 1 admitted model | G1 | Единственная provisional family. Wheels/lights/collision envelope/LODs documented | TRA-01 |
| TRAFFIC-02 | Traffic appearance variants | 4 colors/1 mesh | G1 | Минимальная визуальная вариативность. Shared materials, no need 4 unique 4K sets | TRA-01 |
| TRAFFIC-03 | Passenger-car roster expansion | 3 silhouette classes | POST_G1 | Small/medium/long retrofuturist traffic. Owner-approved roster/license/budgets | POST-06 |
| TRAFFIC-04 | Motorcycle/sidecar | 1 each | POST_G1 | Lower-level vehicle canon. Distinct envelope and passing behavior | POST-04 |
| TRAFFIC-05 | Freight/container convoy | single + 3–5 unit compositions | POST_G1 | Opposite ceiling machinery. Stable convoy gaps and advertising sockets | POST-03 |
| TRAFFIC-06 | Bus/metro/station assets | 1 bus/1 train/station kit | POST_G1 | L2 transit stack. Tracks and clearance; explicit stream budget | POST-05 |

**Формат:** Mesh/rig+wheel/light anchors+simple collision.

## MAT

| AssetId | Что нужно | Минимум вариантов | Этап | Назначение / приёмка | Task |
|---|---|---|---|---|---|
| MAT-01 | Road master + asphalt atlas | 1 master/4 regions | G1 | Worn/patch/rut/wet-edge. UV density/tiling, wetness bounded, no giant repeats from wrong scale | ASSET-04 |
| MAT-02 | Road decals and marks atlas | 1–2 pages | G1 | Crack/seam/arrow/line/parking graphics. Color-space, alpha, mip readability | CITY-04 |
| MAT-03 | Concrete/barrier/grime atlas | 1–2 pages | G1 | Panel/P-6V/drains/runoff. Shared master, roughness not inverted | ASSET-04 |
| MAT-04 | Panel windows/frontage atlas | 1–2 pages | G1 | Lit/dark windows and building blocks. Emission intensity budget; no individual real light per window | CITY-08 |
| MAT-05 | Metal/chain-link master | 1 atlas + mask | G1 | Rails/vents/brackets/future wallride. Masked overdraw measured; no wire collision | ASSET-04 |
| MAT-06 | Vegetation atlas | bark/hedge/grass/flowers | G1 | Median identity. No PS1 corn assets from DEADCORN | CITY-08 |
| MAT-07 | Trash/prop atlas | 1–2 pages | G1 | Reusable dirty objects. Instancing eligibility, no per-prop unique materials | CITY-08 |
| MAT-08 | Sign face atlas/generation font | black/orange/white | G1 | All rule categories. Licensed font, legible sizes, versioned graphics | CITY-04 |
| MAT-09 | Vehicle exterior paint/trim | paint/chrome/rubber/plastic | G1 | Readable hero/traffic. Budget surfaces, no noisy mirror reflections | ASSET-03 |
| MAT-10 | Cabin surfaces | upholstery/plastic/metal/wood if fitted | G1 | Coherent animated interior. Source mapping stable, no whole cabin glossy default | CAB-02 |
| MAT-11 | Glass material | windshield/side/mirrors separated | G1 | Transparency and visibility. Inside/outside sort, rain/wiper integration | CAB-06 |
| MAT-12 | Instrument faces/needles/telltales | 1 atlas + emissive mask | G1 | Readable gauges at night. No wrong labels; contrast at driver FOV | CAB-04 |
| MAT-13 | Ads/hologram master | 1 animated sheet family | G1 | Vivid cheap city color. Emissive vs lighting separated, overdraw cap | CITY-09 |
| MAT-14 | People/clothing/crowd atlas | 1–2 pages | G1 | Grouped silhouettes. Licenses, cheap alpha/LOD | CITY-10 |
| MAT-15 | Damage/decal/VFX material | crack/scratch/spray/smoke | G1 | Adopted damage and weather feedback. No gameplay force; cleanup and correct alpha | CITY-10 |
| MAT-16 | Final material variation wave | per approved asset family | POST_G1 | Owner supplied art substitution. Golden-route regression after each family | POST-01 |

**Формат:** PNG/TGA source+material mapping+color-space/alpha.

## SND

| AssetId | Что нужно | Минимум вариантов | Этап | Назначение / приёмка | Task |
|---|---|---|---|---|---|
| SND-01 | Ignition key and starter | key-on/crank/start-fail/start-success | G1 | Physical ignition feedback. One event serial, no overlapping starters | AUD-02 |
| SND-02 | Engine idle | interior/exterior layers | G1 | Air-cooled V8 idle. Clean loop points, matching base RPM, rights | AUD-02 |
| SND-03 | Engine on-load RPM bank | low/mid/high/redline | G1 | Load-bearing acceleration. Crossfades cover full RPM range including 8500 | AUD-02 |
| SND-04 | Engine coast/off-load bank | low/mid/high | G1 | Release gas and engine braking. Differs perceptually from acceleration | AUD-02 |
| SND-05 | Fan/intake/exhaust/compressor | 4 layered cues | G1 | Canonical engine character. Bounded voices, no constant max whine | AUD-02 |
| SND-06 | Stall/limiter/lugging | 3 event/loop families | G1 | Mechanical feedback. Triggered by actual conditions | AUD-02 |
| SND-07 | Transmission whine/shifts | drive/coast/click/clunk | G1 | Manual driveline. Engaged gear/load, event dedup | AUD-03 |
| SND-08 | Gear grind/conflict | 1 bounded loop + stop | G1 | Refused/mismatched shift. No grind from hover alone | AUD-03 |
| SND-09 | Clutch/handbrake linkage | pedal/lever/latch/release | G1 | Physical controls. State-causal sound, no forced damage implication | AUD-04 |
| SND-10 | Tire rolling dry | asphalt/concrete/grate | G1 | Surface-specific speed bed. No loop in air; spatial mono or grouped | AUD-03 |
| SND-11 | Tire rolling wet/spray | 2 layers | G1 | Wet road response. Wetness/contact/speed-driven | AUD-03 |
| SND-12 | Tire skid | dry/wet/slip onset/sustained | G1 | Progressive breakaway. No 100% squeal from any wheel rotation | AUD-03 |
| SND-13 | Bumps suspension cabin rattle | small/medium/large impact + rattle | G1 | Mass and bad-road cues. Impulse threshold/cooldown, bounded repeats | AUD-03 |
| SND-14 | Body scrape/collision | metal/concrete/vehicle | G1 | Contact/damage. Severity mapped, no per-frame one-shot spam | AUD-03 |
| SND-15 | Switches buttons rotary detents | 3 families | G1 | Context interaction feedback. Each adopted event mapped; safe random pitch range | AUD-04 |
| SND-16 | Turn relay and horn | 2 families | G1 | Road signaling. Horn hold stops on release/focus loss | AUD-04 |
| SND-17 | Wipers and washer | dry/wet/speed states + pump | G1 | Glass system. Phase and park align, no loop after stop | AUD-04 |
| SND-18 | Window winding and doors | winder/open/close/latch/lock | G1 | Cabin access. Interior/exterior acoustic transition | AUD-04 |
| SND-19 | HVAC fan | off/low/high crossfade | G1 | Heater/ventilation. Audible speed difference; no thermal simulation invention | AUD-04 |
| SND-20 | Radio tuning/static/power | 3 families | G1 | Search station/volume. Smooth capture, not click per wheel delta | AUD-04 |
| SND-21 | Radio programme/music | 2 original/licensed station loops | G1 | Работающее радио без сетевого сервиса. Composition/recording rights clear, no unlicensed commercial songs | AUD-01 |
| SND-22 | Taxi meter/payment/receipt | 3 families | G1 | Work events. Exactly-once sound from accepted transaction event | AUD-04 |
| SND-23 | Warning cues | adopted faults | G1 | Engine/brake/service alerts. Priorities/concurrency, no constant overlap | AUD-05 |
| SND-24 | Road traffic ambience | near vehicles/mid group/far bed | G1 | Dense city cheap audio. Near→mid→far handoff bounded, not 1 emitter per object | CITY-10 |
| SND-25 | Tunnel wind/rain/frontage | 4 grouped beds | G1 | World spatial atmosphere. Interior filtering, no abrupt seams | CITY-10 |
| SND-26 | Ads/utility/crowd fragments | 6 admitted short cues | G1 | Thin theatrical city life. Cooldown, distance, priority, no intelligibility wall | CITY-10 |
| SND-27 | Freight/metro/bus audio | 3 machine families | POST_G1 | Upper infrastructure. Aggregated emitters, view/occlusion budgets | POST-05 |
| SND-28 | Wallride/magnet/debris audio | contact/instability/reacquire/detach | POST_G1 | Skill feedback. Actual state and accepted timer semantics | POST-03 |
| SND-29 | Passenger voice/character expansion | roster after approval | POST_G1 | Broader taxi narrative. Character approval, voice rights, localisation | POST-06 |
| SND-30 | UI minimal menu feedback | focus/confirm/back | G1 | Menu interaction. Separate bus, volume/mute respects settings | CAM-02 |

**Формат:** WAV PCM masters+loop/cue sheet+rights.

## VFX

| AssetId | Что нужно | Минимум вариантов | Этап | Назначение / приёмка | Task |
|---|---|---|---|---|---|
| VFX-01 | Rain field and local splashes | 1 pooled family | G1 | Wet-night atmosphere. Follow area bounded, no infinite spawned droplets | CITY-10 |
| VFX-02 | Wheel spray/dust | wet/dry profiles | G1 | Contact and speed feedback. No spray in air, surface controls material | AUD-03 |
| VFX-03 | Smoke/sparks/contact chips | 3 pooled families | G1 | Adopted collision/burnout. Rate limited, no simulated debris explosion per frame | CITY-10 |
| VFX-04 | Windshield water/wipe mask | 1 system | G1 | Working wipers/visibility. Clear arcs aligned to real blades, screen readability | CAB-06 |
| VFX-05 | Ads/hologram animation | 1 frame sequence system | G1 | Sequential city ads. Id/frame order deterministic; bounded lights | CITY-09 |
| VFX-06 | Future debris windshield impacts | paper/liquid/soft occlusion | POST_G1 | Wallride transition theatre. Visibility never permanent from leaked effect | POST-02 |
| VFX-07 | Magnet/contact feedback | 1 state family | POST_G1 | Clear residual/reacquire communication. Same authoritative state as gameplay | POST-03 |

**Формат:** Native UE material/Niagara source+pool limits.

## UI

| AssetId | Что нужно | Минимум вариантов | Этап | Назначение / приёмка | Task |
|---|---|---|---|---|---|
| UI-01 | System menu controls sheet | RU + scalable layout | G1 | Start/continue/settings/exit и context hints. No developer implementation text in user flow | INP-07 |
| UI-02 | Diegetic fonts/icons | 1 licensed family | G1 | Gauges/nav/meter/signs where applicable. Units/contrast/RU glyph support | CAB-04 |
| UI-03 | Build identity and diagnostic overlay | Development-only | G1 | QA reproducibility. Hidden from shareholder default view; exact SHA/BuildId | QA-06 |
| UI-04 | Commercial release media/localisation | per release scope | POST_G1 | Store/credits/accessibility/settings. Actual gameplay captures, truthful features | POST-07 |

**Формат:** Editable/vector/font license/localisation source.

## DATA

| AssetId | Что нужно | Минимум вариантов | Этап | Назначение / приёмка | Task |
|---|---|---|---|---|---|
| DATA-01 | Physical/control profile | 1 versioned nominal + experiments | G1 | All tuning in one owner source. Provenance/unit/default/range/QA for fields | VEH-04 |
| DATA-02 | Road/rule/interchange profiles | 1 coherent set | G1 | Geometry and logical city consistency. No duplicated numeric authority | CITY-06 |
| DATA-03 | Asset license BOM | 1 manifest | G1 | All model/material/audio/font rights. No UNVERIFIED entries in distributable | ASSET-02 |
| DATA-04 | Performance profile | 1 per accepted hardware tier | G1 | CPU/GPU/memory/stream/audio limits. Measured before freeze, versioned | QA-02 |
| DATA-05 | Save fixtures/migrations | current + supported old | G1 | Recovery and exactly-once. Corrupt/duplicate/old schema tested | SYS-03 |
| DATA-06 | Shareholder evidence pack | 1 immutable bundle | G1 | Proof all criteria. Hash-bound logs/video/telemetry and human decision | QA-07 |

**Формат:** Versioned JSON/UE data asset plus schema.

## Ограничения количества

Число рекламных вариантов, traffic colors, atlas pages и loops — стартовый production proposal. Число одновременно видимых/активных объектов задаётся QA-02 и streaming budget. Не умножать один emitter/real light/Actor на каждую видимую деталь. Не выдавать планы 1K–2K atlas за окончательный budget без profiling.

## Порядок поставки

1. Права и функциональные детали машины/руки/звук.
2. Дорога, collision kit, разметка/знаки, one traffic model.
3. Facade/median/props и shared materials.
4. Ads/light/ambient/VFX в бюджет.
5. После G1 — окончательная Tatra, artwork wave, оставшиеся уровни и traffic roster.

## Structural count check

`23 + 10 + 27 + 9 + 4 + 17 + 6 + 16 + 30 + 7 + 4 + 6 = 159`.
