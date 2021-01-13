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
 * tg_airports.h
 *
 *  Created on: Sep 17, 2013
 *      Author: jason
 */

#ifndef TG_AIRPORTS_H_
#define TG_AIRPORTS_H_

#include "NatsAirport.h"
#include "GroundWaypointConnectivity.h"
#include "PointWGS84.h"

#include <string>
#include <vector>
#include "pub_TaxiPlan.h"

using std::string;
using std::vector;

#define MAX_AIRPORT_CODE_LENGTH 8
#define MAX_AIRPORT_NAME_LENGTH 23
#define MAX_AIRPORTS            34479

extern vector<NatsAirport> g_airports;

extern taxi_plan_t h_departing_taxi_plan;
extern taxi_plan_t h_landing_taxi_plan;

extern const char* WAYPOINT_ID_V2POINT;
extern const char* WAYPOINT_ID_TOUCHDOWNPOINT;

extern const double DEFAULT_RAMP_TAS_KNOTS;
extern const double DEFAULT_TAXI_TAS_KNOTS;

int load_airports(const string& nats_data_dir, const string& cifp_file="");

double getGroundWaypointLatLon(AirportNodeLink* const airport_node_link, const char* waypoint_id, const int flag_lat_lon);

waypoint_node_t* getSurface_waypoint_node_linkedList(const string airport_code, const vector<string> user_defined_waypoint_ids, double (*v2_or_touchdown_point_lat_lon)[2]);

waypoint_node_t* getSurface_waypoint_node_linkedList_geoStyle(const vector<PointWGS84> user_defined_geoPoints);

void calculate_surface_taxi_plan_waypoint_lat_lon(const int index_flight, const string& airport_code, const int cnt_waypoint, const bool flagDeparting);

int generate_and_load_surface_taxi_plan(const int index_flight, const string airport_code, const string startNode_waypoint_id, const string endNode_waypoint_id, const string runway_name);

int generate_and_load_surface_taxi_plan(const int index_flight, const string airport_code, const string startNode_waypoint_id, const string endNode_waypoint_id, const string vrNode_waypoint_id, const string touchdownNode_waypoint_id);

int generate_and_load_surface_taxi_plan(const int index_flight, const string airport_code, const string startNode_waypoint_id, const string endNode_waypoint_id, double v2Point_lat_lon[2], double touchdownPoint_lat_lon[2]);

int set_user_defined_surface_taxi_plan(const int index_flight, const string airport_code, const vector<string> user_defined_waypoint_ids);

int set_user_defined_surface_taxi_plan(const int index_flight, const string airport_code, const vector<string> user_defined_waypoint_ids, double v2_or_touchdown_point_lat_lon[2]);

waypoint_node_t* produce_surface_waypoint_node_linkedList(const string id, const string airport_code, string str_surface_plan);

#endif /* TG_AIRPORTS_H_ */
