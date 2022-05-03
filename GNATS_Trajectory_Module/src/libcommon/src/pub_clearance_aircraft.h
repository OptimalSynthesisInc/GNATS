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

#ifndef __CLEARANCE_AIRCRAFT_H__
#define __CLEARANCE_AIRCRAFT_H__

#include "cuda_compat.h"

#include <map>
#include <string>
#include <vector>

using namespace std;

#define FOREACH_AIRCRAFT_CLEARANCE(AIRCRAFT_CLEARANCE) \
		AIRCRAFT_CLEARANCE(AIRCRAFT_CLEARANCE_PUSHBACK)   \
		AIRCRAFT_CLEARANCE(AIRCRAFT_CLEARANCE_TAXI_DEPARTING)   \
		AIRCRAFT_CLEARANCE(AIRCRAFT_CLEARANCE_TAKEOFF)   \
		AIRCRAFT_CLEARANCE(AIRCRAFT_CLEARANCE_ENTER_ARTC)   \
		AIRCRAFT_CLEARANCE(AIRCRAFT_CLEARANCE_DESCENT_FROM_CRUISE)   \
		AIRCRAFT_CLEARANCE(AIRCRAFT_CLEARANCE_ENTER_TRACON)   \
		AIRCRAFT_CLEARANCE(AIRCRAFT_CLEARANCE_APPROACH)   \
		AIRCRAFT_CLEARANCE(AIRCRAFT_CLEARANCE_TOUCHDOWN)   \
		AIRCRAFT_CLEARANCE(AIRCRAFT_CLEARANCE_TAXI_LANDING)   \
		AIRCRAFT_CLEARANCE(AIRCRAFT_CLEARANCE_RAMP_LANDING)   \


#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

enum ENUM_Aircraft_Clearance {
    FOREACH_AIRCRAFT_CLEARANCE(GENERATE_ENUM)
};

static const char* ENUM_Aircraft_Clearance_String[] = {
    FOREACH_AIRCRAFT_CLEARANCE(GENERATE_STRING)
};

const int ENUM_Aircraft_Clearance_Count = 10;

typedef struct _Aircraft_Clearance_Data_t {
	int decision;
	float t_request;
	float t_decision;
} Aircraft_Clearance_Data_t;

extern vector< map<ENUM_Aircraft_Clearance, Aircraft_Clearance_Data_t> > g_map_clearance_aircraft;

#if USE_GPU
__device__ ENUM_Aircraft_Clearance getENUM_Aircraft_Clearance(const char* str_aircraft_clearance);
#else
ENUM_Aircraft_Clearance getENUM_Aircraft_Clearance(const char* str_aircraft_clearance);
#endif

#endif
