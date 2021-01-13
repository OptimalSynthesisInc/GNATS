/*
------------------------------------- Credits -----------------------------------------------------------------
Generalized National Airspace Trajectory Simulation (GNATS) software
2017-2021 GNATS Development Team at Optimal Synthesis Inc. are:
Team Lead, Software Architecture and Algorithms: Dr. P. K. Menon
Algorithms and Prototyping: Dr. Parikshit Dutta
Java and C++ Code Development: Oliver Chen and Hari N. Iyer
Illustrative Examples in Python and MATLAB: Dr. Parikshit Dutta, Dr. Bong-Jun Yang, Hari Iyer
Illustrative Examples in SciLab and R: Hari Iyer
Acknowledgements: 
GNATS software was developed under the Arizona State University Subaward No. 18-275 under the NASA University Leadership Initiative Prime Contract No. NNX17AJ86A, with Professor Yongming Liu serving as the Principal Investigator. 
Beta Testing outside Optimal Synthesis Inc. was carried out at Arizona State University under the direction of Professor Yongming Liu, at Vanderbilt University under the direction of Professor Sankaran Mahadevan and Professor Pranav Karve, at the Southwest Research Institute under the direction of Dr. Baron Bichon and Dr. Erin DeCarlo, and at Carnegie-Mellon University under the direction of Professor Pingbo Tang.
NASA Technical points-of-contact: Dr. Anupa Bajwa, Dr. Kaushik Datta, Dr. John Cavolowsky, Dr. Kai Goebel
------------------------------------Legacy Source Code--------------------------------------------------------
Legacy Code for the GNATS software was derived from the software packages developed under the following NASA Small Business Innovation Research Projects:
1. 2004-2006 NASA Contract No. NNA05BE64C with Dr. Shon Grabbe of NASA Ames Research Center as the Technical Monitor.
2. 2008-2010 NASA Contract No. NNX08CA02C with Dr. Joseph Rios of Ames Research Center as the Technical Monitor.
3. 2010-2011 NASA Phase III Contract No. NNA10DC12C with Joseph Rios of Ames Research Center as the Technical Monitor.
3. 2016-2018 NASA Contract No. NNX16CL11C with Dr. Nash’at Ahmad of NASA Langley Research Center as the Technical Monitor.
Contributors to these SBIR projects at Optimal Synthesis Inc. were: Dr. P. K. Menon (Principal Investigator), Jason Kwan (Software Engineer), Gerald M. Diaz (Software Engineer), Dr. Monish Tandale (Research Scientist), Dr. Prasenjit Sengupta (Research Scientist), Dr. Sang-Gyun Park (Research Scientist) and Dr. Parikshit Dutta (Research Scientist).
The inspiration for the SBIR projects is derived from the FACET software developed at NASA Ames Research Center by Drs. Banavar Sridhar, Dr. Karl Bilimoria, Dr. Gano Chatterji, Dr. Shon Grabbe and Dr. Kapil Sheth.
---------------------------------------------------------------------------------------------------------------------
*/

#include "gpuTestSimulation.h"

#include "cuda_compat.h"

#include <curand_kernel.h>
//#include "device_launch_parameters.h"
#include "cuda_runtime_api.h"
#include "device_functions.h"

#include "CUDA_Aircraft_new.h"
#include "CUDA_Aircraft_new_2.h"

#include"matrixCU.h"

#include <iostream>
#include <cstdio>

typedef struct _waypoint_oliver {
	char* wpname; // = NULL;

	_waypoint_node_t* prev_node_ptr; // = NULL;
	_waypoint_node_t* next_node_ptr; // = NULL;
} waypoint_oliver_t;

using namespace std;

//float* d_latitude_deg;
float* host_latitude_deg;
vector<float> host_longitude_deg;

waypoint_oliver_t* host_waypoint_ptr;





__device__ float* dev_latitude_deg;
//__device__ CUDA_Aircraft_new dev_cudaObj;

__device__ vector<float>* dev_longitude_deg;

__device__ waypoint_oliver_t dev_waypoint;
__device__ waypoint_oliver_t* dev_waypoint_ptr;


TestKernelParameters* host_struct_var;
__device__ TestKernelParameters* dev_struct_ptr;
__device__ TestKernelParameters dev_struct_var;


__device__ Matrix *pR;






__global__ void gpuTest_global_func0(void) {
	printf("global_func0() starting\n");
}






__device__ void gpuTest_global_func1_layer2(float* inputPtr) {
	printf("gpuTest_global_func1_layer2() starting\n");
	inputPtr[0] = 99.99;
}

__global__ void gpuTest_global_func1(float* inputPtr) {
	printf("gpuTest_global_func1() starting\n");

	gpuTest_global_func1_layer2(inputPtr);
	//inputPtr[0] = 33.33;
}




__device__ void gpuTest_global_func2_layer2(waypoint_oliver_t input_param) {
	printf("gpuTest_global_func2_layer2() starting\n");

	//dev_latitude_deg[0] = 44.44;
//printf("gpuTest_global_func2_layer2() --> dev_latitude_deg[0] = %f\n", dev_latitude_deg[0]);
	//printf("gpuTest_global_func2_layer2() --> dev_waypoint_ptr->wpname = %s\n", dev_waypoint_ptr->wpname);

printf("gpuTest_global_func2_layer2() --> dev_waypoint.wpname = %s\n", dev_waypoint.wpname);
	printf("gpuTest_global_func2_layer2() --> input_param.wpname = %s\n", input_param.wpname);

	printf("gpuTest_global_func2_layer2() ending\n");
}

__global__ void gpuTest_global_func2(waypoint_oliver_t input_param) {
	printf("gpuTest_global_func2() starting\n");

	//dev_latitude_deg[0] = 44.44;
	gpuTest_global_func2_layer2(input_param);
}






__global__ void gpuTest_global_func3(CUDA_Aircraft_new cudaObj) {
	printf("gpuTest_global_func3() starting\n");

	//gpuTest_global_func3_layer2(inputPtr);
	cudaObj.latitude_deg[0] = 77.77;
}






__global__ void gpuTest_global_func4(vector<float>* inputPtr) {
	printf("gpuTest_global_func4() starting\n");

	//inputPtr->push_back(33.33);
}





__global__ void gpuTest_global_func5(TestKernelParameters inputPtr) {
printf("gpuTest_global_func5() starting\n");

	//gpuTest_global_func1_layer2(inputPtr);

	//inputPtr->latitude_deg[0] = 33.33;
	inputPtr.latitude_deg[0] = 33.33 + 100 * 25;

printf("gpuTest_global_func5() --> inputPtr.latitude_deg[0] = %f\n", inputPtr.latitude_deg[0]);
	printf("gpuTest_global_func5() ending\n");
}




__device__ void gpuTest_global_func6_layer2(CUDA_Aircraft_new_2* inputPtr) {
	//inputPtr->setValue(0, 33.33);
	//inputPtr->setValue(0, float(33.33));
}

__global__ void gpuTest_global_func6(CUDA_Aircraft_new_2* inputPtr) {
	printf("gpuTest_global_func6() starting\n");

	gpuTest_global_func6_layer2(inputPtr);
	//inputPtr->setValue(0, 33.33);
}






void test1() {
	const int na = 5, nb = 4;
	    float a[na] = { 1.2, 3.4, 5.6, 7.8, 9.0 };
	    float *_a, b[nb];

	    size_t sza = size_t(na) * sizeof(float);
	    size_t szb = size_t(nb) * sizeof(float);

	    cudaFree(0);

	    cudaMalloc((void **)&_a, sza );
	    cudaMemcpy( _a, a, sza, cudaMemcpyHostToDevice);
	    cudaMemcpy( b, _a+1, szb, cudaMemcpyDeviceToHost);

	    for(int i=0; i<nb; i++)
	        printf("test1() --> %d %f\n", i, b[i]);
}




// *********************************************************************************************

// Good

int propagate_flights_gpuTest(const float& input_t_end,
		const float& input_t_step,
		const float& input_t_step_terminal,
		const float& input_t_step_airborne) {
	printf("propagate_flights_gpuTest() starting\n");

//test1();

	host_latitude_deg = (float*)calloc(1, sizeof(float));
printf("propagate_flights_gpuTest() --> stop point 0\n");
	host_latitude_deg[0] = 11.11;
printf("propagate_flights_gpuTest() --> stop point 00\n");
	host_longitude_deg.push_back(11.11);

//d_latitude_deg = host_latitude_deg;


	host_waypoint_ptr = (waypoint_oliver_t*)calloc(1, sizeof(waypoint_oliver_t));
	host_waypoint_ptr->wpname = (char*)calloc(4, sizeof(char));
	strcpy(host_waypoint_ptr->wpname, "ABC");
	host_waypoint_ptr->wpname[3] = '\0';

printf("propagate_flights_gpuTest() --> stop point 1\n");

	// Good
	cudaMalloc(&dev_latitude_deg, 1*sizeof(float));
printf("propagate_flights_gpuTest() --> stop point 2\n");

	cudaMalloc((void**)&dev_longitude_deg, 1*sizeof(vector<float>));

printf("propagate_flights_gpuTest() --> stop point 3\n");


	cudaMalloc((void**)&dev_waypoint_ptr, 1*sizeof(waypoint_oliver_t));
printf("propagate_flights_gpuTest() --> stop point 4 --> strlen = %d\n", strlen(host_waypoint_ptr->wpname));

//cudaMalloc((void**)dev_waypoint.wpname, 3 * sizeof(char));
cudaMalloc( (void **) &dev_waypoint.wpname, 3*sizeof(char));

printf("propagate_flights_gpuTest() --> stop point 5\n");

//cudaMalloc((void**)dev_waypoint_ptr->wpname, 3 * sizeof(char));
printf("propagate_flights_gpuTest() --> stop point 6\n");

// Good
//cudaMemcpy(dev_waypoint.wpname, host_waypoint_ptr->wpname, 3 * sizeof(char), cudaMemcpyHostToDevice);

// Not working
cudaMemcpyToSymbol(dev_waypoint.wpname, &(host_waypoint_ptr->wpname), sizeof(char*), 0, cudaMemcpyHostToDevice);

printf("propagate_flights_gpuTest() --> stop point 7\n");


//	//cudaMemcpyToSymbol(dev_latitude_deg, host_latitude_deg, 1 * sizeof(float));
//	//cudaMemcpyToSymbol(&dev_latitude_deg, &host_latitude_deg, 1 * sizeof(float));
//	cudaMemcpy(dev_latitude_deg, host_latitude_deg, sizeof(float), cudaMemcpyHostToDevice);
////cudaMemcpy(dev_latitude_deg, d_latitude_deg, sizeof(float), cudaMemcpyHostToDevice);

	cudaMemcpy(dev_longitude_deg, &host_longitude_deg, sizeof(vector<float>), cudaMemcpyHostToDevice);



	printf("propagate_flights_gpuTest() --> (BEFORE) host_latitude_deg[0] = %f\n", host_latitude_deg[0]);



	// Call device
	//gpuTest_global_func0 <<<1, 1>>> ();

	// Good
	//gpuTest_global_func1 <<<1, 1>>> (dev_latitude_deg);

	// Good.  Working.
	//gpuTest_global_func4 <<<1, 1>>> (dev_longitude_deg);

	gpuTest_global_func2 <<<1, 1>>> (dev_waypoint);

	cudaThreadSynchronize();




//	//cudaMemcpyToSymbol(host_latitude_deg, dev_latitude_deg, 1 * sizeof(float));
//	//cudaMemcpyToSymbol(&host_latitude_deg, &dev_latitude_deg, 1 * sizeof(float));
//cudaMemcpy(host_latitude_deg, dev_latitude_deg, sizeof(float), cudaMemcpyDeviceToHost);
//	//cudaMemcpy(dev_latitude_deg, host_latitude_deg, sizeof(float), cudaMemcpyDeviceToHost);
////cudaMemcpy(d_latitude_deg, dev_latitude_deg, sizeof(float), cudaMemcpyDeviceToHost);

	cudaMemcpy(&host_longitude_deg, dev_longitude_deg, sizeof(vector<float>), cudaMemcpyDeviceToHost);





	printf("propagate_flights_gpuTest() --> (AFTER) host_latitude_deg[0] = %f\n", host_latitude_deg[0]);
	//printf("propagate_flights_gpuTest() --> (AFTER) dev_latitude_deg[0] = %f\n", dev_latitude_deg[0]);
//printf("propagate_flights_gpuTest() --> (AFTER) d_latitude_deg[0] = %f\n", d_latitude_deg[0]);




	cudaFree(dev_latitude_deg);
	cudaFree(dev_longitude_deg);

	free(host_latitude_deg);

	printf("propagate_flights_gpuTest() ending\n");

	return 0;
}






/*
int propagate_flights_gpuTest(const float& input_t_end,
		const float& input_t_step,
		const float& input_t_step_terminal,
		const float& input_t_step_airborne) {
	printf("propagate_flights_gpuTest() starting\n");



	host_latitude_deg = (float*)calloc(1, sizeof(float));
	host_latitude_deg[0] = 11.11;

//d_latitude_deg = host_latitude_deg;




	// Good
	//cudaMalloc(&dev_latitude_deg, 1*sizeof(float));





	//CUDA_Aircraft_new cudaObj();
	dev_cudaObj = CUDA_Aircraft_new();




//	//cudaMemcpyToSymbol(dev_latitude_deg, host_latitude_deg, 1 * sizeof(float));
//	//cudaMemcpyToSymbol(&dev_latitude_deg, &host_latitude_deg, 1 * sizeof(float));
//	cudaMemcpy(dev_latitude_deg, host_latitude_deg, sizeof(float), cudaMemcpyHostToDevice);
////cudaMemcpy(dev_latitude_deg, d_latitude_deg, sizeof(float), cudaMemcpyHostToDevice);

	cudaMemcpy(dev_cudaObj.latitude_deg, host_latitude_deg, sizeof(float), cudaMemcpyHostToDevice);

	printf("propagate_flights_gpuTest() --> (BEFORE) host_latitude_deg[0] = %f\n", host_latitude_deg[0]);



	// Call device
	//gpuTest_global_func0 <<<1, 1>>> ();

	// Good
	//gpuTest_global_func1 <<<1, 1>>> (dev_latitude_deg);

	// Not working
	//gpuTest_global_func2 <<<1, 1>>> ();

	gpuTest_global_func3 <<<1, 1>>> (dev_cudaObj);

	cudaThreadSynchronize();




//	//cudaMemcpyToSymbol(host_latitude_deg, dev_latitude_deg, 1 * sizeof(float));
//	//cudaMemcpyToSymbol(&host_latitude_deg, &dev_latitude_deg, 1 * sizeof(float));
//cudaMemcpy(host_latitude_deg, dev_latitude_deg, sizeof(float), cudaMemcpyDeviceToHost);
//	//cudaMemcpy(dev_latitude_deg, host_latitude_deg, sizeof(float), cudaMemcpyDeviceToHost);
////cudaMemcpy(d_latitude_deg, dev_latitude_deg, sizeof(float), cudaMemcpyDeviceToHost);

	cudaMemcpy(host_latitude_deg, dev_cudaObj.latitude_deg, sizeof(float), cudaMemcpyDeviceToHost);






	printf("propagate_flights_gpuTest() --> (AFTER) host_latitude_deg[0] = %f\n", host_latitude_deg[0]);
	//printf("propagate_flights_gpuTest() --> (AFTER) dev_latitude_deg[0] = %f\n", dev_latitude_deg[0]);
//printf("propagate_flights_gpuTest() --> (AFTER) d_latitude_deg[0] = %f\n", d_latitude_deg[0]);




	cudaFree(dev_latitude_deg);

	free(host_latitude_deg);

	printf("propagate_flights_gpuTest() ending\n");

	return 0;
}
*/




// Good - Struct TestKernelParameters
/*
int propagate_flights_gpuTest(const float& input_t_end,
		const float& input_t_step,
		const float& input_t_step_terminal,
		const float& input_t_step_airborne) {
printf("propagate_flights_gpuTest() starting\n");

	// Prepare host variable
	host_struct_var = (TestKernelParameters*)malloc(sizeof(TestKernelParameters));
	host_struct_var->latitude_deg = (float*)calloc(1, sizeof(float));
	host_struct_var->latitude_deg[0] = 12.12;



	// Good
	//cudaMalloc(&dev_latitude_deg, 1*sizeof(float));


// Allocate memory
cudaMalloc( (void **) &dev_struct_var.latitude_deg, 1*sizeof(float));


	//cudaMalloc(&dev_struct_ptr, 1*sizeof(TestKernelParameters));



//cudaMemcpy(dev_struct_var.latitude_deg, host_latitude_deg, sizeof(float), cudaMemcpyHostToDevice);
// Copy data from host to device
cudaMemcpy(dev_struct_var.latitude_deg, host_struct_var->latitude_deg, sizeof(float), cudaMemcpyHostToDevice);





	printf("propagate_flights_gpuTest() --> (BEFORE) host_struct_var->latitude_deg[0] = %f\n", host_struct_var->latitude_deg[0]);

	// Call device function
	gpuTest_global_func5 <<<1, 1>>> (dev_struct_var);

	//cudaThreadSynchronize();



// Copy data from device to host
cudaMemcpy(host_struct_var->latitude_deg, dev_struct_var.latitude_deg, sizeof(float), cudaMemcpyDeviceToHost);


	printf("propagate_flights_gpuTest() --> (AFTER) host_struct_var->latitude_deg[0] = %f\n", host_struct_var->latitude_deg[0]);



	cudaFree(dev_latitude_deg);
	cudaFree(dev_longitude_deg);

	cudaFree(dev_struct_ptr);

	free(host_latitude_deg);

	printf("propagate_flights_gpuTest() ending\n");

	return 0;
}
*/






const int N = 1000;

__global__ void initialize(Matrix *R) {
	int i= blockIdx.x * blockDim.x + threadIdx.x;
	if (i < N) {
		curandState state;
		curand_init(clock64(), i, 0, &state);
		//R->assignValue2(i, curand_uniform(&state));

		double d11 = 11.11;
		R->assignValue2(i, d11);
	}
}



// Using class.  Working good
/*
int propagate_flights_gpuTest(const float& input_t_end,
		const float& input_t_step,
		const float& input_t_step_terminal,
		const float& input_t_step_airborne) {
printf("propagate_flights_gpuTest() starting\n");

	Matrix R(N);

    //Matrix *pR;
    cudaMallocManaged(&pR, sizeof(Matrix));
    *pR = R;

    initialize<<<4,256>>>(pR);

    cudaDeviceSynchronize();

    pR->displayArray();

	printf("propagate_flights_gpuTest() ending\n");

	return 0;
}
*/




// Test Class.  Not working
/*
int propagate_flights_gpuTest(const float& input_t_end,
		const float& input_t_step,
		const float& input_t_step_terminal,
		const float& input_t_step_airborne) {
	printf("propagate_flights_gpuTest() starting\n");

	host_latitude_deg = (float*)calloc(1, sizeof(float));
	host_latitude_deg[0] = 11.11;


	printf("propagate_flights_gpuTest() --> stop point 1\n");


	// Good
	//cudaMalloc(&dev_latitude_deg, 1*sizeof(float));


	cudaObj_2 = CUDA_Aircraft_new_2();

	printf("propagate_flights_gpuTest() --> stop point 2\n");

	CUDA_Aircraft_new_2* cudaPtr;

	cudaMallocManaged(&cudaPtr,sizeof(CUDA_Aircraft_new_2));

	*cudaPtr = cudaObj_2;

	cudaPtr->setValue(0, host_latitude_deg[0]);





//	//cudaMemcpyToSymbol(dev_latitude_deg, host_latitude_deg, 1 * sizeof(float));
//	//cudaMemcpyToSymbol(&dev_latitude_deg, &host_latitude_deg, 1 * sizeof(float));
//	cudaMemcpy(dev_latitude_deg, host_latitude_deg, sizeof(float), cudaMemcpyHostToDevice);
////cudaMemcpy(dev_latitude_deg, d_latitude_deg, sizeof(float), cudaMemcpyHostToDevice);

	cudaMemcpy(dev_longitude_deg, host_longitude_deg, sizeof(vector<float>), cudaMemcpyHostToDevice);



	printf("propagate_flights_gpuTest() --> (BEFORE) host_latitude_deg[0] = %f\n", host_latitude_deg[0]);



	// Call device

	// Good
	//gpuTest_global_func1 <<<1, 1>>> (dev_latitude_deg);

	gpuTest_global_func6 <<<1, 1>>> (cudaPtr);

	// Not working
	//gpuTest_global_func2 <<<1, 1>>> ();

	cudaThreadSynchronize();




//	//cudaMemcpyToSymbol(host_latitude_deg, dev_latitude_deg, 1 * sizeof(float));
//	//cudaMemcpyToSymbol(&host_latitude_deg, &dev_latitude_deg, 1 * sizeof(float));
//cudaMemcpy(host_latitude_deg, dev_latitude_deg, sizeof(float), cudaMemcpyDeviceToHost);
//	//cudaMemcpy(dev_latitude_deg, host_latitude_deg, sizeof(float), cudaMemcpyDeviceToHost);
////cudaMemcpy(d_latitude_deg, dev_latitude_deg, sizeof(float), cudaMemcpyDeviceToHost);

	cudaMemcpy(host_longitude_deg, dev_longitude_deg, sizeof(vector<float>), cudaMemcpyDeviceToHost);





//	printf("propagate_flights_gpuTest() --> (AFTER) host_latitude_deg[0] = %f\n", host_latitude_deg[0]);
//	//printf("propagate_flights_gpuTest() --> (AFTER) dev_latitude_deg[0] = %f\n", dev_latitude_deg[0]);
////printf("propagate_flights_gpuTest() --> (AFTER) d_latitude_deg[0] = %f\n", d_latitude_deg[0]);

	printf("propagate_flights_gpuTest() --> (AFTER) cudaPtr->getValue(0) = %f\n", cudaPtr->getValue(0));



	cudaFree(dev_latitude_deg);
	cudaFree(dev_longitude_deg);

	free(host_latitude_deg);

	printf("propagate_flights_gpuTest() ending\n");

	return 0;
}
*/
