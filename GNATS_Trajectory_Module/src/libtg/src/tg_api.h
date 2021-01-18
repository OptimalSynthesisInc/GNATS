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

/*
 * tg_api.h
 *
 *  Created on: Sep 17, 2013
 *      Author: jason
 */

#ifndef TG_API_H_
#define TG_API_H_

#include <vector>
#include <map>
#include <set>
#include <string>
#include <cstdio>

#include "tg_airports.h"
#include "tg_flightplan.h"
#include "tg_trajectory.h"
#include "tg_aircraft.h"



using std::vector;
using std::map;
using std::set;
using std::string;

// host-side globals
extern char* GNATS_SERVER_HOME;
extern string g_data_dir;
extern string g_trx_file;
extern string g_mfl_file;
extern string g_cifp_file;
extern string g_wind_dir;
extern string g_out_file;
extern string g_share_dir;
extern real_t g_perturbation;
extern long g_horizon;
extern long g_step_size;
extern long g_step_size_airborne;
extern int g_device_id;
extern bool g_verbose;
extern bool g_output_kml;
extern long g_start_time;
extern long g_start_time_realTime_simulation;
extern bool g_flag_standalone_mode;
extern bool g_flag_gdb;

// Flag variable indicating whether ADB data is available
extern bool flag_adb_available;
// Flag variable indicating whether RAP data is available
extern bool flag_rap_available;
// Flag variable indicating whether Sector data is available
extern bool flag_sector_available;
// Flag variable indicating whether Waypoint data is available
extern bool flag_waypoint_available;
// Flag variable indicating whether Airport data is available
extern bool flag_airport_available;
// Flag variable indicating whether Airway data is available
extern bool flag_airway_available;
// Flag variable indicating whether SID, STAR, Approach data is available
extern bool flag_SidStarApp_available;

extern map<string, NatsAirport*> map_airport;
extern map<string, FlightPlan*> map_flightplan;

typedef enum _tg_output_type {
    TG_OUTPUT_H5=0,
    TG_OUTPUT_XML,
    TG_OUTPUT_XML_GZ
} tg_output_type;

// tg predictor interface functions
int tg_info();
int tg_init(const string& data_dir=g_data_dir, const real_t& cruise_tas_perturbation=g_perturbation, const int& device_id=g_device_id);
int tg_load_rap(const string& grib_file);
int tg_load_trx(const string& trx_file, const string& mfl_file);
int tg_generate(const long& t_horizon_minutes, const long& t_step_sec, vector<Trajectory>* const trajectories=NULL);
int tg_reset_aircraft();
int tg_reset();
int tg_shutdown();

// tg aircraft interface functions
int tg_get_num_flights(int* const num_flights);
int tg_get_flightplans(map<int, FlightPlan>* const flightplans);
int tg_get_flightplan(const int& flight_index, FlightPlan* const fp);
int tg_get_cruise_tas(const int& flight_index, real_t* const cruise_tas);
int tg_get_cruise_altitude(const int& flight_index, real_t* const cruise_alt);
int tg_get_origin_airport(const int& flight_index, string* const airport);
int tg_get_origin_elevation(const int& flight_index, real_t* const elev);
int tg_get_destination_airport(const int& flight_index, string* const airport);
int tg_get_destination_elevation(const int& flight_index, real_t* const elev);
int tg_get_aircraft_type(const int& flight_index, string* const actype);

// tg trajectory interface functions
int tg_get_trajectories(vector<Trajectory>* const trajectories);
int tg_write_trajectories(const string& fname, const vector<Trajectory>& trajectories);
int tg_read_trajectories(const string& fname, vector<Trajectory>* const trajectories, const set<string>& callsign_filter=set<string>());




#endif /* TG_API_H_ */
