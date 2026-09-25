param(
    [Parameter(Mandatory=$true)][string]$Exe,
    [Parameter(Mandatory=$true)][string]$EvidenceDir,
    [string]$Map = ""
)

$ErrorActionPreference = "Stop"
New-Item -ItemType Directory -Force -Path $EvidenceDir | Out-Null
$log = Join-Path $EvidenceDir "PACKAGED_OS_INPUT.log"
$summary = Join-Path $EvidenceDir "PACKAGED_OS_INPUT_SUMMARY.txt"
$screenshot = Join-Path $EvidenceDir "PACKAGED_OS_INPUT.png"
Remove-Item $log,$summary,$screenshot -Force -ErrorAction SilentlyContinue

Add-Type -TypeDefinition @"
using System;
using System.Runtime.InteropServices;
public static class PinkCabNativeInput {
    [StructLayout(LayoutKind.Sequential)]
    public struct RECT { public int Left, Top, Right, Bottom; }
    [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr hWnd);
    [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr hWnd, int nCmdShow);
    [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr hWnd, out RECT rect);
    [DllImport("user32.dll")] public static extern IntPtr GetForegroundWindow();
    [DllImport("user32.dll")] public static extern short GetAsyncKeyState(int vKey);
    [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr hWnd, IntPtr processId);
    [DllImport("kernel32.dll")] public static extern uint GetCurrentThreadId();
    [DllImport("user32.dll")] public static extern bool AttachThreadInput(uint idAttach, uint idAttachTo, bool fAttach);
    [DllImport("user32.dll")] public static extern bool BringWindowToTop(IntPtr hWnd);
    [DllImport("user32.dll")] public static extern bool SetActiveWindow(IntPtr hWnd);
    [DllImport("user32.dll")] public static extern bool SetFocus(IntPtr hWnd);
    [DllImport("user32.dll")] public static extern bool SetCursorPos(int x, int y);
    [DllImport("user32.dll", SetLastError=true)] public static extern bool PostMessage(IntPtr hWnd, uint Msg, IntPtr wParam, IntPtr lParam);
    [DllImport("user32.dll")] static extern void keybd_event(byte vk, byte scan, uint flags, UIntPtr extra);
    [DllImport("user32.dll")] static extern void mouse_event(uint flags, int dx, int dy, uint data, UIntPtr extra);
    const uint KEYUP=0x0002, MOVE=0x0001, LEFTDOWN=0x0002, LEFTUP=0x0004, RIGHTDOWN=0x0008, RIGHTUP=0x0010, WHEEL=0x0800;
    public static void KeyDown(byte vk) { keybd_event(vk,0,0,UIntPtr.Zero); }
    public static void KeyUp(byte vk) { keybd_event(vk,0,KEYUP,UIntPtr.Zero); }
    public static void Move(int dx,int dy) { mouse_event(MOVE,dx,dy,0,UIntPtr.Zero); }
    public static void Wheel(int delta) { mouse_event(WHEEL,0,0,unchecked((uint)delta),UIntPtr.Zero); }
    public static void WheelWindow(IntPtr hWnd, int delta) {
        const uint WM_MOUSEWHEEL = 0x020A;
        long packed = ((long)(ushort)unchecked((short)delta)) << 16;
        PostMessage(hWnd, WM_MOUSEWHEEL, (IntPtr)packed, IntPtr.Zero);
    }
    public static void LeftDown() { mouse_event(LEFTDOWN,0,0,0,UIntPtr.Zero); }
    public static void LeftUp() { mouse_event(LEFTUP,0,0,0,UIntPtr.Zero); }
    public static void RightDown() { mouse_event(RIGHTDOWN,0,0,0,UIntPtr.Zero); }
    public static void RightUp() { mouse_event(RIGHTUP,0,0,0,UIntPtr.Zero); }
    public static bool IsKeyDown(int vk) { return (GetAsyncKeyState(vk) & 0x8000) != 0; }
}
"@
Add-Type -AssemblyName System.Drawing

function Tap-Key([byte]$Vk, [int]$HoldMs=90) {
    [PinkCabNativeInput]::KeyDown($Vk)
    Start-Sleep -Milliseconds $HoldMs
    [PinkCabNativeInput]::KeyUp($Vk)
    Start-Sleep -Milliseconds 120
}
function Get-State {
    if (-not (Test-Path $log)) { return $null }
    $line = Get-Content $log -Tail 500 -ErrorAction SilentlyContinue |
        Where-Object { $_ -match 'PINKCAB_GATE_STATE ' } |
        Select-Object -Last 1
    if (-not $line) { return $null }
    $rx = 'PINKCAB_GATE_STATE menu=(?<menu>\d+) paused=(?<paused>\d+) worlddt=(?<worlddt>-?[\d.]+) awake=(?<awake>\d+) velcm=(?<velcm>-?[\d.]+) ignition=(?<ignition>\d+) requested=(?<requested>-?\d+) engaged=(?<engaged>-?\d+) throttle=(?<throttle>-?[\d.]+) brake=(?<brake>-?[\d.]+) clutch=(?<clutch>-?[\d.]+) handbrake=(?<handbrake>-?[\d.]+) steering=(?<steering>-?[\d.]+) speed=(?<speed>-?[\d.]+) dist=(?<dist>-?[\d.]+) longcm=(?<longcm>-?[\d.]+) gearx=(?<gearx>-?[\d.]+) geary=(?<geary>-?[\d.]+) target=(?<target>\S+) grip=(?<grip>\d+) manipulation=(?<manip>\d+) gaze=(?<gaze>\d+) camera=(?<camera>\d+) aimvalid=(?<aimvalid>\d+) aimyaw=(?<aimyaw>-?[\d.]+) aimpitch=(?<aimpitch>-?[\d.]+) wheels=(?<wheels>\d+) wheelpending=(?<wheelpending>-?\d+) wheelrecipient=(?<wheelrecipient>-?\d+) throttletarget=(?<throttletarget>-?[\d.]+) launchserial=(?<launchserial>\d+)'
    $m = [regex]::Match($line,$rx)
    if (-not $m.Success) { return $null }
    [pscustomobject]@{
        menu=[int]$m.Groups['menu'].Value
        paused=[int]$m.Groups['paused'].Value
        worlddt=[double]$m.Groups['worlddt'].Value
        awake=[int]$m.Groups['awake'].Value
        velcm=[double]$m.Groups['velcm'].Value
        ignition=[int]$m.Groups['ignition'].Value
        requested=[int]$m.Groups['requested'].Value
        engaged=[int]$m.Groups['engaged'].Value
        throttle=[double]$m.Groups['throttle'].Value
        brake=[double]$m.Groups['brake'].Value
        clutch=[double]$m.Groups['clutch'].Value
        handbrake=[double]$m.Groups['handbrake'].Value
        steering=[double]$m.Groups['steering'].Value
        speed=[double]$m.Groups['speed'].Value
        dist=[double]$m.Groups['dist'].Value
        longcm=[double]$m.Groups['longcm'].Value
        gearx=[double]$m.Groups['gearx'].Value
        geary=[double]$m.Groups['geary'].Value
        target=$m.Groups['target'].Value
        grip=[int]$m.Groups['grip'].Value
        manip=[int]$m.Groups['manip'].Value
        gaze=[int]$m.Groups['gaze'].Value
        camera=[int]$m.Groups['camera'].Value
        aimvalid=[int]$m.Groups['aimvalid'].Value
        aimyaw=[double]$m.Groups['aimyaw'].Value
        aimpitch=[double]$m.Groups['aimpitch'].Value
        wheels=[int]$m.Groups['wheels'].Value
        wheelpending=[int]$m.Groups['wheelpending'].Value
        wheelrecipient=[int]$m.Groups['wheelrecipient'].Value
        throttletarget=[double]$m.Groups['throttletarget'].Value
        launchserial=[int]$m.Groups['launchserial'].Value
        raw=$line
    }
}
function Wait-State([scriptblock]$Predicate,[int]$TimeoutMs,[string]$Description) {
    $deadline=[DateTime]::UtcNow.AddMilliseconds($TimeoutMs)
    do {
        $s=Get-State
        if ($null -ne $s -and (& $Predicate $s)) { return $s }
        Start-Sleep -Milliseconds 100
    } while([DateTime]::UtcNow -lt $deadline)
    $last=Get-State
    throw "Timed out: $Description. Last=$($last.raw)"
}
function Focus-GameWindow([IntPtr]$Handle) {
    for($i=0;$i -lt 12;$i++) {
        [PinkCabNativeInput]::ShowWindow($Handle,9) | Out-Null

        $foreground=[PinkCabNativeInput]::GetForegroundWindow()
        $currentTid=[PinkCabNativeInput]::GetCurrentThreadId()
        $targetTid=[PinkCabNativeInput]::GetWindowThreadProcessId($Handle,[IntPtr]::Zero)
        $foregroundTid=if($foreground -ne [IntPtr]::Zero){
            [PinkCabNativeInput]::GetWindowThreadProcessId($foreground,[IntPtr]::Zero)
        } else { 0 }

        $attachedTarget=$false
        $attachedForeground=$false
        try {
            if($targetTid -ne 0 -and $targetTid -ne $currentTid){
                $attachedTarget=[PinkCabNativeInput]::AttachThreadInput($currentTid,$targetTid,$true)
            }
            if($foregroundTid -ne 0 -and $foregroundTid -ne $currentTid -and $foregroundTid -ne $targetTid){
                $attachedForeground=[PinkCabNativeInput]::AttachThreadInput($currentTid,$foregroundTid,$true)
            }
            [PinkCabNativeInput]::BringWindowToTop($Handle) | Out-Null
            [PinkCabNativeInput]::SetActiveWindow($Handle) | Out-Null
            [PinkCabNativeInput]::SetFocus($Handle) | Out-Null
            [PinkCabNativeInput]::SetForegroundWindow($Handle) | Out-Null
        }
        finally {
            if($attachedForeground){ [PinkCabNativeInput]::AttachThreadInput($currentTid,$foregroundTid,$false) | Out-Null }
            if($attachedTarget){ [PinkCabNativeInput]::AttachThreadInput($currentTid,$targetTid,$false) | Out-Null }
        }

        Start-Sleep -Milliseconds 180
        if([PinkCabNativeInput]::GetForegroundWindow() -eq $Handle){ return }

        $rect=New-Object PinkCabNativeInput+RECT
        if([PinkCabNativeInput]::GetWindowRect($Handle,[ref]$rect)) {
            $x=[int](($rect.Left+$rect.Right)/2)
            $y=[int](($rect.Top+$rect.Bottom)/2)
            [PinkCabNativeInput]::SetCursorPos($x,$y) | Out-Null
            [PinkCabNativeInput]::LeftDown()
            Start-Sleep -Milliseconds 40
            [PinkCabNativeInput]::LeftUp()
        }
        Start-Sleep -Milliseconds 180
    }
    $fg=[PinkCabNativeInput]::GetForegroundWindow()
    throw "Packaged game could not acquire foreground focus. target=$Handle foreground=$fg"
}

function Capture-Window([IntPtr]$Handle,[string]$Path) {
    $rect=New-Object PinkCabNativeInput+RECT
    if (-not [PinkCabNativeInput]::GetWindowRect($Handle,[ref]$rect)) { throw "GetWindowRect failed" }
    $ww=$rect.Right-$rect.Left; $hh=$rect.Bottom-$rect.Top
    if ($ww -lt 64 -or $hh -lt 64) { throw "Invalid window rect" }
    $bmp=New-Object System.Drawing.Bitmap($ww,$hh)
    $g=[System.Drawing.Graphics]::FromImage($bmp)
    try { $g.CopyFromScreen($rect.Left,$rect.Top,0,0,$bmp.Size) ; $bmp.Save($Path,[System.Drawing.Imaging.ImageFormat]::Png) }
    finally { $g.Dispose(); $bmp.Dispose() }
}
function Probe-AxisResponse(
    [string]$Field,
    [int]$Dx,
    [int]$Dy,
    [double]$MinDelta = 0.03,
    [int]$TimeoutMs = 1500
) {
    $before=Get-State
    if($null -eq $before){ throw "No packaged telemetry before $Field probe" }
    $start=[double]$before.$Field
    [PinkCabNativeInput]::Move($Dx,$Dy)
    $deadline=[DateTime]::UtcNow.AddMilliseconds($TimeoutMs)
    do {
        Start-Sleep -Milliseconds 50
        $after=Get-State
        if($null -eq $after){ continue }
        $delta=[double]$after.$Field-$start
        if([Math]::Abs($delta) -ge $MinDelta){
            return [Math]::Sign($delta)
        }
    } while([DateTime]::UtcNow -lt $deadline)
    $last=Get-State
    throw "$Field produced no measurable packaged mouse response. Before=$($before.raw) Last=$($last.raw)"
}

function Move-GameAxis([string]$Axis,[double]$Target,[double]$OsSign) {
    for($i=0;$i -lt 16;$i++) {
        $s=Get-State
        $current = if($Axis -eq 'x'){$s.gearx}else{$s.geary}
        $err=$Target-$current
        if([Math]::Abs($err) -le 0.10){ return }
        $scale = if($Axis -eq 'x'){155.0}else{115.0}
        $delta=[int][Math]::Round(($err*$scale)/$OsSign)
        if($delta -gt 65){$delta=65}; if($delta -lt -65){$delta=-65}
        if($Axis -eq 'x'){[PinkCabNativeInput]::Move($delta,0)}else{[PinkCabNativeInput]::Move(0,$delta)}
        Start-Sleep -Milliseconds 160
    }
    $s=Get-State
    throw "Gear axis failed"
}
function Move-GearCursor([double]$X,[double]$Y,[double]$SignX,[double]$SignY) {
    Move-GameAxis 'y' 0.0 $SignY
    Move-GameAxis 'x' $X $SignX
    Move-GameAxis 'y' $Y $SignY
}
function Center-Steering {
    $initial=Get-State
    if($null -eq $initial){ throw "No packaged telemetry before steering centering" }
    if([Math]::Abs($initial.steering) -le 0.03){ return }

    $sign=Probe-AxisResponse 'steering' 120 0 0.02 1800
    for($i=0;$i -lt 20;$i++) {
        $s=Get-State
        if([Math]::Abs($s.steering) -le 0.03){ return }
        $delta=[int][Math]::Round((-1.0*$s.steering*85.0)/$sign)
        if($delta -gt 45){$delta=45}
        if($delta -lt -45){$delta=-45}
        if($delta -eq 0){ $delta=if($s.steering -gt 0){-1*$sign}else{$sign} }
        [PinkCabNativeInput]::Move($delta,0)
        Start-Sleep -Milliseconds 160
    }
    $s=Get-State
    throw "Could not center steering before launch. Last=$($s.raw)"
}

$script:GameHwnd = [IntPtr]::Zero

function Send-Wheel([int]$Delta) {
    if($script:GameHwnd -eq [IntPtr]::Zero){ throw "Packaged game HWND is not initialized for wheel input" }
    if([PinkCabNativeInput]::GetForegroundWindow() -ne $script:GameHwnd) {
        Focus-GameWindow $script:GameHwnd
    }
    [PinkCabNativeInput]::Wheel($Delta)
}

function Assert-HeldKey([int]$Vk,[string]$Name) {
    if($Vk -ne 0 -and -not [PinkCabNativeInput]::IsKeyDown($Vk)) {
        throw "Synthetic held key lost before wheel dosing: $Name"
    }
}

function Dose-To([string]$Field,[double]$Min,[double]$Max,[int]$PrimaryWheelDelta=120,[int]$HeldVk=0,[string]$HeldName='') {
    $before=Get-State
    $wheel=$PrimaryWheelDelta
    $responded=$false

    # Synthetic Windows input can report a key as physically down before the
    # packaged viewport has consumed that transition. Probe both wheel
    # directions, and if neither changes the semantic axis, re-prime the held
    # key once through a real up/down edge. This does not relax the contract:
    # the axis still has to respond to foreground OS input and reach the same
    # target interval.
    for($probe=1;$probe -le 3 -and -not $responded;$probe++) {
        Focus-GameWindow $script:GameHwnd
        Assert-HeldKey $HeldVk $HeldName

        $beforeVal=[double]$before.$Field
        Send-Wheel $PrimaryWheelDelta
        Start-Sleep -Milliseconds 220
        $after=Get-State
        $afterVal=[double]$after.$Field
        Write-Host "PACKAGED_OS_INPUT_DOSE_PROBE field=$Field probe=$probe dir=primary before=$beforeVal after=$afterVal"
        if($afterVal -gt $beforeVal + 0.001) {
            $wheel=$PrimaryWheelDelta
            $responded=$true
            break
        }

        Focus-GameWindow $script:GameHwnd
        Assert-HeldKey $HeldVk $HeldName
        Send-Wheel (-$PrimaryWheelDelta)
        Start-Sleep -Milliseconds 220
        $after=Get-State
        $afterVal=[double]$after.$Field
        Write-Host "PACKAGED_OS_INPUT_DOSE_PROBE field=$Field probe=$probe dir=opposite before=$beforeVal after=$afterVal"
        if($afterVal -gt $beforeVal + 0.001) {
            $wheel=-$PrimaryWheelDelta
            $responded=$true
            break
        }

        if($HeldVk -ne 0) {
            Write-Host "PACKAGED_OS_INPUT_REPRIME_HELD field=$Field key=$HeldName probe=$probe"
            [PinkCabNativeInput]::KeyUp($HeldVk)
            Start-Sleep -Milliseconds 120
            Focus-GameWindow $script:GameHwnd
            [PinkCabNativeInput]::KeyDown($HeldVk)
            Start-Sleep -Milliseconds 220
            Assert-HeldKey $HeldVk $HeldName
        }
        $before=Get-State
    }

    if(-not $responded) {
        $last=Get-State
        throw "Could not start dosing $Field through foreground OS input. Last=$($last.raw)"
    }

    for($i=0;$i -lt 32;$i++) {
        $state=Get-State
        $v=[double]$state.$Field
        if($v -ge $Min -and $v -le $Max){ return }
        if($v -gt $Max) { throw "$Field overshot target: $v Last=$($state.raw)" }
        Focus-GameWindow $script:GameHwnd
        Assert-HeldKey $HeldVk $HeldName
        Send-Wheel $wheel
        Start-Sleep -Milliseconds 160
    }
    $last=Get-State
    throw "Could not dose $Field. Last=$($last.raw)"
}

$proc=$null
$VK_ESC=0x1B; $VK_SPACE=0x20; $VK_Q=0x51; $VK_W=0x57; $VK_E=0x45; $VK_3=0x33; $VK_4=0x34
try {
    $launchArgs=@(
        "-log","-windowed","-ResX=1280","-ResY=720","-PinkCabGateTelemetry","-abslog=$log"
    )
    if(-not [string]::IsNullOrWhiteSpace($Map)){
        $launchArgs = @($Map) + $launchArgs
    }
    $proc=Start-Process -FilePath $Exe -ArgumentList $launchArgs -WorkingDirectory (Split-Path $Exe) -PassThru

    $deadline=[DateTime]::UtcNow.AddSeconds(45)
    do {
        Start-Sleep -Milliseconds 250
        $proc.Refresh()
        if($proc.HasExited){ throw "Packaged game exited before OS-input gate" }
    } while(($proc.MainWindowHandle -eq 0 -or $null -eq (Get-State)) -and [DateTime]::UtcNow -lt $deadline)
    if($proc.MainWindowHandle -eq 0){ throw "Packaged game window not found" }

    $script:GameHwnd = [IntPtr]$proc.MainWindowHandle
    Focus-GameWindow $script:GameHwnd
    Start-Sleep -Milliseconds 500

    Wait-State { param($s) $s.menu -eq 1 -and $s.camera -eq 1 -and $s.wheels -eq 4 } 8000 "startup menu/camera/wheels" | Out-Null

    # Fresh content-heavy packages can publish gate telemetry slightly before
    # the Slate/player input stack is ready to consume the first synthetic
    # keyboard event. Keep this an OS-input test, but settle/focus and retry
    # the startup ESC instead of treating one dropped event as product failure.
    Start-Sleep -Milliseconds 1200
    $menuClosed=$false
    for($attempt=1; $attempt -le 4 -and -not $menuClosed; ++$attempt){
        Focus-GameWindow $proc.MainWindowHandle
        Start-Sleep -Milliseconds 180
        Tap-Key $VK_ESC 220
        $deadlineEsc=[DateTime]::UtcNow.AddMilliseconds(1400)
        do {
            Start-Sleep -Milliseconds 80
            $escState=Get-State
            if($null -ne $escState -and $escState.menu -eq 0){
                $menuClosed=$true
                break
            }
        } while([DateTime]::UtcNow -lt $deadlineEsc)
        if(-not $menuClosed){
            Write-Host "PACKAGED_OS_INPUT_ESC_RETRY=$attempt"
        }
    }
    if(-not $menuClosed){
        $lastEsc=Get-State
        throw "Timed out: ESC closes startup menu after focused retries. Last=$($lastEsc.raw)"
    }

    Focus-GameWindow $proc.MainWindowHandle
    [PinkCabNativeInput]::KeyDown($VK_SPACE)
    Wait-State { param($s) $s.gaze -eq 1 -and $s.aimvalid -eq 1 } 2500 "Space enters gaze mode with ignition aim telemetry" | Out-Null

    function Calibrate-AimAxis([string]$Field,[int]$Dx,[int]$Dy) {
        # The packaged window can publish gaze telemetry a frame before the
        # foreground raw-mouse path is ready. Keep this a real OS-input check,
        # but refocus and retry the calibration sample instead of treating one
        # dropped mouse packet as a product regression.
        for($attempt=1; $attempt -le 4; ++$attempt) {
            Focus-GameWindow $script:GameHwnd
            Wait-State { param($s) $s.gaze -eq 1 -and $s.aimvalid -eq 1 } 1500 "gaze remains active during aim calibration" | Out-Null
            Start-Sleep -Milliseconds (120 * $attempt)

            $scale=$attempt
            $sampleDx=$Dx*$scale
            $sampleDy=$Dy*$scale
            $before=Get-State
            [PinkCabNativeInput]::Move($sampleDx,$sampleDy)
            Start-Sleep -Milliseconds 260
            $after=Get-State
            $delta=[double]$after.$Field-[double]$before.$Field
            $counts = if($sampleDx -ne 0) { [double]$sampleDx } else { [double]$sampleDy }
            Write-Host "PACKAGED_OS_INPUT_AIM_PROBE field=$Field attempt=$attempt delta=$delta counts=$counts"
            if([Math]::Abs($delta) -ge 0.05){
                return $delta/$counts
            }
        }

        $last=Get-State
        throw "Aim calibration produced no measurable $Field response after focused retries. Last=$($last.raw)"
    }

    $yawPerCount=Calibrate-AimAxis 'aimyaw' 24 0
    $pitchPerCount=Calibrate-AimAxis 'aimpitch' 0 24
    for($i=0;$i -lt 36;$i++) {
        $s=Get-State
        if($s.target -eq 'Ignition' -and [Math]::Abs($s.aimyaw) -le 4.0 -and [Math]::Abs($s.aimpitch) -le 4.0) { break }
        $dx=[int][Math]::Round((-1.0*$s.aimyaw)/$yawPerCount)
        $dy=[int][Math]::Round((-1.0*$s.aimpitch)/$pitchPerCount)
        if($dx -gt 45){$dx=45}; if($dx -lt -45){$dx=-45}
        if($dy -gt 45){$dy=45}; if($dy -lt -45){$dy=-45}
        if($dx -eq 0 -and [Math]::Abs($s.aimyaw) -gt 1.0){$dx=if($s.aimyaw*$yawPerCount -gt 0){-1}else{1}}
        if($dy -eq 0 -and [Math]::Abs($s.aimpitch) -gt 1.0){$dy=if($s.aimpitch*$pitchPerCount -gt 0){-1}else{1}}
        [PinkCabNativeInput]::Move($dx,$dy)
        Start-Sleep -Milliseconds 180
    }
    $aimed=Get-State
    if($aimed.target -ne 'Ignition'){
        throw "Closed-loop gaze did not select Ignition. Last=$($aimed.raw)"
    }
    [PinkCabNativeInput]::LeftDown()
    Start-Sleep -Milliseconds 350
    [PinkCabNativeInput]::LeftUp()
    Wait-State { param($s) $s.ignition -eq 1 } 2500 "LMB starts ignition through gaze-selected target" | Out-Null
    [PinkCabNativeInput]::KeyUp($VK_SPACE)
    Wait-State { param($s) $s.gaze -eq 0 } 1500 "Space exits gaze mode" | Out-Null

    [PinkCabNativeInput]::KeyDown($VK_4)
    Wait-State { param($s) $s.target -eq 'Handbrake' } 2500 "held 4 stages handbrake" | Out-Null
    [PinkCabNativeInput]::RightDown(); Start-Sleep -Milliseconds 250
    $hbGrip=Wait-State { param($s) $s.target -eq 'Handbrake' -and $s.grip -eq 1 } 4000 "RMB grips held-quick handbrake"
    [PinkCabNativeInput]::KeyUp($VK_4)
    $hbBefore=$hbGrip.handbrake
    [PinkCabNativeInput]::LeftDown(); Start-Sleep -Milliseconds 150
    [PinkCabNativeInput]::Move(0,80); Start-Sleep -Milliseconds 250
    $hbAfter=(Get-State).handbrake
    $hbDirection = if($hbAfter -lt $hbBefore){1}else{-1}
    $hbBest=$hbAfter
    $hbStagnant=0
    for($i=0;$i -lt 32 -and (Get-State).handbrake -gt 0.0001;$i++){
        [PinkCabNativeInput]::Move(0,$hbDirection*100); Start-Sleep -Milliseconds 140
        $hbNow=(Get-State).handbrake
        if($hbNow -lt ($hbBest - 0.002)){
            $hbBest=$hbNow
            $hbStagnant=0
        } else {
            $hbStagnant++
        }
        if($hbStagnant -ge 6){
            throw "Handbrake synthetic OS throw stopped making progress at $hbNow"
        }
    }
    Wait-State { param($s) $s.handbrake -le 0.0001 -and $s.manip -eq 1 } 4000 "full parking handbrake release" | Out-Null
    [PinkCabNativeInput]::LeftUp(); [PinkCabNativeInput]::RightUp(); Start-Sleep -Milliseconds 250

    [PinkCabNativeInput]::KeyDown($VK_Q)
    Wait-State { param($s) $s.clutch -ge 0.90 } 4000 "Q depresses clutch" | Out-Null
    [PinkCabNativeInput]::KeyDown($VK_3)
    Wait-State { param($s) $s.target -eq 'Gearbox' } 2500 "held 3 stages gearbox" | Out-Null
    [PinkCabNativeInput]::RightDown(); Start-Sleep -Milliseconds 180
    [PinkCabNativeInput]::LeftDown(); Start-Sleep -Milliseconds 180
    Wait-State { param($s) $s.target -eq 'Gearbox' -and $s.grip -eq 1 -and $s.manip -eq 1 } 4000 "RMB+LMB gearbox manipulation from held quick target" | Out-Null
    [PinkCabNativeInput]::KeyUp($VK_3)

    # Calibrate Y while the lever is still on the valid neutral 3/4 rail (X=1).
    # The extended H-gate intentionally blocks fore/aft motion between rails.
    $signY=Probe-AxisResponse 'geary' 0 25 0.03 1500
    Move-GameAxis 'y' 0.0 $signY
    $signX=Probe-AxisResponse 'gearx' 25 0 0.03 1500
    Move-GameAxis 'x' 1.0 $signX

    # Exact packaged proof for the relocated middle rail. This is the bug the
    # previous warm-package shortcut failed to verify: 3/4 must be present and
    # reachable at the old 5/R lateral position before we continue to 1/R.
    Move-GearCursor 1.0 1.0 $signX $signY
    Wait-State {
        param($s)
        $s.requested -eq 3 -and [Math]::Abs($s.gearx-1.0) -le 0.12 -and $s.geary -ge 0.65
    } 4000 "relocated middle rail requests third" | Out-Null
    Write-Host 'CD643_PACKAGED_GEAR3=PASS'

    Move-GearCursor 1.0 0.0 $signX $signY
    Wait-State { param($s) $s.requested -eq 0 } 3000 "third exits to neutral" | Out-Null

    Move-GearCursor 1.0 -1.0 $signX $signY
    Wait-State {
        param($s)
        $s.requested -eq 4 -and [Math]::Abs($s.gearx-1.0) -le 0.12 -and $s.geary -le -0.65
    } 4000 "relocated middle rail requests fourth" | Out-Null
    Write-Host 'CD643_PACKAGED_GEAR4=PASS'

    Move-GearCursor 1.0 0.0 $signX $signY
    Wait-State { param($s) $s.requested -eq 0 } 3000 "fourth exits to neutral" | Out-Null

    Move-GearCursor -1.0 1.0 $signX $signY
    Wait-State { param($s) $s.requested -eq 1 } 4000 "H-gate requests first" | Out-Null
    [PinkCabNativeInput]::LeftUp(); [PinkCabNativeInput]::RightUp()
    Wait-State { param($s) $s.engaged -eq 1 } 4000 "Q allows first engagement" | Out-Null
    Wait-State {
        param($s)
        $s.grip -eq 0 -and $s.manip -eq 0 -and $s.target -eq 'Gearbox'
    } 2500 "gearbox release restores steering ownership while Q keeps gearbox staged" | Out-Null

    Center-Steering
    Wait-State { param($s) [Math]::Abs($s.steering) -le 0.05 } 2500 "steering centered before forward launch" | Out-Null

    [PinkCabNativeInput]::KeyDown($VK_E)
    Wait-State { param($s) $s.throttle -le 0.01 -and $s.clutch -ge 0.90 } 2500 "fresh E does not invent throttle" | Out-Null
    Dose-To 'throttle' 0.25 0.35 120 $VK_E 'E'
    [PinkCabNativeInput]::KeyUp($VK_Q)
    Wait-State { param($s) $s.engaged -eq 1 -and $s.longcm -gt 500.0 -and $s.speed -gt 0.5 } 10000 "forward packaged movement beyond 5m" | Out-Null

    $steerBefore=(Get-State).steering
    [PinkCabNativeInput]::Move(30,0); Start-Sleep -Milliseconds 350
    $steerAfter=(Get-State).steering
    if($steerAfter -le $steerBefore){ throw "Physical Windows mouse-right did not increase semantic right steering" }
    Capture-Window $proc.MainWindowHandle $screenshot

    [PinkCabNativeInput]::KeyUp($VK_E)
    [PinkCabNativeInput]::KeyDown($VK_Q)
    [PinkCabNativeInput]::KeyDown($VK_W)
    Dose-To 'brake' 0.75 1.0 120 $VK_W 'W' $VK_W 'W'
    Wait-State { param($s) [Math]::Abs($s.speed) -lt 1.0 } 8000 "service-brake stop" | Out-Null
    [PinkCabNativeInput]::KeyUp($VK_W)

    [PinkCabNativeInput]::KeyDown($VK_3)
    Wait-State { param($s) $s.target -eq 'Gearbox' } 2500 "held 3 restages gearbox for reverse" | Out-Null
    [PinkCabNativeInput]::RightDown(); Start-Sleep -Milliseconds 160
    [PinkCabNativeInput]::LeftDown(); Start-Sleep -Milliseconds 160
    Wait-State { param($s) $s.target -eq 'Gearbox' -and $s.grip -eq 1 -and $s.manip -eq 1 } 4000 "gearbox retained for reverse manipulation" | Out-Null
    [PinkCabNativeInput]::KeyUp($VK_3)
    Move-GearCursor 2.0 -1.0 $signX $signY
    Wait-State { param($s) $s.requested -eq -1 } 4000 "H-gate requests reverse" | Out-Null
    [PinkCabNativeInput]::LeftUp(); [PinkCabNativeInput]::RightUp()
    Wait-State { param($s) $s.engaged -eq -1 } 4000 "reverse engages with clutch down" | Out-Null

    $reverseStartLongCm=(Get-State).longcm
    [PinkCabNativeInput]::KeyDown($VK_E)
    Wait-State { param($s) $s.throttle -le 0.01 } 2500 "reverse launch requires fresh E+wheel dose" | Out-Null
    Dose-To 'throttle' 0.25 0.35 120 $VK_E 'E'
    [PinkCabNativeInput]::KeyUp($VK_Q)
    Wait-State { param($s) $s.engaged -eq -1 -and $s.speed -lt -0.5 -and $s.longcm -lt ($reverseStartLongCm - 300.0) } 10000 "reverse packaged movement beyond 3m" | Out-Null

    [PinkCabNativeInput]::KeyUp($VK_E)
    [PinkCabNativeInput]::KeyDown($VK_Q)
    [PinkCabNativeInput]::KeyDown($VK_W)
    Dose-To 'brake' 0.75 1.0 120 $VK_W 'W'
    Wait-State { param($s) [Math]::Abs($s.speed) -lt 1.0 } 8000 "reverse stop" | Out-Null
    [PinkCabNativeInput]::KeyUp($VK_W)
    [PinkCabNativeInput]::KeyUp($VK_Q)

    Tap-Key $VK_ESC
    Wait-State { param($s) $s.menu -eq 1 -and $s.grip -eq 0 -and $s.manip -eq 0 } 4000 "menu cleans transient capture" | Out-Null
    Tap-Key $VK_ESC
    Wait-State { param($s) $s.menu -eq 0 } 4000 "resume after menu" | Out-Null

    for($i=1;$i -le 50;$i++){
        [PinkCabNativeInput]::KeyDown($VK_3); Start-Sleep -Milliseconds 25
        [PinkCabNativeInput]::RightDown(); Start-Sleep -Milliseconds 25
        [PinkCabNativeInput]::LeftDown(); Start-Sleep -Milliseconds 25
        [PinkCabNativeInput]::KeyUp($VK_3)
        [PinkCabNativeInput]::LeftUp(); [PinkCabNativeInput]::RightUp()
        Start-Sleep -Milliseconds 45
        if(($i % 10) -eq 0){
            Wait-State { param($x) $x.grip -eq 0 -and $x.manip -eq 0 -and $x.target -eq 'None' } 2500 "interaction cleanup cycle" | Out-Null
        }
    }

    $audioDisabled=@(Select-String -Path $log -Pattern 'Audio Device Manager not initializing due to all audio being disabled' -ErrorAction SilentlyContinue)
    if($audioDisabled.Count -gt 0){ throw "OS-input package was accidentally launched with audio disabled" }
    $audioLines=@(Select-String -Path $log -Pattern '(?i)LogAudio|AudioMixer' -ErrorAction SilentlyContinue)
    if($audioLines.Count -eq 0){ throw "No audio initialization evidence in packaged run" }

    $critical=@(Select-String -Path $log -Pattern '(?i)Fatal error:|Assertion failed|Ensure condition failed|Unhandled Exception|Log[A-Za-z0-9_]+: Error:' -ErrorAction SilentlyContinue)
    if($critical.Count -gt 0){ throw "Critical packaged OS-input log entries: $($critical.Count)" }

    $final=Get-State
    @(
        "scope=WINDOWS_SYNTHETIC_KEY_MOUSE_PLUS_WM_MOUSEWHEEL_NOT_PHYSICAL_HID",
        "wheel_injection=FOREGROUND_WINDOWS_MOUSE_WHEEL_EVENT",
        "startup_menu=PASS",
        "driver_camera=PASS",
        "wheels=4",
        "gaze_space_state=PASS",
        "ignition_space_lmb=PASS",
        "rmb_only_and_rmb_lmb_route=PASS",
        "analog_handbrake_release=PASS",
        "hgate_first=PASS",
        "prelaunch_steering_center=PASS",
        "foreground_focus=PASS",
        "mandatory_e_wheel_launch=PASS",
        "forward_movement_over_5m=PASS",
        "mouse_right_steering=PASS",
        "service_brake_stop=PASS",
        "hgate_reverse=PASS",
        "reverse_movement_over_3m=PASS",
        "menu_focus_cleanup=PASS",
        "interaction_cleanup_cycles=50",
        "audio_log_lines=$($audioLines.Count)",
        "critical=0",
        "screenshot=$screenshot",
        "final=$($final.raw)"
    ) | Set-Content $summary
    Write-Host "PACKAGED_OS_INPUT_PASS"
}
finally {
    [PinkCabNativeInput]::KeyUp($VK_3)
    [PinkCabNativeInput]::KeyUp($VK_4)
    [PinkCabNativeInput]::KeyUp($VK_Q)
    [PinkCabNativeInput]::KeyUp($VK_W)
    [PinkCabNativeInput]::KeyUp($VK_E)
    [PinkCabNativeInput]::KeyUp($VK_SPACE)
    [PinkCabNativeInput]::LeftUp()
    [PinkCabNativeInput]::RightUp()
    if($proc -and -not $proc.HasExited){
        Stop-Process -Id $proc.Id -Force -ErrorAction SilentlyContinue
        Start-Sleep -Milliseconds 300
    }
}
