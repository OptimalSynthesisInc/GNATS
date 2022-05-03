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

/*
 * cuda_compat.h
 *
 * cuda compatability api
 */

#ifndef __CUDA_COMPAT_H__
#define __CUDA_COMPAT_H__

#if USE_GPU
#ifndef __CUDACC__
#define __CUDACC__
#endif

#include "cuda_runtime_api.h"
#include "device_launch_parameters.h"
#include "device_functions.h"
#endif

#include <cstdlib>
#include <cstring>

/*
 * Function/variable decorators
 */
#if !USE_GPU
#define __host__
#define __device__ static
#define __global__ static
#define __constant__
#define __shared__
#endif

/*
 * Error codes
 */
#if USE_GPU
typedef cudaError_t cuda_error_t;
#else
typedef int cuda_error_t;
#endif

/*
 * Memcpy types
 */
#if USE_GPU
#define cuda_memcpy_HtoD cudaMemcpyHostToDevice
#define cuda_memcpy_DtoH cudaMemcpyDeviceToHost
#define cuda_memcpy_DtoD cudaMemcpyDeviceToDevice
typedef cudaMemcpyKind cuda_memcpy_kind_e;
#else
typedef enum _cuda_memcpy_kind_e {
  cuda_memcpy_HtoD = 0,
  cuda_memcpy_DtoH,
  cuda_memcpy_DtoD
} cuda_memcpy_kind_e;
#endif

/*
 * Streams
 */
#if USE_GPU
typedef cudaStream_t cuda_stream_t;
#else
typedef int cuda_stream_t;
#endif

/*
 * Dim3 for device launch params
 */
#if !USE_GPU
class dim3 {
public:
	dim3();
	dim3(unsigned int x);
	dim3(unsigned int x, unsigned int y);
	dim3(unsigned int x, unsigned int y, unsigned int z);
	dim3(const dim3& that);
	virtual ~dim3();

	unsigned int x;
	unsigned int y;
	unsigned int z;
};
#endif

/*
 * Block and dims
 */
#if !USE_GPU
extern dim3 blockDim;
extern dim3 gridDim;
extern dim3 blockIdx;
extern dim3 gridIdx;
#endif

/*
 * Memory allocation
 */
cuda_error_t cuda_malloc(void** const mem, const size_t size);
cuda_error_t cuda_malloc_host(void** const mem, const size_t size);
cuda_error_t cuda_calloc(void** const mem, const size_t size);
cuda_error_t cuda_calloc_host(void** const mem, const size_t size);

/*
 * Memory free
 */
cuda_error_t cuda_free(void* mem);
cuda_error_t cuda_free_host(void* mem);

/*
 * Memset
 */
cuda_error_t cuda_memset(void* mem, int val, size_t size);

/*
 * Memcpy
 */
cuda_error_t cuda_memcpy(void* dst, void* src, size_t size,
			 cuda_memcpy_kind_e kind);

cuda_error_t cuda_memcpy_async(void* dst, void* src, size_t size,
			       cuda_memcpy_kind_e kind, cuda_stream_t stream);

/*
 * Stream create
 */
cuda_error_t cuda_stream_create(cuda_stream_t* const stream);

/*
 * Stream destroy
 */
cuda_error_t cuda_stream_destroy(cuda_stream_t& stream);

/*
 * Stream sync
 */
cuda_error_t cuda_stream_synchronize(const cuda_stream_t& stream);

/*
 * Device sync
 */
cuda_error_t cuda_device_synchronize();


#endif /* __CUDA_COMPAT_H__ */
