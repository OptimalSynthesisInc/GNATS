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

#ifndef PUB_AIRCRAFT_H_
#define PUB_AIRCRAFT_H_

#include "pub_trajectory.h"
#include "pub_WaypointNode.h"

#include "real_t.h"

#include <vector>

using namespace std;

#define MAX_FLIGHT_PLAN_LENGTH   110

class NewMU_Aircraft {
public:
	vector<bool>   flag_external_aircraft;
	vector<bool>   flag_data_initialized;

	vector<real_t> latitude_deg;
	vector<real_t> longitude_deg;
	vector<real_t> latitude_deg_traj;
	vector<real_t> longitude_deg_traj;
	vector<real_t> altitude_ft;
	vector<real_t> rocd_fps;
	vector<real_t> tas_knots;
	vector<real_t> tas_knots_ground;
	vector<real_t> course_rad;
	vector<real_t> fpa_rad;
	vector<ENUM_Flight_Phase> flight_phase;

	vector<real_t> latitude_deg_pre_pause;
	vector<real_t> longitude_deg_pre_pause;
	vector<real_t> altitude_ft_pre_pause;
	vector<real_t> rocd_fps_pre_pause;
	vector<real_t> tas_knots_pre_pause;
	vector<real_t> course_rad_pre_pause;
	vector<real_t> fpa_rad_pre_pause;
	vector<real_t> cruise_alt_ft_pre_pause;
	vector<real_t> cruise_tas_knots_pre_pause;

	vector<real_t> departure_time_sec;
	vector<real_t> cruise_alt_ft;
	vector<real_t> cruise_tas_knots;

	vector<real_t> origin_airport_elevation_ft;
	vector<real_t> destination_airport_elevation_ft;

	vector<int> landed_flag;

	vector<int> adb_aircraft_type_index;

	vector<bool> holding_started;
	vector<bool> holding_stopped;
	vector<bool> has_holding_pattern;
	vector<int>  hold_start_index;
	vector<int>  hold_end_index;
	vector<real_t> holding_tas_knots;

	vector<int> target_waypoint_index;
	vector<real_t> target_altitude_ft;
	vector<int> toc_index;
	vector<int> tod_index;

	vector<char*> runway_name_departing;
	vector<char*> runway_name_landing;

	vector<waypoint_node_t*> target_waypoint_node_ptr;
	vector<bool>             flag_target_waypoint_change;
	vector<waypoint_node_t*> last_WaypointNode_ptr;
	vector<bool>             flag_reached_meterfix_point;

	vector<real_t>           V_horizontal;
	vector<waypoint_node_t*> acceleration_aiming_waypoint_node_ptr;
	vector<real_t>           acceleration;
	vector<real_t>           V2_point_latitude_deg;
	vector<real_t>           V2_point_longitude_deg;
	vector<real_t>           estimate_touchdown_point_latitude_deg;
	vector<real_t>           estimate_touchdown_point_longitude_deg;

	vector<float>            t_takeoff; // Record the time when the aircraft take off
	vector<float>            t_landing; // Record the time when the aircraft land

	vector<ENUM_Flight_Phase> hold_flight_phase; // Variable indicating the flight phase to be held

	vector<real_t>           course_rad_runway;
	vector<real_t>           course_rad_taxi;

	NewMU_Aircraft();
	~NewMU_Aircraft();

	void clear();

	void resize(int size);
};

#endif
