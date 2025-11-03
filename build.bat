@echo off
REM C++ MCP Server Build Script for Windows

echo ==================================
echo C++ MCP Server Build Script
echo ==================================

REM 빌드 디렉토리 생성
echo Creating build directory...
if not exist build mkdir build
cd build

REM vcpkg 경로 확인
if defined VCPKG_ROOT (
    echo Using vcpkg from: %VCPKG_ROOT%
    set CMAKE_ARGS=-DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake
) else (
    echo vcpkg not found. Building without vcpkg...
    set CMAKE_ARGS=
)

REM CMake 설정
echo Running CMake...
cmake .. %CMAKE_ARGS%

if %ERRORLEVEL% NEQ 0 (
    echo CMake configuration failed!
    exit /b 1
)

REM 빌드
echo Building...
cmake --build . --config Release

if %ERRORLEVEL% EQU 0 (
    echo Build successful!
    echo Executable: .\build\Release\mcp_server.exe
    echo.
    echo To test the server:
    echo   set WEATHER_API_KEY=your_api_key
    echo   .\build\Release\mcp_server.exe
) else (
    echo Build failed!
    exit /b 1
)