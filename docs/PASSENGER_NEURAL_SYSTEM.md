# PINK CAB · Passenger Templates, Repeat Clients & Basic Neural

**Status:** CURRENT FIRST-EURO IMPLEMENTATION-FACING CONTRACT / PARTIALLY SPECIFIED
**Product root:** Jira `CD-519`
**BASE-100 taxi owner:** `CD-749`
**Durable authority:** Confluence `6389768` and taxi/fare page `5832724`
**Scope authority:** `CD-753`

## FIRST EURO boundary

Persistent repeat clients + **basic Neural** are part of the first 12-month PC single-player product. Broad social-life simulation, account-backed presence, clubs/live rooms, multiplayer identity, public chat and moderation runtime are POST-FIRST-EURO.

A procedural `PassengerTemplate` and persistent `PassengerIdentity` are separate owners. A streamed body/voice presentation may disappear while the logical passenger identity remains alive in save data.

## PassengerTemplate

Template data may describe bounded reusable context such as archetype/social role, destination purpose, mood, tolerance for speed/rough driving/rule-breaking, topic/scene eligibility, ride-reaction parameters and presentation tags.

Templates do not own persistent relationship history, money settlement or world Actors.

## PassengerIdentity

Minimum FIRST EURO direction:

- stable `PassengerId`;
- reference to template/archetype data;
- stable authored passenger mass used by vehicle load state;
- compact trip/history summary;
- compact relationship/preference state;
- Neural/contact permission state;
- repeat-order eligibility/state;
- simple data-driven message/history references;
- explicit schema/version ownership.

The same identity must retain the same persistent mass and relevant history across stream-out/save/reload. Boarding/exiting applies physical passenger mass exactly once.

## Basic Neural

FIRST EURO Neural is a bounded local persistence/contact layer, not a social platform.

Supported direction:

`NO_ACCESS → PROFESSIONAL/REPEAT_CONTACT → OPTIONAL_EXPANDED_LOCAL_CONTACT → REVOKED/BLOCKED`

Exact labels/axes remain owner-data choices where still OPEN, but the implementation must support:

- granting/revoking local contact permission;
- a passenger messaging later through simple data-driven content;
- a repeat order from the same identity;
- remembered ride context influencing later eligible content;
- identity persistence without a live Actor;
- save/load without duplicate messages/orders/passenger identities.

Repeat orders reuse the normal Order/Fare pipeline. Neural may request work; it may not create a second fare/payment/economy system.

## Content rule

Scene fragments fire only when explicit prerequisites match current passenger, fare, route, vehicle/work state and remembered context. The content system must reject contradictory combinations rather than selecting random incompatible dialogue.

Content breadth and exact promotion/relationship scoring remain data/spec work under `CD-570/CD-571/CD-749`; they are not permission to invent broad online-social behavior.

## Acceptance

A seeded passenger can ride once, preserve stable identity/mass/history after body stream-out, optionally gain Neural access, message later and return through the normal fare pipeline. Save/reload/retry must not duplicate identity, order, message, fare or physical mass state.

Documentation establishes SPECIFIED only. Runtime IMPLEMENTED/VERIFIED requires exact executable evidence.
