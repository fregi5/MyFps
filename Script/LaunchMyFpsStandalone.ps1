param(
    [ValidateRange(1, 4)]
    [int]$Count = 1,

    [ValidateRange(640, 3840)]
    [int]$Width = 1280,

    [ValidateRange(480, 2160)]
    [int]$Height = 720
)

$UnrealEditor = 'G:\UE_5.5\Engine\Binaries\Win64\UnrealEditor.exe'
$ProjectRoot = Split-Path -Parent $PSScriptRoot
$ProjectFile = Join-Path $ProjectRoot 'MyFps.uproject'

if (-not (Test-Path -LiteralPath $UnrealEditor)) {
    throw "UnrealEditor was not found: $UnrealEditor"
}

if (-not (Test-Path -LiteralPath $ProjectFile)) {
    throw "Project file was not found: $ProjectFile"
}

$Arguments = "`"$ProjectFile`" -game -windowed -ResX=$Width -ResY=$Height"

for ($Index = 1; $Index -le $Count; ++$Index) {
    Start-Process -FilePath $UnrealEditor -ArgumentList $Arguments

    if ($Index -lt $Count) {
        Start-Sleep -Seconds 1
    }
}
