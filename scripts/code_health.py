from __future__ import annotations

import argparse
import json
import re
import sys
from collections import Counter, defaultdict
from pathlib import Path
from typing import Any

CPP_SUFFIXES = {".h", ".hpp", ".cpp", ".cc"}
CONTROL_TOKENS = re.compile(r"\b(?:if|for|while|switch|case|catch)\b|&&|\|\||\?")
INCLUDE_RE = re.compile(r'^\s*#include\s+"([^"]+)"', re.MULTILINE)
FUNCTION_RE = re.compile(
    r"(?m)^(?![ \t]*(?:if|for|while|switch|catch|else)\b)[ \t]*"
    r"(?P<sig>[^#\n;{}]+?\b(?P<name>[A-Za-z_~][\w:~]*)\s*\([^;{}]*\)\s*"
    r"(?:const\s*)?(?:noexcept\s*)?(?:override\s*)?(?:final\s*)?)\{"
)
BUILD_DEP_RANGE_RE = re.compile(
    r"(?s)(?:Public|Private)DependencyModuleNames\.AddRange\s*\(.*?\{(?P<body>.*?)\}\s*\)\s*;"
)
BUILD_DEP_ADD_RE = re.compile(
    r'(?:Public|Private)DependencyModuleNames\.Add\s*\(\s*"([^"]+)"\s*\)'
)
QUOTED_STRING_RE = re.compile(r'"([^"]+)"')


def _posix(path: Path) -> str:
    return path.as_posix()


def _sanitize_cpp(text: str) -> str:
    out = list(text)
    i = 0
    state = "code"
    while i < len(text):
        ch = text[i]
        nxt = text[i + 1] if i + 1 < len(text) else ""
        if state == "code":
            if ch == "/" and nxt == "/":
                out[i] = out[i + 1] = " "
                i += 2
                state = "line_comment"
                continue
            if ch == "/" and nxt == "*":
                out[i] = out[i + 1] = " "
                i += 2
                state = "block_comment"
                continue
            if ch == '"':
                out[i] = " "
                state = "string"
            elif ch == "'":
                out[i] = " "
                state = "char"
        elif state == "line_comment":
            if ch == "\n":
                state = "code"
            else:
                out[i] = " "
        elif state == "block_comment":
            if ch == "*" and nxt == "/":
                out[i] = out[i + 1] = " "
                i += 2
                state = "code"
                continue
            if ch != "\n":
                out[i] = " "
        elif state in {"string", "char"}:
            quote = '"' if state == "string" else "'"
            if ch == "\\" and nxt:
                if ch != "\n":
                    out[i] = " "
                if nxt != "\n":
                    out[i + 1] = " "
                i += 2
                continue
            if ch == quote:
                out[i] = " "
                state = "code"
            elif ch != "\n":
                out[i] = " "
        i += 1
    return "".join(out)


def _line_number(text: str, offset: int) -> int:
    return text.count("\n", 0, offset) + 1


def _match_brace(text: str, open_index: int) -> int | None:
    depth = 0
    for index in range(open_index, len(text)):
        if text[index] == "{":
            depth += 1
        elif text[index] == "}":
            depth -= 1
            if depth == 0:
                return index
    return None


def _domain_for_file(relative: str, domains: set[str]) -> str | None:
    parts = Path(relative).parts
    for marker in ("Public", "Private"):
        if marker in parts:
            index = parts.index(marker)
            if index + 1 < len(parts) and parts[index + 1] in domains:
                return parts[index + 1]
    return None


def _violation(rule: str, path: str, line: int, detail: str, *, symbol: str = "", value: int | float | None = None, limit: int | float | None = None) -> dict[str, Any]:
    item: dict[str, Any] = {"rule": rule, "path": path, "line": line, "detail": detail, "symbol": symbol}
    if value is not None:
        item["value"] = value
    if limit is not None:
        item["limit"] = limit
    return item


def validate_policy(policy: dict[str, Any]) -> None:
    if not isinstance(policy.get("source_roots"), list) or not policy["source_roots"]:
        raise ValueError("policy.source_roots must be a non-empty list")
    limits = policy.get("thresholds", {})
    for name in ("cpp_loc", "header_loc", "function_loc", "complexity"):
        value = limits.get(name)
        if not isinstance(value, int) or value <= 0:
            raise ValueError(f"threshold {name} must be a positive integer")
    for entry in policy.get("allowlist", []):
        if not isinstance(entry, dict):
            raise ValueError("allowlist entries must be objects")
        required = {"path", "rule", "reason", "expires_when"}
        if not required.issubset(entry):
            raise ValueError("allowlist entries require path/rule/reason/expires_when")
        path = str(entry["path"])
        if any(token in path for token in ("*", "?", "[", "]")):
            raise ValueError(f"broad allowlist path is forbidden: {path}")
        reason = str(entry["reason"]).strip()
        expires = str(entry["expires_when"]).strip()
        if len(reason) < 8 or reason.lower() in {"legacy", "temporary", "because"}:
            raise ValueError(f"allowlist reason is not specific enough: {reason}")
        if len(expires) < 8 or expires.lower() in {"never", "none", "permanent"}:
            raise ValueError(f"allowlist expiry is not actionable: {expires}")


def _fingerprint(item: dict[str, Any]) -> str:
    return "|".join((str(item.get("rule", "")), str(item.get("path", "")), str(item.get("symbol", ""))))


def _tarjan(vertices: set[str], adjacency: dict[str, set[str]]) -> list[list[str]]:
    index = 0
    stack: list[str] = []
    on_stack: set[str] = set()
    indices: dict[str, int] = {}
    lowlinks: dict[str, int] = {}
    components: list[list[str]] = []

    def visit(vertex: str) -> None:
        nonlocal index
        indices[vertex] = lowlinks[vertex] = index
        index += 1
        stack.append(vertex)
        on_stack.add(vertex)
        for target in sorted(adjacency.get(vertex, set())):
            if target not in indices:
                visit(target)
                lowlinks[vertex] = min(lowlinks[vertex], lowlinks[target])
            elif target in on_stack:
                lowlinks[vertex] = min(lowlinks[vertex], indices[target])
        if lowlinks[vertex] == indices[vertex]:
            component: list[str] = []
            while True:
                member = stack.pop()
                on_stack.remove(member)
                component.append(member)
                if member == vertex:
                    break
            components.append(sorted(component))

    for vertex in sorted(vertices):
        if vertex not in indices:
            visit(vertex)
    return sorted(components, key=lambda item: (len(item), item))


def validate_ownership_manifest(root: Path, manifest: dict[str, Any], onboarding_text: str | None = None) -> list[dict[str, Any]]:
    root = root.resolve()
    violations: list[dict[str, Any]] = []
    concerns = manifest.get("concerns")
    if manifest.get("version") != 1 or not isinstance(concerns, list) or not concerns:
        return [_violation("ownership_manifest", "<ownership-manifest>", 0, "manifest requires version=1 and a non-empty concerns list")]
    seen: set[str] = set()
    required = ("id", "owner_domain", "public_contract", "implementation_root", "test_prefix")
    for index, entry in enumerate(concerns, start=1):
        if not isinstance(entry, dict):
            violations.append(_violation("ownership_manifest", "<ownership-manifest>", index, "concern entry must be an object"))
            continue
        missing = [field for field in required if not isinstance(entry.get(field), str) or not entry[field].strip()]
        concern_id = str(entry.get("id", "")).strip()
        if missing:
            violations.append(_violation("ownership_manifest", "<ownership-manifest>", index, f"concern {concern_id or index} missing fields: {', '.join(missing)}", symbol=concern_id))
            continue
        if concern_id in seen:
            violations.append(_violation("ownership_manifest", "<ownership-manifest>", index, f"duplicate concern id: {concern_id}", symbol=concern_id))
        seen.add(concern_id)
        for field in ("public_contract", "implementation_root"):
            relative = entry[field].replace("\\", "/")
            if not (root / relative).exists():
                violations.append(_violation("ownership_manifest", relative, 0, f"{concern_id} {field} does not exist: {relative}", symbol=concern_id))
        if onboarding_text is not None and f"`{concern_id}`" not in onboarding_text:
            violations.append(_violation("ownership_doc_sync", "docs/ENGINEERING_START_HERE.md", 0, f"onboarding is missing concern id: {concern_id}", symbol=concern_id))
    return violations


def _parse_build_modules(root: Path) -> dict[str, dict[str, Any]]:
    modules: dict[str, dict[str, Any]] = {}
    source_root = root / "Source"
    if not source_root.exists():
        return modules
    for path in sorted(source_root.rglob("*.Build.cs")):
        module_name = path.name[:-len(".Build.cs")]
        raw = path.read_text(encoding="utf-8-sig", errors="replace")
        dependencies: set[str] = set()
        for match in BUILD_DEP_RANGE_RE.finditer(raw):
            dependencies.update(QUOTED_STRING_RE.findall(match.group("body")))
        dependencies.update(BUILD_DEP_ADD_RE.findall(raw))
        modules[module_name] = {
            "path": _posix(path.relative_to(root)),
            "dependencies": sorted(dependencies),
        }
    return modules


def _analyze_module_contract(
    root: Path,
    policy: dict[str, Any],
) -> tuple[dict[str, dict[str, Any]], list[str], list[list[str]], list[dict[str, Any]]]:
    contract = policy.get("module_contract")
    if not contract:
        return {}, [], [], []
    modules = _parse_build_modules(root)
    required = set(contract.get("required_modules", []))
    allowed_map = {
        str(module): set(dependencies)
        for module, dependencies in contract.get("allowed_project_dependencies", {}).items()
    }
    project_names = set(allowed_map) | set(modules)
    missing = sorted(required - set(modules))
    violations: list[dict[str, Any]] = [
        _violation(
            "module_missing",
            "<module-graph>",
            0,
            f"required Unreal module is missing: {module}",
            symbol=module,
        )
        for module in missing
    ]

    adjacency: dict[str, set[str]] = defaultdict(set)
    vertices: set[str] = set()
    for module, metadata in modules.items():
        if module not in allowed_map:
            continue
        vertices.add(module)
        allowed = allowed_map[module]
        for dependency in metadata["dependencies"]:
            if dependency not in project_names and not dependency.startswith("PinkCab"):
                continue
            if dependency.startswith("PinkCab") and dependency not in allowed:
                violations.append(_violation(
                    "module_dependency",
                    metadata["path"],
                    1,
                    f"module dependency outside contract: {module} -> {dependency}",
                    symbol=f"{module}->{dependency}",
                ))
            if dependency in allowed_map:
                adjacency[module].add(dependency)
                vertices.add(dependency)

    components = _tarjan(vertices, adjacency)
    cycles = [component for component in components if len(component) > 1]
    for component in cycles:
        violations.append(_violation(
            "module_cycle",
            "<module-graph>",
            0,
            f"Unreal module dependency cycle: {' -> '.join(component)}",
            symbol="|".join(component),
            value=len(component),
            limit=1,
        ))
    return modules, missing, cycles, violations


def analyze_repository(root: Path, policy: dict[str, Any]) -> dict[str, Any]:
    validate_policy(policy)
    root = root.resolve()
    domains = set(policy.get("domains", []))
    limits = policy["thresholds"]
    violations: list[dict[str, Any]] = []
    files: dict[str, dict[str, Any]] = {}
    edge_evidence: dict[tuple[str, str], list[dict[str, Any]]] = defaultdict(list)
    vertices: set[str] = set()
    modules, missing_modules, module_sccs, module_violations = _analyze_module_contract(root, policy)
    violations.extend(module_violations)

    candidates: list[Path] = []
    for source_root in policy["source_roots"]:
        base = root / source_root
        if base.exists():
            candidates.extend(path for path in base.rglob("*") if path.is_file() and path.suffix.lower() in CPP_SUFFIXES)

    for path in sorted(set(candidates)):
        relative = _posix(path.relative_to(root))
        raw = path.read_text(encoding="utf-8-sig", errors="replace")
        sanitized = _sanitize_cpp(raw)
        code_lines = [line for line in sanitized.splitlines() if line.strip()]
        loc = len(code_lines)
        domain = _domain_for_file(relative, domains)
        if domain:
            vertices.add(domain)
        files[relative] = {"loc": loc, "domain": domain, "functions": []}

        is_header = path.suffix.lower() in {".h", ".hpp"}
        file_limit = limits["header_loc"] if is_header else limits["cpp_loc"]
        if loc > file_limit:
            violations.append(_violation("file_loc", relative, 1, f"{loc} non-comment LOC exceeds {file_limit}", value=loc, limit=file_limit))

        for match in FUNCTION_RE.finditer(sanitized):
            open_index = sanitized.find("{", match.start(), match.end() + 1)
            if open_index < 0:
                continue
            close_index = _match_brace(sanitized, open_index)
            if close_index is None:
                continue
            name = match.group("name")
            body = sanitized[open_index:close_index + 1]
            start_line = _line_number(sanitized, match.start())
            end_line = _line_number(sanitized, close_index)
            function_loc = sum(1 for line in sanitized[match.start():close_index + 1].splitlines() if line.strip())
            complexity = 1 + len(CONTROL_TOKENS.findall(body))
            files[relative]["functions"].append({"name": name, "line": start_line, "loc": function_loc, "complexity": complexity})
            if function_loc > limits["function_loc"]:
                violations.append(_violation("function_loc", relative, start_line, f"function {name} is {function_loc} LOC", symbol=name, value=function_loc, limit=limits["function_loc"]))
            if complexity > limits["complexity"]:
                violations.append(_violation("complexity", relative, start_line, f"function {name} complexity is {complexity}", symbol=name, value=complexity, limit=limits["complexity"]))

        if is_header and "/Public/" in f"/{relative}/":
            logic = CONTROL_TOKENS.search(sanitized)
            if logic:
                violations.append(_violation("public_header_logic", relative, _line_number(sanitized, logic.start()), "non-trivial control flow lives in a Public header"))

        raw_policy = policy.get("raw_input", {})
        raw_allowed = any(relative.startswith(prefix.rstrip("/")) for prefix in raw_policy.get("allowed_prefixes", []))
        if not raw_allowed:
            for pattern in raw_policy.get("patterns", []):
                offset = raw.find(pattern)
                if offset >= 0:
                    violations.append(_violation("raw_input_ownership", relative, _line_number(raw, offset), f"raw input API outside Interaction: {pattern}", symbol=pattern))

        chaos_policy = policy.get("chaos_writes", {})
        chaos_allowed = relative in set(chaos_policy.get("allowed_files", []))
        if not chaos_allowed:
            for pattern in chaos_policy.get("patterns", []):
                search_from = 0
                while True:
                    offset = sanitized.find(pattern, search_from)
                    if offset < 0:
                        break
                    violations.append(_violation(
                        "chaos_write_ownership",
                        relative,
                        _line_number(sanitized, offset),
                        f"direct Chaos write outside adapter: {pattern}",
                        symbol=pattern,
                    ))
                    search_from = offset + len(pattern)

        if domain:
            for include in INCLUDE_RE.finditer(raw):
                include_path = include.group(1)
                target = include_path.split("/", 1)[0]
                if target in domains and target != domain:
                    edge_evidence[(domain, target)].append({"path": relative, "line": _line_number(raw, include.start()), "include": include_path})

    for rule in policy.get("forbidden_dependencies", []):
        sources = set(rule.get("from", []))
        targets = set(rule.get("to", []))
        for (source, target), evidence in edge_evidence.items():
            if source in sources and target in targets:
                for item in evidence:
                    violations.append(_violation("forbidden_dependency", item["path"], item["line"], f"forbidden dependency {source} -> {target} via {item['include']}", symbol=f"{source}->{target}:{item['include']}"))

    adjacency: dict[str, set[str]] = defaultdict(set)
    for source, target in edge_evidence:
        adjacency[source].add(target)
    components = _tarjan(vertices, adjacency)
    cycles = [component for component in components if len(component) > 1]
    for component in cycles:
        symbol = "|".join(component)
        violations.append(_violation("dependency_cycle", "<domain-graph>", 0, f"domain dependency cycle: {' -> '.join(component)}", symbol=symbol, value=len(component), limit=1))

    ownership_manifest = policy.get("ownership_manifest")
    if ownership_manifest:
        manifest_path = root / str(ownership_manifest)
        if not manifest_path.exists():
            violations.append(_violation("ownership_manifest", str(ownership_manifest), 0, "configured ownership manifest is missing"))
        else:
            manifest = json.loads(manifest_path.read_text(encoding="utf-8-sig"))
            onboarding_text: str | None = None
            onboarding_doc = policy.get("onboarding_doc")
            if onboarding_doc:
                onboarding_path = root / str(onboarding_doc)
                if onboarding_path.exists():
                    onboarding_text = onboarding_path.read_text(encoding="utf-8-sig", errors="replace")
                else:
                    violations.append(_violation("ownership_doc_sync", str(onboarding_doc), 0, "configured onboarding document is missing"))
            violations.extend(validate_ownership_manifest(root, manifest, onboarding_text))

    allowed: list[dict[str, Any]] = []
    active: list[dict[str, Any]] = []
    allowlist = {(str(item["path"]), str(item["rule"])): item for item in policy.get("allowlist", [])}
    for item in violations:
        entry = allowlist.get((item["path"], item["rule"]))
        if entry:
            copy = dict(item)
            copy["allowlist_reason"] = entry["reason"]
            copy["allowlist_expires_when"] = entry["expires_when"]
            allowed.append(copy)
        else:
            active.append(item)

    active.sort(key=lambda item: (item["rule"], item["path"], item.get("line", 0), item.get("symbol", "")))
    counts = Counter(item["rule"] for item in active)
    edges = [
        {"from": source, "to": target, "evidence_count": len(evidence), "files": sorted({item["path"] for item in evidence})}
        for (source, target), evidence in sorted(edge_evidence.items())
    ]
    return {
        "version": 1,
        "files": files,
        "dependency_edges": edges,
        "dependency_sccs": cycles,
        "modules": modules,
        "missing_modules": missing_modules,
        "module_sccs": module_sccs,
        "violations": active,
        "allowed_violations": allowed,
        "summary": {"files": len(files), "violations": len(active), "by_rule": dict(sorted(counts.items()))},
    }


def build_baseline(report: dict[str, Any]) -> dict[str, Any]:
    grouped: dict[str, dict[str, Any]] = {}
    for item in report.get("violations", []):
        fingerprint = _fingerprint(item)
        bucket = grouped.setdefault(fingerprint, {"rule": item["rule"], "path": item["path"], "symbol": item.get("symbol", ""), "count": 0, "max_value": None})
        bucket["count"] += 1
        value = item.get("value")
        if isinstance(value, (int, float)):
            bucket["max_value"] = value if bucket["max_value"] is None else max(bucket["max_value"], value)
    return {"version": 1, "items": [grouped[key] for key in sorted(grouped)]}


def compare_baseline(report: dict[str, Any], baseline: dict[str, Any]) -> list[dict[str, Any]]:
    old = {f"{item['rule']}|{item['path']}|{item.get('symbol', '')}": item for item in baseline.get("items", [])}
    current_groups: dict[str, list[dict[str, Any]]] = defaultdict(list)
    for item in report.get("violations", []):
        current_groups[_fingerprint(item)].append(item)
    regressions: list[dict[str, Any]] = []
    for fingerprint, items in sorted(current_groups.items()):
        previous = old.get(fingerprint)
        max_value = max((item.get("value") for item in items if isinstance(item.get("value"), (int, float))), default=None)
        worsened = previous is None or len(items) > int(previous.get("count", 0))
        previous_max = previous.get("max_value") if previous else None
        if previous is not None and max_value is not None and previous_max is not None and max_value > previous_max:
            worsened = True
        if worsened:
            sample = items[0]
            regressions.append(_violation("baseline_regression", sample["path"], sample.get("line", 0), f"new or worsened architecture debt: {sample['rule']}", symbol=fingerprint, value=max_value))
    return regressions


def _load_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8-sig"))


def _write_json(path: Path, data: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(data, indent=2, sort_keys=True) + "\n", encoding="utf-8")


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="PINK CAB repository architecture/code-health analyzer")
    parser.add_argument("--root", default=".")
    parser.add_argument("--policy", required=True)
    parser.add_argument("--baseline")
    parser.add_argument("--report")
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--write-baseline", action="store_true")
    parser.add_argument("--require-zero-debt", action="store_true")
    args = parser.parse_args(argv)
    try:
        root = Path(args.root).resolve()
        policy = _load_json(Path(args.policy))
        report = analyze_repository(root, policy)
        regressions: list[dict[str, Any]] = []
        if args.write_baseline:
            if not args.baseline:
                raise ValueError("--write-baseline requires --baseline")
            _write_json(Path(args.baseline), build_baseline(report))
        if args.check and args.baseline:
            baseline_path = Path(args.baseline)
            if not baseline_path.exists():
                raise ValueError(f"baseline does not exist: {baseline_path}")
            regressions = compare_baseline(report, _load_json(baseline_path))
        report["baseline_regressions"] = regressions
        if args.report:
            _write_json(Path(args.report), report)
        print(json.dumps({
            "files": report["summary"]["files"],
            "violations": report["summary"]["violations"],
            "by_rule": report["summary"]["by_rule"],
            "dependency_sccs": report["dependency_sccs"],
            "module_sccs": report.get("module_sccs", []),
            "missing_modules": report.get("missing_modules", []),
            "baseline_regressions": len(regressions),
        }, indent=2, sort_keys=True))
        if args.require_zero_debt and report["violations"]:
            return 2
        if args.check and regressions:
            return 2
        return 0
    except (OSError, ValueError, json.JSONDecodeError) as exc:
        print(f"code-health error: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
