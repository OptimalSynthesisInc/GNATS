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

#include <iostream>
#include <cstdio>
#include"matrixCU.h"

        Matrix::Matrix(){
            height = 1;
            width = 1;
            sizeArray = height*width;
            cudaError_t err = cudaMallocManaged(&array,sizeArray*sizeof(array[0]));
            if (err != cudaSuccess)
            {
                //cout << "Memory allocation failed"<<endl;
                printf("Memory allocation failed");
            }
        }

        Matrix::Matrix(size_t h){
            height = h;
            width = 1;
            sizeArray = height*width;
            cudaError_t err = cudaMallocManaged(&array,sizeArray*sizeof(array[0]));
            if (err != cudaSuccess)
            {
                //cout << "Memory allocation failed"<<endl;
                printf("Memory allocation failed");
            }
        }

        Matrix::Matrix(size_t h,size_t w){
            height = h;
            width = w;
            sizeArray = height*width;
            cudaError_t err = cudaMallocManaged(&array,sizeArray*sizeof(array[0]));

            if (err != cudaSuccess)
            {
                //cout << "Memory allocation failed"<<endl;
                printf("Memory allocation failed");
            }

        }

        Matrix::Matrix(const Matrix &mat){
            height = mat.height;
            width = mat.width;
            sizeArray = mat.sizeArray;
            cudaError_t err = cudaMallocManaged(&array,sizeArray*sizeof(array[0]));
            if (err != cudaSuccess)
            {
                //cout << "Memory allocation failed"<<endl;
                printf("Memory allocation failed");
            }

            for(size_t i = 0;i<sizeArray;++i){
            array[i] = mat.array[i];
            }

        //copy(mat.array,mat.array+mat.sizeArray,array);
        }

        Matrix &Matrix::operator=(const Matrix &mat){
            height = mat.height;
            width = mat.width;
            sizeArray = mat.sizeArray;
            cudaError_t err = cudaMallocManaged(&array,sizeArray*sizeof(array[0]));
            if (err != cudaSuccess)
            {
                //cout << "Memory allocation failed"<<endl;
                printf("Memory allocation failed");
            }

            for(size_t i = 0;i<sizeArray;++i){
            array[i] = mat.array[i];
            }

            //copy(mat.array,mat.array+mat.sizeArray,array);
            return *this;
        }

        Matrix::~Matrix(){
            cudaFree(array);
        }

        void Matrix::assignValue(size_t i, size_t j, double value) {
            size_t l = i*width + j;
            array[l] = value;
        }

        void Matrix::assignValue2(size_t l, double value) {
            array[l] = value;
        }

        void Matrix::displayArray(){
            size_t i,j,l;
            for(i=0;i<height;++i){
                for(j=0;j<width;++j){
                    l =i*width + j;
                    //cout<<array[l]<<"\t";
                    printf("%f\t",array[l]);
                }
                //cout<<endl;
                printf("\n");
            }
        }
