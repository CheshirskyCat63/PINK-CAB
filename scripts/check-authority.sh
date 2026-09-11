#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

registry="docs/AUTHORITY.yaml"

fail() {
  echo "ERROR: $*" >&2
  exit 1
}

require_file() {
  test -f "$1" || fail "missing required file: $1"
}

require_text() {
  local file="$1"
  local pattern="$2"
  grep -Fq -- "$pattern" "$file" || fail "missing required authority text in $file: $pattern"
}

reject_regex() {
  local file="$1"
  local pattern="$2"
  local message="$3"
  if grep -Eqi -- "$pattern" "$file"; then
    fail "$message ($file)"
  fi
}

require_file "$registry"

# Current product and repository authority.
require_text "$registry" "product: PINK CAB"
require_text "$registry" "product_root: CD-519"
require_text "$registry" "repository: CheshirskyCat63/PINK-CAB"
require_text "$registry" "repository_status: active-dedicated-pink-cab"
require_text "$registry" "working_branch: main"
require_text "$registry" "legacy_migration_source: CheshirskyCat63/DEADRACE"
require_text "$registry" "legacy_staging_sha: 1a4d7e05f1430a6e558b3f697aee3fbf412785bb"
require_text "$registry" "cutover_gate: CD-558"
require_text "$registry" "pre_fgear_program: CD-754"
require_text "$registry" "clean_repository_status: complete"
require_text "$registry" "cutover_baseline_sha: a807db88baed881c526f2df63f87d01e913f41ef"
require_text "$registry" "legacy_freeze_sha: bff83382e3b567e17fa01029f698204c7bce7872"

# Dedicated-repository cut-over must not regress to the legacy active-repo state.
reject_regex "$registry" "repository:[[:space:]]*CheshirskyCat63/DEADRACE" "DEADRACE is falsely presented as the active repository"
reject_regex "$registry" "legacy-named-active-pink-cab|external-blocked-until-repo-admin-access" "obsolete repository cut-over state remains active"

# Current BASE-100 / START-90 census.
require_text "$registry" "jira_ledger: CD-660"
require_text "$registry" "jira_governance: CD-661"
require_text "$registry" "confluence_ledger: 6553617"
require_text "$registry" "owner_pack: docs/PINK_CAB_BASE100_TECH_OWNER_PACK_01.md"
require_text "$registry" "tracked_rows: 196"
require_text "$registry" "status_counts: {locked: 46, calibration: 3, proposed_default: 88, open: 59}"
require_text "$registry" "numerator_points: 93.0"
require_text "$registry" "start90_percent: 47.4"
require_text "$registry" "start90_gate_passed: false"
require_text "$registry" "gap_to_start90_percentage_points: 42.6"
require_text "$registry" "all_proposed_defaults_accepted_percent: 69.9"
require_text "$registry" "open_rows_needed_after_all_defaults_accepted_to_cross_90: 40"
require_text "$registry" "crossing_example_percent: 90.31"
require_text "$registry" "broad_production: HOLD"

# BASE-100 program and FIRST EURO scope.
require_text "$registry" "program: CD-746"
require_text "$registry" "scope_owner: CD-753"
require_text "$registry" "confluence_page: 11239425"
require_text "$registry" "horizon_months: 12"
require_text "$registry" "platform: PC"
require_text "$registry" "single_player: true"
require_text "$registry" "full_gameplay_tiers: [level1, level2]"
require_text "$registry" "- practice_hangar_only_where_level1_training_or_acceptance_requires"
require_text "$registry" "- moving_refueling"
require_text "$registry" "- level3_gameplay"
require_text "$registry" "- daily_insurance_purchase_policy_claim_extra_life"

# Current input contract.
require_text "$registry" "gaze: Space"
require_text "$registry" "quick_start: 1-4"
require_text "$registry" "attention: LMB"
require_text "$registry" "go: RMB"
require_text "$registry" "1: turn_signals"
require_text "$registry" "2: horn"
require_text "$registry" "3: gearbox"
require_text "$registry" "4: handbrake"
require_text "$registry" "rebinding_first_euro: OPEN_BASE100_C08"

# Vehicle and residual owner decisions that must not be falsely closed.
require_text "$registry" "road_dynamics_solver: FGear_Vehicle_Physics"
require_text "$registry" "permanent_body_deformation: Debugging_Delight_Vehicle_Damage_System"
require_text "$registry" "terminal_crash_recovery: OPEN_BASE100_I06"
require_text "$registry" "workday_duration_time_semantics: OPEN_BASE100_I08"
require_text "$registry" "current_first_euro_uninsured_recovery: OPEN_CD-750_BASE100_I06"
require_text "$registry" "first_euro: false"
require_text "$registry" "delivery: POST_FIRST_EURO"

# Current durable authority IDs.
require_text "$registry" "authority_index: 6586369"
require_text "$registry" "active_baseline: 5537802"
require_text "$registry" "readiness_ledger: 6553617"
require_text "$registry" "base100_code_architecture: 11239425"
require_text "$registry" "open_decisions: 5832744"
require_text "$registry" "master_question_register: CD-673"

# Exact current 59-row OPEN owner set.
expected_open="$(cat <<'EOF'
A03
A07
A10
A13
A14
A15
C08
F04
F05
F06
F08
F11
F12
F13
F14
F15
F16
F17
F18
F19
G02
G04
H04
H05
I01
I02
I03
I04
I06
I08
I09
I11
I12
I13
J05
J07
J09
J11
J12
K04
K06
K08
K11
K12
K13
L05
L06
L07
M05
N05
N06
N07
N09
O02
O03
P03
P05
P09
P11
EOF
)"

actual_open="$(awk '
  /^open_owner_rows:/ {section=1; next}
  section && /^  ids:/ {ids=1; next}
  ids && /^    - / {print $2; next}
  ids && $0 !~ /^    - / {exit}
' "$registry")"

if [ "$actual_open" != "$expected_open" ]; then
  echo "Expected OPEN rows:" >&2
  printf '%s\n' "$expected_open" >&2
  echo "Actual OPEN rows:" >&2
  printf '%s\n' "$actual_open" >&2
  fail "normalized 59-row OPEN owner set drifted"
fi
require_text "$registry" "count: 59"
require_text "$registry" "production_line: UE_5.8"
require_text "$registry" "owner_decision: use_Unreal_Engine_5_8"
require_text "$registry" "owner_row: A09"
require_text "$registry" "entrypoint: scripts/build.ps1"
require_text "$registry" "clean_checkout_command: \".\\\\scripts\\\\build.ps1\""
require_text "$registry" "package_command: \".\\\\scripts\\\\build.ps1 -Package\""
require_text "$registry" "visual_studio_role: IDE_only_not_canonical_build_authority"
require_text "$registry" "production_must_not_invent_unknowns: true"
require_text "$registry" "proposed_defaults_are_not_locked_until_owner_accepts: true"

# Current implementation-facing mirrors required by the admin authority tree.
for file in \
  README.md \
  docs/README.md \
  docs/AUTHORITY.yaml \
  docs/FIRST_EURO_SCOPE.md \
  docs/OPEN_DECISIONS.md \
  docs/PINK_CAB_ACTIVE_BASELINE.md \
  docs/PINK_CAB_BASE100_CODE_ARCHITECTURE.md \
  docs/PINK_CAB_BASE100_TECH_OWNER_PACK_01.md \
  docs/PINK_CAB_START90_READINESS.md \
  docs/PINK_CAB_DAILY_INSURANCE_CRASH_RECOVERY.md \
  docs/PROJECT_SETUP.md \
  docs/VERIFICATION_MATRIX.md \
  docs/PINK_CAB_PRE_FGEAR_EXECUTION_INDEX.md \
  docs/PINK_CAB_REPOSITORY_CUTOVER.md; do
  require_file "$file"
done

# Mirror-level current locks.
require_text docs/PINK_CAB_START90_READINESS.md "TOTAL: **196**"
require_text docs/PINK_CAB_START90_READINESS.md "93 / 196 = 47.4%"
require_text docs/PINK_CAB_START90_READINESS.md "40 of the remaining 59 OPEN rows"
require_text docs/PINK_CAB_BASE100_TECH_OWNER_PACK_01.md "196 code-facing rows"
require_text docs/PINK_CAB_BASE100_TECH_OWNER_PACK_01.md "I06 OPEN"
require_text docs/PINK_CAB_BASE100_TECH_OWNER_PACK_01.md "I08 OPEN"
require_text docs/OPEN_DECISIONS.md "59 genuine OPEN owner rows"
require_text docs/OPEN_DECISIONS.md "Practice Hangar only where Level1 training/acceptance requires it"
require_text docs/PINK_CAB_DAILY_INSURANCE_CRASH_RECOVERY.md "remains independently owner-open under BASE-100 state/session work"
require_text docs/VERIFICATION_MATRIX.md "CANON → SPECIFIED → IMPLEMENTED → VERIFIED"

# Contradiction guards for active mirrors.
active_docs=(
  README.md
  docs/README.md
  docs/AUTHORITY.yaml
  docs/FIRST_EURO_SCOPE.md
  docs/OPEN_DECISIONS.md
  docs/PINK_CAB_ACTIVE_BASELINE.md
  docs/PINK_CAB_BASE100_CODE_ARCHITECTURE.md
  docs/PINK_CAB_BASE100_TECH_OWNER_PACK_01.md
  docs/PINK_CAB_START90_READINESS.md
  docs/PINK_CAB_DAILY_INSURANCE_CRASH_RECOVERY.md
  docs/PROJECT_SETUP.md
  docs/VERIFICATION_MATRIX.md
  docs/PINK_CAB_PRE_FGEAR_EXECUTION_INDEX.md
  docs/PINK_CAB_REPOSITORY_CUTOVER.md
)

for file in "${active_docs[@]}"; do
  reject_regex "$file" 'owner-locked.*I06|I06.*owner-locked' "I06 is falsely presented as owner-locked"
  reject_regex "$file" 'LMB[[:space:]]*=[[:space:]]*(press|grab)|RMB[[:space:]]*=[[:space:]]*active manipulation' "superseded mouse-button mapping presented as current"
  reject_regex "$file" 'product_root:[[:space:]]*CD-418|Current gameplay authority:.*DEADRACE|active product:[[:space:]]*DEADRACE' "legacy DEADRACE pursuit authority presented as current"
  reject_regex "$file" 'One work shift = 2 real-world hours' "obsolete two-hour Workday hard-lock remains in an active mirror"
done


# Do not allow active current mirrors to revive the incomplete 166-row census as current authority.
for file in docs/AUTHORITY.yaml docs/OPEN_DECISIONS.md docs/PINK_CAB_ACTIVE_BASELINE.md docs/PINK_CAB_BASE100_TECH_OWNER_PACK_01.md docs/PINK_CAB_START90_READINESS.md; do
  if grep -Eqi 'current[^\n]{0,80}(166[ -]row|166 code-facing)|166[ -]row[^\n]{0,80}current' "$file"; then
    fail "obsolete 166-row census presented as current ($file)"
  fi
done

echo "PINK CAB BASE-100 administrative authority guard: OK"
