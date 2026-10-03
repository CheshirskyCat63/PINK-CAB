# P04 baseline measurements — CD-641 / PHY-017

## Current acceptance and execution

The owner accepted P03 source `edf75e1b91742237d14ba9dfe76284993da77d17` as interim **version 2** on 2026-10-03: good enough to continue, not final-perfect handling. Its verified package is `CD869_ENDLESS_edf75e1b_RUN37117735294`; coordinated verification was run `37116996219`. PR #52 integrated the identical source tree as `ea86bd5a84c2f5de2406bc5f3c57f6218a224fa8`. The source SHA of the accepted binary is not renamed to the merge SHA.

CD-649 and CD-659 are complete for their accepted P03 corrective scope. CD-648 remains the P00–P11 execution umbrella. P04 is authorised under existing CD-641 / CD-642 / CD-646 owners; this first change only instruments the unchanged profile. No P04 tuning result or later P05–P11 gate is accepted by the P03 verdict. Confluence program 22413538 version 24 records the current decision; older P03-rejected/P04-blocked snapshots are historical.

The exact accepted package was revalidated against its 53-file manifest by run `37121710646`. It remains separately accessible through `PINKCAB PLAY - V2 ACCEPTED.lnk` and `CHESHIRE_STUDIO/PINKCAB_Accepted.lnk`. Prior accepted P02 remains a separate rollback. No new package is delivered by this measurement PR.

## Read-only profile tables

`PinkCab.Vehicle.Physics.P04.ProfileDerivedTables` reads the nominal physical profile and production movement defaults. It emits 42 RPM/speed rows: five forward gears plus reverse, with seven RPM points each. The engagement validator must agree with profile ratios and driven-wheel radius within 0.1 percent or 1 RPM, whichever is larger.

The speed values are **no-slip kinematics, not achievable top-speed predictions**. Torque/power rows are calculated from the applied authored curve, not dynamometer measurements. Profile identity and calibration version accompany the table. A future ratio change must keep the engagement validator consistent rather than inheriting stale constants.

## Bounded runtime capture

`PinkCab.Vehicle.Physics.P04.UnchangedProfileBaseline` repeats six cases three times each: first/reverse gear at 25/50/100 percent authored throttle. Each case spawns a fresh healthy native fixture on the same transient flat floor, waits for the established idle/rest gate, then holds the declared throttle for five observed mechanical seconds while clutch coupling rises linearly over 1.5 seconds. It selects neutral and zero throttle and observes two seconds of unforced coast.

Capture actual body mass, endpoint speed/RPM, first observed 30/60 km/h crossings, peak rear-drive torque, wheel/chassis slip-speed difference, and neutral endpoint speed/drive torque. A speed threshold not reached is recorded as -1. Each case has a 35-second wall-clock bound. The floor is extended only in the transient automation world and restored; no road map is saved.

This is a **sterile native drivetrain fixture**, not a complete player-input drive or packaged vehicle model. Its fixed scripted trace does not add automatic clutch/gears to gameplay. Crossing times and extrema have game-thread sampling resolution; they are not claimed to be continuous physics-thread measurements. Later comparisons must declare timing tolerances, confirm repeated spread, keep traces/mass/surface comparable and obtain packaged confirmation.

## Exit boundaries

Passing these tests establishes internally consistent profile tables and complete bounded baseline capture. It does not complete PHY-017..020, choose an A/B tune, establish requested acceleration performance, or supersede the accepted version 2.

Record the baseline and its medians/spreads first. Declare comparative acceleration, dosability and coast targets before changing torque curve, engine inertia/braking or ratios. Preserve accepted P03 controls, full physics/P02 regressions and the separate owner verdict for a changed-feel candidate. Final tyres, suspension, physical geometry, city content and placeholder art are outside this change.

CD-559 clean-checkout reproducibility, permanent delivery hardening and operational retirement remain separate; successful focused measurement must not close them by implication.
