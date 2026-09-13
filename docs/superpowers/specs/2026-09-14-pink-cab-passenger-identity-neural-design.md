# PINK CAB Passenger Identity / Repeat Clients / Basic Neural Design

## Goal

Consolidate the existing passenger social foundation into one bounded authoritative logical record so a passenger can survive Actor stream-out, retain relationship/history/messages, and return through the ordinary taxi order/fare pipeline.

## Existing state

The repo already contains `FPinkCabPassengerIdentity`, `FPinkCabPassengerHistory`, `FPinkCabNeuralContactState`, `FPinkCabRepeatOrderFactory`, and passenger persistence fields. These partially overlap: relationship/repeat state is duplicated between Identity and History, Neural messages are unbounded, and persistence stores only a subset of the social state.

## Architecture

`FPinkCabPassengerRecord` is the canonical owner of dynamic passenger social state. `FPinkCabPassengerRegistry` owns a bounded collection of records keyed by stable passenger ID. Transient passenger Actors and presentation objects may reference an ID but never own relationship, memories, repeat eligibility, Neural state, or social replay journals.

## Canonical record

Each record contains stable `IdentityId`, immutable `TemplateId`, deterministic `IdentitySeed`, resolved passenger mass, bounded preference tags, one relationship state, repeat counters, bounded ride memories, bounded reviews, and bounded Neural state.

Creation rejects invalid IDs, missing template/context data, invalid mass, duplicate identities, and registry capacity overflow. A duplicate create never replaces existing state.

## Relationship and memories

Paid fares and authored relationship events update only the canonical record. Repeat eligibility is derived from the canonical counters: two paid fares or one authored relationship event. Ride memories are bounded and contain stable ride/order IDs plus outcome metadata; oldest memories are evicted deterministically when capacity is reached.

Legacy `FPinkCabPassengerIdentity` and `FPinkCabPassengerHistory` remain compatibility views/adapters where existing tests or call sites require them; they must not become independent authoritative owners once the registry is used.

## Neural

Neural is logical data, not an Actor. A contact has explicit permission and block state. Messages require a stable message ID, are exactly-once, and both the message list and replay journal are bounded. Blocking prevents new messages but does not delete prior history. Actor stream-out has no effect on the contact.

## Repeat orders

Repeat eligibility comes only from the canonical record. A repeat request creates a normal `FPinkCabOrder` carrying the same passenger stable ID and then uses the ordinary `FPinkCabFareSession`; there is no repeat-specific fare runtime.

## Persistence boundary

CD-800 defines a versioned logical passenger snapshot containing the full canonical record and registry ordering. Restore validates the entire snapshot before replacing live state. CD-801 remains responsible for disk save/recovery/workday orchestration and may embed this logical snapshot.

## Daughter / conductor boundary

The daughter/conductor may expose bounded information/service hooks linked to passenger records (for example passenger count, route/service notes, contact status). These hooks are read/service authority only: `HasDrivingAuthority()` and `HasSteeringAuthority()` are always false.

## Acceptance

One passenger completes a paid fare, is streamed out, receives a Neural message later, survives logical snapshot/restore, becomes repeat-eligible, creates a normal repeat order, and enters an ordinary FareSession again. Failure tests cover duplicates, capacity limits, blocked contact, invalid restore, and Actor despawn not deleting state.
