#!/usr/bin/env python3
import argparse
import csv
import json
import re
import sys
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]

def in_scope(path: Path) -> bool:
    p = path.as_posix()
    return (
        p.startswith("Source/PinkCabVehicle/")
        or p.startswith("Source/PinkCab/Private/Vehicle/")
        or p.startswith("Source/PinkCab/Public/Vehicle/")
        or re.match(r"Source/PinkCab/Private/Runtime/PinkCabChaosTatraPawn.*\.(cpp|h)$", p)
        or p == "Source/PinkCab/Public/Runtime/PinkCabChaosTatraPawn.h"
        or p.startswith("Source/PinkCab/Private/Service/")
        or p.startswith("Source/PinkCab/Public/Service/")
        or p in {
            "Source/PinkCab/Private/Persistence/PinkCabVehicleSnapshotArchive.cpp",
            "Source/PinkCab/Public/Persistence/PinkCabVehicleSnapshotArchive.h",
        }
        or re.match(r"Source/PinkCabPersistence/(Private|Public)/Persistence/PinkCabVehicleSnapshot.*\.(cpp|h)$", p)
    ) and "Source/PinkCabTests/" not in p

WRITER_PATTERNS = {
    "CHAOS_STEERING_INPUT": r"\bSetSteeringInput\s*\(",
    "CHAOS_THROTTLE_INPUT": r"\bSetThrottleInput\s*\(",
    "CHAOS_BRAKE_INPUT": r"\bSetBrakeInput\s*\(",
    "CHAOS_HANDBRAKE_INPUT": r"\bSetHandbrakeInput\s*\(",
    "CHAOS_WHEEL_DRIVE_TORQUE": r"\bSetDriveTorque\s*\(",
    "CHAOS_WHEEL_BRAKE_TORQUE": r"\bSetBrakeTorque\s*\(",
    "CHAOS_TORQUE_COMBINE": r"\bSetTorqueCombineMethod\s*\(",
    "CHAOS_TARGET_GEAR": r"\bSetTargetGear\s*\(",
    "CHAOS_MECHANICAL_SIM": r"\bEnableMechanicalSim\s*\(",
    "CHAOS_MASS": r"\bMovement\.Mass\s*=",
    "CHAOS_COM_OVERRIDE_ENABLE": r"\bMovement\.bEnableCenterOfMassOverride\s*=",
    "CHAOS_COM_OVERRIDE_VALUE": r"\bMovement\.CenterOfMassOverride\.[XYZ]\s*=",
    "CHAOS_MASS_PROPERTIES": r"\bUpdateMassProperties\s*\(",
    "CHAOS_ENGINE_MAX_TORQUE": r"\bMovement\.EngineSetup\.MaxTorque\s*=",
    "CHAOS_ENGINE_MAX_RPM": r"\bMovement\.EngineSetup\.MaxRPM\s*=",
    "CHAOS_ENGINE_IDLE_RPM": r"\bMovement\.EngineSetup\.EngineIdleRPM\s*=",
    "CHAOS_ENGINE_BRAKE_EFFECT": r"\bMovement\.EngineSetup\.EngineBrakeEffect\s*=",
    "CHAOS_ENGINE_REV_MOI": r"\bMovement\.EngineSetup\.EngineRevUpMOI\s*=",
    "CHAOS_FINAL_DRIVE": r"\bMovement\.TransmissionSetup\.FinalRatio\s*=",
    "CHAOS_FORWARD_RATIOS": r"\bMovement\.TransmissionSetup\.ForwardGearRatios\s*=",
    "CHAOS_REVERSE_RATIOS": r"\bMovement\.TransmissionSetup\.ReverseGearRatios\s*=",
    "CHAOS_TORQUE_ASSIST_ENABLE": r"\bMovement\.TorqueControl\.Enabled\s*=",
    "CHAOS_TARGET_ROTATION_ASSIST_ENABLE": r"\bMovement\.TargetRotationControl\.Enabled\s*=",
    "CHAOS_STABILIZE_ASSIST_ENABLE": r"\bMovement\.StabilizeControl\.Enabled\s*=",
    "IGNITION_START": r"\bStartEngine\s*\(",
    "IGNITION_STOP": r"\bStopEngine\s*\(",
    "IGNITION_STALL": r"\bStallEngine\s*\(",
    "GEAR_REQUEST": r"\bRequestGear\s*\(",
    "GEAR_FORCE_STATE": r"\bForceState\s*\(",
    "COCKPIT_SELECTED_GEAR": r"\bSetSelectedGear\s*\(",
    "CONTROL_STEERING": r"\bSetSteering\s*\(",
    "CONTROL_CLUTCH": r"\bSetClutch\s*\(",
    "CONTROL_HANDBRAKE": r"\bSetHandbrake\s*\(",
    "CONTROL_DRIVELINE": r"\bSetDriveline\s*\(",
    "CONTROL_TORQUE_CAPACITY": r"\bSetDrivetrainTorqueCapacity\s*\(",
    "CONTROL_EXTERNAL_REAR_TORQUE": r"\bSetExternalRearDriveTorquePerWheel\s*\(",
    "CONTROL_RESOLVED_ENGINE_ACTUATION": r"\bSetResolvedEngineActuation\s*\(",
    "HEALTH_DAMAGE": r"\bApplyFunctionalDamage\s*\(",
    "HEALTH_REPAIR": r"\bRestoreFunctionalHealthTo\s*\(",
    "HEALTH_CLUTCH_TEMP": r"\bSetClutchTemperature01\s*\(",
    "HEALTH_BRAKE_TEMP": r"\bSetBrakeTemperature01\s*\(",
    "LOAD_FUEL": r"\bSetFuelMassKg\s*\(",
    "LOAD_CREW": r"\bSetCrew\s*\(",
}

FORBIDDEN_PATTERNS = {
    "DIRECT_ADD_FORCE": r"\bAddForce(?:AtLocation)?\s*\(",
    "DIRECT_ADD_IMPULSE": r"\bAddImpulse(?:AtLocation)?\s*\(",
    "DIRECT_ADD_TORQUE": r"\bAddTorque(?:InRadians|InDegrees)?\s*\(",
    "DIRECT_LINEAR_VELOCITY_WRITE": r"\b(?:SetPhysicsLinearVelocity|SetAllPhysicsLinearVelocity|SetLinearVelocity)\s*\(",
    "DIRECT_ANGULAR_VELOCITY_WRITE": r"\b(?:SetPhysicsAngularVelocity(?:InRadians|InDegrees)?|SetAllPhysicsAngularVelocity(?:InRadians|InDegrees)?|SetAngularVelocity)\s*\(",
    "DIRECT_ACTOR_TELEPORT": r"\b(?:AddActorWorldOffset|SetActorLocation(?:AndRotation)?)\s*\(",
    "CHAOS_ASSIST_NONFALSE": r"\bMovement\.(?:TorqueControl|TargetRotationControl|StabilizeControl)\.Enabled\s*=\s*(?!false\b)[A-Za-z_]",
}

def scan():
    counts = Counter()
    samples = {}
    forbidden = []
    for path in sorted(ROOT.glob("Source/**/*")):
        if not path.is_file() or path.suffix not in {".cpp", ".h"} or not in_scope(path.relative_to(ROOT)):
            continue
        rel = path.relative_to(ROOT).as_posix()
        # Inventory executable writer sites. Most headers are declarations only;
        # keep the inline control-state setters because they mutate the transport state.
        if path.suffix == ".h" and rel != "Source/PinkCabVehicle/Public/Vehicle/PinkCabVehicleControlState.h":
            continue
        text = path.read_text(encoding="utf-8", errors="replace")
        for category, pattern in WRITER_PATTERNS.items():
            matches = list(re.finditer(pattern, text))
            if matches:
                counts[(category, rel)] += len(matches)
                samples[(category, rel)] = [
                    text.count("\n", 0, m.start()) + 1 for m in matches[:8]
                ]
        for category, pattern in FORBIDDEN_PATTERNS.items():
            for m in re.finditer(pattern, text):
                forbidden.append({
                    "category": category,
                    "path": rel,
                    "line": text.count("\n", 0, m.start()) + 1,
                })
    return counts, samples, forbidden

def load_manifest(path: Path):
    rows = {}
    with path.open(newline="", encoding="utf-8-sig") as fh:
        reader = csv.DictReader(fh)
        required = {"category", "path", "expected_count", "authority", "classification", "notes"}
        if set(reader.fieldnames or []) != required:
            raise SystemExit(
                "PINKCAB_PHY002_MANIFEST_SCHEMA_INVALID "
                f"expected={sorted(required)} actual={reader.fieldnames}"
            )
        for row in reader:
            key = (row["category"].strip(), row["path"].strip())
            if not all(key) or key in rows:
                raise SystemExit(f"PINKCAB_PHY002_MANIFEST_ROW_INVALID key={key}")
            try:
                expected = int(row["expected_count"])
            except ValueError as exc:
                raise SystemExit(f"PINKCAB_PHY002_MANIFEST_COUNT_INVALID key={key}") from exc
            if expected <= 0 or not row["authority"].strip() or not row["classification"].strip() or not row["notes"].strip():
                raise SystemExit(f"PINKCAB_PHY002_MANIFEST_METADATA_INVALID key={key}")
            rows[key] = {**row, "expected_count": expected}
    return rows

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--manifest", required=True)
    ap.add_argument("--evidence", required=True)
    args = ap.parse_args()

    manifest_path = ROOT / args.manifest
    evidence_path = ROOT / args.evidence
    if not manifest_path.is_file():
        raise SystemExit(f"PINKCAB_PHY002_MANIFEST_MISSING path={args.manifest}")

    actual, samples, forbidden = scan()
    manifest = load_manifest(manifest_path)

    unknown = [
        {"category": k[0], "path": k[1], "actual_count": c, "lines": samples.get(k, [])}
        for k, c in sorted(actual.items())
        if k not in manifest
    ]
    stale = [
        {"category": k[0], "path": k[1], "expected_count": v["expected_count"]}
        for k, v in sorted(manifest.items())
        if k not in actual
    ]
    mismatched = [
        {
            "category": k[0],
            "path": k[1],
            "expected_count": manifest[k]["expected_count"],
            "actual_count": c,
            "lines": samples.get(k, []),
        }
        for k, c in sorted(actual.items())
        if k in manifest and manifest[k]["expected_count"] != c
    ]

    evidence = {
        "schema_version": 1,
        "task": "PHY-002",
        "manifest": args.manifest,
        "scoped_writer_occurrences": sum(actual.values()),
        "scoped_writer_groups": len(actual),
        "manifest_groups": len(manifest),
        "unknown": unknown,
        "stale": stale,
        "mismatched": mismatched,
        "forbidden_side_paths": forbidden,
        "actual": [
            {
                "category": k[0],
                "path": k[1],
                "count": c,
                "lines": samples.get(k, []),
                "authority": manifest.get(k, {}).get("authority", ""),
                "classification": manifest.get(k, {}).get("classification", ""),
            }
            for k, c in sorted(actual.items())
        ],
    }
    evidence_path.parent.mkdir(parents=True, exist_ok=True)
    evidence_path.write_text(json.dumps(evidence, indent=2) + "\n", encoding="utf-8")

    for row in unknown:
        print("PINKCAB_PHY002_UNKNOWN_WRITER", json.dumps(row, sort_keys=True))
    for row in stale:
        print("PINKCAB_PHY002_STALE_MANIFEST", json.dumps(row, sort_keys=True))
    for row in mismatched:
        print("PINKCAB_PHY002_COUNT_MISMATCH", json.dumps(row, sort_keys=True))
    for row in forbidden:
        print("PINKCAB_PHY002_FORBIDDEN_SIDE_PATH", json.dumps(row, sort_keys=True))

    if unknown or stale or mismatched or forbidden:
        print(
            "PINKCAB_PHY002_WRITER_INVENTORY=FAIL "
            f"groups={len(actual)} unknown={len(unknown)} stale={len(stale)} "
            f"mismatched={len(mismatched)} forbidden={len(forbidden)}"
        )
        return 1

    print(
        "PINKCAB_PHY002_WRITER_INVENTORY=PASS "
        f"groups={len(actual)} occurrences={sum(actual.values())}"
    )
    return 0

if __name__ == "__main__":
    sys.exit(main())
