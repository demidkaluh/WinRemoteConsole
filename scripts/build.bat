@echo off
setlocal enabledelayedexpansion

set "PROJECT_ROOT=%~dp0.."
set "BUILD_DIR=%PROJECT_ROOT%\build"
set "BIN_DIR=%PROJECT_ROOT%\bin"
set "CONFIG=Release"
set "EXE_NAME=RemoteConsole.exe"

:check_args
if "%1"=="" goto no_args
if "%1"=="debug" set "CONFIG=Debug"
if "%1"=="release" set "CONFIG=Release"
if "%1"=="clean" goto clean
goto build

:no_args
echo Usage: build.bat [debug^|release^|clean]
echo Default: release
goto build

:clean
echo Cleaning build directories...
if exist "%BUILD_DIR%" rmdir /s /q "%BUILD_DIR%"
if exist "%BIN_DIR%" rmdir /s /q "%BIN_DIR%"
echo Clean completed!
goto end

:build
echo Building in %CONFIG% mode...

where cmake >nul 2>nul
if %errorlevel% neq 0 (
    echo ERROR: CMake not found in PATH!
    echo Please install CMake or add it to system PATH
    exit /b 1
)

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
if not exist "%BIN_DIR%" mkdir "%BIN_DIR%"

cd "%BUILD_DIR%"

echo Configuring project with CMake...
cmake "%PROJECT_ROOT%" -G "Visual Studio 17 2022" -A x64

if %errorlevel% neq 0 (
    echo ERROR: CMake configuration failed!
    cd "%PROJECT_ROOT%"
    exit /b 1
)

echo Building project...
cmake --build . --config %CONFIG%

if %errorlevel% neq 0 (
    echo ERROR: Build failed!
    cd "%PROJECT_ROOT%"
    exit /b 1
)

if exist "%BUILD_DIR%\bin\%CONFIG%\%EXE_NAME%" (
    copy "%BUILD_DIR%\bin\%CONFIG%\%EXE_NAME%" "%BIN_DIR%\%EXE_NAME%" >nul
    echo Executable copied to: %BIN_DIR%\%EXE_NAME%
)

cd "%PROJECT_ROOT%"

echo    Build completed successfully!
echo Output: %BIN_DIR%\%EXE_NAME%
echo Config: %CONFIG%

:end
pause
