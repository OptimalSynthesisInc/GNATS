#
# Makefile
#
# This makefile builds a 64-bit Linux shared library

# Build GPU version by default.  This can be overridden by supplying
# USE_GPU=0 to the make command
USE_GPU=0

# Compilers to use
CXX=g++
NVCC=nvcc

# CUDA installation directory.
CUDA_INSTALL_DIR=/usr/local/cuda

# Library name
LIBRARY_NAME=libairport_layout
SHARED_LIBRARY=$(LIBRARY_NAME).so
STATIC_LIBRARY=$(LIBRARY_NAME).a

# Build version
MAJOR=1
MINOR=0

# Automatically build the source file list by searcing for all
# files in this directory (src) that have a .cpp or .cu extension
SOURCES=$(shell find . -name '*.cpp' -or -name '*.cu')
HEADERS=$(shell find . -name '*.h')

# Automatically build the object file list from the source
# file list by changing the extension from .cpp to .o
TMPOBJS=$(SOURCES:.cpp=.o)
OBJS=$(TMPOBJS:.cu=.o)

# Set compiler and linker flags
CXXFLAGS=-g -O3 -std=c++11 -fPIC -Wall -Wextra
LDFLAGS=-L../lib -L../../../lib
LIBS=-lcommon -lastar -ljson-c
INCLUDE_DIRS=-I../include -I../../../include/json-c -I../../libcommon/src -I../../libtg/src -I../../libgeomutils/src  -I../../../include/libastar
NVCC_CFLAGS=-O3 -arch=sm_35 -lineinfo -Xcompiler '-g -O3 -Wall -Wextra'

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
.SUFFIXES: .cu .cpp .o

# Implicit build rule to compile CUDA .cu files to .o files using
# nvcc if USE_GPU is set or with g++ if it is not set.
.cu.o:
ifeq ($(USE_GPU),1)
	$(NVCC) $(NVCC_CFLAGS) $(CPPFLAGS) $(INCLUDE_DIRS) -c -o $@ $<
else
	$(CXX) $(CXXFLAGS) -x c++ $(CPPFLAGS) $(INCLUDE_DIRS) -c -o $@ $<
endif

# Implicit build rule to compile .cpp files to .o files using g++
.cpp.o:
	$(CXX) $(CXXFLAGS) -x c++ $(CPPFLAGS) $(INCLUDE_DIRS) -c -o $@ $<

# Default build rule, build ngcs
all: $(SHARED_LIBRARY)

# Init the build directories
init:
	@mkdir -p ../../../include/libairport_layout

# Compile and build the library
compile: $(OBJS)
	$(CXX) $(LDFLAGS) -shared -o $(SHARED_LIBRARY) $(OBJS) $(LIBS) -fPIC

# Create symlinks in the top-level lib and include directories
install_local: init compile
	@ln -fs ../src/libairport_layout/src/$(SHARED_LIBRARY) ../../../lib/$(SHARED_LIBRARY)
	@rm -rf ../../../share/libairport_layout && ln -fs ../src/libairport_layout/share/libairport_layout ../../../share/libairport_layout
	@$(foreach header,$(notdir $(HEADERS)),ln -fs ../../src/libairport_layout/src/$(header) ../../../include/libairport_layout/$(header);)

# Build the shared library and create the top-level symlinks
$(SHARED_LIBRARY): compile install_local

# Remove all compiled objects, executables, and libraries
clean:
	rm -rf $(BUILD_DIR) *.o $(SHARED_LIBRARY) $(STATIC_LIBRARY)

distclean: clean
	rm -rf ../dist

# Copy distribution files and update top-level symlinks
# this project depends on liblektor being built and dist
dist:
	mkdir -p $(DIST_BIN_DIR) && mkdir -p $(DIST_INCLUDE_DIR) && mkdir -p $(DIST_LIB_DIR) && mkdir -p $(DIST_SHARE_DIR) && make && cp $(SHARED_LIBRARY) $(DIST_LIB_DIR)/. && cp $(HEADERS) $(DIST_INCLUDE_DIR) && cp -rf ../share/* $(DIST_SHARE_DIR)/. && find $(DIST_DIR) -name '.svn' | xargs rm -rf

archive_dist: dist
	cd ../dist && tar czf $(LIBRARY_NAME)-$(MAJOR).$(MINOR).tar.gz $(LIBRARY_NAME)-$(MAJOR).$(MINOR)