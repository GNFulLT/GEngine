@echo off

set BUILD_DIR=build
set REQUIRED_FILE=GEngine_Engined.lib
set BUILD_TYPE=Release

if not exist %REQUIRED_FILE% (
    echo Error: The required lib "%REQUIRED_FILE%" does not exist.
    exit /b 1
)

if not "%1"=="" (
    set BUILD_TYPE=%1
)
echo Configuring CMake for %BUILD_TYPE%... 
cmake -B %BUILD_DIR% -DCMAKE_BUILD_TYPE=%BUILD_TYPE%

if %errorlevel% neq 0 (
    echo CMake configuration failed.
    exit /b %errorlevel%
)

echo Copying file to build directory...
copy %REQUIRED_FILE% %BUILD_DIR%

echo Building with Ninja...
cmake --build %BUILD_DIR%

if %errorlevel% neq 0 (
    echo Build failed.
    exit /b %errorlevel%
)

echo Build successful.
