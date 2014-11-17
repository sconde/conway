## Conway's Game of Life using ArrayFire

This is a simple implementation of Conway's Game of Life using ArrayFire.

The code demonstrates the versatility and ease of using ArrayFire. It also shows simple ways to interop between ArrayFire and OpenGL on the different backends.

## Requirements
Other than ArrayFire and it's dependencies, the code depends on [GLEW](http://glew.sourceforge.net/) and [GLFW](http://www.glfw.org/) (>=3). Currently I have provided a simple makefile which requires modification of the various paths for these dependencies.

## Build and Run
The application currently works for the CPU and CUDA backends. OpenCL will be coming shortly.
Run `make` to build for both backends or `make conway_cpu` for CPU only and `make conway_cuda` for CUDA only.

## Code
This example is really designed to show the ease and portability of ArrayFire. Most of the code is contained in src/common.h. This file includes all code that is common to all backends. It is worth noting that almost all the ArrayFire code is in this file. This truly is a testament to *Write Once. Run Anywhere*.

The conway_cpu.cpp and conway_cuda.cpp files have code that is platform dependent and include the main() function.

I have heavily commented the code which will make it easy to understand. Feel free to ask any question.

## License
This code is distributed under ArrayFire's BSD 3-Clause License which can be found [here](http://arrayfire.com/licenses/BSD-3-Clause).

Parts of the code have been distributed are copyright of NVIDIA as a part of their CUDA 6.5 Toolkit which is under the NVIDIA EULA which permits free incorporation into personal, academic or commercial applications.
