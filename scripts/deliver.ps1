[CmdletBinding()]
param(
    [string]$ExpectedHead,
    [string]$EngineRoot = $env:PINKCAB_UE_ROOT,
    [string]$Desktop = [Environment]::GetFolderPath('Desktop'),
    [switch]$Launch
)
$ErrorActionPreference='Stop'
$RepoRoot=Split-Path -Parent $PSScriptRoot
$head=(git -C $RepoRoot rev-parse HEAD).Trim()
if($ExpectedHead -and $head -ne $ExpectedHead){ throw "PINKCAB_EXACT_HEAD_FAIL expected=$ExpectedHead actual=$head" }
if((git -C $RepoRoot status --porcelain).Count -gt 0){ throw 'PINKCAB_DELIVERY_DIRTY_WORKTREE' }

& (Join-Path $RepoRoot 'scripts\build.ps1') -Package -EngineRoot $EngineRoot
if($LASTEXITCODE -ne 0){ exit $LASTEXITCODE }

$package=Join-Path $RepoRoot 'Artifacts\Package\Windows'
$exe=Join-Path $package 'PinkCab.exe'
if(-not (Test-Path $exe)){ throw "PINKCAB_PACKAGE_EXE_MISSING=$exe" }

$smoke=Start-Process -FilePath $exe -ArgumentList @('-unattended','-NullRHI','-nosplash','-ExecCmds=quit') -PassThru
if(-not $smoke.WaitForExit(45000)){
    try{$smoke.Kill()}catch{}
    throw 'PINKCAB_PACKAGE_SMOKE_TIMEOUT'
}
if($smoke.ExitCode -ne 0){ throw "PINKCAB_PACKAGE_SMOKE_FAIL exit=$($smoke.ExitCode)" }

$final=Join-Path $Desktop 'PINCKCAB_BUILD'
$stage=Join-Path $Desktop 'PINCKCAB_BUILD.new'
$old=Join-Path $Desktop 'PINCKCAB_BUILD.old'
Remove-Item $stage -Recurse -Force -ErrorAction SilentlyContinue
Remove-Item $old -Recurse -Force -ErrorAction SilentlyContinue
Copy-Item $package $stage -Recurse -Force
if(Test-Path $final){ Move-Item $final $old }
Move-Item $stage $final
Remove-Item $old -Recurse -Force -ErrorAction SilentlyContinue

$shortcut=Join-Path $Desktop 'PINCKCAB.lnk'
$shell=New-Object -ComObject WScript.Shell
$link=$shell.CreateShortcut($shortcut)
$link.TargetPath=Join-Path $final 'PinkCab.exe'
$link.WorkingDirectory=$final
$link.Description="PINKCAB $head"
$link.Save()
Set-Content -LiteralPath (Join-Path $final 'SOURCE_HEAD.txt') -Value $head -NoNewline

Write-Host "PINKCAB_DELIVERY=PASS head=$head shortcut=$shortcut"
if($Launch){ Start-Process -FilePath (Join-Path $final 'PinkCab.exe') -WorkingDirectory $final }
