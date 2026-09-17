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
require_text "$registry" "status_counts: {locked: 105, calibration: 3, proposed_default: 69, open: 19}"
require_text "$registry" "numerator_points: 142.5"
require_text "$registry" "start90_percent: 72.7"
require_text "$registry" "start90_gate_passed: false"
require_text "$registry" "gap_to_start90_percentage_points: 17.3"
require_text "$registry" "all_proposed_defaults_accepted_percent: 90.31"
require_text "$registry" "open_rows_needed_after_all_defaults_accepted_to_cross_90: 0"
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
require_text "$registry" "gaze_hold: Space"
require_text "$registry" "quick_recall: 1-4"
require_text "$registry" "quick_recall_semantics: restore_saved_physical_target_or_hand_pose_without_actuation"
require_text "$registry" "right_hand_grip: RMB_brings_or_retains_right_hand_on_current_target_where_grip_is_required"
require_text "$registry" "1: turn_signals"
require_text "$registry" "2: horn"
require_text "$registry" "3: gearbox"
require_text "$registry" "4: handbrake"
require_text "$registry" "momentary_press_hold: LMB_press_or_hold_for_controls_that_support_momentary_actuation"
require_text "$registry" "contextual_wheel: mouse_wheel_adjusts_detents_rotary_or_incremental_controls_when_supported"
require_text "$registry" "target_selection_does_not_actuate: true"
require_text "$registry" "interaction_target_policy: one_bounded_current_target_no_world_scan"
require_text "$registry" "semantic_input_router: one_cpp_enhanced_input_router_distributes_semantic_state_and_commands_no_feature_system_reads_raw_keys"
require_text "$registry" "physical_control_contract: one_reusable_control_interface_each_control_declares_supported_gestures_and_emits_semantic_events"
require_text "$registry" "control_to_subsystem_rule: controls_never_reach_directly_into_unrelated_taxi_economy_vehicle_internals"
require_text "$registry" "reticle_feedback: translucent_default_valid_target_brightens_to_white_matte_less_transparent_no_text_labels"
require_text "$registry" "hand_selection: interaction_truth_hand_agnostic_left_or_right_hand_chosen_automatically_by_presentation_context"
require_text "$registry" "rebinding_first_euro: true"

# Vehicle and residual owner decisions that must not be falsely closed.
require_text "$registry" "road_dynamics_solver: Native_Unreal_Chaos_Vehicles"
require_text "$registry" "required_third_party_vehicle_plugins: none"
require_text "$registry" "fgear_vds_status: archived_research_fallback_only"
require_text "$registry" "owner_gate: CD-848"
require_text "$registry" "consumer_task: CD-855"
require_text "$registry" "status: IMPLEMENTED_VERIFIED_PRE_MODEL_CONTRACT"
require_text "$registry" "asset_import_contract: docs/PINK_CAB_TATRA_ASSET_IMPORT_CONTRACT.md"
require_text "$registry" "model_integration_may_proceed: true"
require_text "$registry" "full_mechanics_freeze_implied: false"
require_text "$registry" "final_model_complete: false"
require_text "$registry" "first_euro: false"
require_text "$registry" "prism_smart_glasses: taxi_uniform_glasses_with_outward_prism"
require_text "$registry" "pre_fgear_blocker: false"
require_text "$registry" "delivery: POST_FIRST_EURO"
require_text "$registry" "exact_full_stop_epsilon_dwell: speed_below_0_5_kmh_continuously_for_0_4_seconds"
require_text "$registry" "meter_start_stop_reset_sequence: boarded_and_door_latched_then_manual_start__destination_eligible_fullstop_then_manual_stop__payment_receipt_passenger_exit_reset_idle_then_new_start"
require_text "$registry" "fare_time_stopped_policy: accrues_while_active_after_start_including_ordinary_stops_and_waiting_excluding_hard_pause"
require_text "$registry" "passenger_cancel_after_boarding: serious_crash_or_terminal_service_failure_only_active_fare_fails_workday_ends_repair_recovery_follows"
require_text "$registry" "passenger_profile_visible_on_approach: true"
require_text "$registry" "local_passenger_review: star_rating_plus_short_text"
require_text "$registry" "first_euro_cross_player_review_transport: false"
require_text "$registry" "second_active_fare_while_occupied: forbidden_offers_may_buffer_but_cannot_activate"
require_text "$registry" "debt: bounded_essential_only_recovery_minimum_roadworthy_repair_mandatory_day_obligations"
require_text "$registry" "workday_duration_time_semantics: 12_ingame_hours_equals_120_real_minutes_time_scale_x6"
require_text "$registry" "force_quit_crash_contract: preserve_all_committed_purchases_refuel_fines_fare_payment_damage_and_resume_last_committed_logical_state"
require_text "$registry" "persistent_delta_classes: vehiclebuild_owned_service_economy_transactions_passengeridentity_local_reviews_important_active_incidents_explicit_persistent_world_changes"
require_text "$registry" "identity_contract: LOCKED_BASE100_J01"
require_text "$registry" "citycode_immutable_existing_campaign: LOCKED_BASE100_J02"
require_text "$registry" "deterministic_logical_ids: LOCKED_BASE100_J03_world_identity_plus_coordinate_or_logical_path"
require_text "$registry" "persistence_model: LOCKED_BASE100_J04_seed_versions_plus_meaningful_deltas_not_full_city_serialization"
require_text "$registry" "transient_traffic_unloaded: LOCKED_BASE100_J06_not_persisted"
require_text "$registry" "deterministic_reproduction: LOCKED_BASE100_J08_same_identity_versions_inputs_same_gameplay_topology_and_ids"
require_text "$registry" "streaming_directionality: predictive_player_forward_priority"
require_text "$registry" "streaming_numeric_budgets: OPEN_BASE100_J11_J12"
require_text "$registry" "incident_persistence: bounded_important_active_only_background_crashes_nearmiss_minor_debris_may_expire_after_reconstruction_window"
require_text "$registry" "old_generator_save_policy: campaign_retains_generator_and_contentset_versions_explicit_versioned_migration_only_no_silent_regeneration"
require_text "$registry" "reacquisition_reset_semantics: reset_after_0_25s_continuous_valid_contact_shorter_recontact_does_not_refresh"

# Current durable authority IDs.
require_text "$registry" "authority_index: 6586369"
require_text "$registry" "active_baseline: 5537802"
require_text "$registry" "readiness_ledger: 6553617"
require_text "$registry" "base100_code_architecture: 11239425"
require_text "$registry" "open_decisions: 5832744"
require_text "$registry" "master_question_register: CD-673"

# Exact current 19-row OPEN owner set.
expected_open="$(cat <<'EOF'
F18
J11
J12
K04
K06
K08
K11
K12
K13
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
  fail "normalized 19-row OPEN owner set drifted"
fi
require_text "$registry" "count: 19"
require_text "$registry" "payment_commit_interaction: passenger_hand_swipe_on_cabin_grabrail_reader_before_exit"
require_text "$registry" "payment_commit_blocks_unpaid_escape: true"
require_text "$registry" "receipt_after_payment: physical_optional_take_or_leave_never_blocks_fare_completion_or_next_order"
require_text "$registry" "promotion_scoring: repeat_pool_after_2_successfully_paid_fares_or_one_authored_relationship_event_then_score_seeded_selection"
require_text "$registry" "relationship_axes: [Trust, Satisfaction, RiskTolerance]"
require_text "$registry" "production_line: UE_5.8"
require_text "$registry" "owner_decision: use_Unreal_Engine_5_8"
require_text "$registry" "owner_row: A09"
require_text "$registry" "entrypoint: scripts/build.ps1"
require_text "$registry" "clean_checkout_command: \".\\\\scripts\\\\build.ps1\""
require_text "$registry" "package_command: \".\\\\scripts\\\\build.ps1 -Package\""
require_text "$registry" "visual_studio_role: IDE_only_not_canonical_build_authority"
require_text "$registry" "shipping: [Core, Vehicle, Taxi, World, Traffic, Economy, Persistence, Interaction]"
require_text "$registry" "provider: GitHub_Actions"
require_text "$registry" "runner: self_hosted_Windows_UE"
require_text "$registry" "supported_os: [Windows_10_x64, Windows_11_x64]"
require_text "$registry" "graphics_api: DX12"
require_text "$registry" "storefront: Steam"
require_text "$registry" "frame_budget_ms: 16.67"
require_text "$registry" "rebinding_scope: semantic_KBM"
require_text "$registry" "rebinding_conflict_detection: required"
require_text "$registry" "rebinding_restore_defaults: required"
require_text "$registry" "initial: [L2, Damage, Neural, MovingFuel, ServiceNodes]"
require_text "$registry" "save_slot_model: 3_campaign_slots_with_bounded_rolling_checkpoint_ring"
require_text "$registry" "autosave_checkpoint_policy: atomic_after_meaningful_commits_plus_safe_5min_checkpoint_3_rolling_per_slot"
require_text "$registry" "manual_quit_contract: forbidden_during_active_fare_or_passenger_and_requires_full_stop_after_fare_completion"
require_text "$registry" "workday_end_summary: gross_fare_tips_fines_fuel_repair_service_net_available_money"
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
  docs/PINK_CAB_VEHICLE_TECH_STACK_CHAOS.md \
  docs/PINK_CAB_PRE_MODEL_VEHICLE_FREEZE.md \
  docs/PINK_CAB_TATRA_ASSET_IMPORT_CONTRACT.md \
  docs/PINK_CAB_REPOSITORY_CUTOVER.md; do
  require_file "$file"
done

# Mirror-level current locks.
require_text docs/PINK_CAB_START90_READINESS.md "TOTAL: **196**"
require_text docs/PINK_CAB_START90_READINESS.md "142.5 / 196 = 72.7%"
require_text docs/PINK_CAB_START90_READINESS.md "without consuming any of the remaining 19 OPEN rows"
require_text docs/PINK_CAB_BASE100_TECH_OWNER_PACK_01.md "196 code-facing rows"
require_text docs/OPEN_DECISIONS.md "19 genuine OPEN owner rows"
require_text docs/OPEN_DECISIONS.md "Practice Hangar only where Level1 training/acceptance requires it"
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
  docs/PINK_CAB_VEHICLE_TECH_STACK_CHAOS.md
  docs/PINK_CAB_REPOSITORY_CUTOVER.md
)

for file in "${active_docs[@]}"; do
  reject_regex "$file" 'quit anywhere is allowed|quit-anywhere is a technical proposal' "superseded unrestricted quit contract remains active"
  reject_regex "$file" 'LMB[[:space:]]*=[[:space:]]*ATTENTION|RMB[[:space:]]*=[[:space:]]*GO|attention:[[:space:]]*LMB|go:[[:space:]]*RMB' "superseded universal ATTENTION/GO mapping presented as current"
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
