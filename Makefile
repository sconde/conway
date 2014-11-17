## How to use this Makefile

## make:			Create a release  executable.
## make debug:		Create an executable in debug mode.
## make profile:	Create a version for profiling using nvvp

#include Makefile.local

# Paths
AF_PATH?=/workspace/af_package
CUDA?=/usr/local/cuda
GLFW_LIB_PATH?=/usr/local/lib

#======Choose================
# Linux compile
CXX=g++ -DLINUX
#============================

LIB=lib
ifeq ($(shell uname), Linux)
  ifeq ($(shell uname -m), x86_64)
	LIB=lib64
  endif
endif
PWD?=$(shell pwd)

# Flags
DEBUG =
CUDA_DEBUG =

DEFINES +=

INCLUDES +=\
	-I$(AF_PATH)/include \
	-I$(CUDA)/include  \
	-I/usr/include  # callgrind

LIBRARIES +=\
	-L$(GLFW_LIB_PATH) -lglfw \
	-lGL -lGLEW -lGLU \

CUDA_LIBS +=\
	-L$(AF_PATH)/lib -lafcuda \
	-L$(CUDA)/$(LIB) -lnvToolsExt \
	-L$(CUDA)/$(LIB) -lcuda -lcudart

CPU_LIBS +=\
	-L$(AF_PATH)/lib -lafcpu

#=======================
NVCC=$(CUDA)/bin/nvcc
CUDA_OPTIMISE=-O3
NVCCFLAGS += -ccbin $(CXX) $(ARCH_FLAGS) $(CUDA_DEBUG) $(CUDA_OPTIMISE)\
	-gencode=arch=compute_20,code=sm_20 \
	-gencode=arch=compute_30,code=sm_30 \
	-gencode=arch=compute_35,code=sm_35 \
	--ptxas-options=-v --machine 64 \
	-Xcompiler -fPIC

# Files which require compiling
CPU_SOURCE_FILES=\
	src/conway_cpu.cpp

CUDA_SOURCE_FILES=\
	src/conway_cuda.cpp

OUT=conway

all: $(OUT)_cpu $(OUT)_cuda

$(OUT)_cpu: $(CPU_SOURCE_FILES) Makefile
	$(CXX) $(CXXFLAGS) $(DEFINES) $(INCLUDES) $(CPU_LIBS) $(LIBRARIES) $(CPU_SOURCE_FILES) -o $@

$(OUT)_cuda: $(CUDA_SOURCE_FILES) Makefile
	$(NVCC) $(NVCCFLAGS) $(DEFINES) $(INCLUDES) $(CUDA_LIBS) $(LIBRARIES) $(CUDA_SOURCE_FILES) -o $@

debug: set_debug $(OUT)

profile: set_profile $(OUT)

set_profile:
	$(eval OPTIMISE = -pg -O3)
	$(eval CUDA_OPTIMISE = -pg -lineinfo -O3)

set_debug:
	$(eval DEBUG = -g -pg)
	$(eval CUDA_DEBUG = -G)
	$(eval OPTIMISE = -O0)
	$(eval CUDA_OPTIMISE = -pg -O0)

.PHONY: clean
clean:
	@echo "Cleaning"
	rm $(OUT)_cpu
	rm $(OUT)_cuda
	@echo done
