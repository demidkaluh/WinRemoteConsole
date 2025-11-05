@echo off
setlocal enabledelayedexpansion

echo    Remote Console Runner

set "PROJECT_ROOT=%~dp0.."
set "BIN_DIR=%PROJECT_ROOT%\bin"
set "EXE_NAME=RemoteConsole.exe"
set "EXE_PATH=%BIN_DIR%\%EXE_NAME%"

if not exist "%EXE_PATH%" (
    echo ERROR: Executable not found: %EXE_PATH%
    echo Please run build.bat first!
    echo.
    echo You can run it from scripts folder: build.bat
    echo Or from project root: scripts\build.bat
    pause
    exit /b 1
)

:menu
echo.
echo Select mode:
echo 1 - Server (console)
echo 2 - Client
echo 3 - Server (service mode)
echo 4 - Install service
echo 5 - Uninstall service
echo 6 - Custom command
echo 0 - Exit
echo.
set /p choice="Enter choice [0-6]: "

if "%choice%"=="1" (
    echo Starting server...
    "%EXE_PATH%" -s
) else if "%choice%"=="2" (
    set /p host="Enter server host [127.0.0.1]: "
    if "!host!"=="" set "host=127.0.0.1"
    set /p port="Enter server port [27015]: "
    if "!port!"=="" set "port=27015"
    echo Connecting to !host!:!port!...
    "%EXE_PATH%" -c !host! !port!
) else if "%choice%"=="3" (
    echo Starting server in service mode...
    "%EXE_PATH%" -service
) else if "%choice%"=="4" (
    echo Installing service...
    "%EXE_PATH%" -install
) else if "%choice%"=="5" (
    echo Uninstalling service...
    "%EXE_PATH%" -uninstall
) else if "%choice%"=="6" (
    set /p cmd="Enter custom command: "
    "%EXE_PATH%" !cmd!
) else if "%choice%"=="0" (
    exit /b 0
) else (
    echo Invalid choice!
)

goto menu
