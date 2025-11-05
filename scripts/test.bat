@echo off
setlocal enabledelayedexpansion

echo    Remote Console Test Script

set "PROJECT_ROOT=%~dp0.."
set "BIN_DIR=%PROJECT_ROOT%\bin"
set "EXE_NAME=RemoteConsole.exe"
set "EXE_PATH=%BIN_DIR%\%EXE_NAME%"

if not exist "%EXE_PATH%" (
    echo Executable not found. Building first...
    call build.bat
    if errorlevel 1 (
        echo Build failed!
        pause
        exit /b 1
    )
)

echo.
echo Testing basic functionality...
echo.

echo 1. Starting test server...
start "RemoteConsole Test Server" cmd /k "echo Starting Server... && %EXE_PATH% -s 27015"
echo Waiting for server to start...
timeout /t 3 >nul

echo 2. Testing client connection...
echo Starting client in new window...
start "RemoteConsole Test Client" cmd /k "echo Starting Client... && %EXE_PATH% -c 127.0.0.1 27015"

echo.
echo 3. Testing if server is responding...
timeout /t 2 >nul
echo Sending test message...
echo test | "%EXE_PATH%" -c 127.0.0.1 27015 >nul 2>&1
if !errorlevel! equ 0 (
    echo SUCCESS: Server is responding!
) else (
    echo WARNING: Server may not be responding properly
)

echo.
echo Test environment is ready!
echo.
echo - Server window: RemoteConsole Test Server
echo - Client window: RemoteConsole Test Client  
echo.
echo You can now test the connection manually.
echo Press any key to close test windows...
pause >nul

echo Closing test windows...
taskkill /f /im %EXE_NAME% >nul 2>&1
taskkill /f /im cmd.exe /fi "windowtitle eq RemoteConsole Test*" >nul 2>&1

echo Test completed!
pause
