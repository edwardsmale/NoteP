@echo off
setlocal enabledelayexpansion

echo Creating NoteP shortcut...

REM Check if the executable exists
if not exist "build\Release\NoteP.exe" (
    echo.
    echo ERROR: NoteP.exe not found!
    echo Please run build.bat first to build the application.
    pause
    exit /b 1
)

REM Use PowerShell to create the shortcut
powershell -Command ^
    "$WshShell = New-Object -ComObject WScript.Shell; ^
    $Shortcut = $WshShell.CreateShortcut('%cd%\NoteP.lnk'); ^
    $Shortcut.TargetPath = '%cd%\build\Release\NoteP.exe'; ^
    $Shortcut.WorkingDirectory = '%cd%'; ^
    $Shortcut.Description = 'NoteP - A Simple Text Editor'; ^
    $Shortcut.IconLocation = '%cd%\build\Release\NoteP.exe'; ^
    $Shortcut.Save()"

if errorlevel 1 (
    echo Failed to create shortcut!
    pause
    exit /b 1
)

echo.
echo Shortcut created: NoteP.lnk
echo You can now launch NoteP from the desktop or the Notep folder.
pause
