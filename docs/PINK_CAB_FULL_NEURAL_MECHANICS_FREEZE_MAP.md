# PINK CAB · Full Neural Mechanics Freeze Dependency Map

Date: 2026-09-16  
Owner task: `CD-846`  
Parent gate: `CD-841`  
State: DESIGN DEPENDENCY MAP / NOT FULL NEURAL IMPLEMENTATION

## What exists now

`CD-800` implements the FIRST EURO basic substrate: stable `PassengerIdentity`, trip/history continuity, compact relationship/contact state, simple data-driven messages, repeat-order support, and repeat-client recognition/callback through the normal Order/Fare path.

That substrate is not the full Neural product. Full Neural is the combined social / identity / payment / civic-service layer described below.

## Authority doctrine

- Neural never owns FareSession, vehicle dynamics, Economy balance, fine debt, licences, or service inventory truth.
- PassengerIdentity owns persistent person identity/relationship state.
- Economy owns monetary transactions and exactly-once settlement.
- Enforcement/rules own violations, penalties and fine state.
- ServiceNode/shops own service lifecycle/inventory through their existing contracts.
- Neural may present, route, notify, message and expose permissions around those authoritative systems.
- No broad online/public social dependency may block the single-player FIRST EURO core unless explicitly promoted into scope.

## Freeze dependency map
| ID | Neural surface | Current scope/truth | Authority dependency | Unresolved freeze decision | Freeze acceptance |
|---|---|---|---|---|---|
| `NEU-01` | Persistent person identity | BASIC IMPLEMENTED | PassengerIdentity / `CD-800` | which fields are universal person identity versus passenger-only gameplay data | versioned identity schema names public/internal fields and survives materialization/streaming |
| `NEU-02` | Profile / public-private data | SCOPE DECISION | PassengerIdentity + authored characters | profile field set, visibility classes, player vs NPC differences | every field has visibility/owner/write rules; no UI invents identity data |
| `NEU-03` | Contacts / following | BASIC contact permission exists; broad social graph unfrozen | PassengerIdentity + Neural | distinction among contact, follow, favorite/repeat-client and service/institution subscription | finite relationship-edge taxonomy and transition rules exist |
| `NEU-04` | Messaging | BASIC IMPLEMENTED | Neural message substrate + authored content | message types, reply capability, thread lifetime, unread/archive rules, authored vs systemic senders | finite message state machine and persistence rules exist |
| `NEU-05` | Repeat-client relationships | BASIC runtime exists; authored content unfrozen | PassengerIdentity + Order/Fare + Neural | promotion rules, authored relationship arcs, callbacks, preferences and cooldowns | relationship progression cannot bypass normal order/fare/economy paths |
| `NEU-06` | Reputation / social credit | PARTIAL concepts exist | Passenger relationship + city/enforcement reputation | separate private passenger taste, public/professional reputation and civic standing; decide which are FIRST EURO visible | each signal has one owner, consumers and bounded consequences; no single magic social score |
| `NEU-07` | Feed / social-network posting | UNFROZEN / broad social simulation not in current FIRST EURO core | future Neural/social content | whether a feed/posts/reactions exist in FIRST EURO or are explicit post-year content | explicit include/exclude decision; if included, finite content/state contract exists |
| `NEU-08` | Identity/passport credential | UNFROZEN | persistent player/person identity | what the credential proves, who can request it, whether it is UI-only lore or gates gameplay | credential fields, verification consumers and failure/permission rules are named |
| `NEU-09` | Persistent person tag / local identification | UNFROZEN | PassengerIdentity + interaction/presentation | how a person is recognized/tagged in-world, distance/consent/privacy limits, offline-only vs network identity | no arbitrary global scan; identity exposure has bounded interaction and privacy rules |
| `NEU-10` | Palm / NFC payment | PRODUCT DIRECTION EXISTS; presentation not frozen | Economy + Fare/Service settlement | which payments use palm/NFC, confirmation gesture, receipts, insufficient-funds UX and offline failure presentation | Neural/interaction only requests payment; Economy remains exactly-once owner and duplicate charge is impossible |
| `NEU-11` | Transaction history / receipts | UNFROZEN PRESENTATION | Economy journal + Fare/Service/Fine transactions | which transaction classes appear, retention length, dispute/status labels | UI reads immutable transaction IDs/results; it cannot mutate settled money state |
| `NEU-12` | Licences / rights / permissions | UNFROZEN | future civic/regulator + existing rule/service contracts | which licences/rights exist in FIRST EURO, expiry/suspension semantics, gameplay gates and who issues them | finite credential/permission catalog or explicit FIRST EURO exclusion exists |
| `NEU-13` | Fines / penalties | CORE FINE STATE EXISTS | Enforcement + Economy | Neural presentation, deadlines/statuses, appeal/payment visibility, relationship to future Taxi Regulator | violation/fine truth remains external; Neural can display and route payment/notice only |
| `NEU-14` | Official / government notices | UNFROZEN | Enforcement + civic/service producers | sender types, priority, acknowledgement, expiration and gameplay consequences | every official notice references an authoritative event/state and cannot create penalties by itself |
| `NEU-15` | Service / shop notifications | UNFROZEN | ServiceNode + Economy + future authored services | order/repair/parts/parking notifications, receipts, reminders and subscription rules | notification is a projection of service state; deleting/reading it cannot change service truth |
| `NEU-16` | Privacy / consent | REQUIRED DESIGN CONTRACT | PassengerIdentity + Neural | data visibility defaults, contact consent, identity reveal, local tag visibility and NPC/public-profile rules | all Neural-visible data has visibility and consent policy before authored content ships |
| `NEU-17` | Block / mute / contact revocation | PARTIAL contact permission only | PassengerIdentity + messaging/repeat-order routing | what block/mute prevents for NPC messages, repeat orders, following and future public users | one bounded permission gate is enforced by all Neural delivery paths |
| `NEU-18` | Principal characters in Neural | UNFROZEN | `WF-12..14` + authored character data | which principals have profiles/messages/official roles and how progression changes them | every FIRST EURO principal has a defined Neural presence or explicit absence |
| `NEU-19` | Authored repeat-client content packs | UNFROZEN | `WF-15` + PassengerIdentity | message packs, call-back conditions, profile changes, relationship milestones | finite data-driven authoring schema exists; no client requires bespoke runtime code |
| `NEU-20` | Offline / future-online boundary | FIRST EURO OFFLINE; online post-year | persistence + future network/social stack | which IDs/state are local-only now and which schemas reserve future network-safe identity | FIRST EURO works with no online service; future transport can be added without replacing local identity keys |
| `NEU-21` | Persistence / migrations | BASIC PERSISTENCE EXISTS | CD-801 + PassengerIdentity/Neural | schema versions for new full-Neural fields and migration/default behavior | old saves load deterministically or fail explicitly; no silent identity/social reset |
| `NEU-22` | Notifications / attention budget | UNFROZEN | Neural presentation | priority classes, batching, interruption rules, driving-safe presentation and accessibility | no required Neural interaction forces unsafe driving interruption; queue/badge behavior is bounded |

## Scope rule for FIRST EURO

The existing FIRST EURO commitment is **repeat clients + basic Neural**, not an automatically expanded public social network. `NEU-01/03/04/05` basic substrate is already represented by `CD-800`; full product surfaces `NEU-02/06..22` require explicit include/exclude decisions during Mechanics Freeze. Fine/payment/service systems may already function without Neural owning them.

## Non-negotiable integration rules

- Money changes only through Economy transaction APIs.
- Fine/violation state changes only through Enforcement/rule owners.
- Repeat orders always enter the normal Order/Fare pipeline.
- Passenger/profile presentation never becomes the authoritative person record.
- Reading/deleting a message or notice never settles money, clears a fine, grants a licence, repairs a car or changes service inventory.
- Future online/public social features must add transport/synchronization around stable IDs rather than replacing single-player save identity.
