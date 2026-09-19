param(
    [switch]$RequireZeroDebt,
    [switch]$WriteBaseline,
    [string]$ReportPath = ""
)

$ErrorActionPreference = "Stop"
$RepoRoot = Split-Path -Parent $PSScriptRoot
$Policy = Join-Path $RepoRoot "Config\CodeHealthPolicy.json"
$Baseline = Join-Path $RepoRoot "Config\CodeHealthBaseline.json"
if ([string]::IsNullOrWhiteSpace($ReportPath)) {
    $ReportPath = Join-Path $RepoRoot "Saved\Reports\code-health.json"
}

$Args = @(
    (Join-Path $RepoRoot "scripts\code_health.py"),
    "--root", $RepoRoot,
    "--policy", $Policy,
    "--baseline", $Baseline,
    "--report", $ReportPath
)
if ($WriteBaseline) { $Args += "--write-baseline" }
$Args += "--check"
if ($RequireZeroDebt) { $Args += "--require-zero-debt" }

& python @Args
exit $LASTEXITCODE
