@echo off
setlocal enabledelayexpansion

echo Building NoteP...
echo.

if not exist build (
    echo Creating build directory...
    mkdir build
)

cd build

echo Configuring with CMake...
cmake .. -G "Visual Studio 17 2022"

if errorlevel 1 (
    echo CMake configuration failed!
    pause
    exit /b 1
)

echo.
echo Building Release configuration...
cmake --build . --config Release

if errorlevel 1 (
    echo Build failed!
    pause
    exit /b 1
)

echo.
echo Build completed successfully!
echo Executable: build\Release\NoteP.exe
pause
