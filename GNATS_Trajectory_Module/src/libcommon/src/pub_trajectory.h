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

#ifndef PUB_TRAJECTORY_H_
#define PUB_TRAJECTORY_H_

#include "cuda_compat.h"

#if USE_GPU
#include "cuda_runtime_api.h"
#include "device_functions.h"
#endif

#include <vector>

#define FOREACH_FLIGHT_PHASE(FLIGHT_PHASE) \
		FLIGHT_PHASE(FLIGHT_PHASE_PREDEPARTURE)   \
		FLIGHT_PHASE(FLIGHT_PHASE_ORIGIN_GATE)   \
		FLIGHT_PHASE(FLIGHT_PHASE_PUSHBACK)   \
		FLIGHT_PHASE(FLIGHT_PHASE_RAMP_DEPARTING)   \
		FLIGHT_PHASE(FLIGHT_PHASE_TAXI_DEPARTING)   \
		FLIGHT_PHASE(FLIGHT_PHASE_RUNWAY_THRESHOLD_DEPARTING)   \
		FLIGHT_PHASE(FLIGHT_PHASE_TAKEOFF)   \
		FLIGHT_PHASE(FLIGHT_PHASE_CLIMBOUT)   \
		FLIGHT_PHASE(FLIGHT_PHASE_HOLD_IN_DEPARTURE_PATTERN)   \
		FLIGHT_PHASE(FLIGHT_PHASE_CLIMB_TO_CRUISE_ALTITUDE)   \
		FLIGHT_PHASE(FLIGHT_PHASE_TOP_OF_CLIMB)   \
		FLIGHT_PHASE(FLIGHT_PHASE_CRUISE)   \
		FLIGHT_PHASE(FLIGHT_PHASE_HOLD_IN_ENROUTE_PATTERN)   \
		FLIGHT_PHASE(FLIGHT_PHASE_TOP_OF_DESCENT)   \
		FLIGHT_PHASE(FLIGHT_PHASE_INITIAL_DESCENT)   \
		FLIGHT_PHASE(FLIGHT_PHASE_HOLD_IN_ARRIVAL_PATTERN)   \
		FLIGHT_PHASE(FLIGHT_PHASE_APPROACH)   \
		FLIGHT_PHASE(FLIGHT_PHASE_FINAL_APPROACH)   \
		FLIGHT_PHASE(FLIGHT_PHASE_GO_AROUND)   \
		FLIGHT_PHASE(FLIGHT_PHASE_TOUCHDOWN)   \
		FLIGHT_PHASE(FLIGHT_PHASE_LAND)   \
		FLIGHT_PHASE(FLIGHT_PHASE_EXIT_RUNWAY)   \
		FLIGHT_PHASE(FLIGHT_PHASE_TAXI_ARRIVING)   \
		FLIGHT_PHASE(FLIGHT_PHASE_RUNWAY_CROSSING)   \
		FLIGHT_PHASE(FLIGHT_PHASE_RAMP_ARRIVING)   \
		FLIGHT_PHASE(FLIGHT_PHASE_DESTINATION_GATE)   \
		FLIGHT_PHASE(FLIGHT_PHASE_LANDED)   \
		FLIGHT_PHASE(FLIGHT_PHASE_HOLDING)   \
		FLIGHT_PHASE(FLIGHT_PHASE_RUNWAYUNDERSHOOT)   \
		FLIGHT_PHASE(FLIGHT_PHASE_RUNWAYOVERSHOOT)   \
		FLIGHT_PHASE(FLIGHT_PHASE_OUTOFRUNWAY)   \
		FLIGHT_PHASE(FLIGHT_PHASE_TAKEOFF_STALL)   \
		FLIGHT_PHASE(FLIGHT_PHASE_USER_INCIDENT)

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

enum ENUM_Flight_Phase {
    FOREACH_FLIGHT_PHASE(GENERATE_ENUM)
};

static const char* ENUM_Flight_Phase_String[] = {
    FOREACH_FLIGHT_PHASE(GENERATE_STRING)
};

const int ENUM_Flight_Phase_Count = 33;

extern char const INITIAL_PT[];
extern char const TOP_OF_CLIMB_PT[];
extern char const TOP_OF_DESCENT_PT[];

bool isFlightPhase_in_ground_departing(const ENUM_Flight_Phase flight_phase);
bool isFlightPhase_in_ground_landing(const ENUM_Flight_Phase flight_phase);
bool isFlightPhase_in_climbing(const ENUM_Flight_Phase flight_phase);
bool isFlightPhase_in_cruising(const ENUM_Flight_Phase flight_phase);
bool isFlightPhase_in_descending(const ENUM_Flight_Phase flight_phase);
bool isFlightPhase_in_airborne(const ENUM_Flight_Phase flight_phase);

ENUM_Flight_Phase getFlight_Phase(const char* flight_phase_string);

#define TRACON_ALT_FT 10000
#define ALT_FT_5000 5000

#endif
