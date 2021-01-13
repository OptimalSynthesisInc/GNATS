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
 * tg_simulation.h
 *
 *  Created on: Sep 19, 2013
 *      Author: jason
 */

#ifndef TG_SIMULATION_H_
#define TG_SIMULATION_H_

#include "cuda_compat.h"

#if USE_GPU
#include "cuda_runtime_api.h"
#include "device_functions.h"
#endif

#include "pub_aircraft.h"
#include "pub_FlightPhase.h"
#include "pub_incidentSequence.h"
#include "pub_WaypointNode.h"
#include "tg_trajectory.h"
#include "tg_adb.h"
#include "tg_sectors.h"
#include "tg_weatherWaypoint.h"
#include "tg_groundVehicle.h"
#include "util_windows_funcs.h"

#include <cfloat>

const int NATS_SIMULATION_STATUS_READY = 0;
const int NATS_SIMULATION_STATUS_START = 1;
const int NATS_SIMULATION_STATUS_PAUSE = 2;
const int NATS_SIMULATION_STATUS_RESUME = 3;
const int NATS_SIMULATION_STATUS_STOP = 4;
const int NATS_SIMULATION_STATUS_ENDED = 5;

const int RUNWAY_OFFSET_FT = 50;

// Data structure for function [update_states]
// [update_states] function calls multiple functions and data must be used by them.
// [update_states_t] data type is to store temporary variables for calculation.
typedef struct _update_states_t {
	char* acid;

	bool flag_data_initialized = false;

	ENUM_Flight_Phase flight_phase;

	bool flag_abnormal_on_runway = false;

	bool flag_aircraft_spacing = false;
	bool flag_aircraft_held_strategic = false;
	bool flag_aircraft_held_tactical = false;
	float duration_held_cdnr = 0;
	real_t tas_knots_before_held_cdnr;

	bool inc_twi_flag = false;
	int landed_flag;

	bool flag_ifs_exist = false; // Whether incident flight sequence data exists
	int simulation_user_incident_index = -1; // Index number of vector of simulation flight phase vector
	float t_processed_ifs = -1;

	real_t altitude_ft;
	real_t altitude_ft_bak;
	real_t lat;
	real_t lon;
	real_t tas_knots; // Speed of the aircraft.  This parameter will change after calculating new speed.
	real_t tas_knots_bak; // Speed when the loop starts.
	int    toc_index;
	int    tod_index;

	real_t cruise_tas_knots;
	real_t cruise_alt_ft;
	real_t target_altitude_ft;
	int    target_waypoint_index;

	real_t dest_airport_elev_ft;

	real_t Hdot_levelOff;

	waypoint_node_t* target_WaypointNode_ptr;
	waypoint_node_t* last_WaypointNode_ptr;
	bool flag_target_waypoint_change;

	waypoint_node_t* waypointNode_withValidAltitude_ptr;

	bool flag_course_rad_modifiedDuringPause;

	real_t go_around_split_point_latitude; // Latitude of the trajectory starting to fly toward Go-Around waypoint
	real_t go_around_split_point_longitude; // Longitude of the trajectory starting to fly toward Go-Around waypoint
	waypoint_node_t* go_around_WaypointNode_ptr; // Waypoint of the Go-Around point
	waypoint_node_t* pre_holding_pattern_target_WaypointNode_ptr; // Keep the target waypoint before HOLDING_PATTERN
	waypoint_node_t* holding_pattern_WaypointNode_ptr; // A list of waypoints where HOLDING_PATTERN happens

	real_t t_reaching_waypoint;

	real_t fp_lat;
	real_t fp_lon;
	int    fp_length;
	real_t tol;

	int ptf_index_lo;
	int ptf_index_hi;

	real_t adb_h;
	real_t adb_dh;
	real_t adb_roc_fpm;
	real_t adb_rod_fpm;

	real_t hdg_rad = 0;
	real_t fpa_rad = 0;
	real_t rocd_fps = 0;
	real_t rocd_fps_bak = 0;

	real_t dh_desired = 0;
	real_t dh_possible = 0;
	real_t dh = 0;

	float elapsedSecond;
	float durationSecond_to_be_proc; // Period of time to be processed.  The aircraft may switch phases within the same time step.  This variable starts with the initial time step and will be updated based on actual waypoint change or flight phase change.
	float durationSecond_altitude_proc;

	real_t S_within_flightPhase_and_simCycle;

	real_t course_rad_runway;
	real_t course_rad_taxi;

	real_t acceleration = 0;
	real_t V_takeoff = 0;
	real_t V_touchdown = 0;
	real_t V_stall_takeoff = 0;
	real_t V_stall_landing = 0;
	real_t V2_tas_knots = 0;
	real_t V_crossWind = 0;
	real_t V_downWind = 0;
	real_t V_horizontal = 0;
	real_t V_ground = 0;
	real_t V_ground_bak = 0;

	double L_pushback = 0;
	double L_takeoff = 0;
	double L_landing = 0;
	double L_re = 0;

	real_t L_to_go = DBL_MAX;

	float departing_runway_entry_latitude = DBL_MAX;
	float departing_runway_entry_longitude = DBL_MAX;
	float departing_runway_end_latitude = DBL_MAX;
	float departing_runway_end_longitude = DBL_MAX;
	float landing_runway_entry_latitude = DBL_MAX;
	float landing_runway_entry_longitude = DBL_MAX;
	float landing_runway_end_latitude = DBL_MAX;
	float landing_runway_end_longitude = DBL_MAX;

	double time_to_conflict = 0.0;
} update_states_t;

extern long sim_id;

//Data structure to record flight phases to be skipped by pilot <Flight Index, Flight Phase>
extern std::map<int, string> skipFlightPhase;

//Data structure to record lag parameter values for controller/pilot
extern std::map< int, std::vector<string> > lagParams;
extern std::map< int, std::map<string, std::vector<float> > > lagParamValues;

//Data structure to record lag parameter values for ground vehicles
extern std::map< string, std::vector<string> > gvLagParams;
extern std::map< string, std::map<string, std::vector<float> > > gvLagParamValues;

//Data structure to record ground operator absence, and the vehicle stops for those many time steps
extern std::map< string, int > groundOperatorAbsence;

//Data structure to record ground operator repeat operations
extern std::map< string, vector <string> > groundOperatorActionRepeat;

//Data structure to record STAR meter fix points
extern std::map< string, std::vector<string> > meterFixMap;

//Data structure to aircraft and cargo cost within simulation
extern std::map< string, map <string, double> > aircraftAndCargoData;

//Data structure to record spacing distance for meter fix in a center
extern std::map< string, map <string, pair <string, float> > > spacingDistMap;

//Data structure for touchdown and takeoff points points of an aircraft
extern std::map< string, map <string, pair <double, double> > > aircraftRunwayData;

//Data structure to store ground vehicle simulation data to be written
extern std::map< string, std::vector<string> > groundVehicleSimulationData;

//Data structure to handle ground operator course change for ground vehicle simulation
extern std::vector<string> courseChange;

//Data structure to store ground communication instrument error
extern std::map< string, pair <string, double> > radarErrorModel;

extern std::map<string, pair<string, float> > map_CDR_status;

//Time step count and default parameter values for controller absence
extern int controllerAway;
extern float defaultSpeed;
extern float defaultCourse;
extern float defaultRocd;

//Flags for controller error template, no error by default
extern int setActionRepeatControllerFlag;
extern int skipFlightPhaseControllerFlag;
extern int setWrongActionControllerFlag;
extern int setActionReversalControllerFlag;
extern int setPartialActionControllerFlag;
extern int skipChangeActionControllerFlag;
extern int setActionLagControllerFlag;
extern int setActionValueControllerFlag;

extern float t_start;
extern float t_end;
extern float t_step;
extern float t_step_terminal;
extern float t_data_collection_period_airborne;

extern int nats_simulation_check_interval;
extern int nats_simulation_status;

extern float time_step_surface_realTime_simulation; // Seconds
extern float time_step_terminal_realTime_simulation; // Seconds
extern float time_step_airborne_realTime_simulation; // Seconds

extern bool flag_realTime_simulation;
extern bool flag_realTime_simulation_singleUser;
extern timeval timeval_realTime_simulation_synchronized;
extern long nextPropagation_utc_time_realTime_simulation;

extern bool flag_enable_strategic_weather_avoidance;
extern bool flag_enable_cdnr;

extern int cnt_event_cdnr;

extern float cdr_initiation_distance_ft_surface;
extern float cdr_initiation_distance_ft_terminal;
extern float cdr_initiation_distance_ft_enroute;
extern float cdr_separation_distance_ft_surface;
extern float cdr_separation_distance_ft_terminal;
extern float cdr_separation_distance_ft_enroute;

extern string tg_pathFilename_polygon;
extern string tg_pathFilename_sigmet;
extern string tg_pathFilename_pirep;

extern vector<WeatherWaypoint> vector_tactical_weather_waypoint;

// device constant memory pointers(CUDA)
__device__ real_t*        c_departure_time_sec;
__device__ real_t*        c_cruise_alt_ft;
__device__ real_t*        c_cruise_tas_knots;

__device__ real_t*        c_origin_airport_elevation_ft;
__device__ real_t*        c_destination_airport_elevation_ft;
__device__ int*           c_sector_index;
__device__ real_t*        c_latitude_deg;
__device__ real_t*        c_longitude_deg;
__device__ real_t*        c_altitude_ft;
__device__ real_t*        c_rocd_fps;
__device__ real_t*        c_prev_rocd_fps;
__device__ real_t*        c_tas_knots;
__device__ real_t*        c_course_rad;
__device__ real_t*        c_fpa_rad;

__device__ ENUM_Flight_Phase* c_flight_phase;

__device__ int*           c_landed_flag;
__device__ int*           c_adb_aircraft_type_index;


__device__ int*           c_target_waypoint_index;
__device__ real_t*        c_target_altitude_ft;
__device__ int*           c_toc_index;
__device__ int*           c_tod_index;

__device__ waypoint_node_t** c_target_waypoint_node_ptr;
__device__ bool*            c_flag_target_waypoint_change;
__device__ waypoint_node_t** c_last_WaypointNode_ptr;
__device__ bool*            c_flag_reached_meterfix_point;

__device__ real_t*           c_V_horizontal;
__device__ real_t*           c_V_ground;
__device__ waypoint_node_t** c_acceleration_aiming_waypoint_node_ptr;
__device__ real_t*           c_acceleration;
__device__ real_t*           c_V2_point_latitude_deg;
__device__ real_t*           c_V2_point_longitude_deg;
__device__ real_t*           c_estimate_touchdown_point_latitude_deg;
__device__ real_t*           c_estimate_touchdown_point_longitude_deg;

__device__ float*              c_t_takeoff; // Record the time when the aircraft take off
__device__ float*              c_t_landing; // Record the time when the aircraft land

__device__ ENUM_Flight_Phase* c_hold_flight_phase; // Variable indicating the flight phase to be held

__device__ real_t*           c_course_rad_runway;
__device__ real_t*           c_course_rad_taxi;

__device__ short*            c_adb_num_rows;
__device__ short*            c_adb_table_start_index;
__device__ short*            c_adb_lower_bound_row;
__device__ short*            c_adb_fl;
__device__ short*            c_adb_vtas_climb_knots;
__device__ short*            c_adb_vtas_descent_knots;
__device__ short*            c_adb_roc_fpm;
__device__ short*            c_adb_rod_fpm;

__device__ int*      c_sector_grid;
__device__ int*      c_sector_grid_cell_counts;
__device__ sector_t* c_sector_array;

__device__ real_t*    c_wind_north;
__device__ real_t*    c_wind_east;
__device__ real_t*    c_wind_north_unc;
__device__ real_t*    c_wind_east_unc;

__device__ real_t     c_lat_min;
__device__ real_t     c_lat_max;
__device__ real_t     c_lat_step;
__device__ real_t     c_lon_min;
__device__ real_t     c_lon_max;
__device__ real_t     c_lon_step;
__device__ real_t     c_alt_min;
__device__ real_t     c_alt_max;
__device__ real_t     c_alt_step;
__device__ int        c_num_lat_cells;
__device__ int        c_num_lon_cells;
__device__ int        c_num_alt_cells;

int  set_device_ruc_pointers();
void get_wind_field_components(const real_t& t, const real_t& lat, const real_t& lon, const real_t& alt, real_t* const wind_east, real_t* const wind_north);

real_t aircraft_compute_ground_speed(const float t,
		const double latitude_deg,
		const double longitude_deg,
		const double altitude_ft,
		const double tas_knots_horizontal,
		const double course_rad);

int compute_flight_sector(const real_t& lat_deg,
		                             	const real_t& lon_deg,
		                                const real_t& alt_ft,
		                                const int& cur_sector_index);
int set_device_sector_pointers();

int insert_airborne_waypointNode(const int index_flight,
		const int waypoint_index_to_insert,
		const string waypoint_type,
		const string waypoint_name,
		const float waypoint_latitude,
		const float waypoint_longitude,
		const float waypoint_altitude,
		const float waypoint_speed_lim,
		const string waypoint_spdlim_desc);

int delete_airborne_waypointNode(const int index_flight,
		const int waypoint_index_to_delete);

string get_latestFilename(const char* dirPath, const char* extFilename);

void clear_trajectory();

int propagate_flights(const float& t_horizon_minutes, const float& t_step_sec);

int propagate_flights(const float& input_t_end,
		const float& input_t_step,
		const float& input_t_step_terminal,
		const float& input_t_step_airborne);

void propagate_flights_proc();

int get_runtime_sim_status();

float get_curr_sim_time();

void nats_simulation_operator(int usr_prop_status);

void set_nats_simulation_duration(long value);

void set_nats_simulation_duration(float value);

void set_target_altitude_ft(int index_flight, float target_altitude_ft);

/*************************************CDNR STARTS*******************************************/
#if CDNR_FLAG
// Conflict detection and Resolution
void conflictDetectionAndResolution(const int& num_flights, const long& t,const long& t_step);

//Resolve conflict between 2 aircraft
void resolveConflict(const int& index1, const int& index2,const long& t, const long& tstep);

//Propagation for conflict resolution
void propagateLateralforDeltaT(const long& tstep,const long& t,
				const real_t& fplat, const real_t& fplon, const real_t& alt,
				const real_t& tas_knots, const real_t& rocd_fps,
				real_t& lat, real_t& lon);

//PropagateVertical
void propagateVerticalforDeltaT(const long& tstep, const flight_mode_e& flight_mode,
		const real_t& target_altitude_ft, const int& adb_table_index,
		real_t& alt, real_t& rocd_fps, real_t& tas_knots) ;

//Update target waypoint for conflict resolution
void updateTargetWayPointForConflictResolution(const long& tstep, const int& fp_length,
		const int& tod_index,const real_t fplat,const real_t fplon,
		const real_t lat, const real_t lon, const real_t alt, const real_t& dest_ap_elev_ft,
		int& target_waypoint_index,real_t& tas_knots, flight_mode_e& flight_mode,
		real_t& target_altitude_ft);

//Change target waypoint to resolve conflict
void changeNextTargetWaypoint(const double& r,
		const real_t& lat1, const real_t& lon1, const real_t& lat2,
		const real_t& lon2,const real_t& alt1, const real_t& alt2,
		real_t& fplat1, real_t& fplon1, real_t& fplat2, real_t& fplon2);
#endif

#endif /* TG_SIMULATION_H_ */
