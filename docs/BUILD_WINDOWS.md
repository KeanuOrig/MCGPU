# Building MC-GPU v1.3 on Windows

This fork adds native Windows support via CMake. No Docker, WSL, or MSYS
required — MC-GPU compiles directly with Microsoft Visual C++ and the CUDA
Toolkit.

## Requirements

| Component | Recommended version | Notes |
|---|---|---|
| Windows | 10 / 11 (x64) | |
| Visual Studio | 2019 or 2022 | Install "Desktop development with C++" workload. |
| NVIDIA GPU driver | ≥ 550 (Windows) | Must match the CUDA Toolkit version. |
| NVIDIA CUDA Toolkit | 12.0 – 12.6 | Installs `nvcc` and integrates with MSVC. |
| CMake | ≥ 3.18 | Ships with recent Visual Studio, or install separately. |
| zlib | any | Easiest via [vcpkg](https://github.com/microsoft/vcpkg). |

### One-time setup

1. Install **Visual Studio** with the C++ workload.
2. Install the **CUDA Toolkit** (reboot afterwards).
3. Install **vcpkg** and set `VCPKG_ROOT`:
   ```cmd
   git clone https://github.com/microsoft/vcpkg C:\vcpkg
   C:\vcpkg\bootstrap-vcpkg.bat
   setx VCPKG_ROOT C:\vcpkg
   ```

## Build

From a `x64 Native Tools Command Prompt for VS`:

```cmd
build.bat
```

This produces `build\Release\MC-GPU_v1.3.exe`.

### Options

```cmd
build.bat Debug                  :: Debug configuration
build.bat Release "75;86;89"     :: custom CUDA architectures (Turing + Ampere + Ada)
```

### Architectures

The default is `sm_86;sm_89`, which covers:

- RTX A6000, A4000, A5000, A2000 (Ampere)
- RTX 3050 / 3060 / 3070 / 3080 / 3090 (Ampere)
- RTX 4060 / 4070 / 4080 / 4090 (Ada Lovelace)

Use `75` for RTX 20-series / T4, `80` for A100, `90` for H100.

## Run

```cmd
build\Release\MC-GPU_v1.3.exe MC-GPU_v1.3_Zubal.in
```

The input file, voxel phantom, spectrum, and material files must be reachable
from the current working directory (same conventions as the Linux build).

## Troubleshooting

**`nvcc fatal: Cannot find compiler 'cl.exe'`**
  You are not in a Visual Studio developer prompt. Use the
  `x64 Native Tools Command Prompt` start-menu shortcut, or run
  `"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"`.

**`Could not find ZLIB`**
  Either install zlib via vcpkg and set `VCPKG_ROOT`, or pass
  `-DZLIB_ROOT=C:\path\to\zlib` to CMake.

**`zlib1.dll not found` at runtime**
  `build.bat` uses the `x64-windows-static-md` triplet which links zlib
  statically, so no DLL should be needed. If you configured CMake manually
  without this triplet, either re-run `build.bat` or copy the dynamic
  `zlib1.dll` next to the exe.

**`unsupported Microsoft Visual Studio version`**
  Newer MSVC versions sometimes ship before CUDA supports them. Either install
  an older MSVC toolset (VS Installer → Individual components), or pass
  `-T v143,cuda=12.x` to CMake.
