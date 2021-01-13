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

/**
 * Loader of airport layout files
 * 
 * Update: 09/10/2018
 * 
 * File location: share/libairport_layout/Airport_Rwy
 * 
 * For every airport, airport layout files consist of two files:
 * # Nodes_Def.csv
 * # Nodes_Links.csv
 */

#include "AirportLayoutDataLoader.h"
#include "AirportNodeLink.h"

#include "geometry_utils.h"

#include "AstarSearch.h"
#include "SearchGraph.h"
#include "SearchPath.h"
#include "util_string.h"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>

using namespace std;
using namespace osi;

#define LINE_LEN 500

map<string, GroundWaypointConnectivity> map_ground_waypoint_connectivity;

AirportLayoutDataLoader::AirportLayoutDataLoader() {}

AirportLayoutDataLoader::~AirportLayoutDataLoader() {}

void release_connectivity(bool** connectivity_ptr, const int num_waypoints) {
	if (num_waypoints > 0) {
		for (int i = 0; i < num_waypoints; i++) {
			if (connectivity_ptr[i] != NULL) {
				free(connectivity_ptr[i]);
				connectivity_ptr[i] = NULL;
			}
		}

		if (connectivity_ptr != NULL) {
			free(connectivity_ptr);
			connectivity_ptr = NULL;
		}
	}
}

/**
 * Get all runways of the queried airport
 */
vector<AirportNode> getVector_AllRunways(const string airport_code) {
	vector<AirportNode> retVector;

	GroundWaypointConnectivity cur_GroundWaypointConnectivity;
	map<string, AirportNode>* cur_map_waypoint_node_ptr;

	if ((!airport_code.empty()) && (map_ground_waypoint_connectivity.find(airport_code) != map_ground_waypoint_connectivity.end())) {
		cur_GroundWaypointConnectivity = map_ground_waypoint_connectivity.at(airport_code);
		cur_map_waypoint_node_ptr = &(cur_GroundWaypointConnectivity.map_waypoint_node);

		if (cur_map_waypoint_node_ptr->size() > 0) {
			set<string> setAllRunways = getAllRunways(airport_code);
			if (setAllRunways.size() > 0) {
				string tmpRunwayName;

				set<string>::iterator ite;
				for (ite = setAllRunways.begin(); ite != setAllRunways.end(); ite++) {
					tmpRunwayName = *ite;

					AirportNode tmpMatchedAirportNode;

					map<string, AirportNode>::iterator ite_waypointNode;
					for (ite_waypointNode = cur_map_waypoint_node_ptr->begin(); ite_waypointNode != cur_map_waypoint_node_ptr->end(); ite_waypointNode++) {
						if ((tmpRunwayName.length() > 0) && (tmpRunwayName.compare(ite_waypointNode->second.refName1) == 0)) {
							tmpMatchedAirportNode = ite_waypointNode->second;

							break;
						}
					}

					if (0 < tmpMatchedAirportNode.id.length()) {
						retVector.push_back(tmpMatchedAirportNode);
					}
				}
			}
		}
	}

	return retVector;
}

/**
 * Get all waypoints on the queried airport runway
 */
vector<string> getVector_AllRunwayWaypoints(const string airport_code, const string runway_name) {
	vector<string> retVector; // Vector of waypoint Ids

	string trimmed_runway_name(runway_name);
	trimmed_runway_name = trim(trimmed_runway_name);

	GroundWaypointConnectivity cur_GroundWaypointConnectivity;
	map<string, AirportNode> cur_map_waypoint_node;

	if (map_ground_waypoint_connectivity.find(airport_code) != map_ground_waypoint_connectivity.end()) {
		cur_GroundWaypointConnectivity = map_ground_waypoint_connectivity.at(airport_code);
		cur_map_waypoint_node = cur_GroundWaypointConnectivity.map_waypoint_node;

		string tmpRunwayWaypointId;

		if (cur_map_waypoint_node.size() > 0) {
			vector<AirportNode> resultVector_allRunways = getVector_AllRunways(airport_code);
			if (resultVector_allRunways.size() > 0) {
				vector<AirportNode>::iterator ite;
				for (ite = resultVector_allRunways.begin(); ite != resultVector_allRunways.end(); ite++) {
					AirportNode tmpAirportNode = *ite;
					if (tmpAirportNode.refName1.find(trimmed_runway_name) != string::npos) {
						tmpRunwayWaypointId = tmpAirportNode.id;

						break;
					}
				}

				if (tmpRunwayWaypointId.length() > 0) {
					int tmpIndex = tmpRunwayWaypointId.find_last_of("_");

					string subString_waypoint_id = tmpRunwayWaypointId.substr(0, tmpIndex);

					map<string, AirportNode>::iterator ite_map_waypoint_node;
					for (ite_map_waypoint_node = cur_map_waypoint_node.begin(); ite_map_waypoint_node != cur_map_waypoint_node.end(); ite_map_waypoint_node++) {
						AirportNode tmpAirportNode = ite_map_waypoint_node->second;
						if (tmpAirportNode.id.find(subString_waypoint_id) != string::npos) {
							retVector.push_back(tmpAirportNode.id);
						}
					}

					// Sorting
					std::sort(retVector.begin(), retVector.end());
				}
			}
		}
	}

	return retVector;
}

/**
 * Get end points of the queried airport runway
 */
pair<string, string> getRunwayEnds(const string airport_code, const string runway_name) {
	pair<string, string> retPair; // Pair of waypoint IDs

	string trimmed_runway_name(runway_name);
	trimmed_runway_name = trim(trimmed_runway_name);

	GroundWaypointConnectivity cur_GroundWaypointConnectivity;
	map<string, AirportNode> cur_map_waypoint_node;

	if (map_ground_waypoint_connectivity.find(airport_code) != map_ground_waypoint_connectivity.end()) {
		cur_GroundWaypointConnectivity = map_ground_waypoint_connectivity.at(airport_code);
		cur_map_waypoint_node = cur_GroundWaypointConnectivity.map_waypoint_node;

		string tmpRunwayWaypointId;

		if (cur_map_waypoint_node.size() > 0) {
			vector<AirportNode> resultVector_allRunways = getVector_AllRunways(airport_code);
			if (resultVector_allRunways.size() > 0) {
				vector<AirportNode>::iterator ite;
				for (ite = resultVector_allRunways.begin(); ite != resultVector_allRunways.end(); ite++) {
					AirportNode tmpAirportNode = *ite;
					if ((tmpAirportNode.refName1.find(trimmed_runway_name) != string::npos) && (tmpAirportNode.type1.find("Entry") != string::npos)){
						retPair.first = tmpAirportNode.id;
					}
					if ((tmpAirportNode.refName2.find(trimmed_runway_name) != string::npos) && (tmpAirportNode.type2.find("End") != string::npos)){
						retPair.second = tmpAirportNode.id;
					}
				}
			}
		}
	}

	return retPair;
}

/**
 * Get taxi route from waypoint A to B based on the input connectivity.
 */
vector<string> get_taxi_route_from_A_To_B(const string airport_code, bool** connectivity, const string startNode_waypoint_id, const string endNode_waypoint_id) {
	vector<string> retVector;

	int num_nodes = 0;

	GroundWaypointConnectivity tmpGroundWaypointConnectivity = map_ground_waypoint_connectivity.at(airport_code);

	num_nodes = tmpGroundWaypointConnectivity.map_waypoint_node.size();

	int startNode_seq = tmpGroundWaypointConnectivity.map_waypoint_node.at(startNode_waypoint_id).index;
	int endNode_seq = tmpGroundWaypointConnectivity.map_waypoint_node.at(endNode_waypoint_id).index;

	double endNode_latitude = tmpGroundWaypointConnectivity.map_waypoint_node.at(endNode_waypoint_id).latitude;
	double endNode_longitude = tmpGroundWaypointConnectivity.map_waypoint_node.at(endNode_waypoint_id).longitude;

	// Allocate space
	double* heuristic_costs = (double*)calloc(num_nodes+1, sizeof(double));
	string node_names[num_nodes+1];

	map<string, AirportNode>::iterator ite;
	for (ite = tmpGroundWaypointConnectivity.map_waypoint_node.begin(); ite != tmpGroundWaypointConnectivity.map_waypoint_node.end(); ite++) {
		string tmpWaypointId = ite->first;
		int tmpSeq = ite->second.index;

		node_names[tmpSeq] = tmpWaypointId; // Set value
		if (tmpWaypointId.compare(endNode_waypoint_id) != 0) {
			double tmpDistance = compute_distance_gc(tmpGroundWaypointConnectivity.map_waypoint_node.at(tmpWaypointId).latitude,
					tmpGroundWaypointConnectivity.map_waypoint_node.at(tmpWaypointId).longitude,
									endNode_latitude,
									endNode_longitude,
									0);

			// Calculate heuristic cost
			heuristic_costs[tmpSeq] = tmpDistance;
		}
	}

	SearchPath result_path;
	SearchGraph graph(num_nodes,
						  connectivity,
						  tmpGroundWaypointConnectivity.path_costs,
						  heuristic_costs,
						  node_names);

	AstarSearch astarSearch;
	astarSearch.findPath(graph, startNode_seq,
					endNode_seq, &result_path);

	deque<int> deque = result_path.getNodes();
	std::deque<int>::iterator iteDeque;
	for (iteDeque = deque.begin(); iteDeque != deque.end(); iteDeque++) {
		int tmpSeq = *iteDeque;

		map<string, AirportNode>::iterator iteMap;
		for (iteMap = tmpGroundWaypointConnectivity.map_waypoint_node.begin(); iteMap != tmpGroundWaypointConnectivity.map_waypoint_node.end(); iteMap++) {
			if (iteMap->second.index == tmpSeq) {
				string tmpWaypointid = iteMap->first;
				retVector.push_back(tmpWaypointid);
			}
		}
	}

	free(heuristic_costs);

	return retVector;
}

/**
 * Get taxi route from waypoint A to B based on normal connectivity.  Normal connectivity allows 2-way traffic on all links.
 */
vector<string> get_taxi_route_from_A_To_B(const string airport_code, const string startNode_waypoint_id, const string endNode_waypoint_id) {
	vector<string> retVector;

	int num_nodes = 0;

	GroundWaypointConnectivity tmpGroundWaypointConnectivity = map_ground_waypoint_connectivity.at(airport_code);
	bool** connectivity = tmpGroundWaypointConnectivity.connectivity;
	num_nodes = tmpGroundWaypointConnectivity.map_waypoint_node.size();

	int startNode_seq = tmpGroundWaypointConnectivity.map_waypoint_node.at(startNode_waypoint_id).index;
	int endNode_seq = tmpGroundWaypointConnectivity.map_waypoint_node.at(endNode_waypoint_id).index;

	double endNode_latitude = tmpGroundWaypointConnectivity.map_waypoint_node.at(endNode_waypoint_id).latitude;
	double endNode_longitude = tmpGroundWaypointConnectivity.map_waypoint_node.at(endNode_waypoint_id).longitude;

	// Allocate space
	double* heuristic_costs = (double*)calloc(num_nodes+1, sizeof(double));
	string node_names[num_nodes+1];

	map<string, AirportNode>::iterator ite;
	for (ite = tmpGroundWaypointConnectivity.map_waypoint_node.begin(); ite != tmpGroundWaypointConnectivity.map_waypoint_node.end(); ite++) {
		string tmpWaypointId = ite->first;
		int tmpSeq = ite->second.index;

		node_names[tmpSeq] = tmpWaypointId; // Set value
		if (tmpWaypointId.compare(endNode_waypoint_id) != 0) {
			double tmpDistance = compute_distance_gc(tmpGroundWaypointConnectivity.map_waypoint_node.at(tmpWaypointId).latitude,
					tmpGroundWaypointConnectivity.map_waypoint_node.at(tmpWaypointId).longitude,
									endNode_latitude,
									endNode_longitude,
									0);

			// Calculate heuristic cost
			heuristic_costs[tmpSeq] = tmpDistance;
		}
	}

	SearchPath result_path;
	SearchGraph graph(num_nodes,
						  connectivity,
						  tmpGroundWaypointConnectivity.path_costs,
						  heuristic_costs,
						  node_names);

	AstarSearch astarSearch;
	astarSearch.findPath(graph, startNode_seq,
					endNode_seq, &result_path);

	deque<int> deque = result_path.getNodes();
	std::deque<int>::iterator iteDeque;
	for (iteDeque = deque.begin(); iteDeque != deque.end(); iteDeque++) {
		int tmpSeq = *iteDeque;

		map<string, AirportNode>::iterator iteMap;
		for (iteMap = tmpGroundWaypointConnectivity.map_waypoint_node.begin(); iteMap != tmpGroundWaypointConnectivity.map_waypoint_node.end(); iteMap++) {
			if (iteMap->second.index == tmpSeq) {
				string tmpWaypointid = iteMap->first;
				retVector.push_back(tmpWaypointid);
			}
		}
	}

	free(heuristic_costs);

	return retVector;
}

/**
 * Get all runways of the queried airport
 */
set<string> getAllRunways(const string& airport_code) {
	set<string> retSet;

	GroundWaypointConnectivity cur_GroundWaypointConnectivity;
	map<string, AirportNode> cur_map_waypoint_node;

	if (map_ground_waypoint_connectivity.find(airport_code) != map_ground_waypoint_connectivity.end()) {
		cur_GroundWaypointConnectivity = map_ground_waypoint_connectivity.at(airport_code);
		cur_map_waypoint_node = cur_GroundWaypointConnectivity.map_waypoint_node;
	} else {
		return retSet;
	}

	map<string, AirportNode>::iterator ite_MapWaypointNode;
	for (ite_MapWaypointNode = cur_map_waypoint_node.begin(); ite_MapWaypointNode != cur_map_waypoint_node.end(); ite_MapWaypointNode++) {
		if ((ite_MapWaypointNode->second.refName1.compare("null") != 0) && (ite_MapWaypointNode->second.refName1.compare("") != 0)) {
			retSet.insert(ite_MapWaypointNode->second.refName1);
		}
	}

	return retSet;
}

/**
 * Generate airport connectivity on all airports
 */
void generate_all_airport_connectivity() {
	if (map_ground_waypoint_connectivity.size() > 0) {
		map<string, GroundWaypointConnectivity>::iterator ite;

		// Iterate every airport.  Generate connectivity data.
		for (ite = map_ground_waypoint_connectivity.begin(); ite != map_ground_waypoint_connectivity.end(); ite++) {
			GroundWaypointConnectivity tmpGroundWaypointConnectivity = ite->second;

			string tmpAirportCode = ite->first;
			int size_map_waypoint_node = tmpGroundWaypointConnectivity.map_waypoint_node.size();
			if (size_map_waypoint_node > 0) {
				// Allocate space for field "connectivity"
				// Field "connectivity" is a 2D bool array
				tmpGroundWaypointConnectivity.connectivity = (bool**)calloc(size_map_waypoint_node, sizeof(bool*));
				for (int i = 0; i < size_map_waypoint_node; i++) {
					// Allocate space for each bool* array element
					tmpGroundWaypointConnectivity.connectivity[i] = (bool*)calloc(size_map_waypoint_node, sizeof(bool));
				}

				// Allocate space for field "path_costs"
				// Field "path_costs" is a 2D double array
				tmpGroundWaypointConnectivity.path_costs = (double**)calloc(size_map_waypoint_node, sizeof(double*));
				for (unsigned int i = 0; i < size_map_waypoint_node; i++) {
					// Allocate space for each double* array element
					tmpGroundWaypointConnectivity.path_costs[i] = (double*)calloc(size_map_waypoint_node, sizeof(double));
				}

				AirportNodeLink tmpAirportNodeLink = tmpGroundWaypointConnectivity.airport_node_link;
				if (tmpAirportNodeLink.n1_id.size() > 0) {
					for (unsigned int i = 0; i < tmpAirportNodeLink.n1_id.size(); i++) {
						int n1_seq = tmpGroundWaypointConnectivity.map_waypoint_node.at(tmpAirportNodeLink.n1_id.at(i)).index;
						int n2_seq = tmpGroundWaypointConnectivity.map_waypoint_node.at(tmpAirportNodeLink.n2_id.at(i)).index;

						tmpGroundWaypointConnectivity.connectivity[n1_seq][n2_seq] = 1;
						tmpGroundWaypointConnectivity.connectivity[n2_seq][n1_seq] = 1;

						// Calculate the distance of the link between n1 and n2
						double tmpDistance = compute_distance_gc(tmpAirportNodeLink.n1_latitude.at(i),
								tmpAirportNodeLink.n1_longitude.at(i),
								tmpAirportNodeLink.n2_latitude.at(i),
								tmpAirportNodeLink.n2_longitude.at(i),
								0);

						tmpGroundWaypointConnectivity.path_costs[n1_seq][n2_seq] = tmpDistance; // Set path_costs value
						tmpGroundWaypointConnectivity.path_costs[n2_seq][n1_seq] = tmpDistance; // Set path_costs value
					}
				}

				// Update map_ground_waypoint_connectivity element
				map_ground_waypoint_connectivity[tmpAirportCode] = tmpGroundWaypointConnectivity;

				int num_waypoints = tmpGroundWaypointConnectivity.map_waypoint_node.size();

				bool** newConnectivity = (bool**)calloc(num_waypoints, sizeof(bool*));
				for (unsigned int i = 0; i < num_waypoints; i++) {
					// Allocate space for each bool* array element
					newConnectivity[i] = (bool*)calloc(num_waypoints, sizeof(bool));
				}

				// Duplicate connectivity data
				for (unsigned int i = 0; i < num_waypoints; i++) {
					for (unsigned int j = 0; j < num_waypoints; j++) {
						newConnectivity[i][j] = tmpGroundWaypointConnectivity.connectivity[i][j];
					}
				}

				// Remove links between two shreshold points on all runways
				// Get all runways of the airport
				set<string> setAllRunways = getAllRunways(tmpAirportCode);
				if (setAllRunways.size() > 0) {
					set<string>::iterator ite_setAllRunways;
					for (ite_setAllRunways = setAllRunways.begin(); ite_setAllRunways != setAllRunways.end(); ite_setAllRunways++) {
						string tmpRunwayName = *ite_setAllRunways;

						string tmpThresholdWaypoint1;
						string tmpThresholdWaypoint2;

						// Get two threshold waypoints on the runway
						map<string, AirportNode>::iterator ite_map_waypoint_node;
						for (ite_map_waypoint_node = tmpGroundWaypointConnectivity.map_waypoint_node.begin(); ite_map_waypoint_node != tmpGroundWaypointConnectivity.map_waypoint_node.end(); ite_map_waypoint_node++) {
							if (tmpRunwayName.compare(ite_map_waypoint_node->second.refName1) == 0) {
								tmpThresholdWaypoint1.assign(ite_map_waypoint_node->second.id);
							}
							if (tmpRunwayName.compare(ite_map_waypoint_node->second.refName2) == 0) {
								tmpThresholdWaypoint2.assign(ite_map_waypoint_node->second.id);
							}

							if ((tmpThresholdWaypoint1.size() > 0) && (tmpThresholdWaypoint2.size() > 0))
								break;
						}

						if ((tmpThresholdWaypoint1.size() > 0) && (tmpThresholdWaypoint2.size() > 0)) {
							// Get shortest route between two shreshold points
							vector<string> tmpVector = get_taxi_route_from_A_To_B(tmpAirportCode, tmpThresholdWaypoint1, tmpThresholdWaypoint2);
							if (tmpVector.size() > 0) {
								int tmpStartNode_seq;
								int tmpEndNode_seq;

								for (unsigned int i = 0; i < tmpVector.size(); i++) {
									if (i > 0) {
										tmpStartNode_seq = tmpGroundWaypointConnectivity.map_waypoint_node.at(tmpVector.at(i-1)).index;
										tmpEndNode_seq = tmpGroundWaypointConnectivity.map_waypoint_node.at(tmpVector.at(i)).index;

										// Remove link
										newConnectivity[tmpStartNode_seq][tmpEndNode_seq] = 0;
										newConnectivity[tmpEndNode_seq][tmpStartNode_seq] = 0;
									}
								}
							}
						}
					}
				}
				// end - Remove links between two shreshold points on all runways

				// Release the current connectivity memory space
				release_connectivity(tmpGroundWaypointConnectivity.connectivity, num_waypoints);

				// Set new connectivity
				tmpGroundWaypointConnectivity.connectivity = newConnectivity;

				// Update map_ground_waypoint_connectivity element
				map_ground_waypoint_connectivity[tmpAirportCode] = tmpGroundWaypointConnectivity;
			}
		}
	}
}

/**
 * Load airport layout data and create map_airport_layout and map_ground_waypoint_connectivity data
 */
int AirportLayoutDataLoader::loadAirportLayout(const string& dirname) {
	struct dirent *dirEnt;
	const char* char_dirname = dirname.c_str();

	const int LINE__LEN = 1000;

	DIR *dir = opendir(dirname.c_str());
	string string_sub_filenamePath;
	ifstream in;

	if (dir == NULL) {
		printf("Could not open directory %s", dirname.c_str());
		return -1;
	}

	// All airport node and link CSV files are resided in the same directory.
	// When we iterate files inside the directory, the iterating order is unknown.
	// To build the data of GroundWaypointConnectivity, we need to process Node_Def data before Node_Links
	// Therefore, we need to iterate directory files twice.

	while ((dirEnt = readdir(dir)) != NULL) {
		struct stat path_stat;
		char* filenamePath = (char*)malloc(LINE_LEN * sizeof(char));
		char* dirEnt_name = dirEnt->d_name;
		string tmpString(dirEnt_name);
		string airport_code;

		if ((strcmp(dirEnt_name, ".") != 0) && (strcmp(dirEnt_name, "..") != 0)) {
			string read_line = "";
			char *entry;
			string tmpWaypointId;

			string_sub_filenamePath.assign(dirname);
			string_sub_filenamePath.append("/");
			string_sub_filenamePath.append(dirEnt_name);

			GroundWaypointConnectivity tmpGroundWaypointConnectivity;

			airport_code = tmpString.substr(0, 4);

			if (endsWith(dirEnt_name, "Nodes_Def.csv")) {
				tmpGroundWaypointConnectivity = GroundWaypointConnectivity();

				in.open(string_sub_filenamePath.c_str());

				while (in.good()) {
					read_line = "";
					getline(in, read_line);
					if (strlen(read_line.c_str()) == 0) continue;

					int tmpIndex;
					double tmpLat;
					double tmpLon;
					string tmpDomain;
					string tmpRefName1, tmpRefName2;
					string tmpType1, tmpType2;

					if (read_line.find("id", 0, 2) == std::string::npos) {
						if (read_line.length() > 0) {
							char* tempCharArray = (char*)calloc((read_line.length()+1), sizeof(char));
							strcpy(tempCharArray, read_line.c_str());

							// Retrieve individual data split by "," sign
							// Get first entry
							entry = strtok(tempCharArray, ",");

							tmpWaypointId.assign(string(entry));

							entry = strtok(NULL, ",");
							tmpIndex = atoi(entry);

							entry = strtok(NULL, ",");
							tmpLat = atof(entry);

							entry = strtok(NULL, ",");
							tmpLon = atof(entry);

							entry = strtok(NULL, ",");

							entry = strtok(NULL, ",");
							tmpDomain.assign(entry);

							entry = strtok(NULL, ",");
							if ((entry != NULL) && (strlen(entry) > 0)) {
								tmpRefName1.assign(entry);
							} else {
								tmpRefName1.assign("");
							}

							entry = strtok(NULL, ",");
							if ((entry != NULL) && (strlen(entry) > 0)) {
								tmpType1.assign(entry);
							} else {
								tmpType1.assign("");
							}

							entry = strtok(NULL, ",");
							if ((entry != NULL) && (strlen(entry) > 0)) {
								tmpRefName2.assign(entry);
							} else {
								tmpRefName2.assign("");
							}

							entry = strtok(NULL, ",");
							if ((entry != NULL) && (strlen(entry) > 0)) {
								tmpType2.assign(entry);
							} else {
								tmpType2.assign("");
							}

							AirportNode tmpAirportNode = AirportNode(tmpWaypointId, tmpIndex, tmpLat, tmpLon, tmpDomain, tmpRefName1, tmpType1, tmpRefName2, tmpType2);

							// If id does not exist in map_waypoint_node
							if (tmpGroundWaypointConnectivity.map_waypoint_node.find(tmpWaypointId) == tmpGroundWaypointConnectivity.map_waypoint_node.end()) {
								// Insert it in map_waypoint_node
								tmpGroundWaypointConnectivity.map_waypoint_node.insert(pair<string, AirportNode>(tmpWaypointId, tmpAirportNode));
							}

							// g++ 9 version change, removed (tempCharArray != '\0')
							if (tempCharArray != NULL) {
								free(tempCharArray);
								tempCharArray = NULL;
							}
						}
					}
				}

				// Add ground waypoint connectivity data
				// At this moment, tmpGroundWaypointConnectivity only contains map_waypoint_node data.  Property "connectivity" is empty.
				map_ground_waypoint_connectivity.insert(pair<string, GroundWaypointConnectivity>(airport_code, tmpGroundWaypointConnectivity));

				in.close();
			}
		}

		free(filenamePath);
		filenamePath = NULL;
	} // end - while loop

	closedir(dir);

	// ==============================================================

	dir = opendir(dirname.c_str());

	while ((dirEnt = readdir(dir)) != NULL) {
		struct stat path_stat;
		char* filenamePath = (char*)malloc(LINE_LEN * sizeof(char));
		char* dirEnt_name = dirEnt->d_name;
		string tmpString(dirEnt_name);
		string airport_code;

		if ((strcmp(dirEnt_name, ".") != 0) && (strcmp(dirEnt_name, "..") != 0)) {
			string read_line = "";
			char *entry;
			char* tempCharArray;

			string_sub_filenamePath.assign(dirname);
			string_sub_filenamePath.append("/");
			string_sub_filenamePath.append(dirEnt_name);

			GroundWaypointConnectivity tmpGroundWaypointConnectivity;

			airport_code = tmpString.substr(0, 4);

			if (endsWith(dirEnt_name, "Nodes_Links.csv")) {
				in.open(string_sub_filenamePath.c_str());

				tmpGroundWaypointConnectivity = map_ground_waypoint_connectivity.at(airport_code);

				AirportNodeLink tmpAirportNodeLink = AirportNodeLink();
				tmpAirportNodeLink.code.assign(dirEnt_name);

				while (in.good()) {
					read_line = "";
					getline(in, read_line);
					if (strlen(read_line.c_str()) == 0) continue;

					if (read_line.find("n1.id", 0, 5) == std::string::npos) {
						// If data size is full
						if (tmpAirportNodeLink.n1_id.size() == tmpAirportNodeLink.n1_id.capacity()) {
							int newSize = 2 * tmpAirportNodeLink.n1_id.size();

							tmpAirportNodeLink.n1_id.reserve(newSize);
							tmpAirportNodeLink.n1_index.reserve(newSize);
							tmpAirportNodeLink.n1_latitude.reserve(newSize);
							tmpAirportNodeLink.n1_longitude.reserve(newSize);
							tmpAirportNodeLink.n2_id.reserve(newSize);
							tmpAirportNodeLink.n2_index.reserve(newSize);
							tmpAirportNodeLink.n2_latitude.reserve(newSize);
							tmpAirportNodeLink.n2_longitude.reserve(newSize);
						}

						double tmpLatitude;
						double tmpLongitude;

						tempCharArray = strdup(read_line.c_str());

						// Retrieve individual data split by "," sign
						// Get first entry
						entry = strtok(tempCharArray, ",");

						if (entry != NULL) {
							tmpAirportNodeLink.n1_id.push_back(string(entry));
						}

						entry = strtok(NULL, ",");
						if (entry != NULL) {
							tmpAirportNodeLink.n1_index.push_back(atoi(entry));
						}

						entry = strtok(NULL, ",");
						if (entry != NULL) {
							tmpAirportNodeLink.n1_latitude.push_back((double)atof(entry));
						}

						entry = strtok(NULL, ",");
						if (entry != NULL) {
							tmpAirportNodeLink.n1_longitude.push_back((double)atof(entry));
						}

						entry = strtok(NULL, ",");
						if (entry != NULL) {
							tmpAirportNodeLink.n2_id.push_back(string(entry));
						}

						entry = strtok(NULL, ",");
						if (entry != NULL) {
							tmpAirportNodeLink.n2_index.push_back(atoi(entry));
						}

						entry = strtok(NULL, ",");
						if (entry != NULL) {
							tmpAirportNodeLink.n2_latitude.push_back((double)atof(entry));
						}

						entry = strtok(NULL, ",");
						if (entry != NULL) {
							tmpAirportNodeLink.n2_longitude.push_back((double)atof(entry));
						}

						free(tempCharArray);
						tempCharArray = NULL;
					}
				}

				// Set airport_node_link data
				map_ground_waypoint_connectivity.at(airport_code).airport_node_link = tmpAirportNodeLink;

				in.close();
			}
		}

		free(filenamePath);
		filenamePath = NULL;
	} // end - while loop

	closedir(dir);

	free(dirEnt);

	// Generate connectivity graph data on all airports
	generate_all_airport_connectivity();

	return 1;
}
