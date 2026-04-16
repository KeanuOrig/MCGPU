@echo off
REM -----------------------------------------------------------------------------
REM MC-GPU v1.3 — Windows build helper.
REM
REM Prerequisites:
REM   * Visual Studio 2019 or 2022 with "Desktop development with C++"
REM   * NVIDIA CUDA Toolkit 12.0 or later
REM   * vcpkg (https://github.com/microsoft/vcpkg), with VCPKG_ROOT set
REM     OR install zlib manually and point CMake at it.
REM
REM Usage:
REM   build.bat                 -> default Release build for sm_86 + sm_89
REM   build.bat Debug           -> Debug build
REM   build.bat Release "75;86" -> override CUDA architectures
REM -----------------------------------------------------------------------------
setlocal

set CONFIG=%1
if "%CONFIG%"=="" set CONFIG=Release

set ARCHS=%~2
if "%ARCHS%"=="" set ARCHS=86;89

if "%VCPKG_ROOT%"=="" (
    echo [WARN] VCPKG_ROOT is not set. CMake will rely on a system ZLIB.
    set TOOLCHAIN=
) else (
    REM Use the static-md triplet so zlib is linked statically and the
    REM resulting exe has no runtime DLL dependency beyond the standard
    REM Windows redistributable MSVCRT.
    set TOOLCHAIN=-DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows-static-md
)

echo.
echo === Configuring ===
cmake -S . -B build %TOOLCHAIN% -DMCGPU_CUDA_ARCHITECTURES="%ARCHS%" || goto :error

echo.
echo === Building (%CONFIG%) ===
cmake --build build --config %CONFIG% --parallel || goto :error

echo.
echo === Done ===
echo Binary: build\%CONFIG%\MC-GPU_v1.3.exe
exit /b 0

:error
echo.
echo Build failed.
exit /b 1
