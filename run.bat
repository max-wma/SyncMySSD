@echo off
echo ==============================================
echo Building SyncMySSD...
echo ==============================================

:: Configure the project
cmake -B build -S .

:: Build the project in Release mode
cmake --build build --config Release

:: Check if the build was successful
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERROR] Build failed! Please check the output above.
    pause
    exit /b %ERRORLEVEL%
)

echo.
echo ==============================================
echo Build Successful! Starting SyncMySSD...
echo ==============================================

:: Start the application and exit the console script
start "" "build\Release\SyncMySSD.exe"
