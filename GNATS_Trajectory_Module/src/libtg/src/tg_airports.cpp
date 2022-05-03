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
 * tg_airports.cpp
 *
 *  Created on: Sep 17, 2013
 *      Author: jason
 */

#include "tg_api.h"
#include "tg_aircraft.h"
#include "tg_airports.h"
#include "tg_flightplan.h"
#include "tg_simulation.h"
#include "AirportLayoutDataLoader.h"
#include "NatsAirport.h"
#include "NatsDataLoader.h"

#include "geometry_utils.h"

#include "rg_api.h"

#include "util_string.h"

#include "json.hpp"

#include <algorithm>
#include <float.h>
#include <string>
#include <vector>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <unistd.h>

using namespace std;
using namespace osi;
using json = nlohmann::json;

/*
 * host-side global variables
 */

taxi_plan_t h_departing_taxi_plan;
taxi_plan_t h_landing_taxi_plan;

const char* WAYPOINT_ID_V2POINT = "_v2Point_";
const char* WAYPOINT_ID_TOUCHDOWNPOINT = "_touchdownPoint_";

const double DEFAULT_RAMP_TAS_KNOTS = 5.0;
const double DEFAULT_TAXI_TAS_KNOTS = 18.0;

bool flag_airport_available = false;

void debug_airport() {
	vector<NatsAirport>::iterator ite_airports;
	for (ite_airports = g_airports.begin(); ite_airports != g_airports.end(); ite_airports++) {
		printf("Airport code = %s, mag_variation = %f\n", ite_airports->code.c_str(), ite_airports->mag_variation);
	}
}

void debug_airport_layout() {
// Show map_ground_waypoint_connectivity data
	GroundWaypointConnectivity tmpGroundWaypointConnectivity;

	map<string, GroundWaypointConnectivity>::iterator iteConn;
	for (iteConn = map_ground_waypoint_connectivity.begin(); iteConn != map_ground_waypoint_connectivity.end(); iteConn++) {
		tmpGroundWaypointConnectivity = iteConn->second;

		printf("Airport = %s, map_waypoint_node.size() = %d\n", iteConn->first.c_str(), tmpGroundWaypointConnectivity.map_waypoint_node.size());

		AirportNodeLink curAirportNodeLink = tmpGroundWaypointConnectivity.airport_node_link;
		for (unsigned int i = 0; i < curAirportNodeLink.n1_id.size(); i++) {
//			printf("	ground_way_id = %s", curAirportNodeLink.ground_way_id.at(i).c_str());
//			printf(", index = %d", curAirportNodeLink.index.at(i));
//			printf(", type = %s", curAirportNodeLink.type.at(i).c_str());
//			printf(", yEdId = %s", curAirportNodeLink.yEdId.at(i).c_str());
//			printf(", annotation = %s", curAirportNodeLink.annotation.at(i).c_str());
//
			printf("	n1_id = %s", curAirportNodeLink.n1_id.at(i).c_str());
//			printf(", n1_index = %d", curAirportNodeLink.n1_index.at(i));
//			printf(", n1_latitude = %.14f", curAirportNodeLink.n1_latitude.at(i));
//			printf(", n1_longitude = %.14f", curAirportNodeLink.n1_longitude.at(i));
//			printf(", n1_yEdId = %s", curAirportNodeLink.n1_yEdId.at(i).c_str());
//			printf(", n1_annotation = %s", curAirportNodeLink.n1_annotation.at(i).c_str());
//			printf(", n1_parent_id = %s", curAirportNodeLink.n1_parent_id.at(i).c_str());
//			printf(", n1_param1 = %d", curAirportNodeLink.n1_param1.at(i));
//
//			printf(", n2_id = %s", curAirportNodeLink.n2_id.at(i).c_str());
//			printf(", n2_index = %d", curAirportNodeLink.n2_index.at(i));
//			printf(", n2_latitude = %.14f", curAirportNodeLink.n2_latitude.at(i));
//			printf(", n2_longitude = %.14f", curAirportNodeLink.n2_longitude.at(i));
//			printf(", n2_yEdId = %s", curAirportNodeLink.n2_yEdId.at(i).c_str());
//			printf(", n2_annotation = %s", curAirportNodeLink.n2_annotation.at(i).c_str());
//			printf(", n2_parent_id = %s", curAirportNodeLink.n2_parent_id.at(i).c_str());
//			printf(", n2_param1 = %d", curAirportNodeLink.n2_param1.at(i));
//
			printf("\n");
		}
	}
}

void debug_airport_connectivity() {
	// Debug: 2018.03.13 Oliver Chen
printf("debug_airport_connectivity() --> map_ground_waypoint_connectivity Total size = %d\n", map_ground_waypoint_connectivity.size());

	unsigned long long count_links;

	map<string, GroundWaypointConnectivity>::iterator ite;
	for (ite = map_ground_waypoint_connectivity.begin(); ite != map_ground_waypoint_connectivity.end(); ite++) {
		count_links = 0;

		GroundWaypointConnectivity tmpGroundWaypointConnectivity = ite->second;
		int size_map_waypoint_node = tmpGroundWaypointConnectivity.map_waypoint_node.size();

		if (size_map_waypoint_node > 0) {
			for (int i = 0; i < size_map_waypoint_node; i++) {
				for (int j = 0; j < size_map_waypoint_node; j++) {
					if (tmpGroundWaypointConnectivity.connectivity[i][j] == 1) {
						count_links++; // Increment count of links
					}
				}
			}
		}

		printf("	count_links = %d\n", count_links);
	}
}

string get_closest_waypoint_id(const string airport_code, const double point_lat_lon[2]) {
	string retString = "Not found"; // Default value

	GroundWaypointConnectivity tmpGroundWaypointConnectivity = map_ground_waypoint_connectivity.at(airport_code);

	int num_nodes = tmpGroundWaypointConnectivity.map_waypoint_node.size();
	if (num_nodes > 0) {
		double shortestDistance = DBL_MAX;

		map<string, AirportNode>::iterator ite;
		for (ite = tmpGroundWaypointConnectivity.map_waypoint_node.begin(); ite != tmpGroundWaypointConnectivity.map_waypoint_node.end(); ite++) {
			double tmpDistance = compute_distance_gc(ite->second.latitude,
									ite->second.longitude,
									point_lat_lon[0],
									point_lat_lon[1],
									0);
			if (tmpDistance< shortestDistance) {
				retString.assign(ite->first);
				shortestDistance = tmpDistance;
			}
		}
	}

	return retString;
}

int load_airports(const string& data_dir, const string& cifp_file) {
	printf("  Loading airport data\n");

	g_airports.clear();

#if (!USE_CIFP)
	// If we can't access the directory
	if ((data_dir.length() == 0) || ( access( data_dir.c_str(), F_OK ) == -1 )) {
		printf("      Failed to open directory %s\n", data_dir.c_str());

		return -1;
	}

	string fname = data_dir + "/Airports.crypt";
#else
	//FIXME:THERE HAS TO BE SOMEWAY OF INPUTTING THIS AUTOMATICALLY. NOW... HARDCODED
	string fname = "";
	if(cifp_file != "") {
	    fname = cifp_file;
	}
#endif

	NatsDataLoader loader;
	int err = loader.loadAirports(fname, &g_airports);

	if (err <= 0) {
		printf("Error loading airports\n");

		// Detach the current thread
		pthread_detach(pthread_self());

		return err;
	}

	// Load airport layout data
	fname = g_share_dir + "/libairport_layout/Airport_Rwy";
	AirportLayoutDataLoader airportLayoutDataLoader;
	err = airportLayoutDataLoader.loadAirportLayout(fname);
	if (err <= 0) {
		printf("Error loading airport layout\n");

		// Detach the current thread
		pthread_detach(pthread_self());

		return err;
	}

	flag_airport_available = true;

	//debug_airport();

	//debug_airport_layout();

	//debug_airport_connectivity();

	// Detach the current thread
	pthread_detach(pthread_self());

	return 0;
}

string get_runwayName_from_taxi_plan(const int index_flight, const bool flagDeparting) {
	string retString;

	waypoint_node_t* tmpWaypoint_Node_ptr = NULL;

	if (flagDeparting) { // Departing
		string departureAirportCode = g_flightplans[index_flight].origin;
		string tmpAirportCode;

		double heading_threshold_v2;

		int waypoint_length = h_departing_taxi_plan.waypoint_length[index_flight];
		if (waypoint_length > 1) {
			tmpWaypoint_Node_ptr = h_departing_taxi_plan.waypoint_node_ptr[index_flight];
			int i = 1;
			while (i < waypoint_length-1) {
				tmpWaypoint_Node_ptr = tmpWaypoint_Node_ptr->next_node_ptr;

				i++;
			}

			double threshold_waypoint_latitude = tmpWaypoint_Node_ptr->latitude;
			double threshold_waypoint_longitude = tmpWaypoint_Node_ptr->longitude;

			double v2_point_latitude = h_departing_taxi_plan.waypoint_final_node_ptr[index_flight]->latitude;
			double v2_point_longitude = h_departing_taxi_plan.waypoint_final_node_ptr[index_flight]->longitude;

			if ((v2_point_latitude != 0.0) && (v2_point_longitude != 0.0)) {
				heading_threshold_v2 = compute_heading_gc(threshold_waypoint_latitude, threshold_waypoint_longitude, v2_point_latitude, v2_point_longitude);

				// Get the vector of all runways of this airport
				tmpAirportCode.assign(departureAirportCode);
				if (tmpAirportCode.length() < 4)
					tmpAirportCode.insert(0, "K");
				vector<AirportNode> resultVector = getVector_AllRunways(tmpAirportCode);
				if (resultVector.size() == 0) {
					tmpAirportCode.assign(departureAirportCode);
					if (tmpAirportCode.length() < 4)
						tmpAirportCode.insert(0, "P");
					resultVector = getVector_AllRunways(tmpAirportCode);
				}

				string matchedRunway;
				double minHeading = DBL_MAX;

				if (resultVector.size() > 0) {
					for (unsigned int i = 0; i < resultVector.size(); i++) {
						threshold_waypoint_latitude = resultVector.at(i).latitude;
						threshold_waypoint_longitude = resultVector.at(i).longitude;

						double tmpHeading = compute_heading_gc(threshold_waypoint_latitude, threshold_waypoint_longitude, v2_point_latitude, v2_point_longitude);
						if ((abs(heading_threshold_v2 - tmpHeading) <= 3.0) && (tmpHeading < minHeading)) {
							matchedRunway.assign(resultVector.at(i).refName1);
							minHeading = tmpHeading; // Update
						}
					}

					// If the matched runway is found
					if (matchedRunway.length() > 0) {
						retString.assign(matchedRunway);
					}
				}
			}
		}
	} else { // Landing
		string arrivalAirportCode = g_flightplans[index_flight].destination;
		string tmpAirportCode;

		double heading_touchdown_threshold;

		int waypoint_length = h_landing_taxi_plan.waypoint_length[index_flight];
		if (waypoint_length > 1) {
			double touchdown_point_latitude = h_landing_taxi_plan.waypoint_node_ptr[index_flight]->latitude;
			double touchdown_point_longitude = h_landing_taxi_plan.waypoint_node_ptr[index_flight]->longitude;

			tmpWaypoint_Node_ptr = h_landing_taxi_plan.waypoint_node_ptr[index_flight];
			tmpWaypoint_Node_ptr = tmpWaypoint_Node_ptr->next_node_ptr;

			double threshold_waypoint_latitude = tmpWaypoint_Node_ptr->latitude;
			double threshold_waypoint_longitude = tmpWaypoint_Node_ptr->longitude;

			if ((touchdown_point_latitude != 0.0) && (touchdown_point_longitude != 0.0)) {
				heading_touchdown_threshold = compute_heading_gc(touchdown_point_latitude, touchdown_point_longitude, threshold_waypoint_latitude, threshold_waypoint_longitude);

				// Get the vector of all runways of this airport
				tmpAirportCode.assign(arrivalAirportCode);
				if (tmpAirportCode.length() < 4)
					tmpAirportCode.insert(0, "K");
				vector<AirportNode> resultVector = getVector_AllRunways(tmpAirportCode);
				if (resultVector.size() == 0) {
					tmpAirportCode.assign(arrivalAirportCode);
					if (tmpAirportCode.length() < 4)
						tmpAirportCode.insert(0, "P");
					resultVector = getVector_AllRunways(tmpAirportCode);
				}

				string matchedRunway;
				double minHeading = DBL_MAX;

				if (resultVector.size() > 0) {
					for (unsigned int i = 0; i < resultVector.size(); i++) {

						threshold_waypoint_latitude = resultVector.at(i).latitude;
						threshold_waypoint_longitude = resultVector.at(i).longitude;

						double tmpHeading = compute_heading_gc(touchdown_point_latitude, touchdown_point_longitude, threshold_waypoint_latitude, threshold_waypoint_longitude);

						if ((abs(heading_touchdown_threshold - tmpHeading) <= 3.0) && (tmpHeading < minHeading)) {
							matchedRunway.assign(resultVector.at(i).refName1);
							minHeading = tmpHeading; // Update
						}
					}

					// If the matched runway is found
					if (matchedRunway.length() > 0) {
						//retObject = jniEnv->NewStringUTF(matchedRunway.c_str());
						retString.assign(matchedRunway);
					}
				}
			}
		}
	}

	return retString;
}

double getGroundWaypointLatLon(AirportNodeLink* const airport_node_link, const char* waypoint_id, const int flag_lat_lon) {
	double retValue = 0.0;
	bool flagFound = false;

	for (unsigned int i = 0; i < airport_node_link->n1_id.size(); i++) {
		if ((airport_node_link->n1_id.at(i).length() > 0) && (waypoint_id != NULL) && (strcmp(airport_node_link->n1_id.at(i).c_str(), waypoint_id) == 0)) {
			if (flag_lat_lon == 0) {
				retValue = airport_node_link->n1_latitude.at(i);
			} else {
				retValue = airport_node_link->n1_longitude.at(i);
			}

			flagFound = true;

			break;
		}
	}

	if (!flagFound) {
		for (unsigned int i = 0; i < airport_node_link->n2_id.size(); i++) {
			if ((airport_node_link->n2_id.at(i).length() > 0) && (waypoint_id != NULL) && (strcmp(airport_node_link->n2_id.at(i).c_str(), waypoint_id) == 0)) {
				if (flag_lat_lon == 0) {
					retValue = airport_node_link->n2_latitude.at(i);
				} else {
					retValue = airport_node_link->n2_longitude.at(i);
				}

				flagFound = true;

				break;
			}
		}
	}

	return retValue;
}

bool** getConnectivity_remove_intermediate_links_to_runway(bool** srcConnectivity, const string airport_code) {
	bool** retConnectivity;

	GroundWaypointConnectivity tmpGroundWaypointConnectivity = map_ground_waypoint_connectivity.at(airport_code);

	int num_waypoints = tmpGroundWaypointConnectivity.map_waypoint_node.size();

	retConnectivity = (bool**)calloc(num_waypoints, sizeof(bool*));
	for (unsigned int i = 0; i < num_waypoints; i++) {
		// Allocate space for each bool* array element
		retConnectivity[i] = (bool*)calloc(num_waypoints, sizeof(bool));
	}

	// Duplicate connectivity data
	for (unsigned int i = 0; i < num_waypoints; i++) {
		for (unsigned int j = 0; j < num_waypoints; j++) {
			retConnectivity[i][j] = srcConnectivity[i][j];
		}
	}

	set<string> setAllRunways = getAllRunways(airport_code);
	if (setAllRunways.size() > 0) {
		int tmpNode_seq;

		set<string>::iterator ite_setAllRunways;
		for (ite_setAllRunways = setAllRunways.begin(); ite_setAllRunways != setAllRunways.end(); ite_setAllRunways++) {
			string tmpRunwayName = *ite_setAllRunways;

			// Get all waypoint Ids of the runway
			vector<string> vectorAllRunwayWaypoints = getVector_AllRunwayWaypoints(airport_code, tmpRunwayName);
			if (vectorAllRunwayWaypoints.size() > 0) {
				for (unsigned int i = 0; i < vectorAllRunwayWaypoints.size(); i++) {
					// For "not first" and "not final" waypoint, remove all links connecting to it.
					// This will remove all intermediate links connecting to the runway except for the starting and ending waypoint
					if ((i != 0) && (i != vectorAllRunwayWaypoints.size()-1)) {
						tmpNode_seq = tmpGroundWaypointConnectivity.map_waypoint_node.at(vectorAllRunwayWaypoints.at(i)).index;

						// Remove all links connecting to this waypoint
						for (unsigned int j = 0; j < num_waypoints; j++) {
							retConnectivity[tmpNode_seq][j] = 0;
							retConnectivity[j][tmpNode_seq] = 0;
						}
					}
				}
			}
		}
	}

	return retConnectivity;
}

/**
 * Get customized connectivity matrix which sets one-way links from waypoint A to B.
 */
bool** getConnectivity_one_way_from_A_To_B(const int index_flight, const string airport_code, bool** srcConnectivity, const string startNode_waypoint_id, const string endNode_waypoint_id) {
	bool** retConnectivity;

	GroundWaypointConnectivity tmpGroundWaypointConnectivity = map_ground_waypoint_connectivity.at(airport_code);

	int num_waypoints = tmpGroundWaypointConnectivity.map_waypoint_node.size();

	retConnectivity = (bool**)calloc(num_waypoints, sizeof(bool*));
	for (int i = 0; i < num_waypoints; i++) {
		// Allocate space for each bool* array element
		retConnectivity[i] = (bool*)calloc(num_waypoints, sizeof(bool));
	}

	// Duplicate connectivity data
	for (int i = 0; i < num_waypoints; i++) {
		for (int j = 0; j < num_waypoints; j++) {
			retConnectivity[i][j] = srcConnectivity[i][j];
		}
	}

	int tmpCurNodeSeq;
	int tmpPrevNodeSeq;

	// Reverse the order of starting and ending waypoints.  Get the taxi route.
	vector<string> resultVector = get_taxi_route_from_A_To_B(airport_code, endNode_waypoint_id, startNode_waypoint_id);
	if (resultVector.size() > 0) {
		for (int i = 0; i < resultVector.size(); i++) {
			string tmpWaypointid = resultVector.at(i);
			tmpCurNodeSeq = tmpGroundWaypointConnectivity.map_waypoint_node.at(tmpWaypointid).index;

			if (i > 0) {
				// Disable the link
				retConnectivity[tmpPrevNodeSeq][tmpCurNodeSeq] = 0;
			}

			tmpPrevNodeSeq = tmpCurNodeSeq;
		}
	}

	return retConnectivity;
}

/**
 * Reset taxi plan data
 */
static void reset_surface_taxi_plan(const int index_flight, const bool flagDeparting, const TrxRecord& trxRecord) {
	waypoint_node_t tmpNode;
	waypoint_node_t* tmpWaypoint_Node_ptr = NULL;
	waypoint_node_t* tmpWaypoint_Final_Node_ptr = NULL;

	// Reset ground waypoint data
	// Reason. This function can be repeatedly called.  If the user call this function to generate taxi plan on the same aircraft and airport with different starting/ending waypoints, the result path routes will be different.  Need to reset it to clean up the left-over data.
	if (flagDeparting) { // Departing
		if (h_departing_taxi_plan.waypoint_length[index_flight] != 0) {
			printf("Aircraft %s is cleaning up existing departing taxi plan and processing new one.\n", trxRecord.acid.c_str());
		}

		tmpWaypoint_Node_ptr = h_departing_taxi_plan.waypoint_node_ptr[index_flight];
		tmpWaypoint_Final_Node_ptr = h_departing_taxi_plan.waypoint_final_node_ptr[index_flight];
		while (tmpWaypoint_Node_ptr != NULL) {
			tmpNode = *tmpWaypoint_Node_ptr;

			if ((tmpWaypoint_Node_ptr->wpname != NULL) && (tmpWaypoint_Node_ptr->wpname[0] != '\0')) {
				free(tmpWaypoint_Node_ptr->wpname);
				tmpWaypoint_Node_ptr->wpname = NULL;
			}

			tmpWaypoint_Node_ptr = tmpWaypoint_Node_ptr->next_node_ptr; // Update waypoint node pointer to the next node
			tmpNode.prev_node_ptr = NULL; // Update pointer to NULL
			tmpNode.next_node_ptr = NULL; // Update pointer to NULL
		}
	} else { // Landing
		if (h_landing_taxi_plan.waypoint_length[index_flight] != 0) {
			printf("Aircraft %s is cleaning up existing landing taxi plan and processing new one.\n", trxRecord.acid.c_str());
		}

		tmpWaypoint_Node_ptr = h_landing_taxi_plan.waypoint_node_ptr[index_flight];
		tmpWaypoint_Final_Node_ptr = h_landing_taxi_plan.waypoint_final_node_ptr[index_flight];
		while (tmpWaypoint_Node_ptr != NULL) {
			tmpNode = *tmpWaypoint_Node_ptr;

			if ((tmpWaypoint_Node_ptr->wpname != NULL) && (tmpWaypoint_Node_ptr->wpname[0] != '\0')) {
				free(tmpWaypoint_Node_ptr->wpname);
				tmpWaypoint_Node_ptr->wpname = NULL;
			}

			tmpWaypoint_Node_ptr = tmpWaypoint_Node_ptr->next_node_ptr; // Update waypoint node pointer to the next node
			tmpNode.prev_node_ptr = NULL; // Update pointer to NULL
			tmpNode.next_node_ptr = NULL; // Update pointer to NULL
		}
	}

	// Reset data
	if (flagDeparting) { // Departing
		if ((h_departing_taxi_plan.runway_name[index_flight] != NULL) && (h_departing_taxi_plan.runway_name[index_flight][0] != '\0')) {
			free(h_departing_taxi_plan.runway_name[index_flight]);
			h_departing_taxi_plan.runway_name[index_flight] = NULL;
		}

		h_departing_taxi_plan.waypoint_length[index_flight] = 0;
	} else { // Landing
		if ((h_landing_taxi_plan.runway_name[index_flight] != NULL) && (h_landing_taxi_plan.runway_name[index_flight][0] != '\0')) {
			free(h_landing_taxi_plan.runway_name[index_flight]);
			h_landing_taxi_plan.runway_name[index_flight] = NULL;
		}

		h_landing_taxi_plan.waypoint_length[index_flight] = 0;
	}
}

/**
 * Return a list of surface waypoint nodes from the given waypoint IDs.  This function serves both for departing and landing.
 */
waypoint_node_t* getSurface_waypoint_node_linkedList(const string airport_code, const vector<string> user_defined_waypoint_ids, double (*v2_or_touchdown_point_lat_lon)[2]) {
	waypoint_node_t* ret_ptr = NULL;

	waypoint_node_t* tmpWaypoint_Node_ptr = NULL;
	waypoint_node_t* tmpWaypoint_Final_Node_ptr = NULL;

	int cnt_waypoints = user_defined_waypoint_ids.size();
	if (cnt_waypoints > 0) {
		AirportNodeLink cur_airport_node_link = map_ground_waypoint_connectivity.at(airport_code).airport_node_link;

		tmpWaypoint_Node_ptr = NULL; // Reset
		tmpWaypoint_Final_Node_ptr = NULL; // Reset

		for (int i = 0; i < cnt_waypoints; i++) {
			string tmp_waypoint_id = user_defined_waypoint_ids[i];
			waypoint_node_t* newWaypoint_Node_ptr = (waypoint_node_t*)calloc(1, sizeof(waypoint_node_t));
			newWaypoint_Node_ptr->prev_node_ptr = NULL; // Reset
			newWaypoint_Node_ptr->next_node_ptr = NULL; // Reset

			newWaypoint_Node_ptr->wpname = (char*)calloc((tmp_waypoint_id.length()+1), sizeof(char));
			if (tmp_waypoint_id.length() > 0) {
				strcpy(newWaypoint_Node_ptr->wpname, tmp_waypoint_id.c_str());
				newWaypoint_Node_ptr->wpname[tmp_waypoint_id.length()] = '\0';
			} else {
				newWaypoint_Node_ptr->wpname[0] = '\0';
			}

			if (tmpWaypoint_Final_Node_ptr != NULL) {
				newWaypoint_Node_ptr->prev_node_ptr = tmpWaypoint_Final_Node_ptr;
				tmpWaypoint_Final_Node_ptr->next_node_ptr = newWaypoint_Node_ptr;
			}

			tmpWaypoint_Final_Node_ptr = newWaypoint_Node_ptr;

			if (tmpWaypoint_Node_ptr == NULL) {
				tmpWaypoint_Node_ptr = newWaypoint_Node_ptr;
			}
		}

		waypoint_node_t* tmpNode_ptr = tmpWaypoint_Node_ptr;

		// Calculate waypoint node data -- latitude, longitude, course_rad_to_next_node
		while (tmpNode_ptr != NULL) {
			tmpNode_ptr->latitude = getGroundWaypointLatLon(&cur_airport_node_link, tmpNode_ptr->wpname, 0);
			tmpNode_ptr->longitude = getGroundWaypointLatLon(&cur_airport_node_link, tmpNode_ptr->wpname, 1);

			if (tmpNode_ptr->prev_node_ptr != NULL) {
				// Calculate the heading angle from the previous waypoint to the current one.
				tmpNode_ptr->prev_node_ptr->course_rad_to_next_node = compute_heading_rad_gc(tmpNode_ptr->prev_node_ptr->latitude,
						tmpNode_ptr->prev_node_ptr->longitude,
						tmpNode_ptr->latitude,
						tmpNode_ptr->longitude);
			}

			tmpNode_ptr = tmpNode_ptr->next_node_ptr; // Update waypoint node pointer to the next node
		}
	}

	ret_ptr = tmpWaypoint_Node_ptr;

	return ret_ptr;
}

/**
 * Return a list of surface waypoint nodes from the given user-defined Geo-style point vector.  This function serves both for departing and landing.
 */
waypoint_node_t* getSurface_waypoint_node_linkedList_geoStyle(const vector<PointWGS84> user_defined_geoPoints) {
	waypoint_node_t* ret_ptr = NULL;

	waypoint_node_t* tmpWaypoint_Node_ptr = NULL;
	waypoint_node_t* tmpWaypoint_Final_Node_ptr = NULL;

	int cnt_waypoints = user_defined_geoPoints.size();
	if (cnt_waypoints > 0) {
		tmpWaypoint_Node_ptr = NULL; // Reset
		tmpWaypoint_Final_Node_ptr = NULL; // Reset

		for (int i = 0; i < cnt_waypoints; i++) {
			PointWGS84 tmp_geoPoint = user_defined_geoPoints[i];

			waypoint_node_t* newWaypoint_Node_ptr = (waypoint_node_t*)calloc(1, sizeof(waypoint_node_t));
			newWaypoint_Node_ptr->flag_geoStyle = true;
			newWaypoint_Node_ptr->prev_node_ptr = NULL; // Reset
			newWaypoint_Node_ptr->next_node_ptr = NULL; // Reset

			newWaypoint_Node_ptr->wptype = (char*)calloc((tmp_geoPoint.type.length()+1), sizeof(char));
			if (tmp_geoPoint.type.length() > 0) {
				strcpy(newWaypoint_Node_ptr->wptype, tmp_geoPoint.type.c_str());
				newWaypoint_Node_ptr->wptype[tmp_geoPoint.type.length()] = '\0';
			} else {
				newWaypoint_Node_ptr->wptype[0] = '\0';
			}

			newWaypoint_Node_ptr->latitude = tmp_geoPoint.latitude;
			newWaypoint_Node_ptr->longitude = tmp_geoPoint.longitude;

			if (tmpWaypoint_Final_Node_ptr != NULL) {
				newWaypoint_Node_ptr->prev_node_ptr = tmpWaypoint_Final_Node_ptr;
				tmpWaypoint_Final_Node_ptr->next_node_ptr = newWaypoint_Node_ptr;
			}

			tmpWaypoint_Final_Node_ptr = newWaypoint_Node_ptr;

			if (tmpWaypoint_Node_ptr == NULL) {
				tmpWaypoint_Node_ptr = newWaypoint_Node_ptr;
			}

			if (newWaypoint_Node_ptr->prev_node_ptr != NULL) {
				// Calculate the heading angle from the previous waypoint to the current one.
				newWaypoint_Node_ptr->prev_node_ptr->course_rad_to_next_node = compute_heading_rad_gc(newWaypoint_Node_ptr->prev_node_ptr->latitude,
						newWaypoint_Node_ptr->prev_node_ptr->longitude,
						newWaypoint_Node_ptr->latitude,
						newWaypoint_Node_ptr->longitude);
			}
		}
	}

	ret_ptr = tmpWaypoint_Node_ptr;

	return ret_ptr;
}

/**
 * Set taxi plan(departing or landing) by user-defined surface taxi waypoints.
 */
int set_user_defined_surface_taxi_plan(const int index_flight, const string airport_code, const vector<string> user_defined_waypoint_ids) {
	int retValue = 1; // Initial value.  1 means error

	bool flagDeparting = true; // Flag indicating it is a departing taxi plan or not.

	TrxRecord tmpTrxRecord = g_trx_records[index_flight];

	string airportOrigin = g_flightplans.at(index_flight).origin;
	string airportDestination = g_flightplans.at(index_flight).destination;

	if (airportOrigin.length() == 4) {
		airportOrigin = airportOrigin.substr(1, 3);
	}
	if (airportDestination.length() == 4) {
		airportDestination = airportDestination.substr(1, 3);
	}

	if ((airport_code.length() > 0) && ((airport_code.compare(1, 3, airportOrigin) == 0) || (airport_code.compare(1, 3, airportDestination) == 0))) {
		if (airport_code.compare(1, 3, airportOrigin) == 0) {
			flagDeparting = true;
		} else {
			flagDeparting = false;
		}

		waypoint_node_t tmpNode;
		waypoint_node_t* tmpWaypoint_Node_ptr = NULL;
		waypoint_node_t* tmpWaypoint_Final_Node_ptr = NULL;

		reset_surface_taxi_plan(index_flight, flagDeparting, tmpTrxRecord);

		int cnt_waypoints = user_defined_waypoint_ids.size();
		if (cnt_waypoints > 0) {
			AirportNodeLink cur_airport_node_link = map_ground_waypoint_connectivity.at(airport_code).airport_node_link;

			tmpWaypoint_Node_ptr = NULL; // Reset
			tmpWaypoint_Final_Node_ptr = NULL; // Reset

			for (int i = 0; i < cnt_waypoints; i++) {
				string tmp_waypoint_id = user_defined_waypoint_ids[i];
				waypoint_node_t* newWaypoint_Node_ptr = (waypoint_node_t*)calloc(1, sizeof(waypoint_node_t));
				newWaypoint_Node_ptr->prev_node_ptr = NULL; // Reset
				newWaypoint_Node_ptr->next_node_ptr = NULL; // Reset

				newWaypoint_Node_ptr->wpname = (char*)calloc((tmp_waypoint_id.length()+1), sizeof(char));
				if (tmp_waypoint_id.length() > 0) {
					strcpy(newWaypoint_Node_ptr->wpname, tmp_waypoint_id.c_str());
					newWaypoint_Node_ptr->wpname[tmp_waypoint_id.length()] = '\0';
				} else {
					newWaypoint_Node_ptr->wpname[0] = '\0';
				}

				if (tmpWaypoint_Final_Node_ptr != NULL) {
					newWaypoint_Node_ptr->prev_node_ptr = tmpWaypoint_Final_Node_ptr;
					tmpWaypoint_Final_Node_ptr->next_node_ptr = newWaypoint_Node_ptr;
				}

				tmpWaypoint_Final_Node_ptr = newWaypoint_Node_ptr;

				if (tmpWaypoint_Node_ptr == NULL) {
					tmpWaypoint_Node_ptr = newWaypoint_Node_ptr;
				}
			}

			if (flagDeparting) { // Departing
				h_departing_taxi_plan.waypoint_node_ptr[index_flight] = tmpWaypoint_Node_ptr;

				h_departing_taxi_plan.waypoint_final_node_ptr[index_flight] = tmpWaypoint_Final_Node_ptr;

				h_departing_taxi_plan.waypoint_length[index_flight] = cnt_waypoints;
			} else { // Landing
				h_landing_taxi_plan.waypoint_node_ptr[index_flight] = tmpWaypoint_Node_ptr;

				h_landing_taxi_plan.waypoint_final_node_ptr[index_flight] = tmpWaypoint_Final_Node_ptr;

				h_landing_taxi_plan.waypoint_length[index_flight] = cnt_waypoints;
			}

			calculate_surface_taxi_plan_waypoint_lat_lon(index_flight, airport_code, cnt_waypoints, flagDeparting);

			// Update V2 point or touchdown point latitude and longitude in d_aircraft_soa data structure
			if (flagDeparting) {
				array_Airborne_Flight_Plan_ptr[index_flight]->latitude = h_departing_taxi_plan.waypoint_final_node_ptr[index_flight]->latitude;
				array_Airborne_Flight_Plan_ptr[index_flight]->longitude = h_departing_taxi_plan.waypoint_final_node_ptr[index_flight]->longitude;

				if ((h_aircraft_soa.runway_name_departing[index_flight] != NULL) && (strlen(h_aircraft_soa.runway_name_departing[index_flight]) > 0)) {
					if (strlen(h_aircraft_soa.runway_name_departing[index_flight]) > 0) {
						h_departing_taxi_plan.runway_name[index_flight] = (char*)malloc((strlen(h_aircraft_soa.runway_name_departing[index_flight])+1) * sizeof(char));
						strcpy(h_departing_taxi_plan.runway_name[index_flight], h_aircraft_soa.runway_name_departing[index_flight]);
						h_departing_taxi_plan.runway_name[index_flight][strlen(h_aircraft_soa.runway_name_departing[index_flight])] = '\0';
					}
				}

				retValue = 0;
			} else {
				array_Airborne_Flight_Plan_Final_Node_ptr[index_flight]->latitude = h_landing_taxi_plan.waypoint_node_ptr[index_flight]->latitude;
				array_Airborne_Flight_Plan_Final_Node_ptr[index_flight]->longitude = h_landing_taxi_plan.waypoint_node_ptr[index_flight]->longitude;

				if ((h_aircraft_soa.runway_name_landing[index_flight] != NULL) && (strlen(h_aircraft_soa.runway_name_landing[index_flight]) > 0)) {
					if (strlen(h_aircraft_soa.runway_name_landing[index_flight]) > 0) {
						h_landing_taxi_plan.runway_name[index_flight] = (char*)malloc((strlen(h_aircraft_soa.runway_name_landing[index_flight])+1) * sizeof(char));
						strcpy(h_landing_taxi_plan.runway_name[index_flight], h_aircraft_soa.runway_name_landing[index_flight]);
						h_landing_taxi_plan.runway_name[index_flight][strlen(h_aircraft_soa.runway_name_landing[index_flight])] = '\0';
					}
				}

				retValue = 0;
			}
		}
	}

	return retValue;
}

/**
 * Set taxi plan(departing or landing) by user-defined surface taxi waypoints and V2 or touchdown point latitude and longitude.
 */
int set_user_defined_surface_taxi_plan(const int index_flight, const string airport_code, const vector<string> user_defined_waypoint_ids, double v2_or_touchdown_point_lat_lon[2]) {
	int retValue = 1; // Initial value.  1 means error

	bool flagDeparting = true; // Flag indicating it is a departing taxi plan or not.

	FlightPlan tmpFlightPlan = g_flightplans.at(index_flight);

	TrxRecord tmpTrxRecord = g_trx_records[index_flight];

	string airportOrigin = tmpFlightPlan.origin;
	string airportDestination = tmpFlightPlan.destination;

	if (airportOrigin.length() == 4) {
		airportOrigin = airportOrigin.substr(1, 3);
	}
	if (airportDestination.length() == 4) {
		airportDestination = airportDestination.substr(1, 3);
	}

	if ((airport_code.compare(1, 3, airportOrigin) == 0) || (airport_code.compare(1, 3, airportDestination) == 0)) {
		if (airport_code.compare(1, 3, airportOrigin) == 0) {
			flagDeparting = true;
		} else {
			flagDeparting = false;
		}

		waypoint_node_t tmpNode;
		waypoint_node_t* tmpWaypoint_Node_ptr = NULL;
		waypoint_node_t* tmpWaypoint_Final_Node_ptr = NULL;

		reset_surface_taxi_plan(index_flight, flagDeparting, tmpTrxRecord);

		int cnt_waypoints = user_defined_waypoint_ids.size();
		if (cnt_waypoints > 0) {
			AirportNodeLink cur_airport_node_link = map_ground_waypoint_connectivity.at(airport_code).airport_node_link;

			int v2_index_departing_taxi_plan;
			int touchdown_index_landing_taxi_plan;

			waypoint_node_t* newWaypoint_Node_ptr;

			for (int i = 0; i < cnt_waypoints; i++) {
				string tmp_waypoint_id = user_defined_waypoint_ids[i];

				newWaypoint_Node_ptr = (waypoint_node_t*)calloc(1, sizeof(waypoint_node_t));
				newWaypoint_Node_ptr->prev_node_ptr = NULL; // Reset
				newWaypoint_Node_ptr->next_node_ptr = NULL; // Reset

				newWaypoint_Node_ptr->wpname = (char*)calloc(tmp_waypoint_id.length(), sizeof(char));
				strcpy(newWaypoint_Node_ptr->wpname, tmp_waypoint_id.c_str());

				if (tmpWaypoint_Final_Node_ptr != NULL) {
					newWaypoint_Node_ptr->prev_node_ptr = tmpWaypoint_Final_Node_ptr;
					tmpWaypoint_Final_Node_ptr->next_node_ptr = newWaypoint_Node_ptr;
				}

				tmpWaypoint_Final_Node_ptr = newWaypoint_Node_ptr;

				if (tmpWaypoint_Node_ptr == NULL) {
					tmpWaypoint_Node_ptr = newWaypoint_Node_ptr;
				}
			}

			if (flagDeparting) { // Departing
				h_departing_taxi_plan.waypoint_node_ptr[index_flight] = tmpWaypoint_Node_ptr;

				h_departing_taxi_plan.waypoint_final_node_ptr[index_flight] = tmpWaypoint_Final_Node_ptr;

				h_departing_taxi_plan.waypoint_length[index_flight] = cnt_waypoints;
			} else { // Landing
				h_landing_taxi_plan.waypoint_node_ptr[index_flight] = tmpWaypoint_Node_ptr;

				h_landing_taxi_plan.waypoint_final_node_ptr[index_flight] = tmpWaypoint_Final_Node_ptr;

				h_landing_taxi_plan.waypoint_length[index_flight] = cnt_waypoints;
			}

			calculate_surface_taxi_plan_waypoint_lat_lon(index_flight, airport_code, cnt_waypoints, flagDeparting);

			// For latitude/longitude of v2Point and touchdownPoint, we need to handle them separately
			if (flagDeparting) {
				newWaypoint_Node_ptr = (waypoint_node_t*)calloc(1, sizeof(waypoint_node_t));
				newWaypoint_Node_ptr->prev_node_ptr = NULL; // Reset
				newWaypoint_Node_ptr->next_node_ptr = NULL; // Reset

				newWaypoint_Node_ptr->wpname = (char*)calloc(strlen(WAYPOINT_ID_V2POINT), sizeof(char));
				strcpy(newWaypoint_Node_ptr->wpname, WAYPOINT_ID_V2POINT);

				newWaypoint_Node_ptr->latitude = v2_or_touchdown_point_lat_lon[0];
				newWaypoint_Node_ptr->longitude = v2_or_touchdown_point_lat_lon[1];

				if (h_departing_taxi_plan.waypoint_final_node_ptr[index_flight] != NULL) {
					newWaypoint_Node_ptr->prev_node_ptr = h_departing_taxi_plan.waypoint_final_node_ptr[index_flight];
					h_departing_taxi_plan.waypoint_final_node_ptr[index_flight]->next_node_ptr = newWaypoint_Node_ptr;
				}

				h_departing_taxi_plan.waypoint_final_node_ptr[index_flight] = newWaypoint_Node_ptr;

				h_departing_taxi_plan.waypoint_length[index_flight] = cnt_waypoints + 1;

				array_Airborne_Flight_Plan_ptr[index_flight]->latitude = v2_or_touchdown_point_lat_lon[0];
				array_Airborne_Flight_Plan_ptr[index_flight]->longitude = v2_or_touchdown_point_lat_lon[1];

				char* runway_name = h_aircraft_soa.runway_name_departing[index_flight];
				if (runway_name != NULL) {
					h_departing_taxi_plan.runway_name[index_flight] = (char*)malloc(strlen(runway_name) * sizeof(char));
					strcpy(h_departing_taxi_plan.runway_name[index_flight], runway_name);
				}

				retValue = 0;  // Success
			} else {
				newWaypoint_Node_ptr = (waypoint_node_t*)calloc(1, sizeof(waypoint_node_t));
				newWaypoint_Node_ptr->prev_node_ptr = NULL; // Reset
				newWaypoint_Node_ptr->next_node_ptr = NULL; // Reset

				newWaypoint_Node_ptr->wpname = (char*)calloc(strlen(WAYPOINT_ID_TOUCHDOWNPOINT), sizeof(char));
				strcpy(newWaypoint_Node_ptr->wpname, WAYPOINT_ID_TOUCHDOWNPOINT);

				newWaypoint_Node_ptr->latitude = v2_or_touchdown_point_lat_lon[0];
				newWaypoint_Node_ptr->longitude = v2_or_touchdown_point_lat_lon[1];

				if (h_landing_taxi_plan.waypoint_node_ptr[index_flight] != NULL) {
					newWaypoint_Node_ptr->prev_node_ptr = h_landing_taxi_plan.waypoint_final_node_ptr[index_flight];
					newWaypoint_Node_ptr->next_node_ptr = h_landing_taxi_plan.waypoint_node_ptr[index_flight];
				}

				h_landing_taxi_plan.waypoint_node_ptr[index_flight] = newWaypoint_Node_ptr;

				h_landing_taxi_plan.waypoint_length[index_flight] = cnt_waypoints + 1;

				h_landing_taxi_plan.waypoint_length[index_flight] = cnt_waypoints + 1;

				array_Airborne_Flight_Plan_Final_Node_ptr[index_flight]->latitude = v2_or_touchdown_point_lat_lon[0];
				array_Airborne_Flight_Plan_Final_Node_ptr[index_flight]->longitude = v2_or_touchdown_point_lat_lon[1];

				char* runway_name = h_aircraft_soa.runway_name_landing[index_flight];
				if (runway_name != NULL) {
					h_landing_taxi_plan.runway_name[index_flight] = (char*)malloc(strlen(runway_name) * sizeof(char));
					strcpy(h_landing_taxi_plan.runway_name[index_flight], runway_name);
				}

				retValue = 0;  // Success
			}
		}
	}

	return retValue;
}

/**
 * Calculate latitude and longitude degree value and set it to departing/landing taxi plan.
 */
void calculate_surface_taxi_plan_waypoint_lat_lon(const int index_flight, const string& airport_code, const int cnt_waypoint, const bool flagDeparting) {
	int fp_index;

	AirportNodeLink cur_airport_node_link = map_ground_waypoint_connectivity.at(airport_code).airport_node_link;

	waypoint_node_t* tmpWaypoint_node_ptr;

	if (flagDeparting) { // Departing
		h_departing_taxi_plan.airport_code[index_flight] = (char*)calloc((strlen(airport_code.c_str())+1), sizeof(char));
		strcpy(h_departing_taxi_plan.airport_code[index_flight], airport_code.c_str());
		h_departing_taxi_plan.airport_code[index_flight][strlen(airport_code.c_str())] = '\0';

		// Set latitude/longitude of the waypoint node
		tmpWaypoint_node_ptr = h_departing_taxi_plan.waypoint_node_ptr[index_flight];
		while (tmpWaypoint_node_ptr != NULL) {
			tmpWaypoint_node_ptr->latitude = getGroundWaypointLatLon(&cur_airport_node_link, tmpWaypoint_node_ptr->wpname, 0);
			tmpWaypoint_node_ptr->longitude = getGroundWaypointLatLon(&cur_airport_node_link, tmpWaypoint_node_ptr->wpname, 1);

			if (tmpWaypoint_node_ptr->prev_node_ptr != NULL) {
				// Calculate the heading angle from the previous waypoint to the current one.
				tmpWaypoint_node_ptr->prev_node_ptr->course_rad_to_next_node = compute_heading_rad_gc(tmpWaypoint_node_ptr->prev_node_ptr->latitude,
						tmpWaypoint_node_ptr->prev_node_ptr->longitude,
						tmpWaypoint_node_ptr->latitude,
						tmpWaypoint_node_ptr->longitude);
			}

			if (tmpWaypoint_node_ptr == h_departing_taxi_plan.waypoint_final_node_ptr[index_flight]) {
				break;
			}

			tmpWaypoint_node_ptr = tmpWaypoint_node_ptr->next_node_ptr; // Update waypoint node pointer to the next node
		}
	} else { // Landing
		h_landing_taxi_plan.airport_code[index_flight] = (char*)calloc((strlen(airport_code.c_str())+1), sizeof(char));
		strcpy(h_landing_taxi_plan.airport_code[index_flight], airport_code.c_str());
		h_landing_taxi_plan.airport_code[index_flight][strlen(airport_code.c_str())] = '\0';

		// Set latitude/longitude of the waypoint node
		tmpWaypoint_node_ptr = h_landing_taxi_plan.waypoint_node_ptr[index_flight];
		while (tmpWaypoint_node_ptr != NULL) {
			tmpWaypoint_node_ptr->latitude = getGroundWaypointLatLon(&cur_airport_node_link, tmpWaypoint_node_ptr->wpname, 0);
			tmpWaypoint_node_ptr->longitude = getGroundWaypointLatLon(&cur_airport_node_link, tmpWaypoint_node_ptr->wpname, 1);

			if (tmpWaypoint_node_ptr->prev_node_ptr != NULL) {
				// Calculate the heading angle from the previous waypoint to the current one.
				tmpWaypoint_node_ptr->prev_node_ptr->course_rad_to_next_node = compute_heading_rad_gc(tmpWaypoint_node_ptr->prev_node_ptr->latitude,
						tmpWaypoint_node_ptr->prev_node_ptr->longitude,
						tmpWaypoint_node_ptr->latitude,
						tmpWaypoint_node_ptr->longitude);
			}

			if (tmpWaypoint_node_ptr == h_landing_taxi_plan.waypoint_final_node_ptr[index_flight]) {
				break;
			}

			tmpWaypoint_node_ptr = tmpWaypoint_node_ptr->next_node_ptr; // Update waypoint node pointer to the next node
		}
	}
}

int generate_and_load_surface_taxi_plan(const int index_flight, const string airport_code, const string startNode_waypoint_id, const string endNode_waypoint_id, const string runway_name) {
	int retValue = 1; // Initial value.  1 means error

	bool flagDeparting = true; // Flag indicating it is a departing taxi plan or not.

	FlightPlan tmpFlightPlan = g_flightplans.at(index_flight);

	TrxRecord tmpTrxRecord = g_trx_records[index_flight];

	string airportOrigin = tmpFlightPlan.origin;
	string airportDestination = tmpFlightPlan.destination;

	if (airportOrigin.length() == 4) {
		airportOrigin = airportOrigin.substr(1, 3);
	}
	if (airportDestination.length() == 4) {
		airportDestination = airportDestination.substr(1, 3);
	}

	if ((airport_code.compare(1, 3, airportOrigin) == 0) || (airport_code.compare(1, 3, airportDestination) == 0)) {
		if (airport_code.compare(1, 3, airportOrigin) == 0) {
			flagDeparting = true;
		} else {
			flagDeparting = false;
		}

		waypoint_node_t tmpNode;
		waypoint_node_t* tmpWaypoint_Node_ptr = NULL;
		waypoint_node_t* tmpWaypoint_Final_Node_ptr = NULL;

		reset_surface_taxi_plan(index_flight, flagDeparting, tmpTrxRecord);

		int num_waypoints = map_ground_waypoint_connectivity.at(airport_code).map_waypoint_node.size();

		vector<string> resultVector_AllRunwayWaypoints = getVector_AllRunwayWaypoints(airport_code, runway_name);

		string farest_waypoint_id;

		int cnt_waypoint = 0;

		int v2_index_departing_taxi_plan;
		int touchdown_index_landing_taxi_plan;

		tmpWaypoint_Node_ptr = NULL; // Reset
		tmpWaypoint_Final_Node_ptr = NULL; // Reset

		vector<string> resultVector = get_taxi_route_from_A_To_B(airport_code, startNode_waypoint_id, endNode_waypoint_id);
		if (resultVector.size() > 0) {
			for (int i = 0; i < resultVector.size(); i++) {
				string tmpWaypointid = resultVector.at(i);

				waypoint_node_t* newWaypoint_Node_ptr = (waypoint_node_t*)calloc(1, sizeof(waypoint_node_t));
				newWaypoint_Node_ptr->prev_node_ptr = NULL; // Reset
				newWaypoint_Node_ptr->next_node_ptr = NULL; // Reset

				newWaypoint_Node_ptr->wpname = (char*)calloc(tmpWaypointid.length(), sizeof(char));
				strcpy(newWaypoint_Node_ptr->wpname, tmpWaypointid.c_str());

				if (tmpWaypoint_Final_Node_ptr != NULL) {
					newWaypoint_Node_ptr->prev_node_ptr = tmpWaypoint_Final_Node_ptr;
					tmpWaypoint_Final_Node_ptr->next_node_ptr = newWaypoint_Node_ptr;
				}

				tmpWaypoint_Final_Node_ptr = newWaypoint_Node_ptr;

				if (tmpWaypoint_Node_ptr == NULL) {
					tmpWaypoint_Node_ptr = newWaypoint_Node_ptr;
				}

				cnt_waypoint++;
			}

			if (flagDeparting) { // Departing
				h_departing_taxi_plan.waypoint_node_ptr[index_flight] = tmpWaypoint_Node_ptr;

				h_departing_taxi_plan.waypoint_final_node_ptr[index_flight] = tmpWaypoint_Final_Node_ptr;

				h_departing_taxi_plan.waypoint_length[index_flight] = cnt_waypoint;
			} else { // Landing
				h_landing_taxi_plan.waypoint_node_ptr[index_flight] = tmpWaypoint_Node_ptr;

				h_landing_taxi_plan.waypoint_final_node_ptr[index_flight] = tmpWaypoint_Final_Node_ptr;

				h_landing_taxi_plan.waypoint_length[index_flight] = cnt_waypoint;
			}
		}

		// Calculate latitude/longitude of waypoints
		calculate_surface_taxi_plan_waypoint_lat_lon(index_flight, airport_code, cnt_waypoint, flagDeparting);

		// Update first waypoint(for transitioning from departing) or last waypoint(for transitioning to landing) latitude and longitude in d_aircraft_soa data structure
		if (flagDeparting) {
			array_Airborne_Flight_Plan_ptr[index_flight]->latitude = h_departing_taxi_plan.waypoint_final_node_ptr[index_flight]->latitude;
			array_Airborne_Flight_Plan_ptr[index_flight]->longitude = h_departing_taxi_plan.waypoint_final_node_ptr[index_flight]->longitude;

			h_departing_taxi_plan.runway_name[index_flight] = (char*)malloc(runway_name.size() * sizeof(char));
			strcpy(h_departing_taxi_plan.runway_name[index_flight], runway_name.c_str());

			retValue = 0;  // Success
		} else {
			array_Airborne_Flight_Plan_Final_Node_ptr[index_flight]->latitude = h_landing_taxi_plan.waypoint_node_ptr[index_flight]->latitude;
			array_Airborne_Flight_Plan_Final_Node_ptr[index_flight]->longitude = h_landing_taxi_plan.waypoint_node_ptr[index_flight]->longitude;

			h_landing_taxi_plan.runway_name[index_flight] = (char*)malloc(runway_name.size() * sizeof(char));
			strcpy(h_landing_taxi_plan.runway_name[index_flight], runway_name.c_str());

			retValue = 0;  // Success
		}
	}

	return retValue;
}

int generate_and_load_surface_taxi_plan(const int index_flight, const string airport_code, const string startNode_waypoint_id, const string endNode_waypoint_id, const string v2Node_waypoint_id, const string touchdownNode_waypoint_id) {
	int retValue = 1; // Initial value.  1 means error

	if (((v2Node_waypoint_id.size() == 0) && (touchdownNode_waypoint_id.size() == 0))
		||
		((v2Node_waypoint_id.size() != 0) && (touchdownNode_waypoint_id.size() != 0))
		)
		return retValue;

	bool flagDeparting = true; // Flag indicating it is a departing taxi plan or not.

	FlightPlan tmpFlightPlan = g_flightplans.at(index_flight);

	TrxRecord tmpTrxRecord = g_trx_records[index_flight];

	string airportOrigin = tmpFlightPlan.origin;
	string airportDestination = tmpFlightPlan.destination;

	if (airportOrigin.length() == 4) {
		airportOrigin = airportOrigin.substr(1, 3);
	}
	if (airportDestination.length() == 4) {
		airportDestination = airportDestination.substr(1, 3);
	}

	if ((airport_code.compare(1, 3, airportOrigin) == 0) || (airport_code.compare(1, 3, airportDestination) == 0)) {
		if (airport_code.compare(1, 3, airportOrigin) == 0) {
			flagDeparting = true;
		} else {
			flagDeparting = false;
		}

		waypoint_node_t tmpNode;
		waypoint_node_t* tmpWaypoint_Node_ptr = NULL;
		waypoint_node_t* tmpWaypoint_Final_Node_ptr = NULL;

		reset_surface_taxi_plan(index_flight, flagDeparting, tmpTrxRecord);

		int num_waypoints = map_ground_waypoint_connectivity.at(airport_code).map_waypoint_node.size();

		int cnt_waypoint = 0;

		int v2_index_departing_taxi_plan;
		int touchdown_index_landing_taxi_plan;

		tmpWaypoint_Node_ptr = NULL; // Reset
		tmpWaypoint_Final_Node_ptr = NULL; // Reset

		waypoint_node_t* newWaypoint_Node_ptr;

		vector<string> resultVector = get_taxi_route_from_A_To_B(airport_code, startNode_waypoint_id, endNode_waypoint_id);
		if (resultVector.size() > 0) {
			for (int i = 0; i < resultVector.size(); i++) {
				string tmpWaypointid = resultVector.at(i);

				newWaypoint_Node_ptr = (waypoint_node_t*)calloc(1, sizeof(waypoint_node_t));
				newWaypoint_Node_ptr->prev_node_ptr = NULL; // Reset
				newWaypoint_Node_ptr->next_node_ptr = NULL; // Reset

				newWaypoint_Node_ptr->wpname = (char*)calloc(tmpWaypointid.length(), sizeof(char));
				strcpy(newWaypoint_Node_ptr->wpname, tmpWaypointid.c_str());

				if (tmpWaypoint_Final_Node_ptr != NULL) {
					newWaypoint_Node_ptr->prev_node_ptr = tmpWaypoint_Final_Node_ptr;
					tmpWaypoint_Final_Node_ptr->next_node_ptr = newWaypoint_Node_ptr;
				}

				tmpWaypoint_Final_Node_ptr = newWaypoint_Node_ptr;

				if (tmpWaypoint_Node_ptr == NULL) {
					tmpWaypoint_Node_ptr = newWaypoint_Node_ptr;
				}

				cnt_waypoint++;
			}

			if (flagDeparting) { // Departing
				h_departing_taxi_plan.waypoint_node_ptr[index_flight] = tmpWaypoint_Node_ptr;

				h_departing_taxi_plan.waypoint_final_node_ptr[index_flight] = tmpWaypoint_Final_Node_ptr;

				h_departing_taxi_plan.waypoint_length[index_flight] = cnt_waypoint;
			} else { // Landing
				h_landing_taxi_plan.waypoint_node_ptr[index_flight] = tmpWaypoint_Node_ptr;

				h_landing_taxi_plan.waypoint_final_node_ptr[index_flight] = tmpWaypoint_Final_Node_ptr;

				h_landing_taxi_plan.waypoint_length[index_flight] = cnt_waypoint;
			}
		}

		// Insert V2 point or touchdown point latitude and longitude to the waypoint list
		if (flagDeparting) {
			newWaypoint_Node_ptr = (waypoint_node_t*)calloc(1, sizeof(waypoint_node_t));
			newWaypoint_Node_ptr->prev_node_ptr = NULL; // Reset
			newWaypoint_Node_ptr->next_node_ptr = NULL; // Reset

			newWaypoint_Node_ptr->wpname = (char*)calloc(v2Node_waypoint_id.length(), sizeof(char));
			strcpy(newWaypoint_Node_ptr->wpname, v2Node_waypoint_id.c_str());

			if (h_departing_taxi_plan.waypoint_final_node_ptr[index_flight] != NULL) {
				newWaypoint_Node_ptr->prev_node_ptr = h_departing_taxi_plan.waypoint_final_node_ptr[index_flight];
				h_departing_taxi_plan.waypoint_final_node_ptr[index_flight]->next_node_ptr = newWaypoint_Node_ptr;
			}

			h_departing_taxi_plan.waypoint_final_node_ptr[index_flight] = newWaypoint_Node_ptr;

			h_departing_taxi_plan.waypoint_length[index_flight] = cnt_waypoint + 1;
		} else {
			newWaypoint_Node_ptr = (waypoint_node_t*)calloc(1, sizeof(waypoint_node_t));
			newWaypoint_Node_ptr->prev_node_ptr = NULL; // Reset
			newWaypoint_Node_ptr->next_node_ptr = NULL; // Reset

			newWaypoint_Node_ptr->wpname = (char*)calloc(touchdownNode_waypoint_id.length(), sizeof(char));
			strcpy(newWaypoint_Node_ptr->wpname, touchdownNode_waypoint_id.c_str());

			if (h_landing_taxi_plan.waypoint_node_ptr[index_flight] != NULL) {
				newWaypoint_Node_ptr->prev_node_ptr = h_landing_taxi_plan.waypoint_final_node_ptr[index_flight];
				newWaypoint_Node_ptr->next_node_ptr = h_landing_taxi_plan.waypoint_node_ptr[index_flight];
			}

			h_landing_taxi_plan.waypoint_node_ptr[index_flight] = newWaypoint_Node_ptr;

			h_landing_taxi_plan.waypoint_length[index_flight] = cnt_waypoint + 1;
		}

		// Calculate latitude/longitude of waypoints
		calculate_surface_taxi_plan_waypoint_lat_lon(index_flight, airport_code, cnt_waypoint+1, flagDeparting);

		// Update V2 point or touchdown point latitude and longitude in d_aircraft_soa data structure
		if (flagDeparting) {
			array_Airborne_Flight_Plan_ptr[index_flight]->latitude = h_departing_taxi_plan.waypoint_final_node_ptr[index_flight]->latitude;
			array_Airborne_Flight_Plan_ptr[index_flight]->longitude = h_departing_taxi_plan.waypoint_final_node_ptr[index_flight]->longitude;

			string runway_name = get_runwayName_from_taxi_plan(index_flight, flagDeparting);
			if (runway_name.size() > 0) {
				h_departing_taxi_plan.runway_name[index_flight] = (char*)malloc(runway_name.size() * sizeof(char));
				strcpy(h_departing_taxi_plan.runway_name[index_flight], runway_name.c_str());
			}

			retValue = 0;  // Success
		} else {
			array_Airborne_Flight_Plan_Final_Node_ptr[index_flight]->latitude = h_landing_taxi_plan.waypoint_node_ptr[index_flight]->latitude;
			array_Airborne_Flight_Plan_Final_Node_ptr[index_flight]->longitude = h_landing_taxi_plan.waypoint_node_ptr[index_flight]->longitude;

			string runway_name = get_runwayName_from_taxi_plan(index_flight, flagDeparting);
			if (runway_name.size() > 0) {
				h_landing_taxi_plan.runway_name[index_flight] = (char*)malloc(runway_name.size() * sizeof(char));
				strcpy(h_landing_taxi_plan.runway_name[index_flight], runway_name.c_str());
			}

			retValue = 0;  // Success
		}
	}

	return retValue;
}

int generate_and_load_surface_taxi_plan(const int index_flight, const string airport_code, const string startNode_waypoint_id, const string endNode_waypoint_id, double v2Point_lat_lon[2], double touchdownPoint_lat_lon[2]) {
	int retValue = 1; // Initial value.  1 means error

	if (((v2Point_lat_lon == NULL) && (touchdownPoint_lat_lon == NULL))
		||
		((v2Point_lat_lon != NULL) && (touchdownPoint_lat_lon != NULL))
		)
		return retValue;

	bool flagDeparting = true; // Flag indicating it is a departing taxi plan or not.

	FlightPlan tmpFlightPlan = g_flightplans.at(index_flight);

	TrxRecord tmpTrxRecord = g_trx_records[index_flight];

	string airportOrigin = tmpFlightPlan.origin;
	string airportDestination = tmpFlightPlan.destination;

	if (airportOrigin.length() == 4) {
		airportOrigin = airportOrigin.substr(1, 3);
	}
	if (airportDestination.length() == 4) {
		airportDestination = airportDestination.substr(1, 3);
	}

	if ((airport_code.compare(1, 3, airportOrigin) == 0) || (airport_code.compare(1, 3, airportDestination) == 0)) {
		if (airport_code.compare(1, 3, airportOrigin) == 0) {
			flagDeparting = true;
		} else {
			flagDeparting = false;
		}

		waypoint_node_t tmpNode;
		waypoint_node_t* tmpWaypoint_Node_ptr = NULL;
		waypoint_node_t* tmpWaypoint_Final_Node_ptr = NULL;

		reset_surface_taxi_plan(index_flight, flagDeparting, tmpTrxRecord);

		// Reset data
		if (flagDeparting) { // Departing
			if (h_departing_taxi_plan.runway_name[index_flight] != NULL) {
				free(h_departing_taxi_plan.runway_name[index_flight]);
				h_departing_taxi_plan.runway_name[index_flight] = NULL;
			}

			h_departing_taxi_plan.waypoint_length[index_flight] = 0;
		} else { // Landing
			if (h_landing_taxi_plan.runway_name[index_flight] != NULL) {
				free(h_landing_taxi_plan.runway_name[index_flight]);
				h_landing_taxi_plan.runway_name[index_flight] = NULL;
			}

			h_landing_taxi_plan.waypoint_length[index_flight] = 0;
		}

		int cnt_waypoint = 0;

		int num_waypoints = map_ground_waypoint_connectivity.at(airport_code).map_waypoint_node.size();

		tmpWaypoint_Node_ptr = NULL; // Reset
		tmpWaypoint_Final_Node_ptr = NULL; // Reset

		waypoint_node_t* newWaypoint_Node_ptr;

		vector<string> resultVector = get_taxi_route_from_A_To_B(airport_code, startNode_waypoint_id, endNode_waypoint_id);
		if (resultVector.size() > 0) {
			for (int i = 0; i < resultVector.size(); i++) {
				string tmpWaypointid = resultVector.at(i);

				newWaypoint_Node_ptr = (waypoint_node_t*)calloc(1, sizeof(waypoint_node_t));
				newWaypoint_Node_ptr->prev_node_ptr = NULL; // Reset
				newWaypoint_Node_ptr->next_node_ptr = NULL; // Reset

				newWaypoint_Node_ptr->wpname = (char*)calloc(tmpWaypointid.length(), sizeof(char));
				strcpy(newWaypoint_Node_ptr->wpname, tmpWaypointid.c_str());

				if (tmpWaypoint_Final_Node_ptr != NULL) {
					newWaypoint_Node_ptr->prev_node_ptr = tmpWaypoint_Final_Node_ptr;
					tmpWaypoint_Final_Node_ptr->next_node_ptr = newWaypoint_Node_ptr;
				}

				tmpWaypoint_Final_Node_ptr = newWaypoint_Node_ptr;

				if (tmpWaypoint_Node_ptr == NULL) {
					tmpWaypoint_Node_ptr = newWaypoint_Node_ptr;
				}

				cnt_waypoint++;
			}

			if (flagDeparting) {
				h_departing_taxi_plan.waypoint_node_ptr[index_flight] = tmpWaypoint_Node_ptr;
				h_departing_taxi_plan.waypoint_final_node_ptr[index_flight] = tmpWaypoint_Final_Node_ptr;

				h_departing_taxi_plan.waypoint_length[index_flight] = cnt_waypoint;
			} else {
				h_landing_taxi_plan.waypoint_node_ptr[index_flight] = tmpWaypoint_Node_ptr;
				h_landing_taxi_plan.waypoint_final_node_ptr[index_flight] = tmpWaypoint_Final_Node_ptr;

				h_landing_taxi_plan.waypoint_length[index_flight] = cnt_waypoint;
			}
		}

		// Calculate latitude/longitude of waypoints
		calculate_surface_taxi_plan_waypoint_lat_lon(index_flight, airport_code, cnt_waypoint, flagDeparting);

		// Insert V2 point or touchdown point latitude and longitude to the waypoint list
		if (flagDeparting) {
			newWaypoint_Node_ptr = (waypoint_node_t*)calloc(1, sizeof(waypoint_node_t));
			newWaypoint_Node_ptr->prev_node_ptr = NULL; // Reset
			newWaypoint_Node_ptr->next_node_ptr = NULL; // Reset

			newWaypoint_Node_ptr->wpname = (char*)calloc(strlen(WAYPOINT_ID_V2POINT), sizeof(char));
			strcpy(newWaypoint_Node_ptr->wpname, WAYPOINT_ID_V2POINT);

			newWaypoint_Node_ptr->latitude = v2Point_lat_lon[0];
			newWaypoint_Node_ptr->longitude = v2Point_lat_lon[1];

			if (h_departing_taxi_plan.waypoint_final_node_ptr[index_flight] != NULL) {
				newWaypoint_Node_ptr->prev_node_ptr = h_departing_taxi_plan.waypoint_final_node_ptr[index_flight];
				h_departing_taxi_plan.waypoint_final_node_ptr[index_flight]->next_node_ptr = newWaypoint_Node_ptr;
			}

			h_departing_taxi_plan.waypoint_final_node_ptr[index_flight] = newWaypoint_Node_ptr;

			h_departing_taxi_plan.waypoint_length[index_flight] = cnt_waypoint + 1;
		} else {
			newWaypoint_Node_ptr = (waypoint_node_t*)calloc(1, sizeof(waypoint_node_t));
			newWaypoint_Node_ptr->prev_node_ptr = NULL; // Reset
			newWaypoint_Node_ptr->next_node_ptr = NULL; // Reset

			newWaypoint_Node_ptr->wpname = (char*)calloc(strlen(WAYPOINT_ID_TOUCHDOWNPOINT), sizeof(char));
			strcpy(newWaypoint_Node_ptr->wpname, WAYPOINT_ID_TOUCHDOWNPOINT);

			newWaypoint_Node_ptr->latitude = touchdownPoint_lat_lon[0];
			newWaypoint_Node_ptr->longitude = touchdownPoint_lat_lon[1];

			if (h_landing_taxi_plan.waypoint_node_ptr[index_flight] != NULL) {
				newWaypoint_Node_ptr->prev_node_ptr = h_landing_taxi_plan.waypoint_final_node_ptr[index_flight];
				newWaypoint_Node_ptr->next_node_ptr = h_landing_taxi_plan.waypoint_node_ptr[index_flight];
			}

			h_landing_taxi_plan.waypoint_node_ptr[index_flight] = newWaypoint_Node_ptr;

			h_landing_taxi_plan.waypoint_length[index_flight] = cnt_waypoint + 1;
		}

		// For latitude/longitude of v2Point and touchdownPoint, we need to handle them separately
		if (flagDeparting) {
			array_Airborne_Flight_Plan_ptr[index_flight]->latitude = h_departing_taxi_plan.waypoint_final_node_ptr[index_flight]->latitude;
			array_Airborne_Flight_Plan_ptr[index_flight]->longitude = h_departing_taxi_plan.waypoint_final_node_ptr[index_flight]->longitude;

			string runway_name = get_runwayName_from_taxi_plan(index_flight, flagDeparting);
			if (runway_name.size() > 0) {
				h_departing_taxi_plan.runway_name[index_flight] = (char*)malloc(runway_name.size() * sizeof(char));
				strcpy(h_departing_taxi_plan.runway_name[index_flight], runway_name.c_str());
			}

			retValue = 0;  // Success
		} else {
			array_Airborne_Flight_Plan_Final_Node_ptr[index_flight]->latitude = h_landing_taxi_plan.waypoint_node_ptr[index_flight]->latitude;
			array_Airborne_Flight_Plan_Final_Node_ptr[index_flight]->longitude = h_landing_taxi_plan.waypoint_node_ptr[index_flight]->longitude;

			string runway_name = get_runwayName_from_taxi_plan(index_flight, flagDeparting);
			if (runway_name.size() > 0) {
				h_landing_taxi_plan.runway_name[index_flight] = (char*)malloc(runway_name.size() * sizeof(char));
				strcpy(h_landing_taxi_plan.runway_name[index_flight], runway_name.c_str());
			}

			retValue = 0;  // Success
		}
	}

	return retValue;
}

waypoint_node_t* produce_surface_waypoint_node_linkedList(const string id, const string airport_code, string str_surface_plan) {
	waypoint_node_t* ret_ptr = NULL;

	string tmpValue_Id;
	string tmpValue_Latitude;
	string tmpValue_Longitude;
	vector<string> tmpVector_taxi_plan;

	json jsonObj;

	str_surface_plan = trim(str_surface_plan);
	if (str_surface_plan.length() < 0) {
		printf("%s: Surface plan is not valid.\n", id.c_str());

		return NULL;
	} else {
		if (str_surface_plan.find_first_of("{") != 0) {
			printf("%s: Surface plan is not valid.\n", id.c_str());

			return NULL;
		}

		while (str_surface_plan.find_first_of("{") == 0) {
			tmpValue_Id.clear(); // Reset
			tmpValue_Latitude.clear(); // Reset
			tmpValue_Longitude.clear(); // Reset

			int tmpPos_parenthesis_L = str_surface_plan.find_first_of("{");
			int tmpPos_parenthesis_R = str_surface_plan.find_first_of("}");
			if ((tmpPos_parenthesis_L < 0) || (tmpPos_parenthesis_R < 0)) {
				printf("%s: Surface plan is not valid.\n", id.c_str());
				tmpVector_taxi_plan.clear();

				return NULL;
			}

			string tmpJsonStr = str_surface_plan.substr(tmpPos_parenthesis_L, (tmpPos_parenthesis_R - tmpPos_parenthesis_L + 1));

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
				tmpVector_taxi_plan.push_back(tmpValue_Id);
			}

			if (str_surface_plan.length() > (tmpPos_parenthesis_R+1)) {
				str_surface_plan.erase(0, tmpPos_parenthesis_R+1);
			} else {
				break;
			}

			if (str_surface_plan.find_first_of(",")+1 > 0) {
				str_surface_plan.erase(0, str_surface_plan.find_first_of(",")+1);
			}

			trim(str_surface_plan);
			if (str_surface_plan.find_first_of("{") != 0) {
				printf("%s: Surface plan is not valid.\n", id.c_str());
				tmpVector_taxi_plan.clear();

				return NULL;
			}
		} // end - while loop

		if (tmpVector_taxi_plan.size() == 0) {
			return NULL;
		} else {
			if ((tmpValue_Latitude.length() > 0) && (tmpValue_Longitude.length() > 0)) {
				if (tmpVector_taxi_plan.size() <= 1) {
					printf("%s: Surface plan is not valid.\n", id.c_str());
					tmpVector_taxi_plan.clear();

					return NULL;
				} else {
					double tmpLatLon[2];
					tmpLatLon[0] = strtod(tmpValue_Latitude.c_str(), NULL);
					tmpLatLon[1] = strtod(tmpValue_Longitude.c_str(), NULL);

					double (*tmpLatLon_ptr)[2];
					tmpLatLon_ptr = &tmpLatLon;

					ret_ptr = getSurface_waypoint_node_linkedList(airport_code, tmpVector_taxi_plan, tmpLatLon_ptr);
				}
			} else {
				if (tmpVector_taxi_plan.size() <= 2) {
					printf("%s: Surface plan is not valid.\n", id.c_str());
					tmpVector_taxi_plan.clear();

					return NULL;
				} else {
					ret_ptr = getSurface_waypoint_node_linkedList(airport_code, tmpVector_taxi_plan, NULL);
				}
			}
		}
	}

	return ret_ptr;
}
