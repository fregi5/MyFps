@echo off
cd /d G:\project\MyFps

echo ================================
echo Building MyFpsEditor...
echo ================================
echo.

call "G:\UE_5.5\Engine\Build\BatchFiles\Build.bat" MyFpsEditor Win64 Development -Project="G:\project\MyFps_UE5.4\MyFps.uproject" -WaitMutex

echo.
echo ================================
echo Build finished.
echo Exit Code: %ERRORLEVEL%
echo ================================
echo.
pause