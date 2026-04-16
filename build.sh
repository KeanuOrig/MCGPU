#!/usr/bin/env bash
# -----------------------------------------------------------------------------
# MC-GPU v1.3 — Linux build helper.
#
# Prerequisites:
#   * gcc or clang
#   * NVIDIA CUDA Toolkit 12.0 or later
#   * zlib development headers (apt: zlib1g-dev, dnf: zlib-devel)
#
# Usage:
#   ./build.sh                 -> default Release build for sm_86 + sm_89
#   ./build.sh Debug           -> Debug build
#   ./build.sh Release "75;86" -> override CUDA architectures
# -----------------------------------------------------------------------------
set -euo pipefail

CONFIG="${1:-Release}"
ARCHS="${2:-86;89}"

echo
echo "=== Configuring ==="
cmake -S . -B build \
    -DCMAKE_BUILD_TYPE="${CONFIG}" \
    -DMCGPU_CUDA_ARCHITECTURES="${ARCHS}"

echo
echo "=== Building (${CONFIG}) ==="
cmake --build build --config "${CONFIG}" --parallel

echo
echo "=== Done ==="
echo "Binary: build/MC-GPU_v1.3"
