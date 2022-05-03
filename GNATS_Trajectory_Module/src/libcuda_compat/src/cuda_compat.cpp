/*
------------------------------------- Credits -----------------------------------------------------------------
Generalized National Airspace Trajectory Simulation (GNATS) software
2017-2021 GNATS Development Team at Optimal Synthesis Inc. are:
Team Lead, Software Architecture and Algorithms: Dr. P. K. Menon
Algorithms and Prototyping: Dr. Parikshit Dutta
Java and C++ Code Development: Oliver Chen and Hari N. Iyer
Illustrative Examples in Python and MATLAB: Dr. Parikshit Dutta, Dr. Bong-Jun Yang, Hari Iyer
Illustrative Examples in SciLab and R: Hari Iyer
Vedik Jayaraj (Summer Intern) helped digitize 39 US airports together with the Arrival-Departure procedures and helped in beta testing of GNATS.
Acknowledgements: 
GNATS software was developed under the Arizona State University Subaward No. 18-275 under the NASA University Leadership Initiative Prime Contract No. NNX17AJ86A, with Professor Yongming Liu serving as the Principal Investigator. 
Beta Testing outside Optimal Synthesis Inc. was carried out at Arizona State University under the direction of Professor Yongming Liu, at Vanderbilt University under the direction of Professor Sankaran Mahadevan and Professor Pranav Karve, at the Southwest Research Institute under the direction of Dr. Baron Bichon and Dr. Erin DeCarlo, and at Carnegie-Mellon University under the direction of Professor Pingbo Tang.
NASA Technical points-of-contact: Dr. Anupa Bajwa, Dr. Kaushik Datta, Dr. John Cavolowsky, Dr. Kai Goebel
------------------------------------Legacy Source Code--------------------------------------------------------
Legacy Code for the GNATS software was derived from the software packages developed under the following NASA Small Business Innovation Research Projects:
1. 2004-2006 NASA Contract No. NNA05BE64C with Dr. Shon Grabbe of NASA Ames Research Center as the Technical Monitor.
2. 2008-2010 NASA Contract No. NNX08CA02C with Dr. Joseph Rios of Ames Research Center as the Technical Monitor.
3. 2010-2011 NASA Phase III Contract No. NNA10DC12C with Dr. Joseph Rios of Ames Research Center as the Technical Monitor.
4. 2016-2018 NASA Contract No. NNX16CL11C with Dr. Nash’at Ahmad of NASA Langley Research Center as the Technical Monitor.

Contributors to these SBIR projects at Optimal Synthesis Inc. were: Dr. P. K. Menon (Principal Investigator), Jason Kwan (Software Engineer), Gerald M. Diaz (Software Engineer), Dr. Monish Tandale (Research Scientist), Dr. Prasenjit Sengupta (Research Scientist), Dr. Sang-Gyun Park (Research Scientist) and Dr. Parikshit Dutta (Research Scientist).
The inspiration for the SBIR projects is derived from the FACET software developed at NASA Ames Research Center by Dr. Banavar Sridhar, Dr. Karl Bilimoria, Dr. Gano Chatterji, Dr. Shon Grabbe and Dr. Kapil Sheth.

Dr. Victor H. L. Cheng of Optimal Synthesis Inc. provided the digitized data for 40 major US Airports
---------------------------------------------------------------------------------------------------------------------
*/



#include "cuda_compat.h"

#include <set>
#include <cstdlib>
#include <cstring>

using std::set;


#if !USE_GPU
static set<cuda_stream_t*> streams;
static int stream_counter = 0;
#endif

#if !USE_GPU
dim3::dim3() : x(0), y(0), z(0) {}
dim3::dim3(unsigned int x) : x(x), y(0), z(0) {}
dim3::dim3(unsigned int x, unsigned int y) : x(x), y(y), z(0) {}
dim3::dim3(unsigned int x, unsigned int y, unsigned int z) : x(x), y(y), z(z) {}
dim3::dim3(const dim3& that) : x(that.x), y(that.y), z(that.z) {}
dim3::~dim3() {}
#endif

cuda_error_t cuda_malloc(void** const mem, const size_t size) {
#if USE_GPU
  return cudaMalloc(mem, size);
#else
  cuda_error_t err = 0;
  *mem = malloc(size);
  if(!(*mem)) {
    err = -1;
  }
  return err;
#endif
}

cuda_error_t cuda_malloc_host(void** const mem, const size_t size) {
#if USE_GPU
  return cudaMallocHost(mem, size);
#else
  cuda_error_t err = 0;
  *mem = malloc(size);
  if(!(*mem)) {
    err = -1;
  }
  return err;
#endif
}

cuda_error_t cuda_calloc(void** const mem, const size_t size) {
#if USE_GPU
  cuda_error_t err;
  err = cudaMalloc(mem, size);
  if(err) return err;
  return cudaMemset(mem, 0, size);
#else
  *mem = malloc(size);
  if(!(*mem)) {
    return -1;
  }
  memset(*mem, 0, size);
  return 0;
#endif
}

cuda_error_t cuda_calloc_host(void** const mem, const size_t size) {
#if USE_GPU
  cuda_error_t err;
  err = cudaMallocHost(mem, size);
  if(err) return err;
  memset(*mem, 0, size);
  return err;
#else
  *mem = malloc(size);
  if(!(*mem)) {
    return -1;
  }
  memset(*mem, 0, size);
  return 0;
#endif
}

cuda_error_t cuda_free(void* mem) {
#if USE_GPU
  return cudaFree(mem);
#else
  cuda_error_t err = 0;
  free(mem);
  mem = NULL;
  return err;
#endif
}

cuda_error_t cuda_free_host(void* mem) {
#if USE_GPU
  return cudaFreeHost(mem);
#else
  cuda_error_t err = 0;
  if (mem != NULL)
	  free(mem);
  mem = NULL;
  return err;
#endif
}

cuda_error_t cuda_memset(void* mem, int val, size_t size) {
#if USE_GPU
	return cudaMemset(mem, val, size);
#else
	cuda_error_t err = 0;
	memset(mem, val, size);
	return err;
#endif
}

cuda_error_t cuda_memcpy(void* dst, void* src, size_t size,
			 cuda_memcpy_kind_e kind) {
#if USE_GPU
  return cudaMemcpy(dst, src, size, kind);
#else
  (void)kind;
  if(!dst) return -1;
  if(!src) return -1;
  if(src==dst) return 0;
  memcpy(dst, src, size);
  return 0;
#endif
}

cuda_error_t cuda_memcpy_async(void* dst, void* src, size_t size,
			       cuda_memcpy_kind_e kind, cuda_stream_t stream) {
#if USE_GPU
  return cudaMemcpyAsync(dst, src, size, kind, stream);
#else
  (void)kind;
  (void)stream;
  if(!dst) return -1;
  if(!src) return -1;
  if(src==dst) return 0;
  memcpy(dst, src, size);

  return 0;
#endif
}

cuda_error_t cuda_stream_create(cuda_stream_t* const stream) {
#if USE_GPU
  return cudaStreamCreate(stream);
#else
  if(!stream) return -1;
  *stream = stream_counter++;
  streams.insert(stream);
  return 0;
#endif
}

cuda_error_t cuda_stream_destroy(cuda_stream_t& stream) {
#if USE_GPU
  return cudaStreamDestroy(stream);
#else
  streams.erase(&stream);
  stream = -1;
  return 0;
#endif
}

cuda_error_t cuda_stream_synchronize(const cuda_stream_t& stream) {
#if USE_GPU
  return cudaStreamSynchronize(stream);
#else
  (void)stream;
  return 0;
#endif
}

cuda_error_t cuda_device_synchronize() {
#if USE_GPU
	return cudaDeviceSynchronize();
#else
	return 0;
#endif
}
