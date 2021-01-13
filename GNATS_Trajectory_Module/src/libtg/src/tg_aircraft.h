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

/*
 * tg_aircraft.h
 *
 *  Created on: Sep 17, 2013
 *      Author: jason
 */

#ifndef TG_AIRCRAFT_H_
#define TG_AIRCRAFT_H_

#include "FlightPlan.h"

#include "tg_trajectory.h"
#include "TrxRecord.h"

#include "tg_Point.h"
#include "pub_WaypointNode.h"

#include <string>
#include <vector>
#include <map>
#include "pub_aircraft.h"
#include "pub_TaxiPlan.h"
#include "pub_User.h"

using std::string;
using std::vector;
using std::map;

#define UNDEFINED_HEADING        -999

#define ECCENTRICITY_SQ_EARTH  0.00669437999014
#define	SEMI_MAJOR_EARTH_FT 6378137.0*3.2808399
#define NauticalMilestoFeet 6076.12

extern map<string, int> map_Acid_FlightSeq;

extern int external_aircraft_seq;
extern const char* EXTERNAL_AIRCRAFT_ID;

// Map to store aircraft id and the owner user
extern map<string, string> map_aircraft_owner;

typedef struct _aircraft_t {

	bool* flag_geoStyle; // Flag indicating whether the aircraft is defined with geo-location waypoints

	int*    sector_index; // Sector data.  As of 2020.01.23, sector data is not output in the trajectory file.  This field may be reviewed and removed later.

	// aircraft current state
	real_t* latitude_deg;
	real_t* longitude_deg;
	real_t* altitude_ft;
	real_t* rocd_fps;
	real_t* tas_knots;
	real_t* tas_knots_ground;
	real_t* course_rad;
	real_t* fpa_rad;
	ENUM_Flight_Phase* flight_phase;

	// aircraft static data
	real_t* departure_time_sec;
	real_t* cruise_alt_ft;
	real_t* cruise_tas_knots;

	// Store value before pause
	real_t* latitude_deg_pre_pause;
	real_t* longitude_deg_pre_pause;
	real_t* altitude_ft_pre_pause;
	real_t* rocd_fps_pre_pause;
	real_t* tas_knots_pre_pause;
	real_t* course_rad_pre_pause;
	real_t* fpa_rad_pre_pause;
	real_t* cruise_alt_ft_pre_pause;
	real_t* cruise_tas_knots_pre_pause;
	// end - Store value before pause

	//TILL HERE
	real_t* origin_airport_elevation_ft;
	real_t* destination_airport_elevation_ft;

	// other flags
	int* landed_flag;

	// aircraft ADB PTF climb/descent
	int* adb_aircraft_type_index; // selects the table

	// aircraft holding pattern data
	bool* holding_started;
	bool* holding_stopped;
	bool* has_holding_pattern;
	int*  hold_start_index;
	int*  hold_end_index;
	real_t* holding_tas_knots;

	// aircraft intent
	int* target_waypoint_index;
	real_t* target_altitude_ft;
	int* toc_index;
	int* tod_index;

	char** runway_name_departing;
	char** runway_name_landing;

	waypoint_node_t** target_waypoint_node_ptr;
	bool*            flag_target_waypoint_change;
	waypoint_node_t** last_WaypointNode_ptr;
	bool*            flag_reached_meterfix_point;

	real_t*           V_horizontal;
	waypoint_node_t** acceleration_aiming_waypoint_node_ptr;
	real_t*           acceleration;
	real_t*           V2_point_latitude_deg; // Actual TAKEOFF point
	real_t*           V2_point_longitude_deg; // Actual TAKEOFF point
	real_t*           estimate_takeoff_point_latitude_deg; // Estimate TAKEOFF point.  This point could be calculated by simulation logic or set by user.
	real_t*           estimate_takeoff_point_longitude_deg; // Estimate TAKEOFF point.  This point could be calculated by simulation logic or set by user.
	real_t*           estimate_touchdown_point_latitude_deg; // Estimate TOUCHDOWN point.  This point could be calculated by simulation logic or set by user.
	real_t*           estimate_touchdown_point_longitude_deg; // Estimate TOUCHDOWN point.  This point could be calculated by simulation logic or set by user.

	float*            t_takeoff; // Record the time when the aircraft take off
	float*            t_landing; // Record the time when the aircraft land

	ENUM_Flight_Phase* hold_flight_phase; // Variable indicating the flight phase to be held

	real_t*           course_rad_runway;
	real_t*           course_rad_taxi;
} aircraft_t;

typedef struct _wind_t {

	// wind field components
	real_t* wind_north_fps;
	real_t* wind_east_fps;

} wind_t;

extern map<int, osi::TrxRecord> g_trx_records;
extern vector<Trajectory> g_trajectories;

extern waypoint_node_t** array_Airborne_Flight_Plan_ptr;
extern waypoint_node_t** array_Airborne_Flight_Plan_toc_ptr;
extern waypoint_node_t** array_Airborne_Flight_Plan_tod_ptr;
extern waypoint_node_t** array_Airborne_Flight_Plan_Final_Node_ptr;

extern int* array_Airborne_Flight_Plan_Waypoint_length;

extern aircraft_t h_aircraft_soa;
extern aircraft_t d_aircraft_soa;

// This variable is only for external aircrafts.  Temporary purpose
//
// Future modification to be done
// All aircrafts(from TRX and from external) must be handled by a single variable
extern NewMU_Aircraft newMU_external_aircraft;

real_t compute_descent_dist(int adb_table, real_t dest_elev, real_t cruise_alt);

real_t compute_climb_dist(int adb_table, real_t orig_elev, real_t cruise_alt);

waypoint_node_t* getWaypointNodePtr_by_flightSeq(int flightSeq, int index_airborne_waypoint);

real_t get_airport_elevation(const string& airport_code);

/**
 * Get touchdown point latitude and longitude
 */
pair<double,double> getTouchdownPoint(const string& ap, const string& runway, const double airport_alt_ft, const double& landing_length);

 /*Get runway heading*/
double get_runway_hdg(const string& ap,		const string& rw1,		const string& rw2);

int load_aircraft(const string& trx_file, const string& mfl_file, const real_t& cruise_perturbation=0);

// This function is planned to create aircraft data for multi-user environment.  However, currently this function only validates flight plan.
// To be done in multi-user environment --> Insert flight data in vector for further simulation.
bool collect_user_upload_aircraft(string string_track_time,
		string string_track,
		string string_fp_route,
		const int mfl_ft,
		const bool flag_validator,
		string& errorMsg);

int destroy_aircraft();
int get_num_flights();
void reset_num_flights();
int select_flightSeq_by_aircraftId(string acid);

#endif /* TG_AIRCRAFT_H_ */
