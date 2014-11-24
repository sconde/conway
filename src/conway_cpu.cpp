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

#define __BACKEND__ CPU
#include "common.h"

using namespace af;

void copyArrayToPBO()
{
    // Get device pointer
    array X;
    if(game_w != buff_w || game_h != buff_h) {
        X = resize(state, buff_w, buff_h, AF_INTERP_NEAREST);
    } else {
        X = state;
    }
    float *d_X = X.device<float>();

    // Copy data to PBO
    glBufferData(GL_PIXEL_UNPACK_BUFFER_ARB, buff_w * buff_h * sizeof(float), d_X, GL_STREAM_COPY);

    // Unlock array
    // Not implemented yet
    // state.unlock();
}

int main(int argc, char* argv[])
{
    try {
        af::info();

        initGLFW(disp_w, disp_h, 1);
        initOpenGL();

        // Frame Counter
        int count  = 0;
        // Render Loop
        while(!glfwWindowShouldClose(window)) {
            run(count);
        }
        cleanup();
    } catch (af::exception& e) {
        fprintf(stderr, "%s\n", e.what());
        throw;
    }
    return 0;
}
