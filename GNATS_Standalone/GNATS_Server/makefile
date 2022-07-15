#
# makefile on Linux environment
#
# compiler & linker: g++


# TODO: move BASEDIR to makefile includes
BASEDIR=${PWD}

JDKDIR=/opt/java/64/jdk1.7.0_80

SRCDIR=$(BASEDIR)/src
OBJDIR=$(BASEDIR)/obj
DISTDIR=$(BASEDIR)/dist
LIBDIR=$(BASEDIR)/lib

GNATS_TRAJECTORY_MODULE_DIR=../GNATS_Trajectory_Module


LD_LIBRARY_PATH=lib

# TODO: move CC to makefile includes
CXX = g++
ARCHFLAG = 

#CFLAGS = -g ${ARCHFLAG} -I$(SRCDIR) -I$(LIBDIR)/grib_api/include/ -fPIC
# -Wall -Wextra
CXXFLAGS=-g -O3 -std=c++11 -fopenmp
LDFLAGS = -L$(LIBDIR)
LIBS = -lcommon -lastar -lhuman_error -lcontroller -lgeomutils -lrg -ltg -lfp -ltrx -lgomp -lhdf5 -lwind -lzip -lgpu
INCLUDE_DIRS=-I/usr/include -I/usr/local/include -I$(JDKDIR)/include -I$(JDKDIR)/include/linux -Isrc/com/osi/gnats/engine -I$(GNATS_TRAJECTORY_MODULE_DIR)/include -I$(GNATS_TRAJECTORY_MODULE_DIR)/include/libcommon -I$(GNATS_TRAJECTORY_MODULE_DIR)/include/libastar -I$(GNATS_TRAJECTORY_MODULE_DIR)/include/libhuman_error -I$(GNATS_TRAJECTORY_MODULE_DIR)/include/libcontroller -I$(GNATS_TRAJECTORY_MODULE_DIR)/include/libgeomutils -I$(GNATS_TRAJECTORY_MODULE_DIR)/include/libtg -I$(GNATS_TRAJECTORY_MODULE_DIR)/include/libfp -I$(GNATS_TRAJECTORY_MODULE_DIR)/include/libtrx -I$(GNATS_TRAJECTORY_MODULE_DIR)/include/libcuda_compat -I$(GNATS_TRAJECTORY_MODULE_DIR)/include/libcommon -I$(GNATS_TRAJECTORY_MODULE_DIR)/include/libpilot -I$(GNATS_TRAJECTORY_MODULE_DIR)/include/librg -I$(GNATS_TRAJECTORY_MODULE_DIR)/include/libadb -I$(GNATS_TRAJECTORY_MODULE_DIR)/include/libnats_data -I$(GNATS_TRAJECTORY_MODULE_DIR)/include/libairport_layout -I$(GNATS_TRAJECTORY_MODULE_DIR)/include/libwind -I$(GNATS_TRAJECTORY_MODULE_DIR)/include/libgpu -I$(GNATS_TRAJECTORY_MODULE_DIR)/src/libwind/third-party/hdf5install/include

SRC_FILES=$(shell find src -name '*.cpp')
TMP_OBJS=$(shell find src -name '*.o')


all: GNATS_Engine


# Link & Build Targets
GNATS_Engine: 
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) $(SRC_FILES) -shared -o $(DISTDIR)/libgnatsengine.so $(TMP_OBJS) $(LDFLAGS) $(LIBS) -fPIC


clean:
	rm ${OBJDIR}/*.o

