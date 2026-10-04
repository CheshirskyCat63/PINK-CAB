param(
    [Parameter(Mandatory=$true)][string]$ExePath,
    [Parameter(Mandatory=$true)][string]$LogRoot,
    [string]$Map = '/Game/Dev/Maps/L_PinkCab_L1_EndlessStraight',
    [int]$TimeoutSecondsPerCase = 110
)

$ErrorActionPreference = 'Stop'

if(-not (Test-Path -LiteralPath $ExePath)){
    throw "PINKCAB_P04_PACKAGE_EXE_MISSING=$ExePath"
}
New-Item -ItemType Directory -Force -Path $LogRoot | Out-Null

function Read-Result([string]$Line){
    $result=@{}
    foreach($m in [regex]::Matches($Line,'([A-Za-z0-9_]+)=([^\s]+)')){
        $result[$m.Groups[1].Value]=$m.Groups[2].Value
    }
    return $result
}

$cases=@(
    'forward_baseline',
    'forward_candidate',
    'reverse25',
    'reverse50',
    'reverse100',
    'lift',
    'counter_low',
    'counter_urban',
    'counter_high',
    'top'
)
$results=@{}

foreach($case in $cases){
    $log=Join-Path $LogRoot ("P04_PACKAGED_{0}.log" -f $case)
    Remove-Item -LiteralPath $log -Force -ErrorAction SilentlyContinue
    Write-Host "PINKCAB_P04_PACKAGE_CASE_START=$case"

    $proc=Start-Process -FilePath $ExePath -ArgumentList @(
        $Map,
        '-log',
        '-windowed',
        '-ResX=640',
        '-ResY=360',
        '-NoSound',
        '-PinkCabGateTelemetry',
        ("-PinkCabP04Acceptance={0}" -f $case),
        ("-abslog={0}" -f $log)
    ) -WorkingDirectory (Split-Path $ExePath) -PassThru

    try{
        $deadline=[DateTime]::UtcNow.AddSeconds($TimeoutSecondsPerCase)
        $resultLine=$null
        do{
            Start-Sleep -Milliseconds 250
            $proc.Refresh()
            if(Test-Path -LiteralPath $log){
                $resultMatch=Select-String -LiteralPath $log -Pattern 'PINKCAB_P04_RESULT ' | Select-Object -Last 1
                if($resultMatch){
                    $resultLine=$resultMatch.Line
                    break
                }
            }
            if($proc.HasExited){
                break
            }
        }while([DateTime]::UtcNow -lt $deadline)

        if(-not $resultLine -and (Test-Path -LiteralPath $log)){
            $resultMatch=Select-String -LiteralPath $log -Pattern 'PINKCAB_P04_RESULT ' | Select-Object -Last 1
            if($resultMatch){ $resultLine=$resultMatch.Line }
        }
        if(-not $resultLine){
            throw "PINKCAB_P04_PACKAGE_RESULT_MISSING case=$case exited=$($proc.HasExited)"
        }

        $critical=@(
            Select-String -LiteralPath $log -Pattern '(?i)Fatal error:|Assertion failed|Unhandled Exception|LogStreaming: Error:|LogLinker: Error:' -ErrorAction SilentlyContinue
        )
        if($critical.Count -gt 0){
            throw "PINKCAB_P04_PACKAGE_CRITICAL case=$case count=$($critical.Count)"
        }

        $parsed=Read-Result $resultLine
        if(-not $parsed.ContainsKey('pass') -or $parsed['pass'] -ne '1'){
            throw "PINKCAB_P04_PACKAGE_CASE_FAIL case=$case result=$resultLine"
        }
        $results[$case]=$parsed
        Write-Host $resultLine
        Write-Host "PINKCAB_P04_PACKAGE_CASE_PASS=$case"
    }
    finally{
        if($proc -and -not $proc.HasExited){
            Stop-Process -Id $proc.Id -Force -ErrorAction SilentlyContinue
        }
    }
}

function N([hashtable]$Row,[string]$Key){
    if(-not $Row.ContainsKey($Key)){ throw "PINKCAB_P04_RESULT_FIELD_MISSING=$Key" }
    return [double]::Parse($Row[$Key],[Globalization.CultureInfo]::InvariantCulture)
}

$baseline=$results['forward_baseline']
$candidate=$results['forward_candidate']
$base30=N $baseline 'time30_s'
$base60=N $baseline 'time60_s'
$cand30=N $candidate 'time30_s'
$cand60=N $candidate 'time60_s'
if($base30 -le 0 -or $base60 -le 0 -or $cand30 -le 0 -or $cand60 -le 0){
    throw "PINKCAB_P04_FORWARD_TIMES_INVALID base30=$base30 base60=$base60 cand30=$cand30 cand60=$cand60"
}
if($cand30 -ge $base30 -or $cand60 -ge $base60){
    throw "PINKCAB_P04_AB_NOT_IMPROVED base30=$base30 cand30=$cand30 base60=$base60 cand60=$cand60"
}
Write-Host ("PINKCAB_P04_PHY017_AB=PASS base30={0:F4} cand30={1:F4} base60={2:F4} cand60={3:F4}" -f $base30,$cand30,$base60,$cand60)

$r25=N $results['reverse25'] 'neutral_entry_kmh'
$r50=N $results['reverse50'] 'neutral_entry_kmh'
$r100=N $results['reverse100'] 'neutral_entry_kmh'
if(-not ($r25 -lt $r50 -and $r50 -lt $r100)){
    throw "PINKCAB_P04_REVERSE_DOSES_NOT_DISTINCT r25=$r25 r50=$r50 r100=$r100"
}
Write-Host ("PINKCAB_P04_PHY019_REVERSE=PASS r25={0:F3} r50={1:F3} r100={2:F3}" -f $r25,$r50,$r100)

foreach($case in @('counter_low','counter_urban','counter_high')){
    $right=N $results[$case] 'right_steer'
    $counter=N $results[$case] 'counter_steer'
    if($right -lt 0.99 -or $counter -gt -0.99){
        throw "PINKCAB_P04_COUNTERSTEER_AUTHORITY_FAIL case=$case right=$right counter=$counter"
    }
}
Write-Host 'PINKCAB_P04_T026_COUNTERSTEER=PASS low_urban_high=1'

$top=$results['top']
$peak=N $top 'peak_speed_kmh'
$ratioError=N $top 'gear5_ratio_error_pct'
$postLift=N $top 'post_lift_max_rear_drive_nm'
if($peak -lt 195.0){
    throw "PINKCAB_P04_TOP_SPEED_TARGET_FAIL peak=$peak"
}
if($ratioError -lt 0 -or $ratioError -gt 8.0){
    throw "PINKCAB_P04_GEAR5_RATIO_FAIL error_pct=$ratioError"
}
if($postLift -gt 1.0){
    throw "PINKCAB_P04_GEAR5_RESIDUAL_BOOST_FAIL torque_nm=$postLift"
}
Write-Host ("PINKCAB_P04_PHY018_TOP=PASS peak_kmh={0:F3} gear5_ratio_error_pct={1:F3}" -f $peak,$ratioError)
Write-Host ("PINKCAB_P04_PHY036_NO_ANTISTALL=PASS post_lift_max_rear_drive_nm={0:F3}" -f $postLift)

Write-Host 'PINKCAB_P04_PACKAGED_ACCEPTANCE=PASS PHY017=1 PHY018=1 PHY019=1 PHY020=1 T026=1'
