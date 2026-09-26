[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [ValidatePattern('^[0-9a-fA-F]{40}$')]
    [string]$CandidateSha,

    [Parameter(Mandatory = $true)]
    [string]$CurrentExe,

    [Parameter(Mandatory = $true)]
    [string]$RollbackRoot,

    [Parameter(Mandatory = $true)]
    [ValidatePattern('^[0-9a-fA-F]{40}$')]
    [string]$RollbackSha,

    [Parameter(Mandatory = $true)]
    [string]$RollbackRunId,

    [Parameter(Mandatory = $true)]
    [string]$ProfileLog,

    [Parameter(Mandatory = $true)]
    [string]$EngineRoot,

    [string]$FreezeRootBase = "",

    [Parameter(Mandatory = $true)]
    [string]$OutputPath
)

$ErrorActionPreference = 'Stop'

function Require-File {
    param([string]$Path, [string]$Marker)
    if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
        throw "$Marker path=$Path"
    }
}

function Read-Trimmed {
    param([string]$Path)
    return (Get-Content -LiteralPath $Path -Raw).Trim()
}

function Sha256 {
    param([string]$Path)
    return (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToLowerInvariant()
}

$CandidateSha = $CandidateSha.ToLowerInvariant()
$RollbackSha = $RollbackSha.ToLowerInvariant()

Require-File $CurrentExe 'PINKCAB_PHY001_CURRENT_EXE_MISSING'
Require-File $ProfileLog 'PINKCAB_PHY001_PROFILE_LOG_MISSING'

$rollbackExe = Join-Path $RollbackRoot 'Windows\PinkCab\Binaries\Win64\PinkCab.exe'
$rollbackBuildShaPath = Join-Path $RollbackRoot 'BUILD_SHA.txt'
$rollbackGateManifestPath = Join-Path $RollbackRoot 'GATE_MANIFEST.txt'
Require-File $rollbackExe 'PINKCAB_PHY001_ROLLBACK_EXE_MISSING'
Require-File $rollbackBuildShaPath 'PINKCAB_PHY001_ROLLBACK_BUILD_SHA_MISSING'
Require-File $rollbackGateManifestPath 'PINKCAB_PHY001_ROLLBACK_MANIFEST_MISSING'

$actualRollbackSha = (Read-Trimmed $rollbackBuildShaPath).ToLowerInvariant()
if ($actualRollbackSha -ne $RollbackSha) {
    throw "PINKCAB_PHY001_ROLLBACK_SHA_MISMATCH actual=$actualRollbackSha expected=$RollbackSha"
}

$rollbackGateLines = @(Get-Content -LiteralPath $rollbackGateManifestPath)
$rollbackSourceLine = @($rollbackGateLines | Where-Object { $_ -match '^source_sha=' } | Select-Object -Last 1)
$rollbackRunLine = @($rollbackGateLines | Where-Object { $_ -match '^workflow_run_id=' } | Select-Object -Last 1)
if ($rollbackSourceLine.Count -eq 0 -or $rollbackRunLine.Count -eq 0) {
    throw 'PINKCAB_PHY001_ROLLBACK_MANIFEST_IDENTITY_MISSING'
}
$manifestRollbackSha = (($rollbackSourceLine[0] -split '=', 2)[1]).Trim().ToLowerInvariant()
$manifestRollbackRun = (($rollbackRunLine[0] -split '=', 2)[1]).Trim()
if ($manifestRollbackSha -ne $RollbackSha) {
    throw "PINKCAB_PHY001_ROLLBACK_MANIFEST_SHA_MISMATCH actual=$manifestRollbackSha expected=$RollbackSha"
}
if ($manifestRollbackRun -ne $RollbackRunId) {
    throw "PINKCAB_PHY001_ROLLBACK_RUN_MISMATCH actual=$manifestRollbackRun expected=$RollbackRunId"
}

$profileLine = @(Select-String -LiteralPath $ProfileLog -Pattern 'PINKCAB_PHYSICS_PROFILE ModelId=' | Select-Object -Last 1)
if ($profileLine.Count -eq 0) {
    throw 'PINKCAB_PHY001_PROFILE_MARKER_MISSING'
}
$profileMatch = [regex]::Match(
    $profileLine[0].Line,
    'PINKCAB_PHYSICS_PROFILE ModelId=(?<model>\S+) ProfileId=(?<profile>\S+) Schema=(?<schema>\d+) Calibration=(?<calibration>\d+) Variant=(?<variant>\d+) Hash=(?<hash>[0-9A-Fa-f]{16})')
if (-not $profileMatch.Success) {
    throw "PINKCAB_PHY001_PROFILE_MARKER_INVALID line=$($profileLine[0].Line)"
}

$buildVersionPath = Join-Path $EngineRoot 'Engine\Build\Build.version'
Require-File $buildVersionPath 'PINKCAB_PHY001_UE_BUILD_VERSION_MISSING'
$buildVersion = Get-Content -LiteralPath $buildVersionPath -Raw | ConvertFrom-Json
$ueVersion = '{0}.{1}.{2}' -f $buildVersion.MajorVersion, $buildVersion.MinorVersion, $buildVersion.PatchVersion

$currentSignature = (Get-AuthenticodeSignature -FilePath $CurrentExe).Status.ToString()
$rollbackSignature = (Get-AuthenticodeSignature -FilePath $rollbackExe).Status.ToString()
if ($currentSignature -ne 'Valid') {
    throw "PINKCAB_PHY001_CURRENT_SIGNATURE_INVALID status=$currentSignature"
}
if ($rollbackSignature -ne 'Valid') {
    throw "PINKCAB_PHY001_ROLLBACK_SIGNATURE_INVALID status=$rollbackSignature"
}

$currentItem = Get-Item -LiteralPath $CurrentExe
$rollbackItem = Get-Item -LiteralPath $rollbackExe
$currentHash = Sha256 $CurrentExe
$rollbackHash = Sha256 $rollbackExe
$profileHash = $profileMatch.Groups['hash'].Value.ToUpperInvariant()
$capturedUtc = (Get-Date).ToUniversalTime().ToString('o')
$runId = [string]$env:GITHUB_RUN_ID
$runUrl = if ($env:GITHUB_SERVER_URL -and $env:GITHUB_REPOSITORY -and $runId) {
    "$env:GITHUB_SERVER_URL/$env:GITHUB_REPOSITORY/actions/runs/$runId"
} else {
    ''
}

$frozenRoot = ''
$frozenExe = ''
if (-not [string]::IsNullOrWhiteSpace($FreezeRootBase)) {
    if ([string]::IsNullOrWhiteSpace($runId)) {
        throw 'PINKCAB_PHY001_FREEZE_REQUIRES_GITHUB_RUN_ID'
    }
    $short = $CandidateSha.Substring(0, 7)
    $frozenRoot = Join-Path $FreezeRootBase ("PHYSICS_BASELINE_{0}_RUN{1}" -f $short, $runId)
    if (Test-Path -LiteralPath $frozenRoot) {
        throw "PINKCAB_PHY001_IMMUTABLE_BASELINE_EXISTS path=$frozenRoot"
    }

    New-Item -ItemType Directory -Force -Path $frozenRoot | Out-Null
    & robocopy $RollbackRoot $frozenRoot /E /COPY:DAT /DCOPY:DAT /R:1 /W:1 /NFL /NDL /NJH /NJS /NP
    $robocopyCode = $LASTEXITCODE
    if ($robocopyCode -ge 8) {
        throw "PINKCAB_PHY001_BASELINE_COPY_FAILED code=$robocopyCode"
    }
    $global:LASTEXITCODE = 0

    $frozenExe = Join-Path $frozenRoot 'Windows\PinkCab\Binaries\Win64\PinkCab.exe'
    Require-File $frozenExe 'PINKCAB_PHY001_FROZEN_EXE_TARGET_MISSING'
    Copy-Item -LiteralPath $CurrentExe -Destination $frozenExe -Force
    if ((Sha256 $frozenExe) -ne $currentHash) {
        throw 'PINKCAB_PHY001_FROZEN_EXE_HASH_MISMATCH'
    }
    if ((Get-AuthenticodeSignature -FilePath $frozenExe).Status.ToString() -ne 'Valid') {
        throw 'PINKCAB_PHY001_FROZEN_EXE_SIGNATURE_INVALID'
    }

    Set-Content -LiteralPath (Join-Path $frozenRoot 'BUILD_SHA.txt') -Value $CandidateSha -NoNewline
    Set-Content -LiteralPath (Join-Path $frozenRoot 'PHYSICS_PROFILE_HASH.txt') -Value $profileHash -NoNewline
    @(
        "source_sha=$CandidateSha",
        "workflow_run_id=$runId",
        'mode=vehicle_physics_baseline_overlay',
        "derived_from_rollback_sha=$RollbackSha",
        "derived_from_rollback_run_id=$RollbackRunId",
        "profile_hash=$profileHash"
    ) | Set-Content -LiteralPath (Join-Path $frozenRoot 'GATE_MANIFEST.txt')
}

$manifest = [ordered]@{
    schema_version = 1
    task = 'PHY-001'
    captured_utc = $capturedUtc
    candidate_sha = $CandidateSha
    github_run_id = $runId
    github_run_url = $runUrl
    unreal_engine_version = $ueVersion
    profile = [ordered]@{
        model_id = $profileMatch.Groups['model'].Value
        profile_id = $profileMatch.Groups['profile'].Value
        schema_version = [int]$profileMatch.Groups['schema'].Value
        calibration_version = [int]$profileMatch.Groups['calibration'].Value
        calibration_variant = [int]$profileMatch.Groups['variant'].Value
        deterministic_hash = $profileHash
    }
    current_executable = [ordered]@{
        source_path = $currentItem.FullName
        sha256 = $currentHash
        size_bytes = [int64]$currentItem.Length
        authenticode = $currentSignature
        frozen_root = $frozenRoot
        frozen_executable = $frozenExe
    }
    rollback = [ordered]@{
        root = (Resolve-Path -LiteralPath $RollbackRoot).Path
        build_sha = $RollbackSha
        workflow_run_id = $RollbackRunId
        executable_path = $rollbackItem.FullName
        sha256 = $rollbackHash
        size_bytes = [int64]$rollbackItem.Length
        authenticode = $rollbackSignature
    }
}

$outputDirectory = Split-Path -Parent $OutputPath
if (-not [string]::IsNullOrWhiteSpace($outputDirectory)) {
    New-Item -ItemType Directory -Force -Path $outputDirectory | Out-Null
}
$json = $manifest | ConvertTo-Json -Depth 8
Set-Content -LiteralPath $OutputPath -Value $json -Encoding UTF8
if (-not [string]::IsNullOrWhiteSpace($frozenRoot)) {
    Set-Content -LiteralPath (Join-Path $frozenRoot 'PHY001_BASELINE.json') -Value $json -Encoding UTF8
}

Write-Host "PINKCAB_PHY001_CANDIDATE_SHA=$CandidateSha"
Write-Host "PINKCAB_PHY001_UE_VERSION=$ueVersion"
Write-Host "PINKCAB_PHY001_PROFILE_HASH=$profileHash"
Write-Host "PINKCAB_PHY001_CURRENT_EXE_SHA256=$currentHash"
Write-Host "PINKCAB_PHY001_ROLLBACK_SHA=$RollbackSha"
Write-Host "PINKCAB_PHY001_ROLLBACK_RUN_ID=$RollbackRunId"
Write-Host "PINKCAB_PHY001_ROLLBACK_EXE_SHA256=$rollbackHash"
if (-not [string]::IsNullOrWhiteSpace($frozenRoot)) {
    Write-Host "PINKCAB_PHY001_FROZEN_ROOT=$frozenRoot"
}
Write-Host "PINKCAB_PHY001_BASELINE_CAPTURE=PASS"
