/*
 * Minimal replacement for the CUDA Samples "helper_cuda.h" header.
 *
 * MC-GPU v1.3 depends on symbols from the NVIDIA CUDA Samples
 * (helper_cuda.h / helper_functions.h), which are no longer distributed
 * with the CUDA Toolkit since CUDA 11. This header provides drop-in
 * replacements for the four symbols MC-GPU actually uses:
 *
 *   - checkCudaErrors(call)         : abort on non-success return.
 *   - getLastCudaError(msg)         : abort if cudaGetLastError() set.
 *   - _ConvertSMVer2Cores(M, m)     : cores per SM for a compute cap.
 *   - gpuGetMaxGflopsDeviceId()     : device id with most est. gflops.
 *
 * Drop this file on the include path in place of the CUDA Samples header.
 */
#ifndef MCGPU_COMPAT_HELPER_CUDA_H
#define MCGPU_COMPAT_HELPER_CUDA_H

#include <cuda_runtime.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef checkCudaErrors
#define checkCudaErrors(call)                                                  \
    do {                                                                       \
        cudaError_t _mcgpu_err = (call);                                       \
        if (_mcgpu_err != cudaSuccess) {                                       \
            fprintf(stderr,                                                    \
                    "CUDA error at %s:%d code=%d(%s) \"%s\"\n",                \
                    __FILE__, __LINE__, (int)_mcgpu_err,                       \
                    cudaGetErrorString(_mcgpu_err), #call);                    \
            exit(EXIT_FAILURE);                                                \
        }                                                                      \
    } while (0)
#endif

static inline void _mcgpu_getLastCudaError(const char* msg,
                                           const char* file, int line) {
    cudaError_t err = cudaGetLastError();
    if (err != cudaSuccess) {
        fprintf(stderr,
                "CUDA error at %s:%d : getLastCudaError() : %s : %s\n",
                file, line, msg, cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }
}

#ifndef getLastCudaError
#define getLastCudaError(msg) _mcgpu_getLastCudaError((msg), __FILE__, __LINE__)
#endif

/* Cores per SM for each compute capability. Source: NVIDIA CUDA Samples
 * helper_cuda.h (common convention across driver generations). The table
 * is deliberately generous so newer architectures fall back to a
 * reasonable default instead of failing.
 */
static inline int _ConvertSMVer2Cores(int major, int minor) {
    typedef struct {
        int sm;     /* 0xMm as in compute capability */
        int cores;  /* CUDA cores per SM */
    } sm_to_cores_t;

    static const sm_to_cores_t table[] = {
        {0x30, 192}, {0x32, 192}, {0x35, 192}, {0x37, 192},  /* Kepler */
        {0x50, 128}, {0x52, 128}, {0x53, 128},               /* Maxwell */
        {0x60,  64}, {0x61, 128}, {0x62, 128},               /* Pascal */
        {0x70,  64}, {0x72,  64}, {0x75,  64},               /* Volta / Turing */
        {0x80,  64}, {0x86, 128}, {0x87, 128}, {0x89, 128},  /* Ampere / Ada */
        {0x90, 128},                                         /* Hopper */
        {-1,   -1}
    };

    int key = (major << 4) + minor;
    for (int i = 0; table[i].sm != -1; ++i) {
        if (table[i].sm == key) return table[i].cores;
    }
    /* Unknown architecture — fall back to the most common recent value. */
    return 128;
}

/* Return the device id with the highest estimated raw throughput
 * (cores * SM count * clock rate). Falls back to device 0 if nothing
 * else is available. Matches the CUDA Samples helper semantics.
 */
static inline int gpuGetMaxGflopsDeviceId(void) {
    int device_count = 0;
    if (cudaGetDeviceCount(&device_count) != cudaSuccess || device_count < 1) {
        return 0;
    }

    int best_device = 0;
    unsigned long long best_score = 0;

    for (int i = 0; i < device_count; ++i) {
        cudaDeviceProp prop;
        if (cudaGetDeviceProperties(&prop, i) != cudaSuccess) continue;
        if (prop.computeMode == cudaComputeModeProhibited) continue;

        unsigned long long cores_per_sm =
            (unsigned long long)_ConvertSMVer2Cores(prop.major, prop.minor);
        unsigned long long score =
            cores_per_sm
            * (unsigned long long)prop.multiProcessorCount
            * (unsigned long long)prop.clockRate;

        if (score > best_score) {
            best_score = score;
            best_device = i;
        }
    }
    return best_device;
}

#endif /* MCGPU_COMPAT_HELPER_CUDA_H */
