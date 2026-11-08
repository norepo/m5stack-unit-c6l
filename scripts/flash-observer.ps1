# PowerShell Flashing Script for M5Stack Unit C6L - Repeater + Observer (4x MQTT)
param (
    [string]$Port = "",
    [switch]$Erase,
    [switch]$SecondarySlot # If set, flashes to OTA slot 1 (0x1F0000) for dual-boot
)

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Split-Path -Parent $ScriptDir
$FwDir = Join-Path $ProjectRoot "firmware\03-repeater-observer"
$Bootloader = Join-Path $FwDir "bootloader.bin"
$Partitions = Join-Path $FwDir "partitions.bin"
$Firmware = Join-Path $FwDir "firmware.bin"

Write-Host "==========================================================" -ForegroundColor Cyan
Write-Host "  M5Stack Unit C6L - Flashing Repeater + Observer (MQTT)" -ForegroundColor Cyan
Write-Host "==========================================================" -ForegroundColor Cyan

# Find esptool
$EsptoolPath = "$env:USERPROFILE\.platformio\packages\tool-esptoolpy\esptool.py"
$PythonCmd = "python"

$PortArg = @()
if ($Port -ne "") {
    $PortArg = @("-p", $Port)
}

if ($SecondarySlot) {
    Write-Host "[Dual-Boot] Flashing Observer as secondary app to OTA slot 1 (0x1F0000)..." -ForegroundColor Yellow
    if (Test-Path $EsptoolPath) {
        & $PythonCmd $EsptoolPath --chip esp32c6 @PortArg --baud 921600 write_flash -z --flash_mode dio --flash_freq 80m 0x1F0000 $Firmware
    } else {
        Write-Error "esptool not found. Run: pip install esptool"
    }
} else {
    Write-Host "Flashing Repeater + Observer as primary firmware (0x10000)..." -ForegroundColor Green
    if (Test-Path $EsptoolPath) {
        if ($Erase) {
            Write-Host "Erasing full flash memory (clean wipe)..." -ForegroundColor Yellow
            & $PythonCmd $EsptoolPath --chip esp32c6 @PortArg --baud 921600 erase_flash
        }
        & $PythonCmd $EsptoolPath --chip esp32c6 @PortArg --baud 921600 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size 4MB 0x0 $Bootloader 0x8000 $Partitions 0x10000 $Firmware
    } else {
        $mcDev = Join-Path (Split-Path -Parent $ProjectRoot) "MeshCore-dev"
        if (Test-Path $mcDev) {
            Set-Location $mcDev
            pio run -e M5Stack_Unit_C6L_repeater_observer -t upload
        } else {
            Write-Error "esptool not found. Run: pip install esptool"
        }
    }
}
