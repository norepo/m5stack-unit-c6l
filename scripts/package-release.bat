@echo off
setlocal
cd /d "%~dp0"
echo ========================================================
echo  Packaging MeshCore Release Bundles
echo ========================================================
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0package-release.ps1"
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Packaging failed with error code %ERRORLEVEL%
    pause
    exit /b %ERRORLEVEL%
)
echo.
echo [DONE] Release bundles created in releases/
pause
