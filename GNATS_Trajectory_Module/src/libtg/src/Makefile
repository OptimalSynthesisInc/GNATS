#
# Makefile
#
# This makefile builds a 64-bit Linux shared library

# Build GPU version by default.  This can be overridden by supplying
# USE_GPU=0 to the make command
USE_GPU=0

DOUBLE_PRECISION=1

# Compilers to use
CXX=g++
NVCC=nvcc
MKOCTFILE=mkoctfile

# CUDA installation directory.
CUDA_INSTALL_DIR=/usr/local/cuda

# Library name
LIBRARY_NAME=libtg
SHARED_LIBRARY=$(LIBRARY_NAME).so
STATIC_LIBRARY=$(LIBRARY_NAME).a
JAVA_LIBRARY=jtg.so
OCTAVE_LIBRARY=tg.oct
PYTHON_LIBRARY=_tg.so
PHP_LIBRARY=$(LIBRARY_NAME)_php.so
R_LIBRARY=$(LIBRARY_NAME)_R.so

# Build version
MAJOR=1
MINOR=0

# Automatically build the source file list by searcing for all
# files in this directory (src) that have a .cpp or .cu extension
SOURCES=$(shell find . -name '*.cpp' -or -name '*.cu' -or -name '*.cc')
HEADERS=$(shell find . -name '*.h')

# Automatically build the object file list from the source
# file list by changing the extension from .cpp to .o
TMPOBJS=$(SOURCES:.cpp=.o)
TMPOBJS2=$(TMPOBJS:.cu=.o)
OBJS=$(TMPOBJS2:.cc=.o)

# The swig wrapper module and object names
SWIG_MODULE_NAME=tg_api_wrap.cpp
SWIG_MODULE_OBJ=tg_api_wrap.o

# The directory containing SWIG interface definitions
SWIG_DIR=../wrappers
SWIG_JAVA_DIR=$(SWIG_DIR)/java
SWIG_PYTHON_DIR=$(SWIG_DIR)/python
SWIG_OCTAVE_DIR=$(SWIG_DIR)/octave
SWIG_PHP_DIR=$(SWIG_DIR)/php
SWIG_R_DIR=$(SWIG_DIR)/R

# SWIG objects for various languages
SWIG_JAVA_OBJS=$(SWIG_JAVA_DIR)/$(SWIG_MODULE_OBJ)
SWIG_PYTHON_OBJS=$(SWIG_PYTHON_DIR)/$(SWIG_MODULE_OBJ)
SWIG_OCTAVE_OBJS=$(SWIG_OCTAVE_DIR)/$(SWIG_MODULE_OBJ)
SWIG_PHP_OBJS=$(SWIG_PHP_DIR)/$(SWIG_MODULE_OBJ)
SWIG_R_OBJS=$(SWIG_R_DIR)/$(SWIG_MODULE_OBJ)

# Set compiler and linker flags
CXXFLAGS=-g -O3 -std=c++11 -fPIC -Wall -Wextra -fopenmp -pthread
LDFLAGS=-L../lib -L../../../lib -L../../libwind/third-party/hdf5install/lib -L../../libwind/third-party/grib_api/lib

LIBS=-lcurl -lxml2 -lxml++-2.6 -ljson-c -lcommon -lhuman_error -lcontroller -lcuda_compat -lpilot -lnats_data -lairport_layout -lfp -lgeomutils -llektor -lrg -ltrx -ladb -lastar -lwind -lgrib_api
INCLUDE_DIRS= \
	-I../include \
	-I../../../include \
	-I../../../include/glib-2.0 \
	-I../../../include/glibmm-2.4 \
	-I../../../include/json-c \
	-I../../../include/libairport_layout \
	-I../../../include/libastar \
	-I../../../include/libcommon \
	-I../../../include/libcontroller \
	-I../../../include/libcuda_compat \
	-I../../../include/libnats_data \
	-I../../../include/libhuman_error \
	-I../../../include/libadb \
	-I../../../include/librg \
	-I../../../include/libtrx \
	-I../../../include/libgeomutils \
	-I../../../include/libfp \
	-I../../../include/nlohmann \
	-I../../../include/libpilot \
	-I../../../include/libxml++ \
	-I../../../include/libxml2 \
	-I../../../include/libwind \
	-I../../libcommon/src \
	-I../../libcontroller/src \
	-I../../libcuda_compat/src \
	-I../../libnats_data/src \
	-I../../libhuman_error/src \
	-I../../libadb/src \
	-I../../librg/src \
	-I../../libtrx/src \
	-I../../libgeomutils/src \
	-I../../libfp/src \
	-I../../libairport_layout/src \
	-I../../libastar/src \
	-I../../libpilot/src \
	-I../../libwind/src \
	-I../../libwind/third-party/hdf5install/include \
	-I../../libwind/third-party/grib_api/include

NVCC_CFLAGS=-O3 -std=c++11 -arch=sm_35 -lineinfo -Xcompiler '-g -O3 -fPIC -Wall -Wextra -fopenmp' -Xptxas '-v'

# SWIG additional include dirs for various languages
SWIG_OCTAVE_INCLUDES=$(INCLUDE_DIRS) -I/usr/include -I../src -I../wrappers -I../wrappers/octave
SWIG_PYTHON_INCLUDES=$(INCLUDE_DIRS) -I/usr/include/python2.7 -I../src -I../wrappers -I../wrappers/python

# If ENABLE_DEBUG is set then undefine the preprocessor macro NDEBUG
# otherwise define it
ifeq ($(ENABLE_DEBUG),1)
else
CPPFLAGS += -DNDEBUG
endif

# If USE_GPU is set then append the C++ macro USE_GPU to
# the preprocessor flags so that it gets defined in the source
ifeq ($(USE_GPU),1)
CPPFLAGS += -DUSE_GPU=1
else
CPPFLAGS += -UUSE_GPU
endif

# If USE_GPU is set then append the cuda installation directory
# to the LDFLAGS and cuda libraries to LIBS
ifeq ($(USE_GPU),1)
INCLUDE_DIRS += -I$(CUDA_INSTALL_DIR)/include
LDFLAGS += -L$(CUDA_INSTALL_DIR)/lib64
LIBS += -lcudart
endif

# If USE_GPU is set then set the build directory to gpu
# else set the build directory to cpp.
ifeq ($(USE_GPU),1)
BUILD_DIR=../build/gpu
else
BUILD_DIR=../build/cpp
endif

# Distribution locations
DIST_DIR=../dist/$(LIBRARY_NAME)-$(MAJOR).$(MINOR)
DIST_LIB_DIR=$(DIST_DIR)/lib
DIST_INCLUDE_DIR=$(DIST_DIR)/include/$(LIBRARY_NAME)
DIST_SHARE_DIR=$(DIST_DIR)/share
DIST_BIN_DIR=$(DIST_DIR)/bin

# List of phony targets
.PHONY: clean init init_dist

# Specify suffixes used by implicit build rules
.SUFFIXES:
.SUFFIXES: .cu .cc .cpp .o

# Implicit build rule to compile CUDA .cu files to .o files using
# nvcc if USE_GPU is set or with g++ if it is not set.
.cu.o:
ifeq ($(USE_GPU),1)
	$(NVCC) $(NVCC_CFLAGS) -x cu $(CPPFLAGS) $(INCLUDE_DIRS) -c -o $@ $<
else
	$(CXX) $(CXXFLAGS) -x c++ $(CPPFLAGS) $(INCLUDE_DIRS) -c -o $@ $<
endif

.cc.o:
ifeq ($(USE_GPU),1)
	$(NVCC) $(NVCC_CFLAGS) --expt-relaxed-constexpr -x cu $(CPPFLAGS) $(INCLUDE_DIRS) -c -o $@ $<
else
	$(CXX) $(CXXFLAGS) -x c++ $(CPPFLAGS) $(INCLUDE_DIRS) -c -o $@ $<
endif

# Implicit build rule to compile .cpp files to .o files using g++
.cpp.o:
	$(CXX) $(CXXFLAGS) -x c++ $(CPPFLAGS) $(INCLUDE_DIRS) -c -o $@ $<


# Default build rule, build ngcs
all: $(SHARED_LIBRARY)

# Init the build directories
#init:
#	$(MAKE) -C ../../libcuda_compat/src dist USE_GPU=$(USE_GPU)
#	$(MAKE) -C ../../libtrx/src dist
#	$(MAKE) -C ../../libadb/src dist
#	$(MAKE) -C ../../liblektor/src dist
#	$(MAKE) -C ../../libnats_data/src dist
#	$(MAKE) -C ../../libfp/src dist

# Init the build directories
init:
	@mkdir -p ../../../include/libtg

# Make dependencies
deps:
	$(MAKE) -C ../../libcommon/src
	$(MAKE) -C ../../libhuman_error/src
	$(MAKE) -C ../../libcontroller/src
	$(MAKE) -C ../../libcuda_compat/src USE_GPU=$(USE_GPU)
	$(MAKE) -C ../../libpilot/src
	$(MAKE) -C ../../libfp/src
	$(MAKE) -C ../../librg/src
	$(MAKE) -C ../../libtrx/src 
	$(MAKE) -C ../../libadb/src
	$(MAKE) -C ../../liblektor/src
	$(MAKE) -C ../../libnats_data/src
	$(MAKE) -C ../../libastar/src

# Compile and build the library
compile: deps $(OBJS)
ifeq ($(USE_GPU), 1)
		$(NVCC) $(NVCC_CFLAGS) $(LDFLAGS) --ptxas-options=-v --compiler-options '-fPIC' --shared -o $(SHARED_LIBRARY) $(OBJS) $(LIBS) -lhdf5 -lz -ldl -lm -lgomp
else
		$(CXX) $(LDFLAGS) -shared -fopenmp -o $(SHARED_LIBRARY) $(OBJS) $(LIBS) -lhdf5 -lz -ldl -lm -lgomp -fPIC
endif

# Create symlinks in the top-level lib and include directories
install_local: init compile
	@ln -fs ../src/libtg/src/$(SHARED_LIBRARY) ../../../lib/$(SHARED_LIBRARY)
	@rm -rf ../../../share/libtg && ln -fs ../src/libtg/share/libtg ../../../share/libtg
	@$(foreach header,$(notdir $(HEADERS)),ln -fs ../../src/libtg/src/$(header) ../../../include/libtg/$(header);)

# Build the shared library and create the top-level symlinks
$(SHARED_LIBRARY): compile install_local

# Generate swig interfaces for various languages
swig_octave:
	swig -octave -c++ $(SWIG_OCTAVE_INCLUDES) -outdir $(SWIG_OCTAVE_DIR) -o $(SWIG_OCTAVE_DIR)/$(SWIG_MODULE_NAME) $(SWIG_DIR)/tg_api.i

swig_python:
	swig -c++ -python $(SWIG_PYTHON_INCLUDES) -outdir $(SWIG_PYTHON_DIR) -o $(SWIG_PYTHON_DIR)/$(SWIG_MODULE_NAME) $(SWIG_DIR)/tg_api.i && cp $(SWIG_PYTHON_DIR)/tg.py .


# Comment out.  Oliver Chen  2019.09.17
# Build the shared library
#$(SHARED_LIBRARY): init $(OBJS)
#	$(CXX) $(LDFLAGS) -shared -fopenmp -o $@ $(OBJS) -lhdf5 -lz -ldl -lgomp  -Wl,--whole-archive $(LIBS) -Wl,--no-whole-archive


# Build the octave library
#$(OCTAVE_LIBRARY): $(SHARED_LIBRARY) swig_octave
#	$(MKOCTFILE) $(SWIG_OCTAVE_INCLUDES) $(LDFLAGS) -o $@ $(SWIG_OCTAVE_DIR)/$(SWIG_MODULE_NAME) ../wrappers/tg_api_wrapper.cpp -lhdf5 -lz -ldl -lgomp -ltg

# Build the python library
#$(PYTHON_LIBRARY): $(SHARED_LIBRARY) swig_python
#	$(CXX) $(SWIG_PYTHON_INCLUDES) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -shared -o $@ $(SWIG_PYTHON_DIR)/$(SWIG_MODULE_NAME) ../wrappers/tg_api_wrapper.cpp -lhdf5 -lz -ldl -lgomp -ltg 

# Build the java library

# Build the R library

# Remove all compiled objects, executables, and libraries
clean:
	rm -rf $(BUILD_DIR) *.o $(SHARED_LIBRARY) $(STATIC_LIBRARY) $(OCTAVE_LIBRARY) $(PYTHON_LIBRARY) $(SWIG_DIR)/*.o $(SWIG_PYTHON_DIR)/* $(SWIG_OCTAVE_DIR)/*

distclean:
	rm -rf ../dist

# Copy distribution files and update top-level symlinks
# this project depends on liblektor being built and dist
dist:
	mkdir -p $(DIST_BIN_DIR) && mkdir -p $(DIST_INCLUDE_DIR) && mkdir -p $(DIST_LIB_DIR) && mkdir -p $(DIST_SHARE_DIR) && make && cp $(SHARED_LIBRARY) $(DIST_LIB_DIR)/. && cp $(HEADERS) $(DIST_INCLUDE_DIR) && cp -rf ../share/* $(DIST_SHARE_DIR)/. && find $(DIST_DIR) -name '.svn' | xargs rm -rf

archive_dist: dist
	cd ../dist && tar czf $(LIBRARY_NAME)-$(MAJOR).$(MINOR).tar.gz $(LIBRARY_NAME)-$(MAJOR).$(MINOR)
