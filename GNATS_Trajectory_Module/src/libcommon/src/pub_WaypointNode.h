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

#ifndef __WAYPOINT_NODE_H__
#define __WAYPOINT_NODE_H__

#include "cuda_compat.h"

#if USE_GPU
#include "cuda_runtime_api.h"
#include "device_functions.h"
#endif

#include <cstdlib>

/**
 * Definition of waypoint node
 * 
 * This waypoint node structure is used to create linked lists of departing taxi plan, landing taxi plan, airborne flight plan and ground vehicle drive plan
 */
typedef struct _waypoint_node_t {
	bool flag_geoStyle = false;

	char* wpname = NULL;
	char* wptype = NULL;

	double latitude;
	double longitude;

	double distance_to_next_node; // Distance from the current waypoint to the next one

	double course_rad_to_next_node;

	char* alt_desc = NULL;
	double alt_1;
	double alt_2;

	double altitude_estimate;

	char* procname = NULL;
	char* proctype = NULL;

	char* recco_navaid = NULL;
	double theta;
	double rho;
	double mag_course;
	double rt_dist;

	char* spdlim_desc = NULL;
	double speed_lim;

	char* phase = NULL;

	_waypoint_node_t* prev_node_ptr = NULL;
	_waypoint_node_t* next_node_ptr = NULL;
} waypoint_node_t;

#if USE_GPU
__device__ int releaseWaypointNodeContent(waypoint_node_t* waypoint_node_ptr);
#else
int releaseWaypointNodeContent(waypoint_node_t* waypoint_node_ptr);
#endif

#endif
