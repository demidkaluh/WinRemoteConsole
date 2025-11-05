@echo off
setlocal

echo    Remote Console Deploy Script

set "PROJECT_ROOT=%~dp0.."
set "BIN_DIR=%PROJECT_ROOT%\bin"
set "EXE_NAME=RemoteConsole.exe"
set "EXE_PATH=%BIN_DIR%\%EXE_NAME%"

if not exist "%EXE_PATH%" (
    echo Executable not found. Building first...
    call build.bat
    if errorlevel 1 (
        echo Build failed! Cannot deploy.
        pause
        exit /b 1
    )
)

set "DEPLOY_DIR=%PROJECT_ROOT%\deploy"
if not exist "%DEPLOY_DIR%" mkdir "%DEPLOY_DIR%"

copy "%EXE_PATH%" "%DEPLOY_DIR%\%EXE_NAME%" >nul
copy "%PROJECT_ROOT%\README.md" "%DEPLOY_DIR%\" >nul 2>&1

echo Creating deploy package...
echo @echo off > "%DEPLOY_DIR%\start_server.bat"
echo echo Starting Remote Console Server... >> "%DEPLOY_DIR%\start_server.bat"
echo %EXE_NAME% -s >> "%DEPLOY_DIR%\start_server.bat"
echo pause >> "%DEPLOY_DIR%\start_server.bat"

echo @echo off > "%DEPLOY_DIR%\start_client.bat"
echo echo Starting Remote Console Client... >> "%DEPLOY_DIR%\start_client.bat"
echo %EXE_NAME% -c 127.0.0.1 27015 >> "%DEPLOY_DIR%\start_client.bat"
echo pause >> "%DEPLOY_DIR%\start_client.bat"

echo @echo off > "%DEPLOY_DIR%\install_service.bat"
echo echo Installing as Windows Service... >> "%DEPLOY_DIR%\install_service.bat"
echo %EXE_NAME% -install >> "%DEPLOY_DIR%\install_service.bat"
echo pause >> "%DEPLOY_DIR%\install_service.bat"

echo Deploy files created in: %DEPLOY_DIR%
echo.
echo Files included:
dir "%DEPLOY_DIR%" /b

echo.
echo Deployment completed!
pause
