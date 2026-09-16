# PINK CAB · Mechanics Freeze Entry Matrix

Date: 2026-09-16  
Owner task: `CD-847`  
Parent gate: `CD-841`  
State: ENTRY MATRIX COMPLETE / MECHANICS FREEZE NOT YET PASSED

## Gate result

Core code, canonical Git, playable build, vehicle authority and cockpit/input authority are reconciled. The remaining design surface is finite: **42 unresolved decision IDs** = `WF-01..20` + `NEU-01..22`.

Mechanics Freeze may be claimed only when every row below is either:

1. `LOCKED` with durable authority + objective acceptance; or
2. `EXCLUDED` from FIRST EURO with an explicit non-invasive extension boundary.

No generic `later`, undocumented default, or implementation-time designer/programmer invention is allowed.

Source inventories:

- `docs/PINK_CAB_MECHANICS_FREEZE_WORLD_CONTENT_INVENTORY.md` (`CD-845`, 20 rows)
- `docs/PINK_CAB_FULL_NEURAL_MECHANICS_FREEZE_MAP.md` (`CD-846`, 22 rows)

## Ordered decision backlog
| Order | ID | Scope | Current authority | Owner / next action | Implementation dependency | Freeze proof |
|---:|---|---|---|---|---|---|
| 1 | `WF-01` | FIRST EURO | L1 pages 33/37 + `CD-797/798` | World design: lock production geometry/density envelopes | World/Traffic/Vertical | data bounds cover every gameplay-bearing L1 geometry class |
| 2 | `WF-02` | FIRST EURO | ServiceNode page 36 | World+Service design: lock destination spacing/approach grammar | ServiceNode/RoadGraph | deterministic placement contract for each FE destination class |
| 3 | `WF-03` | FIRST EURO | L2 pages 09/35 + `CD-798` | World design: lock station/interchange topology and transfers | Vertical/RoadGraph/streaming | L2 authoring needs no new traversal/state rules |
| 4 | `WF-04` | FIRST EURO | pages 09/35 + Traffic | World+Traffic design: lock density/timing/clearance profiles | Traffic/Vertical/perf | named ordinary/stress profiles with objective bounds |
| 5 | `WF-06` | FIRST EURO | pages 16/36 | Service+Economy design: lock parking classes/pricing/signage/legality | Service/Economy/Enforcement | each class has rule, price source, persistence and consequence |
| 6 | `WF-07` | FIRST EURO | pages 16/36 | Vehicle+Service design: lock tuning categories/catalog/compatibility | Service/Economy/VehicleBuild | finite category/compatibility contract |
| 7 | `WF-08` | FIRST EURO | pages 16/36 | Service+Economy design: lock parts taxonomy/stock/pricing | Service/Economy/VehicleBuild | deterministic inventory/transaction contract |
| 8 | `WF-09` | FIRST EURO | page 36 + VehicleHealth | Service design: lock repair actions/partial-full policy/prices | Service/VehicleHealth/Economy | every repairable state maps to explicit service action |
| 9 | `WF-12` | FIRST EURO | Active Baseline + cockpit canon | Character design: lock driver identity/personality presentation | Character content/Neural | playable-character brief cannot alter input/vehicle authority |
| 10 | `WF-13` | FIRST EURO | page 14 + daughter canon | Character design: lock daughter FE task/dialogue/state limits | Taxi/presentation/Neural | every responsibility named; no hidden driving authority |
| 11 | `WF-14` | SCOPE DECISION | no final principal roster | Product/character: define roster and FE/post-year classification | PassengerIdentity/Neural | finite roster or explicit exclusions |
| 12 | `WF-15` | FIRST EURO content | `CD-800` substrate | Character+Taxi: lock authored recurring clients/arcs/triggers | PassengerIdentity/Order/Fare/Neural | finite data-driven client set/schema |
| 13 | `WF-16` | FIRST EURO | Economy + Service specs | Economy design: lock versioned price/balance tables | Economy/Service/Enforcement | one balance source; no location hardcoded money truth |
| 14 | `WF-17` | FIRST EURO | EnforcementEvent + traffic/rules | Rules design: lock fine catalog/signage/notice mapping | Traffic/Economy/Neural | event→fine→notice→persistence mapping for each chargeable rule |
| 15 | `WF-18` | RECONCILE | page 24 / `CD-538` | World-canon owner: resolve player-visible conflicts | world/narrative content | zero contradictory active FE world facts |
| 16 | `WF-19` | FIRST EURO | Active Baseline + world/vehicle profiles | World+vehicle design: lock weather set and gameplay-vs-presentation effects | World/Vehicle/Traffic | every weather state names authoritative gameplay effects |
| 17 | `WF-20` | FIRST EURO | bounded World/Traffic architecture | World+performance design: lock event taxonomy/spawn-expiry/population caps | World/Traffic/perf | finite classes and hard runtime budgets |
| 18 | `WF-05` | POST-FIRST-EURO | page 06 + `CD-753` | Product: confirm explicit L3 exclusion and extension boundary | CityCode schema only | EXCLUDED with no L1/L2 rewrite dependency |
| 19 | `WF-10` | POST-FIRST-EURO | page 16 | Product: confirm mall/food/lifestyle exclusion | future ServiceNode/Neural | EXCLUDED; no FIRST EURO blocker/dependency |
| 20 | `WF-11` | POST-FIRST-EURO | page 17 | Product: confirm bar/club/live-social exclusion | future Neural/online stack | EXCLUDED; tracked future program only |
| 21 | `NEU-01` | BASIC FE / schema decision | `CD-800`, page 05 | Neural+data: lock universal identity vs passenger-only fields | PassengerIdentity | versioned identity schema with public/internal fields |
| 22 | `NEU-02` | SCOPE DECISION | page 05 + authored characters | Neural product: lock profile fields/visibility/write rules | PassengerIdentity/content | every profile field has owner and visibility class |
| 23 | `NEU-03` | BASIC FE + expansion decision | `CD-800`, page 05 | Neural product: lock contact/follow/favorite/repeat/subscription edges | PassengerIdentity/Neural | finite edge taxonomy and transition rules |
| 24 | `NEU-04` | BASIC FE | `CD-800`, page 05 | Neural content: lock message types/thread/lifetime/read rules | Neural/content | finite message state machine + persistence |
| 25 | `NEU-05` | BASIC runtime / authored FE | `CD-800` + Order/Fare | Taxi+Neural: lock promotion/arcs/callback/cooldown rules | PassengerIdentity/Order/Fare | cannot bypass normal order/fare/economy paths |
| 26 | `NEU-06` | SCOPE DECISION | page 11 + relationship/city reputation | Product: separate passenger taste, professional reputation and civic standing | PassengerIdentity/Enforcement | each signal has one owner and bounded consequences |
| 27 | `NEU-07` | SCOPE DECISION | broad social not in current FE core | Product: include feed/posts/reactions or mark post-year | future Neural/content | explicit LOCKED contract or EXCLUDED |
| 28 | `NEU-08` | SCOPE DECISION | persistent identity direction | Product+world: define identity/passport credential role | identity/civic consumers | credential fields and verification consumers named or EXCLUDED |
| 29 | `NEU-09` | SCOPE DECISION | PassengerIdentity + interaction | Neural+privacy: define bounded person-tag recognition/exposure | Interaction/PassengerIdentity | no global scan; bounded consent/privacy contract |
| 30 | `NEU-10` | PRODUCT DIRECTION | Economy + fare/service transactions | Interaction+Economy: lock palm/NFC eligible payments/confirmation/failure UX | Economy/Fare/Service | exactly-once Economy owner; duplicate charge impossible |
| 31 | `NEU-11` | SCOPE DECISION | Economy journal | Neural+Economy: lock receipt/history classes and retention/status | Economy/UI | immutable transaction IDs/results drive presentation |
| 32 | `NEU-12` | SCOPE DECISION | future civic/regulator + rules | Product: define licence/right/permission catalog or exclude from FE | civic/rules/services | finite catalog/state transitions or explicit EXCLUDED |
| 33 | `NEU-13` | FIRST EURO fine projection | Enforcement + Economy | Neural+rules: lock fine status/deadline/payment/appeal visibility | Enforcement/Economy | Neural never owns violation/fine truth |
| 34 | `NEU-14` | SCOPE DECISION | Enforcement/civic producers | Product: lock official-notice sender/priority/ack/expiry rules | Enforcement/Neural | every notice references authoritative event/state or EXCLUDED |
| 35 | `NEU-15` | SCOPE DECISION | ServiceNode + Economy | Service+Neural: lock parking/repair/parts notification/receipt/reminder rules | Service/Economy | notification cannot mutate service truth |
| 36 | `NEU-16` | REQUIRED CONTRACT | PassengerIdentity + Neural | Neural/privacy: lock defaults, consent and data-exposure rules | all Neural surfaces | every visible datum has visibility/consent policy |
| 37 | `NEU-17` | SCOPE DECISION | contact permission exists | Neural: lock block/mute/revoke semantics across delivery paths | messaging/repeat orders/future social | one permission gate enforced consistently |
| 38 | `NEU-18` | FIRST EURO content decision | `WF-12..14` | Character+Neural: define each principal's Neural presence/absence | character data/Neural | every FE principal explicitly represented or absent |
| 39 | `NEU-19` | FIRST EURO authored content | `WF-15`, `CD-800` | Character+Neural: lock repeat-client message/milestone data schema | PassengerIdentity/Neural | no bespoke runtime code per client |
| 40 | `NEU-20` | FE offline / future online | persistence + future network stack | Architecture: lock local IDs vs reserved network-safe identity boundary | Persistence/future online | FE has zero online dependency; future transport wraps stable IDs |
| 41 | `NEU-21` | FIRST EURO persistence | `CD-801` + identity substrate | Persistence: lock full-Neural schema versions/default/migration rules | Persistence/Neural | deterministic migration or explicit failure; no silent social reset |
| 42 | `NEU-22` | SCOPE DECISION | Neural presentation | UX: lock notification priority/batching/interruption/accessibility | Neural/UI/input | bounded queue; no mandatory unsafe driving interruption |

## Required resolution order

1. **Scope first:** `WF-05`, `WF-10`, `WF-11`, `WF-14`, `NEU-07`, `NEU-08`, `NEU-12`, `NEU-14`, `NEU-15`, `NEU-17`, `NEU-22` — include/exclude decisions prevent wasted detailed design.
2. **World/service mechanics:** `WF-01..04`, `WF-06..09`, `WF-16..20` — lock production authoring rules and budgets before asset/content scale-up.
3. **Characters/repeat clients:** `WF-12..15`, then `NEU-18/19` — freeze who exists and what relationship content is authored.
4. **Neural data/permissions:** `NEU-01..06`, `NEU-09/16/20/21` — lock identity, graph, persistence and privacy boundaries.
5. **Neural civic/economy projections:** `NEU-10/11/13` plus any included `NEU-08/12/14/15` — freeze presentation over authoritative money/civic/service state.
6. **Presentation/attention:** included `NEU-07/22` and remaining UI-specific choices last, after their underlying state contracts are frozen.

## Entry gate status

`CD-847` can close when this finite matrix is durable and cross-linked. **Mechanics Freeze itself remains NOT PASSED while any non-excluded row above is unresolved.**

The next gate must resolve these decisions; it must not reopen CD-793 Core, CD-802 integration, CD-823 playable build, CD-843 Chaos authority, or CD-844 cockpit input authority unless executable evidence demonstrates a real contradiction.

Model/mesh work — including integration of the user-supplied Tatra placeholder — remains deliberately after this administrative/design decision gate and is not modified here.
