# PINK CAB · FGear Official Evaluation & License Boundary Report

Date: 2026-09-12
Jira: `CD-777`
Program: `CD-784`
Financial mode: `EUR 0`
State: `PUBLIC_EVALUATION_PATH_CONFIRMED / INSTALLABLE_TRIAL_NOT_PUBLICLY_CONFIRMED / VENDOR_REPLY_PENDING`

## Executive finding

FGear has a legitimate zero-cost public evaluation surface, but it is currently limited to official packaged demos, seller/Fab documentation, videos and the official Example Project link. No public seller/Fab source located during this review explicitly offers an installable FGear plugin trial/evaluation binary or temporary plugin license.

The latest vendor-posted compatibility evidence found is FGear v1.8.4 with Unreal Engine 5.7 support. No vendor statement confirming UE 5.8 support was found. PINK CAB therefore treats UE 5.8 compatibility as UNKNOWN until Lazybit Games confirms it or publishes an official build.

A vendor email request is prepared as an unsent Gmail draft. It asks for an official evaluation/trial plugin route, UE 5.8 compatibility and the licensing boundary.

## Official/free artifacts allowed now

- Fab product page: https://www.fab.com/listings/8904fcf9-debf-416c-aefa-4b2d125e46b7
- Official Epic Developer Community FGear thread with seller-posted demos: https://forums.unrealengine.com/t/fgear-vehicle-physics-v1-4/128793
- Official seller documentation link exposed by Fab: https://www.dropbox.com/s/o8fdgsiezfaf20h/fgue4docs.rar?dl=0
- Official seller Example Project link exposed by Fab: https://www.dropbox.com/s/n77usmravb97vun/FGearExample.rar?dl=0
- Seller site/reference material: https://fgearvp.wordpress.com/

These artifacts are evaluation/reference material only. Packaged demo contents are not a lawful source for extracting plugin binaries or source.

## Compatibility evidence

The vendor forum post dated 2025-12-14 states that v1.8.4 is available and explicitly lists Unreal 5.7 support. Later public discussion shows users working with 5.7. No public seller statement confirming Unreal 5.8 support was found in the reviewed sources.

- UE 5.7: vendor-confirmed for v1.8.4.
- UE 5.8: `UNKNOWN / NOT PUBLICLY CONFIRMED`.
- `CD-781` may not claim executable UE 5.8 compatibility without an official plugin/build and a real smoke test.

## Trial/evaluation determination

Public official sources expose playable packaged demos, documentation and an Example Project. They do not currently advertise a public installable evaluation/trial plugin.

Legal zero-cost path:

1. use packaged demos for behavior/feel reference;
2. inspect docs and Example Project structure/configuration;
3. continue adapter mapping and Tatra calibration without plugin binaries;
4. ask Lazybit Games for an official evaluation build/license;
5. if the vendor confirms none exists, record that as purchase-gate evidence rather than sourcing binaries elsewhere.

## License boundary

FGear is sold on Fab as a Tools & Plugins / Unreal Engine product. The Fab Standard License summary permits private/commercial use, modification for incorporation into projects, and distribution of projects with licensed content incorporated, while prohibiting standalone redistribution.

For code plugins, Fab licensing is per-seat. Current Epic Developer Community guidance quoting the Fab EULA states that code plugins are offered on a per-seat basis and may only be used by the number of users licensed for the plugin. Recheck the current Fab EULA at purchase time because license terms can change.

PINK CAB rules remain stricter:

- no paid acquisition before explicit owner approval at `CD-782`;
- no mirrors, warez, extracted binaries or redistributed plugin archives;
- no FGear binary/source in production `main` during evaluation;
- any vendor-supplied evaluation build remains isolated to Evaluation Lab/integration work and follows vendor-stated evaluation terms.

## Vendor contact

An unsent Gmail draft to `lazybitgames@gmail.com` asks:

- whether an official installable evaluation/trial build or temporary license exists;
- whether FGear supports UE 5.8;
- whether a 5.8 evaluation build/compatibility route exists;
- whether public demos/docs/Example Project are reference-only until Fab purchase;
- confirmation of the post-purchase per-seat boundary for a small team.

The draft is intentionally not sent without owner review/action.

## CD-777 close criteria status

- official demos/docs/Example Project identified: DONE
- standalone demo vs project/plugin boundary recorded: DONE
- illegal extraction/mirror prohibition recorded: DONE
- public installable trial discovered: NO PUBLIC TRIAL FOUND
- official vendor request prepared: DONE / UNSENT
- UE 5.8 compatibility: OPEN / VENDOR CONFIRMATION REQUIRED
- spend: EUR 0
- production repo plugin state: FGear-free

`CD-777` remains IN PROGRESS until the vendor reply is received or the owner decides that the documented no-public-trial result is sufficient to close the intake task.
