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

#include "pub_trajectory.h"

#include "util_string.h"

#define stringify(name) #name

char const INITIAL_PT[] = "INITIAL_PT";
char const TOP_OF_CLIMB_PT[] = "TOP_OF_CLIMB_PT";
char const TOP_OF_DESCENT_PT[] = "TOP_OF_DESCENT_PT";

bool isFlightPhase_in_ground_departing(const ENUM_Flight_Phase flight_phase) {
	bool retValue = false;

	if ((FLIGHT_PHASE_ORIGIN_GATE <= flight_phase) && (flight_phase <= FLIGHT_PHASE_TAKEOFF)) {
		retValue = true;
	}

	return retValue;
}

bool isFlightPhase_in_ground_landing(const ENUM_Flight_Phase flight_phase) {
	bool retValue = false;

	if ((FLIGHT_PHASE_TOUCHDOWN <= flight_phase) && (flight_phase <= FLIGHT_PHASE_DESTINATION_GATE)) {
		retValue = true;
	}

	return retValue;
}

bool isFlightPhase_in_climbing(const ENUM_Flight_Phase flight_phase) {
	bool retValue = false;

	if ((FLIGHT_PHASE_CLIMBOUT <= flight_phase) && (flight_phase <= FLIGHT_PHASE_CLIMB_TO_CRUISE_ALTITUDE)) {
		retValue = true;
	}

	return retValue;
}

bool isFlightPhase_in_cruising(const ENUM_Flight_Phase flight_phase) {
	bool retValue = false;

	if ((FLIGHT_PHASE_CRUISE <= flight_phase) && (flight_phase <= FLIGHT_PHASE_HOLD_IN_ENROUTE_PATTERN)) {
		retValue = true;
	}

	return retValue;
}

bool isFlightPhase_in_descending(const ENUM_Flight_Phase flight_phase) {
	bool retValue = false;

	if ((FLIGHT_PHASE_INITIAL_DESCENT <= flight_phase) && (flight_phase <= FLIGHT_PHASE_GO_AROUND)) {
		retValue = true;
	}

	return retValue;
}

bool isFlightPhase_in_airborne(const ENUM_Flight_Phase flight_phase) {
	bool retValue = false;

	if ((FLIGHT_PHASE_CLIMBOUT <= flight_phase) && (flight_phase <= FLIGHT_PHASE_GO_AROUND)) {
		retValue = true;
	}

	return retValue;
}

ENUM_Flight_Phase getFlight_Phase(const char* flight_phase_string) {
	ENUM_Flight_Phase retFlight_phase = FLIGHT_PHASE_PREDEPARTURE; // Default

	for (unsigned int i = 0; i < ENUM_Flight_Phase_Count; i++) {
		if (strcmp(ENUM_Flight_Phase_String[i], flight_phase_string) == 0) {
			retFlight_phase = ENUM_Flight_Phase(i);

			break;
		}
	}

	return retFlight_phase;
}
