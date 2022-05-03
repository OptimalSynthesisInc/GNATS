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

#ifndef TG_WEATHER_H_
#define TG_WEATHER_H_

#include <cstdio>
#include <iostream>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace std;

int writeWindGridDefFile(const string grid_outfile,
		const double& lat_min,
		const double& lat_max,
		const double& lat_step,
		const double& lon_min,
		const double& lon_max,
		const double& lon_step,
		const double& alt_min,
		const double& alt_max,
		const double& alt_step);

int readWindConfigFile(const string& config_file,
		       double* const lat_min,
		       double* const lat_max,
		       double* const lat_step,
		       double* const lon_min,
		       double* const lon_max,
		       double* const lon_step,
		       double* const alt_min,
		       double* const alt_max,
		       double* const alt_step,
		       string* const grid_def_file,
		       map<int, string>* const files);

int writeWindH5Files(const double& lat_min,
		const double& lat_max,
		const double& lat_step,
		const double& lon_min,
		const double& lon_max,
		const double& lon_step,
		const double& alt_min,
		const double& alt_max,
		const double& alt_step,
		map<int, string>* const grib_files);

int tg_download_wind_files(const string dirtodnld = ".",
		const int hour1 = -100,
		const int day1 = -100,
		const int month1 = -100,
		const int year1 = -100,
		const int hour2 = -100,
		const int day2 = -100,
		const int month2 = -100,
		const int year2 = -100);

int tg_download_weather_files();

#endif
