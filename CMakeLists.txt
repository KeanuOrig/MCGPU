# ------------------------------------------------------------------------------
# MC-GPU v1.3 — cross-platform CMake build
#
# Supports:
#   * Linux  (gcc / clang + nvcc + system zlib)
#   * Windows (MSVC + nvcc + vcpkg zlib)
#
# Quick start:
#   Linux:    cmake -S . -B build && cmake --build build --parallel
#   Windows:  cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake
#             cmake --build build --config Release --parallel
#
# Options:
#   -DMCGPU_CUDA_ARCHITECTURES="86;89"  GPU compute capabilities to compile for.
#                                       Default covers RTX 30/40-series and A6000.
#   -DMCGPU_ENABLE_MPI=ON               Enable MPI-based multi-GPU build.
# ------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.18)

project(MCGPU
    VERSION 1.3.0
    DESCRIPTION "GPU Monte Carlo x-ray transport simulator"
    LANGUAGES CXX CUDA
)

# --- Options -----------------------------------------------------------------
set(MCGPU_CUDA_ARCHITECTURES "86;89" CACHE STRING
    "Semicolon-separated list of CUDA compute capabilities to compile for.")
option(MCGPU_ENABLE_MPI "Enable MPI-based multi-GPU build" OFF)

# --- Dependencies ------------------------------------------------------------
find_package(ZLIB REQUIRED)

if(MCGPU_ENABLE_MPI)
    find_package(MPI REQUIRED)
endif()

# --- Target ------------------------------------------------------------------
# Internal target name `mcgpu` (clean CMake identifier); the output filename
# keeps the upstream convention of `MC-GPU_v1.3`.
add_executable(mcgpu MC-GPU_v1.3.cu)

# MC-GPU_v1.3.cu #include's MC-GPU_kernel_v1.3.cu. Track it so edits to the
# kernel trigger a rebuild.
set_source_files_properties(MC-GPU_v1.3.cu PROPERTIES
    OBJECT_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/MC-GPU_kernel_v1.3.cu"
)

set_target_properties(mcgpu PROPERTIES
    OUTPUT_NAME "MC-GPU_v1.3"
    CUDA_ARCHITECTURES "${MCGPU_CUDA_ARCHITECTURES}"
    CUDA_SEPARABLE_COMPILATION OFF
)

target_include_directories(mcgpu PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/compat
)

target_compile_definitions(mcgpu PRIVATE USING_CUDA)
if(MCGPU_ENABLE_MPI)
    target_compile_definitions(mcgpu PRIVATE USING_MPI)
    target_link_libraries(mcgpu PRIVATE MPI::MPI_CXX)
endif()

target_link_libraries(mcgpu PRIVATE ZLIB::ZLIB)

# --- nvcc flags --------------------------------------------------------------
target_compile_options(mcgpu PRIVATE
    $<$<COMPILE_LANGUAGE:CUDA>:-use_fast_math>
    $<$<COMPILE_LANGUAGE:CUDA>:--ptxas-options=-v>
)

# --- MSVC hygiene ------------------------------------------------------------
if(MSVC)
    # Silence deprecation warnings for standard C string functions that MC-GPU
    # uses deliberately (sprintf, strcpy). These are upstream code style, not
    # actual vulnerabilities — the inputs are bounded.
    target_compile_definitions(mcgpu PRIVATE
        _CRT_SECURE_NO_WARNINGS
        _CRT_NONSTDC_NO_DEPRECATE
    )

    # MC-GPU allocates large on-stack arrays (voxel buffers, dose tallies).
    # Linux's default 8 MB stack fits them; Windows' 1 MB does not, producing
    # an immediate STATUS_STACK_OVERFLOW on launch. Reserve 32 MB.
    target_link_options(mcgpu PRIVATE /STACK:33554432)
endif()

# --- Install -----------------------------------------------------------------
install(TARGETS mcgpu RUNTIME DESTINATION bin)
install(DIRECTORY materials DESTINATION share/mcgpu)
