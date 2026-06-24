@echo off
setlocal

set "UNREAL_EDITOR=G:\UE_5.5\Engine\Binaries\Win64\UnrealEditor.exe"
set "PROJECT_FILE=%~dp0..\MyFps.uproject"
set "INSTANCE_COUNT=%~1"

if "%INSTANCE_COUNT%"=="" set "INSTANCE_COUNT=1"

if not exist "%UNREAL_EDITOR%" (
    echo UnrealEditor was not found:
    echo %UNREAL_EDITOR%
    pause
    exit /b 1
)

if not exist "%PROJECT_FILE%" (
    echo Project file was not found:
    echo %PROJECT_FILE%
    pause
    exit /b 1
)

for /l %%I in (1,1,%INSTANCE_COUNT%) do (
    start "MyFps Standalone %%I" "%UNREAL_EDITOR%" "%PROJECT_FILE%" -game -windowed -ResX=1280 -ResY=720
    timeout /t 1 /nobreak >nul
)

endlocal
