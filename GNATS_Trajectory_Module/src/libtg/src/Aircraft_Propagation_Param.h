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

#ifndef __AIRCRAFT_PROPAGATION_PARAM_H__
#define __AIRCRAFT_PROPAGATION_PARAM_H__

#include "real_t.h"
//#include "AdbOPFModel.h"

typedef struct {
	bool* flag_geoStyle;

	char** flight_phase;

	// Check if device param can point to the host
	waypoint_node_t** airborne_flight_plan_ptr;
	waypoint_node_t** waypoint_final_node_ptr;

	char** skip_flight_phase;

	//AdbOPFModel* adbOPFModel;







//	char* acid;
//
//	bool flag_data_initialized = false;
//
//	ENUM_Flight_Phase flight_phase;
//
//	bool flag_abnormal_on_runway = false;
//
//	bool flag_aircraft_spacing = false;
//	bool flag_aircraft_held_strategic = false;
//	bool flag_aircraft_held_tactical = false;
//	float duration_held_cdnr = 0;
//	real_t tas_knots_before_held_cdnr;
//
//	bool inc_twi_flag = false;
//	int landed_flag;
//
//	bool flag_ifs_exist = false; // Whether incident flight sequence data exists
//	int simulation_user_incident_index = -1; // Index number of vector of simulation flight phase vector
//	float t_processed_ifs = -1;
//
//	real_t altitude_ft;
//	real_t altitude_ft_bak;
	real_t* lat;
//	real_t lon;
//	real_t tas_knots; // Speed of the aircraft.  This parameter will change after calculating new speed.
//	real_t tas_knots_bak; // Speed when the loop starts.
//	int    toc_index;
//	int    tod_index;
//
//	real_t cruise_tas_knots;
//	real_t cruise_alt_ft;
//	real_t target_altitude_ft;
//	int    target_waypoint_index;
//
//	real_t dest_airport_elev_ft;
//
//	real_t Hdot_levelOff;
//
//	waypoint_node_t* target_WaypointNode_ptr;
//	waypoint_node_t* last_WaypointNode_ptr;
//	bool flag_target_waypoint_change;
//
//	waypoint_node_t* waypointNode_withValidAltitude_ptr;
//
//	bool flag_course_rad_modifiedDuringPause;
//
//	real_t go_around_split_point_latitude; // Latitude of the trajectory starting to fly toward Go-Around waypoint
//	real_t go_around_split_point_longitude; // Longitude of the trajectory starting to fly toward Go-Around waypoint
//	waypoint_node_t* go_around_WaypointNode_ptr; // Waypoint of the Go-Around point
//	waypoint_node_t* pre_holding_pattern_target_WaypointNode_ptr; // Keep the target waypoint before HOLDING_PATTERN
//	waypoint_node_t* holding_pattern_WaypointNode_ptr; // A list of waypoints where HOLDING_PATTERN happens
//
//	real_t t_reaching_waypoint;
//
//	real_t fp_lat;
//	real_t fp_lon;
//	int    fp_length;
//	real_t tol;
//
//	//int sector;
//	//int sector_prev;
//
//	int ptf_index_lo;
//	int ptf_index_hi;
//
//	real_t adb_h;
//	real_t adb_dh;
//	real_t adb_roc_fpm;
//	real_t adb_rod_fpm;
//
//	real_t hdg_rad = 0;
//	real_t fpa_rad = 0;
//	real_t rocd_fps = 0;
//	real_t rocd_fps_bak = 0;
//
//	real_t dh_desired = 0;
//	real_t dh_possible = 0;
//	real_t dh = 0;
//
//	float elapsedSecond;
//	float durationSecond_to_be_proc; // Period of time to be processed.  The aircraft may switch phases within the same time step.  This variable starts with the initial time step and will be updated based on actual waypoint change or flight phase change.
//	float durationSecond_altitude_proc;
//
//	real_t S_within_flightPhase_and_simCycle;
//
//	real_t course_rad_runway;
//	real_t course_rad_taxi;
//
//	real_t acceleration = 0;
//	real_t V_takeoff = 0;
//	real_t V_touchdown = 0;
	real_t* V_stall_takeoff;
	real_t* V_stall_landing;
	real_t* V2_tas_knots;
//	real_t V_crossWind = 0;
//	real_t V_downWind = 0;
//	real_t V_horizontal = 0;
//	real_t V_ground = 0;
//	real_t V_ground_bak = 0;

	real_t* ldl;

//	double L_pushback = 0;
//	double L_takeoff = 0;
//	double L_landing = 0;
//	double L_re = 0;
//
//	real_t L_to_go = DBL_MAX;
//
//	float departing_runway_entry_latitude = DBL_MAX;
//	float departing_runway_entry_longitude = DBL_MAX;
//	float departing_runway_end_latitude = DBL_MAX;
//	float departing_runway_end_longitude = DBL_MAX;
//	float landing_runway_entry_latitude = DBL_MAX;
//	float landing_runway_entry_longitude = DBL_MAX;
//	float landing_runway_end_latitude = DBL_MAX;
//	float landing_runway_end_longitude = DBL_MAX;
//
//	double time_to_conflict = 0.0;
} Aircraft_Propagation_Param;

#endif
