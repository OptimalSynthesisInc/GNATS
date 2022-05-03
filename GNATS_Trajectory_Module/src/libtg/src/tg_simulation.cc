/*
 * tg_simulation.cc
 *
 * Updated: 09/11/2018
 * Author: Oliver Chen
 *
 *  Simulation function is triggered in function
 * 		propagate_flights(const float& input_t_end,
 *			const float& input_t_step,
 *			const float& input_t_step_terminal,
 *			const float& input_t_step_airborne)
 *	And it starts another thread running function [propagate_flights_proc]
 *		this function processes the looping.  "Start", "Pause", "Resume" and "Stop" happen in this function.
 *
 *  [propagate_flights_proc] function calls [launch_kernel] function
 *
 *  [launch_kernel] function calls [update_states] function
 *
 *  [update_states] function is the primary place for main algorithm.
 *  It calls:
 *  1. pilot_proc_flightPhase function: Process the transition of flight phase
 *  2. pilot_algorithm_pre_proc function: Execute pre-processing and data preparation before main algorithm starts
 *  3. update_states_proc_flightPhase_algorithm function: Trigger main algorithm of individual flight phase
 *
 *  Waypoints used in simulation are forming 3 different data:
 *  1. Departing taxi plan
 *  2. Airborne flight plan
 *  3. Landing taxi plan
 *  These 3 data is stored separately in 3 variables: h_departing_taxi_plan, array_Airborne_Flight_Plan_ptr, h_landing_taxi_plan
 *  The element data types of these 3 variables are using same type which is [waypoint_node_t].
 *
 *	The important variable of the simulation is [update_states->target_WaypointNode_ptr]
 *	[update_states->target_WaypointNode_ptr] is a pointer of waypoint.  It points to the target waypoint that the aircraft is flying to.
 */

#include "AandBCoefficient.h"
#include "AirportLayoutDataLoader.h"

#include "Controller.h"

#include "geometry_utils.h"

#include "rg_exec.h"

#include "tg_api.h"
#include "tg_flightplan.h"
#include "tg_simulation.h"

#include "tg_aircraft.h"
#include "tg_groundVehicle.h"
#include "tg_airports.h"
#include "tg_incidentFlightPhase.h"
#include "tg_rap.h"
#include "tg_riskMeasures.h"
#include "tg_sectors.h"
#include "tg_sidstars.h"
#include "tg_waypoints.h"

#include "Pilot.h"
#include "ECEF.h"
#include "pub_clearance_aircraft.h"

#include "pub_logger.h"

#include "pub_trajectory.h"
#include "pub_WaypointNode.h"

#include <omp.h>

#include <cmath>
#include <cstdio>
#include <cstdlib>

#include <dirent.h>
#include <fstream>
#include <iomanip>
#include <string>

#include <sys/stat.h>
#include <sys/time.h>

#include <thread>

#include <unistd.h>

#include "adb.h"
#include "NatsWaypoint.h"
#include "util_string.h"
#include "util_time.h"

using namespace std;
using namespace osi;

#define stringify(name) #name

#define BLOCK_SIZE              256
#if USE_GPU
#define NUM_STREAMS             4
#else
#define NUM_STREAMS             4
#endif

//TODO :PARIKSHIT'S DEFINITIONS
#define ENF_HDG_CONT_AIRBORNE 1
#if ENF_HDG_CONT_AIRBORNE
#define ENF_HDG_CONT_DBG 0
#define IDXVAL 9
#endif

#define FEET_TO_METERS          0.3048
#define RADIUS_EARTH_FT         20925524.9
#define PI                      3.14159265359

#define ENABLE_DEBUG_TRAJ_FILE  0
#define ENABLE_PROFILER         0

//TODO:PARIKSHIT'S DEFNIITIONS FOR NATS BEGIN
#define CDNR_FLAG 0
#define DETECTION_THRESHOLD_NM 50
#define RESOLUTION_THRESHOLD_NM 30

#define CLEARANCE_BUFFER_ALT_RANGE_FT 1000
#define REQUEST_TOUCHDOWN_CLEARANCE_ALT_FT 700
#define DETECT_MIN_ALT_FT 1000
#define RESOLVE_MIN_ALT_FT 500
#define MIN_ALT_FOR_CDNR_FT 1000
#define REF_LAT 37.615223
#define REF_LON -122.389977
#define METER_TO_NAUTICAL_MILE 0.000539957
#define NauticalMilestoFeet 6076.12
#define NauticalMilestoLatDeg 0.01666666666
#define NauticalMilestoLonDeg 0.01445671659 //Actually this value is not constant max at equator and
											// zero at poles. Here for simplicity we take max value
#define KnotsToFps 1.68780986
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define MIN_SPEED_COEFFICIENT_TAKEOFF 1.2
#define MIN_SPEED_COEFFICIENT_NOT_TAKEOFF 1.3
//TODO:PARIKSHIT'S DEFINITIONS FOR NATS ENDS

#if ENABLE_PROFILER
#include "cuda_profiler_api.h"
#endif

long sim_id;

bool* ground_departing_data_init; // Array of boolean flag indicating if the ground departing data is loaded
bool* ground_landing_data_init; // Array of boolean flag indicating if the ground landing data is loaded

//Initialization from extern in header file
float t_start = 0;
float t_end = 0;
float t_step = 0;
float t_step_terminal = 10; // Time step for terminal area simulation
float t_data_collection_period_airborne = 1;

const double distance_detect = 30;
const double distance_resolve = 50;

const string STRING_TAKEOFF = "TAKEOFF";
const string STRING_LANDING = "LANDING";

//Data structure to record flight phases to be skipped by pilot <Flight Index, Flight Phase>
std::map<int, string> skipFlightPhase;

//Data structure to record lag parameter values for pilot/controller
std::map<int, std::vector<string> > lagParams;
std::map<int, std::map<string, std::vector<float> > > lagParamValues;

//Data structure to record lag parameter values for ground vehicles
std::map< string, std::vector<string> > gvLagParams;
std::map< string, std::map<string, std::vector<float> > > gvLagParamValues;

//Data structure to record ground operator absence, and the vehicle stops for those many time steps
std::map< string, int > groundOperatorAbsence;

//Data structure to record ground operator repeat operations
std::map< string, vector <string> > groundOperatorActionRepeat;

//Data structure to record meter fixes for Merge-Space operations
std::map<string, std::vector<string> > meterFixMap;
std::map<string, std::vector< map <string, int> > > meterFixAircraftMap;

//Data structure to record spacing distance for meter fixes
std::map< string, map <string, pair < string, float> > > spacingDistMap;

//Data structure for touchdown and takeoff points of an aircraft
// Here, the takeoff and touchdown points are only the user-preferred points.  Simulation logic will try to takeoff/touchdown at the point but it may not be guaranteed.
std::map< string, map <string, pair <double, double> > > aircraftRunwayData;

//Data structure to aircraft and cargo cost within simulation
std::map< string, map <string, double> > aircraftAndCargoData;

//Data structure to store ground vehicle simulation data to be written
std::map< string, std::vector<string> > groundVehicleSimulationData;

//Data structure to handle ground operator course change for ground vehicle simulation
std::vector<string> courseChange;

//Data structure to store ground communication instrument error
std::map< string, pair <string, double> > radarErrorModel;

std::map<string, pair<string, float>> map_CDR_status;

//Region of regard for given aircraft
std::map< string, std::vector<vector<double>> > regionOfRegard;

//Weather sample for simulation
std::vector<double> weatherSample;

//Time step count for controller absence
int controllerAway = 0;
bool angleConverging;
float defaultSpeed = 0.0;
float defaultCourse = 0.0;
float defaultRocd = 0.0;

//Data structure for hold pattern
__device__ double holdPattern[9][2];

float time_step_surface_realTime_simulation; // Seconds
float time_step_terminal_realTime_simulation; // Seconds
float time_step_airborne_realTime_simulation; // Seconds
float pause_duration_realTime_simulation = 5000 * 1000; // micro seconds
float sleep_duration_realTime_simulation = 1000 * 1000; // micro seconds

bool flag_realTime_simulation = false;
timeval timeval_realTime_simulation_synchronized;
long nextPropagation_utc_time_realTime_simulation;

stringstream cdnr_oss_doc;
int cnt_event_cdnr = 0;

bool flag_enable_strategic_weather_avoidance = false;
bool flag_enable_cdnr = false;

float cdr_initiation_distance_ft_surface = 600.0;
float cdr_initiation_distance_ft_terminal = 20 * NauticalMilestoFeet;//10 * NauticalMilestoFeet;
float cdr_initiation_distance_ft_enroute = 20 * NauticalMilestoFeet;//10 * NauticalMilestoFeet;
float cdr_separation_distance_ft_surface = 300.0;
float cdr_separation_distance_ft_terminal = 7 * NauticalMilestoFeet;//3 * NauticalMilestoFeet;
float cdr_separation_distance_ft_enroute = 10 * NauticalMilestoFeet;//5 * NauticalMilestoFeet;

string tg_pathFilename_polygon;
string tg_pathFilename_sigmet;
string tg_pathFilename_pirep;

vector<WeatherWaypoint> vector_tactical_weather_waypoint;

// flight state propagation prototypes
__global__ void update_states(const int num_flights, const long t, const long t_step, const long t_step_terminal, const long t_step_airborne, const int stream_id, const int thread_id, const bool flag_proc_airborne_trajectory);

// device utility function prototypes
__device__ real_t compute_distance_gc(const real_t& lat1, const real_t& lon1, const real_t& lat2, const real_t& lon2, const real_t& alt);

// adb performance prototypes
__device__ void   compute_adb_indices(const short& fl_lo, const int& model_index, int* const index_lo, int* const index_hi);

// ruc/rap wind lookup prototypes
__device__ int    get_ruc_index(const int& i, const int& j, const int& k);

// sector lookup prototypes
__device__ bool   polygon_contains(const real_t* const poly_x, const real_t* const poly_y, const int& num_points, const real_t& x, const real_t& y);
__device__ bool   sector_contains(const real_t& lat_deg, const real_t& lon_deg, const real_t& alt_ft, const int& sector_index);
__device__ int    get_sector_grid_index(const int& i, const int& j, const int& k);
__device__ int    get_sector_grid_index(const int& i, const int& j, const int& k, const int& l);
int compute_flight_sector(const real_t& lat_deg, const real_t& lon_deg, const real_t& alt_ft, const int& cur_sector_index);

double trunc_double(double val, int digits) {
    double temp = 0.0;

    temp = (int) (val * pow(10,digits));

    temp /= pow(10,digits);

    return temp;
}

/*
 * Function to calculate bearing angle (in degrees) between to (lat,lon) points.
 */
float calculateBearing(float lat1, float lon1, float lat2, float lon2) {
	float longitude1, longitude2, latitude1, latitude2, longitudeDifference, x, y, bearingAngle;

	longitude1 = lon1;
	longitude2 = lon2;
	longitudeDifference = longitude2-longitude1;
	y = sin(longitudeDifference)*cos(lat2);
	x = cos(lat1)*sin(lat2)-sin(lat1)*cos(lat2)*cos(longitudeDifference);
	bearingAngle = fmod(((atan2(y, x) * 180.0 / M_PI)+360), 360.0);
	return bearingAngle;
}

//The 2D array with hold pattern waypoints is stored in holdPattern[][], starting with the entry point and goes in clockwise order.
__device__ void getHoldPattern(double entryPointLatitude, double entryPointLongitude, double currCourse) {
	double course = (currCourse) * M_PI / 180.0;
	float tempLat, tempLon;
	double distanceToRadius = ((13200 * 0.0003048) / (6378.1));

	/*
	 * Calculation for each point on the holding pattern ellipse, length of 5 miles and breadth of 2.5 miles.
	 *
	 * Holding pattern point format:
	 *
	 *  2  3
	 *1      4
	 *
	 *0
	 *
	 *8      5
	 *  7  6
	 */

	holdPattern[0][0] = entryPointLatitude * M_PI / 180.0;
	holdPattern[0][1] = entryPointLongitude * M_PI / 180.0;

	holdPattern[1][0] = (asin(sin(holdPattern[0][0]) * cos(distanceToRadius) + cos(holdPattern[0][0]) * sin(distanceToRadius) * cos(course)));
	holdPattern[1][1] = (holdPattern[0][1] + atan2(sin(course) * sin(distanceToRadius) * cos(holdPattern[0][0]), cos(distanceToRadius) - sin(holdPattern[0][0]) * sin(holdPattern[1][0])));

	holdPattern[8][0] = (asin(sin(holdPattern[0][0]) * cos(distanceToRadius) + cos(holdPattern[0][0]) * sin(distanceToRadius) * cos(course + M_PI)));
	holdPattern[8][1] = (holdPattern[0][1] + atan2(sin(course + M_PI) * sin(distanceToRadius) * cos(holdPattern[0][0]), cos(distanceToRadius) - sin(holdPattern[0][0]) * sin(holdPattern[8][0])));

	tempLat = (asin(sin(holdPattern[0][0]) * cos(distanceToRadius) + cos(holdPattern[0][0]) * sin(distanceToRadius) * cos(course + (M_PI / 2))));
	tempLon = (holdPattern[0][1] + atan2(sin(course + (M_PI / 2)) * sin(distanceToRadius) * cos(holdPattern[0][0]), cos(distanceToRadius) - sin(holdPattern[0][0]) * sin(tempLat)));

	holdPattern[4][0] = (asin(sin(tempLat) * cos(distanceToRadius) + cos(tempLat) * sin(distanceToRadius) * cos(course)));
	holdPattern[4][1] = (tempLon + atan2(sin(course) * sin(distanceToRadius) * cos(tempLat), cos(distanceToRadius) - sin(tempLat) * sin(holdPattern[4][0])));

	holdPattern[5][0] = (asin(sin(tempLat) * cos(distanceToRadius) + cos(tempLat) * sin(distanceToRadius) * cos(course + M_PI)));
	holdPattern[5][1] = (tempLon + atan2(sin(course + M_PI) * sin(distanceToRadius) * cos(tempLat), cos(distanceToRadius) - sin(tempLat) * sin(holdPattern[5][0])));

	tempLat = (asin(sin(holdPattern[0][0]) * cos(distanceToRadius / 4) + cos(holdPattern[0][0]) * sin(distanceToRadius / 4) * cos(course + (M_PI / 2))));
	tempLon = (holdPattern[0][1] + atan2(sin(course + (M_PI / 2)) * sin(distanceToRadius / 4) * cos(holdPattern[0][0]), cos(distanceToRadius / 4) - sin(holdPattern[0][0]) * sin(tempLat)));

	holdPattern[2][0] = (asin(sin(tempLat) * cos(distanceToRadius * 1.25) + cos(tempLat) * sin(distanceToRadius * 1.25) * cos(course)));
	holdPattern[2][1] = (tempLon + atan2(sin(course) * sin(distanceToRadius * 1.25) * cos(tempLat), cos(distanceToRadius * 1.25) - sin(tempLat) * sin(holdPattern[2][0])));

	holdPattern[7][0] = (asin(sin(tempLat) * cos(distanceToRadius * 1.25) + cos(tempLat) * sin(distanceToRadius * 1.25) * cos(course + M_PI)));
	holdPattern[7][1] = (tempLon + atan2(sin(course + M_PI) * sin(distanceToRadius * 1.25) * cos(tempLat), cos(distanceToRadius * 1.25) - sin(tempLat) * sin(holdPattern[7][0])));

	tempLat = (asin(sin(holdPattern[0][0]) * cos(3 * distanceToRadius / 4) + cos(holdPattern[0][0]) * sin(3 * distanceToRadius / 4) * cos(course + (M_PI / 2))));
	tempLon = (holdPattern[0][1] + atan2(sin(course + (M_PI / 2)) * sin(3 * distanceToRadius / 4) * cos(holdPattern[0][0]), cos(3 * distanceToRadius / 4) - sin(holdPattern[0][0]) * sin(tempLat)));

	holdPattern[3][0] = (asin(sin(tempLat) * cos(distanceToRadius * 1.25) + cos(tempLat) * sin(distanceToRadius * 1.25) * cos(course)));
	holdPattern[3][1] = (tempLon + atan2(sin(course) * sin(distanceToRadius * 1.25) * cos(tempLat), cos(distanceToRadius * 1.25) - sin(tempLat) * sin(holdPattern[3][0])));

	holdPattern[6][0] = (asin(sin(tempLat) * cos(distanceToRadius * 1.25) + cos(tempLat) * sin(distanceToRadius * 1.25) * cos(course + M_PI)));
	holdPattern[6][1] = (tempLon + atan2(sin(course + M_PI) * sin(distanceToRadius * 1.25) * cos(tempLat), cos(distanceToRadius * 1.25) - sin(tempLat) * sin(holdPattern[6][0])));

	// At the end here, the global variable holdPattern has been populated with point locations for the holding pattern.
}


bool pilot_check_clearance_aircraft(const int t,
		const int index_flight,
		const HumanErrorEvent_t error_event);

update_states_t** array_update_states_ptr; // Array to hold all update_states pointer for all aircraft

real_t calculate_dh_desired(const real_t target_altitude_ft, const real_t altitude_ft);

real_t compute_hdot_slope(const short adb_fl_lo,
		const short adb_fl_hi,
        const short adb_rod_fpm_lo,
		const short adb_rod_fpm_hi);

real_t compute_tas_slope(const short adb_fl_lo,
		const short adb_fl_hi,
        const short adb_vtas_knots_lo,
		const short adb_vtas_knots_hi);

real_t compute_fpa_rad(const real_t tas_knots, const real_t rocd_fps);

real_t compute_heading_gc(const real_t& lat1, const real_t& lon1, const real_t& lat2, const real_t& lon2);

real_t compute_lat_dot(const real_t hdg_rad, const real_t tas_knots, const real_t fpa_rad, const real_t wind_north_fps, const real_t wind_east_fps, const real_t altitude_ft);

real_t compute_lon_dot(const real_t hdg_rad, const real_t tas_knots, const real_t fpa_rad, const real_t wind_north_fps, const real_t wind_east_fps, const real_t altitude_ft, const real_t latitude);

// todo:parikshit adder
//__device__ bool   is_out_of_bounds(const real_t& lat1, const real_t& lon1, const real_t lat2, const real_t& lon2, const real_t& plat, const real_t& plon );
__device__ double conv_tas_to_cas_nom(const double& tas );
__device__ void   compute_x_y(const double &lat, const double &lon, double &x, double &y);
__device__ float conformal_latitude(const float& lat);
//todo:parikshit adder ends

#if CDNR_FLAG
//TODO:PARIKSHIT ADDERS FOR NATS
static inline real_t RN(real_t lat){
	return SEMI_MAJOR_EARTH_FT/sqrt(1-ECCENTRICITY_SQ_EARTH*sin(lat*M_PI/180)*sin(lat*M_PI/180) );
}
//SFO is the reference in ECEF coordinates
static inline real_t X_ref(){
	return RN( REF_LAT ) * cos( REF_LAT * M_PI/180) * cos( REF_LON * M_PI/180);}
static inline real_t Y_ref(){
	return RN( REF_LAT ) * cos( REF_LAT * M_PI/180) * sin( REF_LON * M_PI/180);}
static inline real_t Z_ref(){
	return ( ( 1 - ECCENTRICITY_SQ_EARTH ) * RN( REF_LAT ) ) * sin( REF_LAT * M_PI/180);}

//JUST FUNCTION DEFINITIONS
static void ECEFtoNED(real_t* const X, real_t* const Y, real_t* const Z,
		real_t* x, real_t* y, real_t* z);
static void NEDtoECEF(real_t* const x, real_t* const y, real_t* const z,
		real_t* X, real_t* Y, real_t* Z);
static void LatLontoNED(real_t* const lat, real_t* const lon, const real_t* const alt,
					real_t* x, real_t* y, real_t* z);
static void NEDtoLatLon(real_t* const x, real_t* const y, real_t* const z,
					real_t* lat, real_t* lon, real_t* h);
//TODO:PARIKSHIT ADDERS ENDS
#endif

static int 	copy_char_arrays(char**& dst, char**& src, int sz);

// private host prototypes
static void traj_data_callback(cuda_stream_t& stream, void* data, const long& t);
int set_device_sector_pointers();
static int  set_device_ac_pointers();
static int  set_device_adb_pointers();

////////////////////////////////////////////////////////////////////////////////
// Device function impl                                                       //
////////////////////////////////////////////////////////////////////////////////

/**
 * Get altitude of a waypoint
 *
 * This function check the existence of alt_1 and alt_2 value of the waypoint and calculate the returned value.
 */
float get_waypoint_altitude(waypoint_node_t* waypoint_node_ptr) {
	float retValue = 0.0;

	if (waypoint_node_ptr != NULL) {
		if (waypoint_node_ptr->alt_1 > 0) {
			retValue = waypoint_node_ptr->alt_1;
		}
		if (waypoint_node_ptr->alt_2 > 0) {
			if (retValue > 0) {
				retValue = (retValue + waypoint_node_ptr->alt_2) / 2;
			} else {
				retValue = waypoint_node_ptr->alt_2;
			}
		}
	}

	return retValue;
}

/**
 * Calculate the distance along the route from waypoint A to B
 */
double compute_route_distance_between_waypoints(waypoint_node_t* from_waypoint_ptr, waypoint_node_t* to_waypoint_ptr) {
    double retValue = -1;

    waypoint_node_t* tmpWaypoint_ptr;

    if ((from_waypoint_ptr != NULL) && (to_waypoint_ptr != NULL)) {
    	retValue = 0;

        tmpWaypoint_ptr = from_waypoint_ptr;
        while ((tmpWaypoint_ptr != to_waypoint_ptr) && (tmpWaypoint_ptr->next_node_ptr != NULL)) {
			retValue = retValue + compute_distance_gc(tmpWaypoint_ptr->latitude,
				tmpWaypoint_ptr->longitude,
				tmpWaypoint_ptr->next_node_ptr->latitude,
				tmpWaypoint_ptr->next_node_ptr->longitude,
				tmpWaypoint_ptr->altitude_estimate);

            tmpWaypoint_ptr = tmpWaypoint_ptr->next_node_ptr;
        }
    }

    return retValue;
}

void debug_list_all_waypoint_nodes(const int index_flight) {
	logger_printLog(LOG_LEVEL_DEBUG, 0, "\n");

	logger_printLog(LOG_LEVEL_DEBUG, 2, "debug_list_all_waypoint_nodes() --> index_flight = %d, acid = %s ************\n", index_flight, g_trajectories.at(index_flight).callsign.c_str());

	if (array_Airborne_Flight_Plan_ptr[index_flight] != NULL) {
		waypoint_node_t* tmpWaypoint_Node_ptr = array_Airborne_Flight_Plan_ptr[index_flight];
		while (tmpWaypoint_Node_ptr != NULL) {
			if (!h_aircraft_soa.flag_geoStyle[index_flight]) {
				logger_printLog(LOG_LEVEL_DEBUG, 9, "debug_list_all_waypoint_nodes() --> Airborne wpname = %s, proctype = %s, lat/lon = %f , %f, alt1 = %f, alt2 = %f, altitude_estimate = %f, speed_lim = %f, course_rad_to_next_node = %f\n", tmpWaypoint_Node_ptr->wpname, tmpWaypoint_Node_ptr->proctype, tmpWaypoint_Node_ptr->latitude, tmpWaypoint_Node_ptr->longitude, tmpWaypoint_Node_ptr->alt_1, tmpWaypoint_Node_ptr->alt_2, tmpWaypoint_Node_ptr->altitude_estimate, tmpWaypoint_Node_ptr->speed_lim, tmpWaypoint_Node_ptr->course_rad_to_next_node);
			} else {
				logger_printLog(LOG_LEVEL_DEBUG, 9, "debug_list_all_waypoint_nodes() --> Airborne wpname = %s, phase = %s, lat/lon = %f , %f, altitude_estimate = %f, speed_lim = %f, course_rad_to_next_node = %f\n", tmpWaypoint_Node_ptr->wpname, tmpWaypoint_Node_ptr->phase, tmpWaypoint_Node_ptr->latitude, tmpWaypoint_Node_ptr->longitude, tmpWaypoint_Node_ptr->altitude_estimate, tmpWaypoint_Node_ptr->speed_lim, tmpWaypoint_Node_ptr->course_rad_to_next_node);
			}

			tmpWaypoint_Node_ptr = tmpWaypoint_Node_ptr->next_node_ptr;
		}
	}
}

/**
 * Insert a new waypoint in the airborne flight plan waypoint list
 */

int insert_airborne_waypointNode(const int index_flight,
		const int waypoint_index_to_insert,
		const string waypoint_type,
		const string waypoint_name,
		const float waypoint_latitude,
		const float waypoint_longitude,
		const float waypoint_altitude,
		const float waypoint_speed_lim,
		const string waypoint_spdlim_desc) {
	int retValue = 1;

	if (-1 < index_flight) {
		waypoint_node_t* tmpWaypoint_node_ptr = array_Airborne_Flight_Plan_ptr[index_flight];
		if (tmpWaypoint_node_ptr != NULL) {
			int index_waypoint = 0;

			// Traverse to the waypoint node which a new waypoint will be inserted in front
			while ((tmpWaypoint_node_ptr != NULL) && (index_waypoint < waypoint_index_to_insert)) {
				tmpWaypoint_node_ptr = tmpWaypoint_node_ptr->next_node_ptr;
				index_waypoint++;
			}

			waypoint_node_t* newWaypoint_node_ptr = (waypoint_node_t*)calloc(1, sizeof(waypoint_node_t));
			newWaypoint_node_ptr->wpname = (char*)calloc(waypoint_name.length()+1, sizeof(char));
			strcpy(newWaypoint_node_ptr->wpname, waypoint_name.c_str());
			newWaypoint_node_ptr->wpname[waypoint_name.length()] = '\0';

			newWaypoint_node_ptr->proctype = (char*)calloc(waypoint_type.length()+1, sizeof(char));
			strcpy(newWaypoint_node_ptr->proctype, waypoint_type.c_str());
			newWaypoint_node_ptr->proctype[waypoint_type.length()] = '\0';

			newWaypoint_node_ptr->latitude = waypoint_latitude;
			newWaypoint_node_ptr->longitude = waypoint_longitude;
			newWaypoint_node_ptr->altitude_estimate = waypoint_altitude;

			newWaypoint_node_ptr->speed_lim = waypoint_speed_lim;

			newWaypoint_node_ptr->spdlim_desc = (char*)calloc(waypoint_spdlim_desc.length()+1, sizeof(char));
			strcpy(newWaypoint_node_ptr->spdlim_desc, waypoint_spdlim_desc.c_str());
			newWaypoint_node_ptr->spdlim_desc[waypoint_spdlim_desc.length()] = '\0';

			newWaypoint_node_ptr->prev_node_ptr = NULL; // Reset
			newWaypoint_node_ptr->next_node_ptr = NULL; // Reset

			if (waypoint_index_to_insert == 0) { // Insert new waypoint as the first node
				newWaypoint_node_ptr->next_node_ptr = tmpWaypoint_node_ptr;
				tmpWaypoint_node_ptr->prev_node_ptr = newWaypoint_node_ptr;

				array_Airborne_Flight_Plan_ptr[index_flight] = newWaypoint_node_ptr;
			} else if (waypoint_index_to_insert >= array_Airborne_Flight_Plan_Waypoint_length[index_flight]) { // Insert new waypoint as the last node
				array_Airborne_Flight_Plan_Final_Node_ptr[index_flight]->next_node_ptr = newWaypoint_node_ptr;
				newWaypoint_node_ptr->prev_node_ptr = array_Airborne_Flight_Plan_Final_Node_ptr[index_flight];

				array_Airborne_Flight_Plan_Final_Node_ptr[index_flight] = newWaypoint_node_ptr;
			} else {
				newWaypoint_node_ptr->prev_node_ptr = tmpWaypoint_node_ptr->prev_node_ptr;
				newWaypoint_node_ptr->next_node_ptr = tmpWaypoint_node_ptr;
				tmpWaypoint_node_ptr->prev_node_ptr->next_node_ptr = newWaypoint_node_ptr;
				tmpWaypoint_node_ptr->prev_node_ptr = newWaypoint_node_ptr;
			}

			// Add length of waypoint by 1
			array_Airborne_Flight_Plan_Waypoint_length[index_flight] = array_Airborne_Flight_Plan_Waypoint_length[index_flight] + 1;

			retValue = 0;
		}
	}

	return retValue;
}

/**
 * Delete a waypoint in the flight plan waypoint list
 *
 * After deleting, there is no post-processing on other waypoints
 */
int delete_airborne_waypointNode(const int index_flight,
		const int waypoint_index_to_delete) {
	int retValue = 1;

	if (-1 < index_flight) {
		waypoint_node_t* tmpWaypoint_node_ptr = array_Airborne_Flight_Plan_ptr[index_flight];
		if (tmpWaypoint_node_ptr != NULL) {
			int index_waypoint = 0;
			// Traverse to the waypoint node which the waypoint to be deleted
			while (index_waypoint < waypoint_index_to_delete) {
				tmpWaypoint_node_ptr = tmpWaypoint_node_ptr->next_node_ptr;
				index_waypoint++;
			}

			if (waypoint_index_to_delete == 0) { // The to-be-deleted waypoint is the first node
				array_Airborne_Flight_Plan_ptr[index_flight] = tmpWaypoint_node_ptr->next_node_ptr;

				tmpWaypoint_node_ptr->next_node_ptr->prev_node_ptr = NULL;
			} else if (waypoint_index_to_delete == (array_Airborne_Flight_Plan_Waypoint_length[index_flight] - 1)) { // The to-be-deleted waypoint is the last node
				array_Airborne_Flight_Plan_Final_Node_ptr[index_flight] = tmpWaypoint_node_ptr->prev_node_ptr;

				tmpWaypoint_node_ptr->prev_node_ptr->next_node_ptr = NULL;
			} else {
				tmpWaypoint_node_ptr->prev_node_ptr->next_node_ptr = tmpWaypoint_node_ptr->next_node_ptr;
				tmpWaypoint_node_ptr->next_node_ptr->prev_node_ptr = tmpWaypoint_node_ptr->prev_node_ptr;
			}

			releaseWaypointNodeContent(tmpWaypoint_node_ptr);

			// Subtract length of waypoint by 1
			array_Airborne_Flight_Plan_Waypoint_length[index_flight] = array_Airborne_Flight_Plan_Waypoint_length[index_flight] - 1;

			retValue = 0;
		}
	}

	return retValue;
}

real_t compute_hdot_slope(const short adb_fl_lo,
							const short adb_fl_hi,
		                    const short adb_rod_fpm_lo,
							const short adb_rod_fpm_hi) {
	short h_lo = adb_fl_lo * 100;
	short h_hi = adb_fl_hi * 100;
	short hdot_lo = adb_rod_fpm_lo / 60;
	short hdot_hi = adb_rod_fpm_hi / 60;
	real_t slope = (real_t)(hdot_hi - hdot_lo) / (real_t)(h_hi - h_lo);

	return slope; // output units: fps / ft
}

real_t compute_tas_slope(const short adb_fl_lo,
		const short adb_fl_hi,
        const short adb_vtas_knots_lo,
		const short adb_vtas_knots_hi) {
	short h_lo = adb_fl_lo * 100;
	short h_hi = adb_fl_hi * 100;
	short tas_lo = adb_vtas_knots_lo;
	short tas_hi = adb_vtas_knots_hi;
	real_t slope = (real_t)(tas_hi - tas_lo) / (real_t)(h_hi - h_lo);

	return slope; // output units: knots/ft
}

real_t compute_fpa_rad(const real_t tas_knots, const real_t rocd_fps) {
	return tas_knots == 0 ? 0 : asin(rocd_fps / (tas_knots*KNOTS_TO_FPS));
}

__device__ real_t compute_distance_gc(const real_t& lat1,
		const real_t& lon1,
		const real_t& lat2,
		const real_t& lon2,
		const real_t& alt) {
	real_t lat1_rad = lat1*PI/180.;
	real_t lon1_rad = lon1*PI/180.;
	real_t lat2_rad = lat2*PI/180.;
	real_t lon2_rad = lon2*PI/180.;

	real_t sin_half_dlat = sin( .5*(lat1_rad - lat2_rad) );
	real_t sin_half_dlon = sin( .5*(lon1_rad - lon2_rad) );
	real_t cos_lat1 = cos(lat1_rad);
	real_t cos_lat2 = cos(lat2_rad);
	real_t x = sin_half_dlat*sin_half_dlat + cos_lat1 * cos_lat2 * sin_half_dlon*sin_half_dlon;

	return 2*(RADIUS_EARTH_FT + alt)*asin(sqrt(x));
}

real_t compute_heading_gc(const real_t& lat1, const real_t& lon1, const real_t& lat2, const real_t& lon2) {
	real_t lat1_rad = lat1*PI/180.;
	real_t lon1_rad = lon1*PI/180.;
	real_t lat2_rad = lat2*PI/180.;
	real_t lon2_rad = lon2*PI/180.;

	real_t num = sin(lon2_rad - lon1_rad) * cos(lat2_rad);
	real_t den = sin(lat2_rad) * cos(lat1_rad) -
			sin(lat1_rad) * cos(lat2_rad) * cos(lon2_rad - lon1_rad);

	return atan2(num, den);
}

real_t compute_lat_dot(const real_t hdg_rad, const real_t tas_knots, const real_t fpa_rad, const real_t wind_north_fps, const real_t wind_east_fps, const real_t altitude_ft) {
	real_t cos_hdg = cos(hdg_rad);
	real_t sin_hdg = sin(hdg_rad);

	real_t update = tas_knots * KNOTS_TO_FPS * cos(fpa_rad) +
					wind_north_fps * cos_hdg +
					wind_east_fps * sin_hdg;

	return (update * cos_hdg) / (RADIUS_EARTH_FT + altitude_ft);
}

real_t compute_lon_dot(const real_t hdg_rad, const real_t tas_knots, const real_t fpa_rad, const real_t wind_north_fps, const real_t wind_east_fps, const real_t altitude_ft, const real_t latitude) {
	real_t cos_hdg = cos(hdg_rad);
	real_t sin_hdg = sin(hdg_rad);

	real_t update = tas_knots * KNOTS_TO_FPS * cos(fpa_rad) +
					wind_north_fps * cos_hdg +
					wind_east_fps * sin_hdg;

	return (update * sin_hdg) / ((RADIUS_EARTH_FT + altitude_ft) * 	cos(latitude*PI/180.));
}

__device__ void compute_adb_indices(const short& fl_lo,
		const int& model_index,
		update_states_t* update_states) {
	//if (!index_lo) return;
	if (!update_states) return;

	int lb_index = model_index * ADB_MAX_ROWS + floor((float)fl_lo /
			(float)ADB_PTF_ROW_SPACING);

	int adb_lower_bound_row = c_adb_lower_bound_row[lb_index];
	int adb_table_start_index = c_adb_table_start_index[model_index];
	int adb_num_rows = c_adb_num_rows[model_index];

	int end_index = adb_table_start_index + adb_num_rows - 1;

	update_states->ptf_index_lo = adb_table_start_index + adb_lower_bound_row - 1;

	if (update_states->ptf_index_lo < 0)
		update_states->ptf_index_lo = 0;

	if (update_states->ptf_index_lo > end_index)
		update_states->ptf_index_lo = end_index;

	update_states->ptf_index_hi = (update_states->ptf_index_lo >= end_index ? update_states->ptf_index_lo : update_states->ptf_index_lo+1);
}

__device__ bool polygon_contains(const real_t* const poly_x,
		                            const real_t* const poly_y,
		                            const int& num_points,
		                            const real_t& x, const real_t& y) {
	// use the ray-casting algorithm to determine if a point is inside
	// the polygon.
	real_t x0 = poly_x[0];
	real_t y0 = poly_y[0];
	real_t x_last = poly_x[num_points-1];
	real_t y_last = poly_y[num_points-1];

	int n = num_points;

	// determine if the polygon is opened or closed.
	if(x0 == x_last && y0 == y_last) {
		// closed
		--n;
	}

	int count = 0;

	real_t xi = x0;
	real_t yi = y0;
	real_t xi_plus1;
	real_t yi_plus1;
	real_t ucross;

	// check for crossing for all edges except closing edge
	for(int i=1; i<n; ++i) {
		xi_plus1 = poly_x[i];
		yi_plus1 = poly_y[i];
		if((yi < y && yi_plus1 >= y) || (yi >= y && yi_plus1 < y)) {
			ucross = xi - x - (yi-y)*(xi_plus1-xi) / (yi_plus1-yi);
			if(ucross > 0) ++count;
		}
		xi = xi_plus1;
		yi = yi_plus1;
	}

	// check for crossing for the closing edge
	if((yi < y && y0 >= y) || (yi >= y && y0 < y)) {
		ucross = xi - x - (yi - y)*(x0-xi) / (y0-yi);
		if(ucross > 0) ++count;
	}

	return (count & 1) != 0;
}

__device__ bool sector_contains(const real_t& lat_deg,
		                           const real_t& lon_deg,
		                           const real_t& alt_ft,
		                           const int& sector_index) {

	// aircraft lat/lon is in range (-180,180] while the sector boundaries
	// are in the range of [-360,0]...
	real_t alt = alt_ft;
	real_t lat = lat_deg;
	real_t lon = lon_deg;
	if(lon > 0) lon -= 360.;

	if ((sector_index >= 0) && (sector_index < g_sectors.size())) {
		// first check the bounding box of the current sector
		real_t lat_min = c_sector_array[sector_index].lat_min;
		real_t lat_max = c_sector_array[sector_index].lat_max;
		real_t lon_min = c_sector_array[sector_index].lon_min;
		real_t lon_max = c_sector_array[sector_index].lon_max;
		real_t alt_min = c_sector_array[sector_index].alt_min;
		real_t alt_max = c_sector_array[sector_index].alt_max;

		// if within the bounding box then check if the polygon
		// contains lat/lon
		if(lat >= lat_min && lat <= lat_max &&
		   lon >= lon_min && lon <= lon_max &&
		   alt >= alt_min && alt <= alt_max) {

			return polygon_contains(c_sector_array[sector_index].latitude,
					                c_sector_array[sector_index].longitude,
					                c_sector_array[sector_index].num_vertices,
					                lat, lon);
		}
	}

	// if we get here, then either the cur index is not set or the
	// bounding box didn't contain the flight, or the polygon
	// didn't contain the flight.
	return false;
}

__device__ int get_sector_grid_index(const int& i, const int& j, const int& k) {
	return SECTOR_GRID_ALT_SIZE * (i*SECTOR_GRID_LON_SIZE + j) + k;
}

__device__ int get_sector_grid_index(const int& i, const int& j, const int& k, const int& l) {
	return SECTOR_GRID_CELL_MAX_COUNT * (SECTOR_GRID_ALT_SIZE *
			(i*SECTOR_GRID_LON_SIZE + j) + k) + l;
}

// Compute sector
int compute_flight_sector(const real_t& lat_deg,
		                             	const real_t& lon_deg,
		                                const real_t& alt_ft,
		                                const int& cur_sector_index) {
	// first, if the cur sector is set, check to see if the flight is
	// still in the same sector.  if not, check all sectors in the grid cell.
	if (cur_sector_index >= 0) {
		if (sector_contains(lat_deg, lon_deg, alt_ft, cur_sector_index)) {
			return cur_sector_index;
		}
	}

	real_t lat = lat_deg;
	real_t lon = lon_deg;
	real_t alt = alt_ft;

	// the input longitude is in the range (-180,180]. we
	// need to put in in the range (-360,0].
	if (lon > 0) lon -= 360.;

	// compute the current sector grid cell indices
	int i = (int)(lat_deg / SECTOR_GRID_CELL_LAT_SIZE);
	int j = (int)((lon + 360.) / SECTOR_GRID_CELL_LON_SIZE);
	int k = (int)(alt_ft / SECTOR_GRID_CELL_ALT_SIZE);

	// check all sectors in the current grid cell
	if (i >= 0 && j >= 0 && k >= 0) {
		int cell_index = get_sector_grid_index(i, j, k);

		int cell_count = 0;
		// If cell_index less than the size of variable "c_sector_grid_cell_counts"
		if ((cell_index >= 0) && (cell_index < VAR_SECTOR_GRID_CELL_COUNTS_SIZE)) {
			cell_count = c_sector_grid_cell_counts[cell_index];

			for (int s=0; s<cell_count; ++s) {
				int grid_index = get_sector_grid_index(i, j, k, s);

				// If grid_index less than the size of variable "c_sector_grid"
				if (grid_index < VAR_SECTOR_GRID_SIZE) {
					int sector_index = c_sector_grid[grid_index];
					bool contains = sector_contains(lat, lon, alt, sector_index);
					if (contains) {
						// found a new sector in current grid cell
						return sector_index;
					}
				} else {
					break;
				}
			}
		}
	}

	// if we make it to here, then the flight was not found in any sector
	// in the grid cell. ideally, this should never happen. but it is
	// possible that the flight is in a 'hole' between sector boundaries.
	// we return -1 to indicate that no sector was found.
	return -1;
}

__device__ int get_ruc_index(const int& i, const int& j, const int& k) {
	return c_num_alt_cells * (i*c_num_lon_cells + j) + k;
}

void get_wind_field_components(const real_t& t,
		                                  const real_t& lat,
	                                      const real_t& lon,
		                                  const real_t& alt,
		                                  real_t* const wind_east,
		                                  real_t* const wind_north) {
	// check the bounding box
	if(lat < c_lat_min || lat > c_lat_max ||
	   lon < c_lon_min || lon > c_lon_max ||
	   alt < c_alt_min || alt > c_alt_max) {
		// out of bounds
		if(wind_east) *wind_east = 0;
		if(wind_north) *wind_north = 0;
		return;
	}

	// compute the offset into the wind arrays for the current sim time.
	// this assumes that the simulation starts at time 0.
	int hour = (int)(t / 3600.);
	int offset = c_num_lat_cells*c_num_lon_cells*c_num_alt_cells * hour;

	// compute the 1d ruc array index
	int i = (int)((lat - c_lat_min) / c_lat_step);
	int j = (int)((lon - c_lon_min) / c_lon_step);
	int k = (int)((alt - c_alt_min) / c_alt_step);
	int wind_index = offset + get_ruc_index(i, j, k);

	if ((wind_north) && (wind_index > -1))
		*wind_north = c_wind_north[wind_index] + c_wind_north_unc[wind_index];

	if ((wind_east) && (wind_index > -1))
		*wind_east = c_wind_east[wind_index] + c_wind_east_unc[wind_index];
}

/**
 * Load departing taxi plan data into [update_states] variable
 *
 * ground_departing_data_init[index_flight]: A flag to check if the taxi plan is loaded
 * When the data is loaded, set it to true
 */
void load_init_states_ground_departing(const int index_flight, update_states_t* update_states) {
	// Check if ground drive plan is loaded.  If not, load it.
	if ((!ground_departing_data_init[index_flight]) && (h_departing_taxi_plan.waypoint_length[index_flight] > 1)) {
		update_states->altitude_ft = c_origin_airport_elevation_ft[index_flight];

		update_states->target_altitude_ft = c_origin_airport_elevation_ft[index_flight];

		update_states->landed_flag = 0;
	}

	if (!h_aircraft_soa.flag_geoStyle[index_flight]) {
		// Get runway endpoint
		GroundWaypointConnectivity tmpGroundWaypointConnectivity = map_ground_waypoint_connectivity.at(g_trajectories.at(index_flight).origin_airport);
		map<string, AirportNode> tmpMap_waypoint_node = tmpGroundWaypointConnectivity.map_waypoint_node;

		map<string, AirportNode>::iterator iterator_map_wp;
		for (iterator_map_wp = tmpMap_waypoint_node.begin(); iterator_map_wp != tmpMap_waypoint_node.end(); iterator_map_wp++) {
			if (h_departing_taxi_plan.runway_name[index_flight] != NULL) {
				if ((iterator_map_wp->second.type1.find("Entry") != string::npos) && (iterator_map_wp->second.refName1.find(h_departing_taxi_plan.runway_name[index_flight]) != string::npos)) {
					update_states->departing_runway_entry_latitude = iterator_map_wp->second.latitude;
					update_states->departing_runway_entry_longitude = iterator_map_wp->second.longitude;
				} else if ((iterator_map_wp->second.type2.find("End") != string::npos) && (iterator_map_wp->second.refName2.find(h_departing_taxi_plan.runway_name[index_flight]) != string::npos)) {
					update_states->departing_runway_end_latitude = iterator_map_wp->second.latitude;
					update_states->departing_runway_end_longitude = iterator_map_wp->second.longitude;
				}
			}
		}
		// end - Get runway endpoint
	} else {
		if ((h_departing_taxi_plan.runway_entry_latitude_geoStyle[index_flight] != DBL_MAX) && (h_departing_taxi_plan.runway_entry_longitude_geoStyle[index_flight] != DBL_MAX)) {
			update_states->departing_runway_entry_latitude = h_departing_taxi_plan.runway_entry_latitude_geoStyle[index_flight];
			update_states->departing_runway_entry_longitude = h_departing_taxi_plan.runway_entry_longitude_geoStyle[index_flight];
		}
		if ((h_departing_taxi_plan.runway_end_latitude_geoStyle[index_flight] != DBL_MAX) && (h_departing_taxi_plan.runway_end_longitude_geoStyle[index_flight] != DBL_MAX)) {
			update_states->departing_runway_end_latitude = h_departing_taxi_plan.runway_end_latitude_geoStyle[index_flight];
			update_states->departing_runway_end_longitude = h_departing_taxi_plan.runway_end_longitude_geoStyle[index_flight];
		}
	}

	if ((update_states->departing_runway_entry_latitude != DBL_MAX) && (update_states->departing_runway_entry_longitude != DBL_MAX)
			&& (update_states->departing_runway_end_latitude != DBL_MAX) && (update_states->departing_runway_end_longitude != DBL_MAX)) {
		// Calculate runway heading
		update_states->course_rad_runway = compute_heading_rad_gc(update_states->departing_runway_entry_latitude,
			update_states->departing_runway_entry_longitude,
			update_states->departing_runway_end_latitude,
			update_states->departing_runway_end_longitude);
	}

	ground_departing_data_init[index_flight] = true;
}

/**
 * Load departing taxi plan data into [update_states] variable
 *
 * ground_landing_data_init[index_flight]: A flag to check if the taxi plan is loaded
 * When the data is loaded, set it to true
 */
void load_init_states_ground_landing(const int index_flight, update_states_t* update_states) {
	// Check if ground drive plan is loaded.  If not, load it.
	if ((!ground_landing_data_init[index_flight]) && (h_landing_taxi_plan.waypoint_length[index_flight] > 1)) {
		update_states->altitude_ft = c_destination_airport_elevation_ft[index_flight];

		update_states->target_altitude_ft = c_destination_airport_elevation_ft[index_flight];

		update_states->landed_flag = 0;
	}

	if (!h_aircraft_soa.flag_geoStyle[index_flight]) {
		// Get runway endpoint
		GroundWaypointConnectivity tmpGroundWaypointConnectivity = map_ground_waypoint_connectivity.at(g_trajectories.at(index_flight).destination_airport);
		map<string, AirportNode> tmpMap_waypoint_node = tmpGroundWaypointConnectivity.map_waypoint_node;

		map<string, AirportNode>::iterator iterator_map_wp;
		for (iterator_map_wp = tmpMap_waypoint_node.begin(); iterator_map_wp != tmpMap_waypoint_node.end(); iterator_map_wp++) {
			if ((iterator_map_wp->second.type1.find("Entry") != string::npos) && (iterator_map_wp->second.refName1.find(h_landing_taxi_plan.runway_name[index_flight]) != string::npos)) {
				update_states->landing_runway_entry_latitude = iterator_map_wp->second.latitude;
				update_states->landing_runway_entry_longitude = iterator_map_wp->second.longitude;
			} else if ((iterator_map_wp->second.type2.find("End") != string::npos) && (iterator_map_wp->second.refName2.find(h_landing_taxi_plan.runway_name[index_flight]) != string::npos)) {
				update_states->landing_runway_end_latitude = iterator_map_wp->second.latitude;
				update_states->landing_runway_end_longitude = iterator_map_wp->second.longitude;
			}
		}
		// end - Get runway endpoint
	} else {
		if ((h_landing_taxi_plan.runway_entry_latitude_geoStyle[index_flight] != DBL_MAX) && (h_landing_taxi_plan.runway_entry_longitude_geoStyle[index_flight] != DBL_MAX)) {
			update_states->landing_runway_entry_latitude = h_landing_taxi_plan.runway_entry_latitude_geoStyle[index_flight];
			update_states->landing_runway_entry_longitude = h_landing_taxi_plan.runway_entry_longitude_geoStyle[index_flight];
		}
		if ((h_landing_taxi_plan.runway_end_latitude_geoStyle[index_flight] != DBL_MAX) && (h_landing_taxi_plan.runway_end_longitude_geoStyle[index_flight] != DBL_MAX)) {
			update_states->landing_runway_end_latitude = h_landing_taxi_plan.runway_end_latitude_geoStyle[index_flight];
			update_states->landing_runway_end_longitude = h_landing_taxi_plan.runway_end_longitude_geoStyle[index_flight];
		}
	}

	if ((update_states->landing_runway_entry_latitude != DBL_MAX) && (update_states->landing_runway_entry_longitude != DBL_MAX)
			&& (update_states->landing_runway_end_latitude != DBL_MAX) && (update_states->landing_runway_end_longitude != DBL_MAX)) {
		update_states->course_rad_runway = compute_heading_rad_gc(update_states->landing_runway_entry_latitude,
			update_states->landing_runway_entry_longitude,
			update_states->landing_runway_end_latitude,
			update_states->landing_runway_end_longitude);
	}

	ground_landing_data_init[index_flight] = true;
}

__device__ float compute_mach_from_calibrated(float casIn, float hpIn) {
	double m, thetas, deltam;
	double cas = (double) casIn;
	double hp = (double) hpIn;

	if (hp < 36089.) {
		thetas = (1. - 6.87535E-6 * hp);
		deltam = pow(thetas, 5.2561);
	} else {
		thetas = .7519;
		deltam = .2234 * pow(2.718, ( -((hp - 36089.) / 20806.)));
	}

	m = sqrt(5. * ((pow((1. + ((
			pow((1. + 0.2 * (pow((cas / 661.5), 2))), 3.5)) - 1.) / deltam),
								  (2. / 7.))) - 1.));

	return ((float) m);
}

__device__ float compute_true_speed_from_calibrated
            (float dtk, /* use in sos calulation   */
             float aset, /* barometric pressure     */
             float alt, /* Current altitude        */
             float ias) /* Input value, IAS        */
            {
	/* from DHW */
	float hp; /* pressure altitude                (ft)       */
	float ta; /* ambient temperature at altitude  (degs K)   */
	float deltam; /* standard day pressure ratio                 */
	float thetas; /* standard day temperature ratio              */
	float sos; /* speed of sound                   (knots)    */
	float mach;
	float sign = 1.0f;
	float tas;

	/* compute standard day parameters */
	hp = alt - (aset - 29.92f) * 1000.f;
	if (hp < 36089.0f) {
		thetas = (1.f - 6.87535E-6f * hp);
		deltam = pow(thetas, 5.2561);
	} else {
		thetas = .7519f;
		deltam = .2234f * pow(2.718, ( -((hp - 36089.) / 20806.)));
	}

	/* compute speed of sound */
	ta = thetas * 288.15f + dtk;
	sos = 65.7703f * sqrt(ta) * .59249f;

	/* input was true */
	if (ias < 0.0f) {
		sign = -1.0f;
		ias = -ias;
	}

	mach = compute_mach_from_calibrated(ias, alt);
	tas = mach * sos;

	return tas;
}

__device__ float compute_true_speed_from_calibrated
            (double cas_knt, /* airspeed in knots. */
             double altitude) /* feet MSL */
            {
	float retValue = 0.0f;

	retValue = compute_true_speed_from_calibrated
			  (0, /* use in sos calulation   */
			   29.92f, /* barometric pressure     */
			   (float) altitude, /* Current altitude        */
			   (float) cas_knt); /* Input value, TAS        */

	return retValue;
}

__device__ double conv_tas_to_cas_nom(const double& tas ){
	double A0 = 661.4788;
	double P0 = 101.325e3;
	double rho0 = 1.225;
	double knots_to_mps = 0.514444;
	double q_c_by_p_0 = 0.5 * rho0 * tas * tas * knots_to_mps * knots_to_mps / P0;
	double cas_1 = 0.5 * ( pow( (q_c_by_p_0 + 1) , 2.0/7.0) - 1 );
	double cas = A0 * sqrt(cas_1);

	return cas;
}

__device__ float conformal_latitude(const float& geo) {
	float e = 0.081992;
	float num = pow( 1 - e * sin(geo) , e / 2.0 ) ;
	num = num * tan( geo/2.0 + M_PI/4.0 );
    float den = pow( 1 + e * sin(geo) , e / 2.0 ) ;

    float con = 2.0 * atan2(num,den) - M_PI/2.0;

    return (con);
}

__device__ void   compute_x_y(const double &lat, const double &lon,
							double &x, double &y){
	/*
	 * FIXME: HARD CODED VALUES. NEED TO CHANGE THEM IN FUTURE.
	 */
	float lat_stereo = 38.3294;
	float lon_stereo = 83.5569;
	float E0 = 3439.620850;
	float xt = 317.750000;
	float yt = 188.750000;
	/*
	 * TILL HERE
	 */
    float lambda0 = lon_stereo*M_PI/180;
    float phi0_prime = lat_stereo * M_PI/180;
    float phi0 = conformal_latitude(phi0_prime);
    float phi_prime = lat * M_PI/180;
    float lambda = lon * M_PI/180;
    float delta_lambda = lambda0 - lambda;
    float phi = conformal_latitude(phi_prime);

    float sin_delta_lambda = sin(delta_lambda);
    float cos_delta_lambda = cos(delta_lambda);

    float sin_phi = sin(phi);
    float cos_phi = cos(phi);

    float sin_phi0 = sin(phi0);
    float cos_phi0 = cos(phi0);

    float denom = (1 + sin_phi * sin_phi0 + cos_phi * cos_phi0 *
             cos_delta_lambda);
    float x0 = 2.0 * E0 * sin_delta_lambda * cos_phi / denom;
    float y0 = 2.0 * E0 *
         (sin_phi * cos_phi0 - cos_phi * sin_phi0 * cos_delta_lambda) / denom;

    x = (double) (x0 + xt);
    y = (double) (y0 + yt);
}

float calculate_elapsedSecond(const float value) {
	float retValue = -1; // Default

	if (value < 0) {
		retValue = 0;
	} else {
		retValue = value;
	}

	return retValue;
}

double get_CIFP_V(update_states_t* update_states,
		const int index_flight,
		const bool flagClimbing) {
	double retV = 0.0;

	int tmpAdb_aircraft_type_index = c_adb_aircraft_type_index[index_flight];
	AdbOPFModel tmpAdbOPFModel = g_adb_opf_models.at(tmpAdb_aircraft_type_index);

	double take_off_length;
	double vstall_take_off;
	double v2_tas_knots;

	double vA;
	double vB;
	double distance_A_to_Current;
	double distance_A_to_B;

	return retV;
}

float get_aiming_speed(update_states_t* update_states,
		const int index_flight,
		const bool flagClimbing) {
	float aiming_V = -1.0f; // The speed at the target waypoint that the aircraft is going to reach

	waypoint_node_t* tmpWaypoint_node_ptr = update_states->target_WaypointNode_ptr;

	int tmpAdb_aircraft_type_index = c_adb_aircraft_type_index[index_flight];
	AdbOPFModel tmpAdbOPFModel = g_adb_opf_models.at(tmpAdb_aircraft_type_index);
	AdbPTFModel tmpAdbPTFModel = g_adb_ptf_models.at(tmpAdb_aircraft_type_index);

	float tmpAltitude_waypoint = -1.0;

	if (flagClimbing) { // Climbing
		if ((tmpWaypoint_node_ptr != NULL) && (tmpWaypoint_node_ptr->speed_lim > 0.0)) {
			aiming_V = tmpWaypoint_node_ptr->speed_lim; // Get aiming speed from the target waypoint
			c_acceleration_aiming_waypoint_node_ptr[index_flight] = tmpWaypoint_node_ptr;
		}

		if (aiming_V <= 0.0) {
			aiming_V = c_cruise_tas_knots[index_flight];
			c_acceleration_aiming_waypoint_node_ptr[index_flight] = array_Airborne_Flight_Plan_toc_ptr[index_flight];
		}
	} else { // Descenting
		if ((tmpWaypoint_node_ptr != NULL) && (tmpWaypoint_node_ptr->speed_lim > 0.0)) {
			aiming_V = tmpWaypoint_node_ptr->speed_lim; // Get aiming speed from the target waypoint
			c_acceleration_aiming_waypoint_node_ptr[index_flight] = tmpWaypoint_node_ptr;
		}

		if (aiming_V <= 0.0) {
			if (update_states->target_WaypointNode_ptr != array_Airborne_Flight_Plan_Final_Node_ptr[index_flight]) {
				while ((tmpWaypoint_node_ptr != NULL) && (aiming_V <= 0.0)) {
					if (tmpWaypoint_node_ptr->speed_lim > 0.0) {
						aiming_V = tmpWaypoint_node_ptr->speed_lim;
						c_acceleration_aiming_waypoint_node_ptr[index_flight] = tmpWaypoint_node_ptr;
					} else {
						if (tmpWaypoint_node_ptr->next_node_ptr != NULL) {
							tmpWaypoint_node_ptr = tmpWaypoint_node_ptr->next_node_ptr;
						} else {
							break;
						}
					}
				}
			} else {
				aiming_V = tmpAdbOPFModel.vstall.at(LANDING);
				aiming_V = compute_true_speed_from_calibrated(MIN_SPEED_COEFFICIENT_NOT_TAKEOFF * aiming_V, 0);

				c_acceleration_aiming_waypoint_node_ptr[index_flight] = array_Airborne_Flight_Plan_Final_Node_ptr[index_flight];
			}
		}
	}

	return aiming_V;
}

float compute_acceleration(update_states_t* update_states,
		const int index_flight,
		const bool flagClimbing) {
	float acceleration = -1.0f; // Default

	c_acceleration_aiming_waypoint_node_ptr[index_flight] = NULL; // Reset

	float vInitial = update_states->tas_knots;
	float vAimingWaypoint = get_aiming_speed(update_states, index_flight, flagClimbing);

	if (vAimingWaypoint > 0) {
		if ((flagClimbing && (update_states->tas_knots_bak < vAimingWaypoint))
				||
			(!flagClimbing && (update_states->tas_knots_bak > vAimingWaypoint))) {
			double tmpDistance;

			if (update_states->target_WaypointNode_ptr != array_Airborne_Flight_Plan_Final_Node_ptr[index_flight]) {
				tmpDistance = compute_distance_gc(update_states->lat,
								update_states->lon,
								c_acceleration_aiming_waypoint_node_ptr[index_flight]->latitude,
								c_acceleration_aiming_waypoint_node_ptr[index_flight]->longitude,
								update_states->altitude_ft);
			} else {
				tmpDistance = compute_distance_gc(update_states->lat,
								update_states->lon,
								c_estimate_touchdown_point_latitude_deg[index_flight],
								c_estimate_touchdown_point_longitude_deg[index_flight],
								update_states->altitude_ft);
			}

			acceleration = abs(pow(vAimingWaypoint, 2) - pow(vInitial, 2)) / (2 * tmpDistance);
		}
	}

	return acceleration;
}

void obtain_go_around_waypoint(update_states_t* update_states,
		const int index_flight) {
	if (update_states->go_around_WaypointNode_ptr == NULL) {
		if (!h_aircraft_soa.flag_geoStyle[index_flight]) { // Not geoStyle
			string tmpWaypoint_go_around;
			string tmpAltitude_go_around;

			bool flag_found_go_around_waypoint = false;

			FlightPlan curFlightPlan = g_flightplans.at(index_flight);

			vector<PointWGS84> curRoute = curFlightPlan.route;

			// Check all airborne waypoints.  Find the first APPROACH one.
			for (int i = 0; i < curRoute.size(); i++) {
				if (strcmp("APPROACH", curRoute.at(i).proctype.c_str()) == 0) {
					// Read file: ApproachHoldingPoints.csv
					ifstream in;
					string read_line;

					in.open("share/tg/ApproachHoldingPoints.csv");
					if (!in.is_open()) {
						printf("Error: share/tg/ApproachHoldingPoints.csv not found\n");

						return;
					}

					while (in.good()) {
						read_line = "";
						getline(in, read_line);
						if (strlen(read_line.c_str()) == 0) continue;

						char* entry;

						char* tempCharArray = (char*)calloc((read_line.length()+1), sizeof(char));
						strcpy(tempCharArray, read_line.c_str());

						// Retrieve individual data split by "," sign
						entry = strtok(tempCharArray, ","); // First entry

						// If airport code does not match
						if (g_trajectories.at(index_flight).destination_airport.find(entry) == string::npos) {
							free(tempCharArray);

							continue;
						}

						entry = strtok(NULL, ","); // 2nd entry

						if (curRoute.at(i).procname.find(entry) == string::npos) {
							free(tempCharArray);

							continue;
						}

						entry = strtok(NULL, ","); // 3rd entry
						tmpWaypoint_go_around = "";
						tmpWaypoint_go_around.assign(entry); // Set name of the Go-Around waypoint

						entry = strtok(NULL, ","); // 4th entry
						tmpAltitude_go_around = "";
						tmpAltitude_go_around.assign(entry);

						flag_found_go_around_waypoint = true;

						free(tempCharArray);

						if (flag_found_go_around_waypoint)
							break;
					}

					in.close();

					break;
				}
			} // end - for

			if (flag_found_go_around_waypoint) {
				NatsWaypoint tmpNatsWaypoint;
				tmpNatsWaypoint.name = tmpWaypoint_go_around;

				vector<NatsWaypoint>::iterator ite_waypoint = find(g_waypoints.begin(), g_waypoints.end(), tmpNatsWaypoint);
				if (ite_waypoint != g_waypoints.end()) {
					update_states->go_around_WaypointNode_ptr = (waypoint_node_t*)calloc(1, sizeof(waypoint_node_t));
					update_states->go_around_WaypointNode_ptr->prev_node_ptr = NULL;
					update_states->go_around_WaypointNode_ptr->next_node_ptr = NULL;

					update_states->go_around_WaypointNode_ptr->wpname = (char*)calloc(tmpWaypoint_go_around.length()+1, sizeof(char));
					strcpy(update_states->go_around_WaypointNode_ptr->wpname, tmpWaypoint_go_around.c_str());
					update_states->go_around_WaypointNode_ptr->wpname[tmpWaypoint_go_around.length()] = '\0';

					update_states->go_around_WaypointNode_ptr->latitude = ite_waypoint->latitude;
					update_states->go_around_WaypointNode_ptr->longitude = ite_waypoint->longitude;

					update_states->go_around_WaypointNode_ptr->proctype = (char*)calloc(strlen("GOAROUND")+1, sizeof(char));
					strcpy(update_states->go_around_WaypointNode_ptr->proctype, "GOAROUND");
					update_states->go_around_WaypointNode_ptr->proctype[strlen("GOAROUND")] = '\0';

					update_states->go_around_WaypointNode_ptr->altitude_estimate = atof(tmpAltitude_go_around.c_str());

					update_states->target_WaypointNode_ptr = update_states->go_around_WaypointNode_ptr; // Update new target waypoint

					update_states->S_within_flightPhase_and_simCycle = 0; // Reset

					update_states->flag_target_waypoint_change = false; // Reset

					update_states->go_around_split_point_latitude = update_states->lat;
					update_states->go_around_split_point_longitude = update_states->lon;
				}
			}
		}
	}
}

/**
 * Pilot module to compute latitude and longitude
 *
 * The algorithm is:
 * 1. Set current latitude and longitude as the start point
 * 2. Calculate the latitude and longitude of the end point using the course heading and the distance of the current time step.
 */
void pilot_compute_Lat_Lon_default_logic(update_states_t* update_states,
		const int index_flight) {
	double lat_rad_start;
	double lon_rad_start;
	double lat_rad_end;
	double lon_rad_end;

	lat_rad_start = update_states->lat * PI / 180.;
	lon_rad_start = update_states->lon * PI / 180.;

	lat_rad_end = asin( sin(lat_rad_start) * cos(update_states->S_within_flightPhase_and_simCycle / RADIUS_EARTH_FT) +
	        cos(lat_rad_start)* sin(update_states->S_within_flightPhase_and_simCycle / RADIUS_EARTH_FT) * cos(update_states->hdg_rad));

	lon_rad_end = lon_rad_start +
			atan2(sin(update_states->hdg_rad) * sin(update_states->S_within_flightPhase_and_simCycle / RADIUS_EARTH_FT) * cos(lat_rad_start),
	        cos(update_states->S_within_flightPhase_and_simCycle / RADIUS_EARTH_FT) - sin(lat_rad_start) * sin(lat_rad_end));

	update_states->lat = lat_rad_end * 180. / PI;
	update_states->lon = lon_rad_end * 180. / PI;
}

/**
 * Pilot module to compute latitude and longitude
 *
 * The logic will be split into multiple flight phases and processed individually.
 */
void pilot_compute_Lat_Lon_logic(update_states_t* update_states,
		const int index_flight) {
	double hdg_to_runway_end = 0.0;
	double distance_to_runway_end = 0.0;
	double runway_length_departing = 0.0;
	double runway_length_landing = 0.0;

	switch (update_states->flight_phase) {
		case FLIGHT_PHASE_RAMP_DEPARTING:
			if (c_hold_flight_phase[index_flight] != FLIGHT_PHASE_RAMP_DEPARTING) {
				pilot_compute_Lat_Lon_default_logic(update_states, index_flight);
			}

			break;

		case FLIGHT_PHASE_TAKEOFF:
			pilot_compute_Lat_Lon_default_logic(update_states, index_flight);

			if ((update_states->tas_knots > h_departing_taxi_plan.taxi_tas_knots[index_flight])
					&& (update_states->altitude_ft == g_trajectories.at(index_flight).origin_airport_elevation_ft)) {
				if ((h_departing_taxi_plan.runway_name[index_flight] != NULL) && (!update_states->flag_abnormal_on_runway)) {
					// Check if aircraft experiences abnormal event on the runway
					hdg_to_runway_end = compute_heading_rad_gc(update_states->lat,
								update_states->lon,
								update_states->departing_runway_end_latitude,
								update_states->departing_runway_end_longitude);

					distance_to_runway_end = compute_distance_gc(update_states->lat,
								update_states->lon,
								update_states->departing_runway_end_latitude,
								update_states->departing_runway_end_longitude,
								g_trajectories[index_flight].destination_airport_elevation_ft);

					runway_length_departing = compute_distance_gc(
								update_states->departing_runway_entry_latitude,
								update_states->departing_runway_entry_longitude,
								update_states->departing_runway_end_latitude,
								update_states->departing_runway_end_longitude,
								g_trajectories[index_flight].origin_airport_elevation_ft);
					if (RUNWAY_OFFSET_FT < distance_to_runway_end * sin(abs(hdg_to_runway_end - update_states->course_rad_runway))) {
						update_states->flag_abnormal_on_runway = true;
						update_states->flight_phase = FLIGHT_PHASE_OUTOFRUNWAY;
						printf("Aircraft %s deviate from the runway during departing\n", update_states->acid);
					} else  if ((abs(update_states->course_rad_runway - hdg_to_runway_end) * 180 / PI < 80)
									&& (runway_length_departing < distance_to_runway_end)) {
						update_states->flag_abnormal_on_runway = true;
						printf("Aircraft %s undershoot the runway during departing\n", update_states->acid);
					} else if (abs(update_states->hdg_rad - hdg_to_runway_end) >= (PI * 10 / 180)) {
						update_states->flag_abnormal_on_runway = true;
						update_states->flight_phase = FLIGHT_PHASE_RUNWAYOVERSHOOT;
						printf("Aircraft %s overshoot the runway during departing\n", update_states->acid);
					}
					// end - Check if aircraft experiences abnormal event on the runway
				}
			}

			break;

		//case FLIGHT_PHASE_HOLD_IN_ENROUTE_PATTERN:
		//	break;

		case FLIGHT_PHASE_LAND:
			if ((update_states->flag_target_waypoint_change)
					&& ((update_states->target_WaypointNode_ptr != NULL) && (update_states->target_WaypointNode_ptr->prev_node_ptr != NULL) && (update_states->target_WaypointNode_ptr->prev_node_ptr->wpname != NULL) && (indexOf(update_states->target_WaypointNode_ptr->prev_node_ptr->wpname, "Rwy") > -1))) {
				update_states->lat = h_landing_taxi_plan.waypoint_node_ptr[index_flight]->latitude;
				update_states->lon = h_landing_taxi_plan.waypoint_node_ptr[index_flight]->longitude;
			} else {
				pilot_compute_Lat_Lon_default_logic(update_states, index_flight);
			}

			if (update_states->tas_knots > h_landing_taxi_plan.taxi_tas_knots[index_flight]) {
				if ((h_landing_taxi_plan.runway_name[index_flight] != NULL) && (!update_states->flag_abnormal_on_runway)) {
					// Check if aircraft experiences abnormal event on the runway
					hdg_to_runway_end = compute_heading_rad_gc(update_states->lat,
								update_states->lon,
								update_states->landing_runway_end_latitude,
								update_states->landing_runway_end_longitude);

					distance_to_runway_end = compute_distance_gc(update_states->lat,
								update_states->lon,
								update_states->landing_runway_end_latitude,
								update_states->landing_runway_end_longitude,
								g_trajectories[index_flight].destination_airport_elevation_ft);

					runway_length_landing = compute_distance_gc(
								update_states->landing_runway_entry_latitude,
								update_states->landing_runway_entry_longitude,
								update_states->landing_runway_end_latitude,
								update_states->landing_runway_end_longitude,
								g_trajectories[index_flight].destination_airport_elevation_ft);

					if (RUNWAY_OFFSET_FT < (distance_to_runway_end * sin(abs(hdg_to_runway_end - update_states->course_rad_runway)))) {
						update_states->flag_abnormal_on_runway = true;
						update_states->flight_phase = FLIGHT_PHASE_OUTOFRUNWAY;
						printf("Aircraft %s deviate from the runway during landing\n", update_states->acid);
					} else if ((abs(update_states->course_rad_runway - hdg_to_runway_end) * 180 / PI) > 90) {
						update_states->flag_abnormal_on_runway = true;
						update_states->flight_phase = FLIGHT_PHASE_RUNWAYOVERSHOOT;
						printf("Aircraft %s overshoot the runway during landing\n", update_states->acid);
					} else if ((abs(update_states->course_rad_runway - hdg_to_runway_end) * 180 / PI < 80)
									&& (runway_length_landing < distance_to_runway_end)) {
						update_states->flag_abnormal_on_runway = true;
						update_states->flight_phase = FLIGHT_PHASE_RUNWAYUNDERSHOOT;
						printf("Aircraft %s undershoot the runway during landing\n", update_states->acid);
					}
					// end - Check if aircraft experiences abnormal event on the runway
				}
			}

			break;

		case FLIGHT_PHASE_EXIT_RUNWAY:
			if (c_hold_flight_phase[index_flight] != FLIGHT_PHASE_EXIT_RUNWAY) {
				if ((update_states->flag_target_waypoint_change)
						&& ((update_states->target_WaypointNode_ptr != NULL) && (update_states->target_WaypointNode_ptr->prev_node_ptr != NULL) && (update_states->target_WaypointNode_ptr->prev_node_ptr->wpname != NULL) && (indexOf(update_states->target_WaypointNode_ptr->prev_node_ptr->wpname, "Txy") > -1))) {
					update_states->lat = update_states->target_WaypointNode_ptr->prev_node_ptr->latitude;
					update_states->lon = update_states->target_WaypointNode_ptr->prev_node_ptr->longitude;
				} else {
					pilot_compute_Lat_Lon_default_logic(update_states, index_flight);
				}
			}

			break;

		case FLIGHT_PHASE_TAXI_ARRIVING:
			if (c_hold_flight_phase[index_flight] != FLIGHT_PHASE_TAXI_ARRIVING) {
				pilot_compute_Lat_Lon_default_logic(update_states, index_flight);
			}

			break;

		default:
			pilot_compute_Lat_Lon_default_logic(update_states, index_flight);

			break;
	}
}

/**
 * Pilot module to compute latitude and longitude
 */
void pilot_compute_Lat_Lon(update_states_t* update_states,
		const int index_flight) {
	pilot_compute_Lat_Lon_logic(update_states,
			index_flight);
}

/**
 * Aircraft module to compute altitude
 *
 * The logic is to process different calculation based on different flight phase
 */
void aircraft_compute_altitude(update_states_t* update_states,
		const int index_flight,
		const float t_step_surface,
		const float t_step_terminal,
		const float t_step_airborne) {
	double dh;

	switch (update_states->flight_phase) {

		case FLIGHT_PHASE_ORIGIN_GATE:
			update_states->altitude_ft = c_origin_airport_elevation_ft[index_flight];

			break;

		case FLIGHT_PHASE_PUSHBACK:
			update_states->altitude_ft = c_origin_airport_elevation_ft[index_flight];

			break;

		case FLIGHT_PHASE_RAMP_DEPARTING:
			update_states->altitude_ft = c_origin_airport_elevation_ft[index_flight];

			break;

		case FLIGHT_PHASE_TAXI_DEPARTING:
			update_states->altitude_ft = c_origin_airport_elevation_ft[index_flight];

			break;

		case FLIGHT_PHASE_RUNWAY_THRESHOLD_DEPARTING:
			update_states->altitude_ft = c_origin_airport_elevation_ft[index_flight];

			break;

		case FLIGHT_PHASE_TAKEOFF:
			update_states->altitude_ft = update_states->altitude_ft + update_states->rocd_fps * update_states->durationSecond_altitude_proc;

			break;
/*
		case FLIGHT_PHASE_CLIMBOUT:

			break;
*/
		case FLIGHT_PHASE_HOLD_IN_DEPARTURE_PATTERN:

			break;
/*
		case FLIGHT_PHASE_CLIMB_TO_CRUISE_ALTITUDE:

			break;
*/
		case FLIGHT_PHASE_TOP_OF_CLIMB:
			update_states->altitude_ft = c_cruise_alt_ft[index_flight];

			break;
/*
		case FLIGHT_PHASE_CRUISE:

			break;
*/
		case FLIGHT_PHASE_HOLD_IN_ENROUTE_PATTERN:

			break;
/*
		case FLIGHT_PHASE_TOP_OF_DESCENT:

			break;

		case FLIGHT_PHASE_INITIAL_DESCENT:

			break;
*/
		case FLIGHT_PHASE_HOLD_IN_ARRIVAL_PATTERN:

			break;
/*
		case FLIGHT_PHASE_APPROACH:

			break;

		case FLIGHT_PHASE_FINAL_APPROACH:

			break;
*/
		case FLIGHT_PHASE_GO_AROUND:
			dh = update_states->durationSecond_altitude_proc * update_states->rocd_fps;
			update_states->altitude_ft += dh;

			break;

		case FLIGHT_PHASE_TOUCHDOWN:
			if (update_states->altitude_ft < c_destination_airport_elevation_ft[index_flight]) {
				update_states->altitude_ft = c_destination_airport_elevation_ft[index_flight];
			}

			break;

		case FLIGHT_PHASE_LAND:
			update_states->altitude_ft = c_destination_airport_elevation_ft[index_flight];

			break;

		case FLIGHT_PHASE_EXIT_RUNWAY:
			update_states->altitude_ft = c_destination_airport_elevation_ft[index_flight];

			break;

		case FLIGHT_PHASE_TAXI_ARRIVING:
			update_states->altitude_ft = c_destination_airport_elevation_ft[index_flight];

			break;

		case FLIGHT_PHASE_RUNWAY_CROSSING:
			update_states->altitude_ft = c_destination_airport_elevation_ft[index_flight];

			break;

		case FLIGHT_PHASE_RAMP_ARRIVING:
			update_states->altitude_ft = c_destination_airport_elevation_ft[index_flight];

			break;

		case FLIGHT_PHASE_DESTINATION_GATE:
			update_states->altitude_ft = c_destination_airport_elevation_ft[index_flight];

			break;

		default:
			if (isFlightPhase_in_climbing(update_states->flight_phase)) {
				dh = update_states->durationSecond_altitude_proc * update_states->rocd_fps;

				update_states->altitude_ft += dh;
			} else 	if (isFlightPhase_in_descending(update_states->flight_phase)) {
				dh = update_states->durationSecond_altitude_proc * update_states->rocd_fps;

				update_states->altitude_ft += dh;

				if ((c_destination_airport_elevation_ft[index_flight] < update_states->altitude_ft_bak) && (update_states->altitude_ft <= c_destination_airport_elevation_ft[index_flight])) {
					update_states->altitude_ft = c_destination_airport_elevation_ft[index_flight];

					update_states->flight_phase = FLIGHT_PHASE_TOUCHDOWN;
					update_states->rocd_fps = 0;
					update_states->fpa_rad = 0;

					if (((!h_aircraft_soa.flag_geoStyle[index_flight]) && (h_landing_taxi_plan.runway_name[index_flight] != NULL))
								||
						(h_aircraft_soa.flag_geoStyle[index_flight])) {
						load_init_states_ground_landing(index_flight, update_states);

						update_states->target_WaypointNode_ptr = h_landing_taxi_plan.waypoint_node_ptr[index_flight];
					}
				}
			} else {
				dh = update_states->durationSecond_altitude_proc * update_states->rocd_fps;

				update_states->altitude_ft += dh;

				if ((g_trajectories.at(index_flight).origin_airport_elevation_ft == update_states->altitude_ft_bak)
						&& (update_states->altitude_ft < 0)) {
					update_states->altitude_ft = g_trajectories.at(index_flight).origin_airport_elevation_ft;
				} else if ((g_trajectories.at(index_flight).destination_airport_elevation_ft == update_states->altitude_ft_bak)
						&& (update_states->altitude_ft < 0)) {
					update_states->altitude_ft = g_trajectories.at(index_flight).destination_airport_elevation_ft;
				}

				if (update_states->altitude_ft < 0) {
					update_states->altitude_ft = 0;
				}
			}

			break;
	} // end - switch
}

void aircraft_compute_heading(update_states_t* update_states,
		const int index_flight) {
	pair<double, double> tmpPairLatLon;

	if (update_states != NULL) {
		switch (update_states->flight_phase) {

			case FLIGHT_PHASE_TAKEOFF:
				if ((update_states->rocd_fps == 0)
						&& ((h_aircraft_soa.estimate_takeoff_point_latitude_deg[index_flight] != 0) && (h_aircraft_soa.estimate_takeoff_point_longitude_deg[index_flight] != 0))) {
					update_states->hdg_rad = compute_heading_rad_gc(update_states->lat,
												update_states->lon,
												h_aircraft_soa.estimate_takeoff_point_latitude_deg[index_flight],
												h_aircraft_soa.estimate_takeoff_point_longitude_deg[index_flight]);
				} else {
					update_states->hdg_rad = compute_heading_rad_gc(update_states->lat,
							update_states->lon,
							update_states->target_WaypointNode_ptr->latitude,
							update_states->target_WaypointNode_ptr->longitude);
				}

				break;

			case FLIGHT_PHASE_FINAL_APPROACH:
				if ((aircraftRunwayData.find(string(update_states->acid)) != aircraftRunwayData.end())
						&& (aircraftRunwayData.find(string(update_states->acid))->second.find(STRING_LANDING) != aircraftRunwayData.find(string(update_states->acid))->second.end())) {
					tmpPairLatLon = aircraftRunwayData.at(string(update_states->acid)).at(STRING_LANDING);
					update_states->hdg_rad = compute_heading_rad_gc(update_states->lat,
												update_states->lon,
												tmpPairLatLon.first,
												tmpPairLatLon.second);
				} else {
					update_states->hdg_rad = compute_heading_rad_gc(update_states->lat,
							update_states->lon,
							update_states->target_WaypointNode_ptr->latitude,
							update_states->target_WaypointNode_ptr->longitude);
				}

				break;

			default:
				if (update_states->target_WaypointNode_ptr != NULL) {
					update_states->hdg_rad = compute_heading_rad_gc(update_states->lat,
							update_states->lon,
							update_states->target_WaypointNode_ptr->latitude,
							update_states->target_WaypointNode_ptr->longitude);
				}

				break;

		}
	}
}

void aircraft_compute_runway_heading(update_states_t* update_states,
		const int index_flight) {
	double startPoint_lat = 0.0;
	double startPoint_lon = 0.0;
	double endPoint_lat = 0.0;
	double endPoint_lon = 0.0;

	map<string, AirportNode> map_waypoint_node;
	map<string, AirportNode>::iterator ite_map_waypoint_node;

	if (h_departing_taxi_plan.runway_name[index_flight] != NULL) {
		map_waypoint_node = map_ground_waypoint_connectivity.at(g_trajectories.at(index_flight).origin_airport).map_waypoint_node;

		for (ite_map_waypoint_node = map_waypoint_node.begin(); ite_map_waypoint_node != map_waypoint_node.end(); ite_map_waypoint_node++) {
			if ((!ite_map_waypoint_node->second.refName1.empty()) && (h_departing_taxi_plan.runway_name[index_flight] != NULL) && (strlen(h_departing_taxi_plan.runway_name[index_flight]) > 0) && (strcmp(ite_map_waypoint_node->second.refName1.c_str(), h_departing_taxi_plan.runway_name[index_flight]) == 0)
					&& (!ite_map_waypoint_node->second.type1.empty()) && (strcmp(ite_map_waypoint_node->second.type1.c_str(), "Entry") == 0)) {
				startPoint_lat = ite_map_waypoint_node->second.latitude;
				startPoint_lon = ite_map_waypoint_node->second.longitude;
			} else if ((!ite_map_waypoint_node->second.refName2.empty()) && (h_departing_taxi_plan.runway_name[index_flight] != NULL) && (strlen(h_departing_taxi_plan.runway_name[index_flight]) > 0) && (strcmp(ite_map_waypoint_node->second.refName2.c_str(), h_departing_taxi_plan.runway_name[index_flight]) == 0)
					&& (!ite_map_waypoint_node->second.type2.empty()) && (strcmp(ite_map_waypoint_node->second.type2.c_str(), "End") == 0)) {
				endPoint_lat = ite_map_waypoint_node->second.latitude;
				endPoint_lon = ite_map_waypoint_node->second.longitude;
			}

			if ((startPoint_lat >= 0) && (startPoint_lon)
					&& (endPoint_lat >= 0) && (endPoint_lon))
				break;
		}
	}

	if ((startPoint_lat >= 0) && (startPoint_lon)
			&& (endPoint_lat >= 0) && (endPoint_lon)) {
		update_states->hdg_rad = compute_heading_rad_gc(startPoint_lat, startPoint_lon,
									endPoint_lat, endPoint_lon);
	}
}

real_t aircraft_compute_ground_speed(const float t,
		const double latitude_deg,
		const double longitude_deg,
		const double altitude_ft,
		const double tas_knots_horizontal,
		const double course_rad) {
	real_t retValue = tas_knots_horizontal;

	real_t wind_east_fps = 0, wind_north_fps = 0;

	get_wind_field_components(t, latitude_deg, longitude_deg, altitude_ft, &wind_east_fps, &wind_north_fps);

	real_t cos_hdg = cos(course_rad);

	real_t sin_hdg = sin(course_rad);

	real_t V_update = wind_north_fps/KnotsToFps * cos_hdg + wind_east_fps/KnotsToFps * sin_hdg;

	retValue = tas_knots_horizontal + V_update;

	return retValue;
}

#if CDNR_FLAG
//TODO:PARIKSHIT STATIC ADDERS FOR NATS STARTS
static void ECEFtoNED(real_t* const X, real_t* const Y, real_t* const Z,
		real_t* x, real_t* y, real_t* z){

	*x = - (*X - X_ref()) * sin( REF_LAT * M_PI/180) * cos( REF_LON * M_PI/180)
		 - (*Y - Y_ref()) * sin( REF_LAT * M_PI/180) * sin( REF_LON * M_PI/180)
		 + (*Z - Z_ref()) * cos( REF_LAT * M_PI/180);

	*y = - (*X - X_ref()) * sin( REF_LON * M_PI/180)
		 + (*Y - Y_ref()) * cos( REF_LON * M_PI/180);

	*z = - (*X - X_ref()) * cos( REF_LAT * M_PI/180) * cos( REF_LON * M_PI/180)
		 - (*Y - Y_ref()) * cos( REF_LAT * M_PI/180) * sin( REF_LON * M_PI/180)
		 - (*Z - Z_ref()) * sin( REF_LAT * M_PI/180);
}

static void NEDtoECEF(real_t* const x, real_t* const y, real_t* const z,
		real_t* X, real_t* Y, real_t* Z){

	*X = - (*x) * sin( REF_LAT * M_PI/180) * cos( REF_LON * M_PI/180)
		 - (*y) * sin( REF_LON * M_PI/180)
		 - (*z) * cos( REF_LAT * M_PI/180) * cos( REF_LON * M_PI/180) + X_ref();

	*Y = - (*x) * sin( REF_LAT * M_PI/180) * sin( REF_LON * M_PI/180)
		 + (*y) * cos( REF_LON * M_PI/180)
		 - (*z) * cos( REF_LAT * M_PI/180) * sin( REF_LON * M_PI/180) + Y_ref();

	*Z =  (*x) * cos( REF_LAT * M_PI/180) - (*z) * sin( REF_LAT * M_PI/180) + Z_ref();
}

static void LatLontoNED(real_t* const lat, real_t* const lon, const real_t* const alt,
			real_t* x, real_t* y, real_t* z){
	real_t Rn = RN( (*lat) ) + *alt;
	real_t X = Rn * cos((*lat) * M_PI/180) * cos( (*lon) * M_PI/180);
	real_t Y = Rn * cos((*lat) * M_PI/180) * sin( (*lon) * M_PI/180);
	real_t Z = ( ( 1 - ECCENTRICITY_SQ_EARTH ) * Rn )*sin( (*lat) * M_PI/180);
	ECEFtoNED(&X,&Y,&Z,x,y,z);

}

static void NEDtoLatLon(real_t* const x, real_t* const y, real_t* const z,
			real_t* lat, real_t* lon, real_t* h){
	real_t X=0,Y=0,Z=0;
	NEDtoECEF(x,y,z,&X,&Y,&Z);

	*lon = atan2(Y,X);

	double p = sqrt(X*X + Y*Y);

	double lat_i = atan2( p,Z );
	if (lat_i>= M_PI)
		lat_i = lat_i-M_PI;
	else if (lat_i <= -M_PI)
		lat_i = lat_i +M_PI;

	while(true){

		*h = p/cos(lat_i)- RN(lat_i*180/M_PI);

		double num = Z * (RN(lat_i*180/M_PI) + (*h) );
		double den = p * ( RN(lat_i*180/M_PI)*(1-ECCENTRICITY_SQ_EARTH) + (*h) );
		(*lat) = atan2(num, den);

		if (*lat>= M_PI)
			*lat = *lat-M_PI;
		else if (*lat <= -M_PI)
			*lat = *lat +M_PI;
		if (fabs( (*lat) - lat_i) <1e-6){
			break;
		}
		else if ( (fabs(*lat-M_PI/2) < 1e-6 || fabs(lat_i-M_PI/2) < 1e-6) ||
				(fabs(*lat+M_PI/2) < 1e-6 || fabs(lat_i+M_PI/2) < 1e-6) ){
			break;
		}
		else{
			lat_i = *lat;
		}
	}

	*lat = *lat * 180/M_PI;
	*lon = *lon * 180/M_PI;
}
//TODO:PARIKSHIT STATIC ADDERS FOR NATS ENDS
#endif

int num_flights;
int stream_len;
int num_overflow;

cuda_stream_t* streams;

string get_latestFilename(const char* dirPath, const char* extFilename) {
	string retString = "";

	DIR* dir;
	struct dirent *dirEnt;

	set<string> tmpSetFilename;

	dir = opendir(dirPath);

	if (dir == NULL) {
		printf("Could not open directory %s", dirPath);
	}

	while ((dirEnt = readdir(dir)) != NULL) {
		struct stat path_stat;
		char* dirEnt_name = dirEnt->d_name;

		if ((strcmp(dirEnt_name, ".") == 0) || (strcmp(dirEnt_name, "..") == 0)
				|| (!endsWith(dirEnt_name, extFilename))) {
			continue;
		}

		if (strlen(dirEnt_name) != strlen("xxxxxxxx_xxxxxx")+strlen(extFilename))
			continue;

		bool boolCharNum = false;

		for (int qq = 0; qq < 8; qq++) {
			boolCharNum = (('0' <= dirEnt_name[qq]) && (dirEnt_name[qq] <= '9'));

			if (!boolCharNum)
				break;
		}

		if (!boolCharNum)
			continue;

		if (dirEnt_name[8] != '_')
			continue;

		boolCharNum = false;

		for (int qq = 9; qq < 15; qq++) {
			boolCharNum = (('0' <= dirEnt_name[qq]) && (dirEnt_name[qq] <= '9'));

			if (!boolCharNum)
				break;
		}

		if (!boolCharNum)
			continue;

		tmpSetFilename.insert(dirEnt_name);
	}

	if (tmpSetFilename.size() > 0) {
		set<string>::reverse_iterator tmpIterator = tmpSetFilename.rbegin();
		retString.assign(*tmpIterator);
	}

	return retString;
}

__device__ void cleanUp_waypointNode_linkedList(waypoint_node_t* start_waypointNode_ptr) {
	waypoint_node_t* tmp_next_waypointNode_ptr;
	waypoint_node_t* tmp_cur_waypointNode_ptr;

	if (start_waypointNode_ptr != NULL) {
		tmp_next_waypointNode_ptr = start_waypointNode_ptr;
		while (tmp_next_waypointNode_ptr != NULL) {
			tmp_cur_waypointNode_ptr = tmp_next_waypointNode_ptr;

			releaseWaypointNodeContent(tmp_cur_waypointNode_ptr);

			tmp_cur_waypointNode_ptr->prev_node_ptr = NULL;
			tmp_next_waypointNode_ptr = tmp_cur_waypointNode_ptr->next_node_ptr;
			tmp_cur_waypointNode_ptr->next_node_ptr = NULL;

			free(tmp_cur_waypointNode_ptr);

			if (tmp_next_waypointNode_ptr == start_waypointNode_ptr)
				break;
		}

		start_waypointNode_ptr = NULL;
	}
}

/**
 * Clean up the data [update_states]
 */
__global__ void cleanUp_update_states(update_states_t* update_states) {
	if (update_states->acid != NULL) {
		free(update_states->acid);

		update_states->acid = NULL;
	}

	// Clean up temporary variable
	update_states->target_WaypointNode_ptr = NULL;
	update_states->last_WaypointNode_ptr = NULL;

	waypoint_node_t* tmp_next_waypointNode_ptr;
	waypoint_node_t* tmp_cur_waypointNode_ptr;

	cleanUp_waypointNode_linkedList(update_states->go_around_WaypointNode_ptr);

	cleanUp_waypointNode_linkedList(update_states->holding_pattern_WaypointNode_ptr);

	free(update_states);
	update_states = NULL;
}

/**
 * Check clearance from the Pilot's perspective
 *
 * Description
 * A pilot can submit clearance request to the controller.
 * The controller may or may not give response.
 * In other words, the decision of a clearance could be -1(undecided), 0(rejected) or 1(granted).
 *
 * The logic of this function:
 * 1. When the clearance is submitted, [t_request] records current timestamp
 * 2. g_humanError_Pilot[index_flight]->second.pre_delaySecond: Delayed seconds of the pilot before submitting the clearance request
 *    g_humanError_Controller[0][index_flight].delaySecond: Delayed seconds of the controller.
 *    These two delay seconds combine and form the total delayed seconds before the clearance can be granted by the controller.
 *    Then we can calculate the timestamp of the granted time.
 * 3. When the simulation timestamp is latter than the granted timestamp, this function returns the clearance decision.
 *
 * Notice.(2018.09.11)
 * We assume the clearance will always be granted, not be rejected.
 * If we have to consider the "clearance rejection" case, we have to re-design this function.
 */
__device__ int pilot_check_clearance_aircraft(const float t,
		const int index_flight,
		const HumanErrorEvent_t error_event) {
	int retValue = -1;

	if (index_flight > -1) {
		ENUM_Aircraft_Clearance aircraft_clearance = getENUM_Aircraft_Clearance(error_event.name);
		std::map<ENUM_Aircraft_Clearance, Aircraft_Clearance_Data_t>* tmpMap_clearance_ptr = &(g_map_clearance_aircraft.at(index_flight));
		Aircraft_Clearance_Data_t tmpAircraft_Clearance_Data;

		// If no existing aircraft clearance data, insert a new one.
		if (tmpMap_clearance_ptr->find(aircraft_clearance) == tmpMap_clearance_ptr->end()) {
			tmpAircraft_Clearance_Data.decision = -1;
			tmpAircraft_Clearance_Data.t_request = t;

			tmpMap_clearance_ptr->insert(pair<ENUM_Aircraft_Clearance, Aircraft_Clearance_Data_t>(aircraft_clearance, tmpAircraft_Clearance_Data));
		} else {
			tmpAircraft_Clearance_Data = tmpMap_clearance_ptr->find(aircraft_clearance)->second;
		}

		if (tmpAircraft_Clearance_Data.decision == -1) {
			HumanErrorEvent_t tmpHumanErrorEvent_pilot;
			tmpHumanErrorEvent_pilot.type = PILOT_ERROR_EVENT_TYPE_CLEARANCE;
			tmpHumanErrorEvent_pilot.name = error_event.name;

			HumanErrorEvent_t tmpHumanErrorEvent_controller;
			tmpHumanErrorEvent_controller.type = CONTROLLER_ERROR_EVENT_TYPE_CLEARANCE;
			tmpHumanErrorEvent_controller.name = error_event.name;

			int tmpTime_gap = t - tmpAircraft_Clearance_Data.t_request;

			float tmpDelaySecond = 0.0;

			if (g_humanError_Pilot[index_flight].find(tmpHumanErrorEvent_pilot) != g_humanError_Pilot[index_flight].end()) {
				tmpDelaySecond += g_humanError_Pilot[index_flight].find(tmpHumanErrorEvent_pilot)->second.pre_delaySecond;
			}
			if (g_humanError_Controller[0][index_flight].find(tmpHumanErrorEvent_controller) != g_humanError_Controller[0][index_flight].end()) {
				tmpDelaySecond += g_humanError_Controller[0][index_flight].find(tmpHumanErrorEvent_controller)->second.delaySecond;
			}

			if (tmpTime_gap >= tmpDelaySecond) {
				g_map_clearance_aircraft[index_flight].find(aircraft_clearance)->second.decision = 1;
				g_map_clearance_aircraft[index_flight].find(aircraft_clearance)->second.t_decision = g_map_clearance_aircraft[index_flight].find(aircraft_clearance)->second.t_request + tmpDelaySecond;

				tmpAircraft_Clearance_Data.decision = 1;
			}
		}

		retValue = tmpAircraft_Clearance_Data.decision;
	}

	return retValue;
}

/**
 * Pilot module to process flight phase transition
 *
 * This function check the current flight phase and change it to another phase when certain condition is satisfied.
 */
__device__ void pilot_proc_flightPhase(update_states_t* update_states,
		const int index_flight,
		const float t,
		const float t_step_surface,
		const float t_step_terminal,
		const float t_step_airborne) {
	// If flight phase = FLIGHT_PHASE_HOLDING, quit this function
	if (update_states->flight_phase == FLIGHT_PHASE_HOLDING) {
		return;
	}

	if (update_states->flight_phase == FLIGHT_PHASE_LANDED) {
		return;
	}

	// If flight phase to be skipped by pilot, quit this function
	if (ENUM_Flight_Phase_String[update_states->flight_phase] == skipFlightPhase[index_flight]) {
		for (unsigned int i = 0; i < ENUM_Flight_Phase_Count; i++) {
			if (ENUM_Flight_Phase_String[i] == skipFlightPhase[index_flight]) {
				update_states->flight_phase = ENUM_Flight_Phase(i + 1);
				break;
			}
		}
	}

	int tmpAdb_aircraft_type_index = c_adb_aircraft_type_index[index_flight];
	AdbOPFModel tmpAdbOPFModel = g_adb_opf_models.at(tmpAdb_aircraft_type_index);
	AdbPTFModel tmpAdbPTFModel = g_adb_ptf_models.at(tmpAdb_aircraft_type_index);

	double take_off_length;
	double vstall_take_off;
	double vstall_landing;
	double V2_tas_knots;
	double vTouchdown_tas_knots;
	float tmp_t_step;

	HumanErrorEvent_t tmpHumanErrorEvent;

	if (update_states->altitude_ft < TRACON_ALT_FT) {
		tmp_t_step = t_step_terminal;
	} else {
		tmp_t_step = t_step_airborne;
	}

	waypoint_node_t* prev_waypointNode_ptr;
	waypoint_node_t* new_waypointNode_ptr;

	pair<double, double> tmpPairLatLon;

	// Here we handle the switch of nominal flight phase
	switch (update_states->flight_phase) {
		case FLIGHT_PHASE_ORIGIN_GATE:
			tmpHumanErrorEvent.type = PILOT_ERROR_EVENT_TYPE_CLEARANCE;
			tmpHumanErrorEvent.name = stringify(AIRCRAFT_CLEARANCE_PUSHBACK);

			if ((pilot_skip_request_clearance_aircraft(index_flight, tmpHumanErrorEvent)) || (pilot_check_clearance_aircraft(t, index_flight, tmpHumanErrorEvent) == 1)) {
				update_states->flight_phase = FLIGHT_PHASE_PUSHBACK;
			} else {
				// No clearance.  Set durationSecond_to_be_proc = 0
				update_states->durationSecond_to_be_proc = 0;
			}

			break;

		case FLIGHT_PHASE_PUSHBACK:
			if ((update_states->flag_target_waypoint_change) && (update_states->last_WaypointNode_ptr != NULL)) {
				if (((!h_aircraft_soa.flag_geoStyle[index_flight]) && (strncasecmp(update_states->last_WaypointNode_ptr->wpname, "Ramp", 4) == 0))
						||
					((h_aircraft_soa.flag_geoStyle[index_flight]) && (strncasecmp(update_states->last_WaypointNode_ptr->wptype, "Ramp", 4) == 0))) {
					update_states->flight_phase = FLIGHT_PHASE_RAMP_DEPARTING;
				} else if (((!h_aircraft_soa.flag_geoStyle[index_flight]) && (strncasecmp(update_states->last_WaypointNode_ptr->wpname, "Txy", 3) == 0))
								||
						   ((h_aircraft_soa.flag_geoStyle[index_flight]) && (strncasecmp(update_states->last_WaypointNode_ptr->wptype, "Taxiway", 7) == 0))
						   ) {
					update_states->flight_phase = FLIGHT_PHASE_TAXI_DEPARTING;
				}
			}

			break;

		case FLIGHT_PHASE_RAMP_DEPARTING:
			tmpHumanErrorEvent.type = PILOT_ERROR_EVENT_TYPE_CLEARANCE;
			tmpHumanErrorEvent.name = stringify(AIRCRAFT_CLEARANCE_TAXI_DEPARTING);

			if ((update_states->flag_target_waypoint_change) && (update_states->last_WaypointNode_ptr != NULL)) {
				if (((!h_aircraft_soa.flag_geoStyle[index_flight]) && (strncasecmp(update_states->last_WaypointNode_ptr->wpname, "Txy", 3) == 0))
						||
					((h_aircraft_soa.flag_geoStyle[index_flight]) && (strncasecmp(update_states->last_WaypointNode_ptr->wptype, "Taxiway", 7) == 0))
					) {
					if ((pilot_skip_request_clearance_aircraft(index_flight, tmpHumanErrorEvent)) || (pilot_check_clearance_aircraft(t, index_flight, tmpHumanErrorEvent) == 1)) {
						update_states->flight_phase = FLIGHT_PHASE_TAXI_DEPARTING;
					} else {
						c_hold_flight_phase[index_flight] = FLIGHT_PHASE_RAMP_DEPARTING;
					}
				}
			}

			if (c_hold_flight_phase[index_flight] == FLIGHT_PHASE_RAMP_DEPARTING) {
				if ((pilot_skip_request_clearance_aircraft(index_flight, tmpHumanErrorEvent)) || (pilot_check_clearance_aircraft(t, index_flight, tmpHumanErrorEvent) == 1)) {
					update_states->flight_phase = FLIGHT_PHASE_TAXI_DEPARTING;

					c_hold_flight_phase[index_flight] = FLIGHT_PHASE_PREDEPARTURE; // Set it as NULL value
				}
			}

			break;

		case FLIGHT_PHASE_TAXI_DEPARTING:
			if ((update_states->flag_target_waypoint_change) && (update_states->last_WaypointNode_ptr != NULL)) {
				if (((!h_aircraft_soa.flag_geoStyle[index_flight]) && (strncasecmp(update_states->last_WaypointNode_ptr->wpname, "Gate", 4) == 0))
						||
					((h_aircraft_soa.flag_geoStyle[index_flight]) && (strncasecmp(update_states->last_WaypointNode_ptr->wptype, "Gate", 4) == 0))) {
					update_states->flight_phase = FLIGHT_PHASE_ORIGIN_GATE;

					update_states->durationSecond_to_be_proc = 0; // Reset
				} else if (((!h_aircraft_soa.flag_geoStyle[index_flight]) && (strncasecmp(update_states->last_WaypointNode_ptr->wpname, "Ramp", 4) == 0))
						||
						((h_aircraft_soa.flag_geoStyle[index_flight]) && (strncasecmp(update_states->last_WaypointNode_ptr->wptype, "Ramp", 4) == 0))) {
					update_states->flight_phase = FLIGHT_PHASE_RAMP_DEPARTING;
				} else if ((h_departing_taxi_plan.waypoint_final_node_ptr[index_flight] != NULL)
								&& (update_states->last_WaypointNode_ptr == h_departing_taxi_plan.waypoint_final_node_ptr[index_flight])) {
					update_states->flight_phase = FLIGHT_PHASE_RUNWAY_THRESHOLD_DEPARTING;

					if (array_Airborne_Flight_Plan_ptr[index_flight] != NULL) {
						// Set new value
						update_states->target_waypoint_index = 1;
						update_states->target_WaypointNode_ptr = array_Airborne_Flight_Plan_ptr[index_flight]->next_node_ptr;

						update_states->fp_lat = update_states->target_WaypointNode_ptr->latitude;
						update_states->fp_lon = update_states->target_WaypointNode_ptr->longitude;
					}

					update_states->durationSecond_to_be_proc = 0; // Reset
				}
			}

			break;

		case FLIGHT_PHASE_RUNWAY_THRESHOLD_DEPARTING:
			tmpHumanErrorEvent.type = PILOT_ERROR_EVENT_TYPE_CLEARANCE;
			tmpHumanErrorEvent.name = stringify(AIRCRAFT_CLEARANCE_TAKEOFF);

			if (array_Airborne_Flight_Plan_ptr[index_flight] != NULL) {
				if ((pilot_skip_request_clearance_aircraft(index_flight, tmpHumanErrorEvent)) || (pilot_check_clearance_aircraft(t, index_flight, tmpHumanErrorEvent) == 1)) {
					update_states->flight_phase = FLIGHT_PHASE_TAKEOFF;

					update_states->target_waypoint_index = 1;
					update_states->target_WaypointNode_ptr = array_Airborne_Flight_Plan_ptr[index_flight]->next_node_ptr;
				}
			}

			break;

		case FLIGHT_PHASE_TAKEOFF:
			update_states->V_stall_takeoff = tmpAdbOPFModel.vstall.at(TAKEOFF);
			update_states->V2_tas_knots = compute_true_speed_from_calibrated(MIN_SPEED_COEFFICIENT_TAKEOFF * update_states->V_stall_takeoff, 0);

			if ((c_t_takeoff[index_flight] <= 0) && (update_states->tas_knots >= update_states->V2_tas_knots)) {
				c_t_takeoff[index_flight] = t;
			}

			if ((c_flag_reached_meterfix_point[index_flight])
					|| ((0 < c_t_takeoff[index_flight]) && (update_states->target_WaypointNode_ptr != NULL) && (array_Airborne_Flight_Plan_ptr[index_flight]->next_node_ptr != NULL) && (array_Airborne_Flight_Plan_ptr[index_flight]->next_node_ptr->next_node_ptr != NULL) && (update_states->target_WaypointNode_ptr == array_Airborne_Flight_Plan_ptr[index_flight]->next_node_ptr->next_node_ptr))) {
				update_states->flight_phase = FLIGHT_PHASE_CLIMBOUT;
			}

			break;

		case FLIGHT_PHASE_CLIMBOUT:
			if (TRACON_ALT_FT <= update_states->altitude_ft) {
				tmpHumanErrorEvent.type = PILOT_ERROR_EVENT_TYPE_CLEARANCE;
				tmpHumanErrorEvent.name = stringify(AIRCRAFT_CLEARANCE_ENTER_ARTC);

				if ((pilot_skip_request_clearance_aircraft(index_flight, tmpHumanErrorEvent)) || (pilot_check_clearance_aircraft(t, index_flight, tmpHumanErrorEvent) == 1)) {
					update_states->flight_phase = FLIGHT_PHASE_CLIMB_TO_CRUISE_ALTITUDE;
				} else {
					update_states->flight_phase = FLIGHT_PHASE_HOLD_IN_DEPARTURE_PATTERN;

					c_hold_flight_phase[index_flight] = FLIGHT_PHASE_CLIMBOUT;

					// Generate hold pattern waypoints
					prev_waypointNode_ptr = NULL;

					// Get a list of holding pattern waypoints
					// Result is stored in "holdPattern"
					getHoldPattern(update_states->lat, update_states->lon, (update_states->hdg_rad * 180 / PI));
					for (int i = 0; i < 9; i++) { // Variable "holdPattern" has 8 elements
						new_waypointNode_ptr = (waypoint_node_t*)calloc(1, sizeof(waypoint_node_t));
						new_waypointNode_ptr->latitude = holdPattern[i][0] * 180 / PI;
						new_waypointNode_ptr->longitude = holdPattern[i][1] * 180 / PI;
						new_waypointNode_ptr->wpname = NULL;

						if (i > 0) {
							new_waypointNode_ptr->prev_node_ptr = prev_waypointNode_ptr;
							prev_waypointNode_ptr->next_node_ptr = new_waypointNode_ptr;
						}

						if (i == 0)
							update_states->holding_pattern_WaypointNode_ptr = new_waypointNode_ptr;

						prev_waypointNode_ptr = new_waypointNode_ptr;
					}

					new_waypointNode_ptr->next_node_ptr = update_states->holding_pattern_WaypointNode_ptr;
					update_states->holding_pattern_WaypointNode_ptr->prev_node_ptr = new_waypointNode_ptr;
					// end - Generate hold pattern waypoints

					update_states->pre_holding_pattern_target_WaypointNode_ptr = update_states->target_WaypointNode_ptr;

					// Set new target waypoint
					update_states->target_WaypointNode_ptr = update_states->holding_pattern_WaypointNode_ptr;
				}
			}

			break;

		case FLIGHT_PHASE_HOLD_IN_DEPARTURE_PATTERN:
			tmpHumanErrorEvent.type = PILOT_ERROR_EVENT_TYPE_CLEARANCE;

			if (c_hold_flight_phase[index_flight] == FLIGHT_PHASE_CLIMBOUT) {
				tmpHumanErrorEvent.name = stringify(AIRCRAFT_CLEARANCE_ENTER_ARTC);

				if ((pilot_skip_request_clearance_aircraft(index_flight, tmpHumanErrorEvent)) || (pilot_check_clearance_aircraft(t, index_flight, tmpHumanErrorEvent) == 1)) {
					update_states->flight_phase = FLIGHT_PHASE_CLIMB_TO_CRUISE_ALTITUDE;

					update_states->rocd_fps = tmpAdbPTFModel.getClimbRate(update_states->altitude_ft, LOW) / 60;

					update_states->target_altitude_ft = update_states->cruise_alt_ft;

					c_hold_flight_phase[index_flight] = FLIGHT_PHASE_PREDEPARTURE; // Set it as NULL value

					// Set new target waypoint
					update_states->target_WaypointNode_ptr = update_states->pre_holding_pattern_target_WaypointNode_ptr;

					update_states->pre_holding_pattern_target_WaypointNode_ptr = NULL; // Reset
				}
			}

			break;

		case FLIGHT_PHASE_CLIMB_TO_CRUISE_ALTITUDE:
			if (update_states->cruise_alt_ft <= update_states->altitude_ft) {
				update_states->flight_phase = FLIGHT_PHASE_TOP_OF_CLIMB;
			}

			break;

		case FLIGHT_PHASE_TOP_OF_CLIMB:
			if (update_states->flight_phase == FLIGHT_PHASE_TOP_OF_CLIMB) {
				update_states->flight_phase = FLIGHT_PHASE_CRUISE;
			}

			break;

		case FLIGHT_PHASE_CRUISE:
			if ((update_states->flag_target_waypoint_change) && (update_states->last_WaypointNode_ptr != NULL) && (indexOf(update_states->last_WaypointNode_ptr->wpname, "TOP_OF_DESCENT_PT") == 0)) {
				update_states->flight_phase = FLIGHT_PHASE_TOP_OF_DESCENT;
			}

			break;

		case FLIGHT_PHASE_HOLD_IN_ENROUTE_PATTERN:
			tmpHumanErrorEvent.type = PILOT_ERROR_EVENT_TYPE_CLEARANCE;

			if (c_hold_flight_phase[index_flight] == FLIGHT_PHASE_TOP_OF_DESCENT) {
				tmpHumanErrorEvent.name = stringify(AIRCRAFT_CLEARANCE_DESCENT_FROM_CRUISE);

				if ((pilot_skip_request_clearance_aircraft(index_flight, tmpHumanErrorEvent)) || (pilot_check_clearance_aircraft(t, index_flight, tmpHumanErrorEvent) == 1)) {
					update_states->target_WaypointNode_ptr = array_Airborne_Flight_Plan_tod_ptr[index_flight]->next_node_ptr; // Set new target waypoint

					update_states->flight_phase = FLIGHT_PHASE_INITIAL_DESCENT;

					cleanUp_waypointNode_linkedList(update_states->holding_pattern_WaypointNode_ptr);

					update_states->holding_pattern_WaypointNode_ptr = NULL;

					c_hold_flight_phase[index_flight] = FLIGHT_PHASE_PREDEPARTURE; // Set it as NULL value
				}
			}

			break;

		case FLIGHT_PHASE_TOP_OF_DESCENT:
			update_states->S_within_flightPhase_and_simCycle = 0; // Reset

			tmpHumanErrorEvent.type = PILOT_ERROR_EVENT_TYPE_CLEARANCE;
			tmpHumanErrorEvent.name = stringify(AIRCRAFT_CLEARANCE_DESCENT_FROM_CRUISE);

			if ((pilot_skip_request_clearance_aircraft(index_flight, tmpHumanErrorEvent)) || (pilot_check_clearance_aircraft(t, index_flight, tmpHumanErrorEvent)) == 1) {
				update_states->flight_phase = FLIGHT_PHASE_INITIAL_DESCENT;
			} else {
				// Set new flight phase
				update_states->flight_phase = FLIGHT_PHASE_HOLD_IN_ENROUTE_PATTERN;

				c_hold_flight_phase[index_flight] = FLIGHT_PHASE_TOP_OF_DESCENT;

				// Generate hold pattern waypoints
				prev_waypointNode_ptr = NULL;

				// Get a list of holding pattern waypoints
				// Result is stored in "holdPattern"
				getHoldPattern(update_states->lat, update_states->lon, (update_states->hdg_rad * 180 / PI));
				for (int i = 0; i < 9; i++) {
					new_waypointNode_ptr = (waypoint_node_t*)calloc(1, sizeof(waypoint_node_t));
					new_waypointNode_ptr->latitude = holdPattern[i][0] * 180 / PI;
					new_waypointNode_ptr->longitude = holdPattern[i][1] * 180 / PI;
					new_waypointNode_ptr->wpname = NULL;

					if (i > 0) {
						new_waypointNode_ptr->prev_node_ptr = prev_waypointNode_ptr;
						prev_waypointNode_ptr->next_node_ptr = new_waypointNode_ptr;
					}

					if (i == 0)
						update_states->holding_pattern_WaypointNode_ptr = new_waypointNode_ptr;

					prev_waypointNode_ptr = new_waypointNode_ptr;
				}

				new_waypointNode_ptr->next_node_ptr = update_states->holding_pattern_WaypointNode_ptr;
				update_states->holding_pattern_WaypointNode_ptr->prev_node_ptr = new_waypointNode_ptr;
				// end - Generate hold pattern waypoints

				// Set new target waypoint
				update_states->target_WaypointNode_ptr = update_states->holding_pattern_WaypointNode_ptr;
			}

			break;

		case FLIGHT_PHASE_INITIAL_DESCENT:
			if ((update_states->altitude_ft < (TRACON_ALT_FT + CLEARANCE_BUFFER_ALT_RANGE_FT))) {
				tmpHumanErrorEvent.type = PILOT_ERROR_EVENT_TYPE_CLEARANCE;
				tmpHumanErrorEvent.name = stringify(AIRCRAFT_CLEARANCE_ENTER_TRACON);

				if ((pilot_skip_request_clearance_aircraft(index_flight, tmpHumanErrorEvent)) || (pilot_check_clearance_aircraft(t, index_flight, tmpHumanErrorEvent) == 1)) {
					if (update_states->target_WaypointNode_ptr != NULL) {
						if (update_states->altitude_ft <= TRACON_ALT_FT) {
							update_states->flight_phase = FLIGHT_PHASE_APPROACH;
						}
					}
				} else {
					update_states->flight_phase = FLIGHT_PHASE_HOLD_IN_ARRIVAL_PATTERN;

					c_hold_flight_phase[index_flight] = FLIGHT_PHASE_INITIAL_DESCENT;

					// Generate hold pattern waypoints
					prev_waypointNode_ptr = NULL;

					// Get a list of holding pattern waypoints
					// Result is stored in "holdPattern"
					getHoldPattern(update_states->lat, update_states->lon, (update_states->hdg_rad * 180 / PI));
					for (int i = 0; i < 9; i++) {
						new_waypointNode_ptr = (waypoint_node_t*)calloc(1, sizeof(waypoint_node_t));
						new_waypointNode_ptr->latitude = holdPattern[i][0] * 180 / PI;
						new_waypointNode_ptr->longitude = holdPattern[i][1] * 180 / PI;
						new_waypointNode_ptr->wpname = NULL;

						if (i > 0) {
							new_waypointNode_ptr->prev_node_ptr = prev_waypointNode_ptr;
							prev_waypointNode_ptr->next_node_ptr = new_waypointNode_ptr;
						}

						if (i == 0)
							update_states->holding_pattern_WaypointNode_ptr = new_waypointNode_ptr;

						prev_waypointNode_ptr = new_waypointNode_ptr;
					}

					new_waypointNode_ptr->next_node_ptr = update_states->holding_pattern_WaypointNode_ptr;
					update_states->holding_pattern_WaypointNode_ptr->prev_node_ptr = new_waypointNode_ptr;
					// end - Generate hold pattern waypoints

					update_states->pre_holding_pattern_target_WaypointNode_ptr = update_states->target_WaypointNode_ptr;

					// Set new target waypoint
					update_states->target_WaypointNode_ptr = update_states->holding_pattern_WaypointNode_ptr;
				}
			}

			break;

		case FLIGHT_PHASE_HOLD_IN_ARRIVAL_PATTERN:
			tmpHumanErrorEvent.type = PILOT_ERROR_EVENT_TYPE_CLEARANCE;

			if (c_hold_flight_phase[index_flight] == FLIGHT_PHASE_INITIAL_DESCENT) {
				tmpHumanErrorEvent.name = stringify(AIRCRAFT_CLEARANCE_ENTER_TRACON);

				if ((pilot_skip_request_clearance_aircraft(index_flight, tmpHumanErrorEvent)) || (pilot_check_clearance_aircraft(t, index_flight, tmpHumanErrorEvent) == 1)) {
					update_states->flight_phase = FLIGHT_PHASE_INITIAL_DESCENT;

					c_hold_flight_phase[index_flight] = FLIGHT_PHASE_PREDEPARTURE; // Set it as NULL value

					// Set new target waypoint
					update_states->target_WaypointNode_ptr = update_states->pre_holding_pattern_target_WaypointNode_ptr;

					update_states->pre_holding_pattern_target_WaypointNode_ptr = NULL; // Reset
				}
			} else if (c_hold_flight_phase[index_flight] == FLIGHT_PHASE_GO_AROUND) {
				tmpHumanErrorEvent.name = stringify(AIRCRAFT_CLEARANCE_APPROACH);

				if ((pilot_skip_request_clearance_aircraft(index_flight, tmpHumanErrorEvent)) || (pilot_check_clearance_aircraft(t, index_flight, tmpHumanErrorEvent) == 1)) {
					waypoint_node_t* tmpWaypointNode_ptr = array_Airborne_Flight_Plan_ptr[index_flight];
					while (tmpWaypointNode_ptr != NULL) {
						if (((!h_aircraft_soa.flag_geoStyle[index_flight]) && (indexOf(tmpWaypointNode_ptr->proctype, "APPROACH") > -1))
								||
							((h_aircraft_soa.flag_geoStyle[index_flight]) && (indexOf(tmpWaypointNode_ptr->phase, "APPROACH") > -1))) {
							update_states->target_WaypointNode_ptr = tmpWaypointNode_ptr; // Found first APPROACH waypoint

							update_states->flight_phase = FLIGHT_PHASE_APPROACH;

							c_hold_flight_phase[index_flight] = FLIGHT_PHASE_PREDEPARTURE; // Set it as NULL value

							cleanUp_waypointNode_linkedList(update_states->holding_pattern_WaypointNode_ptr);
							update_states->holding_pattern_WaypointNode_ptr = NULL;

							cleanUp_waypointNode_linkedList(update_states->go_around_WaypointNode_ptr);
							update_states->go_around_WaypointNode_ptr = NULL;

							break;
						}

						tmpWaypointNode_ptr = tmpWaypointNode_ptr->next_node_ptr;
					}
				}
			}

			break;

		case FLIGHT_PHASE_APPROACH:
			if ((update_states->target_WaypointNode_ptr != NULL) && (update_states->target_WaypointNode_ptr->prev_node_ptr != NULL)) {
				if (((!h_aircraft_soa.flag_geoStyle[index_flight]) && (update_states->target_WaypointNode_ptr->prev_node_ptr->proctype != NULL) && (indexOf(update_states->target_WaypointNode_ptr->prev_node_ptr->proctype, "APPROACH") > -1))
						||
					((h_aircraft_soa.flag_geoStyle[index_flight]) && (update_states->target_WaypointNode_ptr->prev_node_ptr->phase != NULL) && (indexOf(update_states->target_WaypointNode_ptr->prev_node_ptr->phase, "FINAL_APPROACH") > -1))) {
					update_states->flight_phase = FLIGHT_PHASE_FINAL_APPROACH;
				}
			}

			break;

		case FLIGHT_PHASE_FINAL_APPROACH:
			// Calculate estimate touchdown point
			if ((c_estimate_touchdown_point_latitude_deg[index_flight] == 0) && (c_estimate_touchdown_point_longitude_deg[index_flight] == 0)) {
				if ((aircraftRunwayData.find(string(update_states->acid)) != aircraftRunwayData.end())
										&& (aircraftRunwayData.find(string(update_states->acid))->second.find(STRING_LANDING) != aircraftRunwayData.find(string(update_states->acid))->second.end())){
					tmpPairLatLon = aircraftRunwayData.at(string(update_states->acid)).at(STRING_LANDING);
					c_estimate_touchdown_point_latitude_deg[index_flight] = tmpPairLatLon.first;
					c_estimate_touchdown_point_longitude_deg[index_flight] = tmpPairLatLon.second;
				} else if (h_landing_taxi_plan.runway_name[index_flight] != NULL) {
					tmpPairLatLon = getTouchdownPoint(string(g_trajectories.at(index_flight).destination_airport), string(h_landing_taxi_plan.runway_name[index_flight]), g_trajectories.at(index_flight).destination_airport_elevation_ft, tmpAdbOPFModel.ldl);
					c_estimate_touchdown_point_latitude_deg[index_flight] = tmpPairLatLon.first;
					c_estimate_touchdown_point_longitude_deg[index_flight] = tmpPairLatLon.second;
				} else {
					c_estimate_touchdown_point_latitude_deg[index_flight] = array_Airborne_Flight_Plan_Final_Node_ptr[index_flight]->latitude;
					c_estimate_touchdown_point_longitude_deg[index_flight] = array_Airborne_Flight_Plan_Final_Node_ptr[index_flight]->longitude;
				}
			}

			tmpHumanErrorEvent.type = PILOT_ERROR_EVENT_TYPE_CLEARANCE;
			tmpHumanErrorEvent.name = stringify(AIRCRAFT_CLEARANCE_TOUCHDOWN);

			// If "pilot skip requesting clearance" or "clear to TOUCHDOWN"
			if ((pilot_skip_request_clearance_aircraft(index_flight, tmpHumanErrorEvent)) || (pilot_check_clearance_aircraft(t, index_flight, tmpHumanErrorEvent) == 1)) {
				// If target waypoint changes
				if ((update_states->flag_target_waypoint_change)
						&& (update_states->target_WaypointNode_ptr == NULL)
						) {
					update_states->flight_phase = FLIGHT_PHASE_TOUCHDOWN;
				} else {
					vstall_landing = tmpAdbOPFModel.vstall.at(LANDING);
					vTouchdown_tas_knots = compute_true_speed_from_calibrated(MIN_SPEED_COEFFICIENT_NOT_TAKEOFF * vstall_landing, 0);

					if (update_states->altitude_ft <= c_destination_airport_elevation_ft[index_flight]) {
						update_states->flight_phase = FLIGHT_PHASE_TOUCHDOWN;
					} else if ((update_states->altitude_ft <= c_destination_airport_elevation_ft[index_flight]+100) &&
								(update_states->tas_knots_bak <= vTouchdown_tas_knots)) {
						update_states->flight_phase = FLIGHT_PHASE_TOUCHDOWN;
					}
				}

				if (update_states->flight_phase == FLIGHT_PHASE_TOUCHDOWN) {
					if (((!h_aircraft_soa.flag_geoStyle[index_flight]) && (h_landing_taxi_plan.runway_name[index_flight] != NULL))
							||
						(h_aircraft_soa.flag_geoStyle[index_flight])) {
						load_init_states_ground_landing(index_flight, update_states);

						update_states->target_WaypointNode_ptr = h_landing_taxi_plan.waypoint_node_ptr[index_flight];
					}
				}
			} else {
				if (update_states->altitude_ft <= (c_destination_airport_elevation_ft[index_flight]+500)) {
					update_states->flight_phase = FLIGHT_PHASE_GO_AROUND;

					obtain_go_around_waypoint(update_states, index_flight);

					if (h_aircraft_soa.flag_geoStyle[index_flight]) { // Geo-Style
						// If the target waypoint is not GOAROUND phase
						if ((update_states->target_WaypointNode_ptr == NULL) || (update_states->target_WaypointNode_ptr->phase == NULL) || (strcmp(update_states->target_WaypointNode_ptr->phase, "GOAROUND") != 0)) {
							printf("%s: No GOAROUND waypoint defined.  Simulation ended.\n", array_update_states_ptr[index_flight]->acid);

							array_update_states_ptr[index_flight]->flight_phase = FLIGHT_PHASE_LANDED;
							array_update_states_ptr[index_flight]->landed_flag = true;
							c_landed_flag[index_flight] = true;

							h_aircraft_soa.flight_phase[index_flight] = FLIGHT_PHASE_LANDED;
						}
					}
				}
			}

			break;

		case FLIGHT_PHASE_GO_AROUND:
			if (!h_aircraft_soa.flag_geoStyle[index_flight]) { // Geo-Style
				// In Go-Around phase, target waypoint changes.
				// This means the aircraft reached the Go-Around waypoint.  Next, the program will generate the hold pattern waypoints.
				if (update_states->flag_target_waypoint_change) {
					if (update_states->go_around_WaypointNode_ptr != NULL) {
						float tmpCourseHeading = compute_heading_gc(update_states->go_around_split_point_latitude, update_states->go_around_split_point_longitude, update_states->go_around_WaypointNode_ptr->latitude, update_states->go_around_WaypointNode_ptr->longitude);

						prev_waypointNode_ptr = NULL;

						// Get a list of holding pattern waypoints
						// Result is stored in "holdPattern"
						getHoldPattern(update_states->go_around_WaypointNode_ptr->latitude, update_states->go_around_WaypointNode_ptr->longitude, tmpCourseHeading);
						for (int i = 0; i < 9; i++) {
							new_waypointNode_ptr = (waypoint_node_t*)calloc(1, sizeof(waypoint_node_t));
							new_waypointNode_ptr->latitude = holdPattern[i][0] * 180 / PI;
							new_waypointNode_ptr->longitude = holdPattern[i][1] * 180 / PI;
							new_waypointNode_ptr->wpname = NULL;

							if (i > 0) {
								new_waypointNode_ptr->prev_node_ptr = prev_waypointNode_ptr;
								prev_waypointNode_ptr->next_node_ptr = new_waypointNode_ptr;
							}

							if (i == 0)
								update_states->holding_pattern_WaypointNode_ptr = new_waypointNode_ptr;

							prev_waypointNode_ptr = new_waypointNode_ptr;
						}

						new_waypointNode_ptr->next_node_ptr = update_states->holding_pattern_WaypointNode_ptr;
						update_states->holding_pattern_WaypointNode_ptr->prev_node_ptr = new_waypointNode_ptr;
					}

					// Set new target waypoint
					update_states->target_WaypointNode_ptr = update_states->holding_pattern_WaypointNode_ptr->next_node_ptr;

					update_states->flight_phase = FLIGHT_PHASE_HOLD_IN_ARRIVAL_PATTERN; // Update new flight phase

					c_hold_flight_phase[index_flight] = FLIGHT_PHASE_GO_AROUND;
				}
			} else { // Geo-style
				if (update_states->flag_target_waypoint_change) {
					if ((update_states->target_WaypointNode_ptr != NULL) && (update_states->target_WaypointNode_ptr->prev_node_ptr != NULL)) {
						// Check the waypoint passed
						if (strcmp(update_states->target_WaypointNode_ptr->prev_node_ptr->phase, "APPROACH") == 0) {
							update_states->flight_phase = FLIGHT_PHASE_APPROACH;
						} else if (strcmp(update_states->target_WaypointNode_ptr->prev_node_ptr->phase, "FINAL_APPROACH") == 0) {
							update_states->flight_phase = FLIGHT_PHASE_FINAL_APPROACH;
						}
					}
				}
			}

			break;

		case FLIGHT_PHASE_TOUCHDOWN:
			if (ground_landing_data_init[index_flight]) {
				update_states->flight_phase = FLIGHT_PHASE_LAND;
			} else {
				update_states->durationSecond_to_be_proc = 0; // Reset
			}

			break;

		case FLIGHT_PHASE_LAND:
			if (update_states->target_WaypointNode_ptr != NULL) {
				if (update_states->flag_target_waypoint_change) {
					if (((!h_aircraft_soa.flag_geoStyle[index_flight]) && (strncasecmp(update_states->target_WaypointNode_ptr->wpname, "Txy", 3) == 0))
							||
						((h_aircraft_soa.flag_geoStyle[index_flight]) && (strncasecmp(update_states->target_WaypointNode_ptr->wptype, "Taxiway", 7) == 0))) {
						update_states->flight_phase = FLIGHT_PHASE_EXIT_RUNWAY;
					}
				}
			}

			break;

		case FLIGHT_PHASE_EXIT_RUNWAY:
			tmpHumanErrorEvent.type = PILOT_ERROR_EVENT_TYPE_CLEARANCE;
			tmpHumanErrorEvent.name = stringify(AIRCRAFT_CLEARANCE_TAXI_LANDING);

			if (update_states->target_WaypointNode_ptr != NULL) {
				if (((!h_aircraft_soa.flag_geoStyle[index_flight]) && (update_states->target_WaypointNode_ptr->prev_node_ptr != NULL) && (strncasecmp(update_states->target_WaypointNode_ptr->prev_node_ptr->wpname, "Txy", 3) == 0))
						||
					((h_aircraft_soa.flag_geoStyle[index_flight]) && (update_states->target_WaypointNode_ptr->prev_node_ptr != NULL) && (strncasecmp(update_states->target_WaypointNode_ptr->prev_node_ptr->wptype, "Taxiway", 7) == 0))) {
					if ((pilot_skip_request_clearance_aircraft(index_flight, tmpHumanErrorEvent)) || (pilot_check_clearance_aircraft(t, index_flight, tmpHumanErrorEvent) == 1)) {
						update_states->flight_phase = FLIGHT_PHASE_TAXI_ARRIVING;

						c_hold_flight_phase[index_flight] = FLIGHT_PHASE_PREDEPARTURE; // Set it as NULL value
					} else {
						c_hold_flight_phase[index_flight] = FLIGHT_PHASE_EXIT_RUNWAY;
					}
				}
			}

			if (c_hold_flight_phase[index_flight] == FLIGHT_PHASE_RAMP_ARRIVING) {
				if ((pilot_skip_request_clearance_aircraft(index_flight, tmpHumanErrorEvent)) || (pilot_check_clearance_aircraft(t, index_flight, tmpHumanErrorEvent) == 1)) {
					update_states->flight_phase = FLIGHT_PHASE_TAXI_ARRIVING;

					c_hold_flight_phase[index_flight] = FLIGHT_PHASE_PREDEPARTURE; // Set it as NULL value
				}
			}

			break;

		case FLIGHT_PHASE_TAXI_ARRIVING:
			tmpHumanErrorEvent.type = PILOT_ERROR_EVENT_TYPE_CLEARANCE;
			tmpHumanErrorEvent.name = stringify(AIRCRAFT_CLEARANCE_RAMP_LANDING);

			if (update_states->target_WaypointNode_ptr != NULL) {
				if (((!h_aircraft_soa.flag_geoStyle[index_flight]) && ((strncasecmp(update_states->target_WaypointNode_ptr->wpname, "Ramp", 4) == 0)
						|| (strncasecmp(update_states->target_WaypointNode_ptr->wpname, "Gate", 4) == 0)
						|| (strncasecmp(update_states->target_WaypointNode_ptr->wpname, "Spot", 4) == 0)
						|| (strncasecmp(update_states->target_WaypointNode_ptr->wpname, "Parking", 7) == 0)
						))
						||
					((h_aircraft_soa.flag_geoStyle[index_flight]) && ((strncasecmp(update_states->target_WaypointNode_ptr->wptype, "Ramp", 4) == 0)
						|| (strncasecmp(update_states->target_WaypointNode_ptr->wptype, "Gate", 4) == 0)
						|| (strncasecmp(update_states->target_WaypointNode_ptr->wptype, "Spot", 4) == 0)
						|| (strncasecmp(update_states->target_WaypointNode_ptr->wptype, "Parking", 7) == 0)
						))
					) {
					if ((pilot_skip_request_clearance_aircraft(index_flight, tmpHumanErrorEvent)) || (pilot_check_clearance_aircraft(t, index_flight, tmpHumanErrorEvent) == 1)) {
						update_states->flight_phase = FLIGHT_PHASE_RAMP_ARRIVING;

						c_hold_flight_phase[index_flight] = FLIGHT_PHASE_PREDEPARTURE; // Set it as NULL value
					} else {
						c_hold_flight_phase[index_flight] = FLIGHT_PHASE_TAXI_ARRIVING;
					}
				}
			}

			if (c_hold_flight_phase[index_flight] == FLIGHT_PHASE_TAXI_ARRIVING) {
				if ((pilot_skip_request_clearance_aircraft(index_flight, tmpHumanErrorEvent)) || (pilot_check_clearance_aircraft(t, index_flight, tmpHumanErrorEvent) == 1)) {
					update_states->flight_phase = FLIGHT_PHASE_RAMP_ARRIVING;

					c_hold_flight_phase[index_flight] = FLIGHT_PHASE_PREDEPARTURE; // Set it as NULL value
				}
			}

			break;

		case FLIGHT_PHASE_RUNWAY_CROSSING:
			if (update_states->flag_target_waypoint_change) {
				if (((!h_aircraft_soa.flag_geoStyle[index_flight]) && (update_states->target_WaypointNode_ptr != NULL)
						&& (strncasecmp(update_states->last_WaypointNode_ptr->wpname, "Ramp", 4) == 0))
						||
					((h_aircraft_soa.flag_geoStyle[index_flight]) && (update_states->target_WaypointNode_ptr != NULL)
						&& (strncasecmp(update_states->last_WaypointNode_ptr->wptype, "Ramp", 4) == 0))
					) {
					update_states->flight_phase = FLIGHT_PHASE_RAMP_ARRIVING;
				} else if (((!h_aircraft_soa.flag_geoStyle[index_flight]) && (update_states->last_WaypointNode_ptr != NULL) && (strncasecmp(update_states->last_WaypointNode_ptr->wpname, "Txy", 3) == 0))
								||
						   ((h_aircraft_soa.flag_geoStyle[index_flight]) && (update_states->last_WaypointNode_ptr != NULL) && (strncasecmp(update_states->last_WaypointNode_ptr->wptype, "Taxiway", 3) == 0))) {
					update_states->flight_phase = FLIGHT_PHASE_TAXI_ARRIVING;
				}
			}

			break;

		case FLIGHT_PHASE_RAMP_ARRIVING:
			if (update_states->target_WaypointNode_ptr == NULL) {
				update_states->flight_phase = FLIGHT_PHASE_DESTINATION_GATE;
			}

			break;

		case FLIGHT_PHASE_DESTINATION_GATE:

			break;
	}
}

void pilot_proc_Lat_Lon(const float t,
		update_states_t* update_states,
		const int index_flight,
		const float t_step_surface,
		const float t_step_terminal,
		const float t_step_airborne) {
	pilot_compute_Lat_Lon(update_states,
			index_flight);
}

void aircraft_proc_altitude(update_states_t* update_states,
		const int index_flight,
		const float t_step_surface,
		const float t_step_terminal,
		const float t_step_airborne) {
	aircraft_compute_altitude(update_states,
			index_flight,
			t_step_surface,
			t_step_terminal,
			t_step_airborne);
}

/**
 * Pilot module to execute pre-processing before triggering the main algorithm
 *
 * The purpose of this function is to prepare the data that is required by the main algorithm.
 *
 * Main algorithm may be triggered more than one time during the time step.  Pre-processing only runs once during the time step.
 */
void pilot_algorithm_pre_proc(update_states_t* update_states,
		const int index_flight,
		const float t,
		const float t_step_surface,
		const float t_step_terminal,
		const float t_step_airborne) {
	int tmpAdb_aircraft_type_index = c_adb_aircraft_type_index[index_flight];
	AdbOPFModel tmpAdbOPFModel = g_adb_opf_models.at(tmpAdb_aircraft_type_index);
	AdbPTFModel tmpAdbPTFModel = g_adb_ptf_models.at(tmpAdb_aircraft_type_index);

	if (ENUM_Flight_Phase_String[update_states->flight_phase] == skipFlightPhase[index_flight]) {
		for (unsigned int i = 0; i < ENUM_Flight_Phase_Count; i++) {
			if (ENUM_Flight_Phase_String[i] == skipFlightPhase[index_flight]) {
				update_states->flight_phase = ENUM_Flight_Phase(i + 1);
				break;
			}
		}
	}

	pair<double, double> tmpPairLatLon;

	switch (update_states->flight_phase) {
		case FLIGHT_PHASE_ORIGIN_GATE:

			break;

		case FLIGHT_PHASE_PUSHBACK:

			break;

		case FLIGHT_PHASE_RAMP_DEPARTING:

			break;

		case FLIGHT_PHASE_TAXI_DEPARTING:

			break;

		case FLIGHT_PHASE_RUNWAY_THRESHOLD_DEPARTING:

			break;

		case FLIGHT_PHASE_TAKEOFF:
			if ((h_aircraft_soa.estimate_takeoff_point_latitude_deg[index_flight] == 0) && (h_aircraft_soa.estimate_takeoff_point_longitude_deg[index_flight] == 0)) {
				if ((aircraftRunwayData.find(string(update_states->acid)) != aircraftRunwayData.end())
						&& (aircraftRunwayData.find(string(update_states->acid))->second.find(STRING_TAKEOFF) != aircraftRunwayData.find(string(update_states->acid))->second.end())) {
					tmpPairLatLon = aircraftRunwayData.at(string(update_states->acid)).at(STRING_TAKEOFF);
					h_aircraft_soa.estimate_takeoff_point_latitude_deg[index_flight] = tmpPairLatLon.first;
					h_aircraft_soa.estimate_takeoff_point_longitude_deg[index_flight] = tmpPairLatLon.second;
				}
			}

			update_states->L_takeoff = tmpAdbOPFModel.tol;

			update_states->V_stall_takeoff = tmpAdbOPFModel.vstall.at(TAKEOFF);
			update_states->V2_tas_knots = compute_true_speed_from_calibrated(MIN_SPEED_COEFFICIENT_TAKEOFF * update_states->V_stall_takeoff, 0);

			update_states->V_downWind = 0;

			update_states->acceleration = pow((update_states->V2_tas_knots - update_states->V_downWind)*KnotsToFps, 2) / (2 * update_states->L_takeoff);

			break;

		case FLIGHT_PHASE_CLIMBOUT:

			break;

		case FLIGHT_PHASE_HOLD_IN_DEPARTURE_PATTERN:

			break;

		case FLIGHT_PHASE_CLIMB_TO_CRUISE_ALTITUDE:

			break;

		case FLIGHT_PHASE_TOP_OF_CLIMB:

			break;

		case FLIGHT_PHASE_CRUISE:

			break;

		//case FLIGHT_PHASE_HOLD_IN_ENROUTE_PATTERN:
		//
		//	break;

		case FLIGHT_PHASE_TOP_OF_DESCENT:


			break;

		case FLIGHT_PHASE_INITIAL_DESCENT:

			break;

		case FLIGHT_PHASE_HOLD_IN_ARRIVAL_PATTERN:

			break;

		case FLIGHT_PHASE_APPROACH:

			break;

		case FLIGHT_PHASE_FINAL_APPROACH:

			break;

		case FLIGHT_PHASE_GO_AROUND:

			break;

		case FLIGHT_PHASE_TOUCHDOWN:

			break;

		case FLIGHT_PHASE_LAND:
			if (!ground_landing_data_init[index_flight]) {
				load_init_states_ground_landing(index_flight, update_states);

				update_states->course_rad_runway = compute_heading_rad_gc(update_states->landing_runway_entry_latitude,
						update_states->landing_runway_entry_longitude,
						update_states->landing_runway_end_latitude,
						update_states->landing_runway_end_longitude);
			}

			update_states->L_landing = tmpAdbOPFModel.ldl;

			update_states->V_stall_landing = tmpAdbOPFModel.vstall.at(LANDING);
			update_states->V_touchdown = compute_true_speed_from_calibrated(MIN_SPEED_COEFFICIENT_NOT_TAKEOFF * update_states->V_stall_landing, 0);

			update_states->acceleration = pow((update_states->V_touchdown - update_states->V_downWind)*KnotsToFps, 2) / (2 * update_states->L_landing);

			break;

		case FLIGHT_PHASE_EXIT_RUNWAY:
			update_states->L_re = compute_distance_gc(h_landing_taxi_plan.waypoint_node_ptr[index_flight]->latitude,
					h_landing_taxi_plan.waypoint_node_ptr[index_flight]->longitude,
					h_landing_taxi_plan.waypoint_node_ptr[index_flight]->next_node_ptr->latitude,
					h_landing_taxi_plan.waypoint_node_ptr[index_flight]->next_node_ptr->longitude,
					c_destination_airport_elevation_ft[index_flight]);

			break;

		case FLIGHT_PHASE_TAXI_ARRIVING:

			break;

		case FLIGHT_PHASE_RUNWAY_CROSSING:

			break;

		case FLIGHT_PHASE_RAMP_ARRIVING:

			break;

		case FLIGHT_PHASE_DESTINATION_GATE:

			break;

		case FLIGHT_PHASE_USER_INCIDENT:

			break;

		default:

			break;
	}

	if (update_states->target_WaypointNode_ptr != NULL) {
		if (update_states->target_WaypointNode_ptr == array_Airborne_Flight_Plan_Final_Node_ptr[index_flight]) {
			update_states->L_to_go = compute_distance_gc(update_states->lat,
				update_states->lon,
				c_estimate_touchdown_point_latitude_deg[index_flight],
				c_estimate_touchdown_point_longitude_deg[index_flight],
				update_states->target_WaypointNode_ptr->altitude_estimate);
		} else {
			update_states->L_to_go = compute_distance_gc(update_states->lat,
				update_states->lon,
				update_states->target_WaypointNode_ptr->latitude,
				update_states->target_WaypointNode_ptr->longitude,
				update_states->target_WaypointNode_ptr->altitude_estimate);

			if (update_states->flight_phase == FLIGHT_PHASE_GO_AROUND) {
				if (!h_aircraft_soa.flag_geoStyle[index_flight]) { // Not Geo-Style
					if (update_states->go_around_WaypointNode_ptr != NULL) {
						update_states->L_to_go = compute_distance_gc(update_states->lat,
										update_states->lon,
										update_states->go_around_WaypointNode_ptr->latitude,
										update_states->go_around_WaypointNode_ptr->longitude,
										update_states->go_around_WaypointNode_ptr->altitude_estimate);
					}
				}
			}
		}
	}
}

bool detect_userIncident(update_states_t* update_states,
		const int index_flight,
		const float t,
		const float t_step_surface,
		const float t_step_terminal,
		const float t_step_airborne) {
	bool retValue = false;

	if (update_states->flag_ifs_exist) {
		if ((update_states->simulation_user_incident_index+1 < incidentFlightPhaseMap.at(string(update_states->acid)).size()) && ((incidentFlightPhaseMap.at(string(update_states->acid)).at(update_states->simulation_user_incident_index+1).transition_time - update_states->t_processed_ifs) < update_states->elapsedSecond)) {
			// Calculate new elapsed period
			update_states->elapsedSecond = incidentFlightPhaseMap.at(string(update_states->acid)).at(update_states->simulation_user_incident_index+1).transition_time - update_states->t_processed_ifs;

			// Calculate new state at the transition time
			update_states->S_within_flightPhase_and_simCycle = update_states->V_ground * update_states->elapsedSecond;
			pilot_proc_Lat_Lon(t,
				update_states,
				index_flight,
				t_step_surface,
				t_step_terminal,
				t_step_airborne);

			update_states->altitude_ft = update_states->altitude_ft + update_states->rocd_fps * update_states->elapsedSecond;

			if ((g_trajectories.at(index_flight).origin_airport_elevation_ft == update_states->altitude_ft_bak)
					&& (update_states->altitude_ft < 0)) {
				update_states->altitude_ft = g_trajectories.at(index_flight).origin_airport_elevation_ft;
			} else if ((g_trajectories.at(index_flight).destination_airport_elevation_ft == update_states->altitude_ft_bak)
					&& (update_states->altitude_ft < 0)) {
				update_states->altitude_ft = g_trajectories.at(index_flight).destination_airport_elevation_ft;
			}

			if (update_states->altitude_ft < 0) {
				update_states->altitude_ft = 0;
			}
			// end - Calculate new state at the transition time

			update_states->durationSecond_to_be_proc = update_states->durationSecond_to_be_proc - update_states->elapsedSecond; // Update

			update_states->flight_phase = FLIGHT_PHASE_USER_INCIDENT;
			update_states->simulation_user_incident_index++;

			update_states->t_processed_ifs = update_states->t_processed_ifs + update_states->elapsedSecond;

			printf("Aircraft %s entering incident phase %s at t = %f due to %s\n", g_trajectories.at(index_flight).callsign.c_str(), incidentFlightPhaseMap.at(string(update_states->acid)).at(update_states->simulation_user_incident_index).getName().c_str(), update_states->t_processed_ifs, incidentFlightPhaseMap.at(string(update_states->acid)).at(update_states->simulation_user_incident_index).transitionCondition.c_str() );

			retValue = true; // User incident occurs
		}
	}

	return retValue;
}

/**
 * Main algorithm of flight phase: FLIGHT_PHASE_ORIGIN_GATE
 */
void flightPhase_algorithm_ORIGIN_GATE(update_states_t* update_states,
		const int index_flight,
		const float t,
		const float t_step_surface,
		const float t_step_terminal,
		const float t_step_airborne) {
	// Set speed
	update_states->tas_knots = 0.0;

	// Set Hdot
	update_states->rocd_fps = 0.0;

	// Set course angle
	update_states->hdg_rad = h_departing_taxi_plan.waypoint_node_ptr[index_flight]->course_rad_to_next_node;
}

/**
 * Main algorithm of flight phase: FLIGHT_PHASE_PUSHBACK
 */
void flightPhase_algorithm_PUSHBACK(update_states_t* update_states,
		const int index_flight,
		const float t,
		const float t_step_surface,
		const float t_step_terminal,
		const float t_step_airborne) {
	if (update_states->L_to_go > 0) {
		// Set speed
		update_states->tas_knots = h_departing_taxi_plan.ramp_tas_knots[index_flight];
		update_states->V_horizontal = update_states->tas_knots;

		aircraft_compute_heading(update_states, index_flight);

		update_states->V_ground = update_states->V_horizontal;

		// Moving distance during this time step
		update_states->S_within_flightPhase_and_simCycle = update_states->V_ground * KnotsToFps * update_states->durationSecond_to_be_proc;

		// If the moving distance is larger than the distance to the target waypoint, the aircraft will reach the target waypoint and even fly over it.
		// In this case, we calculate the elapsed time from "current location" to "target waypoint" then calculate the "durationSecond_to_be_proc"(remaining time step) we have to finish.
		if (update_states->L_to_go <= update_states->S_within_flightPhase_and_simCycle) {
			if (update_states->L_to_go < update_states->S_within_flightPhase_and_simCycle) {
				update_states->elapsedSecond = calculate_elapsedSecond(update_states->L_to_go / update_states->V_ground);

				// Calculate remaining duration to process
				update_states->durationSecond_to_be_proc = update_states->durationSecond_to_be_proc - update_states->elapsedSecond;

				update_states->S_within_flightPhase_and_simCycle = 0; // Reset
			} else {
				update_states->durationSecond_to_be_proc = 0;
			}

			if (update_states->target_WaypointNode_ptr != NULL) {
				update_states->lat = update_states->target_WaypointNode_ptr->latitude;
				update_states->lon = update_states->target_WaypointNode_ptr->longitude;
			}

			// Target waypoint increment
			update_states->target_waypoint_index++;
			update_states->last_WaypointNode_ptr = update_states->target_WaypointNode_ptr;
			if (update_states->target_WaypointNode_ptr != NULL) {
				update_states->target_WaypointNode_ptr = update_states->target_WaypointNode_ptr->next_node_ptr;
			}
			update_states->flag_target_waypoint_change = true;
		} else {
			update_states->durationSecond_to_be_proc = 0;
		}
	} else {
		if (update_states->target_WaypointNode_ptr != NULL) {
			update_states->lat = update_states->target_WaypointNode_ptr->latitude;
			update_states->lon = update_states->target_WaypointNode_ptr->longitude;
		}

		// Target waypoint increment
		update_states->target_waypoint_index++;
		update_states->last_WaypointNode_ptr = update_states->target_WaypointNode_ptr;
		if (update_states->target_WaypointNode_ptr != NULL) {
			update_states->target_WaypointNode_ptr = update_states->target_WaypointNode_ptr->next_node_ptr;
		}
		update_states->flag_target_waypoint_change = true;

		update_states->durationSecond_to_be_proc = 0;
	}
}

/**
 * Main algorithm of flight phase: FLIGHT_PHASE_RAMP_DEPARTING
 */
void flightPhase_algorithm_RAMP_DEPARTING(update_states_t* update_states,
		const int index_flight,
		const float t,
		const float t_step_surface,
		const float t_step_terminal,
		const float t_step_airborne) {
	if (update_states->L_to_go > 0) {
		// Set speed
		update_states->tas_knots = h_departing_taxi_plan.ramp_tas_knots[index_flight];
		update_states->V_horizontal = update_states->tas_knots;

		aircraft_compute_heading(update_states, index_flight);

		update_states->V_ground = update_states->V_horizontal;

		// Moving distance during this time step
		update_states->S_within_flightPhase_and_simCycle = update_states->V_ground * KnotsToFps * update_states->durationSecond_to_be_proc;

		// If the moving distance is larger than the distance to the target waypoint, the aircraft will reach the target waypoint and even fly over it.
		// In this case, we calculate the elapsed time from "current location" to "target waypoint" then calculate the "durationSecond_to_be_proc"(remaining time step) we have to finish.
		if (update_states->L_to_go <= update_states->S_within_flightPhase_and_simCycle) {
			if (update_states->L_to_go < update_states->S_within_flightPhase_and_simCycle) {
				update_states->elapsedSecond = calculate_elapsedSecond(update_states->L_to_go / update_states->V_ground);

				if (detect_userIncident(update_states,
						index_flight,
						t,
						t_step_surface,
						t_step_terminal,
						t_step_airborne))
					return;

				// Calculate remaining duration to process
				update_states->durationSecond_to_be_proc = update_states->durationSecond_to_be_proc - update_states->elapsedSecond;

				update_states->S_within_flightPhase_and_simCycle = 0; // Reset
			} else {
				update_states->elapsedSecond = update_states->durationSecond_to_be_proc;
				if (detect_userIncident(update_states,
						index_flight,
						t,
						t_step_surface,
						t_step_terminal,
						t_step_airborne))
					return;

				update_states->durationSecond_to_be_proc = 0;
			}

			// Set current location at the target waypoint
			if (update_states->target_WaypointNode_ptr != NULL) {
				update_states->lat = update_states->target_WaypointNode_ptr->latitude;
				update_states->lon = update_states->target_WaypointNode_ptr->longitude;
			}

			// Target waypoint increment
			update_states->target_waypoint_index++;
			update_states->last_WaypointNode_ptr = update_states->target_WaypointNode_ptr;
			if (update_states->target_WaypointNode_ptr != NULL) {
				update_states->target_WaypointNode_ptr = update_states->target_WaypointNode_ptr->next_node_ptr;
			}
			update_states->flag_target_waypoint_change = true;
		} else {
			update_states->elapsedSecond = update_states->durationSecond_to_be_proc;
			if (detect_userIncident(update_states,
					index_flight,
					t,
					t_step_surface,
					t_step_terminal,
					t_step_airborne))
				return;

			update_states->durationSecond_to_be_proc = 0;
		}
	} else {
		// Set current location at the target waypoint
		if (update_states->target_WaypointNode_ptr != NULL) {
			update_states->lat = update_states->target_WaypointNode_ptr->latitude;
			update_states->lon = update_states->target_WaypointNode_ptr->longitude;
		}

		// Target waypoint increment
		update_states->target_waypoint_index++;
		update_states->last_WaypointNode_ptr = update_states->target_WaypointNode_ptr;
		if (update_states->target_WaypointNode_ptr != NULL) {
			update_states->target_WaypointNode_ptr = update_states->target_WaypointNode_ptr->next_node_ptr;
		}
		update_states->flag_target_waypoint_change = true;

		update_states->durationSecond_to_be_proc = 0;
	}
}

void flightPhase_algorithm_TAXI_DEPARTING(update_states_t* update_states,
		const int index_flight,
		const float t,
		const float t_step_surface,
		const float t_step_terminal,
		const float t_step_airborne) {
	if (update_states->L_to_go > 0) {
		// Set speed
		update_states->tas_knots = h_departing_taxi_plan.taxi_tas_knots[index_flight];
		update_states->V_horizontal = update_states->tas_knots;

		aircraft_compute_heading(update_states, index_flight);

		update_states->V_ground = update_states->V_horizontal;

		// Moving distance during this time step
		update_states->S_within_flightPhase_and_simCycle = update_states->V_ground * KnotsToFps * update_states->durationSecond_to_be_proc;

		// If the moving distance is larger than the distance to the target waypoint, the aircraft will reach the target waypoint and even fly over it.
		// In this case, we calculate the elapsed time from "current location" to "target waypoint" then calculate the "durationSecond_to_be_proc"(remaining time step) we have to finish.
		if (update_states->L_to_go <= update_states->S_within_flightPhase_and_simCycle) {
			if (update_states->L_to_go < update_states->S_within_flightPhase_and_simCycle) {
				update_states->elapsedSecond = calculate_elapsedSecond(update_states->L_to_go / update_states->V_ground);

				if (detect_userIncident(update_states,
						index_flight,
						t,
						t_step_surface,
						t_step_terminal,
						t_step_airborne))
					return;

				// Calculate remaining duration to process
				update_states->durationSecond_to_be_proc = update_states->durationSecond_to_be_proc - update_states->elapsedSecond;

				update_states->S_within_flightPhase_and_simCycle = 0; // Reset
			} else {
				update_states->elapsedSecond = update_states->durationSecond_to_be_proc;
				if (detect_userIncident(update_states,
						index_flight,
						t,
						t_step_surface,
						t_step_terminal,
						t_step_airborne))
					return;

				update_states->durationSecond_to_be_proc = 0;
			}

			// Set current location at the target waypoint
			if (update_states->target_WaypointNode_ptr != NULL) {
				update_states->lat = update_states->target_WaypointNode_ptr->latitude;
				update_states->lon = update_states->target_WaypointNode_ptr->longitude;
			}

			// Target waypoint increment
			update_states->target_waypoint_index++;
			update_states->last_WaypointNode_ptr = update_states->target_WaypointNode_ptr;
			if (update_states->target_WaypointNode_ptr != NULL) {
				update_states->target_WaypointNode_ptr = update_states->target_WaypointNode_ptr->next_node_ptr;
			}
			update_states->flag_target_waypoint_change = true;

			// Reached the final node of departing taxi plan
			if ((h_departing_taxi_plan.waypoint_final_node_ptr[index_flight] != NULL)
					&& (update_states->last_WaypointNode_ptr == h_departing_taxi_plan.waypoint_final_node_ptr[index_flight])) {
				update_states->S_within_flightPhase_and_simCycle = 0; // Reset

				update_states->durationSecond_to_be_proc = 0;
			}
		} else {
			update_states->elapsedSecond = update_states->durationSecond_to_be_proc;
			if (detect_userIncident(update_states,
					index_flight,
					t,
					t_step_surface,
					t_step_terminal,
					t_step_airborne))
				return;

			update_states->durationSecond_to_be_proc = 0;
		}
	} else {
		// Set current location at the target waypoint
		if (update_states->target_WaypointNode_ptr != NULL) {
			update_states->lat = update_states->target_WaypointNode_ptr->latitude;
			update_states->lon = update_states->target_WaypointNode_ptr->longitude;
		}

		// Target waypoint increment
		update_states->target_waypoint_index++;
		update_states->last_WaypointNode_ptr = update_states->target_WaypointNode_ptr;
		if (update_states->target_WaypointNode_ptr != NULL) {
			update_states->target_WaypointNode_ptr = update_states->target_WaypointNode_ptr->next_node_ptr;
		}
		update_states->flag_target_waypoint_change = true;

		update_states->durationSecond_to_be_proc = 0;
	}
}

void flightPhase_algorithm_RUNWAY_THRESHOLD_DEPARTING(update_states_t* update_states,
		const int index_flight,
		const float t,
		const float t_step_surface,
		const float t_step_terminal,
		const float t_step_airborne) {
	double startPoint_lat = 0.0;
	double startPoint_lon = 0.0;
	double endPoint_lat = 0.0;
	double endPoint_lon = 0.0;

	map<string, AirportNode> map_waypoint_node;
	map<string, AirportNode>::iterator ite_map_waypoint_node;

	// Calculate course heading of the runway
	if ((!h_aircraft_soa.flag_geoStyle[index_flight]) && (h_departing_taxi_plan.runway_name[index_flight] != NULL)) {
		map_waypoint_node = map_ground_waypoint_connectivity.at(g_trajectories.at(index_flight).origin_airport).map_waypoint_node;

		for (ite_map_waypoint_node = map_waypoint_node.begin(); ite_map_waypoint_node != map_waypoint_node.end(); ite_map_waypoint_node++) {
			if ((!ite_map_waypoint_node->second.refName1.empty()) && (h_departing_taxi_plan.runway_name[index_flight] != NULL) && (strlen(h_departing_taxi_plan.runway_name[index_flight]) > 0) && (strcmp(ite_map_waypoint_node->second.refName1.c_str(), h_departing_taxi_plan.runway_name[index_flight]) == 0)
					&& (!ite_map_waypoint_node->second.type1.empty()) && (strcmp(ite_map_waypoint_node->second.type1.c_str(), "Entry") == 0)) {
				startPoint_lat = ite_map_waypoint_node->second.latitude;
				startPoint_lon = ite_map_waypoint_node->second.longitude;
			} else if ((!ite_map_waypoint_node->second.refName2.empty()) && (h_departing_taxi_plan.runway_name[index_flight] != NULL) && (strlen(h_departing_taxi_plan.runway_name[index_flight]) > 0) && (strcmp(ite_map_waypoint_node->second.refName2.c_str(), h_departing_taxi_plan.runway_name[index_flight]) == 0)
					&& (!ite_map_waypoint_node->second.type2.empty()) && (strcmp(ite_map_waypoint_node->second.type2.c_str(), "End") == 0)) {
				endPoint_lat = ite_map_waypoint_node->second.latitude;
				endPoint_lon = ite_map_waypoint_node->second.longitude;
			}

			if ((startPoint_lat >= 0) && (startPoint_lon)
					&& (endPoint_lat >= 0) && (endPoint_lon))
				break;
		}
	} else if ((h_aircraft_soa.flag_geoStyle[index_flight])
				&& (h_departing_taxi_plan.runway_entry_latitude_geoStyle[index_flight] != DBL_MAX)
				&& (h_departing_taxi_plan.runway_entry_longitude_geoStyle[index_flight] != DBL_MAX)
				&& (h_departing_taxi_plan.runway_end_latitude_geoStyle[index_flight] != DBL_MAX)
				&& (h_departing_taxi_plan.runway_end_longitude_geoStyle[index_flight] != DBL_MAX)
				) {
		startPoint_lat = h_departing_taxi_plan.runway_entry_latitude_geoStyle[index_flight];
		startPoint_lon = h_departing_taxi_plan.runway_entry_longitude_geoStyle[index_flight];
		endPoint_lat = h_departing_taxi_plan.runway_end_latitude_geoStyle[index_flight];
		endPoint_lon = h_departing_taxi_plan.runway_end_longitude_geoStyle[index_flight];
	}

	if ((startPoint_lat >= 0) && (startPoint_lon)
			&& (endPoint_lat >= 0) && (endPoint_lon)) {
		update_states->course_rad_runway = compute_heading_rad_gc(startPoint_lat, startPoint_lon,
				endPoint_lat, endPoint_lon);
		update_states->fpa_rad = 0;
	}
	// end - Calculate course heading of the runway

	update_states->hdg_rad = update_states->course_rad_runway;

	update_states->tas_knots = 0.0;
	update_states->V_horizontal = update_states->tas_knots;
	update_states->V_ground = update_states->tas_knots;

	update_states->durationSecond_to_be_proc = 0;

	if (array_Airborne_Flight_Plan_ptr[index_flight] != NULL) {
		// Target waypoint increment
		update_states->target_waypoint_index = 1;
		update_states->last_WaypointNode_ptr = NULL;
		update_states->target_WaypointNode_ptr = array_Airborne_Flight_Plan_ptr[index_flight]->next_node_ptr;
		update_states->flag_target_waypoint_change = true;
	}
}

void flightPhase_algorithm_TAKEOFF(update_states_t* update_states,
		const int index_flight,
		const float t,
		const float t_step_surface,
		const float t_step_terminal,
		const float t_step_airborne) {
	int tmpAdb_aircraft_type_index = c_adb_aircraft_type_index[index_flight];
	AdbPTFModel tmpAdbPTFModel = g_adb_ptf_models.at(tmpAdb_aircraft_type_index);

	if (update_states->rocd_fps == 0) {

		update_states->fpa_rad = 0;

		update_states->tas_knots = update_states->tas_knots + update_states->acceleration * update_states->durationSecond_to_be_proc;

		update_states->V_horizontal = update_states->tas_knots;

		aircraft_compute_heading(update_states, index_flight);

		update_states->V_ground = update_states->V_horizontal;

		// Moving distance during this time step
		update_states->S_within_flightPhase_and_simCycle = update_states->V_ground_bak * KnotsToFps * update_states->durationSecond_to_be_proc + 0.5 * update_states->acceleration * pow(update_states->durationSecond_to_be_proc, 2);
	} else {
		update_states->rocd_fps = tmpAdbPTFModel.getClimbRate(update_states->altitude_ft, LOW) / 60;
		update_states->tas_knots = get_CIFP_V(update_states, index_flight, true);
		if (update_states->tas_knots <= 0) {
			update_states->tas_knots = tmpAdbPTFModel.getClimbTas(update_states->altitude_ft);
		}

		update_states->fpa_rad = asin(update_states->rocd_fps / update_states->tas_knots);

		if (h_aircraft_soa.flag_geoStyle[index_flight]) {
			double tmpFpa_rad_wp = atan2(abs(update_states->target_WaypointNode_ptr->altitude_estimate - update_states->altitude_ft),
					compute_distance_gc(update_states->lat,
									update_states->lon,
									update_states->target_WaypointNode_ptr->latitude,
									update_states->target_WaypointNode_ptr->longitude,
									update_states->altitude_ft));
			double tmpRocd_fps_wp = update_states->tas_knots * sin(tmpFpa_rad_wp);

			if (abs(update_states->rocd_fps) > abs(tmpRocd_fps_wp)) {
				update_states->rocd_fps = tmpRocd_fps_wp;
				update_states->fpa_rad = tmpFpa_rad_wp;
			}
		}

		update_states->V_horizontal = update_states->tas_knots * cos(update_states->fpa_rad);

		aircraft_compute_heading(update_states, index_flight);

		update_states->V_ground = aircraft_compute_ground_speed(t,
				update_states->lat,
				update_states->lon,
				update_states->altitude_ft,
				update_states->V_horizontal,
				update_states->hdg_rad);

		// Moving distance during this time step
		update_states->S_within_flightPhase_and_simCycle = update_states->V_ground_bak * KnotsToFps * update_states->durationSecond_to_be_proc + 0.5 * update_states->acceleration * pow(update_states->durationSecond_to_be_proc, 2);
	}

	if (update_states->rocd_fps == 0) {
		// If no user-defined TAKEOFF point
		if ((h_aircraft_soa.estimate_takeoff_point_latitude_deg[index_flight] == 0) && (h_aircraft_soa.estimate_takeoff_point_longitude_deg[index_flight] == 0)) {
			if (update_states->tas_knots > update_states->V2_tas_knots) {
				update_states->rocd_fps = tmpAdbPTFModel.getClimbRate(update_states->altitude_ft, LOW) / 60;

				update_states->fpa_rad = asin(update_states->rocd_fps / update_states->tas_knots);
				update_states->V_horizontal = update_states->tas_knots * cos(update_states->fpa_rad);

				aircraft_compute_heading(update_states, index_flight);

				update_states->V_ground = aircraft_compute_ground_speed(t,
						update_states->lat,
						update_states->lon,
						update_states->altitude_ft,
						update_states->V_horizontal,
						update_states->hdg_rad);

				update_states->elapsedSecond = update_states->durationSecond_to_be_proc;
				if (detect_userIncident(update_states,
						index_flight,
						t,
						t_step_surface,
						t_step_terminal,
						t_step_airborne))
					return;

				// Moving distance during this time step
				update_states->S_within_flightPhase_and_simCycle = update_states->V_ground * update_states->durationSecond_to_be_proc;
			}

			update_states->durationSecond_to_be_proc = 0;
		} else { // There is user-defined TAKEOFF point
			double tmpDist_to_estimate_takeoff_point = compute_distance_gc(update_states->lat,
					update_states->lon,
					h_aircraft_soa.estimate_takeoff_point_latitude_deg[index_flight],
					h_aircraft_soa.estimate_takeoff_point_longitude_deg[index_flight],
					h_aircraft_soa.origin_airport_elevation_ft[index_flight]);

			if (tmpDist_to_estimate_takeoff_point < update_states->S_within_flightPhase_and_simCycle) {
				// Calculate duration from current position to the estimate TAKEOFF point
				double tmpV_estimate_takeoff_point = sqrt(pow(update_states->tas_knots_bak, 2) + 2 * update_states->acceleration * tmpDist_to_estimate_takeoff_point);

				update_states->elapsedSecond = calculate_elapsedSecond((tmpV_estimate_takeoff_point - update_states->tas_knots_bak) / update_states->acceleration);
				update_states->durationSecond_to_be_proc = update_states->durationSecond_to_be_proc - update_states->elapsedSecond;
				update_states->durationSecond_altitude_proc = update_states->durationSecond_to_be_proc; // Update

				update_states->tas_knots = tmpV_estimate_takeoff_point;

				if (tmpV_estimate_takeoff_point >= update_states->V2_tas_knots) {
					update_states->rocd_fps = tmpAdbPTFModel.getClimbRate(update_states->altitude_ft, LOW) / 60;

					update_states->fpa_rad = asin(update_states->rocd_fps / update_states->tas_knots);
					update_states->V_horizontal = update_states->tas_knots * cos(update_states->fpa_rad);

					update_states->V_ground = aircraft_compute_ground_speed(t,
							update_states->lat,
							update_states->lon,
							update_states->altitude_ft,
							update_states->V_horizontal,
							update_states->hdg_rad);

					if (detect_userIncident(update_states,
							index_flight,
							t,
							t_step_surface,
							t_step_terminal,
							t_step_airborne))
						return;

					// Update current location
					update_states->lat = h_aircraft_soa.estimate_takeoff_point_latitude_deg[index_flight];
					update_states->lon = h_aircraft_soa.estimate_takeoff_point_longitude_deg[index_flight];

					// Moving distance during this time step
					update_states->S_within_flightPhase_and_simCycle = update_states->V_ground * KnotsToFps * update_states->durationSecond_to_be_proc;
				} else {
					// Aircraft take off stall
					update_states->flight_phase = FLIGHT_PHASE_TAKEOFF_STALL;
					update_states->flag_abnormal_on_runway = true;

					update_states->durationSecond_to_be_proc = 0;

					printf("Aircraft %s take off stall\n", update_states->acid);
				}
			} else {
				update_states->elapsedSecond = update_states->durationSecond_to_be_proc;
				if (detect_userIncident(update_states,
						index_flight,
						t,
						t_step_surface,
						t_step_terminal,
						t_step_airborne))
					return;

				update_states->durationSecond_to_be_proc = 0;
			}
		}
	} else if ((update_states->rocd_fps != 0)
				&& (update_states->target_WaypointNode_ptr != NULL) && (indexOf(update_states->target_WaypointNode_ptr->wpname, "HEADING OR COURSE_TO_ALTITUDE") > -1) && (update_states->L_to_go <= update_states->S_within_flightPhase_and_simCycle)) {
		if (update_states->L_to_go < update_states->S_within_flightPhase_and_simCycle) {
			update_states->elapsedSecond = calculate_elapsedSecond(update_states->L_to_go / update_states->V_ground);

			if (detect_userIncident(update_states,
					index_flight,
					t,
					t_step_surface,
					t_step_terminal,
					t_step_airborne))
				return;

			// Calculate remaining duration to process
			update_states->durationSecond_to_be_proc = update_states->durationSecond_to_be_proc - update_states->elapsedSecond;
			update_states->durationSecond_altitude_proc = update_states->durationSecond_to_be_proc; // Update

			// Calculate new altitude
			update_states->altitude_ft = update_states->altitude_ft + update_states->rocd_fps * update_states->elapsedSecond;
			update_states->altitude_ft_bak = update_states->altitude_ft;

			update_states->S_within_flightPhase_and_simCycle = 0; // Reset
		} else {
			update_states->elapsedSecond = update_states->durationSecond_to_be_proc;
			if (detect_userIncident(update_states,
					index_flight,
					t,
					t_step_surface,
					t_step_terminal,
					t_step_airborne))
				return;

			update_states->durationSecond_to_be_proc = 0;
		}

		// Set current location at the target waypoint
		if (update_states->target_WaypointNode_ptr != NULL) {
			update_states->lat = update_states->target_WaypointNode_ptr->latitude;
			update_states->lon = update_states->target_WaypointNode_ptr->longitude;
		}

		// Target waypoint increment
		update_states->target_waypoint_index++;
		update_states->last_WaypointNode_ptr = update_states->target_WaypointNode_ptr;
		if (update_states->target_WaypointNode_ptr != NULL) {
			update_states->target_WaypointNode_ptr = update_states->target_WaypointNode_ptr->next_node_ptr;
		}
		update_states->flag_target_waypoint_change = true;

		c_flag_reached_meterfix_point[index_flight] = true;
	} else if ((update_states->rocd_fps != 0)
				&& (update_states->target_WaypointNode_ptr != NULL) && (indexOf(update_states->target_WaypointNode_ptr->wpname, "HEADING OR COURSE_TO_ALTITUDE") < 0) && (update_states->altitude_ft > c_origin_airport_elevation_ft[index_flight] + 1000)) {
		c_flag_reached_meterfix_point[index_flight] = true;

		if (update_states->L_to_go < update_states->S_within_flightPhase_and_simCycle) {
			update_states->elapsedSecond = calculate_elapsedSecond(update_states->L_to_go / update_states->V_ground);

			if (detect_userIncident(update_states,
					index_flight,
					t,
					t_step_surface,
					t_step_terminal,
					t_step_airborne))
				return;

			// Calculate remaining duration to process
			update_states->durationSecond_to_be_proc = update_states->durationSecond_to_be_proc - update_states->elapsedSecond;
			update_states->durationSecond_altitude_proc = update_states->durationSecond_to_be_proc; // Update

			// Calculate new altitude
			update_states->altitude_ft = update_states->altitude_ft + update_states->rocd_fps * update_states->elapsedSecond;
			update_states->altitude_ft_bak = update_states->altitude_ft;

			update_states->S_within_flightPhase_and_simCycle = 0; // Reset

			// Set current location at the target waypoint
			if (update_states->target_WaypointNode_ptr != NULL) {
				update_states->lat = update_states->target_WaypointNode_ptr->latitude;
				update_states->lon = update_states->target_WaypointNode_ptr->longitude;
			}

			// Target waypoint increment
			update_states->target_waypoint_index++;
			update_states->last_WaypointNode_ptr = update_states->target_WaypointNode_ptr;
			if (update_states->target_WaypointNode_ptr != NULL) {
				update_states->target_WaypointNode_ptr = update_states->target_WaypointNode_ptr->next_node_ptr;
			}
			update_states->flag_target_waypoint_change = true;
		} else {
			update_states->elapsedSecond = update_states->durationSecond_to_be_proc;
			if (detect_userIncident(update_states,
					index_flight,
					t,
					t_step_surface,
					t_step_terminal,
					t_step_airborne))
				return;

			update_states->durationSecond_to_be_proc = 0;
		}
	} else {
		update_states->elapsedSecond = update_states->durationSecond_to_be_proc;
		if (detect_userIncident(update_states,
				index_flight,
				t,
				t_step_surface,
				t_step_terminal,
				t_step_airborne))
			return;

		update_states->durationSecond_to_be_proc = 0;
	}
}

void flightPhase_algorithm_TAKEOFF_no_taxi_plan(update_states_t* update_states,
		const int index_flight,
		const float t,
		const float t_step_surface,
		const float t_step_terminal,
		const float t_step_airborne) {
	int tmpAdb_aircraft_type_index = c_adb_aircraft_type_index[index_flight];
	AdbPTFModel tmpAdbPTFModel = g_adb_ptf_models.at(tmpAdb_aircraft_type_index);

	if (update_states->rocd_fps == 0) {
		update_states->tas_knots = update_states->tas_knots + update_states->acceleration * update_states->durationSecond_to_be_proc;

		update_states->V_horizontal = update_states->tas_knots;

		aircraft_compute_heading(update_states, index_flight);

		update_states->V_ground = update_states->V_horizontal;

		// Moving distance during this time step
		update_states->S_within_flightPhase_and_simCycle = update_states->V_ground_bak * KnotsToFps * update_states->durationSecond_to_be_proc + 0.5 * update_states->acceleration * pow(update_states->durationSecond_to_be_proc, 2);
	} else {
		update_states->rocd_fps = tmpAdbPTFModel.getClimbRate(update_states->altitude_ft, LOW) / 60;

		update_states->tas_knots = get_CIFP_V(update_states, index_flight, true);
		if (update_states->tas_knots <= 0) {
			update_states->tas_knots = tmpAdbPTFModel.getClimbTas(update_states->altitude_ft);
		}

		update_states->fpa_rad = asin(update_states->rocd_fps / update_states->tas_knots);
		update_states->V_horizontal = update_states->tas_knots * cos(update_states->fpa_rad);

		aircraft_compute_heading(update_states, index_flight);

		update_states->V_ground = aircraft_compute_ground_speed(t,
				update_states->lat,
				update_states->lon,
				update_states->altitude_ft,
				update_states->V_horizontal,
				update_states->hdg_rad);

		// Moving distance during this time step
		update_states->S_within_flightPhase_and_simCycle = update_states->V_ground_bak * KnotsToFps * update_states->durationSecond_to_be_proc + 0.5 * update_states->acceleration * pow(update_states->durationSecond_to_be_proc, 2);
	}

	// Reach V2 speed
	if ((update_states->rocd_fps == 0) && (update_states->tas_knots > update_states->V2_tas_knots)) {
		update_states->rocd_fps = tmpAdbPTFModel.getClimbRate(update_states->altitude_ft, LOW) / 60;

		update_states->fpa_rad = asin(update_states->rocd_fps / update_states->tas_knots);
		update_states->V_horizontal = update_states->tas_knots * cos(update_states->fpa_rad);

		aircraft_compute_heading(update_states, index_flight);

		update_states->V_ground = aircraft_compute_ground_speed(t,
				update_states->lat,
				update_states->lon,
				update_states->altitude_ft,
				update_states->V_horizontal,
				update_states->hdg_rad);

		// Moving distance during this time step
		update_states->S_within_flightPhase_and_simCycle = update_states->V_ground * KnotsToFps * update_states->durationSecond_to_be_proc;

		update_states->durationSecond_to_be_proc = 0;
	} else if ((update_states->rocd_fps != 0)
				&& (update_states->target_WaypointNode_ptr != NULL) && (indexOf(update_states->target_WaypointNode_ptr->wpname, "HEADING OR COURSE_TO_ALTITUDE") > -1) && (update_states->L_to_go <= update_states->S_within_flightPhase_and_simCycle)) {
		if (update_states->L_to_go < update_states->S_within_flightPhase_and_simCycle) {
			update_states->elapsedSecond = calculate_elapsedSecond(update_states->L_to_go / update_states->V_ground);

			// Calculate remaining duration to process
			update_states->durationSecond_to_be_proc = update_states->durationSecond_to_be_proc - update_states->elapsedSecond;
			update_states->durationSecond_altitude_proc = update_states->durationSecond_to_be_proc; // Update

			update_states->S_within_flightPhase_and_simCycle = 0; // Reset
		} else {
			update_states->durationSecond_to_be_proc = 0;
		}

		// Set current location at the target waypoint
		if (update_states->target_WaypointNode_ptr != NULL) {
			update_states->lat = update_states->target_WaypointNode_ptr->latitude;
			update_states->lon = update_states->target_WaypointNode_ptr->longitude;
		}

		// Target waypoint increment
		update_states->target_waypoint_index++;
		update_states->last_WaypointNode_ptr = update_states->target_WaypointNode_ptr;
		if (update_states->target_WaypointNode_ptr != NULL) {
			update_states->target_WaypointNode_ptr = update_states->target_WaypointNode_ptr->next_node_ptr;
		}
		update_states->flag_target_waypoint_change = true;

		c_flag_reached_meterfix_point[index_flight] = true;
	} else if ((update_states->rocd_fps != 0)
				&& (update_states->target_WaypointNode_ptr != NULL) && (indexOf(update_states->target_WaypointNode_ptr->wpname, "HEADING OR COURSE_TO_ALTITUDE") < 0) && (update_states->altitude_ft > c_origin_airport_elevation_ft[index_flight] + 1000)) {
		c_flag_reached_meterfix_point[index_flight] = true;

		// If the moving distance is larger than the distance to the target waypoint, the aircraft will reach the target waypoint and even fly over it.
		// In this case, we calculate the elapsed time from "current location" to "target waypoint" then calculate the "durationSecond_to_be_proc"(remaining time step) we have to finish.
		if (update_states->L_to_go < update_states->S_within_flightPhase_and_simCycle) {
			update_states->elapsedSecond = calculate_elapsedSecond(update_states->L_to_go / update_states->V_ground);

			// Calculate remaining duration to process
			update_states->durationSecond_to_be_proc = update_states->durationSecond_to_be_proc - update_states->elapsedSecond;
			update_states->durationSecond_altitude_proc = update_states->durationSecond_to_be_proc; // Update

			update_states->S_within_flightPhase_and_simCycle = 0; // Reset

			// Set current location at the target waypoint
			if (update_states->target_WaypointNode_ptr != NULL) {
				update_states->lat = update_states->target_WaypointNode_ptr->latitude;
				update_states->lon = update_states->target_WaypointNode_ptr->longitude;
			}

			// Target waypoint increment
			update_states->target_waypoint_index++;
			update_states->last_WaypointNode_ptr = update_states->target_WaypointNode_ptr;
			if (update_states->target_WaypointNode_ptr != NULL) {
				update_states->target_WaypointNode_ptr = update_states->target_WaypointNode_ptr->next_node_ptr;
			}
			update_states->flag_target_waypoint_change = true;
		} else {
			update_states->durationSecond_to_be_proc = 0;
		}
	} else {
		update_states->durationSecond_to_be_proc = 0;
	}
}

void flightPhase_algorithm_CLIMBOUT(update_states_t* update_states,
		const int index_flight,
		const float t,
		const float t_step_surface,
		const float t_step_terminal,
		const float t_step_airborne) {
	int tmpAdb_aircraft_type_index = c_adb_aircraft_type_index[index_flight];
	AdbPTFModel tmpAdbPTFModel = g_adb_ptf_models.at(tmpAdb_aircraft_type_index);

	update_states->tas_knots = get_CIFP_V(update_states, index_flight, true);
	if (update_states->tas_knots <= 0) {
		update_states->tas_knots = tmpAdbPTFModel.getClimbTas(update_states->altitude_ft);
	}

	if ((update_states->altitude_ft + update_states->rocd_fps_bak * update_states->durationSecond_to_be_proc) > TRACON_ALT_FT) {
		update_states->rocd_fps = (TRACON_ALT_FT - update_states->altitude_ft) / update_states->durationSecond_to_be_proc;
	} else {
		update_states->rocd_fps = tmpAdbPTFModel.getClimbRate(update_states->altitude_ft, LOW) / 60;
	}

	update_states->fpa_rad = asin(update_states->rocd_fps / update_states->tas_knots);

	if (h_aircraft_soa.flag_geoStyle[index_flight]) {
		double tmpFpa_rad_wp = atan2(abs(update_states->target_WaypointNode_ptr->altitude_estimate - update_states->altitude_ft),
				compute_distance_gc(update_states->lat,
								update_states->lon,
								update_states->target_WaypointNode_ptr->latitude,
								update_states->target_WaypointNode_ptr->longitude,
								update_states->altitude_ft));
		double tmpRocd_fps_wp = update_states->tas_knots * sin(tmpFpa_rad_wp);

		if (abs(update_states->rocd_fps) > abs(tmpRocd_fps_wp)) {
			update_states->rocd_fps = tmpRocd_fps_wp;
			update_states->fpa_rad = tmpFpa_rad_wp;
		}
	}

	update_states->V_horizontal = update_states->tas_knots * cos(update_states->fpa_rad);

	aircraft_compute_heading(update_states, index_flight);

	update_states->V_ground = aircraft_compute_ground_speed(t,
				update_states->lat,
				update_states->lon,
				update_states->altitude_ft,
				update_states->V_horizontal,
				update_states->hdg_rad);

	// Moving distance during this time step
	update_states->S_within_flightPhase_and_simCycle = update_states->V_ground * KnotsToFps * update_states->durationSecond_to_be_proc;

	// If the moving distance is larger than the distance to the target waypoint, the aircraft will reach the target waypoint and even fly over it.
	// In this case, we calculate the elapsed time from "current location" to "target waypoint" then calculate the "durationSecond_to_be_proc"(remaining time step) we have to finish.
	if (update_states->L_to_go <= update_states->S_within_flightPhase_and_simCycle) {
		if (update_states->L_to_go < update_states->S_within_flightPhase_and_simCycle) {
			update_states->elapsedSecond = calculate_elapsedSecond(update_states->L_to_go / update_states->V_ground);

			if (detect_userIncident(update_states,
					index_flight,
					t,
					t_step_surface,
					t_step_terminal,
					t_step_airborne))
				return;

			// Calculate remaining duration to process
			update_states->durationSecond_to_be_proc = update_states->durationSecond_to_be_proc - update_states->elapsedSecond;
			update_states->durationSecond_altitude_proc = update_states->durationSecond_to_be_proc; // Update

			// Calculate new altitude
			update_states->altitude_ft = update_states->altitude_ft + update_states->rocd_fps * update_states->elapsedSecond;
			update_states->altitude_ft_bak = update_states->altitude_ft;

			update_states->S_within_flightPhase_and_simCycle = 0; // Reset
		} else {
			update_states->durationSecond_to_be_proc = 0;
		}

		// Set current location at the target waypoint
		if (update_states->target_WaypointNode_ptr != NULL) {
			update_states->lat = update_states->target_WaypointNode_ptr->latitude;
			update_states->lon = update_states->target_WaypointNode_ptr->longitude;
		}

		// Target waypoint increment
		update_states->target_waypoint_index++;
		update_states->last_WaypointNode_ptr = update_states->target_WaypointNode_ptr;
		if (update_states->target_WaypointNode_ptr != NULL) {
			update_states->target_WaypointNode_ptr = update_states->target_WaypointNode_ptr->next_node_ptr;
		}
		update_states->flag_target_waypoint_change = true;
	} else {
		update_states->elapsedSecond = update_states->durationSecond_to_be_proc;
		if (detect_userIncident(update_states,
				index_flight,
				t,
				t_step_surface,
				t_step_terminal,
				t_step_airborne))
			return;

		update_states->durationSecond_to_be_proc = 0;
	}
}

void flightPhase_algorithm_HOLD_IN_DEPARTURE_PATTERN(update_states_t* update_states,
		const int index_flight,
		const float t,
		const float t_step_surface,
		const float t_step_terminal,
		const float t_step_airborne) {
	update_states->fpa_rad = 0;

	update_states->rocd_fps = 0;

	update_states->V_horizontal = update_states->tas_knots;

	aircraft_compute_heading(update_states, index_flight);

	update_states->V_ground = aircraft_compute_ground_speed(t,
				update_states->lat,
				update_states->lon,
				update_states->altitude_ft,
				update_states->V_horizontal,
				update_states->hdg_rad);

	// Moving distance during this time step
	update_states->S_within_flightPhase_and_simCycle = update_states->V_ground * KnotsToFps * update_states->durationSecond_to_be_proc;

	// If the moving distance is larger than the distance to the target waypoint, the aircraft will reach the target waypoint and even fly over it.
	// In this case, we calculate the elapsed time from "current location" to "target waypoint" then calculate the "durationSecond_to_be_proc"(remaining time step) we have to finish.
	if (update_states->L_to_go <= update_states->S_within_flightPhase_and_simCycle) {
		if (update_states->L_to_go < update_states->S_within_flightPhase_and_simCycle) {
			update_states->elapsedSecond = calculate_elapsedSecond(update_states->L_to_go / update_states->V_ground);

			if (detect_userIncident(update_states,
					index_flight,
					t,
					t_step_surface,
					t_step_terminal,
					t_step_airborne))
				return;

			// Calculate remaining duration to process
			update_states->durationSecond_to_be_proc = update_states->durationSecond_to_be_proc - update_states->elapsedSecond;
			update_states->durationSecond_altitude_proc = update_states->durationSecond_to_be_proc; // Update

			// Calculate new altitude
			update_states->altitude_ft = update_states->altitude_ft + update_states->rocd_fps * update_states->elapsedSecond;
			update_states->altitude_ft_bak = update_states->altitude_ft;

			update_states->S_within_flightPhase_and_simCycle = 0; // Reset
		} else {
			update_states->durationSecond_to_be_proc = 0;
		}

		// Set current location at the target waypoint
		if (update_states->target_WaypointNode_ptr != NULL) {
			update_states->lat = update_states->target_WaypointNode_ptr->latitude;
			update_states->lon = update_states->target_WaypointNode_ptr->longitude;
		}

		// Target waypoint increment
		update_states->target_waypoint_index++;
		update_states->last_WaypointNode_ptr = update_states->target_WaypointNode_ptr;
		if (update_states->target_WaypointNode_ptr != NULL) {
			update_states->target_WaypointNode_ptr = update_states->target_WaypointNode_ptr->next_node_ptr;
		}
		update_states->flag_target_waypoint_change = true;
	} else {
		update_states->elapsedSecond = update_states->durationSecond_to_be_proc;
		if (detect_userIncident(update_states,
				index_flight,
				t,
				t_step_surface,
				t_step_terminal,
				t_step_airborne))
			return;

		update_states->durationSecond_to_be_proc = 0;
	}
}

void flightPhase_algorithm_CLIMB_TO_CRUISE_ALTITUDE(update_states_t* update_states,
		const int index_flight,
		const float t,
		const float t_step_surface,
		const float t_step_terminal,
		const float t_step_airborne) {
	int tmpAdb_aircraft_type_index = c_adb_aircraft_type_index[index_flight];
	AdbPTFModel tmpAdbPTFModel = g_adb_ptf_models.at(tmpAdb_aircraft_type_index);

	double altitude_waypoint;
	double roc_waypoint;

	update_states->tas_knots = get_CIFP_V(update_states, index_flight, true);
	if (update_states->tas_knots <= 0) {
		update_states->tas_knots = tmpAdbPTFModel.getClimbTas(update_states->altitude_ft);
	}

	int row_HighestAltitudeBelowCruiseAltitude = 0;
	for (int i = 0; i < tmpAdbPTFModel.getNumRows(); i++) {
		if (c_cruise_alt_ft[index_flight] <= tmpAdbPTFModel.getAltitude(i)) {
			row_HighestAltitudeBelowCruiseAltitude = i - 1;

			break;
		}
	}

	// Level off before reaching cruise altitude
	if ((tmpAdbPTFModel.getAltitude(row_HighestAltitudeBelowCruiseAltitude) < update_states->altitude_ft) && (update_states->altitude_ft < c_cruise_alt_ft[index_flight])) {
		// Get altitude of target waypoint
		if (update_states->target_waypoint_index == update_states->toc_index) {
			altitude_waypoint = update_states->cruise_alt_ft;
		} else {
			if (update_states->target_WaypointNode_ptr != NULL) {
				altitude_waypoint = update_states->target_WaypointNode_ptr->altitude_estimate;
			}
		}

		// Level-off Hdot calculation
		if ((update_states->altitude_ft + update_states->rocd_fps * update_states->durationSecond_to_be_proc) > c_cruise_alt_ft[index_flight]) {
			update_states->rocd_fps = (c_cruise_alt_ft[index_flight] - update_states->altitude_ft) / update_states->durationSecond_to_be_proc;
		} else {
			if (altitude_waypoint > 0) {
				roc_waypoint = tmpAdbPTFModel.getClimbRate(altitude_waypoint, LOW) / 60;

				if (altitude_waypoint > update_states->altitude_ft) {
					//update_states->rocd_fps = update_states->rocd_fps + ((tmp_t_step * update_states->rocd_fps) * (roc_waypoint - update_states->rocd_fps) / (altitude_waypoint - update_states->altitude_ft));
					update_states->rocd_fps = update_states->rocd_fps + ((update_states->durationSecond_to_be_proc * update_states->rocd_fps) * (roc_waypoint - update_states->rocd_fps) / (altitude_waypoint - update_states->altitude_ft));
				}
			}
		}
		// end - Level-off Hdot calculation
	} else {
		// Set Hdot
		update_states->rocd_fps = tmpAdbPTFModel.getClimbRate(update_states->altitude_ft, LOW) / 60;
	}

	update_states->fpa_rad = asin(update_states->rocd_fps / update_states->tas_knots);

	if (h_aircraft_soa.flag_geoStyle[index_flight]) {
		double tmpFpa_rad_wp = atan2(abs(update_states->target_WaypointNode_ptr->altitude_estimate - update_states->altitude_ft),
				compute_distance_gc(update_states->lat,
								update_states->lon,
								update_states->target_WaypointNode_ptr->latitude,
								update_states->target_WaypointNode_ptr->longitude,
								update_states->altitude_ft));
		double tmpRocd_fps_wp = update_states->tas_knots * sin(tmpFpa_rad_wp);

		if (abs(update_states->rocd_fps) > abs(tmpRocd_fps_wp)) {
			update_states->rocd_fps = tmpRocd_fps_wp;
			update_states->fpa_rad = tmpFpa_rad_wp;
		}
	}

	update_states->V_horizontal = update_states->tas_knots * cos(update_states->fpa_rad);

	aircraft_compute_heading(update_states, index_flight);

	update_states->V_ground = aircraft_compute_ground_speed(t,
				update_states->lat,
				update_states->lon,
				update_states->altitude_ft,
				update_states->V_horizontal,
				update_states->hdg_rad);

	// Moving distance during this time step
	update_states->S_within_flightPhase_and_simCycle = update_states->V_ground * KnotsToFps * update_states->durationSecond_to_be_proc;

	// If the moving distance is larger than the distance to the target waypoint, the aircraft will reach the target waypoint and even fly over it.
	// In this case, we calculate the elapsed time from "current location" to "target waypoint" then calculate the "durationSecond_to_be_proc"(remaining time step) we have to finish.
	if (update_states->L_to_go <= update_states->S_within_flightPhase_and_simCycle) {
		if (update_states->L_to_go < update_states->S_within_flightPhase_and_simCycle) {
			update_states->elapsedSecond = calculate_elapsedSecond(update_states->L_to_go / update_states->V_ground);

			if (detect_userIncident(update_states,
					index_flight,
					t,
					t_step_surface,
					t_step_terminal,
					t_step_airborne))
				return;

			// Calculate remaining duration to process
			update_states->durationSecond_to_be_proc = update_states->durationSecond_to_be_proc - update_states->elapsedSecond;
			update_states->durationSecond_altitude_proc = update_states->durationSecond_to_be_proc; // Update

			// Calculate new altitude
			update_states->altitude_ft = update_states->altitude_ft + update_states->rocd_fps * update_states->elapsedSecond;
			update_states->altitude_ft_bak = update_states->altitude_ft;

			update_states->S_within_flightPhase_and_simCycle = 0; // Reset
		} else {
			update_states->durationSecond_to_be_proc = 0;
		}

		// Set current location at the target waypoint
		if (update_states->target_WaypointNode_ptr != NULL) {
			update_states->lat = update_states->target_WaypointNode_ptr->latitude;
			update_states->lon = update_states->target_WaypointNode_ptr->longitude;
		}

		// Target waypoint increment
		update_states->target_waypoint_index++;
		update_states->last_WaypointNode_ptr = update_states->target_WaypointNode_ptr;
		if (update_states->target_WaypointNode_ptr != NULL) {
			update_states->target_WaypointNode_ptr = update_states->target_WaypointNode_ptr->next_node_ptr;
		}
		update_states->flag_target_waypoint_change = true;

		if (strcmp(update_states->last_WaypointNode_ptr->wpname, "TOP_OF_CLIMB_PT") == 0) {
			update_states->flight_phase = FLIGHT_PHASE_TOP_OF_CLIMB;
			update_states->altitude_ft = h_aircraft_soa.cruise_alt_ft[index_flight];
			update_states->rocd_fps = 0;
			update_states->fpa_rad = 0;
		}
	} else {
		update_states->elapsedSecond = update_states->durationSecond_to_be_proc;
		if (detect_userIncident(update_states,
				index_flight,
				t,
				t_step_surface,
				t_step_terminal,
				t_step_airborne))
			return;

		update_states->durationSecond_to_be_proc = 0;
	}
}

void flightPhase_algorithm_TOP_OF_CLIMB(update_states_t* update_states,
		const int index_flight,
		const float t,
		const float t_step_surface,
		const float t_step_terminal,
		const float t_step_airborne) {
	// Set Hdot
	update_states->rocd_fps = 0;

	// Set speed
	update_states->tas_knots = c_cruise_tas_knots[index_flight];

	update_states->V_horizontal = update_states->tas_knots;

	aircraft_compute_heading(update_states, index_flight);

	update_states->V_ground = aircraft_compute_ground_speed(t,
				update_states->lat,
				update_states->lon,
				update_states->altitude_ft,
				update_states->V_horizontal,
				update_states->hdg_rad);
}

void flightPhase_algorithm_CRUISE(update_states_t* update_states,
		const int index_flight,
		const float t,
		const float t_step_surface,
		const float t_step_terminal,
		const float t_step_airborne) {
	update_states->fpa_rad = 0;

	// Set Hdot
	update_states->rocd_fps = 0;

	// Set speed
	update_states->tas_knots = c_cruise_tas_knots[index_flight];

	update_states->V_horizontal = update_states->tas_knots;

	aircraft_compute_heading(update_states, index_flight);

	update_states->V_ground = aircraft_compute_ground_speed(t,
				update_states->lat,
				update_states->lon,
				update_states->altitude_ft,
				update_states->V_horizontal,
				update_states->hdg_rad);

	// Moving distance during this time step
	update_states->S_within_flightPhase_and_simCycle = update_states->V_ground * KnotsToFps * update_states->durationSecond_to_be_proc;

	// If the moving distance is larger than the distance to the target waypoint, the aircraft will reach the target waypoint and even fly over it.
	// In this case, we calculate the elapsed time from "current location" to "target waypoint" then calculate the "durationSecond_to_be_proc"(remaining time step) we have to finish.
	if (update_states->L_to_go <= update_states->S_within_flightPhase_and_simCycle) {
		if (update_states->L_to_go < update_states->S_within_flightPhase_and_simCycle) {
			update_states->elapsedSecond = calculate_elapsedSecond(update_states->L_to_go / update_states->V_ground);

			if (detect_userIncident(update_states,
					index_flight,
					t,
					t_step_surface,
					t_step_terminal,
					t_step_airborne))
				return;

			// Calculate remaining duration to process
			update_states->durationSecond_to_be_proc = update_states->durationSecond_to_be_proc - update_states->elapsedSecond;
			update_states->durationSecond_altitude_proc = update_states->durationSecond_to_be_proc; // Update

			update_states->S_within_flightPhase_and_simCycle = 0; // Reset
		} else {
			update_states->durationSecond_to_be_proc = 0;
		}

		// Set current location at the target waypoint
		if (update_states->target_WaypointNode_ptr != NULL) {
			update_states->lat = update_states->target_WaypointNode_ptr->latitude;
			update_states->lon = update_states->target_WaypointNode_ptr->longitude;
		}

		// Target waypoint increment
		update_states->target_waypoint_index++;
		update_states->last_WaypointNode_ptr = update_states->target_WaypointNode_ptr;
		if (update_states->target_WaypointNode_ptr != NULL) {
			update_states->target_WaypointNode_ptr = update_states->target_WaypointNode_ptr->next_node_ptr;
		}
		update_states->flag_target_waypoint_change = true;
	} else {
		update_states->elapsedSecond = update_states->durationSecond_to_be_proc;
		if (detect_userIncident(update_states,
				index_flight,
				t,
				t_step_surface,
				t_step_terminal,
				t_step_airborne))
			return;

		update_states->durationSecond_to_be_proc = 0;
	}
}

void flightPhase_algorithm_HOLD_IN_ENROUTE_PATTERN(update_states_t* update_states,
		const int index_flight,
		const float t,
		const float t_step_surface,
		const float t_step_terminal,
		const float t_step_airborne) {
	update_states->fpa_rad = 0;

	update_states->rocd_fps = 0;

	update_states->V_horizontal = update_states->tas_knots;

	aircraft_compute_heading(update_states, index_flight);

	update_states->V_ground = aircraft_compute_ground_speed(t,
				update_states->lat,
				update_states->lon,
				update_states->altitude_ft,
				update_states->V_horizontal,
				update_states->hdg_rad);

	// Moving distance during this time step
	update_states->S_within_flightPhase_and_simCycle = update_states->V_ground * KnotsToFps * update_states->durationSecond_to_be_proc;

	// If the moving distance is larger than the distance to the target waypoint, the aircraft will reach the target waypoint and even fly over it.
	// In this case, we calculate the elapsed time from "current location" to "target waypoint" then calculate the "durationSecond_to_be_proc"(remaining time step) we have to finish.
	if (update_states->L_to_go <= update_states->S_within_flightPhase_and_simCycle) {
		if (update_states->L_to_go < update_states->S_within_flightPhase_and_simCycle) {
			update_states->elapsedSecond = calculate_elapsedSecond(update_states->L_to_go / update_states->V_ground);

			if (detect_userIncident(update_states,
					index_flight,
					t,
					t_step_surface,
					t_step_terminal,
					t_step_airborne))
				return;

			// Calculate remaining duration to process
			update_states->durationSecond_to_be_proc = update_states->durationSecond_to_be_proc - update_states->elapsedSecond;
			update_states->durationSecond_altitude_proc = update_states->durationSecond_to_be_proc; // Update

			update_states->S_within_flightPhase_and_simCycle = 0; // Reset
		} else {
			update_states->durationSecond_to_be_proc = 0;
		}

		// Set current location at the target waypoint
		if (update_states->target_WaypointNode_ptr != NULL) {
			update_states->lat = update_states->target_WaypointNode_ptr->latitude;
			update_states->lon = update_states->target_WaypointNode_ptr->longitude;
		}

		// Target waypoint increment
		update_states->target_waypoint_index++;
		update_states->last_WaypointNode_ptr = update_states->target_WaypointNode_ptr;
		if (update_states->target_WaypointNode_ptr != NULL) {
			update_states->target_WaypointNode_ptr = update_states->target_WaypointNode_ptr->next_node_ptr;
		}
		update_states->flag_target_waypoint_change = true;
	} else {
		update_states->elapsedSecond = update_states->durationSecond_to_be_proc;
		if (detect_userIncident(update_states,
				index_flight,
				t,
				t_step_surface,
				t_step_terminal,
				t_step_airborne))
			return;

		update_states->durationSecond_to_be_proc = 0;
	}
}

void flightPhase_algorithm_TOP_OF_DESCENT(update_states_t* update_states,
		const int index_flight,
		const float t,
		const float t_step_surface,
		const float t_step_terminal,
		const float t_step_airborne) {
	if ((update_states->target_WaypointNode_ptr != NULL) && (update_states->target_WaypointNode_ptr->wpname != NULL) && (indexOf(update_states->target_WaypointNode_ptr->wpname, "TOP_OF_DESCENT_PT") == 0)) {
		// Set current location at the target waypoint
		if (update_states->target_WaypointNode_ptr != NULL) {
			update_states->lat = update_states->target_WaypointNode_ptr->latitude;
			update_states->lon = update_states->target_WaypointNode_ptr->longitude;
		}

		update_states->S_within_flightPhase_and_simCycle = 0; // Reset

		update_states->last_WaypointNode_ptr = update_states->target_WaypointNode_ptr;
		update_states->target_WaypointNode_ptr = update_states->target_WaypointNode_ptr->next_node_ptr;

		update_states->flag_target_waypoint_change = true;
	}
}

void flightPhase_algorithm_INITIAL_DESCENT(update_states_t* update_states,
		const int index_flight,
		const float t,
		const float t_step_surface,
		const float t_step_terminal,
		const float t_step_airborne) {
	int tmpAdb_aircraft_type_index = c_adb_aircraft_type_index[index_flight];
	AdbPTFModel tmpAdbPTFModel = g_adb_ptf_models.at(tmpAdb_aircraft_type_index);

	double rod_waypoint;

	int row_LowestAltitudeAboveTRACON = 0;
	for (int i = 0; i < tmpAdbPTFModel.getNumRows(); i++) {
		if (TRACON_ALT_FT < tmpAdbPTFModel.getAltitude(i)) {
			row_LowestAltitudeAboveTRACON = i;

			break;
		}
	}

	update_states->tas_knots = get_CIFP_V(update_states, index_flight, false);
	if (update_states->tas_knots <= 0) {
		update_states->tas_knots = tmpAdbPTFModel.getDescentTas(update_states->altitude_ft);
	}

	if (!h_aircraft_soa.flag_geoStyle[index_flight]) {
		if (update_states->altitude_ft < tmpAdbPTFModel.getAltitude(row_LowestAltitudeAboveTRACON)) {
			if ((TRACON_ALT_FT < update_states->altitude_ft) && ((update_states->altitude_ft + update_states->durationSecond_to_be_proc * update_states->rocd_fps_bak) < TRACON_ALT_FT)) {
				// Level off
				update_states->rocd_fps = (TRACON_ALT_FT - update_states->altitude_ft) / update_states->durationSecond_to_be_proc;
			} else {
				update_states->rocd_fps = (-1) * tmpAdbPTFModel.getDescentRate(update_states->altitude_ft, NOMINAL) / 60;
			}

			float tmp_rocd_over_speed = update_states->rocd_fps / update_states->tas_knots;
			if (tmp_rocd_over_speed > 1.0) {
				tmp_rocd_over_speed = 1.0;
			} else if (tmp_rocd_over_speed < -1.0) {
				tmp_rocd_over_speed = -1.0;
			}

			update_states->fpa_rad = asin((-1) * tmp_rocd_over_speed);
		} else { // Altitude higher than "lowest ADB table altitude above TRACON"
			if ((update_states->target_WaypointNode_ptr != NULL) && (update_states->target_WaypointNode_ptr->prev_node_ptr != NULL)) {
				update_states->fpa_rad = atan2(abs(update_states->target_WaypointNode_ptr->altitude_estimate - update_states->target_WaypointNode_ptr->prev_node_ptr->altitude_estimate),
					compute_distance_gc(update_states->target_WaypointNode_ptr->prev_node_ptr->latitude,
									update_states->target_WaypointNode_ptr->prev_node_ptr->longitude,
									update_states->target_WaypointNode_ptr->latitude,
									update_states->target_WaypointNode_ptr->longitude,
									update_states->target_WaypointNode_ptr->altitude_estimate));
			}

			update_states->rocd_fps = (-1) * tmpAdbPTFModel.getDescentRate(update_states->altitude_ft, NOMINAL) / 60;
		}
	} else {
		update_states->fpa_rad = atan2(abs(update_states->target_WaypointNode_ptr->altitude_estimate - update_states->altitude_ft),
							compute_distance_gc(update_states->lat,
											update_states->lon,
											update_states->target_WaypointNode_ptr->latitude,
											update_states->target_WaypointNode_ptr->longitude,
											update_states->altitude_ft));

		update_states->rocd_fps = (-1) * update_states->tas_knots * sin(update_states->fpa_rad);
	}

	update_states->V_horizontal = update_states->tas_knots * cos(update_states->fpa_rad);

	aircraft_compute_heading(update_states, index_flight);

	update_states->V_ground = aircraft_compute_ground_speed(t,
				update_states->lat,
				update_states->lon,
				update_states->altitude_ft,
				update_states->V_horizontal,
				update_states->hdg_rad);

	// Moving distance during this time step
	update_states->S_within_flightPhase_and_simCycle = update_states->V_ground * KnotsToFps * update_states->durationSecond_to_be_proc;

	// If the moving distance is larger than the distance to the target waypoint, the aircraft will reach the target waypoint and even fly over it.
	// In this case, we calculate the elapsed time from "current location" to "target waypoint" then calculate the "durationSecond_to_be_proc"(remaining time step) we have to finish.
	if (update_states->L_to_go <= update_states->S_within_flightPhase_and_simCycle) {
		if (update_states->L_to_go < update_states->S_within_flightPhase_and_simCycle) {
			update_states->elapsedSecond = calculate_elapsedSecond(update_states->L_to_go / update_states->V_ground);

			if (detect_userIncident(update_states,
					index_flight,
					t,
					t_step_surface,
					t_step_terminal,
					t_step_airborne))
				return;

			// Calculate remaining duration to process
			update_states->durationSecond_to_be_proc = update_states->durationSecond_to_be_proc - update_states->elapsedSecond;
			update_states->durationSecond_altitude_proc = update_states->durationSecond_to_be_proc; // Update

			// Calculate new altitude
			update_states->altitude_ft = update_states->altitude_ft + update_states->rocd_fps * update_states->elapsedSecond;
			update_states->altitude_ft_bak = update_states->altitude_ft;

			update_states->S_within_flightPhase_and_simCycle = 0; // Reset
		} else {
			update_states->durationSecond_to_be_proc = 0;
		}

		// Set current location at the target waypoint
		if (update_states->target_WaypointNode_ptr != NULL) {
			update_states->lat = update_states->target_WaypointNode_ptr->latitude;
			update_states->lon = update_states->target_WaypointNode_ptr->longitude;
		}

		// Target waypoint increment
		update_states->target_waypoint_index++;
		update_states->last_WaypointNode_ptr = update_states->target_WaypointNode_ptr;
		if (update_states->target_WaypointNode_ptr != NULL) {
			update_states->target_WaypointNode_ptr = update_states->target_WaypointNode_ptr->next_node_ptr;
		}
		update_states->flag_target_waypoint_change = true;
	} else {
		update_states->elapsedSecond = update_states->durationSecond_to_be_proc;
		if (detect_userIncident(update_states,
				index_flight,
				t,
				t_step_surface,
				t_step_terminal,
				t_step_airborne))
			return;

		update_states->durationSecond_to_be_proc = 0;
	}
}

void flightPhase_algorithm_INITIAL_DESCENT_final(update_states_t* update_states,
		const int index_flight,
		const float t,
		const float t_step_surface,
		const float t_step_terminal,
		const float t_step_airborne) {
	int tmpAdb_aircraft_type_index = c_adb_aircraft_type_index[index_flight];
	AdbPTFModel tmpAdbPTFModel = g_adb_ptf_models.at(tmpAdb_aircraft_type_index);

	double rod_waypoint;

	update_states->tas_knots = get_CIFP_V(update_states, index_flight, false);
	if (update_states->tas_knots <= 0) {
		update_states->tas_knots = tmpAdbPTFModel.getDescentTas(update_states->altitude_ft);
	}

	if ((update_states->target_WaypointNode_ptr != NULL) && (update_states->target_WaypointNode_ptr->prev_node_ptr != NULL)) {
		update_states->fpa_rad = atan2(abs(update_states->target_WaypointNode_ptr->altitude_estimate - update_states->target_WaypointNode_ptr->prev_node_ptr->altitude_estimate),
			compute_distance_gc(update_states->target_WaypointNode_ptr->prev_node_ptr->latitude,
							update_states->target_WaypointNode_ptr->prev_node_ptr->longitude,
							update_states->target_WaypointNode_ptr->latitude,
							update_states->target_WaypointNode_ptr->longitude,
							update_states->target_WaypointNode_ptr->altitude_estimate));
	}

	if ((update_states->target_WaypointNode_ptr != NULL)
			&& (update_states->target_WaypointNode_ptr->altitude_estimate > 0)
			&& (update_states->altitude_ft >= update_states->target_WaypointNode_ptr->altitude_estimate)
			&& ((update_states->altitude_ft + update_states->durationSecond_to_be_proc * update_states->rocd_fps_bak) < update_states->target_WaypointNode_ptr->altitude_estimate)) {
		// Level off
		update_states->rocd_fps = (update_states->target_WaypointNode_ptr->altitude_estimate - update_states->altitude_ft) / update_states->durationSecond_to_be_proc;
	} else {
		update_states->rocd_fps = (-1) * update_states->tas_knots * sin(update_states->fpa_rad);
	}

	update_states->V_horizontal = update_states->tas_knots * cos(update_states->fpa_rad);

	aircraft_compute_heading(update_states, index_flight);

	update_states->V_ground = aircraft_compute_ground_speed(t,
				update_states->lat,
				update_states->lon,
				update_states->altitude_ft,
				update_states->V_horizontal,
				update_states->hdg_rad);

	// Moving distance during this time step
	update_states->S_within_flightPhase_and_simCycle = update_states->V_ground * KnotsToFps * update_states->durationSecond_to_be_proc;

	// If the moving distance is larger than the distance to the target waypoint, the aircraft will reach the target waypoint and even fly over it.
	// In this case, we calculate the elapsed time from "current location" to "target waypoint" then calculate the "durationSecond_to_be_proc"(remaining time step) we have to finish.
	if (update_states->L_to_go <= update_states->S_within_flightPhase_and_simCycle) {
		if (update_states->L_to_go < update_states->S_within_flightPhase_and_simCycle) {
			update_states->elapsedSecond = calculate_elapsedSecond(update_states->L_to_go / update_states->V_ground);

			if (detect_userIncident(update_states,
					index_flight,
					t,
					t_step_surface,
					t_step_terminal,
					t_step_airborne))
				return;

			// Calculate remaining duration to process
			update_states->durationSecond_to_be_proc = update_states->durationSecond_to_be_proc - update_states->elapsedSecond;
			update_states->durationSecond_altitude_proc = update_states->durationSecond_to_be_proc; // Update

			// Calculate new altitude
			update_states->altitude_ft = update_states->altitude_ft + update_states->rocd_fps * update_states->elapsedSecond;
			update_states->altitude_ft_bak = update_states->altitude_ft;

			update_states->S_within_flightPhase_and_simCycle = 0;
		} else {
			update_states->durationSecond_to_be_proc = 0;
		}

		// Set current location at the target waypoint
		if (update_states->target_WaypointNode_ptr != NULL) {
			update_states->lat = update_states->target_WaypointNode_ptr->latitude;
			update_states->lon = update_states->target_WaypointNode_ptr->longitude;
		}

		// Target waypoint increment
		update_states->target_waypoint_index++;
		update_states->last_WaypointNode_ptr = update_states->target_WaypointNode_ptr;
		if (update_states->target_WaypointNode_ptr != NULL) {
			update_states->target_WaypointNode_ptr = update_states->target_WaypointNode_ptr->next_node_ptr;
		}
		update_states->flag_target_waypoint_change = true;
	} else {
		update_states->elapsedSecond = update_states->durationSecond_to_be_proc;
		if (detect_userIncident(update_states,
				index_flight,
				t,
				t_step_surface,
				t_step_terminal,
				t_step_airborne))
			return;

		update_states->durationSecond_to_be_proc = 0;
	}
}

void flightPhase_algorithm_HOLD_IN_ARRIVAL_PATTERN(update_states_t* update_states,
		const int index_flight,
		const float t,
		const float t_step_surface,
		const float t_step_terminal,
		const float t_step_airborne) {
	update_states->fpa_rad = 0;

	update_states->rocd_fps = 0;

	update_states->V_horizontal = update_states->tas_knots * cos(update_states->fpa_rad);

	aircraft_compute_heading(update_states, index_flight);

	update_states->V_ground = aircraft_compute_ground_speed(t,
				update_states->lat,
				update_states->lon,
				update_states->altitude_ft,
				update_states->V_horizontal,
				update_states->hdg_rad);

	// Moving distance during this time step
	update_states->S_within_flightPhase_and_simCycle = update_states->V_ground * KnotsToFps * update_states->durationSecond_to_be_proc;

	// If the moving distance is larger than the distance to the target waypoint, the aircraft will reach the target waypoint and even fly over it.
	// In this case, we calculate the elapsed time from "current location" to "target waypoint" then calculate the "durationSecond_to_be_proc"(remaining time step) we have to finish.
	if (update_states->L_to_go <= update_states->S_within_flightPhase_and_simCycle) {
		if (update_states->L_to_go < update_states->S_within_flightPhase_and_simCycle) {
			update_states->elapsedSecond = calculate_elapsedSecond(update_states->L_to_go / update_states->V_ground);

			if (detect_userIncident(update_states,
					index_flight,
					t,
					t_step_surface,
					t_step_terminal,
					t_step_airborne))
				return;

			// Calculate remaining duration to process
			update_states->durationSecond_to_be_proc = update_states->durationSecond_to_be_proc - update_states->elapsedSecond;
			update_states->durationSecond_altitude_proc = update_states->durationSecond_to_be_proc; // Update

			// Calculate new altitude
			update_states->altitude_ft = update_states->altitude_ft + update_states->rocd_fps * update_states->elapsedSecond;
			update_states->altitude_ft_bak = update_states->altitude_ft;

			update_states->S_within_flightPhase_and_simCycle = 0; // Reset
		} else {
			update_states->durationSecond_to_be_proc = 0;
		}

		// Set current location at the target waypoint
		if (update_states->target_WaypointNode_ptr != NULL) {
			update_states->lat = update_states->target_WaypointNode_ptr->latitude;
			update_states->lon = update_states->target_WaypointNode_ptr->longitude;
		}

		// Target waypoint increment
		update_states->target_waypoint_index++;
		update_states->last_WaypointNode_ptr = update_states->target_WaypointNode_ptr;
		if (update_states->target_WaypointNode_ptr != NULL) {
			update_states->target_WaypointNode_ptr = update_states->target_WaypointNode_ptr->next_node_ptr;
		}
		update_states->flag_target_waypoint_change = true;
	} else {
		update_states->elapsedSecond = update_states->durationSecond_to_be_proc;
		if (detect_userIncident(update_states,
				index_flight,
				t,
				t_step_surface,
				t_step_terminal,
				t_step_airborne))
			return;

		update_states->durationSecond_to_be_proc = 0;
	}
}

void flightPhase_algorithm_APPROACH(update_states_t* update_states,
		const int index_flight,
		const float t,
		const float t_step_surface,
		const float t_step_terminal,
		const float t_step_airborne) {
	int tmpAdb_aircraft_type_index = c_adb_aircraft_type_index[index_flight];
	AdbPTFModel tmpAdbPTFModel = g_adb_ptf_models.at(tmpAdb_aircraft_type_index);

	waypoint_node_t* waypoint_lower_alt;

	update_states->tas_knots = get_CIFP_V(update_states, index_flight, false);
	if (update_states->tas_knots <= 0) {
		update_states->tas_knots = tmpAdbPTFModel.getDescentTas(update_states->altitude_ft);
	}

	if (!h_aircraft_soa.flag_geoStyle[index_flight]) {
		if ((update_states->target_WaypointNode_ptr != NULL) && (update_states->target_WaypointNode_ptr->prev_node_ptr != NULL)) {
			waypoint_lower_alt = update_states->target_WaypointNode_ptr; // Set the initial waypoint at the target waypoint

			// Traverse each waypoint and find the waypoint with lower altitude than the current altitude
			while ((waypoint_lower_alt != NULL) && (waypoint_lower_alt->altitude_estimate >= update_states->altitude_ft)) {
				waypoint_lower_alt = waypoint_lower_alt->next_node_ptr;
			}

			if (waypoint_lower_alt != NULL) {
				update_states->fpa_rad = atan2((waypoint_lower_alt->altitude_estimate - update_states->altitude_ft),
												compute_distance_gc(update_states->lat,
																	update_states->lon,
																	waypoint_lower_alt->latitude,
																	waypoint_lower_alt->longitude,
																	waypoint_lower_alt->altitude_estimate));
			}
		}

		if ((update_states->target_WaypointNode_ptr != NULL)
				&& (update_states->target_WaypointNode_ptr->altitude_estimate > 0)
				&& (update_states->altitude_ft > update_states->target_WaypointNode_ptr->altitude_estimate)
				&& ((update_states->altitude_ft + update_states->durationSecond_to_be_proc * update_states->rocd_fps_bak) < update_states->target_WaypointNode_ptr->altitude_estimate)) {
			// Level off
			update_states->rocd_fps = (update_states->target_WaypointNode_ptr->altitude_estimate - update_states->altitude_ft) / (update_states->L_to_go / (update_states->tas_knots * cos(update_states->fpa_rad)));
		} else {
			update_states->rocd_fps = update_states->tas_knots * sin(update_states->fpa_rad);
		}
	} else {
		update_states->fpa_rad = atan2(abs(update_states->target_WaypointNode_ptr->altitude_estimate - update_states->altitude_ft),
							compute_distance_gc(update_states->lat,
											update_states->lon,
											update_states->target_WaypointNode_ptr->latitude,
											update_states->target_WaypointNode_ptr->longitude,
											update_states->altitude_ft));

		update_states->rocd_fps = (-1) * update_states->tas_knots * sin(update_states->fpa_rad);
	}

	update_states->V_horizontal = update_states->tas_knots * cos(update_states->fpa_rad);

	aircraft_compute_heading(update_states, index_flight);

	update_states->V_ground = aircraft_compute_ground_speed(t,
				update_states->lat,
				update_states->lon,
				update_states->altitude_ft,
				update_states->V_horizontal,
				update_states->hdg_rad);

	// Moving distance during this time step
	update_states->S_within_flightPhase_and_simCycle = update_states->V_ground * KnotsToFps * update_states->durationSecond_to_be_proc;

	// If the moving distance is larger than the distance to the target waypoint, the aircraft will reach the target waypoint and even fly over it.
	// In this case, we calculate the elapsed time from "current location" to "target waypoint" then calculate the "durationSecond_to_be_proc"(remaining time step) we have to finish.
	if (update_states->L_to_go <= update_states->S_within_flightPhase_and_simCycle) {
		if (update_states->L_to_go < update_states->S_within_flightPhase_and_simCycle) {
			update_states->elapsedSecond = calculate_elapsedSecond(update_states->L_to_go / update_states->V_ground);

			if (detect_userIncident(update_states,
					index_flight,
					t,
					t_step_surface,
					t_step_terminal,
					t_step_airborne))
				return;

			// Calculate remaining duration to process
			update_states->durationSecond_to_be_proc = update_states->durationSecond_to_be_proc - update_states->elapsedSecond;
			update_states->durationSecond_altitude_proc = update_states->durationSecond_to_be_proc; // Update

			// Calculate new altitude
			update_states->altitude_ft = update_states->altitude_ft + update_states->rocd_fps * update_states->elapsedSecond;
			update_states->altitude_ft_bak = update_states->altitude_ft;

			update_states->S_within_flightPhase_and_simCycle = 0; // Reset
		} else {
			update_states->durationSecond_to_be_proc = 0;
		}

		// Set current location at the target waypoint
		if (update_states->target_WaypointNode_ptr != NULL) {
			update_states->lat = update_states->target_WaypointNode_ptr->latitude;
			update_states->lon = update_states->target_WaypointNode_ptr->longitude;
		}

		// Target waypoint increment
		update_states->target_waypoint_index++;
		update_states->last_WaypointNode_ptr = update_states->target_WaypointNode_ptr;
		if (update_states->target_WaypointNode_ptr != NULL) {
			update_states->target_WaypointNode_ptr = update_states->target_WaypointNode_ptr->next_node_ptr;
		}
		update_states->flag_target_waypoint_change = true;
	} else {
		update_states->elapsedSecond = update_states->durationSecond_to_be_proc;
		if (detect_userIncident(update_states,
				index_flight,
				t,
				t_step_surface,
				t_step_terminal,
				t_step_airborne))
			return;

		update_states->durationSecond_to_be_proc = 0;
	}
}

void flightPhase_algorithm_FINAL_APPROACH(update_states_t* update_states,
		const int index_flight,
		const float t,
		const float t_step_surface,
		const float t_step_terminal,
		const float t_step_airborne) {
	int tmpAdb_aircraft_type_index = c_adb_aircraft_type_index[index_flight];
	AdbPTFModel tmpAdbPTFModel = g_adb_ptf_models.at(tmpAdb_aircraft_type_index);

	double altitude_waypoint;
	double rod_waypoint;

	double altitude_final_approach_fix = 0.0;
	double distance_finalApproachFix_to_touchdown = 0.0;

	update_states->tas_knots = get_CIFP_V(update_states, index_flight, false);
	if (update_states->tas_knots <= 0) {
		update_states->tas_knots = tmpAdbPTFModel.getDescentTas(update_states->altitude_ft);
	}

	if (!h_aircraft_soa.flag_geoStyle[index_flight]) { // Not Geo-style
		real_t num = -1.0;
		real_t den = -1.0;
		real_t range = -1.0;

		if ((update_states->target_WaypointNode_ptr == array_Airborne_Flight_Plan_Final_Node_ptr[index_flight]) && (c_estimate_touchdown_point_latitude_deg[index_flight] != 0) && (c_estimate_touchdown_point_longitude_deg[index_flight] != 0)) {
			num = c_destination_airport_elevation_ft[index_flight] - update_states->altitude_ft;
			den = 	compute_distance_gc(update_states->lat,
					update_states->lon,
					c_estimate_touchdown_point_latitude_deg[index_flight],
					c_estimate_touchdown_point_longitude_deg[index_flight],
					c_destination_airport_elevation_ft[index_flight]);
		} else {
			if (update_states->target_WaypointNode_ptr != NULL) {
				num = update_states->target_WaypointNode_ptr->altitude_estimate - update_states->altitude_ft;
				den = 	compute_distance_gc(update_states->lat,
						update_states->lon,
						update_states->target_WaypointNode_ptr->latitude,
						update_states->target_WaypointNode_ptr->longitude,
						update_states->target_WaypointNode_ptr->altitude_estimate);
			}
		}

		if ((num != -1) && (den != -1)) {
			update_states->fpa_rad = num / den;

			range = sqrt(pow(num, 2) + pow(den, 2));

			real_t rocd_fps_using_num_range = (num / range) * update_states->tas_knots;
			real_t V_horizontal_using_den_range = (den / range) * update_states->tas_knots;

			update_states->rocd_fps = rocd_fps_using_num_range;

			update_states->V_horizontal = V_horizontal_using_den_range;
		}
	} else { // Geo-style
		update_states->fpa_rad = atan2(abs(update_states->target_WaypointNode_ptr->altitude_estimate - update_states->altitude_ft),
							compute_distance_gc(update_states->lat,
											update_states->lon,
											update_states->target_WaypointNode_ptr->latitude,
											update_states->target_WaypointNode_ptr->longitude,
											update_states->altitude_ft));

		update_states->rocd_fps = (-1) * update_states->tas_knots * sin(update_states->fpa_rad);
	}

	aircraft_compute_heading(update_states, index_flight);

	update_states->V_ground = aircraft_compute_ground_speed(t,
				update_states->lat,
				update_states->lon,
				update_states->altitude_ft,
				update_states->V_horizontal,
				update_states->hdg_rad);

	// Moving distance during this time step
	update_states->S_within_flightPhase_and_simCycle = update_states->V_ground * KnotsToFps * update_states->durationSecond_to_be_proc;

	if (!h_aircraft_soa.flag_geoStyle[index_flight]) { // Not Geo-style
		if (array_Airborne_Flight_Plan_Final_Node_ptr[index_flight]->prev_node_ptr != NULL) {
			altitude_final_approach_fix = array_Airborne_Flight_Plan_Final_Node_ptr[index_flight]->prev_node_ptr->altitude_estimate;
		}

		if (0 < altitude_final_approach_fix) {
			distance_finalApproachFix_to_touchdown = compute_distance_gc(array_Airborne_Flight_Plan_Final_Node_ptr[index_flight]->prev_node_ptr->latitude,
					array_Airborne_Flight_Plan_Final_Node_ptr[index_flight]->prev_node_ptr->longitude,
					c_estimate_touchdown_point_latitude_deg[index_flight],
					c_estimate_touchdown_point_longitude_deg[index_flight],
					altitude_final_approach_fix);
		}

		if ((0 < altitude_final_approach_fix) && (update_states->altitude_ft < altitude_final_approach_fix)) {
			if ((update_states->altitude_ft + update_states->durationSecond_to_be_proc * update_states->rocd_fps_bak) < c_destination_airport_elevation_ft[index_flight]) {
				// Level off
				update_states->rocd_fps = (c_destination_airport_elevation_ft[index_flight] - update_states->altitude_ft) / update_states->durationSecond_to_be_proc;
			}
		}
	}

	// If the moving distance is larger than the distance to the target waypoint, the aircraft will reach the target waypoint and even fly over it.
	// In this case, we calculate the elapsed time from "current location" to "target waypoint" then calculate the "durationSecond_to_be_proc"(remaining time step) we have to finish.
	if (update_states->L_to_go <= update_states->S_within_flightPhase_and_simCycle) {
		if (update_states->L_to_go < update_states->S_within_flightPhase_and_simCycle) {
			update_states->elapsedSecond = calculate_elapsedSecond(update_states->L_to_go / update_states->V_ground);

			if (detect_userIncident(update_states,
					index_flight,
					t,
					t_step_surface,
					t_step_terminal,
					t_step_airborne))
				return;

			// Calculate remaining duration to process
			update_states->durationSecond_to_be_proc = update_states->durationSecond_to_be_proc - update_states->elapsedSecond;
			update_states->durationSecond_altitude_proc = update_states->durationSecond_to_be_proc; // Update

			// Calculate new altitude
			update_states->altitude_ft = update_states->altitude_ft + update_states->rocd_fps * update_states->elapsedSecond;
			update_states->altitude_ft_bak = update_states->altitude_ft;

			update_states->S_within_flightPhase_and_simCycle = 0; // Reset
		} else {
			update_states->durationSecond_to_be_proc = 0;
		}

		// Set target waypoint location or the touchdown location to the current location
		if (update_states->target_WaypointNode_ptr != NULL) {
			if (update_states->target_WaypointNode_ptr == array_Airborne_Flight_Plan_Final_Node_ptr[index_flight]) {
				update_states->lat = c_estimate_touchdown_point_latitude_deg[index_flight];
				update_states->lon = c_estimate_touchdown_point_longitude_deg[index_flight];

				// Calculate new altitude of the TOUCHDOWN point
				float dh = t_step_terminal * update_states->rocd_fps;
				update_states->altitude_ft += dh;
			} else {
				update_states->lat = update_states->target_WaypointNode_ptr->latitude;
				update_states->lon = update_states->target_WaypointNode_ptr->longitude;
			}
		}

		// Target waypoint increment
		update_states->target_waypoint_index++;
		update_states->last_WaypointNode_ptr = update_states->target_WaypointNode_ptr;
		if (update_states->target_WaypointNode_ptr != NULL) {
			update_states->target_WaypointNode_ptr = update_states->target_WaypointNode_ptr->next_node_ptr;
		}
		update_states->flag_target_waypoint_change = true;
	} else {
		update_states->elapsedSecond = update_states->durationSecond_to_be_proc;
		if (detect_userIncident(update_states,
				index_flight,
				t,
				t_step_surface,
				t_step_terminal,
				t_step_airborne))
			return;

		update_states->durationSecond_to_be_proc = 0;
	}
}

void flightPhase_algorithm_GO_AROUND(update_states_t* update_states,
		const int index_flight,
		const float t,
		const float t_step_surface,
		const float t_step_terminal,
		const float t_step_airborne) {
	int tmpAdb_aircraft_type_index = c_adb_aircraft_type_index[index_flight];
	AdbPTFModel tmpAdbPTFModel = g_adb_ptf_models.at(tmpAdb_aircraft_type_index);

	update_states->tas_knots = get_CIFP_V(update_states, index_flight, false);

	waypoint_node_t* tmp_waypoint_ptr = NULL;

	if (!h_aircraft_soa.flag_geoStyle[index_flight]) { // Not Geo-Style
		tmp_waypoint_ptr = update_states->go_around_WaypointNode_ptr;
	} else { // Geo-style
		tmp_waypoint_ptr = update_states->target_WaypointNode_ptr;
	}

	if (tmp_waypoint_ptr->altitude_estimate > update_states->altitude_ft) {
		if (update_states->tas_knots <= 0) {
			update_states->tas_knots = tmpAdbPTFModel.getClimbTas(update_states->altitude_ft);
		}
	} else if (tmp_waypoint_ptr->altitude_estimate < update_states->altitude_ft) {
		if (update_states->tas_knots <= 0) {
			update_states->tas_knots = tmpAdbPTFModel.getDescentTas(update_states->altitude_ft);
		}
	} else {
		if (update_states->tas_knots <= 0) {
			update_states->tas_knots = tmpAdbPTFModel.getClimbTas(update_states->altitude_ft);
		}
	}

	update_states->fpa_rad = atan2((update_states->target_WaypointNode_ptr->altitude_estimate - update_states->altitude_ft),
			compute_distance_gc(update_states->lat,
					update_states->lon,
					update_states->target_WaypointNode_ptr->latitude,
					update_states->target_WaypointNode_ptr->longitude,
					update_states->target_WaypointNode_ptr->altitude_estimate));

	update_states->rocd_fps = update_states->tas_knots * sin(update_states->fpa_rad);

	update_states->V_horizontal = update_states->tas_knots * cos(update_states->fpa_rad);

	aircraft_compute_heading(update_states, index_flight);

	update_states->V_ground = aircraft_compute_ground_speed(t,
				update_states->lat,
				update_states->lon,
				update_states->altitude_ft,
				update_states->V_horizontal,
				update_states->hdg_rad);

	// Moving distance during this time step
	update_states->S_within_flightPhase_and_simCycle = update_states->V_ground * KnotsToFps * update_states->durationSecond_to_be_proc;

	// If the moving distance is larger than the distance to the target waypoint, the aircraft will reach the target waypoint and even fly over it.
	// In this case, we calculate the elapsed time from "current location" to "target waypoint" then calculate the "durationSecond_to_be_proc"(remaining time step) we have to finish.
	if (update_states->L_to_go <= update_states->S_within_flightPhase_and_simCycle) {
		if (update_states->L_to_go < update_states->S_within_flightPhase_and_simCycle) {
			update_states->elapsedSecond = calculate_elapsedSecond(update_states->L_to_go / update_states->V_ground);

			if (detect_userIncident(update_states,
					index_flight,
					t,
					t_step_surface,
					t_step_terminal,
					t_step_airborne))
				return;

			// Calculate remaining duration to process
			update_states->durationSecond_to_be_proc = update_states->durationSecond_to_be_proc - update_states->elapsedSecond;
			update_states->durationSecond_altitude_proc = update_states->durationSecond_to_be_proc; // Update

			// Calculate new altitude
			update_states->altitude_ft = update_states->altitude_ft + update_states->rocd_fps * update_states->elapsedSecond;
			update_states->altitude_ft_bak = update_states->altitude_ft;

			update_states->S_within_flightPhase_and_simCycle = 0; // Reset
		} else {
			update_states->durationSecond_to_be_proc = 0;
		}

		// Set current location at the target waypoint
		if (update_states->target_WaypointNode_ptr != NULL) {
			update_states->lat = update_states->target_WaypointNode_ptr->latitude;
			update_states->lon = update_states->target_WaypointNode_ptr->longitude;
		}

		// Target waypoint increment
		update_states->target_waypoint_index++;
		update_states->last_WaypointNode_ptr = update_states->target_WaypointNode_ptr;
		if (update_states->target_WaypointNode_ptr != NULL) {
			update_states->target_WaypointNode_ptr = update_states->target_WaypointNode_ptr->next_node_ptr;
		}
		update_states->flag_target_waypoint_change = true;
	} else {
		update_states->elapsedSecond = update_states->durationSecond_to_be_proc;
		if (detect_userIncident(update_states,
				index_flight,
				t,
				t_step_surface,
				t_step_terminal,
				t_step_airborne))
			return;

		update_states->durationSecond_to_be_proc = 0;
	}
}

void flightPhase_algorithm_TOUCHDOWN(update_states_t* update_states,
		const int index_flight,
		const float t,
		const float t_step_surface,
		const float t_step_terminal,
		const float t_step_airborne) {
	int tmpAdb_aircraft_type_index = c_adb_aircraft_type_index[index_flight];
	AdbOPFModel tmpAdbOPFModel = g_adb_opf_models.at(tmpAdb_aircraft_type_index);

	double landing_length;
	double vstall_landing;
	double vTouchdown_tas_knots;

	landing_length = 1000 * METER_TO_NAUTICAL_MILE * tmpAdbOPFModel.ldl;

	vstall_landing = tmpAdbOPFModel.vstall.at(LANDING);
	vTouchdown_tas_knots = compute_true_speed_from_calibrated(MIN_SPEED_COEFFICIENT_NOT_TAKEOFF * vstall_landing, 0);

	update_states->tas_knots = vTouchdown_tas_knots;
	update_states->V_horizontal = update_states->tas_knots;

	double startPoint_lat = 0.0;
	double startPoint_lon = 0.0;
	double endPoint_lat = 0.0;
	double endPoint_lon = 0.0;

	map<string, AirportNode> map_waypoint_node;
	map<string, AirportNode>::iterator ite_map_waypoint_node;

	// Calculate course heading of runway
	if ((!h_aircraft_soa.flag_geoStyle[index_flight]) && (h_landing_taxi_plan.runway_name[index_flight] != NULL)) {
		map_waypoint_node = map_ground_waypoint_connectivity.at(g_trajectories.at(index_flight).destination_airport).map_waypoint_node;

		for (ite_map_waypoint_node = map_waypoint_node.begin(); ite_map_waypoint_node != map_waypoint_node.end(); ite_map_waypoint_node++) {
			if ((!ite_map_waypoint_node->second.refName1.empty()) && (h_landing_taxi_plan.runway_name[index_flight] != NULL) && (strlen(h_landing_taxi_plan.runway_name[index_flight]) > 0) && (ite_map_waypoint_node->second.refName1.find(h_landing_taxi_plan.runway_name[index_flight]) != string::npos)
					&& (!ite_map_waypoint_node->second.type1.empty()) && (ite_map_waypoint_node->second.type1.find("Entry") != string::npos)) {
				startPoint_lat = ite_map_waypoint_node->second.latitude;
				startPoint_lon = ite_map_waypoint_node->second.longitude;
			} else if ((!ite_map_waypoint_node->second.refName2.empty()) && (h_landing_taxi_plan.runway_name[index_flight] != NULL) && (strlen(h_landing_taxi_plan.runway_name[index_flight]) > 0) && (ite_map_waypoint_node->second.refName2.find(h_landing_taxi_plan.runway_name[index_flight]) != string::npos)
					&& (!ite_map_waypoint_node->second.type2.empty()) && (ite_map_waypoint_node->second.type2.find("End") != string::npos)) {
				endPoint_lat = ite_map_waypoint_node->second.latitude;
				endPoint_lon = ite_map_waypoint_node->second.longitude;
			}

			if ((startPoint_lat >= 0) && (startPoint_lon)
					&& (endPoint_lat >= 0) && (endPoint_lon))
				break;
		}

		update_states->V_ground = update_states->V_horizontal;
	} else if ((h_aircraft_soa.flag_geoStyle[index_flight])
				&& (h_landing_taxi_plan.runway_entry_latitude_geoStyle[index_flight] != DBL_MAX)
				&& (h_landing_taxi_plan.runway_entry_longitude_geoStyle[index_flight] != DBL_MAX)
				&& (h_landing_taxi_plan.runway_end_latitude_geoStyle[index_flight] != DBL_MAX)
				&& (h_landing_taxi_plan.runway_end_longitude_geoStyle[index_flight] != DBL_MAX)
			) {
		startPoint_lat = h_landing_taxi_plan.runway_entry_latitude_geoStyle[index_flight];
		startPoint_lon = h_landing_taxi_plan.runway_entry_longitude_geoStyle[index_flight];
		endPoint_lat = h_landing_taxi_plan.runway_end_latitude_geoStyle[index_flight];
		endPoint_lon = h_landing_taxi_plan.runway_end_longitude_geoStyle[index_flight];

		update_states->V_ground = update_states->V_horizontal;
	} else {
		update_states->durationSecond_to_be_proc = 0;
	}

	if ((startPoint_lat != 0) && (startPoint_lon != 0)
			&& (endPoint_lat != 0) && (endPoint_lon != 0)) {
		update_states->hdg_rad = compute_heading_rad_gc(startPoint_lat, startPoint_lon,
				endPoint_lat, endPoint_lon);

		update_states->course_rad_runway = update_states->hdg_rad;
		if ((h_landing_taxi_plan.waypoint_node_ptr[index_flight] != NULL) && (h_landing_taxi_plan.waypoint_node_ptr[index_flight]->next_node_ptr != NULL)) {
			update_states->course_rad_taxi = h_landing_taxi_plan.waypoint_node_ptr[index_flight]->next_node_ptr->course_rad_to_next_node;
		}
	}
	// end - Calculate course heading of runway

	update_states->rocd_fps = 0;
	update_states->fpa_rad = 0;

	if (update_states->target_WaypointNode_ptr == h_landing_taxi_plan.waypoint_node_ptr[index_flight]) {
		// Set touchdown point location to the current location
		update_states->lat = c_estimate_touchdown_point_latitude_deg[index_flight];
		update_states->lon = c_estimate_touchdown_point_longitude_deg[index_flight];
	}

	if ((h_landing_taxi_plan.runway_name[index_flight] != NULL) && (!update_states->flag_abnormal_on_runway)) {
		// Check if aircraft experiences abnormal event on the runway
		double hdg_to_runway_end = compute_heading_rad_gc(update_states->lat,
									update_states->lon,
									update_states->landing_runway_end_latitude,
									update_states->landing_runway_end_longitude);

		double distance_to_runway_end = compute_distance_gc(update_states->lat,
					update_states->lon,
					update_states->landing_runway_end_latitude,
					update_states->landing_runway_end_longitude,
					g_trajectories[index_flight].destination_airport_elevation_ft);

		double runway_length_landing = compute_distance_gc(
				update_states->landing_runway_entry_latitude,
				update_states->landing_runway_entry_longitude,
					update_states->landing_runway_end_latitude,
					update_states->landing_runway_end_longitude,
					g_trajectories[index_flight].destination_airport_elevation_ft);

		if ((abs(update_states->course_rad_runway - hdg_to_runway_end) * 180 / PI < 80)
				&& (runway_length_landing < distance_to_runway_end)) {
			update_states->flag_abnormal_on_runway = true;
			update_states->flight_phase = FLIGHT_PHASE_RUNWAYUNDERSHOOT;
			printf("Aircraft %s undershoot the runway during landing\n", update_states->acid);

			update_states->durationSecond_to_be_proc = 0;
		}
		// end - Check if aircraft experiences abnormal event on the runway
	}

	if (h_landing_taxi_plan.waypoint_node_ptr[index_flight] == NULL) {
		update_states->durationSecond_to_be_proc = 0;
	}
}

void flightPhase_algorithm_LAND(update_states_t* update_states,
		const int index_flight,
		const float t,
		const float t_step_surface,
		const float t_step_terminal,
		const float t_step_airborne) {
	real_t tmpNewV = 0;
	double tmpDuration_to_NewV = 0;
	double tmpDuration_NewV_remaining = 0;

	if (c_t_landing[index_flight] < 0) {
		c_t_landing[index_flight] = t + (t_step_terminal - update_states->durationSecond_to_be_proc);

		update_states->V_downWind = 0;

		update_states->tas_knots = update_states->V_touchdown - update_states->V_downWind;
	}

	if (h_landing_taxi_plan.taxi_tas_knots[index_flight] < update_states->tas_knots) {
		tmpNewV = update_states->tas_knots - update_states->acceleration * update_states->durationSecond_to_be_proc;
	}

	if (tmpNewV < h_landing_taxi_plan.taxi_tas_knots[index_flight]) {
		tmpNewV = h_landing_taxi_plan.taxi_tas_knots[index_flight];
	}

	tmpDuration_to_NewV = (update_states->tas_knots - tmpNewV) / update_states->acceleration;
	if (tmpNewV == h_landing_taxi_plan.taxi_tas_knots[index_flight]) {
		tmpDuration_NewV_remaining = update_states->durationSecond_to_be_proc - tmpDuration_to_NewV;
	}

	// Moving distance during this time step
	update_states->S_within_flightPhase_and_simCycle = (update_states->V_ground_bak * KnotsToFps * tmpDuration_to_NewV - 0.5 * update_states->acceleration * pow(tmpDuration_to_NewV, 2))
			+ tmpNewV * KnotsToFps * tmpDuration_NewV_remaining;

	update_states->rocd_fps = 0;

	// If the moving distance is larger than the distance to the target waypoint, the aircraft will reach the target waypoint and even fly over it.
	// In this case, we calculate the elapsed time from "current location" to "target waypoint" then calculate the "durationSecond_to_be_proc"(remaining time step) we have to finish.
	if (update_states->L_to_go <= update_states->S_within_flightPhase_and_simCycle) {
		update_states->tas_knots = update_states->tas_knots_bak - update_states->acceleration * update_states->durationSecond_to_be_proc;
		if (update_states->tas_knots < h_landing_taxi_plan.taxi_tas_knots[index_flight])
			update_states->tas_knots = h_landing_taxi_plan.taxi_tas_knots[index_flight];

		update_states->V_horizontal = update_states->tas_knots;

		aircraft_compute_heading(update_states, index_flight);

		update_states->V_ground = update_states->V_horizontal;

		real_t tmpV_at_target_waypoint;
		if (h_landing_taxi_plan.taxi_tas_knots[index_flight] < update_states->tas_knots_bak) {
			tmpV_at_target_waypoint = sqrt(pow(update_states->tas_knots_bak, 2) - 2 * update_states->acceleration * update_states->L_to_go);
			if (tmpV_at_target_waypoint < h_landing_taxi_plan.taxi_tas_knots[index_flight])
				tmpV_at_target_waypoint = h_landing_taxi_plan.taxi_tas_knots[index_flight];
		}

		// Aircraft already passed the waypoint of runway exit
		// Check speed
		// Only do the target waypoint forwarding when the speed is under taxi speed
		if (tmpV_at_target_waypoint <= h_landing_taxi_plan.taxi_tas_knots[index_flight]) {
			if (update_states->L_to_go < update_states->S_within_flightPhase_and_simCycle) {
				update_states->elapsedSecond = calculate_elapsedSecond(update_states->L_to_go / update_states->V_ground);

				if (detect_userIncident(update_states,
						index_flight,
						t,
						t_step_surface,
						t_step_terminal,
						t_step_airborne))
					return;

				// Calculate remaining duration to process
				update_states->durationSecond_to_be_proc = update_states->durationSecond_to_be_proc - update_states->elapsedSecond;
			} else {
				update_states->durationSecond_to_be_proc = 0;
			}

			// Set current location at the target waypoint
			if (update_states->target_WaypointNode_ptr != NULL) {
				update_states->lat = update_states->target_WaypointNode_ptr->latitude;
				update_states->lon = update_states->target_WaypointNode_ptr->longitude;
			}
			update_states->S_within_flightPhase_and_simCycle = 0; // Reset

			// Target waypoint increment
			update_states->target_waypoint_index++;
			update_states->last_WaypointNode_ptr = update_states->target_WaypointNode_ptr;
			if (update_states->target_WaypointNode_ptr != NULL) {
				update_states->target_WaypointNode_ptr = update_states->target_WaypointNode_ptr->next_node_ptr;
			}
			update_states->flag_target_waypoint_change = true;
		} else {
			update_states->elapsedSecond = update_states->durationSecond_to_be_proc;
			if (detect_userIncident(update_states,
					index_flight,
					t,
					t_step_surface,
					t_step_terminal,
					t_step_airborne))
				return;

			update_states->durationSecond_to_be_proc = 0;
		}
	} else {
		update_states->elapsedSecond = update_states->durationSecond_to_be_proc;
		if (detect_userIncident(update_states,
				index_flight,
				t,
				t_step_surface,
				t_step_terminal,
				t_step_airborne))
			return;

		update_states->tas_knots = tmpNewV;

		update_states->V_horizontal = update_states->tas_knots;

		aircraft_compute_heading(update_states, index_flight);

		update_states->V_ground = update_states->V_horizontal;

		update_states->durationSecond_to_be_proc = 0;
	}
}

void flightPhase_algorithm_EXIT_RUNWAY(update_states_t* update_states,
		const int index_flight,
		const float t,
		const float t_step_surface,
		const float t_step_terminal,
		const float t_step_airborne) {
	HumanErrorEvent_t tmpHumanErrorEvent;

	update_states->tas_knots = h_landing_taxi_plan.taxi_tas_knots[index_flight];
	update_states->V_horizontal = update_states->tas_knots;

	aircraft_compute_heading(update_states, index_flight);

	update_states->V_ground = update_states->V_horizontal;

	// Moving distance during this time step
	update_states->S_within_flightPhase_and_simCycle = update_states->V_ground * KnotsToFps * update_states->durationSecond_to_be_proc;

	// If the moving distance is larger than the distance to the target waypoint, the aircraft will reach the target waypoint and even fly over it.
	// In this case, we calculate the elapsed time from "current location" to "target waypoint" then calculate the "durationSecond_to_be_proc"(remaining time step) we have to finish.
	if (update_states->L_to_go <= update_states->S_within_flightPhase_and_simCycle) {
		tmpHumanErrorEvent.type = PILOT_ERROR_EVENT_TYPE_CLEARANCE;
		tmpHumanErrorEvent.name = stringify(AIRCRAFT_CLEARANCE_TAXI_LANDING);

		// If the moving distance is larger than the distance to the target waypoint, the aircraft will reach the target waypoint and even fly over it.
		// In this case, we calculate the elapsed time from "current location" to "target waypoint" then calculate the "durationSecond_to_be_proc"(remaining time step) we have to finish.
		if ((pilot_skip_request_clearance_aircraft(index_flight, tmpHumanErrorEvent)) || (pilot_check_clearance_aircraft(t, index_flight, tmpHumanErrorEvent) == 1)) {
			if (update_states->L_to_go < update_states->S_within_flightPhase_and_simCycle) {
				update_states->elapsedSecond = calculate_elapsedSecond(update_states->L_to_go / update_states->V_ground);

				if (detect_userIncident(update_states,
						index_flight,
						t,
						t_step_surface,
						t_step_terminal,
						t_step_airborne))
					return;

				// Calculate remaining duration to process
				update_states->durationSecond_to_be_proc = update_states->durationSecond_to_be_proc - update_states->elapsedSecond;

				update_states->S_within_flightPhase_and_simCycle = 0; // Reset
			} else {
				update_states->durationSecond_to_be_proc = 0;
			}

			// Set current location at the target waypoint
			if (update_states->target_WaypointNode_ptr != NULL) {
				update_states->lat = update_states->target_WaypointNode_ptr->latitude;
				update_states->lon = update_states->target_WaypointNode_ptr->longitude;
			}

			// Target waypoint increment
			update_states->target_waypoint_index++;
			update_states->last_WaypointNode_ptr = update_states->target_WaypointNode_ptr;
			if (update_states->target_WaypointNode_ptr != NULL) {
				update_states->target_WaypointNode_ptr = update_states->target_WaypointNode_ptr->next_node_ptr;
			}
			update_states->flag_target_waypoint_change = true;
		} else {
			update_states->elapsedSecond = update_states->durationSecond_to_be_proc;
			if (detect_userIncident(update_states,
					index_flight,
					t,
					t_step_surface,
					t_step_terminal,
					t_step_airborne))
				return;

			c_hold_flight_phase[index_flight] = FLIGHT_PHASE_EXIT_RUNWAY;

			update_states->durationSecond_to_be_proc = 0;
		}
	} else {
		update_states->elapsedSecond = update_states->durationSecond_to_be_proc;
		if (detect_userIncident(update_states,
				index_flight,
				t,
				t_step_surface,
				t_step_terminal,
				t_step_airborne))
			return;

		update_states->durationSecond_to_be_proc = 0;
	}

	update_states->altitude_ft = c_destination_airport_elevation_ft[index_flight];
}

void flightPhase_algorithm_TAXI_ARRIVING(update_states_t* update_states,
		const int index_flight,
		const float t,
		const float t_step_surface,
		const float t_step_terminal,
		const float t_step_airborne) {
	update_states->tas_knots = h_landing_taxi_plan.taxi_tas_knots[index_flight];
	update_states->V_horizontal = update_states->tas_knots;
	update_states->rocd_fps = 0;

	aircraft_compute_heading(update_states, index_flight);

	update_states->V_ground = update_states->V_horizontal;

	update_states->altitude_ft = c_destination_airport_elevation_ft[index_flight];

	// Moving distance during this time step
	update_states->S_within_flightPhase_and_simCycle = update_states->V_ground * KnotsToFps * update_states->durationSecond_to_be_proc;

	// If the moving distance is larger than the distance to the target waypoint, the aircraft will reach the target waypoint and even fly over it.
	// In this case, we calculate the elapsed time from "current location" to "target waypoint" then calculate the "durationSecond_to_be_proc"(remaining time step) we have to finish.
	if (update_states->L_to_go <= update_states->S_within_flightPhase_and_simCycle) {
		if (update_states->L_to_go < update_states->S_within_flightPhase_and_simCycle) {
			update_states->elapsedSecond = calculate_elapsedSecond(update_states->L_to_go / update_states->V_ground);

			if (detect_userIncident(update_states,
					index_flight,
					t,
					t_step_surface,
					t_step_terminal,
					t_step_airborne))
				return;

			// Calculate remaining duration to process
			update_states->durationSecond_to_be_proc = update_states->durationSecond_to_be_proc - update_states->elapsedSecond;

			update_states->S_within_flightPhase_and_simCycle = 0; // Reset
		} else {
			update_states->durationSecond_to_be_proc = 0;
		}

		// Set current location at the target waypoint
		if (update_states->target_WaypointNode_ptr != NULL) {
			update_states->lat = update_states->target_WaypointNode_ptr->latitude;
			update_states->lon = update_states->target_WaypointNode_ptr->longitude;
		}

		// Target waypoint increment
		update_states->target_waypoint_index++;
		update_states->last_WaypointNode_ptr = update_states->target_WaypointNode_ptr;
		if (update_states->target_WaypointNode_ptr != NULL) {
			update_states->target_WaypointNode_ptr = update_states->target_WaypointNode_ptr->next_node_ptr;
		}
		update_states->flag_target_waypoint_change = true;
	} else {
		update_states->elapsedSecond = update_states->durationSecond_to_be_proc;
		if (detect_userIncident(update_states,
				index_flight,
				t,
				t_step_surface,
				t_step_terminal,
				t_step_airborne))
			return;

		update_states->durationSecond_to_be_proc = 0;
	}
}

void flightPhase_algorithm_RUNWAY_CROSSING(update_states_t* update_states,
		const int index_flight,
		const float t,
		const float t_step_surface,
		const float t_step_terminal,
		const float t_step_airborne) {
}

void flightPhase_algorithm_RAMP_ARRIVING(update_states_t* update_states,
		const int index_flight,
		const float t,
		const float t_step_surface,
		const float t_step_terminal,
		const float t_step_airborne) {
	update_states->tas_knots = h_landing_taxi_plan.ramp_tas_knots[index_flight];
	update_states->V_horizontal = update_states->tas_knots;
	update_states->rocd_fps = 0;

	aircraft_compute_heading(update_states, index_flight);

	update_states->V_ground = update_states->V_horizontal;

	update_states->altitude_ft = c_destination_airport_elevation_ft[index_flight];

	// Moving distance during this time step
	update_states->S_within_flightPhase_and_simCycle = update_states->V_ground * KnotsToFps * update_states->durationSecond_to_be_proc;

	// If the moving distance is larger than the distance to the target waypoint, the aircraft will reach the target waypoint and even fly over it.
	// In this case, we calculate the elapsed time from "current location" to "target waypoint" then calculate the "durationSecond_to_be_proc"(remaining time step) we have to finish.
	if (update_states->L_to_go <= update_states->S_within_flightPhase_and_simCycle) {
		if (update_states->L_to_go < update_states->S_within_flightPhase_and_simCycle) {
			update_states->elapsedSecond = calculate_elapsedSecond(update_states->L_to_go / update_states->V_ground);

			if (detect_userIncident(update_states,
					index_flight,
					t,
					t_step_surface,
					t_step_terminal,
					t_step_airborne))
				return;

			// Calculate remaining duration to process
			update_states->durationSecond_to_be_proc = update_states->durationSecond_to_be_proc - update_states->elapsedSecond;

			update_states->S_within_flightPhase_and_simCycle = 0; // Reset
		} else {
			update_states->durationSecond_to_be_proc = 0;
		}

		// Set current location at the target waypoint
		if (update_states->target_WaypointNode_ptr != NULL) {
			update_states->lat = update_states->target_WaypointNode_ptr->latitude;
			update_states->lon = update_states->target_WaypointNode_ptr->longitude;
		}

		// Target waypoint increment
		update_states->target_waypoint_index++;
		update_states->last_WaypointNode_ptr = update_states->target_WaypointNode_ptr;
		if (update_states->target_WaypointNode_ptr != NULL) {
			update_states->target_WaypointNode_ptr = update_states->target_WaypointNode_ptr->next_node_ptr;
		}
		update_states->flag_target_waypoint_change = true;
	} else {
		update_states->elapsedSecond = update_states->durationSecond_to_be_proc;
		if (detect_userIncident(update_states,
				index_flight,
				t,
				t_step_surface,
				t_step_terminal,
				t_step_airborne))
			return;

		update_states->durationSecond_to_be_proc = 0;
	}
}

void flightPhase_algorithm_DESTINATION_GATE(update_states_t* update_states,
		const int index_flight,
		const float t,
		const float t_step_surface,
		const float t_step_terminal,
		const float t_step_airborne) {
	update_states->tas_knots = 0;
	update_states->V_ground = 0;

	update_states->durationSecond_to_be_proc = 0;
}

void flightPhase_algorithm_USER_INCIDENT(update_states_t* update_states,
		const int index_flight,
		const float t,
		const float t_step_surface,
		const float t_step_terminal,
		const float t_step_airborne) {
	update_states->elapsedSecond = update_states->durationSecond_to_be_proc;

	update_states->V_horizontal = update_states->tas_knots * cos(update_states->fpa_rad);

	update_states->V_ground = aircraft_compute_ground_speed(t,
				update_states->lat,
				update_states->lon,
				update_states->altitude_ft,
				update_states->V_horizontal,
				update_states->hdg_rad);

	if (detect_userIncident(update_states,
			index_flight,
			t,
			t_step_surface,
			t_step_terminal,
			t_step_airborne))
		return;

	if (0 < update_states->durationSecond_to_be_proc) {
		update_states->tas_knots = ((IncidentFlightPhase)incidentFlightPhaseMap.at(string(update_states->acid)).at(update_states->simulation_user_incident_index)).speed;
		update_states->rocd_fps = ((IncidentFlightPhase)incidentFlightPhaseMap.at(string(update_states->acid)).at(update_states->simulation_user_incident_index)).rocd;
		update_states->hdg_rad = ((IncidentFlightPhase)incidentFlightPhaseMap.at(string(update_states->acid)).at(update_states->simulation_user_incident_index)).course * M_PI / 180.;

		// Calculate new distance
		update_states->S_within_flightPhase_and_simCycle = update_states->tas_knots * KnotsToFps * update_states->elapsedSecond;

		double delta_h = update_states->elapsedSecond * update_states->rocd_fps;

		if (0 < update_states->S_within_flightPhase_and_simCycle) {
			update_states->fpa_rad = atan2(delta_h, update_states->S_within_flightPhase_and_simCycle);
		} else {
			if (0 < delta_h) {
				update_states->fpa_rad = M_PI * 90 / 180;
			} else {
				update_states->fpa_rad = (-1) * M_PI * 90 / 180;
			}
		}

		// Calculate new location
		pilot_compute_Lat_Lon_default_logic(update_states, index_flight);

		// Calculate new altitude
		update_states->altitude_ft = update_states->altitude_ft + delta_h;

		if ((g_trajectories.at(index_flight).origin_airport_elevation_ft == update_states->altitude_ft_bak)
				&& (update_states->altitude_ft < 0)) {
			update_states->altitude_ft = g_trajectories.at(index_flight).origin_airport_elevation_ft;
		} else if ((g_trajectories.at(index_flight).destination_airport_elevation_ft == update_states->altitude_ft_bak)
				&& (update_states->altitude_ft < 0)) {
			update_states->altitude_ft = g_trajectories.at(index_flight).destination_airport_elevation_ft;
		}

		if (update_states->altitude_ft < 0) {
			update_states->altitude_ft = 0;
		}
		// end - Calculate new altitude
	}

	update_states->durationSecond_to_be_proc = update_states->durationSecond_to_be_proc - update_states->elapsedSecond;
}

/**
 * Function to prepare data of copying c_ variables to [update_states]
 */
__global__ void kernel_update_states_prepare_data(update_states_t* update_states,
		const int index_flight,
		const float t_step_surface,
		const float t_step_terminal,
		const float t_step_airborne) {
	update_states->V_ground_bak = c_V_ground[index_flight];

	// Prepare update_states data based on different flight_phase
	if (isFlightPhase_in_ground_departing(update_states->flight_phase)) {
		update_states->altitude_ft = c_altitude_ft[index_flight];
		update_states->lat = c_latitude_deg[index_flight];
		update_states->lon = c_longitude_deg[index_flight];
		update_states->tas_knots = c_tas_knots[index_flight];

		update_states->toc_index = c_toc_index[index_flight];
		update_states->tod_index = c_tod_index[index_flight];

		update_states->rocd_fps = c_rocd_fps[index_flight];
		update_states->rocd_fps_bak = c_rocd_fps[index_flight];

		update_states->cruise_tas_knots = c_cruise_tas_knots[index_flight];
		update_states->cruise_alt_ft = c_cruise_alt_ft[index_flight];
		update_states->target_altitude_ft = c_target_altitude_ft[index_flight];
		update_states->target_waypoint_index = c_target_waypoint_index[index_flight];
		update_states->target_WaypointNode_ptr = c_target_waypoint_node_ptr[index_flight];

		update_states->dest_airport_elev_ft = c_destination_airport_elevation_ft[index_flight];

		if (update_states->target_WaypointNode_ptr != NULL) {
			update_states->fp_lat = update_states->target_WaypointNode_ptr->latitude;
			update_states->fp_lon = update_states->target_WaypointNode_ptr->longitude;
		}
		update_states->fp_length = h_departing_taxi_plan.waypoint_length[index_flight];

		update_states->tol = .5 * update_states->tas_knots * KNOTS_TO_FPS * t_step_surface * 1.0;

		update_states->durationSecond_to_be_proc = t_step_surface;
	} else if (isFlightPhase_in_ground_landing(update_states->flight_phase)) {
		update_states->altitude_ft = c_altitude_ft[index_flight];
		update_states->lat = c_latitude_deg[index_flight];
		update_states->lon = c_longitude_deg[index_flight];
		update_states->tas_knots = c_tas_knots[index_flight];

		update_states->toc_index = c_toc_index[index_flight];
		update_states->tod_index = c_tod_index[index_flight];

		update_states->rocd_fps = c_rocd_fps[index_flight];
		update_states->rocd_fps_bak = c_rocd_fps[index_flight];

		update_states->cruise_tas_knots = c_cruise_tas_knots[index_flight];
		update_states->cruise_alt_ft = c_cruise_alt_ft[index_flight];
		update_states->target_altitude_ft = c_target_altitude_ft[index_flight];
		update_states->target_waypoint_index = c_target_waypoint_index[index_flight];
		update_states->target_WaypointNode_ptr = c_target_waypoint_node_ptr[index_flight];

		update_states->dest_airport_elev_ft = c_destination_airport_elevation_ft[index_flight];

		if (update_states->target_WaypointNode_ptr != NULL) {
			update_states->fp_lat = update_states->target_WaypointNode_ptr->latitude;
			update_states->fp_lon = update_states->target_WaypointNode_ptr->longitude;
		}
		update_states->fp_length = h_landing_taxi_plan.waypoint_length[index_flight];

		update_states->tol = .5 * update_states->tas_knots * KNOTS_TO_FPS * t_step_surface * 1.0;

		update_states->durationSecond_to_be_proc = t_step_surface;
	} else {
		update_states->altitude_ft = c_altitude_ft[index_flight];
		update_states->lat = c_latitude_deg[index_flight];
		update_states->lon = c_longitude_deg[index_flight];
		update_states->tas_knots = c_tas_knots[index_flight];

		update_states->toc_index = c_toc_index[index_flight];
		update_states->tod_index = c_tod_index[index_flight];

		update_states->rocd_fps = c_rocd_fps[index_flight];
		update_states->rocd_fps_bak = c_rocd_fps[index_flight];

		update_states->cruise_tas_knots = c_cruise_tas_knots[index_flight];
		update_states->cruise_alt_ft = c_cruise_alt_ft[index_flight];
		update_states->target_altitude_ft = c_target_altitude_ft[index_flight];
		update_states->target_waypoint_index = c_target_waypoint_index[index_flight];
		update_states->target_WaypointNode_ptr = c_target_waypoint_node_ptr[index_flight];

		update_states->dest_airport_elev_ft = c_destination_airport_elevation_ft[index_flight];

		if (update_states->target_WaypointNode_ptr != NULL) {
			update_states->fp_lat = update_states->target_WaypointNode_ptr->latitude;
			update_states->fp_lon = update_states->target_WaypointNode_ptr->longitude;
		}

		update_states->fp_length = array_Airborne_Flight_Plan_Waypoint_length[index_flight];

		if (update_states->target_WaypointNode_ptr == array_Airborne_Flight_Plan_Final_Node_ptr[index_flight]) {
			if (update_states->altitude_ft < TRACON_ALT_FT) {
				update_states->tol = update_states->tas_knots * KNOTS_TO_FPS * t_step_terminal * 1.0;
			} else {
				update_states->tol = update_states->tas_knots * KNOTS_TO_FPS * t_step_airborne * 1.0;
			}
		} else {
			if (update_states->altitude_ft < TRACON_ALT_FT) {
				update_states->tol = .5 * update_states->tas_knots * KNOTS_TO_FPS * t_step_terminal * 1.0;
			} else {
				update_states->tol = .5 * update_states->tas_knots * KNOTS_TO_FPS * t_step_airborne * 1.0;
			}
		}

		if (update_states->altitude_ft < TRACON_ALT_FT) {
			update_states->durationSecond_to_be_proc = t_step_terminal;
		} else {
			update_states->durationSecond_to_be_proc = t_step_airborne;
		}
	}

	update_states->tas_knots_bak = c_tas_knots[index_flight];
	update_states->altitude_ft_bak = c_altitude_ft[index_flight];

	update_states->hdg_rad = c_course_rad[index_flight];

	update_states->acceleration = c_acceleration[index_flight];
	update_states->flag_target_waypoint_change = c_flag_target_waypoint_change[index_flight];
	update_states->last_WaypointNode_ptr = c_last_WaypointNode_ptr[index_flight];

	update_states->course_rad_runway = c_course_rad_runway[index_flight];
	update_states->course_rad_taxi = c_course_rad_taxi[index_flight];
}

__global__ void kernel_update_states_proc_flightPhase_algorithm(update_states_t* update_states,
		const int index_flight,
		const float t,
		const float t_step_surface,
		const float t_step_terminal,
		const float t_step_airborne) {
	update_states->flag_target_waypoint_change = false;

	switch (update_states->flight_phase) {
		case FLIGHT_PHASE_ORIGIN_GATE:
			flightPhase_algorithm_ORIGIN_GATE(update_states,
					index_flight,
					t,
					t_step_surface,
					t_step_terminal,
					t_step_airborne);

			break;

		case FLIGHT_PHASE_PUSHBACK:
			flightPhase_algorithm_PUSHBACK(update_states,
					index_flight,
					t,
					t_step_surface,
					t_step_terminal,
					t_step_airborne);

			break;

		case FLIGHT_PHASE_RAMP_DEPARTING:
			if (c_hold_flight_phase[index_flight] != FLIGHT_PHASE_RAMP_DEPARTING) {
				flightPhase_algorithm_RAMP_DEPARTING(update_states,
					index_flight,
					t,
					t_step_surface,
					t_step_terminal,
					t_step_airborne);
			} else {
				update_states->durationSecond_to_be_proc = 0;
			}

			break;

		case FLIGHT_PHASE_TAXI_DEPARTING:
			flightPhase_algorithm_TAXI_DEPARTING(update_states,
					index_flight,
					t,
					t_step_surface,
					t_step_terminal,
					t_step_airborne);

			break;

		case FLIGHT_PHASE_RUNWAY_THRESHOLD_DEPARTING:
			flightPhase_algorithm_RUNWAY_THRESHOLD_DEPARTING(update_states,
					index_flight,
					t,
					t_step_surface,
					t_step_terminal,
					t_step_airborne);

			break;

		case FLIGHT_PHASE_TAKEOFF:
			if (h_departing_taxi_plan.waypoint_length[index_flight] > 2) {
				flightPhase_algorithm_TAKEOFF(update_states,
					index_flight,
					t,
					t_step_surface,
					t_step_terminal,
					t_step_airborne);
			} else {
				flightPhase_algorithm_TAKEOFF_no_taxi_plan(update_states,
					index_flight,
					t,
					t_step_surface,
					t_step_terminal,
					t_step_airborne);
			}

			break;

		case FLIGHT_PHASE_CLIMBOUT:
			flightPhase_algorithm_CLIMBOUT(update_states,
					index_flight,
					t,
					t_step_surface,
					t_step_terminal,
					t_step_airborne);

			break;

		case FLIGHT_PHASE_HOLD_IN_DEPARTURE_PATTERN:
			flightPhase_algorithm_HOLD_IN_DEPARTURE_PATTERN(update_states,
					index_flight,
					t,
					t_step_surface,
					t_step_terminal,
					t_step_airborne);

			break;

		case FLIGHT_PHASE_CLIMB_TO_CRUISE_ALTITUDE:
			flightPhase_algorithm_CLIMB_TO_CRUISE_ALTITUDE(update_states,
					index_flight,
					t,
					t_step_surface,
					t_step_terminal,
					t_step_airborne);

			break;

		case FLIGHT_PHASE_TOP_OF_CLIMB:
			flightPhase_algorithm_TOP_OF_CLIMB(update_states,
					index_flight,
					t,
					t_step_surface,
					t_step_terminal,
					t_step_airborne);

			break;

		case FLIGHT_PHASE_CRUISE:
			flightPhase_algorithm_CRUISE(update_states,
					index_flight,
					t,
					t_step_surface,
					t_step_terminal,
					t_step_airborne);

			break;

		case FLIGHT_PHASE_HOLD_IN_ENROUTE_PATTERN:
			flightPhase_algorithm_HOLD_IN_ENROUTE_PATTERN(update_states,
					index_flight,
					t,
					t_step_surface,
					t_step_terminal,
					t_step_airborne);

			break;

		case FLIGHT_PHASE_TOP_OF_DESCENT:
			flightPhase_algorithm_TOP_OF_DESCENT(update_states,
					index_flight,
					t,
					t_step_surface,
					t_step_terminal,
					t_step_airborne);

			break;

		case FLIGHT_PHASE_INITIAL_DESCENT:
			if (TRACON_ALT_FT < update_states->altitude_ft) {
				flightPhase_algorithm_INITIAL_DESCENT(update_states,
						index_flight,
						t,
						t_step_surface,
						t_step_terminal,
						t_step_airborne);
			} else {
				flightPhase_algorithm_INITIAL_DESCENT_final(update_states,
						index_flight,
						t,
						t_step_surface,
						t_step_terminal,
						t_step_airborne);
			}

			break;

		case FLIGHT_PHASE_HOLD_IN_ARRIVAL_PATTERN:
			flightPhase_algorithm_HOLD_IN_ARRIVAL_PATTERN(update_states,
					index_flight,
					t,
					t_step_surface,
					t_step_terminal,
					t_step_airborne);

			break;

		case FLIGHT_PHASE_APPROACH:
			flightPhase_algorithm_APPROACH(update_states,
					index_flight,
					t,
					t_step_surface,
					t_step_terminal,
					t_step_airborne);

			break;

		case FLIGHT_PHASE_FINAL_APPROACH:
			flightPhase_algorithm_FINAL_APPROACH(update_states,
					index_flight,
					t,
					t_step_surface,
					t_step_terminal,
					t_step_airborne);

			break;

		case FLIGHT_PHASE_GO_AROUND:
			flightPhase_algorithm_GO_AROUND(update_states,
					index_flight,
					t,
					t_step_surface,
					t_step_terminal,
					t_step_airborne);

			break;

		case FLIGHT_PHASE_TOUCHDOWN:
			flightPhase_algorithm_TOUCHDOWN(update_states,
					index_flight,
					t,
					t_step_surface,
					t_step_terminal,
					t_step_airborne);

			break;

		case FLIGHT_PHASE_LAND:
			flightPhase_algorithm_LAND(update_states,
					index_flight,
					t,
					t_step_surface,
					t_step_terminal,
					t_step_airborne);

			break;

		case FLIGHT_PHASE_EXIT_RUNWAY:
			if (c_hold_flight_phase[index_flight] != FLIGHT_PHASE_EXIT_RUNWAY) {
				flightPhase_algorithm_EXIT_RUNWAY(update_states,
					index_flight,
					t,
					t_step_surface,
					t_step_terminal,
					t_step_airborne);
			} else {
				update_states->durationSecond_to_be_proc = 0;
			}

			break;

		case FLIGHT_PHASE_TAXI_ARRIVING:
			if (c_hold_flight_phase[index_flight] != FLIGHT_PHASE_TAXI_ARRIVING) {
				flightPhase_algorithm_TAXI_ARRIVING(update_states,
					index_flight,
					t,
					t_step_surface,
					t_step_terminal,
					t_step_airborne);
			} else {
				update_states->durationSecond_to_be_proc = 0;
			}

			break;

		case FLIGHT_PHASE_RUNWAY_CROSSING:
			flightPhase_algorithm_RUNWAY_CROSSING(update_states,
					index_flight,
					t,
					t_step_surface,
					t_step_terminal,
					t_step_airborne);

			break;

		case FLIGHT_PHASE_RAMP_ARRIVING:
			flightPhase_algorithm_RAMP_ARRIVING(update_states,
					index_flight,
					t,
					t_step_surface,
					t_step_terminal,
					t_step_airborne);

			break;

		case FLIGHT_PHASE_DESTINATION_GATE:
			flightPhase_algorithm_DESTINATION_GATE(update_states,
					index_flight,
					t,
					t_step_surface,
					t_step_terminal,
					t_step_airborne);

			break;

		case FLIGHT_PHASE_USER_INCIDENT:
			flightPhase_algorithm_USER_INCIDENT(update_states,
					index_flight,
					t,
					t_step_surface,
					t_step_terminal,
					t_step_airborne);
			break;

		default:

			break;

	} // end - switch

	if ((update_states->flag_target_waypoint_change) && (update_states->L_to_go == update_states->S_within_flightPhase_and_simCycle)) {
		update_states->S_within_flightPhase_and_simCycle = 0;
	}
}

/**
 * Write [update_states] data to c_ variables
 */
__global__ void kernel_update_states_write_data_from_updateStates_to_c(update_states_t* update_states,
		const int index_flight) {
	// write updated states back to global memory
	c_altitude_ft[index_flight] = update_states->altitude_ft;
	c_latitude_deg[index_flight] = update_states->lat;
	c_longitude_deg[index_flight] = update_states->lon;
	c_rocd_fps[index_flight] = update_states->rocd_fps;

	// Change speed
	c_tas_knots[index_flight] = update_states->tas_knots;


	c_V_horizontal[index_flight] = update_states->V_horizontal;
	c_V_ground[index_flight] = update_states->V_ground;

	c_course_rad[index_flight] = update_states->hdg_rad;
	c_fpa_rad[index_flight] = update_states->fpa_rad;
	c_landed_flag[index_flight] = update_states->landed_flag;
	c_target_waypoint_index[index_flight] = update_states->target_waypoint_index;
	c_target_altitude_ft[index_flight] = update_states->target_altitude_ft;
	c_flight_phase[index_flight] = update_states->flight_phase;

	c_target_waypoint_node_ptr[index_flight] = update_states->target_WaypointNode_ptr;
	c_flag_target_waypoint_change[index_flight] = update_states->flag_target_waypoint_change;
	c_last_WaypointNode_ptr[index_flight] = update_states->last_WaypointNode_ptr;
	c_acceleration[index_flight] = update_states->acceleration;

	c_course_rad_runway[index_flight] = update_states->course_rad_runway;
	c_course_rad_taxi[index_flight]	= update_states->course_rad_taxi;
}

__global__ void kernel_update_states_write_data_from_c_to_updateStates(update_states_t* update_states,
		const int index_flight) {
	update_states->altitude_ft = c_altitude_ft[index_flight];
	update_states->lat = c_latitude_deg[index_flight];
	update_states->lon = c_longitude_deg[index_flight];
	update_states->rocd_fps = c_rocd_fps[index_flight];

	update_states->tas_knots = c_tas_knots[index_flight];

	update_states->V_horizontal = c_V_horizontal[index_flight];
	update_states->V_ground = c_V_ground[index_flight];

	update_states->hdg_rad = c_course_rad[index_flight];
	update_states->fpa_rad = c_fpa_rad[index_flight];
	update_states->landed_flag = c_landed_flag[index_flight];
	update_states->target_waypoint_index = c_target_waypoint_index[index_flight];

	update_states->target_altitude_ft = c_target_altitude_ft[index_flight];

	update_states->flight_phase = c_flight_phase[index_flight];

	update_states->target_WaypointNode_ptr = c_target_waypoint_node_ptr[index_flight];
	update_states->flag_target_waypoint_change = c_flag_target_waypoint_change[index_flight];
	update_states->last_WaypointNode_ptr = c_last_WaypointNode_ptr[index_flight];
	update_states->acceleration = c_acceleration[index_flight];

	update_states->course_rad_runway = c_course_rad_runway[index_flight];
	update_states->course_rad_taxi = c_course_rad_taxi[index_flight];
}

__global__ void kernel_update_states_stage1(const int num_flights,
		const float t,
		const float t_step_surface,
		const float t_step_terminal,
		const float t_step_airborne,
		const int stream_id,
		const int thread_id,
		const bool flag_proc_airborne_trajectory) {
	// num flights in a stream array
	int stream_len = ceil((real_t)num_flights/(real_t)NUM_STREAMS);

	// beginning of the stream array in the global flights array
	int stream_start = stream_id * stream_len;

	// end of the stream array in the global flights array
	int stream_end = stream_start + stream_len;

	// offset into the stream arrays
#if USE_GPU
	int stream_offset = blockDim.x*blockIdx.x + threadIdx.x;
#else
	int stream_offset = thread_id;
#endif

	// index into the global arrays
	int index = stream_start + stream_offset;

	// ensure that the global arrays offset is within bounds for
	// the current stream.
	if (index < stream_start) {
		return;
	}
	if ((index >= stream_end) || (index >= num_flights)) {
		return;
	}

	update_states_t* update_states = array_update_states_ptr[index];

	if (index < num_flights) {
		if ((update_states == NULL) || (!update_states->flag_data_initialized))
			return;

		update_states->flight_phase = c_flight_phase[index];

		// If the aircraft is flying in the air AND flag_proc_airborne_trajectory is false, leave this function.
		if ((isFlightPhase_in_airborne(update_states->flight_phase))
				&&
			((!flag_proc_airborne_trajectory)
				&&
			 !((c_altitude_ft[index] < TRACON_ALT_FT) && (fmod(trunc_double(t, 1), t_step_terminal) == 0))
			)
		   ) {

			return;
		}

		update_states->landed_flag = c_landed_flag[index];

		// If the flight was marked landed in the previous iteration
		// then set the mode landed and return
		if (update_states->landed_flag == 1) {
			c_flight_phase[index] = FLIGHT_PHASE_LANDED;

			return;
		}

		// If the aircraft landed already then return here.
		if (update_states->flight_phase == FLIGHT_PHASE_LANDED) {
			return;
		}

		// If this flight is predeparture and its departure time is
		// bigger than current time then don't start processing it yet.
		if (update_states->flight_phase == FLIGHT_PHASE_PREDEPARTURE) {
			if (c_departure_time_sec[index] > t) {
				return;
			}
		}

		kernel_update_states_prepare_data(update_states,
				index,
				t_step_surface,
				t_step_terminal,
				t_step_airborne);

		if ((array_update_states_ptr[index]->flag_aircraft_held_strategic) || (array_update_states_ptr[index]->flag_aircraft_held_tactical)
				//|| (array_update_states_ptr[index]->flag_aircraft_held_cdnr)) {
				|| (0 < array_update_states_ptr[index]->duration_held_cdnr)) {
			return;
		}

		// Handle ground departing mode
		if (isFlightPhase_in_ground_departing(update_states->flight_phase)) {
			// Departure time is later than current time.  Quit this function.
			if (c_departure_time_sec[index] > t) {
				return;
			}

			// Load initial state of ground departing for this aircraft flight
			load_init_states_ground_departing(index, update_states);
		}

		update_states->elapsedSecond = 0; // Reset

		// Set durationSecond_to_be_proc
		if ((isFlightPhase_in_ground_departing(update_states->flight_phase))
				|| (isFlightPhase_in_ground_landing(update_states->flight_phase))) {
			update_states->durationSecond_to_be_proc = t_step_surface;
		} else {
			if (update_states->altitude_ft < TRACON_ALT_FT) {
				update_states->durationSecond_to_be_proc = t_step_terminal;
			} else {
				update_states->durationSecond_to_be_proc = t_step_airborne;
			}
		}

		update_states->durationSecond_altitude_proc = update_states->durationSecond_to_be_proc; // Reset

		while (update_states->durationSecond_to_be_proc > 0) {
			// Process the change of flight phase
			pilot_proc_flightPhase(update_states,
					index,
					t,
					t_step_surface,
					t_step_terminal,
					t_step_airborne);

			if (update_states->flight_phase == FLIGHT_PHASE_LANDED) {
				update_states->durationSecond_to_be_proc = 0;

				c_flight_phase[index] = update_states->flight_phase;
				c_landed_flag[index] = 1;

				return;
			}

			pilot_algorithm_pre_proc(update_states,
					index,
					t,
					t_step_surface,
					t_step_terminal,
					t_step_airborne);

			if ((isFlightPhase_in_ground_landing(update_states->flight_phase))
					&& (update_states->flight_phase != FLIGHT_PHASE_TOUCHDOWN)
					&& (h_landing_taxi_plan.waypoint_length[index] == 0)) {
				update_states->flight_phase = FLIGHT_PHASE_LANDED;
				update_states->durationSecond_to_be_proc = 0;

				c_flight_phase[index] = update_states->flight_phase;
				c_landed_flag[index] = 1;

				return;
			}

			update_states->flag_target_waypoint_change = false; // Reset

			// Process algorithm by the corresponding flight phase
			kernel_update_states_proc_flightPhase_algorithm(update_states,
								index,
								t,
								t_step_surface,
								t_step_terminal,
								t_step_airborne);
		} // end - while

		kernel_update_states_write_data_from_updateStates_to_c(update_states, index);
	}
}

__global__ void kernel_update_states_stage2(const int num_flights,
		const float t,
		const float t_step_surface,
		const float t_step_terminal,
		const float t_step_airborne,
		const int stream_id,
		const int thread_id,
		const bool flag_proc_airborne_trajectory) {
	// num flights in a stream array
	int stream_len = ceil((real_t)num_flights/(real_t)NUM_STREAMS);

	// beginning of the stream array in the global flights array
	int stream_start = stream_id * stream_len;

	// end of the stream array in the global flights array
	int stream_end = stream_start + stream_len;

	// offset into the stream arrays
#if USE_GPU
	int stream_offset = blockDim.x*blockIdx.x + threadIdx.x;
#else
	int stream_offset = thread_id;
#endif

	// index into the global arrays
	int index = stream_start + stream_offset;

	// ensure that the global arrays offset is within bounds for
	// the current stream.
	if (index < stream_start) {
		return;
	}
	if ((index >= stream_end) || (index >= num_flights)) {
		return;
	}

	if (array_update_states_ptr[index] == NULL) {
		return;
	}

	if (array_update_states_ptr[index]->landed_flag) {
		return;
	}

	if (!array_update_states_ptr[index]->flag_data_initialized)
		return;

	if ((array_update_states_ptr[index]->flag_aircraft_held_strategic) || (array_update_states_ptr[index]->flag_aircraft_held_tactical)
			|| (0 < array_update_states_ptr[index]->duration_held_cdnr)) {
		return;
	}

	// ========================================================================

	// Obtain the update_state pointer of this aircraft
	update_states_t* cur_update_states_ptr = array_update_states_ptr[index];

	kernel_update_states_write_data_from_c_to_updateStates(cur_update_states_ptr, index);

	aircraft_proc_altitude(cur_update_states_ptr,
		index,
		t_step_surface,
		t_step_terminal,
		t_step_airborne);

	pilot_proc_Lat_Lon(t,
		cur_update_states_ptr,
		index,
		t_step_surface,
		t_step_terminal,
		t_step_airborne);

	if ((c_V2_point_latitude_deg[index] < 0) && (cur_update_states_ptr->flight_phase == FLIGHT_PHASE_TAKEOFF) && (cur_update_states_ptr->tas_knots >= cur_update_states_ptr->V2_tas_knots)) {
		c_V2_point_latitude_deg[index] = cur_update_states_ptr->lat;
		c_V2_point_longitude_deg[index] = cur_update_states_ptr->lon;
	}

	if (cur_update_states_ptr->flag_abnormal_on_runway) {
		array_update_states_ptr[index]->durationSecond_to_be_proc = 0;
	}
}

////////////////////////////////////////////////////////////////////////////////
// Private host function impl                                                 //
////////////////////////////////////////////////////////////////////////////////

static void traj_data_callback(const float t,
		const float t_step_terminal,
		const bool flag_proc_airborne_trajectory) {
	int num_flights = get_num_flights();

#if USE_GPU
#pragma omp parallel for schedule(static, 1) num_threads(4)
#endif

	for (int i = 0; i < num_flights; ++i) {
		if (((h_aircraft_soa.flight_phase[i] != FLIGHT_PHASE_LANDED) &&
				(h_aircraft_soa.flight_phase[i] != FLIGHT_PHASE_PREDEPARTURE)
				)
			) {
			if ((t == c_departure_time_sec[i])
					||
				((isFlightPhase_in_ground_departing(h_aircraft_soa.flight_phase[i])) || (isFlightPhase_in_ground_landing(h_aircraft_soa.flight_phase[i])))
					||
				((isFlightPhase_in_airborne(h_aircraft_soa.flight_phase[i])) && (flag_proc_airborne_trajectory))
					||
				((isFlightPhase_in_airborne(h_aircraft_soa.flight_phase[i])) && (h_aircraft_soa.altitude_ft[i] < TRACON_ALT_FT) && (fmod(trunc_double(t, 1), t_step_terminal) == 0))
					||
				(h_aircraft_soa.flight_phase[i] == FLIGHT_PHASE_USER_INCIDENT)
					||
				((array_update_states_ptr[i] != NULL) && (array_update_states_ptr[i]->flag_abnormal_on_runway))
			   ) {
				if (t >= c_departure_time_sec[i]) {
					update_states_t* tmp_update_states = array_update_states_ptr[i];

					// determine if the trajectory arrays need to be resized
					// all arrays should be of same size so we'll test using
					// the latitude vector
					int size = g_trajectories.at(i).latitude_deg.size();
					int capacity = g_trajectories.at(i).latitude_deg.capacity();
					if (size == capacity) {
						int newsize = 2 * size;

						//g_trajectories.at(i).sector_index.reserve(newsize);
						g_trajectories.at(i).latitude_deg.reserve(newsize);
						g_trajectories.at(i).longitude_deg.reserve(newsize);
						g_trajectories.at(i).altitude_ft.reserve(newsize);
						g_trajectories.at(i).rocd_fps.reserve(newsize);
						g_trajectories.at(i).tas_knots.reserve(newsize);
						g_trajectories.at(i).tas_knots_ground.reserve(newsize);
						g_trajectories.at(i).course_deg.reserve(newsize);
						g_trajectories.at(i).fpa_deg.reserve(newsize);
						g_trajectories.at(i).flight_phase.reserve(newsize);
						g_trajectories.at(i).timestamp.reserve(newsize);
					}

					real_t tmpCourse_deg = h_aircraft_soa.course_rad[i] * 180. / M_PI;
					// Reverse the course heading for the phase of "FLIGHT_PHASE_ORIGIN_GATE" and "FLIGHT_PHASE_PUSHBACK"
					if ((h_aircraft_soa.flight_phase[i] == FLIGHT_PHASE_ORIGIN_GATE) || (h_aircraft_soa.flight_phase[i] == FLIGHT_PHASE_PUSHBACK)) {
						tmpCourse_deg = tmpCourse_deg - 180.0;
					}

					g_trajectories.at(i).latitude_deg.push_back(h_aircraft_soa.latitude_deg[i]);
					g_trajectories.at(i).longitude_deg.push_back(h_aircraft_soa.longitude_deg[i]);

					g_trajectories.at(i).altitude_ft.push_back(h_aircraft_soa.altitude_ft[i]);
					g_trajectories.at(i).rocd_fps.push_back(h_aircraft_soa.rocd_fps[i]);

					if (0 < tmp_update_states->duration_held_cdnr) {
						g_trajectories.at(i).tas_knots.push_back(0.0);
						g_trajectories.at(i).tas_knots_ground.push_back(0.0);
					} else {
						g_trajectories.at(i).tas_knots.push_back(h_aircraft_soa.tas_knots[i]);
						g_trajectories.at(i).tas_knots_ground.push_back(h_aircraft_soa.tas_knots_ground[i]);
					}

					g_trajectories.at(i).course_deg.push_back(tmpCourse_deg);
					g_trajectories.at(i).fpa_deg.push_back(h_aircraft_soa.fpa_rad[i]*180./M_PI);
					g_trajectories.at(i).flight_phase.push_back(h_aircraft_soa.flight_phase[i]);
					g_trajectories.at(i).timestamp.push_back(t);

					if (tmp_update_states->flag_ifs_exist) {
						tmp_update_states->t_processed_ifs = t;
					}
				}
			}
		}
	} // end - for loop

	// Handle external aircrafts
	if (t > 0) {
		for (unsigned int flightSeq = 0; flightSeq < newMU_external_aircraft.flag_external_aircraft.size(); flightSeq++) {
			if (newMU_external_aircraft.flag_external_aircraft.at(flightSeq)) {
				g_trajectories.at(flightSeq).latitude_deg.push_back(newMU_external_aircraft.latitude_deg.at(flightSeq));
				g_trajectories.at(flightSeq).longitude_deg.push_back(newMU_external_aircraft.longitude_deg.at(flightSeq));
				g_trajectories.at(flightSeq).altitude_ft.push_back(newMU_external_aircraft.altitude_ft.at(flightSeq));
				g_trajectories.at(flightSeq).rocd_fps.push_back(newMU_external_aircraft.rocd_fps.at(flightSeq));
				g_trajectories.at(flightSeq).tas_knots.push_back(newMU_external_aircraft.tas_knots.at(flightSeq));
				g_trajectories.at(flightSeq).tas_knots_ground.push_back(newMU_external_aircraft.tas_knots_ground.at(flightSeq));
				g_trajectories.at(flightSeq).course_deg.push_back(newMU_external_aircraft.course_rad.at(flightSeq)*180./M_PI);
				g_trajectories.at(flightSeq).fpa_deg.push_back(newMU_external_aircraft.fpa_rad.at(flightSeq)*180./M_PI);
				g_trajectories.at(flightSeq).flight_phase.push_back(newMU_external_aircraft.flight_phase.at(flightSeq));
				g_trajectories.at(flightSeq).timestamp.push_back(t);
			}
		}
	}
}

int set_device_sector_pointers() {
#if USE_GPU
	cudaMemcpyToSymbol(c_sector_grid, &d_sector_grid, sizeof(int*), 0, cuda_memcpy_HtoD);
	cudaMemcpyToSymbol(c_sector_grid_cell_counts, &d_sector_grid_cell_counts, sizeof(int*), 0, cuda_memcpy_HtoD);
	cudaMemcpyToSymbol(c_sector_array, &d_sector_array, sizeof(sector_t*), 0, cuda_memcpy_HtoD);
#else
	c_sector_grid = d_sector_grid;
	c_sector_grid_cell_counts = d_sector_grid_cell_counts;
	c_sector_array = d_sector_array;
#endif
	return 0;
}

static int set_device_ac_pointers() {
#if USE_GPU
	cudaMemcpyToSymbol(c_departure_time_sec, &d_aircraft_soa.departure_time_sec, sizeof(real_t*), 0, cuda_memcpy_HtoD);
	cudaMemcpyToSymbol(c_cruise_alt_ft, &d_aircraft_soa.cruise_alt_ft, sizeof(real_t*), 0, cuda_memcpy_HtoD);
	cudaMemcpyToSymbol(c_cruise_tas_knots, &d_aircraft_soa.cruise_tas_knots, sizeof(real_t*), 0, cuda_memcpy_HtoD);
	cudaMemcpyToSymbol(c_flight_plan_latitude_deg, &d_aircraft_soa.flight_plan_latitude_deg, sizeof(real_t*), 0, cuda_memcpy_HtoD);
	cudaMemcpyToSymbol(c_flight_plan_longitude_deg, &d_aircraft_soa.flight_plan_longitude_deg, sizeof(real_t*), 0, cuda_memcpy_HtoD);
	cudaMemcpyToSymbol(c_flight_plan_length, &d_aircraft_soa.flight_plan_length, sizeof(int*), 0, cuda_memcpy_HtoD);
	cudaMemcpyToSymbol(c_flight_plan_waypoint_name, d_aircraft_soa.flight_plan_waypoint_name.data(), d_aircraft_soa.flight_plan_waypoint_name.size() * sizeof(std::string), 0, cuda_memcpy_HtoD);
	cudaMemcpyToSymbol(c_destination_airport_elevation_ft, &d_aircraft_soa.destination_airport_elevation_ft, sizeof(real_t*), 0, cuda_memcpy_HtoD);
	cudaMemcpyToSymbol(c_sector_index, &d_aircraft_soa.sector_index, sizeof(int*), 0, cuda_memcpy_HtoD);
	cudaMemcpyToSymbol(c_latitude_deg, &d_aircraft_soa.latitude_deg, sizeof(real_t*), 0, cuda_memcpy_HtoD);
	cudaMemcpyToSymbol(c_longitude_deg, &d_aircraft_soa.longitude_deg, sizeof(real_t*), 0, cuda_memcpy_HtoD);
	cudaMemcpyToSymbol(c_altitude_ft, &d_aircraft_soa.altitude_ft, sizeof(real_t*), 0, cuda_memcpy_HtoD);
	cudaMemcpyToSymbol(c_rocd_fps, &d_aircraft_soa.rocd_fps, sizeof(real_t*), 0, cuda_memcpy_HtoD);
	cudaMemcpyToSymbol(c_tas_knots, &d_aircraft_soa.tas_knots, sizeof(real_t*), 0, cuda_memcpy_HtoD);
	cudaMemcpyToSymbol(c_course_rad, &d_aircraft_soa.course_rad, sizeof(real_t*), 0, cuda_memcpy_HtoD);
	cudaMemcpyToSymbol(c_fpa_rad, &d_aircraft_soa.fpa_rad, sizeof(real_t*), 0, cuda_memcpy_HtoD);
	cudaMemcpyToSymbol(c_flight_mode, &d_aircraft_soa.flight_mode, sizeof(flight_mode_e*), 0, cuda_memcpy_HtoD);
	cudaMemcpyToSymbol(c_landed_flag, &d_aircraft_soa.landed_flag, sizeof(int*), 0, cuda_memcpy_HtoD);
	cudaMemcpyToSymbol(c_adb_aircraft_type_index, &d_aircraft_soa.adb_aircraft_type_index, sizeof(int*), 0, cuda_memcpy_HtoD);
	cudaMemcpyToSymbol(c_holding_started, &d_aircraft_soa.holding_started, sizeof(bool*), 0, cuda_memcpy_HtoD);
	cudaMemcpyToSymbol(c_holding_stopped, &d_aircraft_soa.holding_stopped, sizeof(bool*), 0, cuda_memcpy_HtoD);
	cudaMemcpyToSymbol(c_has_holding_pattern, &d_aircraft_soa.has_holding_pattern, sizeof(bool*), 0, cuda_memcpy_HtoD);
	cudaMemcpyToSymbol(c_hold_start_index, &d_aircraft_soa.hold_start_index, sizeof(int*), 0, cuda_memcpy_HtoD);
	cudaMemcpyToSymbol(c_hold_end_index, &d_aircraft_soa.hold_end_index, sizeof(int*), 0, cuda_memcpy_HtoD);
	cudaMemcpyToSymbol(c_flight_mode_backup, &d_aircraft_soa.flight_mode_backup, sizeof(flight_mode_e*), 0, cuda_memcpy_HtoD);
	cudaMemcpyToSymbol(c_holding_tas_knots, &d_aircraft_soa.holding_tas_knots, sizeof(real_t*), 0, cuda_memcpy_HtoD);
	cudaMemcpyToSymbol(c_target_waypoint_index, &d_aircraft_soa.target_waypoint_index, sizeof(int*), 0, cuda_memcpy_HtoD);
	cudaMemcpyToSymbol(c_target_altitude_ft, &d_aircraft_soa.target_altitude_ft, sizeof(real_t*), 0, cuda_memcpy_HtoD);
	cudaMemcpyToSymbol(c_toc_index, &d_aircraft_soa.toc_index, sizeof(int*), 0, cuda_memcpy_HtoD);
	cudaMemcpyToSymbol(c_tod_index, &d_aircraft_soa.tod_index, sizeof(int*), 0, cuda_memcpy_HtoD);
#else
	int num_flights = get_num_flights();

	c_departure_time_sec = d_aircraft_soa.departure_time_sec;
	c_cruise_alt_ft = d_aircraft_soa.cruise_alt_ft;
	c_cruise_tas_knots = d_aircraft_soa.cruise_tas_knots;

	c_origin_airport_elevation_ft = d_aircraft_soa.origin_airport_elevation_ft;
	c_destination_airport_elevation_ft = d_aircraft_soa.destination_airport_elevation_ft;
	c_sector_index = d_aircraft_soa.sector_index;
	c_latitude_deg = d_aircraft_soa.latitude_deg;
	c_longitude_deg = d_aircraft_soa.longitude_deg;
	c_altitude_ft = d_aircraft_soa.altitude_ft;
	c_rocd_fps = d_aircraft_soa.rocd_fps;
	c_tas_knots = d_aircraft_soa.tas_knots;
	c_course_rad = d_aircraft_soa.course_rad;
	c_fpa_rad = d_aircraft_soa.fpa_rad;
	c_flight_phase = d_aircraft_soa.flight_phase;

	c_landed_flag = d_aircraft_soa.landed_flag;
	c_adb_aircraft_type_index = d_aircraft_soa.adb_aircraft_type_index;
	c_target_waypoint_index = d_aircraft_soa.target_waypoint_index;
	c_target_altitude_ft = d_aircraft_soa.target_altitude_ft;
	c_toc_index = d_aircraft_soa.toc_index;
	c_tod_index = d_aircraft_soa.tod_index;
#endif

	c_target_waypoint_node_ptr = h_aircraft_soa.target_waypoint_node_ptr;
	c_flag_target_waypoint_change = h_aircraft_soa.flag_target_waypoint_change;
	c_last_WaypointNode_ptr = h_aircraft_soa.last_WaypointNode_ptr;
	c_flag_reached_meterfix_point = h_aircraft_soa.flag_reached_meterfix_point;

	c_V_horizontal = h_aircraft_soa.V_horizontal;
	c_V_ground = h_aircraft_soa.tas_knots_ground;
	c_acceleration_aiming_waypoint_node_ptr = h_aircraft_soa.acceleration_aiming_waypoint_node_ptr;
	c_acceleration = h_aircraft_soa.acceleration;
	c_V2_point_latitude_deg = h_aircraft_soa.V2_point_latitude_deg;
	c_V2_point_longitude_deg = h_aircraft_soa.V2_point_longitude_deg;
	c_estimate_touchdown_point_latitude_deg = h_aircraft_soa.estimate_touchdown_point_latitude_deg;
	c_estimate_touchdown_point_longitude_deg = h_aircraft_soa.estimate_touchdown_point_longitude_deg;
	c_t_takeoff = h_aircraft_soa.t_takeoff;
	c_t_landing = h_aircraft_soa.t_landing;
	c_hold_flight_phase = h_aircraft_soa.hold_flight_phase;
	c_course_rad_runway = h_aircraft_soa.course_rad_runway;
	c_course_rad_taxi = h_aircraft_soa.course_rad_taxi;

	return 0;
}


static int set_device_adb_pointers() {
#if USE_GPU
	cudaMemcpyToSymbol(c_adb_num_rows, h_adb_num_rows,
			ADB_PTF_NUM_TYPES*sizeof(short), 0, cuda_memcpy_HtoD);
	cudaMemcpyToSymbol(c_adb_table_start_index, h_adb_table_start_index,
			ADB_PTF_NUM_TYPES*sizeof(short), 0, cuda_memcpy_HtoD);
	cudaMemcpyToSymbol(c_adb_lower_bound_row, h_adb_lower_bound_row,
			ADB_PTF_NUM_LOWER_BOUNDS*sizeof(short), 0, cuda_memcpy_HtoD);
	cudaMemcpyToSymbol(c_adb_fl, h_adb_fl,
			ADB_PTF_NUM_ROWS*sizeof(short), 0, cuda_memcpy_HtoD);
	cudaMemcpyToSymbol(c_adb_vtas_climb_knots, h_adb_vtas_climb_knots,
			ADB_PTF_NUM_ROWS*sizeof(short), 0, cuda_memcpy_HtoD);
	cudaMemcpyToSymbol(c_adb_vtas_descent_knots, h_adb_vtas_descent_knots,
			ADB_PTF_NUM_ROWS*sizeof(short), 0, cuda_memcpy_HtoD);
	cudaMemcpyToSymbol(c_adb_roc_fpm, h_adb_roc_fpm,
			ADB_PTF_NUM_ROWS*sizeof(short), 0, cuda_memcpy_HtoD);
	cudaMemcpyToSymbol(c_adb_rod_fpm, h_adb_rod_fpm,
			ADB_PTF_NUM_ROWS*sizeof(short), 0, cuda_memcpy_HtoD);
#else
	c_adb_num_rows = h_adb_num_rows;
	c_adb_table_start_index = h_adb_table_start_index;
	c_adb_lower_bound_row = h_adb_lower_bound_row;
	c_adb_fl = h_adb_fl;
	c_adb_vtas_climb_knots = h_adb_vtas_climb_knots;
	c_adb_vtas_descent_knots = h_adb_vtas_descent_knots;
	c_adb_roc_fpm = h_adb_roc_fpm;
	c_adb_rod_fpm = h_adb_rod_fpm;
#endif
	return 0;
}

int set_device_ruc_pointers() {
	int nlat = 1+ceil((g_lat_max-g_lat_min)/g_lat_step);
	int nlon = 1+ceil((g_lon_max-g_lon_min)/g_lon_step);
	int nalt = 1+ceil((g_alt_max-g_alt_min)/g_alt_step);
#if USE_GPU
	cudaMemcpyToSymbol(c_wind_north, &d_wind_north, sizeof(real_t*), 0, cuda_memcpy_HtoD);
	cudaMemcpyToSymbol(c_wind_east, &d_wind_east, sizeof(real_t*), 0, cuda_memcpy_HtoD);
	cudaMemcpyToSymbol(c_wind_north_unc, &d_wind_north_unc, sizeof(real_t*), 0, cuda_memcpy_HtoD);
	cudaMemcpyToSymbol(c_wind_east_unc, &d_wind_east_unc, sizeof(real_t*), 0, cuda_memcpy_HtoD);
	cudaMemcpyToSymbol(c_lat_min, &g_lat_min, sizeof(real_t), 0, cuda_memcpy_HtoD);
	cudaMemcpyToSymbol(c_lat_max, &g_lat_max, sizeof(real_t), 0, cuda_memcpy_HtoD);
	cudaMemcpyToSymbol(c_lat_step, &g_lat_step, sizeof(real_t), 0, cuda_memcpy_HtoD);
	cudaMemcpyToSymbol(c_lon_min, &g_lon_min, sizeof(real_t), 0, cuda_memcpy_HtoD);
	cudaMemcpyToSymbol(c_lon_max, &g_lon_max, sizeof(real_t), 0, cuda_memcpy_HtoD);
	cudaMemcpyToSymbol(c_lon_step, &g_lon_step, sizeof(real_t), 0, cuda_memcpy_HtoD);
	cudaMemcpyToSymbol(c_alt_min, &g_alt_min, sizeof(real_t), 0, cuda_memcpy_HtoD);
	cudaMemcpyToSymbol(c_alt_max, &g_alt_max, sizeof(real_t), 0, cuda_memcpy_HtoD);
	cudaMemcpyToSymbol(c_alt_step, &g_alt_step, sizeof(real_t), 0, cuda_memcpy_HtoD);
	cudaMemcpyToSymbol(c_num_lat_cells, &nlat, sizeof(int), 0, cuda_memcpy_HtoD);
	cudaMemcpyToSymbol(c_num_lon_cells, &nlon, sizeof(int), 0, cuda_memcpy_HtoD);
	cudaMemcpyToSymbol(c_num_alt_cells, &nalt, sizeof(int), 0, cuda_memcpy_HtoD);
#else
	c_wind_north = d_wind_north;
	c_wind_east = d_wind_east;
	c_wind_north_unc = d_wind_north_unc;
	c_wind_east_unc = d_wind_east_unc;
	c_lat_min = g_lat_min;
	c_lat_max = g_lat_max;
	c_lat_step = g_lat_step;
	c_lon_min = g_lon_min;
	c_lon_max = g_lon_max;
	c_lon_step = g_lon_step;
	c_alt_min = g_alt_min;
	c_alt_max = g_alt_max;
	c_alt_step = g_alt_step;
	c_num_lat_cells = nlat;
	c_num_lon_cells = nlon;
	c_num_alt_cells = nalt;
#endif
	return 0;
}

static int copy_char_arrays(char**& dst, char**& src, int sz) {
	dst = (char **)calloc(sz, sizeof(char*));

	for (int k = 0; k < sz; ++k) {
		dst[k] = NULL; // Reset
	}

	for (int k = 0; k < sz; ++k) {
		if (src[k] != NULL) {
			size_t lgt = strlen(src[k]);
			if (lgt > 0){
				dst[k] = (char*)calloc(lgt+1, sizeof(char));
				strcpy(dst[k], src[k]);
			}
			else{
				dst[k] = (char*)calloc(1, sizeof(char));
				strcpy(dst[k], "");
			}
		}
	}

	return 1;
}

static void launch_kernel_stage1(dim3 grid_size,
						   dim3 block_size,
		                   size_t smem,
		                   cuda_stream_t stream,
		                   int num_flights,
						   float t,
						   float t_step,
						   float t_step_terminal,
						   float t_step_airborne,
		                   int stream_id,
						   bool flag_proc_airborne_trajectory) {
#if USE_GPU
	update_states<<<grid_size, block_size, smem, stream>>>(
			num_flights, t, t_step, t_step_airborne, stream_id, -1, flag_proc_airborne_trajectory);
#else
	(void)grid_size;
	(void)block_size;
	(void)smem;
	(void)stream;

	for (int thread_id = 0; thread_id < num_flights; ++thread_id) {
		kernel_update_states_stage1(num_flights, t, t_step, t_step_terminal, t_step_airborne, stream_id, thread_id, flag_proc_airborne_trajectory);
	}

#endif
}

static void launch_kernel_stage2(dim3 grid_size,
						   dim3 block_size,
		                   size_t smem,
		                   cuda_stream_t stream,
		                   int num_flights,
						   float t,
						   float t_step,
						   float t_step_terminal,
						   float t_step_airborne,
		                   int stream_id,
						   bool flag_proc_airborne_trajectory) {
	(void)grid_size;
	(void)block_size;
	(void)smem;
	(void)stream;

	for (int thread_id = 0; thread_id < num_flights; ++thread_id) {
		kernel_update_states_stage2(num_flights, t, t_step, t_step_terminal, t_step_airborne, stream_id, thread_id, flag_proc_airborne_trajectory);
	}
}

static void launch_kernel_external_aircraft(float t,
		   float t_step,
		   float t_step_terminal,
		   float t_step_airborne) {
	for (unsigned int flightSeq = 0; flightSeq < newMU_external_aircraft.flag_external_aircraft.size(); flightSeq++) {
		if (newMU_external_aircraft.flag_external_aircraft.at(flightSeq)) {
			if (newMU_external_aircraft.flag_data_initialized.at(flightSeq)) {
				real_t tmpLat_rad_start = newMU_external_aircraft.latitude_deg.at(flightSeq) * PI / 180.;
				real_t tmpLon_rad_start = newMU_external_aircraft.longitude_deg.at(flightSeq) * PI / 180.;

				if (newMU_external_aircraft.tas_knots.at(flightSeq) > 0) {
					real_t tmpAngleGamma = abs(asin(newMU_external_aircraft.rocd_fps.at(flightSeq) / newMU_external_aircraft.tas_knots.at(flightSeq)));
					newMU_external_aircraft.tas_knots.at(flightSeq) = newMU_external_aircraft.tas_knots.at(flightSeq) * cos(tmpAngleGamma);
				}

				newMU_external_aircraft.altitude_ft.at(flightSeq) = newMU_external_aircraft.altitude_ft.at(flightSeq) + newMU_external_aircraft.rocd_fps.at(flightSeq) * t_step;

				real_t tmpDelta_S = newMU_external_aircraft.tas_knots.at(flightSeq) * t_step;

				real_t tmpDelta = tmpDelta_S / RADIUS_EARTH_FT;

				real_t tmpLat_rad_end = asin(sin(tmpLat_rad_start) * cos(tmpDelta) +
							cos(tmpLat_rad_start)* sin(tmpDelta) * cos(newMU_external_aircraft.course_rad.at(flightSeq)));

				real_t tmpLon_rad_end = tmpLon_rad_start +
						atan2(sin(newMU_external_aircraft.course_rad.at(flightSeq)) * sin(tmpDelta) * cos(tmpLat_rad_start),
						cos(tmpDelta) - sin(tmpLat_rad_start) * sin(tmpLat_rad_end));

				newMU_external_aircraft.latitude_deg.at(flightSeq) = tmpLat_rad_end * 180. / PI;
				newMU_external_aircraft.longitude_deg.at(flightSeq) = tmpLon_rad_end * 180. / PI;

				newMU_external_aircraft.latitude_deg_pre_pause.at(flightSeq) = newMU_external_aircraft.latitude_deg.at(flightSeq);
				newMU_external_aircraft.longitude_deg_pre_pause.at(flightSeq) = newMU_external_aircraft.longitude_deg.at(flightSeq);

				// Calculate ground speed
				real_t wind_east_fps = 0, wind_north_fps = 0;

				get_wind_field_components(t, newMU_external_aircraft.latitude_deg.at(flightSeq), newMU_external_aircraft.longitude_deg.at(flightSeq), newMU_external_aircraft.altitude_ft.at(flightSeq), &wind_east_fps, &wind_north_fps);

				real_t cos_hdg = cos(newMU_external_aircraft.course_rad.at(flightSeq));

				real_t sin_hdg = sin(newMU_external_aircraft.course_rad.at(flightSeq));

				real_t V_update = wind_north_fps * cos_hdg + wind_east_fps * sin_hdg;

				newMU_external_aircraft.tas_knots_ground.at(flightSeq) = newMU_external_aircraft.tas_knots.at(flightSeq) * cos(newMU_external_aircraft.fpa_rad.at(flightSeq)) + V_update;
				// end - Calculate ground speed
			} else {
				newMU_external_aircraft.flag_data_initialized.at(flightSeq) = true;
			}
		}
	}
}

float nats_simulation_timestamp = 0;
int nats_simulation_status = 0;
int nats_simulation_check_interval = 200 * 1000; // micro seconds
float nats_simulation_duration = -1;

void set_nats_simulation_duration(long value) {
	if (value <= 0) {
		printf("Setting simulation duration: Failed.  Input value is not valid.  Ignored duration setting.\n");
	} else {
		set_nats_simulation_duration((float)value);
	}
}

void set_nats_simulation_duration(float value) {
	if (value <= 0) {
		printf("Setting simulation duration: Failed.  Input value is not valid.  Ignored duration setting.\n");
	} else {
		nats_simulation_duration = value;
	}
}

void set_target_altitude_ft(int index_flight, float target_altitude_ft) {
	if (isFlightPhase_in_airborne(d_aircraft_soa.flight_phase[index_flight])) {
		if ((index_flight > -1) && !(target_altitude_ft < 0)) {
			c_target_altitude_ft[index_flight] = target_altitude_ft;
			if (c_target_waypoint_node_ptr[index_flight] != NULL) {
				c_target_waypoint_node_ptr[index_flight]->altitude_estimate = target_altitude_ft;
			}
		}
	}
}

/**
 * Return runtime simulation status
 *
 * The return value comes from variable nats_simulation_status
 * Its value, though mostly set by the client program and pass into nats_simulation_operator() function.
 * During the propagation process, the program may change its value
 *
 * return: int value of nats_simulation_status
 */
int get_runtime_sim_status() {
	return nats_simulation_status;
}

float get_curr_sim_time() {
	return nats_simulation_timestamp;
}

void nats_simulation_operator(int usr_prop_status) {
	switch (usr_prop_status) {
		case NATS_SIMULATION_STATUS_START:
			if (nats_simulation_status == NATS_SIMULATION_STATUS_PAUSE) {
				printf("    Propagation is on pause.  [Start] action will be ignored.\n");
			} else {
				nats_simulation_status = NATS_SIMULATION_STATUS_START;
			}

			break;

		case NATS_SIMULATION_STATUS_PAUSE:
			if ((nats_simulation_status != NATS_SIMULATION_STATUS_START) && (nats_simulation_status != NATS_SIMULATION_STATUS_RESUME)) {
				printf("    Propagation is not currently running.  [Pause] action will be ignored.\n");
			} else {
				nats_simulation_status = NATS_SIMULATION_STATUS_PAUSE;
			}

			break;

		case NATS_SIMULATION_STATUS_RESUME:
			if (nats_simulation_status != NATS_SIMULATION_STATUS_PAUSE) {
				printf("    Propagation is not on pause.  [Resume] action will be ignored.\n");
			} else {
				nats_simulation_status = NATS_SIMULATION_STATUS_RESUME;
			}

			break;

		case NATS_SIMULATION_STATUS_STOP:
			if ((nats_simulation_status == NATS_SIMULATION_STATUS_READY) || (nats_simulation_status == NATS_SIMULATION_STATUS_STOP)) {
				printf("    Propagation is not currently running.  [Stop] action will be ignored.\n");
			} else {
				nats_simulation_status = NATS_SIMULATION_STATUS_STOP;
			}

			break;
	}
}

void clear_trajectory() {
	g_trajectories.clear();

	reset_num_flights();

	aircraftRunwayData.clear();
}

/**
 * Process routinely tasks
 *
 * !!!! This function MUST be run as an independent thread when NATS run in real-time mode
 */
void scheduler_proc() {

}

void synchronize_data_from_D_to_H() {
	for (int i = 0; i < NUM_STREAMS; ++i) {
		int stream_start = stream_len * i;

		int num = (i < NUM_STREAMS-1 ? stream_len : stream_len - num_overflow);

		cuda_memcpy_async(&h_aircraft_soa.sector_index[stream_start],
				&d_aircraft_soa.sector_index[stream_start],
				num*sizeof(int), cuda_memcpy_DtoH,
				streams[i]);
		cuda_memcpy_async(&h_aircraft_soa.latitude_deg[stream_start],
				&d_aircraft_soa.latitude_deg[stream_start],
				num*sizeof(real_t), cuda_memcpy_DtoH,
				streams[i]);
		cuda_memcpy_async(&h_aircraft_soa.longitude_deg[stream_start],
				&d_aircraft_soa.longitude_deg[stream_start],
				num*sizeof(real_t), cuda_memcpy_DtoH,
				streams[i]);
		cuda_memcpy_async(&h_aircraft_soa.altitude_ft[stream_start],
				&d_aircraft_soa.altitude_ft[stream_start],
				num*sizeof(real_t), cuda_memcpy_DtoH,
				streams[i]);
		cuda_memcpy_async(&h_aircraft_soa.rocd_fps[stream_start],
				&d_aircraft_soa.rocd_fps[stream_start],
				num*sizeof(real_t), cuda_memcpy_DtoH,
				streams[i]);
		cuda_memcpy_async(&h_aircraft_soa.tas_knots[stream_start],
				&d_aircraft_soa.tas_knots[stream_start],
				num*sizeof(real_t), cuda_memcpy_DtoH,
				streams[i]);
		cuda_memcpy_async(&h_aircraft_soa.tas_knots_ground[stream_start],
				&d_aircraft_soa.tas_knots_ground[stream_start],
				num*sizeof(real_t), cuda_memcpy_DtoH,
				streams[i]);
		cuda_memcpy_async(&h_aircraft_soa.course_rad[stream_start],
				&d_aircraft_soa.course_rad[stream_start],
				num*sizeof(real_t), cuda_memcpy_DtoH,
				streams[i]);
		cuda_memcpy_async(&h_aircraft_soa.fpa_rad[stream_start],
				&d_aircraft_soa.fpa_rad[stream_start],
				num*sizeof(real_t), cuda_memcpy_DtoH,
				streams[i]);
		cuda_memcpy_async(&h_aircraft_soa.flight_phase[stream_start],
				&d_aircraft_soa.flight_phase[stream_start],
				num*sizeof(ENUM_Flight_Phase), cuda_memcpy_DtoH,
				streams[i]);

		cuda_memcpy_async(&h_aircraft_soa.latitude_deg_pre_pause[stream_start],
				&d_aircraft_soa.latitude_deg_pre_pause[stream_start],
				num*sizeof(real_t), cuda_memcpy_DtoH,
				streams[i]);
		cuda_memcpy_async(&h_aircraft_soa.longitude_deg_pre_pause[stream_start],
				&d_aircraft_soa.longitude_deg_pre_pause[stream_start],
				num*sizeof(real_t), cuda_memcpy_DtoH,
				streams[i]);
		cuda_memcpy_async(&h_aircraft_soa.altitude_ft_pre_pause[stream_start],
				&d_aircraft_soa.altitude_ft_pre_pause[stream_start],
				num*sizeof(real_t), cuda_memcpy_DtoH,
				streams[i]);
		cuda_memcpy_async(&h_aircraft_soa.rocd_fps_pre_pause[stream_start],
				&d_aircraft_soa.rocd_fps_pre_pause[stream_start],
				num*sizeof(real_t), cuda_memcpy_DtoH,
				streams[i]);
		cuda_memcpy_async(&h_aircraft_soa.tas_knots_pre_pause[stream_start],
				&d_aircraft_soa.tas_knots_pre_pause[stream_start],
				num*sizeof(real_t), cuda_memcpy_DtoH,
				streams[i]);
		cuda_memcpy_async(&h_aircraft_soa.course_rad_pre_pause[stream_start],
				&d_aircraft_soa.course_rad_pre_pause[stream_start],
				num*sizeof(real_t), cuda_memcpy_DtoH,
				streams[i]);
		cuda_memcpy_async(&h_aircraft_soa.fpa_rad_pre_pause[stream_start],
				&d_aircraft_soa.fpa_rad_pre_pause[stream_start],
				num*sizeof(real_t), cuda_memcpy_DtoH,
				streams[i]);
		cuda_memcpy_async(&h_aircraft_soa.cruise_alt_ft_pre_pause[stream_start],
				&d_aircraft_soa.cruise_alt_ft_pre_pause[stream_start],
				num*sizeof(real_t), cuda_memcpy_DtoH,
				streams[i]);
		cuda_memcpy_async(&h_aircraft_soa.cruise_tas_knots_pre_pause[stream_start],
				&d_aircraft_soa.cruise_tas_knots_pre_pause[stream_start],
				num*sizeof(real_t), cuda_memcpy_DtoH,
				streams[i]);

		cuda_stream_synchronize(streams[i]);
	}
}

ECEF GeodeticToECEFConversion(const double latitude_rad,
		const double longitude_rad,
		const double altitude_ft,
		const double tas_knot_ground,
		const double fpa_rad,
		const double course_heading_rad,
		const double radius_earth_ft
		) {
	ECEF retECEF;

	double tas_fps_ground = tas_knot_ground * KNOTS_TO_FPS;

	retECEF.x_dot = tas_fps_ground * sin(latitude_rad) * cos(longitude_rad) * cos(fpa_rad) * cos(course_heading_rad)
			- tas_fps_ground * sin(longitude_rad) * cos(fpa_rad) * sin(course_heading_rad)
			+ tas_fps_ground * cos(latitude_rad) * cos(longitude_rad) * sin(fpa_rad);

	retECEF.y_dot = tas_fps_ground * sin(latitude_rad) * sin(longitude_rad) * cos(fpa_rad) * cos(course_heading_rad)
			+ tas_fps_ground * cos(longitude_rad) * cos(fpa_rad) * sin(course_heading_rad)
			+ tas_fps_ground * cos(latitude_rad) * sin(longitude_rad) * sin(fpa_rad);

	retECEF.z_dot = (-1) * tas_fps_ground * cos(latitude_rad) * cos(fpa_rad) * cos(course_heading_rad)
			+ tas_fps_ground * sin(latitude_rad) * sin(fpa_rad);

	retECEF.x = (radius_earth_ft + altitude_ft) * cos(latitude_rad) * cos(longitude_rad);

	retECEF.y = (radius_earth_ft + altitude_ft) * cos(latitude_rad) * sin(longitude_rad);

	retECEF.z = (radius_earth_ft + altitude_ft) * sin(latitude_rad);

	return retECEF;
}

AandBCoefficient GetAandBCoefficients(const ECEF* ecef
		) {
	AandBCoefficient ab;

	double D = ecef->x * ecef->y_dot
					- ecef->y * ecef->x_dot;
	if (D == 0) {
		ab.A = 0.0;
		ab.B = 0.0;
	} else {
		ab.A = (ecef->y_dot * ecef->z - ecef->y * ecef->z_dot) / D;
		ab.B = (ecef->z_dot * ecef->x - ecef->z * ecef->x_dot) / D;
	}

	return ab;
}

ECEF GetConflictPoint(
		const AandBCoefficient* ab_1,
		const AandBCoefficient* ab_2,
		const double altitude_ft,
		const double radius_earth_ft) {
	ECEF ecef_conflict;

	ecef_conflict.z = (radius_earth_ft + altitude_ft) *
			sqrt(1 / (1 + pow((ab_1->A - ab_2->A)/(ab_1->A*ab_2->B - ab_2->A*ab_1->B) , 2) + pow((ab_2->B-ab_1->B)/(ab_1->A*ab_2->B - ab_2->A*ab_1->B), 2)));

	ecef_conflict.y = ((ab_1->A - ab_2->A) / (ab_1->A*ab_2->B - ab_2->A*ab_1->B))
			* ecef_conflict.z;

	ecef_conflict.x = ((ab_2->B-ab_1->B) / (ab_1->A*ab_2->B - ab_2->A*ab_1->B))
			* ecef_conflict.z;

	ecef_conflict.x_dot = 0.0; // Reset

	ecef_conflict.y_dot = 0.0; // Reset

	ecef_conflict.z_dot = 0.0; // Reset

	return ecef_conflict;
}

double GetTimeToIntersectionPoint(const ECEF* ecef,
		const ECEF* ecef_conflict,
		const double tas_knot_ground,
		const double altitude_ft,
		const double radius_earth_ft) {
	double t_conflict = 0;

	double sigma = acos((ecef->x*ecef_conflict->x + ecef->y*ecef_conflict->y + ecef->z*ecef_conflict->z) / pow(radius_earth_ft + altitude_ft, 2));

	double d_conflict = (radius_earth_ft + altitude_ft) * sigma;

	t_conflict = d_conflict / (tas_knot_ground * KNOTS_TO_FPS);

	return t_conflict;
}

int FindNumberofTimeStepsToHold(const double dl,
		const double dh,
		const double tas_knot,
		const float t_step) {
	double tmpT = (dh - dl) / (tas_knot *KNOTS_TO_FPS);
	double ns = tmpT / t_step;

	return floor(ns) + 1;
}

bool DetectConflictToGo(const double latitude_rad_1,
		const double longitude_rad_1,
		const double altitude_ft_1,
		const double tas_knot_ground_1,
		const double fpa_rad_1,
		const double course_heading_rad_1,
		double& t_conflict_1,
		const double latitude_rad_2,
		const double longitude_rad_2,
		const double altitude_ft_2,
		const double tas_knot_ground_2,
		const double fpa_rad_2,
		const double course_heading_rad_2,
		double& t_conflict_2,
		const double radius_earth_ft,
		const float t_step) {
	bool retValue = false;

	double h = (altitude_ft_1 + altitude_ft_2) / 2;

	ECEF ecef_1 = GeodeticToECEFConversion(latitude_rad_1,
			longitude_rad_1,
			altitude_ft_1,
			tas_knot_ground_1,
			fpa_rad_1,
			course_heading_rad_1,
			radius_earth_ft);

	ECEF ecef_2 = GeodeticToECEFConversion(latitude_rad_2,
				longitude_rad_2,
				altitude_ft_2,
				tas_knot_ground_2,
				fpa_rad_2,
				course_heading_rad_2,
				radius_earth_ft);

	AandBCoefficient AB_1 = GetAandBCoefficients(&ecef_1);
	AandBCoefficient AB_2 = GetAandBCoefficients(&ecef_2);

	ECEF ecef_conflict = GetConflictPoint(&AB_1, &AB_2, h, radius_earth_ft);

	t_conflict_1 = GetTimeToIntersectionPoint(&ecef_1, &ecef_conflict, tas_knot_ground_1, h, radius_earth_ft);
	t_conflict_2 = GetTimeToIntersectionPoint(&ecef_2, &ecef_conflict, tas_knot_ground_2, h, radius_earth_ft);

	return true;
}

bool ConflictDetection(const double latitude_rad_1,
		const double longitude_rad_1,
		const double altitude_ft_1,
		const double tas_knot_ground_1,
		const double fpa_rad_1,
		const double course_heading_rad_1,
		double& t_conflict_1,
		const double latitude_rad_2,
		const double longitude_rad_2,
		const double altitude_ft_2,
		const double tas_knot_ground_2,
		const double fpa_rad_2,
		const double course_heading_rad_2,
		double& t_conflict_2,
		const double radius_earth_ft,
		const float t_step,
		const double d_initiate) {
	bool retValue = false;

	if ((altitude_ft_1 < 29000) && (altitude_ft_2 < 29000)) {
		if (abs(altitude_ft_1 - altitude_ft_2) > 1000) {
			t_conflict_1 = -1;
			t_conflict_2 = -1;

			retValue = false;

			return retValue;
		}
	} else if (((altitude_ft_1 < 29000) && (altitude_ft_2 >= 29000))
				||
				((altitude_ft_1 >= 29000) && (altitude_ft_2 < 29000))) {
		if (abs(altitude_ft_1 - altitude_ft_2) > 1000) {
			t_conflict_1 = -1;
			t_conflict_2 = -1;

			retValue = false;

			return retValue;
		}
	} else {
		if (abs(altitude_ft_1 - altitude_ft_2) > 2000) {
			t_conflict_1 = -1;
			t_conflict_2 = -1;

			retValue = false;

			return retValue;
		}
	}

	double h = (altitude_ft_1 + altitude_ft_2) / 2;
	double d_gc = compute_distance_gc_rad(latitude_rad_1, longitude_rad_1, latitude_rad_2, longitude_rad_2, h, radius_earth_ft);

	if (d_gc > d_initiate) {
		t_conflict_1 = -1;
		t_conflict_2 = -1;

		retValue = false;

		return retValue;
	}

	return DetectConflictToGo(latitude_rad_1,
				longitude_rad_1,
				altitude_ft_1,
				tas_knot_ground_1,
				fpa_rad_1,
				course_heading_rad_1,
				t_conflict_1,
				latitude_rad_2,
				longitude_rad_2,
				altitude_ft_2,
				tas_knot_ground_2,
				fpa_rad_2,
				course_heading_rad_2,
				t_conflict_2,
				radius_earth_ft,
				t_step);
}

void ConflictResolution(const float t,
		update_states_t* update_states_1,
		update_states_t* update_states_2,
		const double tas_knot_ground_1,
		const double tas_knot_ground_2,
		const double radius_earth_ft,
		const double& t_conflict_1,
		const double& t_conflict_2,
		const double d_separation,
		int& delay_ac,
		int& delay_step) {
	if (t_conflict_1 > t_conflict_2) {
		if ((t_conflict_1 - t_conflict_2)*tas_knot_ground_1*KNOTS_TO_FPS < d_separation) {
			// Hold aircraft 1
			delay_ac = 1;
			delay_step = FindNumberofTimeStepsToHold((t_conflict_1 - t_conflict_2)*tas_knot_ground_1*KNOTS_TO_FPS,
					d_separation,
					tas_knot_ground_1,
					t_step);

			update_states_1->tas_knots_before_held_cdnr = update_states_1->tas_knots;

			update_states_1->tas_knots = 0;
			update_states_1->V_ground = 0;

			cdnr_oss_doc << "At timestamp = " << t << ", aircraft " << update_states_1->acid << " conflicting with " << update_states_2->acid << ".  Holding " << update_states_1->acid << ".\n";
			cnt_event_cdnr++;
		}
	} else if (t_conflict_1 < t_conflict_2) {
		if ((t_conflict_2 - t_conflict_1)*tas_knot_ground_2*KNOTS_TO_FPS < d_separation) {
			// Hold aircraft 2
			delay_ac = 2;
			delay_step = FindNumberofTimeStepsToHold((t_conflict_2 - t_conflict_1)*tas_knot_ground_2*KNOTS_TO_FPS,
					d_separation,
					tas_knot_ground_2,
					t_step);

			update_states_2->tas_knots_before_held_cdnr = update_states_2->tas_knots;

			update_states_2->tas_knots = 0;
			update_states_2->V_ground = 0;

			cdnr_oss_doc << "At timestamp = " << t << ", aircraft " << update_states_2->acid << " conflicting with " << update_states_1->acid << ".  Holding " << update_states_2->acid << ".\n";
			cnt_event_cdnr++;
		}
	} else {
		double tmpT1 = (t_conflict_1 * update_states_1->tas_knots * KNOTS_TO_FPS - d_separation) / (update_states_1->tas_knots * KNOTS_TO_FPS);
		double tmpT2 = (t_conflict_2 * update_states_2->tas_knots * KNOTS_TO_FPS - d_separation) / (update_states_2->tas_knots * KNOTS_TO_FPS);

		if (tmpT1 > tmpT2) {
			delay_ac = 1;
			delay_step = floor(tmpT2 / t_step) + 1;

			update_states_1->tas_knots_before_held_cdnr = update_states_1->tas_knots;

			update_states_1->tas_knots = 0;
			update_states_1->V_ground = 0;

			cdnr_oss_doc << "At timestamp = " << t << ", aircraft " << update_states_1->acid << " conflicting with " << update_states_2->acid << ".  Holding " << update_states_1->acid << ".\n";
			cnt_event_cdnr++;
		} else if (tmpT2 > tmpT1) {
			delay_ac = 2;
			delay_step = floor(tmpT1 / t_step) + 1;

			update_states_2->tas_knots_before_held_cdnr = update_states_2->tas_knots;

			update_states_2->tas_knots = 0;
			update_states_2->V_ground = 0;

			cdnr_oss_doc << "At timestamp = " << t << ", aircraft " << update_states_2->acid << " conflicting with " << update_states_1->acid << ".  Holding " << update_states_2->acid << ".\n";
			cnt_event_cdnr++;
		} else {
			delay_ac = 1;
			delay_step = 1;

			update_states_1->tas_knots_before_held_cdnr = update_states_1->tas_knots;

			update_states_1->tas_knots = 0;
			update_states_1->V_ground = 0;

			cdnr_oss_doc << "At timestamp = " << t << ", aircraft " << update_states_1->acid << " conflicting with " << update_states_2->acid << ".  Holding " << update_states_1->acid << ".\n";
			cnt_event_cdnr++;
		}
	}
}

void ConflictDetectionAndResolution(const float t,
		update_states_t* update_states_1,
		const double latitude_rad_1,
		const double longitude_rad_1,
		const double altitude_ft_1,
		const double tas_knot_ground_1,
		const double fpa_rad_1,
		const double course_heading_rad_1,
		double& t_conflict_1,
		update_states_t* update_states_2,
		const double latitude_rad_2,
		const double longitude_rad_2,
		const double altitude_ft_2,
		const double tas_knot_ground_2,
		const double fpa_rad_2,
		const double course_heading_rad_2,
		double& t_conflict_2,
		const double radius_earth_ft,
		const float t_step,
		const double d_initiate,
		const double d_separation,
		int& delay_ac,
		int& delay_step) {
	bool isConflict = ConflictDetection(latitude_rad_1,
				longitude_rad_1,
				altitude_ft_1,
				tas_knot_ground_1,
				fpa_rad_1,
				course_heading_rad_1,
				t_conflict_1,
				latitude_rad_2,
				longitude_rad_2,
				altitude_ft_2,
				tas_knot_ground_2,
				fpa_rad_2,
				course_heading_rad_2,
				t_conflict_2,
				radius_earth_ft,
				t_step,
				d_initiate);
	if (isConflict) {
		ConflictResolution(t,
				update_states_1,
				update_states_2,
				tas_knot_ground_1,
				tas_knot_ground_2,
				radius_earth_ft,
				t_conflict_1,
				t_conflict_2,
				d_separation,
				delay_ac,
				delay_step);
	} else {
		delay_ac = -1;
		delay_step = -1;
	}
}

////////////////////////////////////////////////////////////////////////////////
// Public host API impl                                                       //
////////////////////////////////////////////////////////////////////////////////

int propagate_flights(const float& input_t_end, const float& input_t_step_airborne) {
	sim_id = 0;

	return propagate_flights(input_t_end,
			input_t_step_airborne,
			input_t_step_airborne,
			input_t_step_airborne);
}

int propagate_flights(const float& input_t_end,
		const float& input_t_step,
		const float& input_t_step_terminal,
		const float& input_t_step_airborne) {
	t_start = 0; // Reset
	t_end = input_t_end; // Update

	t_step = input_t_step;
	t_step_terminal = input_t_step_terminal;
	t_data_collection_period_airborne = input_t_step_airborne;

	if (flag_realTime_simulation) {
		// Set time steps of real-time simulation in seconds
		t_step = time_step_surface_realTime_simulation;
		t_step_terminal = time_step_terminal_realTime_simulation;
		t_data_collection_period_airborne = time_step_airborne_realTime_simulation;
	}

	tg_pathFilename_polygon.assign("NONE"); // Default
	tg_pathFilename_sigmet.assign("NONE"); // Default

	if (60 < t_step) {
		t_step = 60;
		printf("Simulation: Surface time step can't exceed 60 sec.  Resetting it to 60 sec");
	} else if (t_step <= 0) {
		t_step = 0.1;
		printf("Simulation: Surface time step must be larger than zero.  Resetting it to 0.1 sec");
	}

	if (60 < t_step_terminal) {
		t_step_terminal = 60;
		printf("Simulation: Terminal time step can't exceed 60 sec.  Resetting it to 60 sec");
	} else if (t_step_terminal <= 0) {
		t_step_terminal = t_step;
		printf("Simulation: Terminal time step must be larger than zero.  Resetting it to 0.1 sec");
	}

	if (60 < t_data_collection_period_airborne) {
		t_data_collection_period_airborne = 60;
		printf("Simulation: Time step(above TRACON) can't exceed 60 sec.  Resetting it to 60 sec");
	} else if (t_data_collection_period_airborne <= 0) {
		t_data_collection_period_airborne = t_step_terminal;
		printf("Simulation: Time step(above TRACON) must be larger than zero.  Resetting it to 0.1 sec");
	}

	nats_simulation_status = NATS_SIMULATION_STATUS_READY; // Reset

	num_flights = get_num_flights();

	// set device constant pointers
	set_device_adb_pointers();
	set_device_ac_pointers();
	set_device_ruc_pointers();

	streams = (cuda_stream_t*)malloc(NUM_STREAMS*sizeof(cuda_stream_t*));
	for (int i=0; i < NUM_STREAMS; ++i) {
		cuda_stream_create(&streams[i]);
	}

	stream_len = ceil((real_t)num_flights / (real_t)NUM_STREAMS);

	// compute the number of elements by which NUM_STREAMS*stream_len
	// exceeds num_flights
	num_overflow = NUM_STREAMS * stream_len - num_flights;

#if ENABLE_PROFILER
	cudaProfilerStart();
#endif

	// Allocate Risk Measures memory space
	ac_risk_measures_data = (ac_risk_measures_metrics_t*)calloc(num_flights, sizeof(ac_risk_measures_metrics_t));

	// Initialization of Risk Measure variables
	for (int i = 0; i < num_flights; i++) {
		ac_risk_measures_data[i].count_flight_phase = ac_risk_measures_metrics_template.count_flight_phase;
		ac_risk_measures_data[i].flight_phases = (ENUM_Flight_Phase*)calloc(ac_risk_measures_data[i].count_flight_phase, sizeof(ENUM_Flight_Phase));
		ac_risk_measures_data[i].flight_phase_risk = (aviation_occurence_risk_value_t**)calloc(ac_risk_measures_metrics_template.count_flight_phase, sizeof(aviation_occurence_risk_value_t*));

		// Insert data
		for (int j = 0; j < ac_risk_measures_metrics_template.count_flight_phase; j++) {
			ac_risk_measures_data[i].flight_phases[j] = ac_risk_measures_metrics_template.flight_phases[j];

			ac_risk_measures_data[i].flight_phase_risk[j] = (aviation_occurence_risk_value_t*)calloc(1, sizeof(aviation_occurence_risk_value_t));

			ac_risk_measures_data[i].flight_phase_risk[j]->record_count = ac_risk_measures_metrics_template.flight_phase_risk[j]->record_count;

			ac_risk_measures_data[i].flight_phase_risk[j]->codes = (char**)calloc(ac_risk_measures_data[i].flight_phase_risk[j]->record_count, sizeof(char*));

			for (int k = 0; k < ac_risk_measures_data[i].flight_phase_risk[j]->record_count; k++) {
				ac_risk_measures_data[i].flight_phase_risk[j]->codes[k] = NULL; // Reset
			}

			ac_risk_measures_data[i].flight_phase_risk[j]->risk_value = (double*)calloc(ac_risk_measures_data[i].flight_phase_risk[j]->record_count, sizeof(double));

			// Copy data from template
			for (int k = 0; k < ac_risk_measures_metrics_template.flight_phase_risk[j]->record_count; k++) {

				if (ac_risk_measures_metrics_template.flight_phase_risk[j]->codes[k] != NULL) {
					ac_risk_measures_data[i].flight_phase_risk[j]->codes[k] = (char*)calloc(strlen(ac_risk_measures_metrics_template.flight_phase_risk[j]->codes[k])+1, sizeof(char));;
					strcpy(ac_risk_measures_data[i].flight_phase_risk[j]->codes[k], ac_risk_measures_metrics_template.flight_phase_risk[j]->codes[k]);
					ac_risk_measures_data[i].flight_phase_risk[j]->codes[k][strlen(ac_risk_measures_metrics_template.flight_phase_risk[j]->codes[k])] = '\0';
				}

				ac_risk_measures_data[i].flight_phase_risk[j]->risk_value[k] = 0.0; // Reset
			}
		}
	}

	printf("\nFlight propagation is initialized.  Waiting for start.\n");

	std::thread thread_propagation_proc(propagate_flights_proc);
	thread_propagation_proc.detach();

	return 0;
}

void propagate_flights_proc() {
	float t_duration_target = -1; // Reset

	string tmpStr;

	vector<string> vector_filenames_sigmet;
	vector<string> vector_filenames_pirep;

	string weatherDir = "share/tg/weather";

	string str_rap_base_filename;

	cdnr_oss_doc.str(""); // Clean up

	vector_tactical_weather_waypoint.clear(); // Clean up

	dim3 block_size(BLOCK_SIZE, 1, 1);
	dim3 grid_size( ceil((float)num_flights / (float)block_size.x), 1, 1);

	// Flag to process airborne state to variable "g_trajectories"
	bool flag_proc_airborne_trajectory = false; // Reset

	ground_departing_data_init = (bool*)malloc(num_flights * sizeof(bool));
	ground_landing_data_init = (bool*)malloc(num_flights * sizeof(bool));

	// Allocate memory space for all update_states variables
	array_update_states_ptr = (update_states_t**)calloc(num_flights, sizeof(update_states_t*));

	// If enable strategic weather avoidance
	if (flag_enable_strategic_weather_avoidance) {
		load_rg_data_files_NATS(g_wind_dir,
				g_cifp_file,
				"share/rg/config/airport.config",
				"share/rg/config/airway.config",
				"share/rg/config/enrtfix.config");
	}

	// Initializing data
	for (int i = 0; i < num_flights; i++) {
		ground_departing_data_init[i] = false;
		ground_landing_data_init[i] = false;

		c_flag_reached_meterfix_point[i] = false;

		c_acceleration_aiming_waypoint_node_ptr[i] = NULL;
		c_acceleration[i] = -1.0;
		c_V2_point_latitude_deg[i] = -1.0;
		c_V2_point_longitude_deg[i] = -1.0;
		c_estimate_touchdown_point_latitude_deg[i] = 0;
		c_estimate_touchdown_point_longitude_deg[i] = 0;

		c_t_takeoff[i] = -1;
		c_t_landing[i] = -1;
		c_hold_flight_phase[i] = FLIGHT_PHASE_ORIGIN_GATE;
		c_course_rad_runway[i] = DBL_MIN;
		c_course_rad_taxi[i] = DBL_MIN;
	}

	// Calculate estimate altitude of waypoints ============================================
	waypoint_node_t* tmpWaypoint_validAltitude;
	waypoint_node_t* tmpAirborne_Flight_Plan_ptr;
	double distance_prevWaypoint_to_validAltitudeWaypoint;
	double distance_prevWaypoint_to_current;
	double distance_current_to_validAltitudeWaypoint;
	double distance_validAltitudeWaypoint_to_finalWaypoint;
	bool flag_reached_TOC = false;
	bool flag_reached_TOD = false;

	for (int i = 0; i < num_flights; i++) {
		if ((!h_aircraft_soa.flag_geoStyle[i]) && (array_Airborne_Flight_Plan_ptr[i] != NULL)) {
			flag_reached_TOC = false; // Reset
			flag_reached_TOD = false; // Reset

			// If there is no TOD point
			if (array_Airborne_Flight_Plan_tod_ptr[i] == NULL) {
				flag_reached_TOD = true;
			}

			if ((array_Airborne_Flight_Plan_Final_Node_ptr[i]->wpname != NULL) && (strlen(array_Airborne_Flight_Plan_Final_Node_ptr[i]->wpname) > 0) && (indexOf(array_Airborne_Flight_Plan_Final_Node_ptr[i]->wpname, "RW") > -1)) {
				array_Airborne_Flight_Plan_Final_Node_ptr[i]->altitude_estimate = g_trajectories.at(i).destination_airport_elevation_ft;
			}

			if ((array_Airborne_Flight_Plan_ptr[i]->wpname != NULL) && (strlen(array_Airborne_Flight_Plan_ptr[i]->wpname) > 0) && (indexOf(array_Airborne_Flight_Plan_ptr[i]->wpname, "RW") > -1)) {
				tmpAirborne_Flight_Plan_ptr = array_Airborne_Flight_Plan_ptr[i]->next_node_ptr;
			} else {
				tmpAirborne_Flight_Plan_ptr = array_Airborne_Flight_Plan_ptr[i];
			}

			// Traverse waypoints
			while (tmpAirborne_Flight_Plan_ptr != NULL) {
				if ((tmpAirborne_Flight_Plan_ptr->wpname != NULL) && (strcmp(tmpAirborne_Flight_Plan_ptr->wpname, INITIAL_PT) == 0)) {
					// Do nothing
				} else if (((tmpAirborne_Flight_Plan_ptr->proctype != NULL) && (strlen(tmpAirborne_Flight_Plan_ptr->proctype) > 0) && (indexOf(tmpAirborne_Flight_Plan_ptr->proctype, "ENROUTE") >= 0))
						|| ((tmpAirborne_Flight_Plan_ptr->wpname != NULL) && (strlen(tmpAirborne_Flight_Plan_ptr->wpname) > 0) &&
								((indexOf(tmpAirborne_Flight_Plan_ptr->wpname, "TOP_OF_CLIMB_PT") > -1) || (indexOf(tmpAirborne_Flight_Plan_ptr->wpname, "TOP_OF_DESCENT_PT") > -1)))
						) {
					tmpAirborne_Flight_Plan_ptr->altitude_estimate = c_cruise_alt_ft[i];

					if (indexOf(tmpAirborne_Flight_Plan_ptr->wpname, "TOP_OF_CLIMB_PT") > -1) {
						flag_reached_TOC = true;
					} else if (indexOf(tmpAirborne_Flight_Plan_ptr->wpname, "TOP_OF_DESCENT_PT") > -1) {
						flag_reached_TOD = true;
					}
				} else if ((tmpAirborne_Flight_Plan_ptr->wpname != NULL) && (indexOf(tmpAirborne_Flight_Plan_ptr->wpname, "RW") > -1)) {
					tmpStr.assign(tmpAirborne_Flight_Plan_ptr->wpname);
					if (tmpStr.find(g_trajectories.at(i).destination_airport) != string::npos) {
						tmpAirborne_Flight_Plan_ptr->altitude_estimate = g_trajectories.at(i).destination_airport_elevation_ft;
					}
				} else {
					// Get altitude from waypoint alt1, alt2 limit
					tmpAirborne_Flight_Plan_ptr->altitude_estimate = get_waypoint_altitude(tmpAirborne_Flight_Plan_ptr);

					if (tmpAirborne_Flight_Plan_ptr->altitude_estimate <= 0) {
						// From the current waypoint, find the waypoint with valid altitude
						if ((tmpAirborne_Flight_Plan_ptr != NULL) && (tmpAirborne_Flight_Plan_ptr->proctype != NULL) && (strlen(tmpAirborne_Flight_Plan_ptr->proctype) > 0)) {
							tmpWaypoint_validAltitude = tmpAirborne_Flight_Plan_ptr;
							while ((tmpWaypoint_validAltitude != NULL) && (get_waypoint_altitude(tmpWaypoint_validAltitude) <= 0)) {
								tmpWaypoint_validAltitude = tmpWaypoint_validAltitude->next_node_ptr;
							}

							if (indexOf(tmpAirborne_Flight_Plan_ptr->proctype, "SID") >= 0) {
								if (flag_reached_TOC) {
									tmpAirborne_Flight_Plan_ptr->altitude_estimate = c_cruise_alt_ft[i];
								} else {
									if ((tmpWaypoint_validAltitude != NULL) && (tmpAirborne_Flight_Plan_ptr->prev_node_ptr != NULL)) {
										distance_prevWaypoint_to_validAltitudeWaypoint = compute_route_distance_between_waypoints(tmpAirborne_Flight_Plan_ptr->prev_node_ptr, tmpWaypoint_validAltitude);
										distance_prevWaypoint_to_current = compute_distance_gc(tmpAirborne_Flight_Plan_ptr->prev_node_ptr->latitude,
												tmpAirborne_Flight_Plan_ptr->prev_node_ptr->longitude,
												tmpAirborne_Flight_Plan_ptr->latitude,
												tmpAirborne_Flight_Plan_ptr->longitude,
												tmpAirborne_Flight_Plan_ptr->prev_node_ptr->altitude_estimate
												);

										tmpAirborne_Flight_Plan_ptr->altitude_estimate = tmpAirborne_Flight_Plan_ptr->prev_node_ptr->altitude_estimate - ((tmpAirborne_Flight_Plan_ptr->prev_node_ptr->altitude_estimate - get_waypoint_altitude(tmpWaypoint_validAltitude)) * distance_prevWaypoint_to_current / distance_prevWaypoint_to_validAltitudeWaypoint);
									}
								}
							} else {
								if (!flag_reached_TOD) {
									tmpAirborne_Flight_Plan_ptr->altitude_estimate = c_cruise_alt_ft[i];
								} else {
									if ((tmpWaypoint_validAltitude != NULL) && (array_Airborne_Flight_Plan_Final_Node_ptr[i] != NULL)) {
										distance_current_to_validAltitudeWaypoint = compute_distance_gc(tmpAirborne_Flight_Plan_ptr->latitude,
												tmpAirborne_Flight_Plan_ptr->longitude,
												tmpWaypoint_validAltitude->latitude,
												tmpWaypoint_validAltitude->longitude,
												tmpWaypoint_validAltitude->altitude_estimate
												);
										distance_validAltitudeWaypoint_to_finalWaypoint = compute_route_distance_between_waypoints(tmpWaypoint_validAltitude, array_Airborne_Flight_Plan_Final_Node_ptr[i]);

										tmpAirborne_Flight_Plan_ptr->altitude_estimate = array_Airborne_Flight_Plan_Final_Node_ptr[i]->altitude_estimate + (get_waypoint_altitude(tmpWaypoint_validAltitude) - array_Airborne_Flight_Plan_Final_Node_ptr[i]->altitude_estimate) * (distance_current_to_validAltitudeWaypoint + distance_validAltitudeWaypoint_to_finalWaypoint) / distance_validAltitudeWaypoint_to_finalWaypoint;
									}
								}
							}
						}
					}
				}

				tmpAirborne_Flight_Plan_ptr = tmpAirborne_Flight_Plan_ptr->next_node_ptr; // Update pointer
			}
		}
	}
	// end - Calculate estimate altitude of waypoints ============================================

	g_start_time_realTime_simulation = 0; // Reset

	while (1) {
		if (nats_simulation_status == NATS_SIMULATION_STATUS_START) {
			nats_simulation_timestamp = 0; // Reset

			if (flag_realTime_simulation) {
				sleep_duration_realTime_simulation = t_step * 1000 * 1000 - pause_duration_realTime_simulation;

				printf("\n!!!! Real-time simulation enabled !!!!\n");
			}

			for (int i = 0; i < num_flights; ++i) {
				debug_list_all_waypoint_nodes(i);
			}

			if (nats_simulation_duration > 0) {
				t_duration_target = nats_simulation_duration;

				printf("\nBegin flight propagation for %f seconds duration from %f to %f second with time step %.1f seconds (surface), %.1f seconds (terminal area), %.1f seconds (above TRACON)\n\n", nats_simulation_duration, t_start, t_end, t_step, t_step_terminal, t_data_collection_period_airborne);

				nats_simulation_duration = -1; // Reset
			} else {
				if (!flag_realTime_simulation) {
					printf("\nBegin flight propagation from %f to %f second with time step %.1f seconds (surface), %.1f seconds (terminal area), %.1f seconds (above TRACON)\n\n", t_start, t_end, t_step, t_step_terminal, t_data_collection_period_airborne);
				} else {
					printf("\nBegin real-time flight propagation with time step %.1f seconds (surface), %.1f seconds (terminal area), %.1f seconds (above TRACON)\n\n", t_step, t_step_terminal, t_data_collection_period_airborne);
				}
			}

			// ================================================================

			// Loop from t_start to t_end with increments of t_step
			float t = t_start;
			// For every looping of simulation, the logic needs to take care of every aircraft
			// Therefore, there are many sub-looping blocks inside this "while" block.  Each sub-looping block will run the corresponding processing on all aircraft.
			while ((t < t_end) || (flag_realTime_simulation)) {
logger_printLog(LOG_LEVEL_DEBUG, 1, "Propagation timestamp = %f\n", t);
				if ((flag_realTime_simulation) && (g_start_time_realTime_simulation == 0)) {
					if (g_start_time == 0) {
						g_start_time_realTime_simulation = getCurrentCpuTime();
					} else {
						g_start_time_realTime_simulation = g_start_time;
					}
				}

				nats_simulation_timestamp = t;
				flag_proc_airborne_trajectory = false; // Reset

				if (fmod(trunc_double(t, 1), t_data_collection_period_airborne) == 0) {
					flag_proc_airborne_trajectory = true;
				}

				// Initialization of state data
				for (int i = 0; i < num_flights; i++) {
					AdbPTFModel tmpAdbPTFModel;

					// Current timestamp is larger than aircraft start time and update_states_t pointer is not created.  This means this aircraft hasn't processed initial data.
					// Here, process initial data of the aircraft
					// For every aircraft, this only processes once(at the very first time)
					if ((t >= g_trajectories.at(i).start_time) && (array_update_states_ptr[i] == NULL)) {
						array_update_states_ptr[i] = (update_states_t*)calloc(1, sizeof(update_states_t));

						update_states_t* update_states = array_update_states_ptr[i];

						update_states->target_WaypointNode_ptr = NULL; // Reset
						update_states->flag_target_waypoint_change = false; // Reset
						update_states->flag_aircraft_held_strategic = false; // Reset
						update_states->flag_aircraft_held_tactical = false; // Reset
						update_states->flag_data_initialized = false; // Reset
						update_states->flag_aircraft_spacing = false; // Reset
						update_states->simulation_user_incident_index = -1; // Reset

						update_states->acid = (char*)calloc(g_trajectories.at(i).callsign.length()+1, sizeof(char));
						strcpy(update_states->acid, g_trajectories.at(i).callsign.c_str());
						update_states->acid[g_trajectories.at(i).callsign.length()] = '\0';

						if ((incidentFlightPhaseMap.find(string(update_states->acid)) != incidentFlightPhaseMap.end()) && (0 < incidentFlightPhaseMap.at(string(update_states->acid)).size())) {
							update_states->flag_ifs_exist = true;
						} else {
							update_states->flag_ifs_exist = false;
						}

						update_states->flight_phase = c_flight_phase[i];

						if ((update_states->flight_phase == FLIGHT_PHASE_CLIMBOUT) || (update_states->flight_phase == FLIGHT_PHASE_CLIMB_TO_CRUISE_ALTITUDE) || (update_states->flight_phase == FLIGHT_PHASE_INITIAL_DESCENT) || (update_states->flight_phase == FLIGHT_PHASE_FINAL_APPROACH)) {
							if ((update_states->flight_phase == FLIGHT_PHASE_CLIMBOUT) || (update_states->flight_phase == FLIGHT_PHASE_CLIMB_TO_CRUISE_ALTITUDE)) {
								tmpAdbPTFModel = g_adb_ptf_models.at(c_adb_aircraft_type_index[i]);
								c_rocd_fps[i] = tmpAdbPTFModel.getClimbRate(c_altitude_ft[i], LOW) / 60;
							} else {
								tmpAdbPTFModel = g_adb_ptf_models.at(c_adb_aircraft_type_index[i]);
								c_rocd_fps[i] = (-1) * tmpAdbPTFModel.getDescentRate(c_altitude_ft[i], NOMINAL) / 60;
							}

							//update_states->target_WaypointNode_ptr = array_Airborne_Flight_Plan_ptr[i]->next_node_ptr;
						} else if ((c_altitude_ft[i] == 0) || (g_trajectories.at(i).origin_airport_elevation_ft == c_altitude_ft[i])) {
							c_rocd_fps[i] = 0;
						} else if (((update_states->flight_phase == FLIGHT_PHASE_CLIMB_TO_CRUISE_ALTITUDE) && (update_states->altitude_ft >= update_states->cruise_alt_ft))
										||
									(update_states->flight_phase == FLIGHT_PHASE_CRUISE)
									) {
							update_states->flight_phase = FLIGHT_PHASE_CRUISE;
							c_flight_phase[i] = FLIGHT_PHASE_CRUISE;
						}

						c_target_waypoint_node_ptr[i] = h_aircraft_soa.target_waypoint_node_ptr[i];

						update_states->target_WaypointNode_ptr = h_aircraft_soa.target_waypoint_node_ptr[i];

						if (0 < c_tas_knots[i]) {
							c_fpa_rad[i] = asin(c_rocd_fps[i] / c_tas_knots[i]);
						} else {
							c_fpa_rad[i] = 0;
						}

						update_states->fpa_rad = c_fpa_rad[i];

						update_states->V_horizontal = c_tas_knots[i] * cos(c_fpa_rad[i]);

						if (isFlightPhase_in_airborne(update_states->flight_phase) && (g_trajectories.at(i).origin_airport_elevation_ft < c_altitude_ft[i])) {
							update_states->V_ground = aircraft_compute_ground_speed(t,
								c_latitude_deg[i],
								c_longitude_deg[i],
								c_altitude_ft[i],
								update_states->V_horizontal,
								c_course_rad[i]);
						} else {
							update_states->V_ground = h_aircraft_soa.tas_knots[i];
						}

						if (flag_sector_available) {
							c_sector_index[i] = compute_flight_sector(c_latitude_deg[i], c_longitude_deg[i], 0, -1);
						}

						kernel_update_states_prepare_data(update_states,
								i,
								t_step,
								t_step_terminal,
								t_data_collection_period_airborne);

						// Write data back to c_ variables
						kernel_update_states_write_data_from_updateStates_to_c(update_states, i);
					}
				} // end - Initialization of state data

				for (int i = 0; i < num_flights; i++) {
					if (array_update_states_ptr[i] != NULL) {
						array_update_states_ptr[i]->flag_aircraft_held_tactical = false; // Reset
					}
				}

				// Output trajectory data to file
				traj_data_callback(t, t_step_terminal, flag_proc_airborne_trajectory);

				for (int i = 0; i < num_flights; i++) {
					if (array_update_states_ptr[i] != NULL) {
						array_update_states_ptr[i]->flag_data_initialized = true;
					}
				}

				for (int i = 0; i < num_flights; i++) {
					if (array_update_states_ptr[i] != NULL) {
						if ((array_update_states_ptr[i]->altitude_ft == 0)
							|| ((array_update_states_ptr[i]->flag_abnormal_on_runway)
									|| (array_update_states_ptr[i]->flight_phase == FLIGHT_PHASE_DESTINATION_GATE)
									|| (array_update_states_ptr[i]->flight_phase == FLIGHT_PHASE_RUNWAYUNDERSHOOT)
									|| (array_update_states_ptr[i]->flight_phase == FLIGHT_PHASE_RUNWAYOVERSHOOT)
									|| (array_update_states_ptr[i]->flight_phase == FLIGHT_PHASE_OUTOFRUNWAY))
							|| ((array_update_states_ptr[i]->flight_phase == FLIGHT_PHASE_RUNWAY_THRESHOLD_DEPARTING) && (array_Airborne_Flight_Plan_ptr[i] == NULL))
							|| ((array_update_states_ptr[i]->flight_phase == FLIGHT_PHASE_TOUCHDOWN) && (h_landing_taxi_plan.waypoint_node_ptr[i] == NULL))
							) {
							if ((array_update_states_ptr[i]->flight_phase == FLIGHT_PHASE_RUNWAY_THRESHOLD_DEPARTING) && (array_Airborne_Flight_Plan_ptr[i] == NULL)) {
								printf("%s: No airborne flight plan found.  Simulation ended.\n", array_update_states_ptr[i]->acid);
							}

							array_update_states_ptr[i]->flight_phase = FLIGHT_PHASE_LANDED;
							array_update_states_ptr[i]->landed_flag = true;
							c_landed_flag[i] = true;

							h_aircraft_soa.flight_phase[i] = FLIGHT_PHASE_LANDED;
						}
					}
				}

				string currentCenter;
				int meterFixListSize = 0;
				int meterFixCount = 0;
				
				// Traverse all aircraft
				for (int j = 0; j < num_flights; j++) {
					currentCenter = "";

					if (currentCenter == "")
						continue;
					else if (currentCenter == "ZHN" || currentCenter == "ZAN")
					    currentCenter = "P" + currentCenter;
				    else
				        currentCenter = "K" + currentCenter;

					meterFixListSize = 0;
					if (meterFixMap.find(currentCenter) != meterFixMap.end())
						meterFixListSize = meterFixMap[currentCenter].size();

					for (meterFixCount = 0; meterFixCount < meterFixListSize; meterFixCount++) {
						NatsWaypoint tmpNatsWaypoint;
						tmpNatsWaypoint.name = meterFixMap[currentCenter].at(meterFixCount).c_str();

						vector<NatsWaypoint>::iterator ite_wp = find(g_waypoints.begin(), g_waypoints.end(), tmpNatsWaypoint);
						bool waypointExistsInPlan = false;
						waypoint_node_t* tmpWaypoint_Node_ptr = array_Airborne_Flight_Plan_ptr[j];
						while (tmpWaypoint_Node_ptr != NULL) {
							if (!strcmp(tmpWaypoint_Node_ptr->wpname, tmpNatsWaypoint.name.c_str()))
								waypointExistsInPlan = true;
							tmpWaypoint_Node_ptr = tmpWaypoint_Node_ptr->next_node_ptr;
						}
						update_states_t* tmp_update_states_ptr = array_update_states_ptr[j];
						if ((tmp_update_states_ptr != NULL) && (tmp_update_states_ptr->flag_data_initialized) && waypointExistsInPlan) {
							if (!tmp_update_states_ptr->flag_aircraft_spacing) {
								if (spacingDistMap[currentCenter][tmpNatsWaypoint.name].first == "DISTANCE") {
									if ((compute_distance_gc(tmp_update_states_ptr->lat,
											tmp_update_states_ptr->lon,
											ite_wp->latitude,
											ite_wp->longitude,
											tmp_update_states_ptr->altitude_ft)) < spacingDistMap[currentCenter][tmpNatsWaypoint.name].second) {
										tmp_update_states_ptr->flag_aircraft_spacing = true;
										cout << "Aircraft: " << g_trajectories.at(j).callsign << " held for spacing at meter fix point " << tmpNatsWaypoint.name << endl;
									}
								}
								else if (spacingDistMap[currentCenter][tmpNatsWaypoint.name].first == "TIME") {
									if ((compute_distance_gc(tmp_update_states_ptr->lat,
											tmp_update_states_ptr->lon,
											ite_wp->latitude,
											ite_wp->longitude,
											tmp_update_states_ptr->altitude_ft) * 0.00018939 ) < (tmp_update_states_ptr->V_ground * 0.0191796575 * spacingDistMap[currentCenter][tmpNatsWaypoint.name].second)) {
										tmp_update_states_ptr->flag_aircraft_spacing = true;
										cout << "Aircraft: " << g_trajectories.at(j).callsign << " held for spacing at meter fix point " << tmpNatsWaypoint.name << endl;
									}
								}
							}
						}
					}
				}

				// Check tactical weather avoidance
				// User can specify certain waypoints to be blocked for a period of time
				// If the distance to the blocked waypoint is close, we freeze the aircraft
				if (0 < vector_tactical_weather_waypoint.size()) {
					vector<WeatherWaypoint>::iterator ite_weatherWaypoint;
					ite_weatherWaypoint = vector_tactical_weather_waypoint.begin();

					while (ite_weatherWaypoint != vector_tactical_weather_waypoint.end()) {
						WeatherWaypoint tmp_weatherWaypoint;
						tmp_weatherWaypoint.waypoint_name = ite_weatherWaypoint->waypoint_name;
						tmp_weatherWaypoint.durationSecond = ite_weatherWaypoint->durationSecond;

						NatsWaypoint tmpNatsWaypoint;
						tmpNatsWaypoint.name = tmp_weatherWaypoint.waypoint_name;

						vector<NatsWaypoint>::iterator ite_wp = find(g_waypoints.begin(), g_waypoints.end(), tmpNatsWaypoint);
						if (ite_wp != g_waypoints.end()) {
							cout << "Processing tactical weather avoidance of waypoint " << tmp_weatherWaypoint.waypoint_name << " at timestamp = " << t << " sec" << endl;

							// Traverse all aircraft
							for (int j = 0; j < num_flights; j++) {
								update_states_t* tmp_update_states_ptr = array_update_states_ptr[j];
								if ((tmp_update_states_ptr != NULL) && (tmp_update_states_ptr->flag_data_initialized)) {
									if (!tmp_update_states_ptr->flag_aircraft_held_strategic) {
										// Calculate distance from aircraft to the waypoint
										// If distance is less than 50nm, hold the aircraft
										if (compute_distance_gc(tmp_update_states_ptr->lat,
												tmp_update_states_ptr->lon,
												ite_wp->latitude,
												ite_wp->longitude,
												tmp_update_states_ptr->altitude_ft) < (50 * NauticalMilestoFeet)) {
											tmp_update_states_ptr->flag_aircraft_held_tactical = true;

											cout << "Aircraft: " << g_trajectories.at(j).callsign << " held due to tactical weather avoidance of waypoint " << tmp_weatherWaypoint.waypoint_name << endl;
										}
									}
								}
							}
						}

						tmp_weatherWaypoint.durationSecond = tmp_weatherWaypoint.durationSecond - t_step; // Update duration value

						if (tmp_weatherWaypoint.durationSecond <= 0) {  // If the weather waypoint is not active
							// Erase vector element
							vector_tactical_weather_waypoint.erase(ite_weatherWaypoint);
						} else {
							// Update vector element
							*ite_weatherWaypoint = tmp_weatherWaypoint;
						}

						if (ite_weatherWaypoint != vector_tactical_weather_waypoint.end()) {
							ite_weatherWaypoint++;
						}
					}
				}

				// Process stage 1 calculation on all aircrafts
				for (int i = 0; i < NUM_STREAMS; ++i) {
					launch_kernel_stage1(grid_size, block_size, 0, streams[i], num_flights, t, t_step, t_step_terminal, t_data_collection_period_airborne, i, flag_proc_airborne_trajectory);
				}

				// Synchronize data from d_aircraft_soa to h_aircraft_soa so that Java functions can access latest data
				synchronize_data_from_D_to_H();

				// Propagate state data on external aircrafts
				if (t > 0) {
					launch_kernel_external_aircraft(t, t_step, t_step_terminal, t_data_collection_period_airborne);
				}

				if ((t_duration_target > 0) && (t >= t_duration_target)) {
					t_duration_target = -1; // Reset

					nats_simulation_status = NATS_SIMULATION_STATUS_PAUSE;

					printf("    Duration satisfied.  Automatically pausing at time = %f seconds\n", t);
				}

				if (flag_realTime_simulation) {
					if (t == 0) {
						long tmpUTC = getCurrentCpuTime_milliSec();
						nextPropagation_utc_time_realTime_simulation = tmpUTC + long(pause_duration_realTime_simulation / (2*1000));
					}

					usleep(pause_duration_realTime_simulation);
				} else {
					// Handle simulation PAUSE controlling
					if (nats_simulation_status == NATS_SIMULATION_STATUS_PAUSE) {
						while (1) {
							usleep(nats_simulation_check_interval);

							if ((nats_simulation_status == NATS_SIMULATION_STATUS_RESUME) || (nats_simulation_status == NATS_SIMULATION_STATUS_STOP)) {
								if (nats_simulation_status == NATS_SIMULATION_STATUS_RESUME) {
									if (nats_simulation_duration > 0) {
										t_duration_target = t + nats_simulation_duration; // Calculate target time

										printf("    Resume flight propagation for %f seconds duration\n", nats_simulation_duration);

										nats_simulation_duration = -1; // Reset
									} else {
										printf("    Resume flight propagation\n");
									}
								}

								break;
							}
						}
					}
				}

				// Handle simulation STOP controlling
				if (nats_simulation_status == NATS_SIMULATION_STATUS_STOP) {
					printf("Stop flight propagation\n");

					break;
				}

				// After pause, some data needs to be synchronized
				for (int i = 0; i < num_flights; i++) {
					if (array_update_states_ptr[i] != NULL) {
						array_update_states_ptr[i]->tas_knots = h_aircraft_soa.tas_knots[i];
						array_update_states_ptr[i]->V_horizontal = array_update_states_ptr[i]->tas_knots;

						if (!isFlightPhase_in_ground_departing(array_update_states_ptr[i]->flight_phase) && !isFlightPhase_in_ground_landing(array_update_states_ptr[i]->flight_phase)) {
							array_update_states_ptr[i]->V_ground = aircraft_compute_ground_speed(t,
									array_update_states_ptr[i]->lat,
									array_update_states_ptr[i]->lon,
									array_update_states_ptr[i]->altitude_ft,
									array_update_states_ptr[i]->V_horizontal,
									array_update_states_ptr[i]->hdg_rad);
						}

						h_aircraft_soa.tas_knots_ground[i] = array_update_states_ptr[i]->V_ground;
					}
				}

				// If the timestamp is an exact value of hour, execute the corresponding logic at every exact hour.
				if (fmod(t, 3600) == 0) {
					printf("    Simulation time: %2.0f hours\n", (float)t/(float)3600);

					// Strategic weather avoidance
					if (flag_enable_strategic_weather_avoidance) {
						vector<double> vector_waypoint_lat_deg;
						vector<double> vector_waypoint_lon_deg;

						vector<double> vector_reroute_waypoint_lat_deg;
						vector<double> vector_reroute_waypoint_lon_deg;
						vector<pair<int,int> > vector_similar_idx_wpts;

						set<pair<double, double>> set_waypoint_pairs;
						set<pair<double, double>> set_reroute_waypoint_pairs;

						string tmpDir;

						tg_pathFilename_polygon = trim(tg_pathFilename_polygon);
						if ((tg_pathFilename_polygon.length() == 0)
								) {
							// Start looking for the latest polygon file

							tmpDir.assign(GNATS_SERVER_HOME);
							tmpDir.append("/share/rg/polygons");

							string latest_polygon_filename = get_latestFilename(tmpDir.c_str(), ".dat");

							if (latest_polygon_filename.length() > 0) {
								tg_pathFilename_polygon.assign("share/rg/polygons/");
								tg_pathFilename_polygon.append(latest_polygon_filename);
							}
						}

						tg_pathFilename_sigmet = trim(tg_pathFilename_sigmet);
						if ((tg_pathFilename_sigmet.length() == 0)
								) {
							// Start looking for the latest sigmet file

							tmpDir.assign(GNATS_SERVER_HOME);
							tmpDir.append("/share/tg/weather");

							string latest_sigmet_filename = get_latestFilename(tmpDir.c_str(), ".sigmet");

							if (latest_sigmet_filename.length() > 0) {
								tg_pathFilename_sigmet.assign("share/tg/weather/");
								tg_pathFilename_sigmet.append(latest_sigmet_filename);
							}
						}

						// Check and add aircraft flight plan waypoints into airway data if it doesn't exist
						for (int i = 0; i < num_flights; i++) {
							if ((array_update_states_ptr[i] != NULL) && (array_update_states_ptr[i]->flag_data_initialized)) {
								supplement_airways_by_flightPlans(array_update_states_ptr[i]->target_WaypointNode_ptr);
							}
						}

						// For every flight, find the optimized path
						for (int i = 0; i < num_flights; i++) {
							if ((!g_trajectories.at(i).flag_externalAircraft) && (array_update_states_ptr[i] != NULL) && (array_update_states_ptr[i]->target_WaypointNode_ptr != NULL) && (array_update_states_ptr[i]->target_WaypointNode_ptr->proctype != NULL) && (indexOf(array_update_states_ptr[i]->target_WaypointNode_ptr->proctype, "ENROUTE") == 0)
									&& (array_update_states_ptr[i]->target_WaypointNode_ptr->wpname != NULL) && (indexOf(array_update_states_ptr[i]->target_WaypointNode_ptr->wpname, "TOP_OF_DESCENT_PT") < 0)
									&& (array_update_states_ptr[i]->target_WaypointNode_ptr->next_node_ptr != NULL) && (indexOf(array_update_states_ptr[i]->target_WaypointNode_ptr->next_node_ptr->proctype, "ENROUTE") == 0)) {
								array_update_states_ptr[i]->flag_aircraft_held_strategic = false; // Reset

								vector_waypoint_lat_deg.clear(); // Reset
								vector_waypoint_lon_deg.clear(); // Reset

								set_waypoint_pairs.clear(); // Reset

								vector_reroute_waypoint_lat_deg.clear(); // Reset
								vector_reroute_waypoint_lon_deg.clear(); // Reset
								vector_similar_idx_wpts.clear(); // Reset

								set_reroute_waypoint_pairs.clear(); // Reset

								vector_waypoint_lat_deg.push_back(array_update_states_ptr[i]->lat);
								vector_waypoint_lon_deg.push_back(array_update_states_ptr[i]->lon);

								vector_reroute_waypoint_lat_deg.push_back(array_update_states_ptr[i]->lat);
								vector_reroute_waypoint_lon_deg.push_back(array_update_states_ptr[i]->lon);

								set_waypoint_pairs.insert(pair<double, double>(array_update_states_ptr[i]->lat, array_update_states_ptr[i]->lon));

								waypoint_node_t* postRG_WaypointNode_ptr = NULL; // Store waypoint node pointer after RG logic

								waypoint_node_t* tmp_new_waypoint_node_ptr; // Waypoint node pointer of the new waypoint

								waypoint_node_t* tmpWaypointNode_ptr = NULL;

								tmpWaypointNode_ptr = array_update_states_ptr[i]->target_WaypointNode_ptr;
								while (tmpWaypointNode_ptr != NULL) {
									if ((strcmp(tmpWaypointNode_ptr->proctype, "ENROUTE") == 0) || (strcmp(tmpWaypointNode_ptr->wpname, TOP_OF_DESCENT_PT) == 0)) {
										vector_waypoint_lat_deg.push_back(tmpWaypointNode_ptr->latitude);
										vector_waypoint_lon_deg.push_back(tmpWaypointNode_ptr->longitude);

										set_waypoint_pairs.insert(pair<double, double>(tmpWaypointNode_ptr->latitude, tmpWaypointNode_ptr->longitude));
									}

									tmpWaypointNode_ptr = tmpWaypointNode_ptr->next_node_ptr;
								}

								runRgForNATS(tg_pathFilename_polygon,
										tg_pathFilename_sigmet,
										tg_pathFilename_pirep,
										vector_waypoint_lat_deg,
										vector_waypoint_lon_deg,
										vector_reroute_waypoint_lat_deg,
										vector_reroute_waypoint_lon_deg,
										vector_similar_idx_wpts,
										g_cifp_file);

								if (vector_reroute_waypoint_lat_deg.size() <= 2) {
									// Freeze this aircraft
									array_update_states_ptr[i]->flag_aircraft_held_strategic = true;

									cout << g_trajectories.at(i).callsign << ": no available route in weather zone.  This aircraft will be freezed." << endl;
								} else {
									bool tmpFlag_waypoint_change = false;
									for (int cntt = 0; cntt < vector_similar_idx_wpts.size(); cntt++) {
										std::pair<int,int> simidx = vector_similar_idx_wpts.at(cntt);

										if (simidx.first != simidx.second) {
											tmpFlag_waypoint_change = true;
										}
									}

									if (tmpFlag_waypoint_change)
										printf("%s: Found new route to avoid weather issue.  Processing new flight plan.\n", g_trajectories.at(i).callsign.c_str());

									for (int p = 0; p < vector_reroute_waypoint_lat_deg.size(); p++) {
										set_reroute_waypoint_pairs.insert(pair<double, double>(vector_reroute_waypoint_lat_deg.at(p), vector_reroute_waypoint_lon_deg.at(p)));
									}

									int idx_newly_created_waypoint = 0;
									tmpWaypointNode_ptr = array_update_states_ptr[i]->target_WaypointNode_ptr;

									if (vector_reroute_waypoint_lat_deg.size() > 1) {
										std::string tmpString;

										for (int idx_vector_reroute = 1; idx_vector_reroute < vector_reroute_waypoint_lat_deg.size(); idx_vector_reroute++) {
											// Unnecessary to process if idx_vector_reroute is the last one
											if (idx_vector_reroute == vector_reroute_waypoint_lat_deg.size()-1)
												break;

											// Store first waypoint node pointer
											if (idx_vector_reroute == 1) {
												postRG_WaypointNode_ptr = tmpWaypointNode_ptr;
											}

											// New waypoint to insert
											if (set_waypoint_pairs.find(pair<double, double>(vector_reroute_waypoint_lat_deg.at(idx_vector_reroute), vector_reroute_waypoint_lon_deg.at(idx_vector_reroute))) == set_waypoint_pairs.end()) {
												tmp_new_waypoint_node_ptr = (waypoint_node_t*)calloc(1, sizeof(waypoint_node_t));

												char tmpChar[3];
												tmpString.assign("REROUTE WAYPOINT ");

												idx_newly_created_waypoint++;
												sprintf(tmpChar, "%d", idx_newly_created_waypoint);

												tmpString.append(tmpChar);

												tmp_new_waypoint_node_ptr->wpname = (char*)calloc((tmpString.length()+1), sizeof(char));
												strcpy(tmp_new_waypoint_node_ptr->wpname, tmpString.c_str());
												tmp_new_waypoint_node_ptr->wpname[tmpString.length()] = '\0';

												tmp_new_waypoint_node_ptr->proctype = (char*)calloc(strlen("ENROUTE")+1, sizeof(char));
												strcpy(tmp_new_waypoint_node_ptr->proctype, "ENROUTE");
												tmp_new_waypoint_node_ptr->proctype[strlen("ENROUTE")] = '\0';
												tmp_new_waypoint_node_ptr->alt_1 = -10000.00;
												tmp_new_waypoint_node_ptr->alt_2 = -10000.00;

												tmp_new_waypoint_node_ptr->latitude = vector_reroute_waypoint_lat_deg.at(idx_vector_reroute);
												tmp_new_waypoint_node_ptr->longitude = vector_reroute_waypoint_lon_deg.at(idx_vector_reroute);

												if (tmpWaypointNode_ptr->next_node_ptr != NULL) {
													tmp_new_waypoint_node_ptr->next_node_ptr = tmpWaypointNode_ptr->next_node_ptr;
													tmp_new_waypoint_node_ptr->next_node_ptr->prev_node_ptr = tmp_new_waypoint_node_ptr;
												}

												tmpWaypointNode_ptr->next_node_ptr = tmp_new_waypoint_node_ptr;
												tmp_new_waypoint_node_ptr->prev_node_ptr = tmpWaypointNode_ptr;

												tmp_new_waypoint_node_ptr->prev_node_ptr->course_rad_to_next_node = compute_heading_rad_gc(
														tmp_new_waypoint_node_ptr->prev_node_ptr->latitude,
														tmp_new_waypoint_node_ptr->prev_node_ptr->longitude,
														tmp_new_waypoint_node_ptr->latitude,
														tmp_new_waypoint_node_ptr->longitude);

												tmpWaypointNode_ptr = tmp_new_waypoint_node_ptr;

												// Store first waypoint node pointer
												if (idx_vector_reroute == 1)
													postRG_WaypointNode_ptr = tmpWaypointNode_ptr;
											} else {
												tmpWaypointNode_ptr = tmpWaypointNode_ptr->next_node_ptr;
											}
										}
									}

									// Traverse the airborne waypoint node list(starting from the current target waypoint)
									// If the waypoint is not in the reroute path, delete it
									tmpWaypointNode_ptr = array_update_states_ptr[i]->target_WaypointNode_ptr;
									while (tmpWaypointNode_ptr != NULL) {
										waypoint_node_t* tmpPrevWaypointNode_ptr = tmpWaypointNode_ptr->prev_node_ptr;

										if (strcmp(tmpWaypointNode_ptr->proctype, "ENROUTE") == 0) {
											if (set_reroute_waypoint_pairs.find(pair<double, double>(tmpWaypointNode_ptr->latitude, tmpWaypointNode_ptr->longitude)) == set_reroute_waypoint_pairs.end()) {
												tmpWaypointNode_ptr->prev_node_ptr->next_node_ptr = tmpWaypointNode_ptr->next_node_ptr;
												tmpWaypointNode_ptr->next_node_ptr->prev_node_ptr = tmpWaypointNode_ptr->prev_node_ptr;

												// Delete this waypoint node
												releaseWaypointNodeContent(tmpWaypointNode_ptr);

												tmpWaypointNode_ptr = tmpPrevWaypointNode_ptr;
											}
										}

										tmpWaypointNode_ptr = tmpWaypointNode_ptr->next_node_ptr;
									}

									// Update the target waypoint
									array_update_states_ptr[i]->target_WaypointNode_ptr = postRG_WaypointNode_ptr;
									c_target_waypoint_node_ptr[i] = postRG_WaypointNode_ptr;
								}
							}
						} // end - for
					} // end - Strategic weather avoidance
				} // end - Exact hour

				// Ground Vehicle Simulation Logic Start
				int tmpCountWaypoint;
				float distanceCovered = 0.0, distanceBuffer, distanceDiff = 0.0, distanceToRadius, prevLat, prevLon, nextLat, nextLon;

				groundVehiclePreviousStates = groundVehicleStates;
				int i = 0, groundVehicleListSize = groundVehicleStates.size();
				while ( i < groundVehicleListSize) {
					// Externally injected ground vehicle logic
					if(groundVehicleStates.at(i).flag_external_groundVehicle)
						groundVehicleSimulationData[groundVehicleStates.at(i).vehicle_id].push_back(to_string(t) + "," + groundVehicleStates.at(i).aircraft_id + "," + to_string(groundVehicleStates.at(i).latitude * M_PI / 180.0) + "," + to_string(groundVehicleStates.at(i).longitude * M_PI / 180.0) + "," + to_string(groundVehicleStates.at(i).altitude) + "," + to_string(groundVehicleStates.at(i).speed) + "," + to_string(groundVehicleStates.at(i).course));
					else {

						//Get drive plan for current ground vehicle, starting at index 0
						waypoint_node_t* tmp_wp_ptr = groundVehicleStates.at(i).drive_plan_ptr;

						if (groundVehicleStates.at(i).target_waypoint_index < groundVehicleStates.at(i).drive_plan_length - 1) {

							// Check if ground operator is absent for current vehicle
							if (groundOperatorAbsence[groundVehicleStates.at(i).vehicle_id] > 0) {
								groundVehicleStates.at(i).speed = 0;
								groundOperatorAbsence[groundVehicleStates.at(i).vehicle_id]--;
								if (groundOperatorAbsence[groundVehicleStates.at(i).vehicle_id] == 0)
									groundVehicleStates.at(i).speed = g_groundVehicles.at(i).speed;
							}
							// Set lagged parameter values to vehicles if applicable
							int paramCount;
							for (paramCount = 0; paramCount < gvLagParams[groundVehicleStates.at(i).vehicle_id].size(); paramCount++) {
								if (gvLagParams[groundVehicleStates.at(i).vehicle_id][paramCount] == "COURSE") {
									if(gvLagParamValues[groundVehicleStates.at(i).vehicle_id]["COURSE"].size() == 0)
										gvLagParams[groundVehicleStates.at(i).vehicle_id].erase(gvLagParams[groundVehicleStates.at(i).vehicle_id].begin() + paramCount);
									else
										groundVehicleStates.at(i).course = gvLagParamValues[groundVehicleStates.at(i).vehicle_id]["COURSE"][0];
								}
								if (gvLagParams[groundVehicleStates.at(i).vehicle_id][paramCount] == "SPEED") {
									if(gvLagParamValues[groundVehicleStates.at(i).vehicle_id]["SPEED"].size() == 0)
										gvLagParams[groundVehicleStates.at(i).vehicle_id].erase(gvLagParams[groundVehicleStates.at(i).vehicle_id].begin() + paramCount);
									else
										groundVehicleStates.at(i).speed = gvLagParamValues[groundVehicleStates.at(i).vehicle_id]["SPEED"][0];
								}
							}

							if (groundVehicleStates.at(i).target_waypoint_index == -1) {
								// Set target waypoint as next node in drive plan
								groundVehicleStates.at(i).target_waypoint_name = tmp_wp_ptr->next_node_ptr->wpname;
								groundVehicleStates.at(i).target_waypoint_index = 1;
								prevLat = groundVehicleStates.at(i).latitude * M_PI / 180.0;
								prevLon = groundVehicleStates.at(i).longitude * M_PI / 180.0;
								nextLat = tmp_wp_ptr->next_node_ptr->latitude * M_PI / 180.0;
								nextLon = tmp_wp_ptr->next_node_ptr->longitude * M_PI / 180.0;
							}
							// Calculate distance (ft) covered in past time step
							distanceCovered = groundVehicleStates.at(i).speed * 1.68780986 * t_step_terminal;
							tmpCountWaypoint = 0;
							while(tmpCountWaypoint < groundVehicleStates.at(i).target_waypoint_index) {
								tmp_wp_ptr = tmp_wp_ptr->next_node_ptr;
								tmpCountWaypoint++;
							}

							prevLat = groundVehicleStates.at(i).latitude * M_PI / 180.0;
							prevLon = groundVehicleStates.at(i).longitude * M_PI / 180.0;
							nextLat = tmp_wp_ptr->latitude * M_PI / 180.0;
							nextLon = tmp_wp_ptr->longitude * M_PI / 180.0;

							distanceBuffer = compute_distance_gc(groundVehicleStates.at(i).latitude, groundVehicleStates.at(i).longitude, tmp_wp_ptr->latitude, tmp_wp_ptr->longitude, 0.0);
							distanceDiff = distanceCovered;

							while((distanceCovered > distanceBuffer)) {
								distanceBuffer += compute_distance_gc(tmp_wp_ptr->latitude, tmp_wp_ptr->longitude, tmp_wp_ptr->next_node_ptr->latitude, tmp_wp_ptr->next_node_ptr->longitude, 0.0);
								distanceDiff = distanceBuffer - distanceCovered;
								prevLat = tmp_wp_ptr->latitude * M_PI / 180.0;
								prevLon = tmp_wp_ptr->longitude * M_PI / 180.0;
								nextLat = tmp_wp_ptr->next_node_ptr->latitude * M_PI / 180.0;
								nextLon = tmp_wp_ptr->next_node_ptr->longitude * M_PI / 180.0;
								tmp_wp_ptr = tmp_wp_ptr->next_node_ptr;
								tmpCountWaypoint++;
								if(tmpCountWaypoint == groundVehicleStates.at(i).drive_plan_length - 1)
									break;

							}

							distanceToRadius = ((distanceDiff * 0.0003048) / (6367.0211));
							groundVehicleSimulationData[groundVehicleStates.at(i).vehicle_id].push_back(to_string(t) + "," + groundVehicleStates.at(i).aircraft_id + "," + to_string(groundVehicleStates.at(i).latitude) + "," + to_string(groundVehicleStates.at(i).longitude) + "," + to_string(groundVehicleStates.at(i).altitude) + "," + to_string(groundVehicleStates.at(i).speed) + "," + to_string(groundVehicleStates.at(i).course));

							groundVehicleStates.at(i).target_waypoint_index = tmpCountWaypoint;
							groundVehicleStates.at(i).target_waypoint_name = tmp_wp_ptr->wpname;
							if (!std::count(courseChange.begin(), courseChange.end(), groundVehicleStates.at(i).vehicle_id))
								groundVehicleStates.at(i).course = calculateBearing(prevLat, prevLon, nextLat, nextLon) * M_PI / 180.0;
							else {
								for (auto it = courseChange.begin(); it != courseChange.end();)
										if (*it == groundVehicleStates.at(i).vehicle_id)
											it = courseChange.erase(it);
										else
										   ++it;
							}
							groundVehicleStates.at(i).latitude = (asin(sin(prevLat) * cos(distanceToRadius) + cos(prevLat) * sin(distanceToRadius) * cos(groundVehicleStates.at(i).course))) * 180.0 / M_PI;
							groundVehicleStates.at(i).longitude = (prevLon + atan2(sin(groundVehicleStates.at(i).course) * sin(distanceToRadius) * cos(prevLat), cos(distanceToRadius) - sin(prevLat) * sin(groundVehicleStates.at(i).latitude * M_PI / 180.0))) * 180.0 / M_PI;
							if(groundVehicleStates.at(i).target_waypoint_index == groundVehicleStates.at(i).drive_plan_length - 1) {
								groundVehicleStates.at(i).speed = 0;
								groundVehicleStates.at(i).latitude = nextLat * 180.0 / M_PI;
								groundVehicleStates.at(i).longitude = nextLon * 180.0 / M_PI;
								groundVehicleSimulationData[groundVehicleStates.at(i).vehicle_id].push_back(to_string(t + t_step_terminal) + "," + groundVehicleStates.at(i).aircraft_id + "," + to_string(groundVehicleStates.at(i).latitude) + "," + to_string(groundVehicleStates.at(i).longitude) + "," + to_string(groundVehicleStates.at(i).altitude) + "," + to_string(groundVehicleStates.at(i).speed) + "," + to_string(groundVehicleStates.at(i).course));
							}

						}
					}
					groundVehicleListSize = groundVehicleStates.size();
					i++;
				}
				// Ground Vehicle Simulation Logic End

				for (int i = 0; i < num_flights; i++) {
					// If this aircraft is not freezed
					if ((array_update_states_ptr[i] != NULL) && (!array_update_states_ptr[i]->flag_aircraft_held_strategic) && (!array_update_states_ptr[i]->flag_aircraft_held_tactical) && (!array_update_states_ptr[i]->flag_aircraft_spacing)) {
						vector<string>::iterator it = lagParams[i].begin();
						if (std::find(lagParams[i].begin(), lagParams[i].end(), "COURSE") != lagParams[i].end()) {
							h_aircraft_soa.course_rad[i] = lagParamValues[i]["COURSE"].front() * PI/180.;
							d_aircraft_soa.course_rad[i] = lagParamValues[i]["COURSE"].front() * PI/180.;
							if (lagParamValues[i]["COURSE"].size() > 0)
								lagParamValues[i]["COURSE"].erase(lagParamValues[i]["COURSE"].begin());
							else {
								while (it != lagParams[i].end())
								{
									if (lagParams[i][0] == "COURSE")
									{
										it = lagParams[i].erase(it);
									}
									else
									{
										++it;
									}
								}
							}
						}
						else if (controllerAway > 0) {
							d_aircraft_soa.course_rad[i] = defaultCourse;
							h_aircraft_soa.course_rad[i] = d_aircraft_soa.course_rad[i];
						}
						else {
							h_aircraft_soa.course_rad[i] = d_aircraft_soa.course_rad[i];
						}

						if (std::find(lagParams[i].begin(), lagParams[i].end(), "AIRSPEED") != lagParams[i].end()) {
							h_aircraft_soa.tas_knots[i] = lagParamValues[i]["AIRSPEED"].front();
							d_aircraft_soa.tas_knots[i] = lagParamValues[i]["AIRSPEED"].front();
							if (lagParamValues[i]["AIRSPEED"].size() > 0)
								lagParamValues[i]["AIRSPEED"].erase(lagParamValues[i]["AIRSPEED"].begin());
							else {
								while (it != lagParams[i].end())
								{
									if (lagParams[i][0] == "AIRSPEED")
									{
										it = lagParams[i].erase(it);
									}
									else
									{
										++it;
									}
								}
							}
						}
						else if (controllerAway > 0) {
							d_aircraft_soa.tas_knots[i] = defaultSpeed;
							h_aircraft_soa.tas_knots[i] = d_aircraft_soa.tas_knots[i];
						}
						else {
							h_aircraft_soa.tas_knots[i] = d_aircraft_soa.tas_knots[i];
						}

						if (std::find(lagParams[i].begin(), lagParams[i].end(), "VERTICAL_SPEED") != lagParams[i].end()) {
							h_aircraft_soa.rocd_fps[i] = lagParamValues[i]["VERTICAL_SPEED"].front();
							d_aircraft_soa.rocd_fps[i] = lagParamValues[i]["VERTICAL_SPEED"].front();
							if (lagParamValues[i]["VERTICAL_SPEED"].size() > 0)
								lagParamValues[i]["VERTICAL_SPEED"].erase(lagParamValues[i]["VERTICAL_SPEED"].begin());
							else {
								while (it != lagParams[i].end())
								{
									if (lagParams[i][0] == "VERTICAL_SPEED")
									{
										it = lagParams[i].erase(it);
									}
									else
									{
										++it;
									}
								}
							}
						}
						else if (controllerAway > 0) {
							d_aircraft_soa.rocd_fps[i] = defaultRocd;
							h_aircraft_soa.rocd_fps[i] = d_aircraft_soa.rocd_fps[i];
						}
						else {
							h_aircraft_soa.rocd_fps[i] = d_aircraft_soa.rocd_fps[i];
						}
					}
				}

				if (controllerAway > 0) {
					controllerAway--;
				}

				// Aircraft data may be modified during the pause status by users
				// We have to synchronize it again.
				set_device_ac_pointers();

				// Process stage 2 calculation on all aircraft
				for (int i = 0; i < NUM_STREAMS; ++i) {
					launch_kernel_stage2(grid_size, block_size, 0, streams[i], num_flights, t, t_step, t_step_terminal, t_data_collection_period_airborne, i, flag_proc_airborne_trajectory);
				}












				// Risk Measures
				if (flag_exist_ac_risk_measures_data) {
printf("\nRisk Measures starting =================================\n");
					for (int i = 0; i < num_flights; i++) {
						update_states_t* update_states_ptr_i;

						if (array_update_states_ptr[i] != NULL) {
							update_states_ptr_i = array_update_states_ptr[i];

						for (int k = 0; k < ac_risk_measures_data[i].flight_phase_risk[update_states_ptr_i->flight_phase]->record_count; k++) {
							if (ac_risk_measures_data[i].flight_phase_risk[update_states_ptr_i->flight_phase]->codes[k] != NULL) {
								// Find a list of aircraft which is inside the distance of regards

								// Iterate and calculate risk value between the current aircraft and the testing aircraft
								// for (every aircraft from the qualified aircraft list) {

								int ac_idx_toCompare = 2; // Temp aircraft ID for Testing

								if (i != ac_idx_toCompare) {
									// Call corresponding functions
									switch (update_states_ptr_i->flight_phase) {
										case FLIGHT_PHASE_ORIGIN_GATE:
											// Call risk measures functions here

											break;

										case FLIGHT_PHASE_PUSHBACK:

											break;

										case FLIGHT_PHASE_RAMP_DEPARTING:

											break;

										case FLIGHT_PHASE_TAXI_DEPARTING:

											break;

										case FLIGHT_PHASE_RUNWAY_THRESHOLD_DEPARTING:

											break;

										case FLIGHT_PHASE_TAKEOFF:

											break;

										case FLIGHT_PHASE_CLIMBOUT:

											break;

										case FLIGHT_PHASE_HOLD_IN_DEPARTURE_PATTERN:

											break;

										case FLIGHT_PHASE_CLIMB_TO_CRUISE_ALTITUDE:

											break;

										case FLIGHT_PHASE_TOP_OF_CLIMB:

											break;

										case FLIGHT_PHASE_CRUISE:

											break;

										case FLIGHT_PHASE_HOLD_IN_ENROUTE_PATTERN:

											break;

										case FLIGHT_PHASE_TOP_OF_DESCENT:

											break;

										case FLIGHT_PHASE_INITIAL_DESCENT:

											break;

										case FLIGHT_PHASE_HOLD_IN_ARRIVAL_PATTERN:

											break;

										case FLIGHT_PHASE_APPROACH:

											break;

										case FLIGHT_PHASE_FINAL_APPROACH:

											break;

										case FLIGHT_PHASE_GO_AROUND:

											break;

										case FLIGHT_PHASE_TOUCHDOWN:

											break;

										case FLIGHT_PHASE_LAND:

											break;

										case FLIGHT_PHASE_EXIT_RUNWAY:

											break;

										case FLIGHT_PHASE_TAXI_ARRIVING:

											break;

										case FLIGHT_PHASE_RUNWAY_CROSSING:

											break;

										case FLIGHT_PHASE_RAMP_ARRIVING:

											break;

										case FLIGHT_PHASE_DESTINATION_GATE:

											break;

										case FLIGHT_PHASE_USER_INCIDENT:
											break;

										default:

											break;

									} // end - switch
								}
								//}
							}
						}

						}
					}
				} // end - Risk Measures





















				// Conflict Detection and Resolution
				if (flag_enable_cdnr) {
					for (int i = 0; i < num_flights; i++) {
						if (array_update_states_ptr[i] != NULL) {
							// If this aircraft is CDNR held
							if (0 < array_update_states_ptr[i]->duration_held_cdnr) {
								array_update_states_ptr[i]->duration_held_cdnr = array_update_states_ptr[i]->duration_held_cdnr - t_step;

								if (array_update_states_ptr[i]->duration_held_cdnr == 0) {
									// Restore speed
									array_update_states_ptr[i]->tas_knots = array_update_states_ptr[i]->tas_knots_before_held_cdnr;

									map_CDR_status.erase(string(array_update_states_ptr[i]->acid));
								}
							}
						}
					}

					if (num_flights > 1) {
						update_states_t* update_states_ptr_i;
						update_states_t* update_states_ptr_j;

						for (int i = 1; i < num_flights; i++) {
							update_states_ptr_i = array_update_states_ptr[i];

							if (update_states_ptr_i != NULL) {
								for (int j = 0; j < i; j++) {
									update_states_ptr_j = array_update_states_ptr[j];

									if ((update_states_ptr_i != NULL) && (update_states_ptr_j != NULL)) {
										float tmp_distance_of_regard;
										float tmp_distance_of_resolve;

										if ((isFlightPhase_in_ground_departing(update_states_ptr_i->flight_phase)) || (isFlightPhase_in_ground_landing(update_states_ptr_i->flight_phase))) {
											tmp_distance_of_regard = cdr_initiation_distance_ft_surface;
											tmp_distance_of_resolve = cdr_separation_distance_ft_surface;
										} else if (isFlightPhase_in_airborne(update_states_ptr_i->flight_phase)) {
											tmp_distance_of_regard = cdr_initiation_distance_ft_enroute;
											tmp_distance_of_resolve = cdr_separation_distance_ft_enroute;
										} else {
											tmp_distance_of_regard = cdr_initiation_distance_ft_terminal;
											tmp_distance_of_resolve = cdr_separation_distance_ft_terminal;
										}

										int delay_ac = 0;
										int delay_step = 0;

										if ((update_states_ptr_i->duration_held_cdnr == 0) && (update_states_ptr_j->duration_held_cdnr == 0)) {
											ConflictDetectionAndResolution(t,
													update_states_ptr_i,
													update_states_ptr_i->lat * PI/180.,
													update_states_ptr_i->lon * PI/180.,
													update_states_ptr_i->altitude_ft,
													update_states_ptr_i->V_ground,
													update_states_ptr_i->fpa_rad,
													update_states_ptr_i->hdg_rad,
													update_states_ptr_i->time_to_conflict,
													update_states_ptr_j,
													update_states_ptr_j->lat * PI/180.,
													update_states_ptr_j->lon * PI/180.,
													update_states_ptr_j->altitude_ft,
													update_states_ptr_j->V_ground,
													update_states_ptr_j->fpa_rad,
													update_states_ptr_j->hdg_rad,
													update_states_ptr_j->time_to_conflict,
													RADIUS_EARTH_FT,
													t_step,
													tmp_distance_of_regard,
													tmp_distance_of_resolve,
													delay_ac,
													delay_step
													);

											if (delay_ac == 1) {
												update_states_ptr_i->duration_held_cdnr = 0; // Reset

												if (0 < delay_step) {
													update_states_ptr_i->duration_held_cdnr = t_step * delay_step;

													pair<string, float> tmpPair;
													tmpPair.first = string(update_states_ptr_j->acid);
													tmpPair.second = update_states_ptr_i->duration_held_cdnr;
													map_CDR_status.insert(pair<string, pair<string, float>>(string(update_states_ptr_i->acid), tmpPair));
												}
											} else if (delay_ac == 2) {
												update_states_ptr_j->duration_held_cdnr = 0; // Reset

												if (0 < delay_step) {
													update_states_ptr_j->duration_held_cdnr = t_step * delay_step;

													pair<string, float> tmpPair;
													tmpPair.first = string(update_states_ptr_i->acid);
													tmpPair.second = update_states_ptr_j->duration_held_cdnr;
													map_CDR_status.insert(pair<string, pair<string, float>>(string(update_states_ptr_j->acid), tmpPair));
												}
											}
										}
									}
								} // end - for
							}
						}
					}
				} // end - Conflict Detection and Resolution

				// Write data from update_states to c_ data variables
				for (int i = 0; i < num_flights; i++) {
					if (array_update_states_ptr[i] != NULL) {
						kernel_update_states_write_data_from_updateStates_to_c(array_update_states_ptr[i], i);
					}
				}

				if (flag_realTime_simulation) {
					long tmpUTC = getCurrentCpuTime_milliSec();
					nextPropagation_utc_time_realTime_simulation = tmpUTC + long(sleep_duration_realTime_simulation / 1000);

					usleep(sleep_duration_realTime_simulation);
				}

				t += t_step;
			} // end - while loop

			printf("\nFlight propagation completed.\n");

			// Write statistics of CDNR
			if (flag_enable_cdnr) {
				stringstream tmpOSS;
				tmpOSS << "log/cdnr_";
				tmpOSS << getCurrentCpuTime_milliSec();
				tmpOSS << ".log";

				if (cnt_event_cdnr == 0) {
					cdnr_oss_doc << "No aircraft conflicting events.\n";
				} else {
					cdnr_oss_doc << "\nNumber of aircraft conflicting and resolved events: " << cnt_event_cdnr << "\n";
				}

				write_file(tmpOSS.str(), cdnr_oss_doc);
			}

			// Propagation finished.  Quit while loop
			break;
		} // end - if

		usleep(nats_simulation_check_interval);
	} // end - while

	sim_id = 0; // Reset

	for (int i = 0; i < NUM_STREAMS; ++i) {
		cuda_stream_synchronize(streams[i]);
		cuda_stream_destroy(streams[i]);
	}

	// Set value to g_trajectories.interval field
	for (int i = 0; i < num_flights; i++) {
		g_trajectories.at(i).interval_airborne = t_data_collection_period_airborne;
	}

#if ENABLE_PROFILER
	cudaProfilerStop();
#endif

#if ENABLE_DEBUG_TRAJ_FILE
	// debug print the outputs
	{
		string fname = "dbg_traj.out";
		FILE* out = fopen(fname.c_str(), "w");
		int fid = 0; //5000;
		int n = g_trajectories.at(fid).latitude_deg.size();

		for (int j = 0; j < num_flights; j++) {
			fprintf(out, "\n%s\n", g_trajectories.at(j).callsign.c_str());
		}

		fclose(out);
	}
#endif

	free(streams);
	streams = NULL;

	if (ground_departing_data_init != NULL) {
		free(ground_departing_data_init);
		ground_departing_data_init = NULL;
	}
	if (ground_landing_data_init != NULL) {
		free(ground_landing_data_init);
		ground_landing_data_init = NULL;
	}

	if (array_update_states_ptr != NULL) {
		for (int i = 0; i < num_flights; i++) {
			if (array_update_states_ptr[i] != NULL) {
				cleanUp_update_states(array_update_states_ptr[i]);
			}
		}

		free(array_update_states_ptr);

		array_update_states_ptr = NULL;
	}

	// Debug: Print the aircrafts that haven't landed
	for (int i = 0; i < num_flights; i++) {
		if (c_flight_phase[i] != FLIGHT_PHASE_LANDED) {
logger_printLog(LOG_LEVEL_DEBUG, 2, "Flight not landed --> index_flight = %d, acid = %s\n", i, g_trajectories.at(i).callsign.c_str());
		}
	}

	nats_simulation_status = NATS_SIMULATION_STATUS_ENDED;

	if (flag_enable_strategic_weather_avoidance) {
		release_rg_resources();
	}

	flag_enable_strategic_weather_avoidance = false; // Reset
	flag_realTime_simulation = false; // Reset
	flag_exist_ac_risk_measures_data = false; // Reset

	// Detach the current thread
	pthread_detach(pthread_self());
}
