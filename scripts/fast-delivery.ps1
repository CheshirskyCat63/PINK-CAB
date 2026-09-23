[CmdletBinding()]
param(
    [string]$TestFilter = "PinkCab.Cockpit.Playable.Runtime",
    [string]$EngineRoot = $env:PINKCAB_UE_ROOT,
    [string]$IterationBuild = $env:PINKCAB_ITERATION_BUILD,
    [string]$ShortcutPath = $env:PINKCAB_LATEST_SHORTCUT,
    [switch]$SkipTests,
    [switch]$ForceRecook,
    [switch]$PlanOnly
)

$ErrorActionPreference = "Stop"
$RepoRoot = Split-Path -Parent $PSScriptRoot
$Project = Join-Path $RepoRoot "PinkCab.uproject"
if ([string]::IsNullOrWhiteSpace($EngineRoot)) {
    $EngineRoot = "C:\Program Files\Epic Games\UE_5.8"
}
$BuildBat = Join-Path $EngineRoot "Engine\Build\BatchFiles\Build.bat"
$RunUAT = Join-Path $EngineRoot "Engine\Build\BatchFiles\RunUAT.bat"
$EditorCmd = Join-Path $EngineRoot "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
if ([string]::IsNullOrWhiteSpace($IterationBuild)) {
    $IterationBuild = Join-Path $RepoRoot "Artifacts\OwnerIteration"
}
if ([string]::IsNullOrWhiteSpace($ShortcutPath)) {
    $Desktop = [Environment]::GetFolderPath("Desktop")
    if ([string]::IsNullOrWhiteSpace($Desktop)) {
        throw "Desktop path is unavailable; set PINKCAB_LATEST_SHORTCUT explicitly."
    }
    $ShortcutPath = Join-Path $Desktop "PINKCAB Latest.lnk"
}

foreach ($Required in @($Project, $BuildBat, $RunUAT, $EditorCmd)) {
    if (-not (Test-Path $Required)) { throw "Required path missing: $Required" }
}
if ([string]::IsNullOrWhiteSpace($env:TMP)) {
    $env:TMP = if ($env:TEMP) { $env:TEMP } else { [System.IO.Path]::GetTempPath() }
}

$Status = @(git -C $RepoRoot status --porcelain)
if ($Status.Count -gt 0) {
    throw "Fast delivery requires a clean worktree so BUILD_SHA is exact. Commit first."
}
$Head = (git -C $RepoRoot rev-parse HEAD).Trim()
$BaselineShaFile = Join-Path $IterationBuild "BUILD_SHA.txt"
$BaselineSha = if (Test-Path $BaselineShaFile) { (Get-Content $BaselineShaFile -Raw).Trim() } else { "" }

$Changed = @()
$BaselineValid = $false
if ($BaselineSha) {
    git -C $RepoRoot cat-file -e "$BaselineSha^{commit}" 2>$null
    $BaselineValid = ($LASTEXITCODE -eq 0)
}
if ($BaselineValid) {
    $Changed = @(git -C $RepoRoot diff --name-only "$BaselineSha..$Head")
}
$RuntimeChanged = @($Changed | Where-Object {
    $_ -notmatch '^(scripts|docs|Docs)/' -and
    $_ -notmatch '^Source/PinkCabTests/' -and
    $_ -notmatch '^(README|CHANGELOG|LICENSE)'
})
$CppOnly = $BaselineValid -and $RuntimeChanged.Count -gt 0
foreach ($Path in $RuntimeChanged) {
    if (-not $Path.EndsWith(".cpp", [StringComparison]::OrdinalIgnoreCase)) {
        $CppOnly = $false
        break
    }
}
$NoRuntimeChange = $BaselineValid -and $RuntimeChanged.Count -eq 0
$NeedsRecook = $ForceRecook -or ((-not $CppOnly) -and (-not $NoRuntimeChange)) -or -not (Test-Path (Join-Path $IterationBuild "Windows\PinkCab.exe"))
$Mode = if ($NeedsRecook) { "INCREMENTAL_RECOOK" } elseif ($NoRuntimeChange) { "NO_RUNTIME_CHANGE" } else { "CPP_BINARY_PATCH" }

Write-Host "PINK CAB fast delivery"
Write-Host "HEAD:     $Head"
Write-Host "Baseline: $BaselineSha"
Write-Host "Mode:     $Mode"
if ($Changed.Count) {
    Write-Host "Changed:"
    $Changed | ForEach-Object { Write-Host "  $_" }
}
if ($PlanOnly) { exit 0 }

$Total = [System.Diagnostics.Stopwatch]::StartNew()

$BuildWatch = [System.Diagnostics.Stopwatch]::StartNew()
& $BuildBat PinkCab Win64 Development $Project -WaitMutex -NoHotReloadFromIDE
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
$BuildWatch.Stop()
Write-Host ("Game build: {0:n1}s" -f $BuildWatch.Elapsed.TotalSeconds)

if (-not $SkipTests) {
    $TestLog = Join-Path $RepoRoot "Saved\Logs\FastDelivery_Test.log"
    Remove-Item $TestLog -Force -ErrorAction SilentlyContinue
    $TestWatch = [System.Diagnostics.Stopwatch]::StartNew()
    # -Multiprocess skips TargetPlatformManager's startup ValidatePlatforms UBT
    # subprocess. This prevents unrelated UE projects from holding the global
    # Build.bat mutex and stalling a local owner iteration for many minutes.
    & $EditorCmd $Project -Multiprocess -unattended -NullRHI -nosplash -nopause -NoSound -stdout -FullStdOutLogOutput "-abslog=$TestLog" "-ExecCmds=Automation RunTests $TestFilter" "-TestExit=Automation Test Queue Empty"
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    $TestWatch.Stop()
    $Success = @(Select-String -Path $TestLog -Pattern 'Test Completed\. Result=\{Success\}')
    $Failures = @(Select-String -Path $TestLog -Pattern 'Test Completed\. Result=\{Fail\}|LogAutomationController: Error:')
    $Queue = @(Select-String -Path $TestLog -Pattern 'Automation Test Queue Empty [0-9]+ tests performed')
    if ($Queue.Count -eq 0 -or $Success.Count -eq 0 -or $Failures.Count -gt 0) {
        throw "Targeted automation did not finish green. See $TestLog"
    }
    Write-Host ("Targeted test: {0:n1}s; {1}" -f $TestWatch.Elapsed.TotalSeconds, $Queue[-1].Line)
}

if ($NeedsRecook) {
    $Next = $IterationBuild + "_next"
    if (Test-Path $Next) { Remove-Item $Next -Recurse -Force }
    $PackageWatch = [System.Diagnostics.Stopwatch]::StartNew()
    & $RunUAT -WaitForUATMutex BuildCookRun "-project=$Project" -noP4 -platform=Win64 -clientconfig=Development -skipbuild -cook -forcerecook=false "-map=/Game/Dev/Maps/L_PinkCab_ChaosWeave" -stage -pak -archive -nodebuginfo "-archivedirectory=$Next" -utf8output
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    $PackageWatch.Stop()
    Set-Content -Path (Join-Path $Next "BUILD_SHA.txt") -Value $Head -NoNewline
    $Old = $IterationBuild + "_old"
    if (Test-Path $Old) { Remove-Item $Old -Recurse -Force }
    if (Test-Path $IterationBuild) { Rename-Item $IterationBuild (Split-Path $Old -Leaf) }
    Rename-Item $Next (Split-Path $IterationBuild -Leaf)
    if (Test-Path $Old) { Remove-Item $Old -Recurse -Force }
    Write-Host ("Incremental package: {0:n1}s" -f $PackageWatch.Elapsed.TotalSeconds)
} elseif ($NoRuntimeChange) {
    Write-Host "No runtime/package-affecting changes; keeping existing BUILD_SHA."
} else {
    $Inner = Join-Path $IterationBuild "Windows\PinkCab\Binaries\Win64\PinkCab.exe"
    Copy-Item (Join-Path $RepoRoot "Binaries\Win64\PinkCab.exe") $Inner -Force
    Set-Content -Path $BaselineShaFile -Value $Head -NoNewline
    Write-Host "Patched game binary into existing cooked package."
}

$SmokeExe = Join-Path $IterationBuild "Windows\PinkCab\Binaries\Win64\PinkCab.exe"
$SmokeLog = Join-Path $IterationBuild "FastDelivery_Smoke.log"
Remove-Item $SmokeLog -Force -ErrorAction SilentlyContinue
$SmokeWatch = [System.Diagnostics.Stopwatch]::StartNew()
$Proc = Start-Process -FilePath $SmokeExe -ArgumentList @(
    "-log", "-NoSound", "-windowed", "-ResX=1280", "-ResY=720", "-abslog=$SmokeLog"
) -WorkingDirectory (Split-Path $SmokeExe) -PassThru
Start-Sleep -Seconds 8
$Alive = -not $Proc.HasExited
if ($Alive) { Stop-Process -Id $Proc.Id -Force; Start-Sleep -Milliseconds 300 }
$SmokeWatch.Stop()
if (-not (Test-Path $SmokeLog)) { throw "Smoke log missing: $SmokeLog" }
$Map = @(Select-String -Path $SmokeLog -Pattern 'UEngine::LoadMap Load map complete /Game/Dev/Maps/L_PinkCab_ChaosWeave')
$Critical = @(Select-String -Path $SmokeLog -Pattern '(?i)Fatal error:|Assertion failed|Ensure condition failed|Unhandled Exception|Log[A-Za-z0-9_]+: Error:')
if (-not $Alive -or $Map.Count -eq 0 -or $Critical.Count -gt 0) {
    throw "Packaged smoke failed: alive=$Alive map=$($Map.Count) critical=$($Critical.Count)"
}
Write-Host ("Packaged smoke: {0:n1}s, green" -f $SmokeWatch.Elapsed.TotalSeconds)

$OuterExe = Join-Path $IterationBuild "Windows\PinkCab.exe"
$Ws = New-Object -ComObject WScript.Shell
$Shortcut = $Ws.CreateShortcut($ShortcutPath)
$Shortcut.TargetPath = $OuterExe
$Shortcut.WorkingDirectory = Join-Path $IterationBuild "Windows"
$Shortcut.Description = "PINK CAB owner iteration $($Head.Substring(0,7))"
$Shortcut.Save()

$Total.Stop()
Write-Host ("DELIVERED {0} in {1:n1}s" -f $Head.Substring(0,7), $Total.Elapsed.TotalSeconds)
Write-Host "Shortcut: $ShortcutPath"
Write-Host "Build:    $IterationBuild"
