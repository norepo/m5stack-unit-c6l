param(
    [string]$Tag = "",
    [string]$Title = "",
    [string]$Notes = ""
)

$ErrorActionPreference = "Stop"

$repoRoot = (Resolve-Path "$PSScriptRoot\..").Path

if (-not $Tag) {
    $dateStamp = Get-Date -Format "yyyyMMdd"
    $Tag = "v1.18.0-$dateStamp"
}

if (-not $Title) {
    $Title = "MeshCore 1.18 — Release $Tag"
}

# Run packaging script first
& "$PSScriptRoot\package-release.ps1" -Tag $Tag

$releasesDir = Join-Path $repoRoot "releases"
$companionZip = Join-Path $releasesDir "m5stack-unit-c6l-companion-$Tag.zip"
$repeaterZip = Join-Path $releasesDir "m5stack-unit-c6l-repeater-$Tag.zip"
$observerZip = Join-Path $releasesDir "m5stack-unit-c6l-observer-$Tag.zip"

if (-not (Test-Path $companionZip) -or -not (Test-Path $repeaterZip) -or -not (Test-Path $observerZip)) {
    Write-Error "Release archives could not be found."
}

$notesFile = Join-Path $env:TEMP "release_notes_$Tag.md"
if (-not $Notes) {
    @"
## MeshCore 1.18 Firmware Release ($Tag)

Precompiled binaries and standalone flashing packages for **M5Stack Unit C6L** (ESP32-C6 + SX1262 LoRa).

### Included Bundles / Dołączone Paczki
1. **Companion Radio (BLE + Wi-Fi)**:
   - Dynamic 6-digit random PIN pairing on OLED line 2.
   - 15s display auto-off with button wake and silent connection mode.
   - Flash partition: `huge_app.csv` (3.0 MB).
2. **Repeater (Relay)**:
   - Zero-UI headless packet relay (~30 KB RAM usage).
   - Startup boot reason and free heap diagnostics on Serial (115200 baud).
   - Safe default 20 dBm TX power to eliminate USB brownout resets.
   - Hardened buffer bounds for neighbor discovery (`discover.neighbors`).
3. **Repeater + Observer (4x MQTT)**:
   - Hybrid packet relay and multi-broker MQTT observer for Wi-Fi 6 (port 1883).
   - Streams overheard RF packets and node telemetry to up to 4 concurrent MQTT broker instances (e.g. MeshMapper, CoreScope, regional analyzers, and personal custom servers).
   - Low RAM footprint (only 19.5% RAM, >263 KB free heap).
   - Full runtime CLI configuration for Wi-Fi and MQTT (`set wifi.*`, `set mqtt.*`, `set mqtt1..4.*`).

### Quick Start / Szybki Start
Download the ZIP corresponding to your role, unzip, and double-click `flash-companion.bat`, `flash-repeater.bat`, or `flash-observer.bat`. No PlatformIO or Python installation required!
"@ | Set-Content -Path $notesFile -Encoding UTF8
} else {
    $Notes | Set-Content -Path $notesFile -Encoding UTF8
}

Write-Host "Publishing GitHub Release: $Tag..." -ForegroundColor Cyan
gh release create $Tag $companionZip $repeaterZip $observerZip --title "$Title" --notes-file "$notesFile"

if (Test-Path $notesFile) { Remove-Item -Force $notesFile }

Write-Host "[SUCCESS] GitHub Release published successfully: $Tag" -ForegroundColor Green
