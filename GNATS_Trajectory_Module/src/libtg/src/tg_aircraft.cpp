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
 * tg_aircraft.cu
 *
 *  Created on: Sep 19, 2013
 *      Author: jason
 */

#include "AirportLayoutDataLoader.h"
#include "tg_aircraft.h"
#include "tg_trajectory.h"
#include "tg_pars.h"
#include "tg_sidstars.h"
#include "tg_airways.h"
#include "tg_airports.h"
#include "tg_waypoints.h"
#include "tg_adb.h"
#include "tg_flightplan.h"
#include "tg_api.h"
#include "tg_Point.h"

#include "TrxInputStream.h"
#include "TrxInputStreamListener.h"
#include "TrxRecord.h"
#include "FlightPlanParser.h"
#include "FlightPlan.h"

#include "geometry_utils.h"
#include "Controller.h"
#include "ControllerError.h"
#include "cuda_compat.h"

#include "Pilot.h"

#include "json.hpp"

#include "util_string.h"

#include <algorithm>
#include <vector>
#include <deque>
#include <map>
#include <string>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <iomanip>

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_LEN 10000


#define ULIDEMO 1

#ifdef _INC__MINGW_H
#include <fcntl.h>
#define pipe(fds) _pipe(fds, 5000, _O_BINARY)
#endif

using namespace std;
using namespace osi;
using json = nlohmann::json;

char buffer_stdout[MAX_LEN+1] = {0};


/*
 * Globals declared extern in tg_aircraft.h
 */
aircraft_t h_aircraft_soa;
aircraft_t d_aircraft_soa;

NewMU_Aircraft newMU_external_aircraft;

/**
 * Airborne flight plan data structure
 * The flight plan is built by linked list of waypoint nodes
 * 
 * There is only one linked list for airborne flight plan on every aircraft
 * To aid the program logic, we use following variables to mark certain waypoint nodes
 * 
 * We organize all aircraft pointer data and make it in array structure.
 */
waypoint_node_t** array_Airborne_Flight_Plan_ptr; // Pointer refering to the first waypoint node of the airborne flight plan
waypoint_node_t** array_Airborne_Flight_Plan_toc_ptr; // Pointer refering to the Top-of-climb waypoint node of the airborne flight plan
waypoint_node_t** array_Airborne_Flight_Plan_tod_ptr; // Pointer refering to the Top-of-descent waypoint node of the airborne flight plan
waypoint_node_t** array_Airborne_Flight_Plan_Final_Node_ptr; // Pointer refering to the final waypoint node of the airborne flight plan

int* array_Airborne_Flight_Plan_Waypoint_length;

// Map of aircraft id and flight sequence index.
map<string, int> map_Acid_FlightSeq;

int external_aircraft_seq;
const char* EXTERNAL_AIRCRAFT_ID = "EXT_";

map<string, string> map_aircraft_owner;

map<int, osi::TrxRecord> g_trx_records;
vector<Trajectory> g_trajectories;

static int num_flights = 0;
static int ignore_count = 0;
static int filter_count = 0;

static const real_t MIN_ALTITUDE = 10000;


class TrxHandler : public TrxInputStreamListener {
public:
	TrxHandler() :
		startTime(-1),
		currentTrackTime(0),
		records(vector<TrxRecord>()) {
	}

	virtual ~TrxHandler() {
	}

	void onTrackTime(long trackTime) {
		// update the current track time and insert a new empty
		// vector into the map of records at the current track time key
		if (startTime < 0) startTime = trackTime;
		currentTrackTime = trackTime;
	}

	void onTrack(const TrxRecord& trackRecord) {
		// filter the flights by cruise altitude, only accept flights
		// whose altitude is above 10000 ft.
		if (trackRecord.cruiseAltitude >= MIN_ALTITUDE) {
			records.push_back(trackRecord);
		} else {
			if(g_verbose) {
				printf("  WARNING: Ignoring general aviation flight"
						" %s (%s).\n",
						trackRecord.acid.c_str(), trackRecord.actype.c_str());
			}
			filter_count++;
		}
	}

	vector<TrxRecord>& getRecords() {
		return records;
	}

	int getTotalFlightCount() {
		return records.size();
	}

	long getStartTime() {
		return startTime;
	}

private:
	long startTime;
	long currentTrackTime;
	vector<TrxRecord> records;
};

real_t get_airport_elevation(const string& airport_code) {
	NatsAirport airport;
	airport.code = (airport_code.length() == 3 ?
			"K"+airport_code : airport_code);
	vector<NatsAirport>::iterator it = lower_bound(g_airports.begin(),
			g_airports.end(), airport);
	if (it == g_airports.end()) {
		airport.code = (airport_code.length() == 3 ?
				"P"+airport_code : airport_code);
		it = lower_bound(g_airports.begin(), g_airports.end(), airport);
		if (it == g_airports.end()) {
			if (g_verbose) {
				printf("  WARNING: airport %s not found. "
						"using airport elevation of 0.\n",
				    	airport_code.c_str());
			}

			return 0;
		}
	}

	return it->elevation;
}

static string remove_white_spaces(const string inp_str){
	string retString;

	char* str = new char[inp_str.length() + 1];
	strcpy(str, inp_str.c_str());
	
	// To keep track of non-space character count
	int count = 0;
	
	// Traverse the given string. If current character
	    // is not space, then place it at index 'count++'
	for (int i = 0; str[i]; i++)
		if (str[i] != ' ')
			str[count++] = str[i]; // here count is
	                                   // incremented
	str[count] = '\0';

	retString.assign(str);

	delete[] str;

	return retString;
}

static real_t get_airport_lat_lon(const string& airport_code,
		const bool& islat = true){

	NatsAirport airport;
	airport.code = (airport_code.length() == 3 ?
			"K"+airport_code : airport_code);
	vector<NatsAirport>::iterator it = lower_bound(g_airports.begin(),
			g_airports.end(), airport);
	if (it == g_airports.end()) {
		if (g_verbose) {
			printf("  WARNING: airport %s not found. "
					"using airport elevation of 0.\n",
			    	airport_code.c_str());
		}
		return 0;
	}
	if (islat)
		return it->latitude;
	else
		return it->longitude;
}

//TODO:ALSO IN TG_SIMULATION
//LAT LON TO NED
static inline real_t RN(real_t lat){
	return SEMI_MAJOR_EARTH_FT/sqrt(1-ECCENTRICITY_SQ_EARTH*sin(lat*M_PI/180)*sin(lat*M_PI/180) );
}

static inline real_t X_ref(real_t* const lat,real_t* const lon){
	return RN( *lat ) * cos( *lat * M_PI/180) * cos( *lon * M_PI/180);}
static inline real_t Y_ref(real_t* const lat,real_t* const lon){
	return RN( *lat ) * cos( *lat * M_PI/180) * sin( *lon * M_PI/180);}
static inline real_t Z_ref(real_t* const lat){
	return ( ( 1 - ECCENTRICITY_SQ_EARTH ) * RN( *lat ) ) * sin( *lat * M_PI/180);}

static void ECEFtoNED(real_t* const X, real_t* const Y, real_t* const Z, real_t* const ref_lat,
		real_t* const ref_lon, real_t* x, real_t* y, real_t* z){

	*x = - (*X - X_ref(ref_lat,ref_lon)) * sin( *ref_lat * M_PI/180) * cos( *ref_lon * M_PI/180)
		 - (*Y - Y_ref(ref_lat,ref_lon)) * sin( *ref_lat * M_PI/180) * sin( *ref_lon * M_PI/180)
		 + (*Z - Z_ref(ref_lat)) * cos( *ref_lat * M_PI/180);

	*y = - (*X - X_ref(ref_lat,ref_lon)) * sin( *ref_lon * M_PI/180)
		 + (*Y - Y_ref(ref_lat,ref_lon)) * cos( *ref_lon * M_PI/180);

	*z = - (*X - X_ref(ref_lat,ref_lon)) * cos( *ref_lat * M_PI/180) * cos( *ref_lon * M_PI/180)
		 - (*Y - Y_ref(ref_lat,ref_lon)) * cos( *ref_lat * M_PI/180) * sin( *ref_lon * M_PI/180)
		 - (*Z - Z_ref(ref_lat)) * sin( *ref_lat * M_PI/180);
}

static void NEDtoECEF(real_t* const x, real_t* const y, real_t* const z,real_t* const ref_lat,
		real_t* const ref_lon, real_t* X, real_t* Y, real_t* Z){

	*X = - (*x) * sin( *ref_lat * M_PI/180) * cos( *ref_lon * M_PI/180)
		 - (*y) * sin( *ref_lon * M_PI/180)
		 - (*z) * cos( *ref_lat * M_PI/180) * cos( *ref_lon * M_PI/180) + X_ref(ref_lat,ref_lon);

	*Y = - (*x) * sin( *ref_lat * M_PI/180) * sin( *ref_lon * M_PI/180)
		 + (*y) * cos( *ref_lon * M_PI/180)
		 - (*z) * cos( *ref_lat * M_PI/180) * sin( *ref_lon * M_PI/180) + Y_ref(ref_lat,ref_lon);

	*Z =  (*x) * cos( *ref_lat * M_PI/180) - (*z) * sin( *ref_lat * M_PI/180) + Z_ref(ref_lat);
}

static void LatLontoNED(real_t* const lat, real_t* const lon, real_t* const alt,real_t* const ref_lat,
		real_t* const ref_lon,	real_t* x, real_t* y, real_t* z){
	real_t Rn = RN( (*lat) ) + *alt;
	real_t X = Rn * cos((*lat) * M_PI/180) * cos( (*lon) * M_PI/180);
	real_t Y = Rn * cos((*lat) * M_PI/180) * sin( (*lon) * M_PI/180);
	real_t Z = ( ( 1 - ECCENTRICITY_SQ_EARTH ) * Rn )*sin( (*lat) * M_PI/180);
	ECEFtoNED(&X,&Y,&Z,ref_lat,ref_lon,x,y,z);
}

static void NEDtoLatLon(real_t* const x, real_t* const y, real_t* const z,real_t* const ref_lat,
		real_t* const ref_lon,	real_t* lat, real_t* lon, real_t* h){
	real_t X=0,Y=0,Z=0;
	NEDtoECEF(x,y,z,ref_lat,ref_lon,&X,&Y,&Z);


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


/*SOLVE FOR INTERSECTION OF SPHERE AND LINE USING RAY TRACING
TAKEN FROM https://en.wikipedia.org/wiki/Line%E2%80%93sphere_intersection
ADOPTED FROM: David H. Eberly (2006), 3D game engine design: a practical approach
to real-time computer graphics, 2nd edition,
Morgan Kaufmann. ISBN 0-12-229063-1*/

static void get_dme_distance_point(const AdbPTFModel* const model,
		real_t* const course_deg, real_t* const ini_alt,
		real_t* const lat1, real_t* const lon1,
		real_t* const navlat, real_t* const navlon,
		real_t* const dist_nmi,
		real_t* lat_int, real_t* lon_int, real_t* h_int){
	real_t roc_fps = (real_t)model->getClimbRate((double)(*ini_alt),NOMINAL)/60.0;
	real_t tas_knots = (real_t)model->getClimbTas(*ini_alt);

	real_t fpa_rad = tas_knots == 0 ? 0 : asin(roc_fps / (tas_knots*KNOTS_TO_FPS));

	real_t ref_lat = *lat1;
	real_t ref_lon = *lon1;

	double dist_ft = (double)(*dist_nmi)*NauticalMilestoFeet;
	double del_rad = dist_ft/RADIUS_EARTH_FT;

	//latitude
	double term1 = sin((double)(*lat1)*M_PI/180)*cos(del_rad);
	double term2 = cos((double)(*lat1)*M_PI/180)*sin(del_rad)*cos((double)(*course_deg)*M_PI/180);
	real_t lat2 = asin(term1+term2)*180/M_PI;

	//longitude
	term1 = sin((double)(*course_deg)*M_PI/180)* sin(del_rad)* cos((double)(*lat1)*M_PI/180);
	term2 = cos(del_rad)-sin((double)(*lat1)*M_PI/180)*sin(lat2*M_PI/180);
	real_t lon2 = *lon1+atan2(term1,term2)*180/M_PI;
	real_t alt_2 = *ini_alt + dist_ft*tan(fpa_rad);

	//equation of line from two points
	real_t x1=0,y1=0,z1=0;
	LatLontoNED(lat1,lon1,ini_alt,&ref_lat,&ref_lon,&x1,&y1,&z1);

	real_t x2=0,y2=0,z2=0;

	LatLontoNED( &lat2,&lon2,&alt_2,&ref_lat,&ref_lon,&x2,&y2,&z2);

	//center of a circle
	real_t xc=0,yc=0,zc=0;
	LatLontoNED(navlat,navlon,ini_alt,&ref_lat,&ref_lon,&xc,&yc,&zc);

	//direction cosine of the line
	real_t norm_l = sqrt( (x2-x1)*(x2-x1) + (y2-y1)*(y2-y1) + (z2-z1)*(z2-z1) );
	real_t lx = (x2-x1)/norm_l,ly = (y2-y1)/norm_l, lz = (z2-z1)/norm_l;

	//vector origin of the line - center of sphere
	real_t ocx = x1-xc, ocy = y1-yc, ocz = z1-zc;

	//l.(o-c)
	real_t dot_l_oc = lx*ocx + ly*ocy + lz*ocz;

	//(o-c)^2-r^2
	real_t norm_oc = sqrt(ocx*ocx+ocy*ocy+ocz*ocz);
	real_t oc2mr2 = norm_oc*norm_oc-dist_ft*dist_ft;

	//(l.(o-c))^2-(o-c)^2-r^2
	real_t det_val = dot_l_oc*dot_l_oc-oc2mr2;

	if (det_val < 0){
		*lat_int=-9999; *lon_int=-9999; *h_int=-9999;
		return;
	}
	real_t dval_1 = -dot_l_oc+sqrt(det_val);
	real_t dval_2 = -dot_l_oc-sqrt(det_val);

	real_t d_int_from_origin = dval_1;

	if (fabs(dval_1) > fabs( dval_2) ){
		d_int_from_origin = dval_2;
	}

	real_t x_int = x1+lx*d_int_from_origin,y_int = y1+ly*d_int_from_origin,z_int =z1+lz*d_int_from_origin;

	NEDtoLatLon(&x_int, &y_int, &z_int, &ref_lat,
				&ref_lon, lat_int, lon_int, h_int);
}

static void get_intercept_point(real_t* const lat1, real_t* const lon1, real_t* const alt1,
							real_t* const course1_deg,
							real_t* const lat2, real_t* const lon2, real_t* const alt2,
							real_t* const course2_deg,
							real_t* lat_intercept, real_t* lon_intercept, real_t* h){

	real_t ref_lat = *lat1;
	real_t ref_lon = *lon1;


	real_t x1=0,y1=0,z1=0;
	LatLontoNED(lat1,lon1,alt1,&ref_lat,&ref_lon,&x1,&y1,&z1);

	real_t x2=0,y2=0,z2=0;
	LatLontoNED(lat2,lon2,alt2,&ref_lat,&ref_lon,&x2,&y2,&z2);

	real_t m1 = tan( (*course1_deg * M_PI/180.0) );
	real_t m2 = tan( (*course2_deg * M_PI/180.0));

	real_t den = (m1-m2);
	real_t x_intercept = ( (y2-m2*x2) - (y1-m1*x1) ) / den;
	real_t y_intercept = ( m1*(y2-m2*x2) - m2*(y1-m1*x1) ) / den;
	real_t z_intercept = (z1+z2)/2; //JUST A PLACE HOLDER PUT 3D LATER

	NEDtoLatLon(&x_intercept, &y_intercept, &z_intercept, &ref_lat,
				&ref_lon,	lat_intercept, lon_intercept, h);
}

//FIXME: HIGHLY APPROXIMATED ADD ACTUAL DYNAMICS HERE.NEEDS MORE WORK
static void get_termination_pt(const AdbPTFModel* const model, real_t* const ini_alt,
		real_t* const lat1, real_t* const lon1, real_t* const alt1,
		real_t* const course1_deg,real_t* const speed1_kts,
		real_t* lat_f, real_t* lon_f, real_t* h){

#if ULIDEMO
	real_t frac_red = 3.0;
#else
	real_t frac_red = 1.0;
#endif

	if ( fabs(*speed1_kts - 0) <1.0e-3){
		*speed1_kts = (real_t)model->getClimbTas(*alt1);
	}


	real_t roc_fps = (real_t)model->getClimbRate((double)(*alt1),NOMINAL)/60.0/frac_red;

	real_t fpa_rad = *speed1_kts == 0 ? 0 : asin(roc_fps / ((*speed1_kts)*KNOTS_TO_FPS));

	real_t final_alt = *alt1;

	if (*ini_alt > *alt1) *ini_alt = 0;

	real_t dt_sec = (final_alt-*ini_alt)/roc_fps ;

	real_t cos_hdg = cos(*course1_deg*M_PI/180);
	real_t sin_hdg = sin(*course1_deg*M_PI/180);

	real_t update = *speed1_kts * KNOTS_TO_FPS * cos(fpa_rad);
	real_t lat_dot = (update * cos_hdg) / (RADIUS_EARTH_FT + final_alt);
	real_t lon_dot = (update * sin_hdg) / ((RADIUS_EARTH_FT + final_alt) *
				cos(*lat1*M_PI/180.));

	*lat_f = *lat1 + dt_sec * lat_dot * 180./M_PI;
	*lon_f = *lon1 + dt_sec * lon_dot * 180./M_PI;
	*h = *alt1;
}

static waypoint_node_t* getTarget_waypoint_node(real_t lat_deg, real_t lon_deg, waypoint_node_t* waypoint_node_ptr) {
	waypoint_node_t* ret_ptr = NULL;

	real_t theta1, theta2;

	waypoint_node_t* tmp_wp_ptr = waypoint_node_ptr;
	waypoint_node_t* tmp_final_wp_ptr = NULL;

	if (tmp_wp_ptr == NULL) {
		return NULL;
	}

	double dist_to_first_waypoint = compute_distance_gc(lat_deg, lon_deg, tmp_wp_ptr->latitude, tmp_wp_ptr->longitude, 0, RADIUS_EARTH_FT);
	double dist_to_final_waypoint = 0;
	// If the distance to the first waypoint is less than 100 feet, we assume the current location is at or near the first waypoint.
	if (dist_to_first_waypoint <= 100) { // 100 ft
		ret_ptr = tmp_wp_ptr->next_node_ptr; // Set target waypoint to the next node

		return ret_ptr;
	}

	tmp_wp_ptr = tmp_wp_ptr->next_node_ptr; // Start from the 2nd node

	real_t fplat;
	real_t fplon;
	real_t fplatnext;
	real_t fplonnext;

	real_t tmpAngle_difference = 0;
	real_t max_angle_difference = 0;
	waypoint_node_t* waypointNodePtr_max_angle_difference = NULL;

	while (tmp_wp_ptr != NULL) {
		if (tmp_wp_ptr->next_node_ptr == NULL) {
			break;
		} else {
			fplat = tmp_wp_ptr->latitude;
			fplon = tmp_wp_ptr->longitude;
			fplatnext = tmp_wp_ptr->next_node_ptr->latitude;
			fplonnext = tmp_wp_ptr->next_node_ptr->longitude;
		}

		theta1 = compute_heading_rad_gc(fplat, fplon, lat_deg, lon_deg);

		tmpAngle_difference = fabs(theta1 - tmp_wp_ptr->course_rad_to_next_node);

		if (M_PI < tmpAngle_difference) {
			tmpAngle_difference = 2 * M_PI - tmpAngle_difference;
		}

		if (max_angle_difference < tmpAngle_difference) {
			max_angle_difference = tmpAngle_difference;
			waypointNodePtr_max_angle_difference = tmp_wp_ptr;
		}

		tmp_final_wp_ptr = tmp_wp_ptr;

		tmp_wp_ptr = tmp_wp_ptr->next_node_ptr; // Update pointer
	}

	if (waypointNodePtr_max_angle_difference != NULL) {
		ret_ptr = waypointNodePtr_max_angle_difference;
	}

	return ret_ptr;
}

static real_t descent_integrand(real_t h, int model_index) {
	const AdbPTFModel* model = &g_adb_ptf_models.at(model_index);
	real_t v = AdbPTFModel::KNOTS_TO_FEET_PER_MIN * model->getDescentTas(h) / 60.;
	real_t hdot = model->getDescentRate(h, NOMINAL) / 60.;
	return (sqrt(v*v - hdot*hdot) / hdot);
}

static real_t climb_integrand(real_t h, int model_index) {
	const AdbPTFModel* model = &g_adb_ptf_models.at(model_index);
	real_t v = AdbPTFModel::KNOTS_TO_FEET_PER_MIN * model->getClimbTas(h) / 60.;
	real_t hdot = model->getClimbRate(h, NOMINAL) / 60.;
	return (sqrt(v*v - hdot*hdot) / hdot);
}

real_t compute_descent_dist(int adb_table, real_t dest_elev, real_t cruise_alt) {
	real_t dist = 0.0;

	int model_index = adb_table;
	int num_interval = 1000; // Number of segments we split

	const AdbPTFModel* model = &g_adb_ptf_models.at(model_index);

	real_t dH = (cruise_alt - dest_elev) / num_interval;
	real_t tmpH = cruise_alt;
	real_t tmpV;
	real_t tmpHdot;
	real_t tmpAngleGamma;
	real_t tmpdT;
	real_t tmpdS;

	for (unsigned int i = 0; i < num_interval; i++) {
		// Calculate new altitude
		tmpH = cruise_alt - i * dH;

		if (i == 0) {
			tmpV = model->getCruiseTas(cruise_alt);
		} else {
			tmpV = model->getDescentTas(tmpH);
		}

		tmpHdot = model->getDescentRate(tmpH, NOMINAL) / 60;

		tmpAngleGamma = asin(tmpHdot / tmpV);

		tmpdT = dH / tmpHdot;

		tmpdS = abs(tmpV * cos(tmpAngleGamma) * tmpdT);

		dist += tmpdS;
	}

	return dist;
}

real_t compute_climb_dist(int adb_table, real_t orig_elev, real_t cruise_alt) {

	int model_index = adb_table;
	int row = 0;
	int num_interval = 1000;
	const AdbPTFModel* model = &g_adb_ptf_models.at(model_index);

	// find the first adb row that is above orig_elev
	while (model->getAltitude(row) < orig_elev) {
		++row;
	}

	// integrate to get climb dist of first segment (fine resolution)
	real_t dh = model->getAltitude(row) - orig_elev;
	int n = (int)ceil(fabs(dh) / num_interval);
	real_t interval = dh/(real_t)n;
	real_t dist = 0;
	
	if (0 < dh) {
		for (int k=0; k<n; ++k) {
			real_t a,b;
			b = model->getAltitude(row) - interval*k;
			a = model->getAltitude(row) - interval*(k+1);
			real_t f_a = climb_integrand(a, model_index);
			real_t f_b = climb_integrand(b, model_index);
			real_t f_mid = climb_integrand(.5*(a+b), model_index);
			dist += ((b-a) * (f_a + 4*f_mid + f_b) / 6.);
		}
		++row;
	}

	// accumulate dist from table (coarse resolution)
	while ((cruise_alt > model->getAltitude(row)) && (row < model->getNumRows())) {
		dist += model->getClimbDistance(model->getAltitude(row));
		++row;
		if (row >= model->getNumRows()) {
			--row;
			break;
		}
	}

	// integrate to get climb dist of last segment (fine resolution)
	dh = (cruise_alt - model->getAltitude(row));
	n = (int)ceil(fabs(dh) / num_interval);
	interval = dh / (real_t)n;
	for (int k=0; k<n; ++k) {
		real_t a,b;
		a = model->getAltitude(row) + interval*k;
		b = model->getAltitude(row) + interval*(k+1);
		real_t f_a = climb_integrand(a, model_index);
		real_t f_b = climb_integrand(b, model_index);
		real_t f_mid = climb_integrand(.5*(a+b), model_index);
		dist += ((b-a) * (f_a + 4*f_mid + f_b) / 6.);
	}

	return dist;
}

// helper struct for storing host data for active flights
// we use this as a temporary storage struct on the host
// because flights may be ignored
typedef struct _temp_flight_t {
	// aircraft current state
	int    sector_index;
	real_t latitude_deg;
	real_t longitude_deg;
	real_t altitude_ft;
	real_t rocd_fps;
	real_t tas_knots;
	real_t course_rad;
	real_t fpa_rad;

	ENUM_Flight_Phase flight_phase;

	// aircraft static data
	real_t departure_time_sec;
	real_t cruise_alt_ft;
	real_t cruise_tas_knots;
	//FLIGHT PLAN STARTS
	real_t flight_plan_latitude_deg[MAX_FLIGHT_PLAN_LENGTH];
	real_t flight_plan_longitude_deg[MAX_FLIGHT_PLAN_LENGTH];
	char* flight_plan_waypoint_name[MAX_FLIGHT_PLAN_LENGTH];
	char* flight_plan_path_term[MAX_FLIGHT_PLAN_LENGTH];//ARINC 424.18 SECTION 5.21
	char* flight_plan_alt_desc[MAX_FLIGHT_PLAN_LENGTH];//ARINC 424.18 SECTION 5.29
	real_t flight_plan_alt_1[MAX_FLIGHT_PLAN_LENGTH];//ARINC 424.18 SECTION 5.30
	real_t flight_plan_alt_2[MAX_FLIGHT_PLAN_LENGTH];//ARINC 424.18 SECTION 5.30
	real_t flight_plan_speed_lim[MAX_FLIGHT_PLAN_LENGTH];//ARINC 424.18 SECTION 5.72
	  //NEW ADDITIONS
	char* flight_plan_procname[MAX_FLIGHT_PLAN_LENGTH];//ARINC 424.18 SECTION 5.10
	char* flight_plan_proctype[MAX_FLIGHT_PLAN_LENGTH];//ARINC 424.18 SECTION 5.5 (SID/STAR OR APPROACH)
	char* flight_plan_phase[MAX_FLIGHT_PLAN_LENGTH]; // Geo style
	  //NEW ADDITIONS FOR NATS
	char* flight_plan_recco_navaid[MAX_FLIGHT_PLAN_LENGTH];//ARINC 424.18 SECTION 5.23
	real_t flight_plan_theta[MAX_FLIGHT_PLAN_LENGTH];//ARINC 424.18 SECTION 5.24
	real_t flight_plan_rho[MAX_FLIGHT_PLAN_LENGTH];//ARINC 424.18 SECTION 5.25
	real_t flight_plan_mag_course[MAX_FLIGHT_PLAN_LENGTH];//ARINC 424.18 SECTION 5.26
	real_t flight_plan_rt_dist[MAX_FLIGHT_PLAN_LENGTH];//ARINC 424.18 SECTION 5.27
	char* flight_plan_spdlim_desc[MAX_FLIGHT_PLAN_LENGTH];//ARINC 424.18 SECTION 5.261
	//TILL HERE
	//int    flight_plan_length;
	real_t origin_airport_elevation_ft;
	real_t destination_airport_elevation_ft;

	// aircraft adb PTF climb/descent
	int adb_aircraft_type_index; // selects the table

	// aircraft intent
	int target_waypoint_index;
	real_t target_altitude_ft;

	int toc_index;
	int tod_index;

	waypoint_node_t* target_waypoint_ptr = NULL;

	waypoint_node_t* airborne_waypoint_node_ptr = NULL;
	waypoint_node_t* airborne_toc_waypoint_node_ptr = NULL;
	waypoint_node_t* airborne_tod_waypoint_node_ptr = NULL;
	waypoint_node_t* airborne_final_waypoint_node_ptr = NULL;
	int              airborne_waypoint_length = 0;

	waypoint_node_t* departing_waypoint_node_ptr = NULL;
	waypoint_node_t* departing_final_waypoint_node_ptr = NULL;
	int              departing_waypoint_length = 0;
	char*            departing_runway_name = NULL;

	waypoint_node_t* landing_waypoint_node_ptr = NULL;
	waypoint_node_t* landing_final_waypoint_node_ptr = NULL;
	int              landing_waypoint_length = 0;
	char*            landing_runway_name = NULL;

	int key;
} temp_flight_t;

waypoint_node_t* getWaypointNodePtr_by_flightSeq(int flightSeq, int index_airborne_waypoint) {
	waypoint_node_t* retPtr = NULL;

	waypoint_node_t* tmpPtr = NULL;

	if ((0 <= flightSeq) && (flightSeq < num_flights)
			&& (0 <= index_airborne_waypoint)
			&& (index_airborne_waypoint < array_Airborne_Flight_Plan_Waypoint_length[flightSeq])) {
		tmpPtr = array_Airborne_Flight_Plan_ptr[flightSeq];

		int tmpIndex = 0;
		while (tmpIndex < index_airborne_waypoint) {
			tmpPtr = tmpPtr->next_node_ptr;

			tmpIndex++;
		}
	}

	retPtr = tmpPtr;

	return retPtr;
}

double get_runway_hdg(const string& ap,
		const string& rw1,
		const string& rw2){

	vector<AirportNode> allrunways =  getVector_AllRunways(ap);

	double lat1_deg = -10000, lon1_deg = -10000, lat2_deg=-10000, lon2_deg=-10000;

	for (size_t k=0; k < allrunways.size(); ++k){
		string refName1 = allrunways.at(k).refName1;
		string refName2 = allrunways.at(k).refName2;
		refName1 = trim(refName1);
		refName2 = trim(refName2);
		if ((refName1.length() > 0) &&
				(refName1 == rw1)) {
			lat1_deg = allrunways.at(k).latitude;
			lon1_deg = allrunways.at(k).longitude;
		}
		else if ((refName1.length() > 0) &&
				( refName1 == rw2)) {
			lat2_deg = allrunways.at(k).latitude;
			lon2_deg = allrunways.at(k).longitude;
		}

		if (lat1_deg > -180 && lon1_deg >-180 && lat2_deg >-180 && lon2_deg >-180)
			break;

	}

	double rw_hdg_deg = compute_heading_gc(lat1_deg, lon1_deg, lat2_deg, lon2_deg);

	return rw_hdg_deg;
}


pair<double,double> getTouchdownPoint(const string& ap, const string& runway,
		const double airport_alt_ft,
		const double& landing_length) {
	double dist_ft = 1000; // Default allowable length from runway entry point

	double ptlat = -1.0;
	double ptlon = -1.0;

	vector<AirportNode> allrunways =  getVector_AllRunways(ap);

	string opp_runway = "";

	string string_refName1;
	string string_refName2;
	string string_runway(runway);

	string_runway = trim(string_runway);
	if (string_runway.length() > 0) {
		for (size_t k=0; k < allrunways.size(); ++k){
			string_refName1 = allrunways.at(k).refName1;
			string_refName1 = trim(string_refName1);
			if ((string_refName1.length() > 0) &&
					(string_refName1
							== string_runway)) {
				opp_runway = allrunways.at(k).refName2;
				break;
			}
		}

		if (opp_runway == ""){
			printf("No such runway found %s.\n",runway.c_str());
			return make_pair(0.0,0.0);
		}

		double lat1_deg = -10000, lon1_deg = -10000, lat2_deg=-10000, lon2_deg=-10000;

		for (size_t k=0; k < allrunways.size(); ++k){
			string_refName1 = allrunways.at(k).refName1;
			string_refName1 = trim(string_refName1);

			if ((string_refName1.length() > 0) &&
					(string_refName1
							== string_runway)) {
				lat1_deg = allrunways.at(k).latitude;
				lon1_deg = allrunways.at(k).longitude;
			}
			else if ((string_refName1.length() > 0) &&
					(string_refName1
							== opp_runway)) {
				lat2_deg = allrunways.at(k).latitude;
				lon2_deg = allrunways.at(k).longitude;
			}

			if (lat1_deg > -180 && lon1_deg >-180 && lat2_deg >-180 && lon2_deg >-180)
				break;

		}

		double runway_length = compute_distance_gc(lat1_deg, lon1_deg, lat2_deg, lon2_deg, airport_alt_ft, RADIUS_EARTH_FT);

		double rw_hdg_deg = compute_heading_gc(lat1_deg, lon1_deg, lat2_deg, lon2_deg);

		if ((runway_length - landing_length) < dist_ft) {
			dist_ft = runway_length - landing_length;
		}
		double del_rad = dist_ft/RADIUS_EARTH_FT;

		//latitude
		double term1 = sin(lat1_deg*M_PI/180)*cos(del_rad);
		double term2 = cos(lat1_deg*M_PI/180)*sin(del_rad)*cos(rw_hdg_deg*M_PI/180);
		ptlat = asin(term1+term2)*180/M_PI;

		//longitude
		term1 = sin(rw_hdg_deg*M_PI/180)* sin(del_rad)* cos(lat1_deg*M_PI/180);
		term2 = cos(del_rad)-sin(lat1_deg*M_PI/180)*sin(ptlat*M_PI/180);
		ptlon = lon1_deg+atan2(term1,term2)*180/M_PI;
	}

	return make_pair(ptlat, ptlon);
}

/**
 * Build waypoint node list of the airborne flight plan
 */
bool buildAirborne_waypoint_node_linkedList(temp_flight_t& temp_flight, const FlightPlan fp) {
	bool retValue = false;

	vector<PointWGS84> tmpRoute = fp.route;

	temp_flight.airborne_waypoint_node_ptr = NULL; // Reset
	temp_flight.airborne_toc_waypoint_node_ptr = NULL; // Reset
	temp_flight.airborne_tod_waypoint_node_ptr = NULL; // Reset
	temp_flight.airborne_final_waypoint_node_ptr = NULL; // Reset
	temp_flight.airborne_waypoint_length = 0; // Reset

	string tmpStr;

	if (tmpRoute.size() > 0) {
		bool flag_found_initial_target_waypoint = false;

		waypoint_node_t* tmpAirborne_Flight_Plan_ptr = NULL;
		waypoint_node_t* tmpAirborne_Flight_Plan_Final_Node_ptr = NULL;

		int idx = 0;

		vector<PointWGS84>::iterator iterater_route;
		for (iterater_route = tmpRoute.begin();
				iterater_route != tmpRoute.end(); iterater_route++) {
			if (!flag_found_initial_target_waypoint) {
				tmpStr.assign(iterater_route->wpname);
				if ((fp.initial_target.find("RW") != string::npos) && (tmpStr.find(fp.initial_target) == 0)) {
					flag_found_initial_target_waypoint = true;
				} else {
					flag_found_initial_target_waypoint = true;
				}
			}

			if (flag_found_initial_target_waypoint) {
				// Create new waypoint
				waypoint_node_t* newWaypoint_node_ptr = (waypoint_node_t*)calloc(1, sizeof(waypoint_node_t));
				newWaypoint_node_ptr->prev_node_ptr = NULL; // Reset
				newWaypoint_node_ptr->next_node_ptr = NULL; // Reset
				newWaypoint_node_ptr->wpname = NULL;

				if ((iterater_route->wpname.length() > 0)) {
					newWaypoint_node_ptr->wpname = (char*)malloc((iterater_route->wpname.length()+1) * sizeof(char));
					strcpy(newWaypoint_node_ptr->wpname, iterater_route->wpname.c_str());
					newWaypoint_node_ptr->wpname[iterater_route->wpname.length()] = '\0';
				} else {
					newWaypoint_node_ptr->wpname = (char*)malloc(2 * sizeof(char));
					strcpy(newWaypoint_node_ptr->wpname, "");
					newWaypoint_node_ptr->wpname[1] = '\0';
				}

				newWaypoint_node_ptr->latitude = iterater_route->latitude;
				newWaypoint_node_ptr->longitude = iterater_route->longitude;

				if ((iterater_route->alt_desc.length() > 0)) {
					newWaypoint_node_ptr->alt_desc = (char*)malloc((iterater_route->alt_desc.length()+1) * sizeof(char));
					strcpy(newWaypoint_node_ptr->alt_desc, iterater_route->alt_desc.c_str());
					newWaypoint_node_ptr->alt_desc[iterater_route->alt_desc.length()] = '\0';
				} else {
					newWaypoint_node_ptr->alt_desc = (char*)malloc(2 * sizeof(char));
					strcpy(newWaypoint_node_ptr->alt_desc, "");
					newWaypoint_node_ptr->alt_desc[1] = '\0';
				}

				newWaypoint_node_ptr->alt_1 = iterater_route->alt_1;
				newWaypoint_node_ptr->alt_2 = iterater_route->alt_2;

				if ((iterater_route->procname.length() > 0)) {
					newWaypoint_node_ptr->procname = (char*)malloc((iterater_route->procname.length()+1) * sizeof(char));
					strcpy(newWaypoint_node_ptr->procname, iterater_route->procname.c_str());
					newWaypoint_node_ptr->procname[iterater_route->procname.length()] = '\0';
				} else {
					newWaypoint_node_ptr->procname = (char*)malloc(2 * sizeof(char));
					strcpy(newWaypoint_node_ptr->procname, "");
					newWaypoint_node_ptr->procname[1] = '\0';
				}

				if ((iterater_route->proctype.length() > 0)) {
					newWaypoint_node_ptr->proctype = (char*)malloc((iterater_route->proctype.length()+1) * sizeof(char));
					strcpy(newWaypoint_node_ptr->proctype, iterater_route->proctype.c_str());
					newWaypoint_node_ptr->proctype[iterater_route->proctype.length()] = '\0';
				} else {
					newWaypoint_node_ptr->proctype = (char*)malloc(2 * sizeof(char));
					strcpy(newWaypoint_node_ptr->proctype, "");
					newWaypoint_node_ptr->proctype[1] = '\0';
				}

				if ((iterater_route->recco_navaid.length() > 0)) {
					newWaypoint_node_ptr->recco_navaid = (char*)malloc((iterater_route->recco_navaid.length()+1) * sizeof(char));
					strcpy(newWaypoint_node_ptr->recco_navaid, iterater_route->recco_navaid.c_str());
					newWaypoint_node_ptr->recco_navaid[iterater_route->recco_navaid.length()] = '\0';
				} else {
					newWaypoint_node_ptr->recco_navaid = (char*)malloc(2 * sizeof(char));
					strcpy(newWaypoint_node_ptr->recco_navaid, "");
					newWaypoint_node_ptr->recco_navaid[1] = '\0';
				}

				newWaypoint_node_ptr->theta = iterater_route->theta;
				newWaypoint_node_ptr->rho = iterater_route->rho;
				newWaypoint_node_ptr->mag_course = iterater_route->mag_course;
				newWaypoint_node_ptr->rt_dist = iterater_route->rt_dist;

				if ((iterater_route->spdlim_desc.length() > 0)) {
					newWaypoint_node_ptr->spdlim_desc = (char*)malloc((iterater_route->spdlim_desc.length()+1) * sizeof(char));
					strcpy(newWaypoint_node_ptr->spdlim_desc, iterater_route->spdlim_desc.c_str());
					newWaypoint_node_ptr->spdlim_desc[iterater_route->spdlim_desc.length()] = '\0';
				} else {
					newWaypoint_node_ptr->spdlim_desc = (char*)malloc(2 * sizeof(char));
					strcpy(newWaypoint_node_ptr->spdlim_desc, "");
					newWaypoint_node_ptr->spdlim_desc[1] = '\0';
				}

				newWaypoint_node_ptr->speed_lim = iterater_route->speed_lim;

				if (strcmp(newWaypoint_node_ptr->wpname, TOP_OF_CLIMB_PT) == 0) {
					temp_flight.airborne_toc_waypoint_node_ptr = newWaypoint_node_ptr;
					temp_flight.toc_index = idx;
				} else if (strcmp(newWaypoint_node_ptr->wpname, TOP_OF_DESCENT_PT) == 0) {
					temp_flight.airborne_tod_waypoint_node_ptr = newWaypoint_node_ptr;
					temp_flight.tod_index = idx;
				}

				if (tmpAirborne_Flight_Plan_Final_Node_ptr != NULL) {
					newWaypoint_node_ptr->prev_node_ptr = tmpAirborne_Flight_Plan_Final_Node_ptr;
					tmpAirborne_Flight_Plan_Final_Node_ptr->next_node_ptr = newWaypoint_node_ptr;
				}

				if (newWaypoint_node_ptr->prev_node_ptr != NULL) {
					newWaypoint_node_ptr->prev_node_ptr->course_rad_to_next_node = compute_heading_rad_gc(newWaypoint_node_ptr->prev_node_ptr->latitude,
							newWaypoint_node_ptr->prev_node_ptr->longitude,
							newWaypoint_node_ptr->latitude,
							newWaypoint_node_ptr->longitude);
				}

				tmpAirborne_Flight_Plan_Final_Node_ptr = newWaypoint_node_ptr;

				if (tmpAirborne_Flight_Plan_ptr == NULL) {
					tmpAirborne_Flight_Plan_ptr = newWaypoint_node_ptr;
				}

				idx++;
			}
		}

		temp_flight.airborne_waypoint_node_ptr = tmpAirborne_Flight_Plan_ptr;

		temp_flight.airborne_final_waypoint_node_ptr = tmpAirborne_Flight_Plan_Final_Node_ptr;

		if (temp_flight.airborne_final_waypoint_node_ptr != NULL)
			temp_flight.airborne_final_waypoint_node_ptr->next_node_ptr = NULL;

		temp_flight.airborne_waypoint_length = tmpRoute.size();
	} else {
		printf("          No airborne waypoint specified\n");
	}

	retValue = true;

	return retValue;
}

/**
 * Build waypoint node list of the airborne flight plan
 */
bool buildAirborne_waypoint_node_linkedList_geoStyle(temp_flight_t& temp_flight,
		const FlightPlan* fp) {
	bool retValue = false;

	vector<PointWGS84> tmpRoute = fp->route;

	temp_flight.airborne_waypoint_node_ptr = NULL; // Reset
	temp_flight.airborne_toc_waypoint_node_ptr = NULL; // Reset
	temp_flight.airborne_tod_waypoint_node_ptr = NULL; // Reset
	temp_flight.airborne_final_waypoint_node_ptr = NULL; // Reset
	temp_flight.airborne_waypoint_length = 0; // Reset

	string tmpStr;

	if (tmpRoute.size() > 0) {
		bool flag_found_initial_target_waypoint = false;

		waypoint_node_t* tmpAirborne_Flight_Plan_ptr = NULL;
		waypoint_node_t* tmpAirborne_Flight_Plan_Final_Node_ptr = NULL;

		int idx = 0;

		vector<PointWGS84>::iterator iterater_route;
		for (iterater_route = tmpRoute.begin();
			iterater_route != tmpRoute.end(); iterater_route++) {

			// Create new waypoint
			waypoint_node_t* newWaypoint_node_ptr = (waypoint_node_t*)calloc(1, sizeof(waypoint_node_t));
			newWaypoint_node_ptr->prev_node_ptr = NULL; // Reset
			newWaypoint_node_ptr->next_node_ptr = NULL; // Reset
			newWaypoint_node_ptr->wpname = NULL;

			newWaypoint_node_ptr->flag_geoStyle = true;

			if ((iterater_route->wpname.length() > 0)) {
				newWaypoint_node_ptr->wpname = (char*)malloc((iterater_route->wpname.length()+1) * sizeof(char));
				strcpy(newWaypoint_node_ptr->wpname, iterater_route->wpname.c_str());
				newWaypoint_node_ptr->wpname[iterater_route->wpname.length()] = '\0';
			} else {
				newWaypoint_node_ptr->wpname = (char*)malloc(2 * sizeof(char));
				strcpy(newWaypoint_node_ptr->wpname, "");
				newWaypoint_node_ptr->wpname[1] = '\0';
			}

			newWaypoint_node_ptr->latitude = iterater_route->latitude;
			newWaypoint_node_ptr->longitude = iterater_route->longitude;

			if (strcmp(newWaypoint_node_ptr->wpname, TOP_OF_CLIMB_PT) == 0) {
				temp_flight.airborne_toc_waypoint_node_ptr = newWaypoint_node_ptr;
				temp_flight.toc_index = idx;
			} else if (strcmp(newWaypoint_node_ptr->wpname, TOP_OF_DESCENT_PT) == 0) {
				temp_flight.airborne_tod_waypoint_node_ptr = newWaypoint_node_ptr;
				temp_flight.tod_index = idx;
			}

			newWaypoint_node_ptr->altitude_estimate = iterater_route->alt;
			newWaypoint_node_ptr->phase = (char*)calloc(iterater_route->phase.length()+1, sizeof(char));
			strcpy(newWaypoint_node_ptr->phase, iterater_route->phase.c_str());
			newWaypoint_node_ptr->phase[iterater_route->phase.length()] = '\0';

			if (tmpAirborne_Flight_Plan_Final_Node_ptr != NULL) {
				newWaypoint_node_ptr->prev_node_ptr = tmpAirborne_Flight_Plan_Final_Node_ptr;
				tmpAirborne_Flight_Plan_Final_Node_ptr->next_node_ptr = newWaypoint_node_ptr;
			}

			if (newWaypoint_node_ptr->prev_node_ptr != NULL) {
				newWaypoint_node_ptr->prev_node_ptr->course_rad_to_next_node = compute_heading_rad_gc(newWaypoint_node_ptr->prev_node_ptr->latitude,
						newWaypoint_node_ptr->prev_node_ptr->longitude,
						newWaypoint_node_ptr->latitude,
						newWaypoint_node_ptr->longitude);
			}

			tmpAirborne_Flight_Plan_Final_Node_ptr = newWaypoint_node_ptr;

			if (tmpAirborne_Flight_Plan_ptr == NULL) {
				tmpAirborne_Flight_Plan_ptr = newWaypoint_node_ptr;
			}

			idx++;
		}

		temp_flight.airborne_waypoint_node_ptr = tmpAirborne_Flight_Plan_ptr;

		temp_flight.airborne_final_waypoint_node_ptr = tmpAirborne_Flight_Plan_Final_Node_ptr;

		if (temp_flight.airborne_final_waypoint_node_ptr != NULL)
			temp_flight.airborne_final_waypoint_node_ptr->next_node_ptr = NULL;

		temp_flight.airborne_waypoint_length = tmpRoute.size();
	} else {
		printf("          No airborne waypoint specified\n");
	}

	retValue = true;

	return retValue;
}

/**
 * Build waypoint node list of the departing and landing taxi plans
 */
bool buildSurface_waypoint_node_linkedList(temp_flight_t& temp_flight,
		const TrxRecord* record,
		const FlightPlan* fp) {
	// Process departing/landing taxi plans in TRX ********
	vector<string> vector_departing_taxi_plan;
	vector<string> vector_landing_taxi_plan;

	vector_departing_taxi_plan.clear(); // Reset
	vector_landing_taxi_plan.clear(); // Reset

	string tmpRoute_str(record->route_str);

	int pos_dotSlashDot = tmpRoute_str.find("./.");

	int pos_departing_leftArrow = -1;
	int pos_departing_rightArrow = -1;
	int pos_landing_leftArrow = -1;
	int pos_landing_rightArrow = -1;

	if (pos_dotSlashDot < 0) {
		pos_departing_leftArrow = tmpRoute_str.find("<");
		pos_departing_rightArrow = tmpRoute_str.find(">", pos_departing_leftArrow+1);
	}
	pos_landing_leftArrow = tmpRoute_str.find_last_of("<");
	pos_landing_rightArrow = tmpRoute_str.find_last_of(">");

	string tmpRunwayName;

	int pos_1st_dot_after_pos_departing_rightArrow = -1;
	int pos_2nd_dot_after_pos_departing_rightArrow = -1;
	int pos_1st_dot_before_landing_leftArrow = -1;
	int pos_2nd_dot_before_landing_leftArrow = -1;

	if (pos_dotSlashDot < 0) {
		pos_1st_dot_after_pos_departing_rightArrow = tmpRoute_str.find_first_of(".", pos_departing_rightArrow+1);
		pos_2nd_dot_after_pos_departing_rightArrow = tmpRoute_str.find_first_of(".", pos_1st_dot_after_pos_departing_rightArrow+1);
	}
	pos_1st_dot_before_landing_leftArrow = tmpRoute_str.find_last_of(".", pos_landing_leftArrow-1);
	pos_2nd_dot_before_landing_leftArrow = tmpRoute_str.find_last_of(".", pos_1st_dot_before_landing_leftArrow-1);

	if ((pos_dotSlashDot < 0)
			&& (pos_1st_dot_after_pos_departing_rightArrow+1 != pos_2nd_dot_after_pos_departing_rightArrow)) {
		tmpRunwayName = tmpRoute_str.substr(pos_1st_dot_after_pos_departing_rightArrow+1, (pos_2nd_dot_after_pos_departing_rightArrow - pos_1st_dot_after_pos_departing_rightArrow - 1));
		if (tmpRunwayName.length() > 0) {
			temp_flight.departing_runway_name = (char*)malloc((tmpRunwayName.length()+1) * sizeof(char));
			strcpy(temp_flight.departing_runway_name, tmpRunwayName.c_str());
			temp_flight.departing_runway_name[tmpRunwayName.length()] = '\0';
		}
	}

	if (pos_2nd_dot_before_landing_leftArrow+1 != pos_1st_dot_before_landing_leftArrow) {
		tmpRunwayName = tmpRoute_str.substr(pos_2nd_dot_before_landing_leftArrow+1, (pos_1st_dot_before_landing_leftArrow - pos_2nd_dot_before_landing_leftArrow - 1));
		if (tmpRunwayName.length() > 0) {
			temp_flight.landing_runway_name = (char*)malloc((tmpRunwayName.length()+1) * sizeof(char));
			strcpy(temp_flight.landing_runway_name, tmpRunwayName.c_str());
			temp_flight.landing_runway_name[tmpRunwayName.length()] = '\0';
		}
	}

	bool flag_taxi_plan_validity = true;

	string string_departing_taxi_plan;
	if (pos_dotSlashDot < 0) {
		string_departing_taxi_plan = tmpRoute_str.substr(pos_departing_leftArrow+1, (pos_departing_rightArrow - pos_departing_leftArrow - 1));
	}
	string string_landing_taxi_plan = tmpRoute_str.substr(pos_landing_leftArrow+1, (pos_landing_rightArrow - pos_landing_leftArrow - 1));

	if ((string_departing_taxi_plan.length() > 0) && (temp_flight.departing_runway_name == NULL)) {
		printf("             Departing runway name is not valid.\n", record->acid.c_str());
	}
	if ((string_landing_taxi_plan.length() > 0) && (temp_flight.landing_runway_name == NULL)) {
		printf("             Landing runway name is not valid.\n", record->acid.c_str());
	}

	if (!flag_taxi_plan_validity) {
		return false;
	}

	string tmpTaxi_plan_str;

	string tmpValue_Id;
	string tmpValue_Latitude;
	string tmpValue_Longitude;

	// ========================================================================

	flag_taxi_plan_validity = true; // Reset

	// Process departing taxi plan
	tmpTaxi_plan_str.assign(string_departing_taxi_plan);

	trim(tmpTaxi_plan_str);
	if (tmpTaxi_plan_str.length() > 0) {
		if (tmpTaxi_plan_str.find_first_of("{") != 0) {
			printf("             Departing taxi plan is not valid.\n", record->acid.c_str());
			flag_taxi_plan_validity = false;
		}
		while (tmpTaxi_plan_str.find_first_of("{") == 0) {
			tmpValue_Id.clear(); // Reset
			tmpValue_Latitude.clear(); // Reset
			tmpValue_Longitude.clear(); // Reset

			int tmpPos_parenthesis_L = tmpTaxi_plan_str.find_first_of("{");
			int tmpPos_parenthesis_R = tmpTaxi_plan_str.find_first_of("}");
			if ((tmpPos_parenthesis_L < 0) || (tmpPos_parenthesis_R < 0)) {
				printf("             Departing taxi plan is not valid.\n", record->acid.c_str());
				vector_departing_taxi_plan.clear();
				flag_taxi_plan_validity = false;

				break;
			}

			json jsonObj;

			string tmpJsonStr = tmpTaxi_plan_str.substr(tmpPos_parenthesis_L, (tmpPos_parenthesis_R - tmpPos_parenthesis_L + 1));

			jsonObj = json::parse(tmpJsonStr);

			if (jsonObj.contains("id")) {
				tmpValue_Id = jsonObj.at("id").get<string>();
			}

			if (jsonObj.contains("latitude")) {
				tmpValue_Latitude = jsonObj.at("latitude").get<string>();
			}
			if (jsonObj.contains("longitude")) {
				tmpValue_Longitude = jsonObj.at("longitude").get<string>();
			}

			if (tmpValue_Id.length() > 0) {
				vector_departing_taxi_plan.push_back(tmpValue_Id);
			}

			if (tmpTaxi_plan_str.length() > (tmpPos_parenthesis_R+1)) {
				tmpTaxi_plan_str.erase(0, tmpPos_parenthesis_R+1);
			} else {
				break;
			}

			if (tmpTaxi_plan_str.find_first_of(",")+1 > 0) {
				tmpTaxi_plan_str.erase(0, tmpTaxi_plan_str.find_first_of(",")+1);
			}

			trim(tmpTaxi_plan_str);
			if (tmpTaxi_plan_str.find_first_of("{") != 0) {
				printf("             Departing taxi plan is not valid.\n", record->acid.c_str());
				vector_departing_taxi_plan.clear();
				flag_taxi_plan_validity = false;

				break;
			}
		}

		if ((flag_taxi_plan_validity) && (vector_departing_taxi_plan.size() > 0)) {
			if ((tmpValue_Latitude.length() > 0) && (tmpValue_Longitude.length() > 0)) {
				if (vector_departing_taxi_plan.size() <= 1) {
					printf("             Departing taxi plan is not valid.\n", record->acid.c_str());
					flag_taxi_plan_validity = false;
				} else {
					double tmpLatLon[2];
					tmpLatLon[0] = strtod(tmpValue_Latitude.c_str(), NULL);
					tmpLatLon[1] = strtod(tmpValue_Longitude.c_str(), NULL);

					double (*tmpLatLon_ptr)[2];
					tmpLatLon_ptr = &tmpLatLon;

					//set_user_defined_surface_taxi_plan(i, fp->origin, vector_departing_taxi_plan, tmpLatLon);
					temp_flight.departing_waypoint_node_ptr = getSurface_waypoint_node_linkedList(fp->origin, vector_departing_taxi_plan, tmpLatLon_ptr);
				}
			} else {
				if (vector_departing_taxi_plan.size() <= 2) {
					printf("             Departing taxi plan is not valid.\n", record->acid.c_str());
					flag_taxi_plan_validity = false;
				} else {
					//set_user_defined_surface_taxi_plan(i, fp->origin, vector_departing_taxi_plan);
					temp_flight.departing_waypoint_node_ptr = getSurface_waypoint_node_linkedList(fp->origin, vector_departing_taxi_plan, NULL);
				}
			}
		}
	}

	if (flag_taxi_plan_validity) {
		if (temp_flight.departing_waypoint_node_ptr != NULL) {
			int cnt_waypoint = 0;
			waypoint_node_t* tmp_wp_ptr = temp_flight.departing_waypoint_node_ptr;
			while (tmp_wp_ptr != NULL) {
				cnt_waypoint++;

				temp_flight.departing_final_waypoint_node_ptr = tmp_wp_ptr;

				tmp_wp_ptr = tmp_wp_ptr->next_node_ptr; // Update pointer
			}

			temp_flight.departing_waypoint_length = cnt_waypoint;
		}
	} else {
		return flag_taxi_plan_validity;
	}

	// ========================================================================

	flag_taxi_plan_validity = true; // Reset

	// Process landing taxi plan
	tmpTaxi_plan_str.assign(string_landing_taxi_plan);

	trim(tmpTaxi_plan_str);
	if (tmpTaxi_plan_str.length() > 0) {
		if (tmpTaxi_plan_str.find_first_of("{") != 0) {
			printf("Aircraft %s: Landing taxi plan is not valid.\n", record->acid.c_str());
			flag_taxi_plan_validity = false;
		}
		while (tmpTaxi_plan_str.find_first_of("{") == 0) {
			tmpValue_Id.clear(); // Reset
			tmpValue_Latitude.clear(); // Reset
			tmpValue_Longitude.clear(); // Reset

			int tmpPos_parenthesis_L = tmpTaxi_plan_str.find_first_of("{");
			int tmpPos_parenthesis_R = tmpTaxi_plan_str.find_first_of("}");
			if ((tmpPos_parenthesis_L < 0) || (tmpPos_parenthesis_R < 0)) {
				printf("             Landing taxi plan is not valid.\n", record->acid.c_str());
				vector_landing_taxi_plan.clear();
				flag_taxi_plan_validity = false;

				break;
			}

			json jsonObj;

			string tmpJsonStr = tmpTaxi_plan_str.substr(tmpPos_parenthesis_L, (tmpPos_parenthesis_R - tmpPos_parenthesis_L + 1));

			jsonObj = json::parse(tmpJsonStr);

			if (jsonObj.contains("id")) {
				tmpValue_Id = jsonObj.at("id").get<string>();
			}

			if (jsonObj.contains("latitude")) {
				tmpValue_Latitude = jsonObj.at("latitude").get<string>();
			}
			if (jsonObj.contains("longitude")) {
				tmpValue_Longitude = jsonObj.at("longitude").get<string>();
			}

			if (tmpValue_Id.length() > 0) {
				vector_landing_taxi_plan.push_back(tmpValue_Id);
			}

			if (tmpTaxi_plan_str.length() > (tmpPos_parenthesis_R+1)) {
				tmpTaxi_plan_str.erase(0, tmpPos_parenthesis_R+1);
			} else {
				break;
			}

			if (tmpTaxi_plan_str.find_first_of(",")+1 > 0) {
				tmpTaxi_plan_str.erase(0, tmpTaxi_plan_str.find_first_of(",")+1);
			}

			trim(tmpTaxi_plan_str);
			if (tmpTaxi_plan_str.find_first_of("{") != 0) {
				printf("             Landing taxi plan is not valid.\n", record->acid.c_str());
				vector_landing_taxi_plan.clear();
				flag_taxi_plan_validity = false;

				break;
			}
		}

		if ((flag_taxi_plan_validity) && (vector_landing_taxi_plan.size() > 0)) {
			if ((tmpValue_Latitude.length() > 0) && (tmpValue_Longitude.length() > 0)) {
				if (vector_landing_taxi_plan.size() <= 1) {
					printf("             Landing taxi plan is not valid.\n", record->acid.c_str());
					flag_taxi_plan_validity = false;
				} else {
					double tmpLatLon[2];
					tmpLatLon[0] = strtod(tmpValue_Latitude.c_str(), NULL);
					tmpLatLon[1] = strtod(tmpValue_Longitude.c_str(), NULL);

					double (*tmpLatLon_ptr)[2];
					tmpLatLon_ptr = &tmpLatLon;

					//set_user_defined_surface_taxi_plan(i, fp->destination, vector_landing_taxi_plan, tmpLatLon);
					temp_flight.landing_waypoint_node_ptr = getSurface_waypoint_node_linkedList(fp->destination, vector_landing_taxi_plan, tmpLatLon_ptr);
				}
			} else {
				if (vector_landing_taxi_plan.size() <= 2) {
					printf("             Landing taxi plan is not valid.\n", record->acid.c_str());
					flag_taxi_plan_validity = false;
				} else {
					//set_user_defined_surface_taxi_plan(i, fp->destination, vector_landing_taxi_plan);
					temp_flight.landing_waypoint_node_ptr = getSurface_waypoint_node_linkedList(fp->destination, vector_landing_taxi_plan, NULL);
				}
			}
		}
	}

	if (flag_taxi_plan_validity) {
		if (temp_flight.landing_waypoint_node_ptr != NULL) {
			int cnt_waypoint = 0;
			waypoint_node_t* tmp_wp_ptr = temp_flight.landing_waypoint_node_ptr;
			while (tmp_wp_ptr != NULL) {
				cnt_waypoint++;

				temp_flight.landing_final_waypoint_node_ptr = tmp_wp_ptr;

				tmp_wp_ptr = tmp_wp_ptr->next_node_ptr; // Update pointer
			}

			temp_flight.landing_waypoint_length = cnt_waypoint;
		}
	}

	return flag_taxi_plan_validity;
}

/**
 * Build waypoint node list of the departing and landing taxi plans
 */
bool buildSurface_waypoint_node_linkedList_geoStyle(temp_flight_t& temp_flight,
		const TrxRecord* record,
		const FlightPlan* fp) {
	vector<PointWGS84> vector_departing_taxi_plan;
	vector<PointWGS84> vector_landing_taxi_plan;

	vector_departing_taxi_plan.clear(); // Reset
	vector_landing_taxi_plan.clear(); // Reset

	bool flag_taxi_plan_validity = true;

	string string_departing_taxi_plan(fp->departing_taxiPlanString);

	string string_landing_taxi_plan(fp->landing_taxiPlanString);

	string tmpTaxi_plan_str;

	string tmpValue_Type;
	double tmpValue_Latitude;
	double tmpValue_Longitude;

	// ========================================================================

	flag_taxi_plan_validity = true; // Reset

	// Process departing taxi plan
	tmpTaxi_plan_str.assign(string_departing_taxi_plan);

	trim(tmpTaxi_plan_str);
	if (tmpTaxi_plan_str.length() > 0) {
		if (tmpTaxi_plan_str.find_first_of("{") != 0) {
			printf("             Departing taxi plan is not valid.\n", record->acid.c_str());
			flag_taxi_plan_validity = false;
		}
		while (tmpTaxi_plan_str.find_first_of("{") == 0) {
			tmpValue_Type.clear(); // Reset
			tmpValue_Latitude = 0.0; // Reset
			tmpValue_Longitude = 0.0; // Reset

			int tmpPos_parenthesis_L = tmpTaxi_plan_str.find_first_of("{");
			int tmpPos_parenthesis_R = tmpTaxi_plan_str.find_first_of("}");
			if ((tmpPos_parenthesis_L < 0) || (tmpPos_parenthesis_R < 0)) {
				printf("             Departing taxi plan is not valid.\n", record->acid.c_str());
				vector_departing_taxi_plan.clear();
				flag_taxi_plan_validity = false;

				break;
			}

			json jsonObj;

			string tmpJsonStr = tmpTaxi_plan_str.substr(tmpPos_parenthesis_L, (tmpPos_parenthesis_R - tmpPos_parenthesis_L + 1));

			jsonObj = json::parse(tmpJsonStr);

			if ((jsonObj.contains("type")) && (jsonObj.contains("lat")) && (jsonObj.contains("lon"))) {
				tmpValue_Type.assign(jsonObj.at("type").get<string>());
				tmpValue_Latitude = convertLatLonString_to_deg(jsonObj.at("lat").get<string>().c_str());
				tmpValue_Longitude = convertLatLonString_to_deg(jsonObj.at("lon").get<string>().c_str());

				if (tmpValue_Type.length() > 0) {
					vector_departing_taxi_plan.push_back(PointWGS84());
					vector_departing_taxi_plan.back().type.assign(tmpValue_Type);
					vector_departing_taxi_plan.back().latitude = tmpValue_Latitude;
					vector_departing_taxi_plan.back().longitude = tmpValue_Longitude;
				}
			} else {
				printf("             Departing taxi plan is not valid.\n", record->acid.c_str());

				vector_departing_taxi_plan.clear();
				flag_taxi_plan_validity = false;

				return flag_taxi_plan_validity;
			}

			if (tmpTaxi_plan_str.length() > (tmpPos_parenthesis_R+1)) {
				tmpTaxi_plan_str.erase(0, tmpPos_parenthesis_R+1);
			} else {
				break;
			}

			if (tmpTaxi_plan_str.find_first_of(",")+1 > 0) {
				tmpTaxi_plan_str.erase(0, tmpTaxi_plan_str.find_first_of(",")+1);
			}

			trim(tmpTaxi_plan_str);
			if (tmpTaxi_plan_str.find_first_of("{") != 0) {
				printf("             Departing taxi plan is not valid.\n", record->acid.c_str());
				vector_departing_taxi_plan.clear();
				flag_taxi_plan_validity = false;

				break;
			}
		}

		if ((flag_taxi_plan_validity) && (vector_departing_taxi_plan.size() > 0)) {
			if (vector_departing_taxi_plan.size() <= 2) {
				printf("             Departing taxi plan is not valid.\n", record->acid.c_str());
				flag_taxi_plan_validity = false;
			} else {
				temp_flight.departing_waypoint_node_ptr = getSurface_waypoint_node_linkedList_geoStyle(vector_departing_taxi_plan);
			}
		}
	}

	if (flag_taxi_plan_validity) {
		if (temp_flight.departing_waypoint_node_ptr != NULL) {
			int cnt_waypoint = 0;
			waypoint_node_t* tmp_wp_ptr = temp_flight.departing_waypoint_node_ptr;
			while (tmp_wp_ptr != NULL) {
				cnt_waypoint++;

				tmp_wp_ptr->flag_geoStyle = true;

				temp_flight.departing_final_waypoint_node_ptr = tmp_wp_ptr;

				tmp_wp_ptr = tmp_wp_ptr->next_node_ptr; // Update pointer
			}

			temp_flight.departing_waypoint_length = cnt_waypoint;
		}
	} else {
		return flag_taxi_plan_validity;
	}

	// ========================================================================

	flag_taxi_plan_validity = true; // Reset

	// Process landing taxi plan
	tmpTaxi_plan_str.assign(string_landing_taxi_plan);

	trim(tmpTaxi_plan_str);
	if (tmpTaxi_plan_str.length() > 0) {
		if (tmpTaxi_plan_str.find_first_of("{") != 0) {
			printf("Aircraft %s: Landing taxi plan is not valid.\n", record->acid.c_str());
			flag_taxi_plan_validity = false;
		}
		while (tmpTaxi_plan_str.find_first_of("{") == 0) {
			tmpValue_Type.clear(); // Reset
			tmpValue_Latitude = 0.0; // Reset
			tmpValue_Longitude = 0.0; // Reset

			int tmpPos_parenthesis_L = tmpTaxi_plan_str.find_first_of("{");
			int tmpPos_parenthesis_R = tmpTaxi_plan_str.find_first_of("}");
			if ((tmpPos_parenthesis_L < 0) || (tmpPos_parenthesis_R < 0)) {
				printf("             Landing taxi plan is not valid.\n", record->acid.c_str());
				vector_landing_taxi_plan.clear();
				flag_taxi_plan_validity = false;

				break;
			}

			json jsonObj;

			string tmpJsonStr = tmpTaxi_plan_str.substr(tmpPos_parenthesis_L, (tmpPos_parenthesis_R - tmpPos_parenthesis_L + 1));

			jsonObj = json::parse(tmpJsonStr);

			if ((jsonObj.contains("type")) && (jsonObj.contains("lat")) && (jsonObj.contains("lon"))) {
				tmpValue_Type.assign(jsonObj.at("type").get<string>());
				tmpValue_Latitude = convertLatLonString_to_deg(jsonObj.at("lat").get<string>().c_str());
				tmpValue_Longitude = convertLatLonString_to_deg(jsonObj.at("lon").get<string>().c_str());

				if (tmpValue_Type.length() > 0) {
					vector_landing_taxi_plan.push_back(PointWGS84());
					vector_landing_taxi_plan.back().type.assign(tmpValue_Type);
					vector_landing_taxi_plan.back().latitude = tmpValue_Latitude;
					vector_landing_taxi_plan.back().longitude = tmpValue_Longitude;
				}
			} else {
				printf("             Landing taxi plan is not valid.\n", record->acid.c_str());

				vector_landing_taxi_plan.clear();
				flag_taxi_plan_validity = false;

				return flag_taxi_plan_validity;
			}

			if (tmpTaxi_plan_str.length() > (tmpPos_parenthesis_R+1)) {
				tmpTaxi_plan_str.erase(0, tmpPos_parenthesis_R+1);
			} else {
				break;
			}

			if (tmpTaxi_plan_str.find_first_of(",")+1 > 0) {
				tmpTaxi_plan_str.erase(0, tmpTaxi_plan_str.find_first_of(",")+1);
			}

			trim(tmpTaxi_plan_str);
			if (tmpTaxi_plan_str.find_first_of("{") != 0) {
				printf("             Landing taxi plan is not valid.\n", record->acid.c_str());
				vector_landing_taxi_plan.clear();
				flag_taxi_plan_validity = false;

				break;
			}
		}

		if ((flag_taxi_plan_validity) && (vector_landing_taxi_plan.size() > 0)) {
			if (vector_landing_taxi_plan.size() <= 2) {
				printf("             Landing taxi plan is not valid.\n", record->acid.c_str());
				flag_taxi_plan_validity = false;
			} else {
				temp_flight.landing_waypoint_node_ptr = getSurface_waypoint_node_linkedList_geoStyle(vector_landing_taxi_plan);
			}
		}
	}

	if (flag_taxi_plan_validity) {
		if (temp_flight.landing_waypoint_node_ptr != NULL) {
			int cnt_waypoint = 0;
			waypoint_node_t* tmp_wp_ptr = temp_flight.landing_waypoint_node_ptr;
			while (tmp_wp_ptr != NULL) {
				cnt_waypoint++;

				tmp_wp_ptr->flag_geoStyle = true;

				temp_flight.landing_final_waypoint_node_ptr = tmp_wp_ptr;

				tmp_wp_ptr = tmp_wp_ptr->next_node_ptr; // Update pointer
			}

			temp_flight.landing_waypoint_length = cnt_waypoint;
		}
	}

	return flag_taxi_plan_validity;
}

static bool compute_flight_data(real_t departure_time,
		TrxRecord& record,
        FlightPlan& fp,
		temp_flight_t& flight,
        const real_t& cruise_perturbation,
        omp_lock_t& lock) {
	bool retValue = false;

	printf("Aircraft: %s\n", record.acid.c_str());

	FlightPlanParser fp_parser;

	// Temp variable to store flight plan data.  If no error within this function, data of this temp variable will be copied to the formal variable.
	FlightPlan tmpFp = fp;

    if (!fp_parser.parse(record.acid,
    		record.route_str,
    		g_sids,
			g_pars,
			g_stars,
			g_approaches,
    		g_airways,
			g_waypoints,
			g_airports,
			&tmpFp,
			record.altitude,
			record.cruiseAltitude)) {
    	ignore_count++;

    	return retValue;
    }

	// Temp variable to store TRX record data.  If no error within this function, data of this temp variable will be copied to the formal variable.
	TrxRecord tmpTrxRecord = record;

	// Temp variable to store flight data.  If no error within this function, data of this temp variable will be copied to the formal variable.
    temp_flight_t tmpFlight = flight;

	if (tmpFp.route.size() < 2) {
		if ((get_airport_elevation(tmpFp.destination) < record.altitude)
				&& ((tmpFp.route.size() == 0) || (tmpFp.route.at(0).wpname.find("RW") == string::npos))) {
			printf("Aircraft %s: Wrong TRX data.  No intermediate airborne route points.\n", tmpTrxRecord.acid.c_str());

			if (&lock)
				omp_set_lock(&lock);

			++ignore_count;

			if (&lock)
				omp_unset_lock(&lock);

			return false;
		}
	}

	// ADB data
	string synonym = get_adb_synonym(tmpTrxRecord.actype);
	int adb_table = get_adb_table_index(tmpTrxRecord.actype);
	if (adb_table < 0 || adb_table >= adb_ptf_type_num) {
		printf("Aircraft %s: Wrong aircraft type: %s\n", tmpTrxRecord.acid.c_str(), tmpTrxRecord.actype.c_str());

		ignore_count++;

		return false;
	}

	const AdbPTFModel* model = &g_adb_ptf_models.at(adb_table);

	// ADD THE VI AND VA LEGS HERE
    NatsAirport orig;
    orig.code = tmpFp.origin;
    vector<NatsAirport>::iterator itap = find(g_airports.begin(), g_airports.end(), orig);
    real_t orig_mag_var = (real_t)itap->mag_variation;

    NatsAirport dest;
    dest.code = tmpFp.destination;
    itap = find(g_airports.begin(), g_airports.end(), dest);
    real_t dest_mag_var = (real_t)itap->mag_variation;

    string prev_pterm = ""; string ap = "";
    string currwpname = "", prevwpname = "";
	real_t prevlat = 0, prevlon = 0, prevalt = 0, prevcourse = 0, prev_spd_lim = 0;
	real_t currlat = 0, currlon = 0, curralt = 0, currcourse = 0, curr_spd_lim = 0;
	real_t lat_int = 0, lon_int = 0, h_int = 0;
	real_t mag_dec = 0;
	real_t rw_dir = 0;
	size_t s = 0;

	//CHECKING PATH AND TERMINATOR AND ADDING POINTS APPROPRIATELY
	while (s < tmpFp.route.size()) {
		PointWGS84 pt = tmpFp.route.at(s);
		if (pt.proctype == "SID") {
			mag_dec = orig_mag_var;
			ap = orig.code;
		} else if (pt.proctype == "STAR" || pt.proctype == "APPROACH") {
			mag_dec = dest_mag_var;
			ap = dest.code;
		} else {
			mag_dec = 0.0;
			ap = "";
		}

		currwpname = pt.wpname;
		currlat = pt.latitude;currlon = pt.longitude;
		currcourse = pt.mag_course-mag_dec;
		if (pt.alt_1 >= 0) {
			curralt = pt.alt_1;
		}
		if (pt.speed_lim >= 0) {
			curr_spd_lim = pt.speed_lim;
		}

    	if ( pt.path_n_terminator == "VD" || pt.path_n_terminator =="CD") {

    		//CHECK whether there is a VD leg after a VA leg
    		if (s > 0 && s < tmpFp.route.size()){
    			PointWGS84 prevpt = tmpFp.route.at(s-1);
    			size_t find_str = prevpt.wpname.find("HEADING OR COURSE");
    			if (find_str != string::npos){
    				currlat = prevpt.latitude;currlon = prevpt.longitude;
    				curralt = prevpt.alt_1;

        			tmpFp.route.erase(tmpFp.route.begin() + s);
        			s--;
    			}
    		}

    		real_t rt_dist = pt.rt_dist;
    		string recco_navaid = pt.recco_navaid;
    		NatsWaypoint tmpwp; tmpwp.name = recco_navaid;
    		vector<NatsWaypoint>::iterator itwp =
    				find(g_waypoints.begin(),g_waypoints.end(),tmpwp);
    		if (itwp != g_waypoints.end()){
    			real_t nav_lat = (real_t)itwp->latitude;
    			real_t nav_lon = (real_t)itwp->longitude;

    			get_dme_distance_point(model,&currcourse, &curralt,
    					&currlat, &currlon, &nav_lat, &nav_lon,
    					&rt_dist, &lat_int,&lon_int,&h_int);

    			PointWGS84 intpt;
    			intpt.latitude = lat_int;
    			intpt.longitude = lon_int;
    			intpt.path_n_terminator = pt.path_n_terminator;
    			intpt.alt_desc = pt.alt_desc;
    			intpt.alt_1 = h_int;
    			intpt.alt_2 = pt.alt_2;
    			intpt.speed_lim = pt.speed_lim;
    			intpt.wpname = "HEADING OR COURSE_TO_DME_DISTANCE_"+recco_navaid+"_"+currwpname;
    			intpt.procname = pt.procname;
    			intpt.proctype = pt.proctype;
    			intpt.recco_navaid=recco_navaid;
    			intpt.theta=pt.theta;
    			intpt.rho=pt.rho;
    			intpt.mag_course=pt.mag_course;
    			intpt.rt_dist=pt.rt_dist;
    			intpt.spdlim_desc = pt.spdlim_desc;
    			intpt.wp_cat =1;

    			if (s < tmpFp.route.size()-1){
    				vector<PointWGS84>::iterator itpt = tmpFp.route.begin()+s+1;
    				tmpFp.route.insert(itpt,intpt);

    				s++;
    			}
    		}
    	}
    	else if (pt.path_n_terminator == "CF" || pt.path_n_terminator =="DF") {
    		if (prev_pterm == "VI" || prev_pterm == "CI") {
        		if (s > 1 && s < tmpFp.route.size()){
        			PointWGS84 prevpt = tmpFp.route.at(s-2);
        			size_t find_str = prevpt.wpname.find("HEADING OR COURSE");
        			if (find_str != string::npos){
        				prevlat = prevpt.latitude;prevlon = prevpt.longitude;
        				prevalt = prevpt.alt_1;

            			tmpFp.route.erase(tmpFp.route.begin() + s-1);
            			s--;
        			}
        		}

    			get_intercept_point(&prevlat,&prevlon,&prevalt,&prevcourse,
							&currlat,&currlon,&curralt,&currcourse,
							&lat_int,&lon_int,&h_int);

    			PointWGS84 intpt;
    			intpt.latitude = lat_int;
    			intpt.longitude = lon_int;
    			intpt.path_n_terminator = pt.path_n_terminator;
    			intpt.alt_desc = pt.alt_desc;
    			intpt.alt_1 = h_int;
    			intpt.alt_2 = pt.alt_2;
    			intpt.speed_lim = pt.speed_lim;
    			intpt.wpname = "HEADING OR COURSE_TO_INTERCEPT_"+prevwpname+"_"+currwpname;
    			intpt.procname = pt.procname;
    			intpt.proctype = pt.proctype;
    			intpt.recco_navaid="";
    			intpt.theta=pt.theta;
    			intpt.rho=pt.rho;
    			intpt.mag_course=currcourse+mag_dec;
    			intpt.rt_dist=pt.rt_dist;
    			intpt.spdlim_desc = pt.spdlim_desc;
    			intpt.wp_cat =1;

    			vector<PointWGS84>::iterator itpt = tmpFp.route.begin()+s;
    			tmpFp.route.insert(itpt,intpt);

    			s++;
    		}
    	}

    	else if (pt.path_n_terminator == "VA" || pt.path_n_terminator =="CA") {
    		//TODO:ADD THIS
    		//FIXME: HIGHLY APPROXIMATE. TO BE FIXED LATER.
    		// REPLACE PREV WITH V_2 LOCATIONS HERE.

    		real_t v2_lat = currlat,v2_lon = currlon;
    		real_t ini_alt = get_airport_elevation(ap);

    		vector<AirportNode> allrunways =  getVector_AllRunways(ap);

    		for (size_t k = 0; k < allrunways.size(); ++k) {
				string str_refName2 = remove_white_spaces(allrunways.at(k).refName2);

    			if ((0 < allrunways.at(k).id.length())
    					&& ( strncmp(currwpname.c_str(),
    							str_refName2.c_str(),
								str_refName2.length()
								)
    							== 0)) {
    				v2_lat = allrunways.at(k).latitude;
    				v2_lon = allrunways.at(k).longitude;

    				break;
    			}
    		}

    		if (s > 0 && s < tmpFp.route.size()){
    			PointWGS84 prevpt = tmpFp.route.at(s-1);
    			size_t find_str = prevpt.wpname.find("HEADING OR COURSE");
    			if (find_str != string::npos){
    				ini_alt = prevpt.alt_1;
    				v2_lat = prevpt.latitude;
    				v2_lon = prevpt.longitude;
    				tmpFp.route.erase(tmpFp.route.begin() + s);
    				s--;
    			}
    		}

    		get_termination_pt(model,&ini_alt,&v2_lat,&v2_lon,
    				&curralt,&currcourse,&curr_spd_lim,
					&lat_int,&lon_int,&h_int);

    		//Now insert new point
    		PointWGS84 intpt;
    		intpt.latitude = lat_int;
    		intpt.longitude = lon_int;
    		intpt.path_n_terminator = pt.path_n_terminator;
    		intpt.alt_desc = pt.alt_desc;
    		intpt.alt_1 = h_int;
    		intpt.alt_2 = pt.alt_2;
    		intpt.speed_lim = pt.speed_lim;
    		intpt.wpname = "HEADING OR COURSE_TO_ALTITUDE_"+currwpname;
    		intpt.procname = pt.procname;
    		intpt.proctype = pt.proctype;
    		intpt.recco_navaid="";
    		intpt.theta=pt.theta;
    		intpt.rho=pt.rho;
    		intpt.mag_course=pt.mag_course;
    		intpt.rt_dist=pt.rt_dist;
    		intpt.spdlim_desc = pt.spdlim_desc;
    		intpt.wp_cat =1;

    		if (s < tmpFp.route.size()-1){
    			vector<PointWGS84>::iterator itpt = tmpFp.route.begin()+s+1;
    			tmpFp.route.insert(itpt,intpt);
    			s++;
    		}
    	}

    	else if (pt.path_n_terminator == "VR" || pt.path_n_terminator =="CR") {

    		//CHECK whether the VR leg is after a VA/VD etc leg
    		if (s > 0 && s < tmpFp.route.size()){
    			PointWGS84 prevpt = tmpFp.route.at(s-1);
    			size_t find_str = prevpt.wpname.find("HEADING OR COURSE");
    			if (find_str != string::npos){
    				currlat = prevpt.latitude;currlon = prevpt.longitude;
    				curralt = prevpt.alt_1;
    				currcourse = prevpt.mag_course-mag_dec;
    			}
    			tmpFp.route.erase(tmpFp.route.begin() + s);
    			s--;
    		}

    		string recco_navaid = pt.recco_navaid;
    		NatsWaypoint tmpwp; tmpwp.name = recco_navaid;
    		vector<NatsWaypoint>::iterator itwp =
    				find(g_waypoints.begin(),g_waypoints.end(),tmpwp);
    		if (itwp != g_waypoints.end()){

    			real_t navlat = itwp->latitude;
    			real_t navlon = itwp->longitude;
    			real_t navcourse = pt.theta -mag_dec;
    			real_t navalt = pt.alt_1;
    			if (navalt <=0){
    				navalt = curralt;
    			}
    			get_intercept_point(&navlat,&navlon,&navalt,&navcourse,
    			    						&currlat,&currlon,&curralt,&currcourse,
    										&lat_int,&lon_int,&h_int);

    			PointWGS84 intpt;
    			intpt.latitude = lat_int;
    			intpt.longitude = lon_int;
    			intpt.path_n_terminator = pt.path_n_terminator;
    			intpt.alt_desc = pt.alt_desc;
    			intpt.alt_1 = h_int;
    			intpt.alt_2 = pt.alt_2;
    			intpt.speed_lim = pt.speed_lim;
    			intpt.wpname = "HEADING OR COURSE_TO_RADIAL_"+recco_navaid+"_"+currwpname;
    			intpt.procname = pt.procname;
    			intpt.proctype = pt.proctype;
    			intpt.recco_navaid=recco_navaid;
    			intpt.theta=pt.theta;
    			intpt.rho=pt.rho;
    			intpt.mag_course=pt.mag_course;
    			intpt.rt_dist=pt.rt_dist;
    			intpt.spdlim_desc = pt.spdlim_desc;
    			intpt.wp_cat =1;

    			if (s < tmpFp.route.size()-1){
    				vector<PointWGS84>::iterator itpt = tmpFp.route.begin()+s+1;
    				tmpFp.route.insert(itpt,intpt);

    				s++;
    			}
    		}
    	}

    	prevlat = currlat; prevlon = currlon; prevalt = curralt;prevcourse = currcourse;
    	prev_spd_lim = curr_spd_lim;
    	prev_pterm = pt.path_n_terminator;
    	prevwpname = currwpname;

    	s++;
	} // end - while
	//ADDED HERE
	//PATH AND TERMINATOR CHECKS END HERE

	// ========================================================================
	//CHECKING IF FIRST POINT IS RUNWAY OR NOT.
	//IF THE INITIAL TARGET WAYPOINT IS A RUNWAY
	// IF IT IS NOT THEN PUT THE RUNWAY AND HEADING OR COURSE TO ALTITUDE NEXT.
	if(
			(tmpFp.initial_target.substr(0,2) == "RW"
					&& isInteger(tmpFp.initial_target.substr(2, 2)) )
			&&
			(tmpFp.route.front().wpname.substr(0,2) != "RW"
						|| !isInteger(tmpFp.route.front().wpname.substr(2, 2)) )

			){

		//FIRST ADD THE RUNWAY
		vector<AirportNode> allrunways =  getVector_AllRunways(tmpFp.origin);

		real_t lat_val =-10000.0, lon_val = -10000.0;
		size_t rwidx = 0;

		for (size_t rwc = 0; rwc < allrunways.size(); ++rwc){
			if (allrunways.at(rwc).refName1 == tmpFp.initial_target){
				lat_val = allrunways.at(rwc).latitude;
				lon_val = allrunways.at(rwc).longitude;
				rwidx = rwc;
				break;
			}
		}

		if (lat_val > -1000 && lon_val > -1000 ){
			PointWGS84 intpt;
			intpt.latitude = lat_val;
			intpt.longitude = lon_val;
			intpt.path_n_terminator = "VA";
			intpt.alt_desc = "";
			intpt.alt_1 = get_airport_elevation(tmpFp.origin);
			intpt.alt_2 = get_airport_elevation(tmpFp.origin);
			intpt.speed_lim = 0;
			intpt.wpname = tmpFp.initial_target+"-"+tmpFp.origin;
			intpt.procname = tmpFp.route.front().procname;
			intpt.proctype = tmpFp.route.front().proctype;
			intpt.recco_navaid="";
			intpt.theta=-10000;
			intpt.rho=-1000;
			intpt.mag_course= get_runway_hdg(tmpFp.origin,
					allrunways.at(rwidx).refName1,
					allrunways.at(rwidx).refName2) +orig_mag_var;
			intpt.rt_dist=-10000;
			intpt.spdlim_desc = "";
			intpt.wp_cat =1;

			vector<PointWGS84>::iterator itpt = tmpFp.route.begin();
			tmpFp.route.insert(itpt,intpt);

			//NEXT ADD THE VA POINT
			//TODO:ADD THIS
			//FIXME: HIGHLY APPROXIMATE. TO BE FIXED LATER.
			// REPLACE PREV WITH V_2 LOCATIONS HERE.

			vector<PointWGS84>::iterator itptv = tmpFp.route.begin();
			real_t v2_lat = itptv->latitude,v2_lon = itptv->longitude;
			real_t ini_alt = itptv->alt_1;currcourse = itptv->mag_course-orig_mag_var;
			curralt = ini_alt + 1000;curr_spd_lim = 0.0;
			currwpname = itptv->wpname;

			get_termination_pt(model,&ini_alt,&v2_lat,&v2_lon,
					&curralt,&currcourse,&curr_spd_lim,
					&lat_int,&lon_int,&h_int);

			intpt.latitude = lat_int;
			intpt.longitude = lon_int;
			intpt.path_n_terminator = itptv->path_n_terminator;
			intpt.alt_desc = itptv->alt_desc;
			intpt.alt_1 = h_int;
			intpt.alt_2 = itptv->alt_2;
			intpt.speed_lim = itptv->speed_lim;
			intpt.wpname = "HEADING OR COURSE_TO_ALTITUDE_"+currwpname;
			intpt.procname = itptv->procname;
			intpt.proctype = itptv->proctype;
			intpt.recco_navaid="";
			intpt.theta=itptv->theta;
			intpt.rho=itptv->rho;
			intpt.mag_course=itptv->mag_course;
			intpt.rt_dist=itptv->rt_dist;
			intpt.spdlim_desc = itptv->spdlim_desc;
			intpt.wp_cat =-1;


			itpt = tmpFp.route.begin()+1;
			tmpFp.route.insert(itpt,intpt);
		}

	}
	// ========================================================================

	// origin, cruise, dest altitudes
	real_t cruise_alt = tmpTrxRecord.cruiseAltitude;
	real_t origin_alt = get_airport_elevation(tmpFp.origin);
	real_t destination_alt = get_airport_elevation(tmpFp.destination);

	// Find the first STAR or APPROACH waypoint
	real_t first_STAR_or_APPROACH_waypoint_alt = destination_alt;
	int index_first_STAR_or_APPROACH_with_altitude = tmpFp.route.size() - 1;
	for (unsigned int i = 0; i < tmpFp.route.size(); i++) {
		if ((string("STAR").compare(tmpFp.route.at(i).proctype) == 0) || (string("APPROACH").compare(tmpFp.route.at(i).proctype) == 0)) {
			first_STAR_or_APPROACH_waypoint_alt = 0.0; // Reset

			if (tmpFp.route.at(i).alt_1 > 0) {
				first_STAR_or_APPROACH_waypoint_alt = tmpFp.route.at(i).alt_1;
			}
			if (tmpFp.route.at(i).alt_2 > 0) {
				if (first_STAR_or_APPROACH_waypoint_alt > 0) {
					first_STAR_or_APPROACH_waypoint_alt = (first_STAR_or_APPROACH_waypoint_alt + tmpFp.route.at(i).alt_2) / 2;
				} else {
					first_STAR_or_APPROACH_waypoint_alt = tmpFp.route.at(i).alt_2;
				}
			}

			if (first_STAR_or_APPROACH_waypoint_alt > 0) {
				index_first_STAR_or_APPROACH_with_altitude = i;

				break;
			}
		}
	}

	// Compute the descent distance and insert an artificial TOD waypoint
	// into the flight plan object
	real_t descent_dist = 0;
	if ((tmpFp.route.size() > 0) && (tmpFp.route[0].proctype != "APPROACH")) {
		descent_dist = compute_descent_dist(adb_table,
			first_STAR_or_APPROACH_waypoint_alt,
			cruise_alt);
	}

	// Compute the climb distance (from beginning of route) and insert an
	// artificial TOC waypoint into the flight plan object
	real_t climb_dist = 0;
	if ((tmpFp.route.size() > 0) && (tmpFp.route[0].proctype != "ENROUTE") && (tmpFp.route[0].proctype != "STAR") && (tmpFp.route[0].proctype != "APPROACH")) {
		climb_dist = compute_climb_dist(adb_table,
			                                  origin_alt,
			                                  cruise_alt);
	}

	// obtain the path length of the flight plan.
	// if the path length is shorter than the sum of climb and descent
	// distances then the flight will never reach cruise.  we need to
	// search for a new (lower) cruise altitude.
	real_t path_length = tmpFp.getPathLength();

	if (path_length < (descent_dist + climb_dist)) {
		// search for a lower cruise altitude, recompute
		// climb and descent distances
		vector<double>::const_iterator it;
		it = lower_bound(model->altitudes.begin(),
						 model->altitudes.end(),
						 tmpTrxRecord.cruiseAltitude);
		real_t new_cruise = (real_t)(*it);
		if (0 == new_cruise) {
			printf("    Can't calculate new cruise altitude.  Please check maximum flight level.");

			ignore_count++;

			return false;
		} else {
			int new_row = it - model->altitudes.begin();
			while (path_length < (descent_dist + climb_dist)) {
				descent_dist = compute_descent_dist(adb_table,
									first_STAR_or_APPROACH_waypoint_alt,
									new_cruise);
				climb_dist = compute_climb_dist(adb_table,
												   origin_alt,
												   new_cruise);
				new_row--;
				new_cruise = model->altitudes.at(new_row);
				if (new_cruise < MIN_ALTITUDE) {
					printf("Aircraft %s: Cruise altitude can't be lower than %f ft.\n", tmpTrxRecord.acid.c_str(), MIN_ALTITUDE);

					if (&lock)
						omp_set_lock(&lock);

					++ignore_count;

					if (&lock)
						omp_unset_lock(&lock);

					return false;
				}
			}

			cruise_alt = new_cruise;
		}
	}

	// compute the TOC and TOD and insert the artificial waypoints into
	// the flight plan object
	tmpFp.insertTopOfClimb(climb_dist);

	if (0 < climb_dist) {
		tmpFp.insertTopOfDescent(descent_dist, index_first_STAR_or_APPROACH_with_altitude+1); // Index + 1: Because TOC waypoint being inserted in front
	} else {
		tmpFp.insertTopOfDescent(descent_dist, index_first_STAR_or_APPROACH_with_altitude);
	}

	// set ADB table
	tmpFlight.adb_aircraft_type_index = adb_table;

	// set departure time, cruise alt
	tmpFlight.departure_time_sec = departure_time;
	tmpFlight.cruise_alt_ft = tmpTrxRecord.cruiseAltitude;

	// airport elevations
	tmpFlight.origin_airport_elevation_ft = get_airport_elevation(tmpFp.origin);
	tmpFlight.destination_airport_elevation_ft = get_airport_elevation(tmpFp.destination);

	// set current state, we init sector index to -1
	tmpFlight.sector_index = -1;
	tmpFlight.latitude_deg = tmpTrxRecord.latitude;
	tmpFlight.longitude_deg = tmpTrxRecord.longitude;
	tmpFlight.altitude_ft = tmpTrxRecord.altitude;
	tmpFlight.tas_knots = tmpTrxRecord.tas;
	tmpFlight.course_rad = tmpTrxRecord.heading * M_PI / 180.;

	tmpFlight.fpa_rad = 0;

	// compute the cruise tas
	tmpFlight.cruise_tas_knots = model->getCruiseTas(tmpTrxRecord.cruiseAltitude);

	if (cruise_perturbation != 0) {
		tmpFlight.cruise_tas_knots *= (1. - cruise_perturbation);
	}

	// make sure that the fp route length is smaller than max length -2
	// because we need to insert TOC and TOD. if the original flight plan
	// length exceeds the max length then drop waypoints from the
	// middle of the flight plan.
	if (tmpFp.route.size() > (MAX_FLIGHT_PLAN_LENGTH-2)) {
		int num_dropped = tmpFp.route.size() - MAX_FLIGHT_PLAN_LENGTH + 2;

		printf("Aircraft %s: %s flight plan length (%ld) exceeds "
				"maximum length of %d"
				"           Dropping %d waypoints from flight plan.\n", tmpTrxRecord.acid.c_str(), tmpFp.route.size(), MAX_FLIGHT_PLAN_LENGTH, num_dropped);

		int mid = tmpFp.route.size()/2;
		int start = mid - num_dropped/2;
		tmpFp.route.erase(tmpFp.route.begin()+start,
				       tmpFp.route.begin()+start+num_dropped);
	}

	// Traverse each element in tmpFp.route
	// Set values to tmpFlight
	for (unsigned int j=0; j<tmpFp.route.size(); ++j) {
		real_t lat = tmpFp.route.at(j).latitude;
		real_t lon = tmpFp.route.at(j).longitude;
		string wpname = tmpFp.route.at(j).wpname;
		string path_term = tmpFp.route.at(j).path_n_terminator;
		string alt_desc = tmpFp.route.at(j).alt_desc;
		real_t alt_1 = tmpFp.route.at(j).alt_1;
		real_t alt_2  = tmpFp.route.at(j).alt_2;
		real_t speed_lim = tmpFp.route.at(j).speed_lim;
		  //NEW ADDITIONS
		string procname = tmpFp.route.at(j).procname;
		string proctype = tmpFp.route.at(j).proctype;
		  //NEW ADDITIONS FOR NATS
		string recco_navaid = tmpFp.route.at(j).recco_navaid;
		real_t theta = tmpFp.route.at(j).theta;
		real_t rho = tmpFp.route.at(j).rho;
		real_t mag_course = tmpFp.route.at(j).mag_course;
		real_t rt_dist = tmpFp.route.at(j).rt_dist;
		string spdlim_desc = tmpFp.route.at(j).spdlim_desc;

		tmpFlight.flight_plan_latitude_deg[j] = lat;
		tmpFlight.flight_plan_longitude_deg[j] = lon;

		if (wpname.length() > 0) {
			tmpFlight.flight_plan_waypoint_name[j] = (char*)malloc((wpname.length()+1) * sizeof(char));
			strcpy(tmpFlight.flight_plan_waypoint_name[j], wpname.c_str());
			tmpFlight.flight_plan_waypoint_name[j][wpname.length()] = '\0';
		}
		else {
			tmpFlight.flight_plan_waypoint_name[j] = NULL;
		}

		if (path_term.length() > 0) {
			tmpFlight.flight_plan_path_term[j] = (char*)malloc((path_term.length()+1) * sizeof(char));
			strcpy(tmpFlight.flight_plan_path_term[j], path_term.c_str());
			tmpFlight.flight_plan_path_term[j][path_term.length()] = '\0';
		}
		else {
			tmpFlight.flight_plan_path_term[j] = NULL;
		}

		if (alt_desc.length() > 0) {
			tmpFlight.flight_plan_alt_desc[j] = (char*)malloc((alt_desc.length()+1) * sizeof(char));
			strcpy(tmpFlight.flight_plan_alt_desc[j], alt_desc.c_str());
			tmpFlight.flight_plan_alt_desc[j][alt_desc.length()] = '\0';
		}
		else {
			tmpFlight.flight_plan_alt_desc[j] = NULL;
		}

		tmpFlight.flight_plan_alt_1[j] = alt_1;
		tmpFlight.flight_plan_alt_2[j] = alt_2;
		tmpFlight.flight_plan_speed_lim[j] = speed_lim;

		if (procname.length() > 0) {
			tmpFlight.flight_plan_procname[j] = (char*)malloc((procname.length()+1) * sizeof(char));
			strcpy(tmpFlight.flight_plan_procname[j], procname.c_str());
			tmpFlight.flight_plan_procname[j][procname.length()] = '\0';
		}
		else {
			tmpFlight.flight_plan_procname[j] = NULL;
		}

		if (proctype.length() > 0) {
			tmpFlight.flight_plan_proctype[j] = (char*)malloc((proctype.length()+1) * sizeof(char));
			strcpy(tmpFlight.flight_plan_proctype[j], proctype.c_str());
			tmpFlight.flight_plan_proctype[j][proctype.length()] = '\0';
		}
		else {
			tmpFlight.flight_plan_proctype[j] = NULL;
		}

		if (recco_navaid.length() > 0) {
			tmpFlight.flight_plan_recco_navaid[j] = (char*)malloc((recco_navaid.length()+1) * sizeof(char));
			strcpy(tmpFlight.flight_plan_recco_navaid[j], recco_navaid.c_str());
			tmpFlight.flight_plan_recco_navaid[j][recco_navaid.length()] = '\0';
		}
		else {
			tmpFlight.flight_plan_recco_navaid[j] = NULL;
		}

		tmpFlight.flight_plan_theta[j] = theta;
		tmpFlight.flight_plan_rho[j] = rho;
		tmpFlight.flight_plan_mag_course[j] = mag_course;
		tmpFlight.flight_plan_rt_dist[j] = rt_dist;

		if (spdlim_desc.length() > 0) {
			tmpFlight.flight_plan_spdlim_desc[j] = (char*)malloc((spdlim_desc.length()+1) * sizeof(char));
			strcpy(tmpFlight.flight_plan_spdlim_desc[j], spdlim_desc.c_str());
			tmpFlight.flight_plan_spdlim_desc[j][spdlim_desc.length()] = '\0';
		}
		else {
			tmpFlight.flight_plan_spdlim_desc[j] = NULL;
		}
	}

	// Handle flight data(array index starts from tmpFp.route.size() to MAX_FLIGHT_PLAN_LENGTH)
	if (tmpFp.route.size() < MAX_FLIGHT_PLAN_LENGTH) {
		for (unsigned int j= tmpFp.route.size(); j < MAX_FLIGHT_PLAN_LENGTH; ++j) {
			tmpFlight.flight_plan_latitude_deg[j] = -999999.0;
			tmpFlight.flight_plan_longitude_deg[j] = -999999.0;

			tmpFlight.flight_plan_waypoint_name[j] = NULL;
			tmpFlight.flight_plan_path_term[j] = NULL;
			tmpFlight.flight_plan_alt_desc[j] = NULL;

			tmpFlight.flight_plan_alt_1[j] = -10000;
			tmpFlight.flight_plan_alt_2[j] = -10000;
			tmpFlight.flight_plan_speed_lim[j] = -10000;

			tmpFlight.flight_plan_procname[j] = NULL;
			tmpFlight.flight_plan_proctype[j] = NULL;
			tmpFlight.flight_plan_recco_navaid[j] = NULL;

			tmpFlight.flight_plan_theta[j] = -10000;
			tmpFlight.flight_plan_rho[j] = -10000;
			tmpFlight.flight_plan_mag_course[j] = -10000;
			tmpFlight.flight_plan_rt_dist[j] = -10000;

			tmpFlight.flight_plan_spdlim_desc[j] = NULL;
		}
	}

	// ========================================================================

	// Build waypoint node list of airborne flight plan
	bool result_build_waypointNodeList = buildAirborne_waypoint_node_linkedList(tmpFlight, tmpFp);

	if (!result_build_waypointNodeList) {
		return false;
	}

	// Build waypoint node list of departing and landing surface taxi plan
	result_build_waypointNodeList = buildSurface_waypoint_node_linkedList(tmpFlight,
			&tmpTrxRecord,
			&tmpFp);

	// If (departing runway name is not set) AND (initial_target contains "RW" but no "HEADING")
	if ((tmpFlight.departing_runway_name == NULL) && (tmpFp.initial_target.find("RW") != string::npos) && (tmpFp.initial_target.find("HEADING") == string::npos)) {
		tmpFlight.departing_runway_name = (char*)malloc((tmpFp.initial_target.length()+1) * sizeof(char));
		strcpy(tmpFlight.departing_runway_name, tmpFp.initial_target.c_str());
		tmpFlight.departing_runway_name[tmpFp.initial_target.length()] = '\0';
	}

	if (!result_build_waypointNodeList) {
		return false;
	}

	// ========================================================================

	ENUM_Flight_Phase phase = FLIGHT_PHASE_PREDEPARTURE; // Reset
	real_t hdot = 0;
	real_t target_alt = tmpTrxRecord.cruiseAltitude;
	waypoint_node_t* tmpTarget_waypoint_ptr = NULL;
	double dist_to_first_waypoint = 0;

	tmpFlight.target_waypoint_ptr = NULL; // Reset

	// Check departing taxi plan
	if (tmpFlight.departing_waypoint_node_ptr != NULL) {
		if ((tmpTrxRecord.altitude < 0)
					|| (tmpTrxRecord.altitude > tmpFlight.origin_airport_elevation_ft)) {
			printf("             Initial altitude %f can't work with departing taxi plan.  Please check flight plan.\n", tmpTrxRecord.altitude);

			ignore_count++;

			return false;
		} else {
			tmpTarget_waypoint_ptr = tmpFlight.departing_waypoint_node_ptr; // Default

			tmpFlight.target_waypoint_index = 0; // Default

			dist_to_first_waypoint = compute_distance_gc(tmpTrxRecord.latitude, tmpTrxRecord.longitude, tmpFlight.departing_waypoint_node_ptr->latitude, tmpFlight.departing_waypoint_node_ptr->longitude, 0, RADIUS_EARTH_FT);

			if (dist_to_first_waypoint <= 100) {
				tmpTarget_waypoint_ptr = tmpFlight.departing_waypoint_node_ptr->next_node_ptr;

				tmpFlight.target_waypoint_index = 1;

				tmpFlight.latitude_deg = tmpFlight.departing_waypoint_node_ptr->latitude; // Set first waypoint latitude to current location
				tmpFlight.longitude_deg = tmpFlight.departing_waypoint_node_ptr->longitude; // Set first waypoint longitude to current location
			}

			if ((tmpTarget_waypoint_ptr == tmpFlight.departing_waypoint_node_ptr->next_node_ptr)
					&& ((indexOf(tmpFlight.departing_waypoint_node_ptr->wpname, "Gate") > -1) || (indexOf(tmpFlight.departing_waypoint_node_ptr->wpname, "Parking") > -1))) {
				phase = FLIGHT_PHASE_ORIGIN_GATE;
			// If the initial location moves toward Gate
			} else if ((strncasecmp(tmpTarget_waypoint_ptr->wpname, "Gate", 4) == 0) || (strncasecmp(tmpTarget_waypoint_ptr->wpname, "Parking", 7) == 0)) {
				phase = FLIGHT_PHASE_TAXI_DEPARTING;
			} else if (strncasecmp(tmpTarget_waypoint_ptr->wpname, "Txy", 3) == 0) {
				phase = FLIGHT_PHASE_TAXI_DEPARTING;
			} else if (strncasecmp(tmpTarget_waypoint_ptr->wpname, "Ramp", 4) == 0) {
				phase = FLIGHT_PHASE_RAMP_DEPARTING;
			}
		}
	}

	if (tmpTarget_waypoint_ptr == NULL) {
		// Check airborne flight plan
		if (tmpFlight.airborne_waypoint_node_ptr != NULL) {
			// If first airborne waypoint is SID type
			if (indexOf(tmpFlight.airborne_waypoint_node_ptr->proctype, "SID") > -1) {
				if (tmpTrxRecord.altitude < tmpFlight.origin_airport_elevation_ft) {
					printf("             Error(SID): Initial altitude %f can't work with airborne flight plan. Airport altitude is %f.  Please check flight plan.\n",
							tmpTrxRecord.altitude,tmpFlight.origin_airport_elevation_ft);

					ignore_count++;

					return false;
				}

				tmpTarget_waypoint_ptr = tmpFlight.airborne_waypoint_node_ptr; // Default

				tmpFlight.target_waypoint_index = 0; // Default

				// If initial altitude is at the origin airport elevation
				if (tmpTrxRecord.altitude == tmpFlight.origin_airport_elevation_ft) {
					// If there is runway name specified.  This means that the aircraft starts on the runway
					if ((tmpFlight.departing_runway_name != NULL)
							&& (map_ground_waypoint_connectivity.find(tmpFp.origin) != map_ground_waypoint_connectivity.end())) {
						if (indexOf(tmpFlight.airborne_waypoint_node_ptr->wpname, tmpFlight.departing_runway_name) > -1) {
							tmpTarget_waypoint_ptr = tmpFlight.airborne_waypoint_node_ptr->next_node_ptr;

							tmpFlight.target_waypoint_index = 1;

							if ((tmpFlight.airborne_waypoint_node_ptr->next_node_ptr != NULL)
									&& (tmpFlight.airborne_waypoint_node_ptr->next_node_ptr->wpname != NULL)
									&& (indexOf(tmpFlight.airborne_waypoint_node_ptr->next_node_ptr->wpname, "HEADING") > -1)) {
								phase = FLIGHT_PHASE_TAKEOFF;
							} else {
								phase = FLIGHT_PHASE_CLIMBOUT;
							}
						} else {
							tmpTarget_waypoint_ptr = tmpFlight.airborne_waypoint_node_ptr;

							tmpFlight.target_waypoint_index = 0;

							phase = FLIGHT_PHASE_CLIMBOUT;
						}
					} else { // No runway name specified
						if (indexOf(tmpFlight.airborne_waypoint_node_ptr->wpname, "RW") > -1) {
							tmpTarget_waypoint_ptr = tmpFlight.airborne_waypoint_node_ptr->next_node_ptr;

							tmpFlight.target_waypoint_index = 1;
						} else {
							tmpTarget_waypoint_ptr = tmpFlight.airborne_waypoint_node_ptr;

							tmpFlight.target_waypoint_index = 0;
						}

						phase = FLIGHT_PHASE_CLIMBOUT;
					}
				} else if (tmpTrxRecord.altitude < TRACON_ALT_FT) {
					if (indexOf(tmpTarget_waypoint_ptr->wpname, "RW") == 0) {
						if (tmpFlight.airborne_waypoint_node_ptr->next_node_ptr != NULL) {
							tmpTarget_waypoint_ptr = tmpFlight.airborne_waypoint_node_ptr->next_node_ptr;

							tmpFlight.target_waypoint_index = 1;
						}
					}

					phase = FLIGHT_PHASE_CLIMBOUT;
				} else {
					phase = FLIGHT_PHASE_CLIMB_TO_CRUISE_ALTITUDE;
				}
			// If first airborne waypoint is ENROUTE type
			} else if (indexOf(tmpFlight.airborne_waypoint_node_ptr->proctype, "ENROUTE") > -1) {
				if ((tmpTrxRecord.altitude <= tmpFlight.origin_airport_elevation_ft)
						|| (tmpTrxRecord.altitude <= tmpFlight.destination_airport_elevation_ft)
						|| (tmpTrxRecord.altitude < TRACON_ALT_FT)) {
					printf("             Error(ENROUTE): Initial altitude %f can't work with airborne flight plan.  Please check flight plan.\n", tmpTrxRecord.altitude);

					ignore_count++;

					return false;
				}

				tmpTarget_waypoint_ptr = tmpFlight.airborne_waypoint_node_ptr; // Default

				tmpFlight.target_waypoint_index = 0; // Default

				if (tmpTrxRecord.altitude < tmpTrxRecord.cruiseAltitude) {
					phase = FLIGHT_PHASE_CLIMB_TO_CRUISE_ALTITUDE;
				} else {
					phase = FLIGHT_PHASE_CRUISE;
				}
			// If first airborne waypoint is STAR type
			} else if (indexOf(tmpFlight.airborne_waypoint_node_ptr->proctype, "STAR") > -1) {
				if ((tmpTrxRecord.altitude <= tmpFlight.destination_airport_elevation_ft)
						|| (tmpTrxRecord.altitude < TRACON_ALT_FT)) {
					printf("             Error(STAR): Initial altitude %f can't work with airborne flight plan.  Please check flight plan.\n", tmpTrxRecord.altitude);

					ignore_count++;

					return false;
				}

				tmpTarget_waypoint_ptr = tmpFlight.airborne_waypoint_node_ptr; // Default

				tmpFlight.target_waypoint_index = 0; // Default

				if (tmpTrxRecord.altitude == tmpTrxRecord.cruiseAltitude) {
					phase = FLIGHT_PHASE_CRUISE;
				} else if (tmpTrxRecord.altitude < TRACON_ALT_FT) {
					phase = FLIGHT_PHASE_APPROACH;
				} else {
					phase = FLIGHT_PHASE_INITIAL_DESCENT;
				}
			// If first airborne waypoint is APPROACH type
			} else if (indexOf(tmpFlight.airborne_waypoint_node_ptr->proctype, "APPROACH") > -1) {
				if (tmpTrxRecord.altitude < tmpFlight.destination_airport_elevation_ft) {
					printf("             Error(APPROACH): Initial altitude %f can't work with airborne flight plan.  Please check flight plan.\n", tmpTrxRecord.altitude);

					ignore_count++;

					return false;
				}

				if (tmpTrxRecord.altitude > tmpFlight.destination_airport_elevation_ft) {
					tmpTarget_waypoint_ptr = tmpFlight.airborne_waypoint_node_ptr; // Default

					tmpFlight.target_waypoint_index = 0; // Default

					phase = FLIGHT_PHASE_FINAL_APPROACH;
				}
			}
		}
	}

	if (tmpTarget_waypoint_ptr == NULL) {
		// Check landting taxi plan
		if (tmpFlight.landing_waypoint_node_ptr != NULL) {
			if (tmpTrxRecord.altitude < 0) {
				printf("             Error(Landing): Initial altitude %f can't work with landing taxi plan.  Please check flight plan.\n", tmpTrxRecord.altitude);

				ignore_count++;

				return false;
			} else {
				tmpTarget_waypoint_ptr = tmpFlight.landing_waypoint_node_ptr; // Default

				dist_to_first_waypoint = compute_distance_gc(tmpTrxRecord.latitude, tmpTrxRecord.longitude, tmpFlight.landing_waypoint_node_ptr->latitude, tmpFlight.landing_waypoint_node_ptr->longitude, 0, RADIUS_EARTH_FT);

				if (dist_to_first_waypoint <= 100) {
					tmpTarget_waypoint_ptr = tmpFlight.landing_waypoint_node_ptr->next_node_ptr;

					tmpFlight.latitude_deg = tmpFlight.landing_waypoint_node_ptr->latitude; // Set first waypoint latitude to current location
					tmpFlight.longitude_deg = tmpFlight.landing_waypoint_node_ptr->longitude; // Set first waypoint longitude to current location
				}

				if (strncasecmp(tmpTarget_waypoint_ptr->wpname, "Rwy", 3) == 0) {
					phase = FLIGHT_PHASE_LAND;
				} else if (strncasecmp(tmpTarget_waypoint_ptr->wpname, "Txy", 3) == 0) {
					phase = FLIGHT_PHASE_TAXI_ARRIVING;
				} else if ((strncasecmp(tmpTarget_waypoint_ptr->wpname, "Gate", 4) == 0) || (strncasecmp(tmpTarget_waypoint_ptr->wpname, "Ramp", 4) == 0)) {
					phase = FLIGHT_PHASE_RAMP_ARRIVING;
				}
			}
		}
	}

	tmpFlight.target_waypoint_ptr = tmpTarget_waypoint_ptr;

	tmpFlight.flight_phase = phase;
	tmpFlight.rocd_fps = hdot;
	tmpFlight.target_altitude_ft = target_alt;

	retValue = true;

	// Finally, set value
	if (retValue) {
		record = tmpTrxRecord;
		fp = tmpFp;
		flight = tmpFlight;
	}

	return retValue;
}

static bool compute_flight_data_geoStyle(real_t departure_time,
		TrxRecord& record,
        FlightPlan& fp,
		temp_flight_t& flight,
        const real_t& cruise_perturbation,
        omp_lock_t& lock) {
	bool retValue = false;

	printf("Aircraft: %s\n", record.acid.c_str());

	FlightPlanParser fp_parser;

	// Temp variable to store flight plan data.  If no error within this function, data of this temp variable will be copied to the formal variable.
	FlightPlan tmpFp = fp;

    if (!fp_parser.parse(record.acid,
    		record.route_str,
			tmpFp,
			record.altitude,
			record.cruiseAltitude)) {
    	ignore_count++;

    	return retValue;
    }

	// Temp variable to store TRX record data.  If no error within this function, data of this temp variable will be copied to the formal variable.
	TrxRecord tmpTrxRecord = record;

	// Temp variable to store flight data.  If no error within this function, data of this temp variable will be copied to the formal variable.
    temp_flight_t tmpFlight = flight;

	// ADB data
	string synonym = get_adb_synonym(tmpTrxRecord.actype);
	const int adb_table = get_adb_table_index(tmpTrxRecord.actype);

	if (adb_table < 0 || adb_table >= adb_ptf_type_num) {
		printf("Aircraft %s: Wrong aircraft type: %s\n", tmpTrxRecord.acid.c_str(), tmpTrxRecord.actype.c_str());

		ignore_count++;

		return false;
	}

	const AdbPTFModel model = g_adb_ptf_models.at(adb_table);

	// ========================================================================

	real_t cruise_alt = tmpTrxRecord.cruiseAltitude;

	real_t alt_last_waypoint_before_cruise = 0.0;
	real_t alt_first_waypoint_after_cruise = 0.0;

	int idx_last_waypoint_before_cruise = -1;
	int idx_first_waypoint_after_cruise = -1;

	if (tmpFp.route.size() > 2) {
		for (int i = 1; i < tmpFp.route.size(); i++) {
			if ((tmpFp.route.at(i-1).alt < cruise_alt) && (tmpFp.route.at(i).alt == cruise_alt)) {
				alt_last_waypoint_before_cruise = tmpFp.route.at(i-1).alt;
				idx_last_waypoint_before_cruise = i - 1;

				break;
			}
		}

		for (int i = 1; i < tmpFp.route.size(); i++) {
			if ((tmpFp.route.at(i-1).alt == cruise_alt) && (tmpFp.route.at(i).alt < cruise_alt)) {
				alt_first_waypoint_after_cruise = tmpFp.route.at(i).alt;
				idx_first_waypoint_after_cruise = i;

				break;
			}
		}
	}

	ENUM_Flight_Phase tmpWaypointPhase;

	string tmpString("FLIGHT_PHASE_");
	tmpString.append(tmpFp.route.at(0).phase);

	tmpWaypointPhase = getFlight_Phase(tmpString.c_str());

	// Compute the descent distance and insert an artificial TOD waypoint
	// into the flight plan object
	real_t descent_dist = 0;
	if ((tmpFp.route.size() > 0)
			&& !isFlightPhase_in_descending(tmpWaypointPhase)) {
		descent_dist = compute_descent_dist(adb_table,
				alt_first_waypoint_after_cruise,
				cruise_alt);
	}

	// Compute the climb distance (from beginning of route) and insert an
	// artificial TOC waypoint into the flight plan object
	real_t climb_dist = 0;
	if ((tmpFp.route.size() > 0)
			&& ((tmpWaypointPhase == FLIGHT_PHASE_TAKEOFF) || (isFlightPhase_in_climbing(tmpWaypointPhase)))) {
		climb_dist = compute_climb_dist(adb_table,
				alt_last_waypoint_before_cruise,
			    cruise_alt);
	}

	// obtain the path length of the flight plan.
	// if the path length is shorter than the sum of climb and descent
	// distances then the flight will never reach cruise.  we need to
	// search for a new (lower) cruise altitude.
	real_t path_length = tmpFp.getPathLength();

	if (path_length < (descent_dist + climb_dist)) {
		// search for a lower cruise altitude, recompute
		// climb and descent distances
		vector<double>::const_iterator it;
		it = lower_bound(model.altitudes.begin(),
						 model.altitudes.end(),
						 tmpTrxRecord.cruiseAltitude);
		real_t new_cruise = (real_t)(*it);
		if (0 == new_cruise) {
			printf("    Can't calculate new cruise altitude.  Please check maximum flight level.");

			ignore_count++;

			return false;
		} else {
			if (tmpFp.route.size() > 2) {
				for (int i = 1; i < tmpFp.route.size(); i++) {
					if ((tmpFp.route.at(i-1).alt < new_cruise) && (tmpFp.route.at(i).alt >= new_cruise)) {
						alt_last_waypoint_before_cruise = tmpFp.route.at(i-1).alt;
						idx_last_waypoint_before_cruise = i - 1;

						break;
					}
				}

				for (int i = 1; i < tmpFp.route.size(); i++) {
					if ((tmpFp.route.at(i-1).alt >= new_cruise) && (tmpFp.route.at(i).alt < new_cruise)) {
						alt_first_waypoint_after_cruise = tmpFp.route.at(i).alt;
						idx_first_waypoint_after_cruise = i;

						break;
					}
				}
			}

			int new_row = it - model.altitudes.begin();
			while (path_length < (descent_dist + climb_dist)) {
				descent_dist = compute_descent_dist(adb_table,
														alt_first_waypoint_after_cruise,
														new_cruise);
				climb_dist = compute_climb_dist(adb_table,
													alt_last_waypoint_before_cruise,
													new_cruise);
				new_row--;

				new_cruise = model.altitudes.at(new_row);
				if (new_cruise < MIN_ALTITUDE) {
					printf("Aircraft %s: Cruise altitude can't be lower than %f ft.\n", tmpTrxRecord.acid.c_str(), MIN_ALTITUDE);

					if (&lock)
						omp_set_lock(&lock);

					++ignore_count;

					if (&lock)
						omp_unset_lock(&lock);

					return false;
				}
			}

			cruise_alt = new_cruise;
		}
	}

	// Insert TOC waypoint
	if (-1 < idx_last_waypoint_before_cruise)
		tmpFp.insertTopOfClimb_geoStyle(climb_dist, cruise_alt, idx_last_waypoint_before_cruise);

	// Insert TOD waypoint
	if (-1 < idx_last_waypoint_before_cruise) {
		tmpFp.insertTopOfDescent_geoStyle(descent_dist, idx_first_waypoint_after_cruise+1); // +1 means adding one waypoint of TOC
	} else {
		tmpFp.insertTopOfDescent_geoStyle(descent_dist, idx_first_waypoint_after_cruise);
	}

	// set adb table
	tmpFlight.adb_aircraft_type_index = adb_table;

	// set departure time, cruise alt
	tmpFlight.departure_time_sec = departure_time;
	tmpFlight.cruise_alt_ft = tmpTrxRecord.cruiseAltitude;

	// airport elevations
	tmpFlight.origin_airport_elevation_ft = tmpFp.origin_altitude;
	tmpFlight.destination_airport_elevation_ft = tmpFp.destination_altitude;

	// set current state, we init sector index to -1
	tmpFlight.sector_index = -1;
	tmpFlight.latitude_deg = tmpTrxRecord.latitude;
	tmpFlight.longitude_deg = tmpTrxRecord.longitude;
	tmpFlight.altitude_ft = tmpTrxRecord.altitude;
	tmpFlight.tas_knots = tmpTrxRecord.tas;
	tmpFlight.course_rad = tmpTrxRecord.heading * M_PI / 180.;

	tmpFlight.fpa_rad = 0;

	// compute the cruise tas
	tmpFlight.cruise_tas_knots = model.getCruiseTas(tmpTrxRecord.cruiseAltitude);

	if (cruise_perturbation != 0) {
		tmpFlight.cruise_tas_knots *= (1. - cruise_perturbation);
	}

	// ========================================================================

	// Build waypoint node list of airborne flight plan
	bool result_build_waypointNodeList = buildAirborne_waypoint_node_linkedList_geoStyle(tmpFlight, &tmpFp);

	// Debug: Show all airborne waypoint
	if (tmpFlight.airborne_waypoint_node_ptr != NULL) {
		waypoint_node_t* tmp_wp_ptr = tmpFlight.airborne_waypoint_node_ptr;
		while (tmp_wp_ptr != NULL) {
			tmp_wp_ptr = tmp_wp_ptr->next_node_ptr; // Update pointer
		}
	}
	// end - Debug: Show all airborne waypoint

	if (!result_build_waypointNodeList) {
		return false;
	}

	// Build waypoint node list of departing and landing surface taxi plan
	result_build_waypointNodeList = buildSurface_waypoint_node_linkedList_geoStyle(tmpFlight,
			&tmpTrxRecord,
			&tmpFp);

	// Debug departing waypoint node list
	if (tmpFlight.departing_waypoint_node_ptr != NULL) {
		waypoint_node_t* tmp_wp_ptr = tmpFlight.departing_waypoint_node_ptr;

		while (tmp_wp_ptr != NULL) {
			tmp_wp_ptr = tmp_wp_ptr->next_node_ptr; // Update pointer
		}
	}

	// Debug landing waypoint node list
	if (tmpFlight.landing_waypoint_node_ptr != NULL) {
		waypoint_node_t* tmp_wp_ptr = tmpFlight.landing_waypoint_node_ptr;
		while (tmp_wp_ptr != NULL) {
			tmp_wp_ptr = tmp_wp_ptr->next_node_ptr; // Update pointer
		}
	}

	if (!result_build_waypointNodeList) {
		return false;
	}

	// ========================================================================

	ENUM_Flight_Phase phase = FLIGHT_PHASE_PREDEPARTURE; // Reset
	real_t hdot = 0;
	real_t target_alt = tmpTrxRecord.cruiseAltitude;
	waypoint_node_t* tmpTarget_waypoint_ptr = NULL;
	double dist_to_first_waypoint = 0;

	tmpFlight.target_waypoint_ptr = NULL; // Reset

	// Check departing taxi plan
	if (tmpFlight.departing_waypoint_node_ptr != NULL) {
		if ((tmpTrxRecord.altitude < 0)
					|| (tmpTrxRecord.altitude > tmpFlight.origin_airport_elevation_ft)) {
			printf("             Initial altitude %f can't work with departing taxi plan.  Please check flight plan.\n", tmpTrxRecord.altitude);

			ignore_count++;

			return false;
		} else {
			tmpTarget_waypoint_ptr = tmpFlight.departing_waypoint_node_ptr; // Default

			tmpFlight.target_waypoint_index = 0; // Default

			dist_to_first_waypoint = compute_distance_gc(tmpTrxRecord.latitude, tmpTrxRecord.longitude, tmpFlight.departing_waypoint_node_ptr->latitude, tmpFlight.departing_waypoint_node_ptr->longitude, 0, RADIUS_EARTH_FT);

			if (dist_to_first_waypoint <= 100) {
				tmpTarget_waypoint_ptr = tmpFlight.departing_waypoint_node_ptr->next_node_ptr;

				tmpFlight.target_waypoint_index = 1;

				tmpFlight.latitude_deg = tmpFlight.departing_waypoint_node_ptr->latitude; // Set first waypoint latitude to current location
				tmpFlight.longitude_deg = tmpFlight.departing_waypoint_node_ptr->longitude; // Set first waypoint longitude to current location
			}

			if ((tmpTarget_waypoint_ptr == tmpFlight.departing_waypoint_node_ptr->next_node_ptr)
					&& ((indexOf(tmpFlight.departing_waypoint_node_ptr->wptype, "Gate") > -1) || (indexOf(tmpFlight.departing_waypoint_node_ptr->wptype, "Parking") > -1))) {
				phase = FLIGHT_PHASE_ORIGIN_GATE;
			// If the initial location moves toward Gate
			} else if ((strncasecmp(tmpTarget_waypoint_ptr->wptype, "Gate", 4) == 0) || (strncasecmp(tmpTarget_waypoint_ptr->wptype, "Parking", 7) == 0)) {
				phase = FLIGHT_PHASE_TAXI_DEPARTING;
			} else if (strncasecmp(tmpTarget_waypoint_ptr->wptype, "Taxiway", 7) == 0) {
				phase = FLIGHT_PHASE_TAXI_DEPARTING;
			} else if (strncasecmp(tmpTarget_waypoint_ptr->wptype, "Ramp", 4) == 0) {
				phase = FLIGHT_PHASE_RAMP_DEPARTING;
			}
		}
	}

	if (tmpTarget_waypoint_ptr == NULL) {
		// Check airborne flight plan
		if (tmpFlight.airborne_waypoint_node_ptr != NULL) {
			tmpString.assign("FLIGHT_PHASE_");
			tmpString.append(tmpFlight.airborne_waypoint_node_ptr->phase);

			phase = getFlight_Phase(tmpString.c_str());

			tmpTarget_waypoint_ptr = tmpFlight.airborne_waypoint_node_ptr->next_node_ptr;
		}
	}

	if (tmpTarget_waypoint_ptr == NULL) {
		// Check landting taxi plan
		if (tmpFlight.landing_waypoint_node_ptr != NULL) {
			if (tmpTrxRecord.altitude < 0) {
				printf("             Error(Landing): Initial altitude %f can't work with landing taxi plan.  Please check flight plan.\n", tmpTrxRecord.altitude);

				ignore_count++;

				return false;
			} else {
				tmpTarget_waypoint_ptr = tmpFlight.landing_waypoint_node_ptr; // Default

				dist_to_first_waypoint = compute_distance_gc(tmpTrxRecord.latitude, tmpTrxRecord.longitude, tmpFlight.landing_waypoint_node_ptr->latitude, tmpFlight.landing_waypoint_node_ptr->longitude, 0, RADIUS_EARTH_FT);

				if (dist_to_first_waypoint <= 100) {
					tmpTarget_waypoint_ptr = tmpFlight.landing_waypoint_node_ptr->next_node_ptr;

					tmpFlight.latitude_deg = tmpFlight.landing_waypoint_node_ptr->latitude; // Set first waypoint latitude to current location
					tmpFlight.longitude_deg = tmpFlight.landing_waypoint_node_ptr->longitude; // Set first waypoint longitude to current location
				}

				if (strncasecmp(tmpTarget_waypoint_ptr->wptype, "Runway", 6) == 0) {
					phase = FLIGHT_PHASE_LAND;
				} else if (strncasecmp(tmpTarget_waypoint_ptr->wptype, "Taxiway", 7) == 0) {
					phase = FLIGHT_PHASE_TAXI_ARRIVING;
				} else if ((strncasecmp(tmpTarget_waypoint_ptr->wptype, "Gate", 4) == 0) || (strncasecmp(tmpTarget_waypoint_ptr->wptype, "Ramp", 4) == 0)) {
					phase = FLIGHT_PHASE_RAMP_ARRIVING;
				}
			}
		}
	}

	tmpFlight.target_waypoint_ptr = tmpTarget_waypoint_ptr;

	tmpFlight.flight_phase = phase;
	tmpFlight.rocd_fps = hdot;
	tmpFlight.target_altitude_ft = target_alt;

	retValue = true;

	// Finally, set value
	if (retValue) {
		record = tmpTrxRecord;
		fp = tmpFp;
		flight = tmpFlight;
	}

	return retValue;
}

static int malloc_host(void** ptr, size_t bytes) {
	*ptr = malloc(bytes);
	if (!(*ptr)) {
		printf("Error allocating host array.\n");
		*ptr = NULL;
		return -1;
	}
	memset(*ptr, 0, bytes);
	return 0;
}

static int malloc_host_char_array(char*** ptr,size_t bytes){
	*ptr = (char **)malloc(bytes);
	if (!(*ptr)) {
		printf("Error allocating host array.\n");
		*ptr = NULL;
		return -1;
	}
	memset(*ptr, ' ', bytes);
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

	return 0;
}

static bool flight_index_comparator(const Trajectory& a, const Trajectory& b) {
	return a.flight_index < b.flight_index;
}

static bool temp_flight_key_comparator(const temp_flight_t& a, const temp_flight_t& b) {
	return a.key < b.key;
}

void initialize_temp_flight_t(temp_flight_t& flight) {
	for (int i = 0; i < MAX_FLIGHT_PLAN_LENGTH; i++) {
		flight.flight_plan_waypoint_name[i] = NULL;
		flight.flight_plan_path_term[i] = NULL;
		flight.flight_plan_alt_desc[i] = NULL;
		flight.flight_plan_procname[i] = NULL;
		flight.flight_plan_proctype[i] = NULL;
		flight.flight_plan_recco_navaid[i] = NULL;
		flight.flight_plan_spdlim_desc[i] = NULL;
		flight.departing_runway_name = NULL;
		flight.landing_runway_name = NULL;
	}
}

int load_aircraft(const string& trx_file, const string& mfl_file, const real_t& cruise_perturbation)
{
	ignore_count = 0; // Reset
	filter_count = 0; // Reset

	printf("  Loading flight data\n");

	// load the TRX data
	int err = 0;
	TrxHandler handler;
	TrxInputStream in(trx_file, mfl_file);
	in.addTrxInputStreamListener(&handler);
	err = in.parse();
	if (err) {
		printf("Error loading TRX file\n");
		return err;
	}

	// ========================================================================

	long start_time = handler.getStartTime();
	g_start_time = start_time;

	std::map<int, FlightPlan> tmp_flightplans;
	std::map<int, TrxRecord> tmp_records;

	g_trx_records.clear();
	g_flightplans.clear();
	g_trajectories.clear();

	vector<temp_flight_t> tmp_flights;
	tmp_flights.clear();

	// compute flight data for trx records.  we need to compute the data
	// into temporary flight structs because some may be GA flights
	// that we can't compute valid cruise altitudes for.
omp_lock_t lock;
omp_init_lock(&lock);

	string tmpJsonStr;
	string tmp_runway_str;

	int tmpPos_parenthesis_L;
	int tmpPos_parenthesis_R;

	std::string tmp_wp_name;

	for (unsigned int i = 0; i < handler.getRecords().size(); ++i) {
		TrxRecord* record = const_cast<TrxRecord*>(&(handler.getRecords().at(i)));

		long departure_time = record->timestamp - start_time;
		FlightPlan fp;
		temp_flight_t tmp_flight;
		initialize_temp_flight_t(tmp_flight);

		bool result = false;
		if (record->flag_geoStyle) {
			result = compute_flight_data_geoStyle(departure_time,
				*record,
				fp,
				tmp_flight,
				cruise_perturbation,
			    lock);
		} else {
			if (flag_SidStarApp_available) {
				result = compute_flight_data(departure_time,
							*record,
							fp,
							tmp_flight,
							cruise_perturbation,
							lock);
			} else {
				ignore_count++;
			}
		}

		if (result) {
			tmp_flight.key = i;

			tmp_flights.push_back(tmp_flight);

			tmp_flightplans.insert(pair<int, FlightPlan>(i, fp));

			tmp_records.insert(pair<int, TrxRecord>(i, *record));
		}
	}
omp_destroy_lock(&lock);

	num_flights = tmp_flights.size();

	// array sizes for the aircraft soa
	size_t char_array_size = num_flights*sizeof(char);
	size_t real_array_size = num_flights*sizeof(real_t);
	size_t int_array_size = num_flights*sizeof(int);
	size_t bool_array_size = num_flights*sizeof(bool);
	size_t flight_phase_array_size = num_flights*sizeof(ENUM_Flight_Phase);
	size_t fp_array_size = num_flights*MAX_FLIGHT_PLAN_LENGTH*sizeof(real_t);
	size_t fp_char_array_size = num_flights*MAX_FLIGHT_PLAN_LENGTH*sizeof(char*);
	size_t fp_double_array_size = num_flights*MAX_FLIGHT_PLAN_LENGTH*sizeof(double);

	malloc_host((void**)&h_aircraft_soa.flag_geoStyle, bool_array_size);
	cuda_malloc_host((void**)&h_aircraft_soa.sector_index, int_array_size);
	cuda_malloc_host((void**)&h_aircraft_soa.latitude_deg, real_array_size);
	cuda_malloc_host((void**)&h_aircraft_soa.longitude_deg, real_array_size);
	cuda_malloc_host((void**)&h_aircraft_soa.altitude_ft, real_array_size);
	cuda_malloc_host((void**)&h_aircraft_soa.rocd_fps, real_array_size);
	cuda_malloc_host((void**)&h_aircraft_soa.tas_knots, real_array_size);
	cuda_malloc_host((void**)&h_aircraft_soa.tas_knots_ground, real_array_size);
	cuda_malloc_host((void**)&h_aircraft_soa.course_rad, real_array_size);
	cuda_malloc_host((void**)&h_aircraft_soa.fpa_rad, real_array_size);
	cuda_malloc_host((void**)&h_aircraft_soa.flight_phase, flight_phase_array_size);
	malloc_host((void**)&h_aircraft_soa.departure_time_sec, real_array_size);
	malloc_host((void**)&h_aircraft_soa.cruise_alt_ft, real_array_size);
	malloc_host((void**)&h_aircraft_soa.cruise_tas_knots, real_array_size);

	cuda_malloc_host((void**)&h_aircraft_soa.latitude_deg_pre_pause, real_array_size);
	cuda_malloc_host((void**)&h_aircraft_soa.longitude_deg_pre_pause, real_array_size);
	cuda_malloc_host((void**)&h_aircraft_soa.altitude_ft_pre_pause, real_array_size);
	cuda_malloc_host((void**)&h_aircraft_soa.rocd_fps_pre_pause, real_array_size);
	cuda_malloc_host((void**)&h_aircraft_soa.tas_knots_pre_pause, real_array_size);
	cuda_malloc_host((void**)&h_aircraft_soa.course_rad_pre_pause, real_array_size);
	cuda_malloc_host((void**)&h_aircraft_soa.fpa_rad_pre_pause, real_array_size);
	cuda_malloc_host((void**)&h_aircraft_soa.cruise_alt_ft_pre_pause, real_array_size);
	cuda_malloc_host((void**)&h_aircraft_soa.cruise_tas_knots_pre_pause, real_array_size);

	malloc_host((void**)&h_aircraft_soa.origin_airport_elevation_ft, real_array_size);
	malloc_host((void**)&h_aircraft_soa.destination_airport_elevation_ft, real_array_size);

	malloc_host((void**)&h_aircraft_soa.landed_flag, int_array_size);
	malloc_host((void**)&h_aircraft_soa.adb_aircraft_type_index, int_array_size);
	malloc_host((void**)&h_aircraft_soa.holding_started, bool_array_size);
	malloc_host((void**)&h_aircraft_soa.holding_stopped, bool_array_size);
	malloc_host((void**)&h_aircraft_soa.has_holding_pattern, bool_array_size);
	malloc_host((void**)&h_aircraft_soa.hold_start_index, int_array_size);
	malloc_host((void**)&h_aircraft_soa.hold_end_index, int_array_size);
	malloc_host((void**)&h_aircraft_soa.holding_tas_knots, real_array_size);
	malloc_host((void**)&h_aircraft_soa.target_waypoint_index, int_array_size);
	malloc_host((void**)&h_aircraft_soa.target_altitude_ft, real_array_size);
	malloc_host((void**)&h_aircraft_soa.toc_index, int_array_size);
	malloc_host((void**)&h_aircraft_soa.tod_index, int_array_size);

	// Allocate memory space
	array_Airborne_Flight_Plan_ptr = (waypoint_node_t**)calloc(num_flights, sizeof(waypoint_node_t*));
	array_Airborne_Flight_Plan_toc_ptr = (waypoint_node_t**)calloc(num_flights, sizeof(waypoint_node_t*));
	array_Airborne_Flight_Plan_tod_ptr = (waypoint_node_t**)calloc(num_flights, sizeof(waypoint_node_t*));
	array_Airborne_Flight_Plan_Final_Node_ptr = (waypoint_node_t**)calloc(num_flights, sizeof(waypoint_node_t*));
	array_Airborne_Flight_Plan_Waypoint_length = (int*)calloc(num_flights, sizeof(int));

	h_aircraft_soa.runway_name_departing = (char**)malloc(num_flights*sizeof(char*));
	h_aircraft_soa.runway_name_landing = (char**)malloc(num_flights*sizeof(char*));

	// Allocate memory space
	h_departing_taxi_plan.airport_code = (char**)calloc(num_flights, sizeof(char*));
	h_departing_taxi_plan.waypoint_node_ptr = (waypoint_node_t**)calloc(num_flights, sizeof(waypoint_node_t*));
	h_departing_taxi_plan.waypoint_final_node_ptr = (waypoint_node_t**)calloc(num_flights, sizeof(waypoint_node_t*));
	h_departing_taxi_plan.waypoint_length = (int*)malloc(int_array_size);
	h_departing_taxi_plan.runway_name = (char**)malloc(num_flights * sizeof(char*));
	h_departing_taxi_plan.taxi_tas_knots = (double*)malloc(num_flights * sizeof(double));
	h_departing_taxi_plan.ramp_tas_knots = (double*)malloc(num_flights * sizeof(double));
	h_departing_taxi_plan.runway_entry_latitude_geoStyle = (double*)malloc(num_flights * sizeof(double));
	h_departing_taxi_plan.runway_entry_longitude_geoStyle = (double*)malloc(num_flights * sizeof(double));
	h_departing_taxi_plan.runway_end_latitude_geoStyle = (double*)malloc(num_flights * sizeof(double));
	h_departing_taxi_plan.runway_end_longitude_geoStyle = (double*)malloc(num_flights * sizeof(double));

	// Allocate memory space
	h_landing_taxi_plan.airport_code = (char**)calloc(num_flights, sizeof(char*));
	h_landing_taxi_plan.waypoint_node_ptr = (waypoint_node_t**)calloc(num_flights, sizeof(waypoint_node_t*));
	h_landing_taxi_plan.waypoint_final_node_ptr = (waypoint_node_t**)calloc(num_flights, sizeof(waypoint_node_t*));
	h_landing_taxi_plan.waypoint_length = (int*)malloc(int_array_size);
	h_landing_taxi_plan.runway_name = (char**)malloc(num_flights * sizeof(char*));
	h_landing_taxi_plan.taxi_tas_knots = (double*)malloc(num_flights * sizeof(double));
	h_landing_taxi_plan.ramp_tas_knots = (double*)malloc(num_flights * sizeof(double));
	h_landing_taxi_plan.runway_entry_latitude_geoStyle = (double*)malloc(num_flights * sizeof(double));
	h_landing_taxi_plan.runway_entry_longitude_geoStyle = (double*)malloc(num_flights * sizeof(double));
	h_landing_taxi_plan.runway_end_latitude_geoStyle = (double*)malloc(num_flights * sizeof(double));
	h_landing_taxi_plan.runway_end_longitude_geoStyle = (double*)malloc(num_flights * sizeof(double));

	h_aircraft_soa.target_waypoint_node_ptr = (waypoint_node_t**)calloc(num_flights, sizeof(waypoint_node_t*));
	h_aircraft_soa.flag_target_waypoint_change = (bool*)calloc(num_flights, sizeof(bool));
	h_aircraft_soa.last_WaypointNode_ptr = (waypoint_node_t**)calloc(num_flights, sizeof(waypoint_node_t*));
	h_aircraft_soa.flag_reached_meterfix_point = (bool*)calloc(num_flights, sizeof(bool));

	// Initialize value
	for (int i = 0; i < num_flights; i++) {
		int flight_key = tmp_flights[i].key;

		h_departing_taxi_plan.runway_name[i] = NULL; // Reset
		h_landing_taxi_plan.runway_name[i] = NULL; // Reset

		h_aircraft_soa.flag_geoStyle[i] = tmp_records.at(i).flag_geoStyle;

		array_Airborne_Flight_Plan_ptr[i] = tmp_flights[i].airborne_waypoint_node_ptr;
		array_Airborne_Flight_Plan_toc_ptr[i] = tmp_flights[i].airborne_toc_waypoint_node_ptr;
		array_Airborne_Flight_Plan_tod_ptr[i] = tmp_flights[i].airborne_tod_waypoint_node_ptr;
		array_Airborne_Flight_Plan_Final_Node_ptr[i] = tmp_flights[i].airborne_final_waypoint_node_ptr;
		array_Airborne_Flight_Plan_Waypoint_length[i] = tmp_flights[i].airborne_waypoint_length;

		h_departing_taxi_plan.airport_code[i] = (char*)calloc(tmp_flightplans.at(flight_key).origin.length()+1, sizeof(char));
		strcpy(h_departing_taxi_plan.airport_code[i], tmp_flightplans.at(flight_key).origin.c_str());
		h_departing_taxi_plan.airport_code[i][tmp_flightplans.at(flight_key).origin.length()] = '\0';

		if ((tmp_flights[i].departing_runway_name != NULL) && (sizeof(tmp_flights[i].departing_runway_name) > 0)) {
			h_departing_taxi_plan.runway_name[i] = (char*)calloc(sizeof(tmp_flights[i].departing_runway_name), sizeof(char));
			strcpy(h_departing_taxi_plan.runway_name[i], tmp_flights[i].departing_runway_name); // '\0' already existed at the end
		}

		h_departing_taxi_plan.taxi_tas_knots[i] = DEFAULT_TAXI_TAS_KNOTS;
		h_departing_taxi_plan.ramp_tas_knots[i] = DEFAULT_RAMP_TAS_KNOTS;

		h_departing_taxi_plan.waypoint_node_ptr[i] = tmp_flights[i].departing_waypoint_node_ptr;
		h_departing_taxi_plan.waypoint_final_node_ptr[i] = tmp_flights[i].departing_final_waypoint_node_ptr;
		h_departing_taxi_plan.waypoint_length[i] = tmp_flights[i].departing_waypoint_length;

		if (h_aircraft_soa.flag_geoStyle[i]) {
			h_departing_taxi_plan.runway_entry_latitude_geoStyle[i] = DBL_MAX; // Default
			h_departing_taxi_plan.runway_entry_longitude_geoStyle[i] = DBL_MAX; // Default
			h_departing_taxi_plan.runway_end_latitude_geoStyle[i] = DBL_MAX; // Default
			h_departing_taxi_plan.runway_end_longitude_geoStyle[i] = DBL_MAX; // Default

			if (0 < tmp_flightplans.at(i).departing_runwayString.length()) {
				tmp_runway_str.assign(tmp_flightplans.at(i).departing_runwayString);

				// Process departing runway entry point

				tmpPos_parenthesis_L = tmp_runway_str.find_first_of("{");
				tmpPos_parenthesis_R = tmp_runway_str.find_first_of("}");
				if ((tmpPos_parenthesis_L < 0) || (tmpPos_parenthesis_R < 0)) {
					printf("             Departing runway is not valid.\n");

					return false;
				}

				json jsonObj;

				tmpJsonStr = tmp_runway_str.substr(tmpPos_parenthesis_L, (tmpPos_parenthesis_R - tmpPos_parenthesis_L + 1));

				jsonObj = json::parse(tmpJsonStr);

				if (jsonObj.contains("lat")) {
					h_departing_taxi_plan.runway_entry_latitude_geoStyle[i] = convertLatLonString_to_deg(jsonObj.at("lat").get<string>().c_str());
				}
				if (jsonObj.contains("lon")) {
					h_departing_taxi_plan.runway_entry_longitude_geoStyle[i] = convertLatLonString_to_deg(jsonObj.at("lon").get<string>().c_str());
				}

				if (tmp_runway_str.find_first_of(",")+1 > 0) {
					tmp_runway_str.erase(0, tmp_runway_str.find_first_of(",")+1);
				}

				// Process departing runway end point

				tmpPos_parenthesis_L = tmp_runway_str.find_first_of("{");
				tmpPos_parenthesis_R = tmp_runway_str.find_first_of("}");
				if ((tmpPos_parenthesis_L < 0) || (tmpPos_parenthesis_R < 0)) {
					printf("             Departing runway is not valid.\n");

					return false;
				}

				tmpJsonStr = tmp_runway_str.substr(tmpPos_parenthesis_L, (tmpPos_parenthesis_R - tmpPos_parenthesis_L + 1));

				jsonObj = json::parse(tmpJsonStr);

				if (jsonObj.contains("lat")) {
					h_departing_taxi_plan.runway_end_latitude_geoStyle[i] = convertLatLonString_to_deg(jsonObj.at("lat").get<string>().c_str());
				}
				if (jsonObj.contains("lon")) {
					h_departing_taxi_plan.runway_end_longitude_geoStyle[i] = convertLatLonString_to_deg(jsonObj.at("lon").get<string>().c_str());
				}
			}
		}

		h_landing_taxi_plan.airport_code[i] = (char*)calloc(tmp_flightplans.at(flight_key).destination.length()+1, sizeof(char));
		strcpy(h_landing_taxi_plan.airport_code[i], tmp_flightplans.at(flight_key).destination.c_str());
		h_landing_taxi_plan.airport_code[i][tmp_flightplans.at(flight_key).destination.length()] = '\0';

		if ((tmp_flights[i].landing_runway_name != NULL) && (sizeof(tmp_flights[i].landing_runway_name) > 0)) {
			h_landing_taxi_plan.runway_name[i] = (char*)calloc(sizeof(tmp_flights[i].landing_runway_name), sizeof(char));
			strcpy(h_landing_taxi_plan.runway_name[i], tmp_flights[i].landing_runway_name); // '\0' already existed at the end
		}

		h_landing_taxi_plan.taxi_tas_knots[i] = DEFAULT_TAXI_TAS_KNOTS;
		h_landing_taxi_plan.ramp_tas_knots[i] = DEFAULT_RAMP_TAS_KNOTS;

		h_landing_taxi_plan.waypoint_node_ptr[i] = tmp_flights[i].landing_waypoint_node_ptr;
		h_landing_taxi_plan.waypoint_final_node_ptr[i] = tmp_flights[i].landing_final_waypoint_node_ptr;
		h_landing_taxi_plan.waypoint_length[i] = tmp_flights[i].landing_waypoint_length;

		if (h_aircraft_soa.flag_geoStyle[i]) {
			h_landing_taxi_plan.runway_entry_latitude_geoStyle[i] = DBL_MAX; // Default
			h_landing_taxi_plan.runway_entry_longitude_geoStyle[i] = DBL_MAX; // Default
			h_landing_taxi_plan.runway_end_latitude_geoStyle[i] = DBL_MAX; // Default
			h_landing_taxi_plan.runway_end_longitude_geoStyle[i] = DBL_MAX; // Default

			if (0 < tmp_flightplans.at(i).landing_runwayString.length()) {
				tmp_runway_str.assign(tmp_flightplans.at(i).landing_runwayString);

				json jsonObj;

				// Process landing runway entry point

				tmpPos_parenthesis_L = tmp_runway_str.find_first_of("{");
				tmpPos_parenthesis_R = tmp_runway_str.find_first_of("}");
				if ((tmpPos_parenthesis_L < 0) || (tmpPos_parenthesis_R < 0)) {
					printf("             Landing runway is not valid.\n");

					return false;
				}

				tmpJsonStr = tmp_runway_str.substr(tmpPos_parenthesis_L, (tmpPos_parenthesis_R - tmpPos_parenthesis_L + 1));

				jsonObj = json::parse(tmpJsonStr);

				if (jsonObj.contains("lat")) {
					h_landing_taxi_plan.runway_entry_latitude_geoStyle[i] = convertLatLonString_to_deg(jsonObj.at("lat").get<string>().c_str());
				}
				if (jsonObj.contains("lon")) {
					h_landing_taxi_plan.runway_entry_longitude_geoStyle[i] = convertLatLonString_to_deg(jsonObj.at("lon").get<string>().c_str());
				}

				if (tmp_runway_str.find_first_of(",")+1 > 0) {
					tmp_runway_str.erase(0, tmp_runway_str.find_first_of(",")+1);
				}

				// Process landing runway end point

				tmpPos_parenthesis_L = tmp_runway_str.find_first_of("{");
				tmpPos_parenthesis_R = tmp_runway_str.find_first_of("}");
				if ((tmpPos_parenthesis_L < 0) || (tmpPos_parenthesis_R < 0)) {
					printf("             Landing runway is not valid.\n");

					return false;
				}

				tmpJsonStr = tmp_runway_str.substr(tmpPos_parenthesis_L, (tmpPos_parenthesis_R - tmpPos_parenthesis_L + 1));

				jsonObj = json::parse(tmpJsonStr);

				if (jsonObj.contains("lat")) {
					h_landing_taxi_plan.runway_end_latitude_geoStyle[i] = convertLatLonString_to_deg(jsonObj.at("lat").get<string>().c_str());
				}
				if (jsonObj.contains("lon")) {
					h_landing_taxi_plan.runway_end_longitude_geoStyle[i] = convertLatLonString_to_deg(jsonObj.at("lon").get<string>().c_str());
				}
			}
		}
	}

	// ========================================================================

	// iterate over the temp flights and copy the data from the AOS to the SOA
	// we can do this in a parallel for loop.

	// Assign h_aircraft_soa value
	for (int i = 0; i < num_flights; i++) {
		h_aircraft_soa.runway_name_departing[i] = NULL; // Reset
		h_aircraft_soa.runway_name_landing[i] = NULL; // Reset

		h_aircraft_soa.sector_index[i] = tmp_flights[i].sector_index;
		h_aircraft_soa.latitude_deg[i] = tmp_flights[i].latitude_deg;
		h_aircraft_soa.longitude_deg[i] = tmp_flights[i].longitude_deg;
		h_aircraft_soa.altitude_ft[i] = tmp_flights[i].altitude_ft;
		h_aircraft_soa.rocd_fps[i] = tmp_flights[i].rocd_fps;
		h_aircraft_soa.tas_knots[i] = tmp_flights[i].tas_knots;
		h_aircraft_soa.course_rad[i] = tmp_flights[i].course_rad;
		h_aircraft_soa.fpa_rad[i] = tmp_flights[i].fpa_rad;
		h_aircraft_soa.flight_phase[i] = tmp_flights[i].flight_phase;
		h_aircraft_soa.departure_time_sec[i] = tmp_flights[i].departure_time_sec;
		h_aircraft_soa.cruise_alt_ft[i] = tmp_flights[i].cruise_alt_ft;
		h_aircraft_soa.cruise_tas_knots[i] = tmp_flights[i].cruise_tas_knots;

		if ((tmp_flights[i].departing_runway_name != NULL) && (sizeof(tmp_flights[i].departing_runway_name) > 0)) {
			h_aircraft_soa.runway_name_departing[i] = (char*)calloc(sizeof(tmp_flights[i].departing_runway_name), sizeof(char));
			strcpy(h_aircraft_soa.runway_name_departing[i], tmp_flights[i].departing_runway_name); // '\0' already existed at the end
		}

		if ((tmp_flights[i].landing_runway_name != NULL) && (sizeof(tmp_flights[i].landing_runway_name) > 0)) {
			h_aircraft_soa.runway_name_landing[i] = (char*)calloc(sizeof(tmp_flights[i].landing_runway_name), sizeof(char));
			strcpy(h_aircraft_soa.runway_name_landing[i], tmp_flights[i].landing_runway_name); // '\0' already existed at the end
		}

		int offset = i * MAX_FLIGHT_PLAN_LENGTH;

		h_aircraft_soa.origin_airport_elevation_ft[i] = tmp_flights[i].origin_airport_elevation_ft;
		h_aircraft_soa.destination_airport_elevation_ft[i] = tmp_flights[i].destination_airport_elevation_ft;

		h_aircraft_soa.adb_aircraft_type_index[i] = tmp_flights[i].adb_aircraft_type_index;

		if (h_aircraft_soa.adb_aircraft_type_index[i] < 0 ||
				h_aircraft_soa.adb_aircraft_type_index[i] >= adb_ptf_type_num) {
			printf("  ERROR: Invalid ADB index for %s (%s)\n",
					tmp_records[tmp_flights[i].key].acid.c_str(),
					tmp_records[tmp_flights[i].key].actype.c_str());

			exit(-1);
		}

		h_aircraft_soa.target_waypoint_node_ptr[i] = tmp_flights[i].target_waypoint_ptr;
		h_aircraft_soa.target_waypoint_index[i] = tmp_flights[i].target_waypoint_index;

		// holding data is initialized to 0, so no need to copy
		h_aircraft_soa.target_altitude_ft[i] = tmp_flights[i].target_altitude_ft;
		h_aircraft_soa.toc_index[i] = tmp_flights[i].toc_index;
		h_aircraft_soa.tod_index[i] = tmp_flights[i].tod_index;

		// ====================================================================

		int flight_key = tmp_flights[i].key;

		TrxRecord curTrxRecord = tmp_records.at(flight_key);

		long departure_time = curTrxRecord.timestamp - start_time;

		g_trajectories.push_back(Trajectory(i,
				curTrxRecord.acid,
				curTrxRecord.actype,
				tmp_flightplans.at(flight_key).origin,
				tmp_flightplans.at(flight_key).destination,
				departure_time,
				g_step_size,
				g_step_size_airborne,
				tmp_flights[i].cruise_alt_ft,
				tmp_flights[i].cruise_tas_knots,
				tmp_flights[i].origin_airport_elevation_ft,
				tmp_flights[i].destination_airport_elevation_ft,
				false
				));

		g_flightplans.insert(pair<int, FlightPlan>(i, tmp_flightplans.at(flight_key)));
		g_trx_records.insert(pair<int, TrxRecord>(i, tmp_records.at(flight_key)));

		map_Acid_FlightSeq.insert(pair<string, int>(curTrxRecord.acid, i));
	} // end - omp parallel for

#pragma omp barrier

	// Initialize the vector of all aircraft clearance data structure
	for (unsigned int i = 0; i < num_flights; i++) {
		g_map_clearance_aircraft.push_back(map<ENUM_Aircraft_Clearance, Aircraft_Clearance_Data_t>());
	}

/* Debug: List all acid in g_trx_records */
/*
	for (int i=0; i<num_flights; ++i) {
		string v_ac_id = g_trx_records[i].acid;

		int v_Int = map_Acid_FlightSeq.at(v_ac_id);
		printf("Element in map_Acid_FlightSeq --> ac_id = %s, v_Int = %d\n", v_ac_id.c_str(), v_Int);
	}
*/

	// Allocate memory space
	g_humanError_Controller = (map<HumanErrorEvent_t, ControllerErrorData_t>**)calloc(1, sizeof(map<HumanErrorEvent_t, ControllerErrorData_t>*));
	g_humanError_Controller[0] = (map<HumanErrorEvent_t, ControllerErrorData_t>*)calloc(num_flights, sizeof(map<HumanErrorEvent_t, ControllerErrorData_t>));

	// Initialize g_humanError_Controller
	for (unsigned int i = 0; i < num_flights; i++) {
		g_humanError_Controller[0][i] = std::map<HumanErrorEvent_t, ControllerErrorData_t>();
	}

	// ========================================================================

	int total_ignored = ignore_count + filter_count;
	int total_trx_count = num_flights + total_ignored;
	int total_active = num_flights;

	int tmp_flight_key;
	for (int i = 0; i < g_trajectories.size(); i++) {
		tmp_flight_key = tmp_flights.at(i).key;
	}

	printf("\n");
	printf("    Total flight count:  %7d\n", total_trx_count);
	printf("    Ignored flights:     %7d\n", total_ignored);
	printf("    Valid flights:       %7d\n", total_active);

	h_aircraft_soa.V_horizontal = (real_t*)calloc(num_flights, sizeof(real_t));
	h_aircraft_soa.acceleration_aiming_waypoint_node_ptr = (waypoint_node_t**)malloc(num_flights * sizeof(waypoint_node_t*));
	h_aircraft_soa.acceleration = (real_t*)calloc(num_flights, sizeof(real_t));
	h_aircraft_soa.V2_point_latitude_deg = (real_t*)calloc(num_flights, sizeof(real_t));
	h_aircraft_soa.V2_point_longitude_deg = (real_t*)calloc(num_flights, sizeof(real_t));
	h_aircraft_soa.estimate_takeoff_point_latitude_deg = (real_t*)calloc(num_flights, sizeof(real_t));
	h_aircraft_soa.estimate_takeoff_point_longitude_deg = (real_t*)calloc(num_flights, sizeof(real_t));
	h_aircraft_soa.estimate_touchdown_point_latitude_deg = (real_t*)calloc(num_flights, sizeof(real_t));
	h_aircraft_soa.estimate_touchdown_point_longitude_deg = (real_t*)calloc(num_flights, sizeof(real_t));
	h_aircraft_soa.t_takeoff = (float*)calloc(num_flights, sizeof(float));
	h_aircraft_soa.t_landing = (float*)calloc(num_flights, sizeof(float));
	h_aircraft_soa.hold_flight_phase = (ENUM_Flight_Phase*)malloc(num_flights * sizeof(ENUM_Flight_Phase));
	h_aircraft_soa.course_rad_runway = (real_t*)calloc(num_flights, sizeof(real_t));
	h_aircraft_soa.course_rad_taxi = (real_t*)calloc(num_flights, sizeof(real_t));

	// allocate device arrays and copy from host to device if using gpu
	// otherwise, just assign the pointers from h to d
#if USE_GPU
	cuda_malloc((void**)&d_aircraft_soa.departure_time_sec, real_array_size);
	cuda_malloc((void**)&d_aircraft_soa.cruise_alt_ft, real_array_size);
	cuda_malloc((void**)&d_aircraft_soa.cruise_tas_knots, real_array_size);
	cuda_malloc((void**)&d_aircraft_soa.flight_plan_latitude_deg, fp_array_size);
	cuda_malloc((void**)&d_aircraft_soa.flight_plan_longitude_deg, fp_array_size);
	cuda_malloc((void**)&d_aircraft_soa.flight_plan_length, int_array_size);
	d_aircraft_soa.flight_plan_waypoint_name.resize(num_flights*MAX_FLIGHT_PLAN_LENGTH);
	cuda_malloc((void**)&d_aircraft_soa.origin_airport_elevation_ft, real_array_size);
	cuda_malloc((void**)&d_aircraft_soa.destination_airport_elevation_ft, real_array_size);
	cuda_malloc((void**)&d_aircraft_soa.sector_index, int_array_size);
	cuda_malloc((void**)&d_aircraft_soa.latitude_deg, real_array_size);
	cuda_malloc((void**)&d_aircraft_soa.longitude_deg, real_array_size);
	cuda_malloc((void**)&d_aircraft_soa.altitude_ft, real_array_size);
	cuda_malloc((void**)&d_aircraft_soa.rocd_fps, real_array_size);
	cuda_malloc((void**)&d_aircraft_soa.tas_knots, real_array_size);
	cuda_malloc((void**)&d_aircraft_soa.course_rad, real_array_size);
	cuda_malloc((void**)&d_aircraft_soa.fpa_rad, real_array_size);
	cuda_malloc((void**)&d_aircraft_soa.flight_mode, flight_mode_array_size);
	cuda_malloc((void**)&d_aircraft_soa.landed_flag, int_array_size);
	cuda_malloc((void**)&d_aircraft_soa.adb_aircraft_type_index, int_array_size);
	cuda_malloc((void**)&d_aircraft_soa.holding_started, bool_array_size);
	cuda_malloc((void**)&d_aircraft_soa.holding_stopped, bool_array_size);
	cuda_malloc((void**)&d_aircraft_soa.has_holding_pattern, bool_array_size);
	cuda_malloc((void**)&d_aircraft_soa.hold_start_index, int_array_size);
	cuda_malloc((void**)&d_aircraft_soa.hold_end_index, int_array_size);
	cuda_malloc((void**)&d_aircraft_soa.flight_mode_backup, flight_mode_array_size);
	cuda_malloc((void**)&d_aircraft_soa.holding_tas_knots, real_array_size);
	cuda_malloc((void**)&d_aircraft_soa.target_waypoint_index, int_array_size);
	cuda_malloc((void**)&d_aircraft_soa.target_altitude_ft, real_array_size);
	cuda_malloc((void**)&d_aircraft_soa.toc_index, int_array_size);
	cuda_malloc((void**)&d_aircraft_soa.tod_index, int_array_size);

	// copy from host to device
	cuda_memcpy_async(d_aircraft_soa.departure_time_sec, h_aircraft_soa.departure_time_sec, real_array_size, cudaMemcpyHostToDevice, 0);
	cuda_memcpy_async(d_aircraft_soa.cruise_alt_ft, h_aircraft_soa.cruise_alt_ft, real_array_size, cudaMemcpyHostToDevice, 0);
	cuda_memcpy_async(d_aircraft_soa.cruise_tas_knots, h_aircraft_soa.cruise_tas_knots, real_array_size, cudaMemcpyHostToDevice, 0);
	cuda_memcpy_async(d_aircraft_soa.flight_plan_latitude_deg, h_aircraft_soa.flight_plan_latitude_deg, fp_array_size, cudaMemcpyHostToDevice, 0);
	cuda_memcpy_async(d_aircraft_soa.flight_plan_longitude_deg, h_aircraft_soa.flight_plan_longitude_deg, fp_array_size, cudaMemcpyHostToDevice, 0);
	cuda_memcpy_async(d_aircraft_soa.flight_plan_length, h_aircraft_soa.flight_plan_length, int_array_size, cudaMemcpyHostToDevice, 0);
	cuda_memcpy_async(d_aircraft_soa.flight_plan_waypoint_name, h_aircraft_soa.flight_plan_waypoint_name, num_flights*MAX_FLIGHT_PLAN_LENGTH, cudaMemcpyHostToDevice, 0);
	cuda_memcpy_async(d_aircraft_soa.origin_airport_elevation_ft, h_aircraft_soa.origin_airport_elevation_ft, real_array_size, cudaMemcpyHostToDevice, 0);
	cuda_memcpy_async(d_aircraft_soa.destination_airport_elevation_ft, h_aircraft_soa.destination_airport_elevation_ft, real_array_size, cudaMemcpyHostToDevice, 0);
	cuda_memcpy_async(d_aircraft_soa.sector_index, h_aircraft_soa.sector_index, int_array_size, cudaMemcpyHostToDevice, 0);
	cuda_memcpy_async(d_aircraft_soa.latitude_deg, h_aircraft_soa.latitude_deg, real_array_size, cudaMemcpyHostToDevice, 0);
	cuda_memcpy_async(d_aircraft_soa.longitude_deg, h_aircraft_soa.longitude_deg, real_array_size, cudaMemcpyHostToDevice, 0);
	cuda_memcpy_async(d_aircraft_soa.altitude_ft, h_aircraft_soa.altitude_ft, real_array_size, cudaMemcpyHostToDevice, 0);
	cuda_memcpy_async(d_aircraft_soa.rocd_fps, h_aircraft_soa.rocd_fps, real_array_size, cudaMemcpyHostToDevice, 0);
	cuda_memcpy_async(d_aircraft_soa.tas_knots, h_aircraft_soa.tas_knots, real_array_size, cudaMemcpyHostToDevice, 0);
	cuda_memcpy_async(d_aircraft_soa.course_rad, h_aircraft_soa.course_rad, real_array_size, cudaMemcpyHostToDevice, 0);
	cuda_memcpy_async(d_aircraft_soa.fpa_rad, h_aircraft_soa.fpa_rad, real_array_size, cudaMemcpyHostToDevice, 0);
	cuda_memcpy_async(d_aircraft_soa.flight_mode, h_aircraft_soa.flight_mode, flight_mode_array_size, cudaMemcpyHostToDevice, 0);
	cuda_memcpy_async(d_aircraft_soa.landed_flag, h_aircraft_soa.landed_flag, int_array_size, cudaMemcpyHostToDevice, 0);
	cuda_memcpy_async(d_aircraft_soa.adb_aircraft_type_index, h_aircraft_soa.adb_aircraft_type_index, int_array_size, cudaMemcpyHostToDevice, 0);
	cuda_memcpy_async(d_aircraft_soa.holding_started, h_aircraft_soa.holding_started, bool_array_size, cudaMemcpyHostToDevice, 0);
	cuda_memcpy_async(d_aircraft_soa.holding_stopped, h_aircraft_soa.holding_stopped, bool_array_size, cudaMemcpyHostToDevice, 0);
	cuda_memcpy_async(d_aircraft_soa.has_holding_pattern, h_aircraft_soa.has_holding_pattern, bool_array_size, cudaMemcpyHostToDevice, 0);
	cuda_memcpy_async(d_aircraft_soa.hold_start_index, h_aircraft_soa.hold_start_index, int_array_size, cudaMemcpyHostToDevice, 0);
	cuda_memcpy_async(d_aircraft_soa.hold_end_index, h_aircraft_soa.hold_end_index, int_array_size, cudaMemcpyHostToDevice, 0);
	cuda_memcpy_async(d_aircraft_soa.flight_mode_backup, h_aircraft_soa.flight_mode_backup, flight_mode_array_size, cudaMemcpyHostToDevice, 0);
	cuda_memcpy_async(d_aircraft_soa.holding_tas_knots, h_aircraft_soa.holding_tas_knots, real_array_size, cudaMemcpyHostToDevice, 0);
	cuda_memcpy_async(d_aircraft_soa.target_waypoint_index, h_aircraft_soa.target_waypoint_index, int_array_size, cudaMemcpyHostToDevice, 0);
	cuda_memcpy_async(d_aircraft_soa.target_altitude_ft, h_aircraft_soa.target_altitude_ft, real_array_size, cudaMemcpyHostToDevice, 0);
	cuda_memcpy_async(d_aircraft_soa.toc_index, h_aircraft_soa.toc_index, int_array_size, cudaMemcpyHostToDevice, 0);
	cuda_memcpy_async(d_aircraft_soa.tod_index, h_aircraft_soa.tod_index, int_array_size, cudaMemcpyHostToDevice, 0);
#else
	// assign d pointers to h arrays
	d_aircraft_soa.departure_time_sec = h_aircraft_soa.departure_time_sec;
	d_aircraft_soa.cruise_alt_ft = h_aircraft_soa.cruise_alt_ft;
	d_aircraft_soa.cruise_tas_knots = h_aircraft_soa.cruise_tas_knots;

	d_aircraft_soa.origin_airport_elevation_ft = h_aircraft_soa.origin_airport_elevation_ft;
	d_aircraft_soa.destination_airport_elevation_ft = h_aircraft_soa.destination_airport_elevation_ft;
	d_aircraft_soa.sector_index = h_aircraft_soa.sector_index;
	d_aircraft_soa.latitude_deg = h_aircraft_soa.latitude_deg;
	d_aircraft_soa.longitude_deg = h_aircraft_soa.longitude_deg;
	d_aircraft_soa.altitude_ft = h_aircraft_soa.altitude_ft;
	d_aircraft_soa.rocd_fps = h_aircraft_soa.rocd_fps;
	d_aircraft_soa.tas_knots = h_aircraft_soa.tas_knots;
	d_aircraft_soa.tas_knots_ground = h_aircraft_soa.tas_knots_ground;
	d_aircraft_soa.course_rad = h_aircraft_soa.course_rad;
	d_aircraft_soa.fpa_rad = h_aircraft_soa.fpa_rad;
	d_aircraft_soa.flight_phase = h_aircraft_soa.flight_phase;

	d_aircraft_soa.latitude_deg_pre_pause = h_aircraft_soa.latitude_deg_pre_pause;
	d_aircraft_soa.longitude_deg_pre_pause = h_aircraft_soa.longitude_deg_pre_pause;
	d_aircraft_soa.altitude_ft_pre_pause = h_aircraft_soa.altitude_ft_pre_pause;
	d_aircraft_soa.rocd_fps_pre_pause = h_aircraft_soa.rocd_fps_pre_pause;
	d_aircraft_soa.tas_knots_pre_pause = h_aircraft_soa.tas_knots_pre_pause;
	d_aircraft_soa.course_rad_pre_pause = h_aircraft_soa.course_rad_pre_pause;
	d_aircraft_soa.fpa_rad_pre_pause = h_aircraft_soa.fpa_rad_pre_pause;
	d_aircraft_soa.cruise_alt_ft_pre_pause = h_aircraft_soa.cruise_alt_ft_pre_pause;
	d_aircraft_soa.cruise_tas_knots_pre_pause = h_aircraft_soa.cruise_tas_knots_pre_pause;

	d_aircraft_soa.landed_flag = h_aircraft_soa.landed_flag;
	d_aircraft_soa.adb_aircraft_type_index = h_aircraft_soa.adb_aircraft_type_index;
	d_aircraft_soa.holding_started = h_aircraft_soa.holding_started;
	d_aircraft_soa.holding_stopped = h_aircraft_soa.holding_stopped;
	d_aircraft_soa.has_holding_pattern = h_aircraft_soa.has_holding_pattern;
	d_aircraft_soa.hold_start_index = h_aircraft_soa.hold_start_index;
	d_aircraft_soa.hold_end_index = h_aircraft_soa.hold_end_index;
	d_aircraft_soa.holding_tas_knots = h_aircraft_soa.holding_tas_knots;
	d_aircraft_soa.target_waypoint_index = h_aircraft_soa.target_waypoint_index;
	d_aircraft_soa.target_altitude_ft = h_aircraft_soa.target_altitude_ft;
	d_aircraft_soa.toc_index = h_aircraft_soa.toc_index;
	d_aircraft_soa.tod_index = h_aircraft_soa.tod_index;
#endif

	// Allocate memory space
	g_humanError_Pilot = (map<HumanErrorEvent_t, PilotErrorData_t>*)calloc(num_flights, sizeof(map<HumanErrorEvent_t, PilotErrorData_t>));

	// Set initial data
	for (unsigned int i = 0; i < num_flights; i++) {
		g_humanError_Pilot[i] = map<HumanErrorEvent_t, PilotErrorData_t>();

		h_aircraft_soa.estimate_takeoff_point_latitude_deg[i] = 0.0; // Initial value
		h_aircraft_soa.estimate_takeoff_point_longitude_deg[i] = 0.0; // Initial value
		h_aircraft_soa.estimate_touchdown_point_latitude_deg[i] = 0.0; // Initial value
		h_aircraft_soa.estimate_touchdown_point_longitude_deg[i] = 0.0; // Initial value
	}

	// Clean up char** data in tmp_flights
	if (!tmp_flights.empty()) {
		for (unsigned int i = 0; i < tmp_flights.size(); i++) {
			if (tmp_flights[i].flight_plan_waypoint_name != NULL) {
				for (int j = 0; j < MAX_FLIGHT_PLAN_LENGTH; j++) {
					if (tmp_flights[i].flight_plan_waypoint_name[j] != NULL) {
						free(tmp_flights[i].flight_plan_waypoint_name[j]);
						tmp_flights[i].flight_plan_waypoint_name[j] = NULL;
					}
				}
			}

			if (tmp_flights[i].flight_plan_path_term != NULL) {
				for (int j = 0; j < MAX_FLIGHT_PLAN_LENGTH; j++) {
					if (tmp_flights[i].flight_plan_path_term[j] != NULL) {
						free(tmp_flights[i].flight_plan_path_term[j]);
						tmp_flights[i].flight_plan_path_term[j] = NULL;
					}
				}
			}

			if (tmp_flights[i].flight_plan_alt_desc != NULL) {
				for (int j = 0; j < MAX_FLIGHT_PLAN_LENGTH; j++) {
					if (tmp_flights[i].flight_plan_alt_desc[j] != NULL) {
						free(tmp_flights[i].flight_plan_alt_desc[j]);
						tmp_flights[i].flight_plan_alt_desc[j] = NULL;
					}
				}
			}

			if (tmp_flights[i].flight_plan_procname != NULL) {
				for (int j = 0; j < MAX_FLIGHT_PLAN_LENGTH; j++) {
					if (tmp_flights[i].flight_plan_procname[j] != NULL) {
						free(tmp_flights[i].flight_plan_procname[j]);
						tmp_flights[i].flight_plan_procname[j] = NULL;
					}
				}
			}

			if (tmp_flights[i].flight_plan_proctype != NULL) {
				for (int j = 0; j < MAX_FLIGHT_PLAN_LENGTH; j++) {
					if (tmp_flights[i].flight_plan_proctype[j] != NULL) {
						free(tmp_flights[i].flight_plan_proctype[j]);
						tmp_flights[i].flight_plan_proctype[j] = NULL;
					}
				}
			}

			if (tmp_flights[i].flight_plan_recco_navaid != NULL) {
				for (int j = 0; j < MAX_FLIGHT_PLAN_LENGTH; j++) {
					if (tmp_flights[i].flight_plan_recco_navaid[j] != NULL) {
						free(tmp_flights[i].flight_plan_recco_navaid[j]);
						tmp_flights[i].flight_plan_recco_navaid[j] = NULL;
					}
				}
			}

			if (tmp_flights[i].flight_plan_spdlim_desc != NULL) {
				for (int j = 0; j < MAX_FLIGHT_PLAN_LENGTH; j++) {
					if (tmp_flights[i].flight_plan_spdlim_desc[j] != NULL) {
						free(tmp_flights[i].flight_plan_spdlim_desc[j]);
						tmp_flights[i].flight_plan_spdlim_desc[j] = NULL;
					}
				}
			}

			if (tmp_flights[i].departing_runway_name != NULL) {
				free(tmp_flights[i].departing_runway_name);

				tmp_flights[i].departing_runway_name = NULL;
			}

			if (tmp_flights[i].landing_runway_name != NULL) {
				free(tmp_flights[i].landing_runway_name);

				tmp_flights[i].landing_runway_name = NULL;
			}

			// Set target waypoint pointer to NULL
			// If there is a target waypoint pointer, it has been set to h_aircraft_soa.target_waypoint_node_ptr[i].
			// Here we have to set tmp_flights[i].target_waypoint_ptr to NULL to avoid the target waypoint node being deleted in runtime.
			tmp_flights[i].target_waypoint_ptr = NULL;
		}

		tmp_flights.clear();
	}

	newMU_external_aircraft.clear();
	newMU_external_aircraft.resize(num_flights);

	external_aircraft_seq = 0;

	return 0;
}

bool collect_user_upload_aircraft(string string_track_time,
		string string_track,
		string string_fp_route,
		const int mfl_ft,
		const bool flag_validator,
		string& errorMsg) {
	bool retValue = false;

	const real_t cruise_perturbation = 0;

	memset(buffer_stdout, 0, sizeof buffer_stdout);

	string tmpErrorMsg;

	errorMsg.assign(""); // Reset

	int out_pipe[2];
	int saved_stdout;

	if (flag_validator) {
		saved_stdout = dup(STDOUT_FILENO);  /* save stdout for display later */

		if ( pipe(out_pipe) != 0 ) {          /* make a pipe */
			printf("collect_user_upload_aircraft(): Can't create pipe\n");

			exit(1);
		}

		dup2(out_pipe[1], STDOUT_FILENO);   /* redirect stdout to the pipe */
		close(out_pipe[1]);
	}

	long timestamp = -1;
	double mfl = 0;

	TrxRecord trxRecord;

	string_track_time = trim(string_track_time);
	string_track = trim(string_track);
	string_fp_route = trim(string_fp_route);

	if ((string_track_time.length() == 0) || (string_track.length() == 0) || (string_fp_route.length() == 0)) {
		return retValue;
	}

	bool tmpFlag_geoStyle = false;
	if (string_fp_route.find("ap_code") != string::npos) {
		tmpFlag_geoStyle = true;
	}

	deque<string> tokens;
	tokens.clear();

	tokens = tokenize(string_track_time, " ");
	// Process string_track_time
	if (tokens.at(0) == "TRACK_TIME") {
		tokens.pop_front();
		timestamp = atol(tokens.front().c_str());
	}

	if (timestamp < 0) {
		tmpErrorMsg.assign("TRACK TIME is not valid");

		return retValue;
	}

	tokens = tokenize(string_track, " ");
	// Process string_track
	if (tokens.at(0) == "TRACK") {
	    // remove the "TRACK" token from the tokens deque
	    tokens.pop_front();

		// osi - jason: update 7 nov 2017
		// i'm so sick of the stupid separate mfl file.
		// i believe newer versions of nats did away with it long ago
		// and combined the cruise altitude into the main trx file.
		// i'm going to do the same. not sure what the new nats trx file
		// looks like since i haven't seen one since about 2006 when
		// they still used the mfl file.
		// i'm just going to append the mfl to the track line.
		// still want to be able to handle old mfl files though.
		// so if the track line has 9 tokens (not counting 'TRACK')
		// then we use the old separate mfl file. else if the track
		// line has 10 tokens (not counting 'TRACK') then assume that
		// the last token is the mfl.
		if (tokens.size() > 9) {
			tokens.pop_back();
		}

		mfl = mfl_ft;
	}

	if (tokens.size() == 0) {
		tmpErrorMsg.assign("TRACK data is not valid");

		return retValue;
	}

	if (mfl <= 0) {
		tmpErrorMsg.assign("Aircraft MFL(maximum flight level) data is not valid");

		return retValue;
	}

	if (string_fp_route.find("FP_ROUTE") == 0) {
		tokens.insert(tokens.end(), string_fp_route);

		TrxInputStream trxInputStream;

		// Generate TRX record
		trxRecord = trxInputStream.generateTrxRecord(timestamp, tokens, string_track, mfl, tmpFlag_geoStyle);
	}

	// Check if trxRecord has data
	if (trxRecord.trx_str.length() == 0) {
		tmpErrorMsg.assign("FP ROUTE data is not valid");

		return retValue;
	}

	// !!!!!!!!!!!!!! Temporarily set start time = 0 !!!!!!!!!!!!!!
	long start_time = 0;

	long departure_time = trxRecord.timestamp - start_time;
	FlightPlan tmp_fp;
	temp_flight_t tmp_flight;

	omp_lock_t* lock = NULL;

	// ====================================================

	bool result = false;

	if (trxRecord.flag_geoStyle) {
		result = compute_flight_data_geoStyle(departure_time,
					trxRecord,
					tmp_fp,
					tmp_flight,
					cruise_perturbation,
					*lock);
	} else {
		if (flag_SidStarApp_available) {
			result = compute_flight_data(departure_time,
					trxRecord,
					tmp_fp,
					tmp_flight,
					cruise_perturbation,
					*lock);
		} else {
			ignore_count++;
		}
	}

	if (flag_validator) {
		fflush(stdout);

		read(out_pipe[0], buffer_stdout, MAX_LEN); /* read from pipe into buffer */

		dup2(saved_stdout, STDOUT_FILENO);  /* reconnect stdout */

		string str_buffer_stdout(buffer_stdout);
		if (str_buffer_stdout.length() > 0) {
			errorMsg.assign(str_buffer_stdout);
		}
	}

	if (0 < tmpErrorMsg.length()) {
		if (0 < errorMsg.length()) {
			errorMsg.append("\n");
			errorMsg.append(tmpErrorMsg);
		} else {
			errorMsg.assign(tmpErrorMsg);
		}
	}

	if (result) {
		retValue = true; // Success
	}

	return retValue;
}

int destroy_aircraft() {
	g_map_clearance_aircraft.clear();

   	if (!g_trajectories.empty()) g_trajectories.clear();

    if (!map_Acid_FlightSeq.empty()) map_Acid_FlightSeq.clear();

	if (h_aircraft_soa.flag_geoStyle) {
		cuda_free_host(h_aircraft_soa.flag_geoStyle);
		h_aircraft_soa.flag_geoStyle = NULL;
#if !USE_GPU
		d_aircraft_soa.flag_geoStyle = NULL;
#endif
	}

	if (h_aircraft_soa.sector_index) {
		cuda_free_host(h_aircraft_soa.sector_index);
		h_aircraft_soa.sector_index = NULL;
#if !USE_GPU
		d_aircraft_soa.sector_index = NULL;
#endif
	}
	if (h_aircraft_soa.latitude_deg) {
		cuda_free_host(h_aircraft_soa.latitude_deg);
		h_aircraft_soa.latitude_deg = NULL;
#if !USE_GPU
		d_aircraft_soa.latitude_deg = NULL;
#endif
	}
	if (h_aircraft_soa.longitude_deg) {
		cuda_free_host(h_aircraft_soa.longitude_deg);
		h_aircraft_soa.longitude_deg = NULL;
#if !USE_GPU
		d_aircraft_soa.longitude_deg = NULL;
#endif
	}

	if (h_aircraft_soa.altitude_ft) {
		cuda_free_host(h_aircraft_soa.altitude_ft);
		h_aircraft_soa.altitude_ft = NULL;
#if !USE_GPU
		d_aircraft_soa.altitude_ft = NULL;
#endif
	}
	if (h_aircraft_soa.rocd_fps) {
		cuda_free_host(h_aircraft_soa.rocd_fps);
		h_aircraft_soa.rocd_fps = NULL;
#if !USE_GPU
		d_aircraft_soa.rocd_fps = NULL;
#endif
	}
	if (h_aircraft_soa.tas_knots) {
		cuda_free_host(h_aircraft_soa.tas_knots);
		h_aircraft_soa.tas_knots = NULL;
#if !USE_GPU
		d_aircraft_soa.tas_knots = NULL;
#endif
	}
	if (h_aircraft_soa.tas_knots_ground) {
		cuda_free_host(h_aircraft_soa.tas_knots_ground);
		h_aircraft_soa.tas_knots_ground = NULL;
#if !USE_GPU
		d_aircraft_soa.tas_knots_ground = NULL;
#endif
	}
	if (h_aircraft_soa.course_rad) {
		cuda_free_host(h_aircraft_soa.course_rad);
		h_aircraft_soa.course_rad = NULL;
#if !USE_GPU
		d_aircraft_soa.course_rad = NULL;
#endif
	}
	if (h_aircraft_soa.fpa_rad) {
		cuda_free_host(h_aircraft_soa.fpa_rad);
		h_aircraft_soa.fpa_rad = NULL;
#if !USE_GPU
		d_aircraft_soa.fpa_rad = NULL;
#endif
	}
	if (h_aircraft_soa.flight_phase) {
		cuda_free_host(h_aircraft_soa.flight_phase);
		h_aircraft_soa.flight_phase = NULL;
#if !USE_GPU
		d_aircraft_soa.flight_phase = NULL;
#endif
	}
	if (h_aircraft_soa.departure_time_sec != NULL) {
		free(h_aircraft_soa.departure_time_sec);
		h_aircraft_soa.departure_time_sec = NULL;
#if !USE_GPU
		d_aircraft_soa.departure_time_sec = NULL;
#endif
	}
	if (h_aircraft_soa.cruise_alt_ft) {
		free(h_aircraft_soa.cruise_alt_ft);
		h_aircraft_soa.cruise_alt_ft = NULL;
#if !USE_GPU
		d_aircraft_soa.cruise_alt_ft = NULL;
#endif
	}
	if (h_aircraft_soa.cruise_tas_knots) {
		free(h_aircraft_soa.cruise_tas_knots);
		h_aircraft_soa.cruise_tas_knots = NULL;
#if !USE_GPU
		d_aircraft_soa.cruise_tas_knots = NULL;
#endif
	}

	if (h_aircraft_soa.latitude_deg_pre_pause) {
		cuda_free_host(h_aircraft_soa.latitude_deg_pre_pause);
		h_aircraft_soa.latitude_deg_pre_pause = NULL;
#if !USE_GPU
		d_aircraft_soa.latitude_deg_pre_pause = NULL;
#endif
	}
	if (h_aircraft_soa.longitude_deg_pre_pause) {
		cuda_free_host(h_aircraft_soa.longitude_deg_pre_pause);
		h_aircraft_soa.longitude_deg_pre_pause = NULL;
#if !USE_GPU
		d_aircraft_soa.longitude_deg_pre_pause = NULL;
#endif
	}
	if (h_aircraft_soa.altitude_ft_pre_pause) {
		cuda_free_host(h_aircraft_soa.altitude_ft_pre_pause);
		h_aircraft_soa.altitude_ft_pre_pause = NULL;
#if !USE_GPU
		d_aircraft_soa.altitude_ft_pre_pause = NULL;
#endif
	}
	if (h_aircraft_soa.rocd_fps_pre_pause) {
		cuda_free_host(h_aircraft_soa.rocd_fps_pre_pause);
		h_aircraft_soa.rocd_fps_pre_pause = NULL;
#if !USE_GPU
		d_aircraft_soa.rocd_fps_pre_pause = NULL;
#endif
	}
	if (h_aircraft_soa.tas_knots_pre_pause) {
		cuda_free_host(h_aircraft_soa.tas_knots_pre_pause);
		h_aircraft_soa.tas_knots_pre_pause = NULL;
#if !USE_GPU
		d_aircraft_soa.tas_knots_pre_pause = NULL;
#endif
	}
	if (h_aircraft_soa.course_rad_pre_pause) {
		cuda_free_host(h_aircraft_soa.course_rad_pre_pause);
		h_aircraft_soa.course_rad_pre_pause = NULL;
#if !USE_GPU
		d_aircraft_soa.course_rad_pre_pause = NULL;
#endif
	}
	if (h_aircraft_soa.fpa_rad_pre_pause) {
		cuda_free_host(h_aircraft_soa.fpa_rad_pre_pause);
		h_aircraft_soa.fpa_rad_pre_pause = NULL;
#if !USE_GPU
		d_aircraft_soa.fpa_rad_pre_pause = NULL;
#endif
	}
	if (h_aircraft_soa.cruise_alt_ft_pre_pause) {
		cuda_free_host(h_aircraft_soa.cruise_alt_ft_pre_pause);
		h_aircraft_soa.cruise_alt_ft_pre_pause = NULL;
#if !USE_GPU
		d_aircraft_soa.cruise_alt_ft_pre_pause = NULL;
#endif
	}
	if (h_aircraft_soa.cruise_tas_knots_pre_pause) {
		cuda_free_host(h_aircraft_soa.cruise_tas_knots_pre_pause);
		h_aircraft_soa.cruise_tas_knots_pre_pause = NULL;
#if !USE_GPU
		d_aircraft_soa.cruise_tas_knots_pre_pause = NULL;
#endif
	}

	if (h_aircraft_soa.origin_airport_elevation_ft) {
		free(h_aircraft_soa.origin_airport_elevation_ft);
		h_aircraft_soa.origin_airport_elevation_ft = NULL;
#if !USE_GPU
		d_aircraft_soa.origin_airport_elevation_ft = NULL;
#endif
	}
	if (h_aircraft_soa.destination_airport_elevation_ft) {
		free(h_aircraft_soa.destination_airport_elevation_ft);
		h_aircraft_soa.destination_airport_elevation_ft = NULL;
#if !USE_GPU
		d_aircraft_soa.destination_airport_elevation_ft = NULL;
#endif
	}

	if (h_aircraft_soa.adb_aircraft_type_index) {
		free(h_aircraft_soa.adb_aircraft_type_index);
		h_aircraft_soa.adb_aircraft_type_index = NULL;
#if !USE_GPU
		d_aircraft_soa.adb_aircraft_type_index = NULL;
#endif
	}
	if (h_aircraft_soa.landed_flag) {
		free(h_aircraft_soa.landed_flag);
		h_aircraft_soa.landed_flag = NULL;
#if !USE_GPU
		d_aircraft_soa.landed_flag = NULL;
#endif
	}
	if (h_aircraft_soa.holding_started) {
		free(h_aircraft_soa.holding_started);
		h_aircraft_soa.holding_started = NULL;
#if !USE_GPU
		d_aircraft_soa.holding_started = NULL;
#endif
	}
	if (h_aircraft_soa.holding_stopped) {
		free(h_aircraft_soa.holding_stopped);
		h_aircraft_soa.holding_stopped = NULL;
#if !USE_GPU
		d_aircraft_soa.holding_stopped = NULL;
#endif
	}
	if (h_aircraft_soa.has_holding_pattern) {
		free(h_aircraft_soa.has_holding_pattern);
		h_aircraft_soa.has_holding_pattern = NULL;
#if !USE_GPU
		d_aircraft_soa.has_holding_pattern = NULL;
#endif
	}
	if (h_aircraft_soa.hold_start_index) {
		free(h_aircraft_soa.hold_start_index);
		h_aircraft_soa.hold_start_index = NULL;
#if !USE_GPU
		d_aircraft_soa.hold_start_index = NULL;
#endif
	}
	if (h_aircraft_soa.hold_end_index) {
		free(h_aircraft_soa.hold_end_index);
		h_aircraft_soa.hold_end_index = NULL;
#if !USE_GPU
		d_aircraft_soa.hold_end_index = NULL;
#endif
	}

	if (h_aircraft_soa.holding_tas_knots) {
		free(h_aircraft_soa.holding_tas_knots);
		h_aircraft_soa.holding_tas_knots = NULL;
#if !USE_GPU
		d_aircraft_soa.holding_tas_knots = NULL;
#endif
	}
	if (h_aircraft_soa.target_waypoint_index) {
		free(h_aircraft_soa.target_waypoint_index);
		h_aircraft_soa.target_waypoint_index = NULL;
#if !USE_GPU
		d_aircraft_soa.target_waypoint_index = NULL;
#endif
	}
	if (h_aircraft_soa.target_altitude_ft) {
		free(h_aircraft_soa.target_altitude_ft);
		h_aircraft_soa.target_altitude_ft = NULL;
#if !USE_GPU
		d_aircraft_soa.target_altitude_ft = NULL;
#endif
	}
	if (h_aircraft_soa.toc_index) {
		free(h_aircraft_soa.toc_index);
		h_aircraft_soa.toc_index = NULL;
#if !USE_GPU
		d_aircraft_soa.toc_index = NULL;
#endif
	}
	if (h_aircraft_soa.tod_index != NULL) {
		free(h_aircraft_soa.tod_index);
		h_aircraft_soa.tod_index = NULL;
#if !USE_GPU
		d_aircraft_soa.tod_index = NULL;
#endif
	}

	if (h_aircraft_soa.runway_name_departing != NULL) {
		for (int i = 0; i < num_flights; i++) {
			if ((h_aircraft_soa.runway_name_departing[i] != NULL) && (h_aircraft_soa.runway_name_departing[i][0] != '\0')) {
				free(h_aircraft_soa.runway_name_departing[i]);
				h_aircraft_soa.runway_name_departing[i] = NULL;
			}
		}

		free(h_aircraft_soa.runway_name_departing);
		h_aircraft_soa.runway_name_departing = NULL;
	}
	if (h_aircraft_soa.runway_name_landing != NULL) {
		for (int i = 0; i < num_flights; i++) {
			if ((h_aircraft_soa.runway_name_landing[i] != NULL) && (h_aircraft_soa.runway_name_landing[i][0] != '\0')) {
				free(h_aircraft_soa.runway_name_landing[i]);
				h_aircraft_soa.runway_name_landing[i] = NULL;
			}
		}

		free(h_aircraft_soa.runway_name_landing);
		h_aircraft_soa.runway_name_landing = NULL;
	}
	
	if (h_aircraft_soa.target_waypoint_node_ptr != NULL) {
		free(h_aircraft_soa.target_waypoint_node_ptr);
		h_aircraft_soa.target_waypoint_node_ptr = NULL;
	}
	if (h_aircraft_soa.flag_target_waypoint_change != NULL) {
		free(h_aircraft_soa.flag_target_waypoint_change);
		h_aircraft_soa.flag_target_waypoint_change = NULL;
	}
	if (h_aircraft_soa.last_WaypointNode_ptr != NULL) {
		free(h_aircraft_soa.last_WaypointNode_ptr);
		h_aircraft_soa.last_WaypointNode_ptr = NULL;
	}
	if (h_aircraft_soa.flag_reached_meterfix_point != NULL) {
		free(h_aircraft_soa.flag_reached_meterfix_point);
		h_aircraft_soa.flag_reached_meterfix_point = NULL;
	}

	if (h_aircraft_soa.V_horizontal != NULL) {
		free(h_aircraft_soa.V_horizontal);
		h_aircraft_soa.V_horizontal = NULL;
	}
	if (h_aircraft_soa.acceleration_aiming_waypoint_node_ptr != NULL) {
		free(h_aircraft_soa.acceleration_aiming_waypoint_node_ptr);
		h_aircraft_soa.acceleration_aiming_waypoint_node_ptr = NULL;
	}
	if (h_aircraft_soa.acceleration) {
		free(h_aircraft_soa.acceleration);
		h_aircraft_soa.acceleration = NULL;
	}
	if (h_aircraft_soa.V2_point_latitude_deg) {
		free(h_aircraft_soa.V2_point_latitude_deg);
		h_aircraft_soa.V2_point_latitude_deg = NULL;
	}
	if (h_aircraft_soa.V2_point_longitude_deg) {
		free(h_aircraft_soa.V2_point_longitude_deg);
		h_aircraft_soa.V2_point_longitude_deg = NULL;
	}
	if (h_aircraft_soa.estimate_takeoff_point_latitude_deg) {
		free(h_aircraft_soa.estimate_takeoff_point_latitude_deg);
		h_aircraft_soa.estimate_takeoff_point_latitude_deg = NULL;
	}
	if (h_aircraft_soa.estimate_takeoff_point_longitude_deg) {
		free(h_aircraft_soa.estimate_takeoff_point_longitude_deg);
		h_aircraft_soa.estimate_takeoff_point_longitude_deg = NULL;
	}
	if (h_aircraft_soa.estimate_touchdown_point_latitude_deg) {
		free(h_aircraft_soa.estimate_touchdown_point_latitude_deg);
		h_aircraft_soa.estimate_touchdown_point_latitude_deg = NULL;
	}
	if (h_aircraft_soa.estimate_touchdown_point_longitude_deg) {
		free(h_aircraft_soa.estimate_touchdown_point_longitude_deg);
		h_aircraft_soa.estimate_touchdown_point_longitude_deg = NULL;
	}
	if (h_aircraft_soa.t_takeoff) {
		free(h_aircraft_soa.t_takeoff);
		h_aircraft_soa.t_takeoff = NULL;
	}
	if (h_aircraft_soa.t_landing) {
		free(h_aircraft_soa.t_landing);
		h_aircraft_soa.t_landing = NULL;
	}
	if (h_aircraft_soa.hold_flight_phase) {
		free(h_aircraft_soa.hold_flight_phase);
		h_aircraft_soa.hold_flight_phase = NULL;
	}
	if (h_aircraft_soa.course_rad_runway) {
		free(h_aircraft_soa.course_rad_runway);
		h_aircraft_soa.course_rad_runway = NULL;
	}
	if (h_aircraft_soa.course_rad_taxi) {
		free(h_aircraft_soa.course_rad_taxi);
		h_aircraft_soa.course_rad_taxi = NULL;
	}

	// ========================================================================

	// Release ground departing taxi plan memory space
	if (h_departing_taxi_plan.airport_code != NULL) {
		for (int i = 0; i < num_flights; i++) {
			if (h_departing_taxi_plan.airport_code[i] != NULL) {
				free(h_departing_taxi_plan.airport_code[i]);
				h_departing_taxi_plan.airport_code[i] = NULL;
			}
		}

		free(h_departing_taxi_plan.airport_code);
		h_departing_taxi_plan.airport_code = NULL;
	}

	if (h_departing_taxi_plan.waypoint_length != NULL) {
		free(h_departing_taxi_plan.waypoint_length);
		h_departing_taxi_plan.waypoint_length = NULL;
	}

	if (h_departing_taxi_plan.waypoint_node_ptr != NULL) {
		waypoint_node_t* tmp_cur_waypointNode_ptr;
		waypoint_node_t* tmp_next_waypointNode_ptr;

		for (int i = 0; i < num_flights; i++) {
			if (h_departing_taxi_plan.waypoint_node_ptr[i] != NULL) {
				tmp_next_waypointNode_ptr = h_departing_taxi_plan.waypoint_node_ptr[i];
				while (tmp_next_waypointNode_ptr != NULL) {
					tmp_cur_waypointNode_ptr = tmp_next_waypointNode_ptr;

					releaseWaypointNodeContent(tmp_cur_waypointNode_ptr);

					tmp_cur_waypointNode_ptr->prev_node_ptr = NULL;
					tmp_next_waypointNode_ptr = tmp_cur_waypointNode_ptr->next_node_ptr;
					tmp_cur_waypointNode_ptr->next_node_ptr = NULL;

					free(tmp_cur_waypointNode_ptr);
				}

				h_departing_taxi_plan.waypoint_node_ptr[i] = NULL;
			}
		}

		free(h_departing_taxi_plan.waypoint_node_ptr);
		h_departing_taxi_plan.waypoint_node_ptr = NULL;
	}

	if (h_departing_taxi_plan.waypoint_final_node_ptr != NULL) {
		free(h_departing_taxi_plan.waypoint_final_node_ptr);
		h_departing_taxi_plan.waypoint_final_node_ptr = NULL;
	}

	if (h_departing_taxi_plan.runway_name != NULL) {
		for (int i = 0; i < num_flights; i++) {
			if (h_departing_taxi_plan.runway_name[i] != NULL) {
				free(h_departing_taxi_plan.runway_name[i]);
				h_departing_taxi_plan.runway_name[i] = NULL;
			}
		}

		free(h_departing_taxi_plan.runway_name);
		h_departing_taxi_plan.runway_name = NULL;
	}

	if (h_departing_taxi_plan.taxi_tas_knots != NULL) {
		free(h_departing_taxi_plan.taxi_tas_knots);
		h_departing_taxi_plan.taxi_tas_knots = NULL;
	}
	if (h_departing_taxi_plan.ramp_tas_knots != NULL) {
		free(h_departing_taxi_plan.ramp_tas_knots);
		h_departing_taxi_plan.ramp_tas_knots = NULL;
	}
	if (h_departing_taxi_plan.runway_entry_latitude_geoStyle != NULL) {
		free(h_departing_taxi_plan.runway_entry_latitude_geoStyle);
		h_departing_taxi_plan.runway_entry_latitude_geoStyle = NULL;
	}
	if (h_departing_taxi_plan.runway_entry_longitude_geoStyle != NULL) {
		free(h_departing_taxi_plan.runway_entry_longitude_geoStyle);
		h_departing_taxi_plan.runway_entry_longitude_geoStyle = NULL;
	}
	if (h_departing_taxi_plan.runway_end_latitude_geoStyle != NULL) {
		free(h_departing_taxi_plan.runway_end_latitude_geoStyle);
		h_departing_taxi_plan.runway_end_latitude_geoStyle = NULL;
	}
	if (h_departing_taxi_plan.runway_end_longitude_geoStyle != NULL) {
		free(h_departing_taxi_plan.runway_end_longitude_geoStyle);
		h_departing_taxi_plan.runway_end_longitude_geoStyle = NULL;
	}

	// Release ground landing taxi plan memory space
	if (h_landing_taxi_plan.airport_code != NULL) {
		for (int i = 0; i < num_flights; i++) {
			if (h_landing_taxi_plan.airport_code[i] != NULL) {
				free(h_landing_taxi_plan.airport_code[i]);
				h_landing_taxi_plan.airport_code[i] = NULL;
			}
		}

		free(h_landing_taxi_plan.airport_code);
		h_landing_taxi_plan.airport_code = NULL;
	}
	if (h_landing_taxi_plan.waypoint_length != NULL) {
		free(h_landing_taxi_plan.waypoint_length);
		h_landing_taxi_plan.waypoint_length = NULL;
	}
	if (h_landing_taxi_plan.waypoint_node_ptr != NULL) {
		waypoint_node_t* tmp_cur_waypointNode_ptr;
		waypoint_node_t* tmp_next_waypointNode_ptr;

		for (int i = 0; i < num_flights; i++) {
			if (h_landing_taxi_plan.waypoint_node_ptr[i] != NULL) {
				tmp_next_waypointNode_ptr = h_landing_taxi_plan.waypoint_node_ptr[i];
				while (tmp_next_waypointNode_ptr != NULL) {
					tmp_cur_waypointNode_ptr = tmp_next_waypointNode_ptr;

					releaseWaypointNodeContent(tmp_cur_waypointNode_ptr);

					tmp_cur_waypointNode_ptr->prev_node_ptr = NULL;
					tmp_next_waypointNode_ptr = tmp_cur_waypointNode_ptr->next_node_ptr;
					tmp_cur_waypointNode_ptr->next_node_ptr = NULL;

					free(tmp_cur_waypointNode_ptr);
				}

				h_landing_taxi_plan.waypoint_node_ptr[i] = NULL;
			}
		}

		free(h_landing_taxi_plan.waypoint_node_ptr);
		h_landing_taxi_plan.waypoint_node_ptr = NULL;
	}
	if (h_landing_taxi_plan.waypoint_final_node_ptr != NULL) {
		free(h_landing_taxi_plan.waypoint_final_node_ptr);
		h_landing_taxi_plan.waypoint_final_node_ptr = NULL;
	}
	if (h_landing_taxi_plan.runway_name != NULL) {
		for (int i = 0; i < num_flights; i++) {
			if (h_landing_taxi_plan.runway_name[i] != NULL) {
				free(h_landing_taxi_plan.runway_name[i]);
				h_landing_taxi_plan.runway_name[i] = NULL;
			}
		}

		free(h_landing_taxi_plan.runway_name);
		h_landing_taxi_plan.runway_name = NULL;
	}
	if (h_landing_taxi_plan.taxi_tas_knots != NULL) {
		free(h_landing_taxi_plan.taxi_tas_knots);
		h_landing_taxi_plan.taxi_tas_knots = NULL;
	}
	if (h_landing_taxi_plan.ramp_tas_knots != NULL) {
		free(h_landing_taxi_plan.ramp_tas_knots);
		h_landing_taxi_plan.ramp_tas_knots = NULL;
	}
	if (h_landing_taxi_plan.runway_entry_latitude_geoStyle != NULL) {
		free(h_landing_taxi_plan.runway_entry_latitude_geoStyle);
		h_landing_taxi_plan.runway_entry_latitude_geoStyle = NULL;
	}
	if (h_landing_taxi_plan.runway_entry_longitude_geoStyle != NULL) {
		free(h_landing_taxi_plan.runway_entry_longitude_geoStyle);
		h_landing_taxi_plan.runway_entry_longitude_geoStyle = NULL;
	}
	if (h_landing_taxi_plan.runway_end_latitude_geoStyle != NULL) {
		free(h_landing_taxi_plan.runway_end_latitude_geoStyle);
		h_landing_taxi_plan.runway_end_latitude_geoStyle = NULL;
	}
	if (h_landing_taxi_plan.runway_end_longitude_geoStyle != NULL) {
		free(h_landing_taxi_plan.runway_end_longitude_geoStyle);
		h_landing_taxi_plan.runway_end_longitude_geoStyle = NULL;
	}

	if (d_aircraft_soa.sector_index) {
		cuda_free(d_aircraft_soa.sector_index);
		d_aircraft_soa.sector_index = NULL;
	}
	if (d_aircraft_soa.latitude_deg) {
		cuda_free(d_aircraft_soa.latitude_deg);
		d_aircraft_soa.latitude_deg = NULL;
	}
	if (d_aircraft_soa.longitude_deg) {
		cuda_free(d_aircraft_soa.longitude_deg);
		d_aircraft_soa.longitude_deg = NULL;
	}
	if (d_aircraft_soa.altitude_ft) {
		cuda_free(d_aircraft_soa.altitude_ft);
		d_aircraft_soa.altitude_ft = NULL;
	}
	if (d_aircraft_soa.rocd_fps) {
		cuda_free(d_aircraft_soa.rocd_fps);
		d_aircraft_soa.rocd_fps = NULL;
	}
	if (d_aircraft_soa.tas_knots) {
		cuda_free(d_aircraft_soa.tas_knots);
		d_aircraft_soa.tas_knots = NULL;
	}
	if (d_aircraft_soa.tas_knots_ground) {
		cuda_free(d_aircraft_soa.tas_knots_ground);
		d_aircraft_soa.tas_knots_ground = NULL;
	}
	if (d_aircraft_soa.course_rad) {
		cuda_free(d_aircraft_soa.course_rad);
		d_aircraft_soa.course_rad = NULL;
	}
	if (d_aircraft_soa.fpa_rad) {
		cuda_free(d_aircraft_soa.fpa_rad);
		d_aircraft_soa.fpa_rad = NULL;
	}
	if (d_aircraft_soa.flight_phase) {
		cuda_free(d_aircraft_soa.flight_phase);
		d_aircraft_soa.flight_phase = NULL;
	}
	if (d_aircraft_soa.departure_time_sec) {
		cuda_free(d_aircraft_soa.departure_time_sec);
		d_aircraft_soa.departure_time_sec = NULL;
	}
	if (d_aircraft_soa.cruise_alt_ft) {
		cuda_free(d_aircraft_soa.cruise_alt_ft);
		d_aircraft_soa.cruise_alt_ft = NULL;
	}
	if (d_aircraft_soa.cruise_tas_knots) {
		cuda_free(d_aircraft_soa.cruise_tas_knots);
		d_aircraft_soa.cruise_tas_knots = NULL;
	}
	if (d_aircraft_soa.origin_airport_elevation_ft) {
		cuda_free(d_aircraft_soa.origin_airport_elevation_ft);
		d_aircraft_soa.origin_airport_elevation_ft = NULL;
	}
	if (d_aircraft_soa.destination_airport_elevation_ft) {
		cuda_free(d_aircraft_soa.destination_airport_elevation_ft);
		d_aircraft_soa.destination_airport_elevation_ft = NULL;
	}
	if (d_aircraft_soa.adb_aircraft_type_index) {
		cuda_free(d_aircraft_soa.adb_aircraft_type_index);
		d_aircraft_soa.adb_aircraft_type_index = NULL;
	}
	if (d_aircraft_soa.landed_flag) {
		cuda_free(d_aircraft_soa.landed_flag);
		d_aircraft_soa.landed_flag = NULL;
	}
	if (d_aircraft_soa.holding_started) {
		cuda_free(d_aircraft_soa.holding_started);
		d_aircraft_soa.holding_started = NULL;
	}
	if (d_aircraft_soa.holding_stopped) {
		cuda_free(d_aircraft_soa.holding_stopped);
		d_aircraft_soa.holding_stopped = NULL;
	}
	if (d_aircraft_soa.has_holding_pattern) {
		cuda_free(d_aircraft_soa.has_holding_pattern);
		d_aircraft_soa.has_holding_pattern = NULL;
	}
	if (d_aircraft_soa.hold_start_index) {
		cuda_free(d_aircraft_soa.hold_start_index);
		d_aircraft_soa.hold_start_index = NULL;
	}
	if (d_aircraft_soa.hold_end_index) {
		cuda_free(d_aircraft_soa.hold_end_index);
		d_aircraft_soa.hold_end_index = NULL;
	}
	if (d_aircraft_soa.holding_tas_knots) {
		cuda_free(d_aircraft_soa.holding_tas_knots);
		d_aircraft_soa.holding_tas_knots = NULL;
	}
	if (d_aircraft_soa.target_waypoint_index) {
		cuda_free(d_aircraft_soa.target_waypoint_index);
		d_aircraft_soa.target_waypoint_index = NULL;
	}
	if (d_aircraft_soa.target_altitude_ft) {
		cuda_free(d_aircraft_soa.target_altitude_ft);
		d_aircraft_soa.target_altitude_ft = NULL;
	}
	if (d_aircraft_soa.toc_index) {
		cuda_free(d_aircraft_soa.toc_index);
		d_aircraft_soa.toc_index = NULL;
	}
	if (d_aircraft_soa.tod_index) {
		cuda_free(d_aircraft_soa.tod_index);
		d_aircraft_soa.tod_index = NULL;
	}

	if (d_aircraft_soa.runway_name_departing) {
		for (int i = 0; i < num_flights; i++) {
			if (d_aircraft_soa.runway_name_departing[i] != NULL) {
				free(d_aircraft_soa.runway_name_departing[i]);
				d_aircraft_soa.runway_name_departing[i] = NULL;
			}
		}

		d_aircraft_soa.runway_name_departing = NULL;
	}
	if (d_aircraft_soa.runway_name_landing) {
		for (int i = 0; i < num_flights; i++) {
			if (d_aircraft_soa.runway_name_landing[i] != NULL) {
				free(d_aircraft_soa.runway_name_landing[i]);
				d_aircraft_soa.runway_name_landing[i] = NULL;
			}
		}

		d_aircraft_soa.runway_name_landing = NULL;
	}

	// ========================================================================

	if (array_Airborne_Flight_Plan_ptr != NULL) {
		waypoint_node_t* tmp_cur_waypointNode_ptr;
		waypoint_node_t* tmp_next_waypointNode_ptr;

		for (int i = 0; i < num_flights; i++) {
			if (array_Airborne_Flight_Plan_ptr[i] != NULL) {
				tmp_next_waypointNode_ptr = array_Airborne_Flight_Plan_ptr[i];
				while (tmp_next_waypointNode_ptr != NULL) {
					tmp_cur_waypointNode_ptr = tmp_next_waypointNode_ptr;

					releaseWaypointNodeContent(tmp_cur_waypointNode_ptr);

					tmp_cur_waypointNode_ptr->prev_node_ptr = NULL;
					tmp_next_waypointNode_ptr = tmp_cur_waypointNode_ptr->next_node_ptr;
					tmp_cur_waypointNode_ptr->next_node_ptr = NULL;

					free(tmp_cur_waypointNode_ptr);
				}

				array_Airborne_Flight_Plan_ptr[i] = NULL;
			}
		}

		free(array_Airborne_Flight_Plan_ptr);
		array_Airborne_Flight_Plan_ptr = NULL;
	}

	if (array_Airborne_Flight_Plan_toc_ptr != NULL) {
		for (int i = 0; i < num_flights; i++) {
			if (array_Airborne_Flight_Plan_toc_ptr[i] != NULL)
				array_Airborne_Flight_Plan_toc_ptr[i] = NULL;
		}

		free(array_Airborne_Flight_Plan_toc_ptr);
		array_Airborne_Flight_Plan_toc_ptr = NULL;
	}
	if (array_Airborne_Flight_Plan_tod_ptr != NULL) {
		for (int i = 0; i < num_flights; i++) {
			if (array_Airborne_Flight_Plan_tod_ptr[i] != NULL)
				array_Airborne_Flight_Plan_tod_ptr[i] = NULL;
		}

		free(array_Airborne_Flight_Plan_tod_ptr);
		array_Airborne_Flight_Plan_tod_ptr = NULL;
	}
	if (array_Airborne_Flight_Plan_Final_Node_ptr != NULL) {
		for (int i = 0; i < num_flights; i++) {
			if (array_Airborne_Flight_Plan_Final_Node_ptr[i] != NULL)
				array_Airborne_Flight_Plan_Final_Node_ptr[i] = NULL;
		}

		free(array_Airborne_Flight_Plan_Final_Node_ptr);
		array_Airborne_Flight_Plan_Final_Node_ptr = NULL;
	}
	if (array_Airborne_Flight_Plan_Waypoint_length != NULL) {
		free(array_Airborne_Flight_Plan_Waypoint_length);
		array_Airborne_Flight_Plan_Waypoint_length = NULL;
	}

	// Release memory space
	if (g_humanError_Controller != NULL) {
		free(g_humanError_Controller[0]);
		g_humanError_Controller[0] = NULL;

		free(g_humanError_Controller);
		g_humanError_Controller = NULL;
	}

	// Release memory space
	if (g_humanError_Pilot) {
		for (int i = 0; i < num_flights; i++) {
			if (g_humanError_Pilot[i].size() > 0) {
				g_humanError_Pilot[i].clear();
			}
		}

		free(g_humanError_Pilot);
		g_humanError_Pilot = NULL;
	}

	num_flights = 0;

	printf("Flight data released.\n");

	return 0;
}

int get_num_flights() {
	return num_flights;
}

void reset_num_flights() {
	num_flights = 0;
}

int select_flightSeq_by_aircraftId(string acid) {
	int ret_FlightSeq = -1;

	map<string, int>::iterator iterator_map_Acid_FlightSeq;
	iterator_map_Acid_FlightSeq = map_Acid_FlightSeq.find(acid);
	if (iterator_map_Acid_FlightSeq != map_Acid_FlightSeq.end()) {
		ret_FlightSeq = iterator_map_Acid_FlightSeq->second; // Got flight sequence
	}

	return ret_FlightSeq;
}
