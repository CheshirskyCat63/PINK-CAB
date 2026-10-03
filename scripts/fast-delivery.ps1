[CmdletBinding()]
param(
    # Retain the legacy interface so old callers receive an actionable refusal.
    [string]$TestFilter = "PinkCab.Cockpit.Playable.Runtime",
    [string]$EngineRoot,
    [string]$IterationBuild,
    [string]$ShortcutPath,
    [switch]$SkipTests,
    [switch]$ForceRecook,
    [switch]$PlanOnly
)

# CD-950: this entry must never build, launch, patch or promote a package.
# See docs/FAST_DELIVERY_ENTRYPOINT.md for the existing explicit delivery route.
# In particular, no legacy switch can bypass this retirement boundary.
if ($PlanOnly) {
    [ordered]@{
        status = 'RETIRED'
        automatic_dispatch = $false
        package_changes = $false
        baseline_changes = $false
        shortcut_changes = $false
        verification_workflow = 'cd648-p02-phy009.yml'
        delivery_workflow = 'cd869-deliver.yml'
        candidate_requirement = 'Record and verify the exact 40-character source SHA.'
        target_map = '/Game/Dev/Maps/L_PinkCab_L1_EndlessStraight'
        delivery_result = 'HUMAN_PENDING; delivery does not accept or promote LastGood.'
        instructions = 'docs/FAST_DELIVERY_ENTRYPOINT.md'
    } | ConvertTo-Json
    return
}

[Console]::Error.WriteLine(
    'PINKCAB_LEGACY_FAST_DELIVERY_RETIRED: No build, package, baseline or shortcut was changed. ' +
    'Use -PlanOnly for the read-only migration plan and read docs/FAST_DELIVERY_ENTRYPOINT.md. ' +
    'Delivery requires an explicit exact-candidate workflow; SkipTests and ForceRecook do not reactivate this command.'
)
exit 2
