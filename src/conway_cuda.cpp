/*******************************************************
 * Copyright (c) 2014, ArrayFire
 * All rights reserved.
 *
 * This file is distributed under 3-clause BSD license.
 * The complete license agreement can be obtained at:
 * http://arrayfire.com/licenses/BSD-3-Clause
 *
 * Authored by Shehzan Mohammed, ArrayFire
 ********************************************************/

/* This program uses code for CUDA-OpenGL interop demonstrated in the
 * Mandelbrot example shipped with CUDA 6.5. Those parts of code are licensed as follows
 *
 * Copyright 1993-2014 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 *  Mandelbrot sample
 *  submitted by Mark Granger, NewTek
 *
 *  CUDA 2.0 SDK - updated with double precision support
 *  CUDA 2.1 SDK - updated to demonstrate software block scheduling using atomics
 *  CUDA 2.2 SDK - updated with drawing of Julia sets by Konstantin Kolchin, NVIDIA
 *
 * The samples and license can be found at https://developer.nvidia.com/cuda-code-samples
 * which states:
 *
 *"All of the code samples are available under a permissive license that
 * allows you to freely incorporate them into your applications and create
 * derivative works for commercial, academic, or personal use."
*/

#define __BACKEND__ CUDA

#include "common.h"

#include <cuda.h>
#include <cuda_runtime.h>
#include <cuda_gl_interop.h>

#define CUDA(x) do {                                                        \
    cudaError_t err = (x);                                                  \
    if(cudaSuccess != err) {                                                \
        fprintf(stderr, "CUDA Error in %s:%d: %s \nReturned: %s.\n",        \
                __FILE__, __LINE__, #x, cudaGetErrorString(err) );          \
        exit(EXIT_FAILURE);                                                 \
    }                                                                       \
} while(0)

using namespace af;

struct cudaGraphicsResource *cuda_pbo_resource; // handles OpenGL-CUDA exchange

void cudaRegisterPBO()
{
    // Register PBO with CUDA
    CUDA(cudaGraphicsGLRegisterBuffer(&cuda_pbo_resource, gl_PBO,
                                      cudaGraphicsMapFlagsWriteDiscard));
}

void copyArrayToPBO()
{
    // Resize state for drawing larger pixels
    array X = resize(state, width, height, AF_INTERP_NEAREST);

    // Get device pointer
    float *d_X = X.device<float>();

    // Map resource. Copy data to PBO. Unmap resource.
    size_t num_bytes;
    float* d_pbo = NULL;
    CUDA(cudaGraphicsMapResources(1, &cuda_pbo_resource, 0));
    CUDA(cudaGraphicsResourceGetMappedPointer((void **)&d_pbo, &num_bytes, cuda_pbo_resource));
    CUDA(cudaMemcpy(d_pbo, d_X, num_bytes, cudaMemcpyDeviceToDevice));
    CUDA(cudaGraphicsUnmapResources(1, &cuda_pbo_resource, 0));

    // Unlock array
    // Not implemented yet
    // X.unlock();
}

void cudaCleanup()
{
    // Cleanup
    cudaGraphicsUnregisterResource(cuda_pbo_resource);
}

int main(int argc, char* argv[])
{
    try {
        af::info();

        initGLFW(width, height, 1);
        initOpenGL();
        cudaRegisterPBO();

        // Frame Counter
        int count  = 0;
        // Render Loop
        while(!glfwWindowShouldClose(window)) {
            run(count);
        }
        cudaCleanup();
        cleanup();
    } catch (af::exception& e) {
        fprintf(stderr, "%s\n", e.what());
        throw;
    }
    return 0;
}
